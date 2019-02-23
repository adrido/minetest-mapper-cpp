#pragma once

#include "CharEncodingConverter.h"
#include "build_config.h"


#ifdef USE_ICONV
#include <iconv.h>

class CharEncodingConverterIConv : public CharEncodingConverter
{
public:
	CharEncodingConverterIConv(const std::string &to, const std::string &from = "");
	virtual ~CharEncodingConverterIConv();

	static std::string getCurrentCharEncoding();
	std::string convert(const std::string &src) override;

private:
	iconv_t m_iconv;
};
#endif
