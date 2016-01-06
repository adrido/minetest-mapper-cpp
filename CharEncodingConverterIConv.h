
#ifndef _CHARENCODINGCONVERTERICONV_H_INCLUDED_
#define _CHARENCODINGCONVERTERICONV_H_INCLUDED_

#include <iconv.h>
#include "CharEncodingConverter.h"

class CharEncodingConverterIConv : public CharEncodingConverter
{
public:
	CharEncodingConverterIConv(std::string to, std::string from = "");
	virtual ~CharEncodingConverterIConv(void);

	static std::string getCurrentCharEncoding(void);
	std::string convert(const std::string &src) override;
private:
	iconv_t m_iconv;
};

#endif // _CHARENCODINGCONVERTERICONV_H_INCLUDED_

