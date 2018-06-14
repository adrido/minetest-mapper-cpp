#pragma once

#include "config.h"
#ifdef USE_REDIS



#include "db.h"
#include <hiredis.h>

class DBRedis : public DB {
public:
	DBRedis(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPosList();
	virtual const Block getBlockOnPos(const BlockPos &pos);
	~DBRedis();
private:
	int m_blocksReadCount;
	int m_blocksQueriedCount;
	redisContext *ctx;
	std::string hash;
	BlockPosList m_blockPosList;
};

#endif // USE_REDIS
