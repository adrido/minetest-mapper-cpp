#include "CharEncodingConverterIConv.h"

#ifdef USE_ICONV
#include <cerrno>
#include <cstring>
#include <iostream>
#ifndef _WIN32
#include <langinfo.h>
#endif
#include <sstream>
#include <stdexcept>

#define ICONV_BUFSIZE 16

std::string CharEncodingConverterIConv::getCurrentCharEncoding()
{
#ifdef _WIN32
	return "UTF-8"; // TODO: Get the correct codepage on Windows and convert it to something that iconv can use
#else
	setlocale(LC_CTYPE, "");
	char *enc = nl_langinfo(CODESET);
	std::string encoding;
	if (*enc) {
		if (std::string(enc) == "ANSI_X3.4-1968")
			encoding = "US-ASCII";
		else
			encoding = enc;
	}
	else {
		std::cerr << "WARNING: could not determine current character encoding. Assuming UTF-8" << std::endl;
		encoding = "UTF-8";
	}
	// Reset locale to 'C'.
	// Advantage: unknown characters are converted to '?' instead of causing a failure...
	// Disadvantage: transliteration support may be more limited.
	// Alternative: modify conversion to replace unknown chars with '?' manually.
	setlocale(LC_CTYPE, "C");
	return encoding;
#endif
}


CharEncodingConverterIConv::CharEncodingConverterIConv(const std::string &to, const std::string &from)
	: CharEncodingConverter(to, from)
{
	std::string toFormat = m_toFormat + "//TRANSLIT";
	std::string fromFormat = m_fromFormat;
	m_iconv = iconv_open(toFormat.c_str(), fromFormat.c_str());
	if (m_iconv == (iconv_t)-1) {
		int rno = errno;
		std::ostringstream oss;
		oss << "Error initializing iconv charset converter ("
			<< (fromFormat.empty() ? "(default)" : from) << " --> "
			<< (toFormat.empty() ? "(default)" : toFormat) << "): "
			<< strerror(rno) << std::endl;

		throw std::runtime_error(oss.str());
	}
}

CharEncodingConverterIConv::~CharEncodingConverterIConv(void)
{
	if (m_iconv != (iconv_t)-1)
		iconv_close(m_iconv);
}

std::string CharEncodingConverterIConv::convert(const std::string &src)
{
	std::string dst;

	char toBuffer[ICONV_BUFSIZE + 1];
	char *fromBufP, *toBufP;
	size_t fromBufLen, toBufLen;

	fromBufLen = src.length();
	// Assume that iconv() does not write to the source array...
	fromBufP = const_cast<char *>(src.c_str());
	toBufLen = ICONV_BUFSIZE;
	toBufP = toBuffer;

	size_t rv;
	do {
		rv = iconv(m_iconv, &fromBufP, &fromBufLen, &toBufP, &toBufLen);
		if (rv == (size_t)-1) {
			int rno = errno;
			if (rno != E2BIG) {
				std::ostringstream oss;
				oss << "Failure converting character from "
					<< fromFormat() << " to " << toFormat()
					<< " (text: '[" << std::string(src.c_str(), 0, fromBufP - src.c_str()) << "]" << std::string(fromBufP) << "'): "
					<< strerror(rno);
				// Note: strerror() can be misleading, e.g. complaining about invalid input
				// when really the character cannot be represented in the output...
				// (but //TRANSLIT avoids (most of?) those kinds of errors...)
				throw std::runtime_error(oss.str());
			}
		}
		dst += std::string(toBuffer, ICONV_BUFSIZE - toBufLen);
		toBufLen = ICONV_BUFSIZE;
		toBufP = toBuffer;
	} while (rv == (size_t)-1);

	iconv(m_iconv, NULL, NULL, NULL, NULL);

	return dst;
}
#endif
