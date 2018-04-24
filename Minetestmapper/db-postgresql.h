#pragma once

#include "config.h"

#ifdef USE_POSTGRESQL

#include "db.h"
#include <libpq-fe.h>
#include <unordered_map>
#include <string>
#include <sstream>


class DBPostgreSQL : public DB {
public:
	DBPostgreSQL(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPosList();
	virtual const BlockPosList &getBlockPosList(BlockPos minPos, BlockPos maxPos);
	virtual const Block getBlockOnPos(const BlockPos &pos);
	~DBPostgreSQL();
private:
	int m_blocksQueriedCount;
	int m_blocksReadCount;
	PGconn *m_connection;
	BlockPosList m_blockPosList;

	#define POSTGRESQL_MAXPARAMS 6
	uint32_t m_getBlockParams[POSTGRESQL_MAXPARAMS];
	char const *m_getBlockParamList[POSTGRESQL_MAXPARAMS];
	int m_getBlockParamLengths[POSTGRESQL_MAXPARAMS];
	int m_getBlockParamFormats[POSTGRESQL_MAXPARAMS];

	const BlockPosList &processBlockPosListQueryResult(PGresult *result);
};

#endif // USE_POSTGRESQL
