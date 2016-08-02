#ifndef _DB_SQLITE3_H
#define _DB_SQLITE3_H

#include "db.h"
#include <sqlite3.h>
#if __cplusplus >= 201103L
#include <unordered_map>
#else
#include <map>
#endif
#include <string>
#include <sstream>

#include "types.h"

class DBSQLite3 : public DB {
#if __cplusplus >= 201103L
	typedef std::unordered_map<int64_t, ustring>  BlockCache;
#else
	typedef std::map<int64_t, ustring>  BlockCache;
#endif
public:
	DBSQLite3(const std::string &mapdir);
	virtual int getBlocksQueriedCount(void);
	virtual int getBlocksReadCount(void);
	virtual const BlockPosList &getBlockPosList();
	virtual Block getBlockOnPos(const BlockPos &pos);
	~DBSQLite3();
private:
	int m_blocksQueriedCount;
	int m_blocksReadCount;
	sqlite3 *m_db;
	sqlite3_stmt *m_blockPosListStatement;
	sqlite3_stmt *m_blockOnPosStatement;
	sqlite3_stmt *m_blockOnRowidStatement;
	std::ostringstream  m_getBlockSetStatementBlocks;
	BlockCache  m_blockCache;
	BlockPosList m_blockPosList;

	void prepareBlockOnPosStatement(void);
	Block getBlockOnPosRaw(const BlockPos &pos);
	void cacheBlocks(sqlite3_stmt *SQLstatement);
};

#endif // _DB_SQLITE3_H
