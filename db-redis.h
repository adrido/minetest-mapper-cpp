#ifndef DB_REDIS_HEADER
#define DB_REDIS_HEADER

#include "db.h"
#include <hiredis.h>

class DBRedis : public DB {
public:
	DBRedis(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPos();
	virtual Block getBlockOnPos(const BlockPos &pos);
	~DBRedis();
private:
	int m_blocksReadCount;
	int m_blocksQueriedCount;
	redisContext *ctx;
	std::string hash;
	BlockPosList m_blockPosList;
};

#endif // DB_REDIS_HEADER
