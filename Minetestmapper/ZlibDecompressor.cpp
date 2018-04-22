/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  18.09.2012 10:20:47
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#include <zlib.h>
#include <cstdint>
#include "ZlibDecompressor.h"

ZlibDecompressor::ZlibDecompressor(const unsigned char *data, std::size_t size):
	m_data(data),
	m_seekPos(0),
	m_size(size)
{
}

ZlibDecompressor::~ZlibDecompressor()
{
}

void ZlibDecompressor::setSeekPos(std::size_t seekPos)
{
	m_seekPos = seekPos;
}

std::size_t ZlibDecompressor::seekPos() const
{
	return m_seekPos;
}

std::vector<unsigned char> ZlibDecompressor::decompress()
{
	const unsigned char *data = m_data + m_seekPos;
	const std::size_t size = m_size - m_seekPos;

	std::vector<unsigned char> buffer;
	const size_t BUFSIZE = 256 * 1024;
	unsigned char temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = Z_NULL;
	strm.avail_in = static_cast<uInt>(size);

	if (inflateInit(&strm) != Z_OK) {
		throw DecompressError(strm.msg);
	}

	strm.next_in = const_cast<unsigned char *>(data);
	int ret = 0;
	do {
		strm.avail_out = BUFSIZE;
		strm.next_out = &temp_buffer[0];
		ret = inflate(&strm, Z_NO_FLUSH);
		buffer.insert(buffer.end(), &temp_buffer[0], &temp_buffer[BUFSIZE - strm.avail_out]);
	} while (ret == Z_OK);
	if (ret != Z_STREAM_END) {
		throw DecompressError(strm.msg);
	}
	m_seekPos += strm.next_in - data;
	(void)inflateEnd(&strm);

	return buffer;
}

void ZlibDecompressor::decompressVoid()
{
	const unsigned char *data = m_data + m_seekPos;
	const std::size_t size = m_size - m_seekPos;

	//std::vector<unsigned char> buffer;
	const size_t BUFSIZE = 256 * 1024;
	unsigned char temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = Z_NULL;
	strm.avail_in = static_cast<uInt>(size);

	if (inflateInit(&strm) != Z_OK) {
		throw DecompressError(strm.msg);
	}

	strm.next_in = const_cast<unsigned char *>(data);
	int ret = 0;
	do {
		strm.avail_out = BUFSIZE;
		strm.next_out = &temp_buffer[0];
		ret = inflate(&strm, Z_NO_FLUSH);
		//buffer.insert(buffer.end(), &temp_buffer[0], &temp_buffer[BUFSIZE - strm.avail_out]);
	} while (ret == Z_OK);
	if (ret != Z_STREAM_END) {
		throw DecompressError(strm.msg);
	}
	m_seekPos += strm.next_in - data;
	(void)inflateEnd(&strm);

	//return buffer;
}

std::array<unsigned char, ZlibDecompressor::nodesBlockSize> ZlibDecompressor::decompressNodes()
{
	const unsigned char *data = m_data + m_seekPos;
	const uLong size = static_cast<uLong>(m_size - m_seekPos);

	std::array<unsigned char, nodesBlockSize> nodes;
	uLong nodeSize = static_cast<uLong>(nodes.size());

	int status = uncompress(nodes.data(), &nodeSize, data, size);
	switch (status)
	{
	case Z_MEM_ERROR:
		throw DecompressError("not enought Memory");
		break;
	case Z_BUF_ERROR:
		throw DecompressError("Mapblock too big, buffer too small");
		break;
	case Z_DATA_ERROR:
		throw DecompressError("Data Error: Mapblock corrupted");
		break;
	case Z_OK:
	default:
		break;
	}



	return nodes;
}

