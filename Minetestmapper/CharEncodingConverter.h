
#ifndef _CHARENCODINGCONVERTER_H_INCLUDED_
#define _CHARENCODINGCONVERTER_H_INCLUDED_

#include <string>

class CharEncodingConverter
{
public:
	CharEncodingConverter(const std::string to, const std::string from)
		: m_toFormat(to), m_fromFormat(from) {
		if (m_toFormat == "") m_toFormat = getCurrentCharEncoding();
		if (m_fromFormat == "") m_fromFormat = getCurrentCharEncoding();
		}
	virtual ~CharEncodingConverter(void) {}

	// Create a converter with an unspecified but suitable backend.
	// (for usage convenience)
	static CharEncodingConverter *createStandardConverter(const std::string to, const std::string from = "");
	static std::string getCurrentCharEncoding(void);

	virtual std::string convert(const std::string &src) = 0;
	virtual std::string fromFormat(void) { return m_fromFormat; }
	virtual std::string toFormat(void) { return m_toFormat; }
protected:
	std::string m_toFormat;
	std::string m_fromFormat;
};

class CharEncodingConverterDummy : public CharEncodingConverter
{
public:
	CharEncodingConverterDummy(const std::string to, const std::string from = "")
		: CharEncodingConverter(to, from) {}
	virtual ~CharEncodingConverterDummy(void) {}

	static std::string getCurrentCharEncoding(void) { return "UTF-8"; }
	std::string convert(const std::string &src) override { return src; }
};

#endif // _CHARENCODINGCONVERTER_H_INCLUDED_

