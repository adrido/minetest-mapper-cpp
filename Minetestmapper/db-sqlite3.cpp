#include "db-sqlite3.h"

#ifdef USE_SQLITE3

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "porting.h"
#include "types.h"

#define DATAVERSION_STATEMENT		"PRAGMA data_version"
#define BLOCKPOSLIST_STATEMENT		"SELECT pos, rowid FROM blocks"
#define BLOCKPOSLIST_LIMITED_STATEMENT	"SELECT pos, rowid FROM blocks ORDER BY pos LIMIT ? OFFSET ?"
#define BLOCK_STATEMENT_POS		"SELECT pos, data FROM blocks WHERE pos == ?"
#define BLOCK_STATEMENT_ROWID		"SELECT pos, data FROM blocks WHERE rowid == ?"

#define BLOCKLIST_QUERY_SIZE_MIN	2000
#define BLOCKLIST_QUERY_SIZE_DEFAULT	250000

// If zero, a full block list is obtained using a single query.
// If negative, the default value (BLOCKLIST_QUERY_SIZE_DEFAULT) will be used.
int DBSQLite3::m_blockListQuerySize = 0;
bool DBSQLite3::m_firstDatabaseInitialized = false;
bool DBSQLite3::warnDatabaseLockDelay = true;

void DBSQLite3::setLimitBlockListQuerySize(int count)
{
	if (m_firstDatabaseInitialized)
		throw std::runtime_error("Cannot set or change SQLite3 prescan query size: database is already open");
	if (count > 0 && count < BLOCKLIST_QUERY_SIZE_MIN) {
		std::cerr << "Limit for SQLite3 prescan query size is too small - increased to " << BLOCKLIST_QUERY_SIZE_MIN << std::endl;
		count = BLOCKLIST_QUERY_SIZE_MIN;
	}
	if (count < 0)
		m_blockListQuerySize = BLOCKLIST_QUERY_SIZE_DEFAULT;
	else
		m_blockListQuerySize = count;
}

DBSQLite3::DBSQLite3(const std::string &mapdir) :
	m_blocksQueriedCount(0),
	m_blocksReadCount(0),
	m_blockPosListStatement(NULL),
	m_blockOnPosStatement(NULL),
	m_blockOnRowidStatement(NULL)
{
	
	m_firstDatabaseInitialized = true;
	std::string db_name = mapdir + "map.sqlite";
	if (sqlite3_open_v2(db_name.c_str(), &m_db, SQLITE_OPEN_READONLY | SQLITE_OPEN_PRIVATECACHE, 0) != SQLITE_OK) {
		throw std::runtime_error(std::string(sqlite3_errmsg(m_db)) + ", Database file: " + db_name);
	}
	if (SQLITE_OK != sqlite3_prepare_v2(m_db, DATAVERSION_STATEMENT, sizeof(DATAVERSION_STATEMENT)-1, &m_dataVersionStatement, 0)) {
		throw std::runtime_error("Failed to prepare SQL statement (dataVersionStatement)");
	}
	if (m_blockListQuerySize == 0) {
		if (SQLITE_OK != sqlite3_prepare_v2(m_db, BLOCKPOSLIST_STATEMENT, sizeof(BLOCKPOSLIST_STATEMENT)-1, &m_blockPosListStatement, 0)) {
			throw std::runtime_error("Failed to prepare SQL statement (blockPosListStatement)");
		}
	}
	else {
		if (SQLITE_OK != sqlite3_prepare_v2(m_db, BLOCKPOSLIST_LIMITED_STATEMENT,
				sizeof(BLOCKPOSLIST_LIMITED_STATEMENT)-1, &m_blockPosListStatement, 0)) {
			throw std::runtime_error("Failed to prepare SQL statement (blockPosListStatement (limited))");
		}
	}
	if (SQLITE_OK != sqlite3_prepare_v2(m_db, BLOCK_STATEMENT_POS, sizeof(BLOCK_STATEMENT_POS)-1, &m_blockOnPosStatement, 0)) {
		throw std::runtime_error("Failed to prepare SQL statement (blockOnPosStatement)");
	}
	if (SQLITE_OK != sqlite3_prepare_v2(m_db, BLOCK_STATEMENT_ROWID, sizeof(BLOCK_STATEMENT_ROWID)-1, &m_blockOnRowidStatement, 0)) {
		throw std::runtime_error("Failed to prepare SQL statement (blockOnRowidStatement)");
	}
}

DBSQLite3::~DBSQLite3() {
	if (m_blockPosListStatement) sqlite3_finalize(m_blockPosListStatement);
	if (m_blockOnPosStatement) sqlite3_finalize(m_blockOnPosStatement);
	if (m_blockOnRowidStatement) sqlite3_finalize(m_blockOnRowidStatement);
	sqlite3_close(m_db);
}

int DBSQLite3::getBlocksReadCount(void)
{
	return m_blocksReadCount;
}

int DBSQLite3::getBlocksQueriedCount(void)
{
	return m_blocksQueriedCount;
}

int64_t DBSQLite3::getDataVersion()
{
	int64_t version = 0;
	for (;;) {
		int result = sqlite3_step(m_dataVersionStatement);
		if(result == SQLITE_ROW)
			version = sqlite3_column_int64(m_dataVersionStatement, 0);
		else if (result == SQLITE_BUSY) // Wait some time and try again
			sleepMs(10);
		else
			break;
	}
	sqlite3_reset(m_blockPosListStatement);
	return version;
}

const DB::BlockPosList &DBSQLite3::getBlockPosList()
{
	m_blockPosList.clear();

	m_blockPosListQueryTime = 0;
	int64_t dataVersionStart = getDataVersion();

	if (!m_blockListQuerySize) {

		getBlockPosListRows();
		sqlite3_reset(m_blockPosListStatement);

		if (m_blockPosListQueryTime >= 1000 && warnDatabaseLockDelay && getDataVersion() != dataVersionStart) {
			std::ostringstream oss;
			oss << "WARNING: "
				<< "Block list query duration was "
				<< m_blockPosListQueryTime / 1000 << "."
				<< std::fixed << std::setw(3) << std::setfill('0')
				<< m_blockPosListQueryTime % 1000 << " seconds"
				<< " while another process modified the database. Consider using --sqlite3-limit-prescan-query-size";
			std::cout << oss.str() << std::endl;
		}

		return m_blockPosList;
	}

	// As queries overlap, remember which id's have been seen to avoid duplicates
	m_blockIdSet.clear();

	int querySize = 0;

	// Select some more blocks than requested, to make sure that newly inserted
	// blocks don't cause other blocks to be skipped.
	// The enforced minimum of 1000 is probably too small to avoid skipped blocks
	// if heavy world generation is going on, but then, a query size of 10000 is
	// also pretty small.
	if (m_blockListQuerySize <= 10000)
		querySize = m_blockListQuerySize + 1000;
	else if (m_blockListQuerySize <= 100000)
		querySize = m_blockListQuerySize * 1.1;			// 1000 .. 10000
	else if (m_blockListQuerySize <= 1000000)
		querySize = m_blockListQuerySize * 1.011 + 9000;	// 10100 .. 20000
	else if (m_blockListQuerySize <= 10000000)
		querySize = m_blockListQuerySize * 1.0022 + 18000;	// 20200 .. 40000
	else	// More than 10M blocks per query. Most worlds are smaller than this...
		querySize = m_blockListQuerySize * 1.001 + 30000;	// 40000 .. (130K blocks extra for 100M blocks, etc.)

	int rows = 1;
	for (int offset = 0; rows > 0; offset += m_blockListQuerySize) {
		sqlite3_bind_int(m_blockPosListStatement, 1, querySize);
		sqlite3_bind_int(m_blockPosListStatement, 2, offset);
		rows = getBlockPosListRows();
		sqlite3_reset(m_blockPosListStatement);
		if (rows > 0)
			sleepMs(10);		// Be nice to a concurrent user
	}

	m_blockIdSet.clear();

	if (m_blockPosListQueryTime >= 1000 && warnDatabaseLockDelay && getDataVersion() != dataVersionStart) {
		std::ostringstream oss;
		oss << "WARNING: "
			<< "Maximum block list query duration was "
			<< m_blockPosListQueryTime / 1000 << "."
			<< std::fixed << std::setw(3) << std::setfill('0')
			<< m_blockPosListQueryTime % 1000 << " seconds"
			<< " while another process modified the database. Consider decreasing --sqlite3-limit-prescan-query-size";
		std::cout << oss.str() << std::endl;
	}

	return m_blockPosList;
}

int DBSQLite3::getBlockPosListRows()
{
	int rows = 0;

	uint64_t time0 = getRelativeTimeStampMs();

	while (true) {
		int result = sqlite3_step(m_blockPosListStatement);
		if(result == SQLITE_ROW) {
			rows++;
			sqlite3_int64 blocknum = sqlite3_column_int64(m_blockPosListStatement, 0);
			sqlite3_int64 rowid = sqlite3_column_int64(m_blockPosListStatement, 1);
			if (!m_blockListQuerySize || m_blockIdSet.insert(blocknum).second) {
				m_blockPosList.push_back(BlockPos(blocknum, rowid));
			}
		} else if (result == SQLITE_BUSY) // Wait some time and try again
			sleepMs(10);
		else
			break;
	}

	uint64_t time1 = getRelativeTimeStampMs();
	if (time1 - time0 > m_blockPosListQueryTime)
		m_blockPosListQueryTime = time1 - time0;

	return rows;
}


DB::Block DBSQLite3::getBlockOnPos(const BlockPos &pos)
{
	Block block(pos,reinterpret_cast<const unsigned char *>(""));
	int result = 0;
	
	m_blocksQueriedCount++;

	sqlite3_stmt *statement;

	// Disabled RowID querying, as it may cause blocks not to be found when mapping
	// while minetest is running (i.e. modifying blocks).
	if (0 && pos.databasePosIdIsValid()) {
		statement = m_blockOnRowidStatement;
		sqlite3_bind_int64(m_blockOnRowidStatement, 1, pos.databasePosId());
	}
	else {
		statement = m_blockOnPosStatement;
		sqlite3_bind_int64(m_blockOnPosStatement, 1, pos.databasePosI64());
	}

	while (true) {
		result = sqlite3_step(statement);
		if(result == SQLITE_ROW) {
			const unsigned char *data = reinterpret_cast<const unsigned char *>(sqlite3_column_blob(statement, 1));
			int size = sqlite3_column_bytes(statement, 1);
			block = Block(pos, ustring(data, size));
			m_blocksReadCount++;
			break;
		} else if (result == SQLITE_BUSY) { // Wait some time and try again
			sleepMs(10);
		} else {
			break;
		}
	}
	sqlite3_reset(statement);

	return block;
}

#endif // USE_SQLITE3
