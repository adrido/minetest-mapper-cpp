#ifndef _DB_H
#define _DB_H

#include <stdint.h>
#include <vector>
#include <string>
#include <utility>

#include "types.h"
#include "BlockPos.h"


class DB {
public:
	typedef std::pair<BlockPos, ustring> Block;
	typedef std::vector<BlockPos>  BlockPosList;
	virtual const BlockPosList &getBlockPos()=0;
	virtual int getBlocksQueriedCount(void)=0;
	virtual int getBlocksReadCount(void)=0;
	virtual Block getBlockOnPos(const BlockPos &pos)=0;
};

#endif // _DB_H
