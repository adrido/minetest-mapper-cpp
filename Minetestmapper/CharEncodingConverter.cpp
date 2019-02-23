#include "CharEncodingConverter.h"

#include "build_config.h"

#include <iostream>
#ifdef USE_ICONV
#include "CharEncodingConverterIConv.h"
#endif

CharEncodingConverter *CharEncodingConverter::createStandardConverter(const std::string &to, const std::string &from)
{
#ifdef USE_ICONV
	return new CharEncodingConverterIConv(to, from);
#else
	return new CharEncodingConverterDummy(to, from);
#endif
}

std::string CharEncodingConverter::getCurrentCharEncoding(void)
{
#ifdef USE_ICONV
	return CharEncodingConverterIConv::getCurrentCharEncoding();
#else
	return CharEncodingConverterDummy::getCurrentCharEncoding();
#endif
}
