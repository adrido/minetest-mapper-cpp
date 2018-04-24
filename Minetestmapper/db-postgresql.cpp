
#include "db-postgresql.h"

#ifdef USE_POSTGRESQL

#include <stdexcept>
#if _WIN32
#include <Winsock2.h> // htonl
#else
#include <arpa/inet.h> // htonl
#endif

#include "Settings.h"

#define BLOCKPOSLIST_QUERY_COMPAT	"SELECT x, y, z FROM blocks"
#define BLOCKPOSLISTBOUNDED_QUERY_COMPAT "SELECT x, y, z FROM blocks WHERE x BETWEEN $1 AND $2 AND y BETWEEN $3 AND $4 AND z BETWEEN $5 AND $6"
#define BLOCK_QUERY_COMPAT		"SELECT data FROM blocks WHERE x = $1 AND y = $2 AND z = $3"
#define BLOCKPOSLIST_QUERY		"SELECT posX, posY, posZ FROM blocks"
#define BLOCKPOSLISTBOUNDED_QUERY	"SELECT posX, posY, posZ FROM blocks WHERE posX BETWEEN $1 AND $2 AND posY BETWEEN $3 AND $4 AND posZ BETWEEN $5 AND $6"
#define BLOCK_QUERY			"SELECT data FROM blocks WHERE posX = $1 AND posY = $2 AND posZ = $3"

// From pg_type.h
#define PG_INT4OID		23

DBPostgreSQL::DBPostgreSQL(const std::string &mapdir) :
	m_blocksQueriedCount(0),
	m_blocksReadCount(0)
{
	Settings world_mt(mapdir + "/world.mt");
	std::string connection_info;

	bool info_found = false;
	bool compat_mode = false;

	// Official postgresql connection info string
	info_found = world_mt.check("pgsql_connection", connection_info);
	compat_mode = !info_found;

	// ShadowNinja's implementation (historical)
	if (!info_found)
		info_found = world_mt.check("postgresql_connection_info", connection_info);

	// johnnyjoy's implementation (historical)
	if (!info_found)
		info_found = world_mt.check("pg_connection_info", connection_info);

	if (!info_found)
		throw std::runtime_error("Set pgsql_connection in world.mt to use the postgresql backend");

	connection_info += "fallback_application_name=minetestmapper " + connection_info;

	m_connection = PQconnectdb(connection_info.c_str());
	if (PQstatus(m_connection) != CONNECTION_OK) {
		throw std::runtime_error(std::string("Failed to connect to postgresql database: ")
			+ PQerrorMessage(m_connection));
	}

	const char *blockposlist_query = BLOCKPOSLIST_QUERY;
	const char *blockposlistbounded_query = BLOCKPOSLISTBOUNDED_QUERY;
	const char *block_query = BLOCK_QUERY;
	if (compat_mode) {
		blockposlist_query = BLOCKPOSLIST_QUERY_COMPAT;
		blockposlistbounded_query = BLOCKPOSLISTBOUNDED_QUERY_COMPAT;
		block_query = BLOCK_QUERY_COMPAT;
	}

	PGresult *result;

	result = PQprepare(m_connection, "GetBlockPosList", blockposlist_query, 0, NULL);
	if (!result || PQresultStatus(result) != PGRES_COMMAND_OK)
		throw std::runtime_error(std::string("Failed to prepare PostgreSQL statement (GetBlockPosList): ")
			+ (result ? PQresultErrorMessage(result) : "(result was NULL)"));
	PQclear(result);

	result = PQprepare(m_connection, "GetBlockPosListBounded", blockposlistbounded_query, 0, NULL);
	if (!result || PQresultStatus(result) != PGRES_COMMAND_OK)
		throw std::runtime_error(std::string("Failed to prepare PostgreSQL statement (GetBlockPosListBounded): ")
			+ (result ? PQresultErrorMessage(result) : "(result was NULL)"));
	PQclear(result);

	result = PQprepare(m_connection, "GetBlock", block_query, 0, NULL);
	if (!result || PQresultStatus(result) != PGRES_COMMAND_OK)
		throw std::runtime_error(std::string("Failed to prepare PostgreSQL statement (GetBlock): ")
			+ (result ? PQresultErrorMessage(result) : "(result was NULL)"));
	PQclear(result);

	for (int i = 0; i < POSTGRESQL_MAXPARAMS; i++) {
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

const DB::BlockPosList &DBPostgreSQL::getBlockPosList()
{
	PGresult *result = PQexecPrepared(m_connection, "GetBlockPosList", 0, NULL, NULL, NULL, 1);
	return processBlockPosListQueryResult(result);
}

const DB::BlockPosList &DBPostgreSQL::getBlockPosList(BlockPos minPos, BlockPos maxPos)
{
	for (int i = 0; i < 3; i++) {
		m_getBlockParams[2*i] = htonl(minPos.dimension[i]);
		m_getBlockParams[2*i+1] = htonl(maxPos.dimension[i]);
	}
	PGresult *result = PQexecPrepared(m_connection, "GetBlockPosListBounded", 6, m_getBlockParamList, m_getBlockParamLengths, m_getBlockParamFormats, 1);
	return processBlockPosListQueryResult(result);
}

const DB::BlockPosList &DBPostgreSQL::processBlockPosListQueryResult(PGresult *result) {
	m_blockPosList.clear();

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


const DB::Block DBPostgreSQL::getBlockOnPos(const BlockPos &pos)
{
	Block block(pos, {});
	
	m_blocksQueriedCount++;

	for (int i = 0; i < 3; i++) {
		m_getBlockParams[i] = htonl(pos.dimension[i]);
	}

	PGresult *result = PQexecPrepared(m_connection, "GetBlock", 3, m_getBlockParamList, m_getBlockParamLengths, m_getBlockParamFormats, 1);
	if (!result || PQresultStatus(result) != PGRES_TUPLES_OK)
		throw std::runtime_error(std::string("Failed to read block from database: ")
			+ (result ? PQresultErrorMessage(result) : "(result was NULL)"));

	if (PQntuples(result) != 0) {
		block = Block(pos, reinterpret_cast<unsigned char *>(PQgetvalue(result, 0, 0)), PQgetlength(result, 0, 0));
		m_blocksReadCount++;
	}

	PQclear(result);
	return block;
}

#endif // USE_POSTGRESQL
