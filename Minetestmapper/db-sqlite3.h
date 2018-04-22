#pragma once

#include "config.h"
#ifdef USE_SQLITE3

#include "db.h"
#include <sqlite3.h>
#include <string>
#include <unordered_map>
#include <unordered_set>


class DBSQLite3 : public DB {
	typedef std::unordered_map<int64_t, std::vector<unsigned char>>  BlockCache;
	typedef std::unordered_set<int64_t>  BlockIdSet;

public:
	DBSQLite3(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPosList();
	virtual const Block getBlockOnPos(const BlockPos &pos);
	~DBSQLite3();

	static void setLimitBlockListQuerySize(int count = -1);
	static bool warnDatabaseLockDelay;
private:
	static int m_blockListQuerySize;
	static bool m_firstDatabaseInitialized;

	int m_blocksQueriedCount;
	int m_blocksReadCount;
	sqlite3 *m_db = nullptr;
	sqlite3_stmt *m_dataVersionStatement = nullptr;
	sqlite3_stmt *m_blockPosListStatement = nullptr;
	sqlite3_stmt *m_blockOnPosStatement = nullptr;
	sqlite3_stmt *m_blockOnRowidStatement = nullptr;
	std::ostringstream  m_getBlockSetStatementBlocks;
	BlockCache  m_blockCache;
	BlockPosList m_blockPosList;
	BlockIdSet m_blockIdSet;		// temporary storage. Only used if m_blockListQuerySize > 0

	uint64_t m_blockPosListQueryTime;

	int64_t getDataVersion();
	void prepareBlockOnPosStatement();
	int getBlockPosListRows();
	Block getBlockOnPosRaw(const BlockPos &pos);
	void cacheBlocks(sqlite3_stmt *SQLstatement);
};

#endif // USE_SQLITE3
