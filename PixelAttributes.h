/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  25.08.2012 10:55:29
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#ifndef PIXELATTRIBUTES_H_ADZ35GYF
#define PIXELATTRIBUTES_H_ADZ35GYF

#include <limits>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <cassert>
#include "config.h"
#include "Color.h"

class PixelAttribute {
public:
	enum AlphaMixingMode {
		AlphaMixDarkenBit = 0x01,
		AlphaMixCumulative = 0x02,
		AlphaMixCumulativeDarken = 0x03,
		AlphaMixAverage = 0x04,
	};
	static void setMixMode(AlphaMixingMode mode);
	PixelAttribute(): nextEmpty(true), m_n(0), m_h(NAN), m_t(0), m_a(0), m_r(0), m_g(0), m_b(0) {};
//	PixelAttribute(const PixelAttribute &p);
	PixelAttribute(const Color &color, double height);
	PixelAttribute(const ColorEntry &entry, double height);
	bool nextEmpty;
	double h(void) const { return m_h / (m_n ? m_n : 1); }
	double t(void) const { return m_t / (m_n ? m_n : 1); }
	double a(void) const { return m_a / (m_n ? m_n : 1); }
	double r(void) const { return m_r / (m_n ? m_a : 1); }
	double g(void) const { return m_g / (m_n ? m_a : 1); }
	double b(void) const { return m_b / (m_n ? m_a : 1); }
	uint8_t red(void) const { return int(r() * 255 + 0.5); }
	uint8_t green(void) const { return int(g() * 255 + 0.5); }
	uint8_t blue(void) const { return int(b() * 255 + 0.5); }
	uint8_t alpha(void) const { return int(a() * 255 + 0.5); }
	uint8_t thicken(void) const { return int(t() * 255 + 0.5); }
	unsigned height(void) const { return unsigned(h() + 0.5); }
	bool isNormalized(void) const { return !m_n; }
	Color color(void) const { return Color(red(), green(), blue(), alpha()); }

	inline bool is_valid() const { return !std::isnan(m_h); }
	PixelAttribute &operator=(const PixelAttribute &p);
	void normalize(double count = 0, Color defaultColor = Color(127, 127, 127));
	void add(const PixelAttribute &p);
	void mixUnder(const PixelAttribute &p);
private:
	static AlphaMixingMode m_mixMode;
	double m_n;
	double m_h;
	double m_t;
	double m_a;
	double m_r;
	double m_g;
	double m_b;

friend class PixelAttributes;
};

class PixelAttributes
{
public:
	PixelAttributes();
	virtual ~PixelAttributes();
	void setParameters(int width, int lines, int nextY, int scale, bool defaultEmpty);
	void scroll(int keepY);
	PixelAttribute &attribute(int y, int x);
	void renderShading(double emphasis, bool drawAlpha);
	int getNextY(void) { return m_nextY; }
	void setLastY(int y);
	int getLastY(void) { return m_lastY; }

private:
	int yCoord2Line(int y) { return y - m_firstY + m_firstLine; }
	void freeAttributes();

private:
	int m_previousLine;
	int m_firstLine;
	int m_lastLine;
	int m_emptyLine;
	int m_lineCount;
	PixelAttribute **m_pixelAttributes;
	int m_width;
	int m_firstY;
	int m_nextY;
	int m_lastY;
	int m_firstUnshadedY;
	int m_scale;
};

inline void PixelAttributes::setLastY(int y)
{
#ifdef DEBUG
	assert(y - m_firstY <= m_lastLine - m_firstLine);
#else
	if (y - m_firstY > m_lastLine - m_firstLine)
		// Not sure whether this will actually avoid a crash...
		y = m_firstY + (m_lastLine - m_firstLine);
#endif
	m_lastY = y;
}

inline void PixelAttribute::setMixMode(AlphaMixingMode mode)
{
	if (mode == AlphaMixDarkenBit)
		mode = AlphaMixCumulativeDarken;
	m_mixMode = mode;
}

inline PixelAttribute &PixelAttributes::attribute(int y, int x)
{
#ifdef DEBUG
	assert(yCoord2Line(y) >= m_firstLine && yCoord2Line(y) <= m_lastLine);
#else
	static PixelAttribute p;
	if (!(yCoord2Line(y) >= m_firstLine && yCoord2Line(y) <= m_lastLine))
		return p;
#endif
	return m_pixelAttributes[yCoord2Line(y)][x + 1];
}

//inline PixelAttribute::PixelAttribute(const PixelAttribute &p) :
//{
//	operator=(p);
//}

inline PixelAttribute::PixelAttribute(const Color &color, double height) :
	nextEmpty(false), m_n(0), m_h(height), m_t(0), m_a(color.a/255.0),
	m_r(color.r/255.0), m_g(color.g/255.0), m_b(color.b/255.0)
{
}

inline PixelAttribute::PixelAttribute(const ColorEntry &entry, double height) :
	nextEmpty(false), m_n(0), m_h(height), m_t(entry.t/255.0), m_a(entry.a/255.0),
	m_r(entry.r/255.0), m_g(entry.g/255.0), m_b(entry.b/255.0)
{
}

inline PixelAttribute &PixelAttribute::operator=(const PixelAttribute &p)
{
	m_n = p.m_n;
	m_h = p.m_h;
	m_t = p.m_t;
	m_a = p.m_a;
	m_r = p.m_r;
	m_g = p.m_g;
	m_b = p.m_b;
	return *this;
}

#endif /* end of include guard: PIXELATTRIBUTES_H_ADZ35GYF */

