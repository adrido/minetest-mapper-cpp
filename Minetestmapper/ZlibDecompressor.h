/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  18.09.2012 10:20:51
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#pragma once

#include <array>
#include <cstdlib>
#include <utility>
#include <string>
#include <vector>


class ZlibDecompressor
{
public:
	struct DecompressError {
		DecompressError(const std::string &m = "(unknown error)") : message(m) {}
		const std::string message;
	};

	ZlibDecompressor(const unsigned char *data, std::size_t size);
	~ZlibDecompressor();
	void setSeekPos(std::size_t seekPos);
	std::size_t seekPos() const;
	std::vector<unsigned char> decompress();

	void decompressVoid();

	static constexpr const size_t nodesBlockSize = 16 * 16 * 16 * 4;
	std::array<unsigned char, nodesBlockSize> decompressNodes();

private:
	const unsigned char *m_data;
	std::size_t m_seekPos;
	std::size_t m_size;
}; /* -----  end of class ZlibDecompressor  ----- */


