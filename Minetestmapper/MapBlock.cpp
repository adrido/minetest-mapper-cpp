#include "MapBlock.h"

#include "TileGenerator.h"
#include "ZlibDecompressor.h"

using namespace std;

static inline void checkDataLimit(const char *type, size_t offset, size_t length, size_t dataLength)
{
	if (offset + length > dataLength)
		throw TileGenerator::UnpackError(type, offset, length, dataLength);
}

static inline uint8_t readU8(const unsigned char *data, size_t offset, size_t dataLength)
{
	checkDataLimit("uint8", offset, 1, dataLength);
	return data[offset];
}

static inline uint16_t readU16(const unsigned char *data, size_t offset, size_t dataLength)
{
	checkDataLimit("uint16", offset, 2, dataLength);
	return data[offset] << 8 | data[offset + 1];
}

static inline string readString(const unsigned char *data, size_t offset, size_t length, size_t dataLength)
{
	checkDataLimit("string", offset, length, dataLength);
	return string(reinterpret_cast<const char *>(data) + offset, length);
}

void MapBlock::setData(const std::vector<unsigned char>& data)
{
	empty = data.empty();

	if (!empty) {
		deserialize(data);
	}
}

void MapBlock::setData(const unsigned char *data, size_t size)
{
	empty = size == 0;

	if (!empty) {
		deserialize(data, size);
	}
}

int MapBlock::readBlockContent(int datapos) const
{
	//auto mapData = this->mapData.data(); // no noticeable speed improovement
	if (version >= 24) {
		size_t index = static_cast<size_t>(datapos) << 1;
		return (mapData[index] << 8) | mapData[index + 1];
	}
	else if (version >= 20) {
		if (mapData[datapos] <= 0x80) {
			return mapData[datapos];
		}
		else {
			return (int(mapData[datapos]) << 4) | (int(mapData[datapos + 0x2000]) >> 4);
		}
	}
	else {
		std::ostringstream oss;
		oss << "Unsupported map version " << version;
		throw std::runtime_error(oss.str());
	}
}

inline void MapBlock::deserialize(const unsigned char *data, size_t length)
{
	version = readU8(data, 0, length);
	//uint8_t flags = readU8(data, 1, length);

	size_t dataOffset = 0;
	if (version >= 27) {
		dataOffset = 6;
	}
	else if (version >= 22) {
		dataOffset = 4;
	}
	else {
		dataOffset = 2;
	}

	// Zlib header: 2; Deflate header: >=1
	checkDataLimit("zlib", dataOffset, 3, length);
	ZlibDecompressor decompressor(data, length);
	decompressor.setSeekPos(dataOffset);
	mapData = decompressor.decompressNodes();
	checkBlockNodeDataLimit();
	decompressor.decompressVoid();
	//auto mapMetadata = decompressor.decompress();
	dataOffset = decompressor.seekPos();

	// Skip unused data
	if (version <= 21) {
		dataOffset += 2;
	}
	if (version == 23) {
		dataOffset += 1;
	}
	if (version == 24) {
		uint8_t ver = readU8(data, dataOffset++, length);
		if (ver == 1) {
			uint16_t num = readU16(data, dataOffset, length);
			dataOffset += 2;
			dataOffset += 10 * num;
		}
	}

	// Skip unused static objects
	dataOffset++; // Skip static object version
	int staticObjectCount = readU16(data, dataOffset, length);
	dataOffset += 2;
	for (int i = 0; i < staticObjectCount; ++i) {
		dataOffset += 13;
		uint16_t dataSize = readU16(data, dataOffset, length);
		dataOffset += dataSize + 2;
	}
	dataOffset += 4; // Skip timestamp

					 // Read mapping
	if (version >= 22) {
		dataOffset++; // mapping version
		uint16_t numMappings = readU16(data, dataOffset, length);
		dataOffset += 2;
		for (int i = 0; i < numMappings; ++i) {
			int nodeId = readU16(data, dataOffset, length);
			dataOffset += 2;
			uint16_t nameLen = readU16(data, dataOffset, length);
			dataOffset += 2;
			string name = readString(data, dataOffset, nameLen, length);
			//size_t end = name.find_first_of('\0');
			//if (end != std::string::npos)
			//	name.erase(end);
			nodeId2NodeName.emplace(nodeId, name);
			dataOffset += nameLen;
		}
	}

	// Node timers
	if (version >= 25) {
		dataOffset++;
		uint16_t numTimers = readU16(data, dataOffset, length);
		dataOffset += 2;
		dataOffset += numTimers * 10;
	}
}

inline void MapBlock::deserialize(const std::vector<unsigned char>& vdata)
{
	deserialize(vdata.data(), vdata.size());
}

void MapBlock::checkBlockNodeDataLimit()
{
	size_t dataLength = mapData.size();
	constexpr const int datapos = 16 * 16 * 16;
	if (version >= 24) {
		constexpr const size_t index = datapos << 1;
		checkDataLimit("node:24", index, 2, dataLength);
	}
	else if (version >= 20) {
		checkDataLimit("node:20", datapos + 0x2000, 1, dataLength);
	}
	else {
		std::ostringstream oss;
		oss << "Unsupported map version " << version;
		throw std::runtime_error(oss.str());
	}
}
