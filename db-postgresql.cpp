#include "db-postgresql.h"
#include <stdexcept>
#include <unistd.h> // for usleep
#include <arpa/inet.h>
#include "Settings.h"
#include "types.h"

#define BLOCKPOSLIST_QUERY	"SELECT x, y, z FROM blocks"
#define BLOCK_QUERY		"SELECT data FROM blocks WHERE x = $1 AND y = $2 AND z = $3"

// From pg_type.h
#define PG_INT4OID		23

DBPostgreSQL::DBPostgreSQL(const std::string &mapdir) :
	m_blocksQueriedCount(0),
	m_blocksReadCount(0)
{
	Settings world_mt(mapdir + "/world.mt");
	std::string connection_info;

	bool info_found = false;
	// ShadowNinja's implementation
	info_found = world_mt.check("postgresql_connection_info", connection_info);
	// johnnyjoy's implementation
	// The default value is not used here as it seems to me it has a serious issue:
	// creating two worlds without specifying pg_connection_info will result in both
	// worlds using the same database.
	if (!info_found)
		info_found = world_mt.check("pg_connection_info", connection_info);
	if (!info_found)
		throw std::runtime_error("Set postgresql_connection_info or pg_connection_info in world.mt to use the postgresql backend");

	connection_info += "fallback_application_name=minetestmapper " + connection_info;

	m_connection = PQconnectdb(connection_info.c_str());
	if (PQstatus(m_connection) != CONNECTION_OK) {
		throw std::runtime_error(std::string("Failed to connect to postgresql database: ")
			+ PQerrorMessage(m_connection));
	}

	PGresult *result;
	result = PQprepare(m_connection, "GetBlockPosList", BLOCKPOSLIST_QUERY, 0, NULL);
	if (!result || PQresultStatus(result) != PGRES_COMMAND_OK)
		throw std::runtime_error(std::string("Failed to prepare PostgreSQL statement (GetBlockPosList): ")
			+ (result ? PQresultErrorMessage(result) : "(result was NULL)"));
	PQclear(result);

	result = PQprepare(m_connection, "GetBlock", BLOCK_QUERY, 0, NULL);
	if (!result || PQresultStatus(result) != PGRES_COMMAND_OK)
		throw std::runtime_error(std::string("Failed to prepare PostgreSQL statement (GetBlock): ")
			+ (result ? PQresultErrorMessage(result) : "(result was NULL)"));
	PQclear(result);

	for (int i = 0; i < 3; i++) {
		m_getBlockParamList[i] = reinterpret_cast<char const *>(m_getBlockParams + i);
		m_getBlockParamLengths[i] = sizeof(int32_t);
		m_getBlockParamFormats[i] = 1;
	}
}

DBPostgreSQL::~DBPostgreSQL()
{
	PQfinish(m_connection);
}

int DBPostgreSQL::getBlocksReadCount(void)
{
	return m_blocksReadCount;
}

int DBPostgreSQL::getBlocksQueriedCount(void)
{
	return m_blocksQueriedCount;
}

const DB::BlockPosList &DBPostgreSQL::getBlockPos() {
	m_blockPosList.clear();

	PGresult *result = PQexecPrepared(m_connection, "GetBlockPosList", 0, NULL, NULL, NULL, 1);
	if (!result || PQresultStatus(result) != PGRES_TUPLES_OK)
		throw std::runtime_error(std::string("Failed to read block-pos list from database: ")
			+ (result ? PQresultErrorMessage(result) : "(result was NULL)"));

	int rows = PQntuples(result);

	// Make sure that we got the right data types
	if (rows &&
		(  PQftype(result, 0) != PG_INT4OID
		|| PQftype(result, 1) != PG_INT4OID
		|| PQftype(result, 2) != PG_INT4OID)) {
		throw std::runtime_error(std::string("Unexpected data type of block coordinate in database query result."));
	}

	for (int i = 0; i < rows; i++) {
		int32_t x = ntohl(*reinterpret_cast<uint32_t *>(PQgetvalue(result, i, 0)));
		int32_t y = ntohl(*reinterpret_cast<uint32_t *>(PQgetvalue(result, i, 1)));
		int32_t z = ntohl(*reinterpret_cast<uint32_t *>(PQgetvalue(result, i, 2)));
		m_blockPosList.push_back(BlockPos(x, y, z, BlockPos::XYZ));
	}

	PQclear(result);
	return m_blockPosList;
}


DB::Block DBPostgreSQL::getBlockOnPos(const BlockPos &pos)
{
	Block block(pos,reinterpret_cast<const unsigned char *>(""));
	
	m_blocksQueriedCount++;

	for (int i = 0; i < 3; i++) {
		m_getBlockParams[i] = htonl(pos.dimension[i]);
	}

	PGresult *result = PQexecPrepared(m_connection, "GetBlock", 3, m_getBlockParamList, m_getBlockParamLengths, m_getBlockParamFormats, 1);
	if (!result || PQresultStatus(result) != PGRES_TUPLES_OK)
		throw std::runtime_error(std::string("Failed to read block from database: ")
			+ (result ? PQresultErrorMessage(result) : "(result was NULL)"));

	if (PQntuples(result) != 0) {
		block = Block(pos, ustring(reinterpret_cast<unsigned char *>(PQgetvalue(result, 0, 0)), PQgetlength(result, 0, 0)));
		m_blocksReadCount++;
	}

	PQclear(result);
	return block;
}

