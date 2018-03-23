#ifndef _DB_H
#define _DB_H

#include <cstdint>
#include <vector>
#include <string>
#include <utility>

#include "types.h"
#include "BlockPos.h"


class DB {
public:
	virtual ~DB() {}
	typedef std::pair<BlockPos, ustring> Block;
	typedef std::vector<BlockPos>  BlockPosList;
	virtual const BlockPosList &getBlockPosList()=0;
	virtual const BlockPosList &getBlockPosList(BlockPos, BlockPos) { return getBlockPosList(); }
	virtual int getBlocksQueriedCount(void)=0;
	virtual int getBlocksReadCount(void)=0;
	virtual Block getBlockOnPos(const BlockPos &pos)=0;
};

#endif // _DB_H
