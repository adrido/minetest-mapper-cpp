#ifndef _DB_LEVELDB_H
#define _DB_LEVELDB_H

#include "db.h"
#include <leveldb/db.h>
#include <set>

class DBLevelDB : public DB {
public:
	DBLevelDB(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPos();
	virtual Block getBlockOnPos(const BlockPos &pos);
	~DBLevelDB();
private:
	int m_blocksReadCount;
	int m_blocksQueriedCount;
	leveldb::DB *m_db;
	BlockPosList m_blockPosList;
};

#endif // _DB_LEVELDB_H
