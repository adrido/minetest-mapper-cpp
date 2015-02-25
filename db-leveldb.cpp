#include "db-leveldb.h"
#include <stdexcept>
#include <sstream>
#include "types.h"

inline int64_t stoi64(const std::string &s) {
	std::stringstream tmp(s);
	long long t;
	tmp >> t;
	return t;
}

inline std::string i64tos(int64_t i) {
	std::ostringstream o;
	o<<i;
	return o.str();
}

DBLevelDB::DBLevelDB(const std::string &mapdir) :
	m_blocksReadCount(0),
	m_blocksQueriedCount(0)
{
	leveldb::Options options;
	options.create_if_missing = false;
	leveldb::Status status = leveldb::DB::Open(options, mapdir + "map.db", &m_db);
	if(!status.ok())
		throw std::runtime_error(std::string("Failed to open Database: ") + status.ToString());
}

DBLevelDB::~DBLevelDB() {
	delete m_db;
}

int DBLevelDB::getBlocksReadCount(void)
{
	return m_blocksReadCount;
}

int DBLevelDB::getBlocksQueriedCount(void)
{
	return m_blocksQueriedCount;
}

const DB::BlockPosList &DBLevelDB::getBlockPos() {
	m_blockPosList.clear();
	leveldb::Iterator* it = m_db->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		m_blockPosList.push_back(BlockPos(it->key().ToString()));
	}
	delete it;
	return m_blockPosList;
}

DB::Block DBLevelDB::getBlockOnPos(const BlockPos &pos)
{
	std::string datastr;
	leveldb::Status status;

	m_blocksQueriedCount++;

	status = m_db->Get(leveldb::ReadOptions(), pos.databasePosStr(), &datastr);
	if(status.ok()) {
		m_blocksReadCount++;
		return Block(pos, ustring(reinterpret_cast<const unsigned char *>(datastr.c_str()), datastr.size()));
	}
	else {
		return Block(pos, ustring(reinterpret_cast<const unsigned char *>("")));
	}

}

