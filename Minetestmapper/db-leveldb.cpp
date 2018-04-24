#include "build_config.h"

#ifdef USE_LEVELDB

#include "db-leveldb.h"
#include <stdexcept>
#include <sstream>

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
	m_blocksQueriedCount(0),
	m_keyFormatI64Usage(0),
	m_keyFormatAXYZUsage(0)
{
	leveldb::Options options;
	options.create_if_missing = false;
	leveldb::Status status = leveldb::DB::Open(options, mapdir + "map.db", &m_db);
	if(!status.ok())
		#if CPP_ABI_STDSTRING_OK
		throw std::runtime_error(std::string("Failed to open Database: ") + status.ToString());
		#else
		throw std::runtime_error(std::string("Failed to open Database: ") +
			( status.ok() ? "Ok (??? how is this possible ???"")"		// "" is needed to prevent interpretation as trigraph -shudder-
			: status.IsNotFound() ? "NotFound"
			: status.IsCorruption() ? "Corruption"
			: status.IsIOError() ? "IOError"
			: "Cannot determine error type - could be NotSupported or InvalidArgument or something else"));
		#endif
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

const DB::BlockPosList &DBLevelDB::getBlockPosList() {
	m_blockPosList.clear();
	leveldb::Iterator* it = m_db->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		m_blockPosList.push_back(BlockPos(it->key().ToString()));
	}
	delete it;
	return m_blockPosList;
}

const DB::Block DBLevelDB::getBlockOnPos(const BlockPos &pos)
{
	std::string datastr;
	leveldb::Status status;


	// If block format is not known, try both alternatives, but prefer
	// the version that has worked best in recent history
	if (pos.databaseFormat() == BlockPos::Unknown) {
		BlockPos::StrFormat format[2];
		if (m_keyFormatI64Usage > m_keyFormatAXYZUsage) {
			format[0] = BlockPos::I64;
			format[1] = BlockPos::AXYZ;
		}
		else {
			format[0] = BlockPos::AXYZ;
			format[1] = BlockPos::I64;
		}
		int i;
		for (i = 0; i < 2; i++) {
			m_blocksQueriedCount++;
			status = m_db->Get(leveldb::ReadOptions(), pos.databasePosStrFmt(format[i]), &datastr);
			if (status.ok())
				break;
		}
		if (i < 2) {
			if (format[i] == BlockPos::I64) {
				m_keyFormatI64Usage = (m_keyFormatI64Usage << 1) | 0x01;
				m_keyFormatAXYZUsage = (m_keyFormatAXYZUsage >> 1);
			}
			else {
				m_keyFormatAXYZUsage = (m_keyFormatAXYZUsage << 1) | 0x01;
				m_keyFormatI64Usage = (m_keyFormatI64Usage >> 1);
			}
		}
	}
	else {
		m_blocksQueriedCount++;
		status = m_db->Get(leveldb::ReadOptions(), pos.databasePosStr(), &datastr);
	}

	if(status.ok()) {
		m_blocksReadCount++;
		return Block(pos, reinterpret_cast<const unsigned char *>(datastr.c_str()), datastr.size());
	}
	else {
		return Block(pos, {});
	}

}

#endif // USE_LEVELDB
