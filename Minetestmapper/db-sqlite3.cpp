#include "db-sqlite3.h"

#ifdef USE_SQLITE3

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

#define DATAVERSION_STATEMENT		"PRAGMA data_version"
#define BLOCKPOSLIST_STATEMENT		"SELECT pos, rowid FROM blocks"
#define BLOCKPOSLIST_LIMITED_STATEMENT	"SELECT pos, rowid FROM blocks ORDER BY pos LIMIT ? OFFSET ?"
#define BLOCK_STATEMENT_POS		"SELECT pos, data FROM blocks WHERE pos == ?"
#define BLOCK_STATEMENT_ROWID		"SELECT pos, data FROM blocks WHERE rowid == ?"

#define BLOCKLIST_QUERY_SIZE_MIN	2000
#define BLOCKLIST_QUERY_SIZE_DEFAULT	250000

#define sleepMs(x) std::this_thread::sleep_for(std::chrono::milliseconds(x))

using namespace std;

// If zero, a full block list is obtained using a single query.
// If negative, the default value (BLOCKLIST_QUERY_SIZE_DEFAULT) will be used.
int DBSQLite3::m_blockListQuerySize = 0;
bool DBSQLite3::m_firstDatabaseInitialized = false;
bool DBSQLite3::warnDatabaseLockDelay = true;

void DBSQLite3::setLimitBlockListQuerySize(int count)
{
	if (m_firstDatabaseInitialized) {
		throw std::runtime_error("Cannot set or change SQLite3 prescan query size: database is already open");
	}
	if (count > 0 && count < BLOCKLIST_QUERY_SIZE_MIN) {
		std::cerr << "Limit for SQLite3 prescan query size is too small - increased to " << BLOCKLIST_QUERY_SIZE_MIN << std::endl;
		count = BLOCKLIST_QUERY_SIZE_MIN;
	}
	if (count < 0) {
		m_blockListQuerySize = BLOCKLIST_QUERY_SIZE_DEFAULT;
	}
	else {
		m_blockListQuerySize = count;
	}
}

DBSQLite3::DBSQLite3(const std::string &mapdir) :
	m_blocksQueriedCount(0),
	m_blocksReadCount(0)
{

	m_firstDatabaseInitialized = true;
	std::string db_name = mapdir + "map.sqlite";
	if (sqlite3_open_v2(db_name.c_str(), &m_db, SQLITE_OPEN_READONLY | SQLITE_OPEN_PRIVATECACHE, nullptr) != SQLITE_OK) {
		throw runtime_error(std::string(sqlite3_errmsg(m_db)) + ", Database file: " + db_name);
	}
	if (SQLITE_OK != sqlite3_prepare_v2(m_db, DATAVERSION_STATEMENT, sizeof(DATAVERSION_STATEMENT) - 1, &m_dataVersionStatement, nullptr)) {
		throw runtime_error(string("Failed to prepare SQL statement (dataVersionStatement): ") + sqlite3_errmsg(m_db));
	}
	if (m_blockListQuerySize == 0) {
		if (SQLITE_OK != sqlite3_prepare_v2(m_db, BLOCKPOSLIST_STATEMENT, sizeof(BLOCKPOSLIST_STATEMENT) - 1, &m_blockPosListStatement, nullptr)) {
			throw runtime_error(string("Failed to prepare SQL statement (blockPosListStatement): ") + sqlite3_errmsg(m_db));
		}
	}
	else {
		if (SQLITE_OK != sqlite3_prepare_v2(m_db, BLOCKPOSLIST_LIMITED_STATEMENT,
			sizeof(BLOCKPOSLIST_LIMITED_STATEMENT) - 1, &m_blockPosListStatement, nullptr)) {
			throw runtime_error(string("Failed to prepare SQL statement (blockPosListStatement (limited)): ") + sqlite3_errmsg(m_db));
		}
	}
	if (SQLITE_OK != sqlite3_prepare_v2(m_db, BLOCK_STATEMENT_POS, sizeof(BLOCK_STATEMENT_POS) - 1, &m_blockOnPosStatement, nullptr)) {
		throw runtime_error(string("Failed to prepare SQL statement (blockOnPosStatement): ") + +sqlite3_errmsg(m_db));
	}
	if (SQLITE_OK != sqlite3_prepare_v2(m_db, BLOCK_STATEMENT_ROWID, sizeof(BLOCK_STATEMENT_ROWID) - 1, &m_blockOnRowidStatement, nullptr)) {
		throw runtime_error(string("Failed to prepare SQL statement (blockOnRowidStatement): ") + sqlite3_errmsg(m_db));
	}
}

DBSQLite3::~DBSQLite3() {
	if (m_blockPosListStatement) {
		sqlite3_finalize(m_blockPosListStatement);
	}
	if (m_blockOnPosStatement) {
		sqlite3_finalize(m_blockOnPosStatement);
	}
	if (m_blockOnRowidStatement) {
		sqlite3_finalize(m_blockOnRowidStatement);
	}
	sqlite3_close(m_db);
}

int DBSQLite3::getBlocksReadCount()
{
	return m_blocksReadCount;
}

int DBSQLite3::getBlocksQueriedCount()
{
	return m_blocksQueriedCount;
}

int64_t DBSQLite3::getDataVersion()
{
	int64_t version = 0;
	for (;;) {
		int result = sqlite3_step(m_dataVersionStatement);
		if (result == SQLITE_ROW) {
			version = sqlite3_column_int64(m_dataVersionStatement, 0);
		}
		else if (result == SQLITE_BUSY) { // Wait some time and try again
			sleepMs(10);
		}
		else {
			break;
		}
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
	if (m_blockListQuerySize <= 10000) {
		querySize = m_blockListQuerySize + 1000;
	}
	else if (m_blockListQuerySize <= 100000) {
		querySize = static_cast<int>(m_blockListQuerySize * 1.1f);            // 1000 .. 10000
	}
	else if (m_blockListQuerySize <= 1000000) {
		querySize = static_cast<int>(m_blockListQuerySize * 1.011f) + 9000;   // 10100 .. 20000
	}
	else if (m_blockListQuerySize <= 10000000) {
		querySize = static_cast<int>(m_blockListQuerySize * 1.0022f) + 18000; // 20200 .. 40000
	}
	else {	// More than 10M blocks per query. Most worlds are smaller than this...
		querySize = static_cast<int>(m_blockListQuerySize * 1.001f) + 30000;  // 40000 .. (130K blocks extra for 100M blocks, etc.)
	}

	int rows = 1;
	for (int offset = 0; rows > 0; offset += m_blockListQuerySize) {
		sqlite3_bind_int(m_blockPosListStatement, 1, querySize);
		sqlite3_bind_int(m_blockPosListStatement, 2, offset);
		rows = getBlockPosListRows();
		sqlite3_reset(m_blockPosListStatement);
		if (rows > 0) {
			sleepMs(10);		// Be nice to a concurrent user
		}
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

	auto time0 = std::chrono::steady_clock::now();

	while (true) {
		int result = sqlite3_step(m_blockPosListStatement);
		if (result == SQLITE_ROW) {
			rows++;
			sqlite3_int64 blocknum = sqlite3_column_int64(m_blockPosListStatement, 0);
			sqlite3_int64 rowid = sqlite3_column_int64(m_blockPosListStatement, 1);
			if (!m_blockListQuerySize || m_blockIdSet.insert(blocknum).second) {
				m_blockPosList.push_back(BlockPos(blocknum, rowid));
			}
		}
		else if (result == SQLITE_BUSY) { // Wait some time and try again
			sleepMs(10);
		}
		else {
			break;
		}
	}

	auto time1 = std::chrono::steady_clock::now();
	uint64_t diff = std::chrono::duration_cast<std::chrono::nanoseconds>(time1 - time0).count();
	if (diff > m_blockPosListQueryTime) {
		m_blockPosListQueryTime = diff;
	}

	return rows;
}


const DB::Block DBSQLite3::getBlockOnPos(const BlockPos &pos)
{
	static thread_local MapBlock block;
	block.reset();
	block.setPos(pos);
	int result = 0;

	m_blocksQueriedCount++;

	sqlite3_stmt *statement;

	// Disabled RowID querying, as it may cause blocks not to be found when mapping
	// while minetest is running (i.e. modifying blocks).
	if (false && pos.databasePosIdIsValid()) {
		statement = m_blockOnRowidStatement;
		sqlite3_bind_int64(m_blockOnRowidStatement, 1, pos.databasePosId());
	}
	else {
		statement = m_blockOnPosStatement;
		sqlite3_bind_int64(m_blockOnPosStatement, 1, pos.databasePosI64());
	}

	while (true) {
		result = sqlite3_step(statement);
		if (result == SQLITE_ROW) {
			//const auto *data = static_cast<const unsigned char *>(sqlite3_column_blob(statement, 1));
			int size = sqlite3_column_bytes(statement, 1);
			//block = MapBlock(pos, sqlite3_column_blob(statement, 1), size);
			block.setData(sqlite3_column_blob(statement, 1), size);
			//block.second.assign(&data[0], &data[size]);
			m_blocksReadCount++;
			break;
		}
		else if (result == SQLITE_BUSY) { // Wait some time and try again
			sleepMs(10);
		}
		else {
			break;
		}
	}
	sqlite3_reset(statement);

	return block;
}

#endif // USE_SQLITE3
