#pragma once

#include "BlockPos.h"
#include "ZlibDecompressor.h"

#include <string>
#include <unordered_map>
#include <vector>

class MapBlock
{
public:
	MapBlock() = default;
	~MapBlock() = default;

	MapBlock(const BlockPos &pos, const std::vector<unsigned char> &data) {
		setPos(pos);
		setData(data);
	}
	MapBlock(const BlockPos &pos, const unsigned char *data, std::size_t size) {
		setPos(pos);
		setData(data,size);
	}
	MapBlock(const BlockPos &pos, const void *data, std::size_t size) {
		setPos(pos);
		setData(data, size);
	}

	void reset() {
		nodeId2NodeName.clear();
		mapData.clear();
		empty = true;
		version = 0;
	}
	const std::unordered_map<int, std::string> &getMappings() const { return nodeId2NodeName; }
	const BlockPos &getPos() const { return pos; }
	const std::vector<unsigned char> &getMapData() const { return mapData; }
	int getVersion() const { return version; }
	bool isEmpty() const { return empty; }

	void setData(const std::vector<unsigned char> &data);
	void setData(const unsigned char * data, size_t size);
	inline void setData(const void * data, size_t size)
	{
		setData(static_cast<const unsigned char*>(data), size);
	}
	void setPos(const BlockPos &p) { pos = p; }

	int readBlockContent(int datapos) const;

	bool onlyAir() const { return nodeId2NodeName.size() == 1 && nodeId2NodeName.at(0) == "air"; }

	// not shure if this will be true a single time. Why should minetest generate a mapblock and fill it with ignore only?
	bool onlyIgnore() const { return nodeId2NodeName.size() == 1 && nodeId2NodeName.at(0) == "ignore"; }

private:
	std::unordered_map<int, std::string> nodeId2NodeName;
	std::vector<unsigned char> mapData;
	BlockPos pos;
	int version = 0;
	bool empty = true;

	void deserialize(const unsigned char * data, size_t length);
	void deserialize(const std::vector<unsigned char> &vdata);

	void checkBlockNodeDataLimit();

};

