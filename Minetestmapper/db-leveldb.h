#pragma once

#ifdef USE_LEVELDB

#include "db.h"
#include <leveldb/db.h>
#include <set>

class DBLevelDB : public DB {
public:
	DBLevelDB(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPosList();
	virtual Block getBlockOnPos(const BlockPos &pos);
	~DBLevelDB();
private:
	int m_blocksReadCount;
	int m_blocksQueriedCount;
	leveldb::DB *m_db;
	BlockPosList m_blockPosList;
	unsigned m_keyFormatI64Usage;
	unsigned m_keyFormatAXYZUsage;
};
#endif // USE_LEVELDB

