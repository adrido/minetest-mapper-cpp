#include <stdexcept>
#include <sstream>
#include <fstream>
#include "db-redis.h"
#include "types.h"
#include "Settings.h"

static inline int64_t stoi64(const std::string &s)
{
	std::stringstream tmp(s);
	int64_t t;
	tmp >> t;
	return t;
}


#if 0
static inline std::string i64tos(int64_t i)
{
	std::ostringstream os;
	os << i;
	return os.str();
}
#endif

DBRedis::DBRedis(const std::string &mapdir) :
	m_blocksReadCount(0),
	m_blocksQueriedCount(0)
{
	Settings world_mt(mapdir + "/world.mt");
	std::string address;
	if (!world_mt.check("redis_address", address) || !world_mt.check("redis_hash", hash)) {
		throw std::runtime_error("Set redis_address and redis_hash in world.mt to use the redis backend");
	}
	int port = stoi64(world_mt.get("redis_port", "6379"));
	ctx = redisConnect(address.c_str(), port);
	if(!ctx)
		throw std::runtime_error("Cannot allocate redis context");
	else if(ctx->err) {
		std::string err = std::string("Connection error: ") + ctx->errstr;
		redisFree(ctx);
		throw std::runtime_error(err);
	}
}


DBRedis::~DBRedis()
{
	redisFree(ctx);
}


int DBRedis::getBlocksReadCount(void)
{
	return m_blocksReadCount;
}

int DBRedis::getBlocksQueriedCount(void)
{
	return m_blocksQueriedCount;
}


const DB::BlockPosList &DBRedis::getBlockPosList()
{
	redisReply *reply;
	reply = (redisReply*) redisCommand(ctx, "HKEYS %s", hash.c_str());
	if(!reply)
		throw std::runtime_error(std::string("redis command 'HKEYS %s' failed: ") + ctx->errstr);
	if(reply->type != REDIS_REPLY_ARRAY)
		throw std::runtime_error("Failed to get keys from database");
	for(size_t i = 0; i < reply->elements; i++) {
		if(reply->element[i]->type != REDIS_REPLY_STRING)
			throw std::runtime_error("Got wrong response to 'HKEYS %s' command");
		m_blockPosList.push_back(BlockPos(reply->element[i]->str));
	}
	
	freeReplyObject(reply);
	return m_blockPosList;
}


DB::Block DBRedis::getBlockOnPos(const BlockPos &pos)
{
	redisReply *reply;
	std::string tmp;
	Block block(pos,reinterpret_cast<const unsigned char *>(""));

	m_blocksQueriedCount++;

	reply = (redisReply*) redisCommand(ctx, "HGET %s %s", hash.c_str(), pos.databasePosStr().c_str());
	if(!reply)
		throw std::runtime_error(std::string("redis command 'HGET %s %s' failed: ") + ctx->errstr);
	if (reply->type == REDIS_REPLY_STRING && reply->len != 0) {
		m_blocksReadCount++;
		block = Block(pos, ustring(reinterpret_cast<const unsigned char *>(reply->str), reply->len));
	} else
		throw std::runtime_error("Got wrong response to 'HGET %s %s' command");
	freeReplyObject(reply);

	return block;
}

