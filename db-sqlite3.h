#ifndef _DB_SQLITE3_H
#define _DB_SQLITE3_H

#include "db.h"
#include <sqlite3.h>
#if __cplusplus >= 201103L
#include <unordered_map>
#include <unordered_set>
#else
#include <map>
#include <set>
#endif
#include <string>
#include <sstream>

#include "types.h"

class DBSQLite3 : public DB {
#if __cplusplus >= 201103L
	typedef std::unordered_map<int64_t, ustring>  BlockCache;
	typedef std::unordered_set<int64_t>  BlockIdSet;
#else
	typedef std::map<int64_t, ustring>  BlockCache;
	typedef std::set<int64_t>  BlockIdSet;
#endif
public:
	DBSQLite3(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPosList();
	virtual Block getBlockOnPos(const BlockPos &pos);
	~DBSQLite3();

	static void setLimitBlockListQuerySize(int count = -1);
	static bool warnDatabaseLockDelay;
private:
	static int m_blockListQuerySize;
	static bool m_firstDatabaseInitialized;

	int m_blocksQueriedCount;
	int m_blocksReadCount;
	sqlite3 *m_db;
	sqlite3_stmt *m_dataVersionStatement;
	sqlite3_stmt *m_blockPosListStatement;
	sqlite3_stmt *m_blockOnPosStatement;
	sqlite3_stmt *m_blockOnRowidStatement;
	std::ostringstream  m_getBlockSetStatementBlocks;
	BlockCache  m_blockCache;
	BlockPosList m_blockPosList;
	BlockIdSet m_blockIdSet;		// temporary storage. Only used if m_blockListQuerySize > 0

	uint64_t m_blockPosListQueryTime;

	int64_t getDataVersion();
	void prepareBlockOnPosStatement(void);
	int getBlockPosListRows();
	Block getBlockOnPosRaw(const BlockPos &pos);
	void cacheBlocks(sqlite3_stmt *SQLstatement);
};

#endif // _DB_SQLITE3_H
