#ifndef _DB_POSTGRESQL_H
#define _DB_POSTGRESQL_H

#include "db.h"
#include <postgresql/libpq-fe.h>
#if __cplusplus >= 201103L
#include <unordered_map>
#else
#include <map>
#endif
#include <string>
#include <sstream>

#include "types.h"

class DBPostgreSQL : public DB {
#if __cplusplus >= 201103L
	typedef std::unordered_map<int64_t, ustring>  BlockCache;
#else
	typedef std::map<int64_t, ustring>  BlockCache;
#endif
public:
	DBPostgreSQL(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPos();
	virtual Block getBlockOnPos(const BlockPos &pos);
	~DBPostgreSQL();
private:
	int m_blocksQueriedCount;
	int m_blocksReadCount;
	PGconn *m_connection;
	BlockPosList m_blockPosList;

	uint32_t m_getBlockParams[3];
	char const *m_getBlockParamList[3];
	int m_getBlockParamLengths[3];
	int m_getBlockParamFormats[3];
};

#endif // _DB_POSTGRESQL_H
