/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  25.08.2012 10:55:29
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#pragma once

#include "Color.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

class PixelAttribute {
public:
	enum AlphaMixingMode {
		AlphaMixDarkenBit = 0x01,
		AlphaMixCumulative = 0x02,
		AlphaMixCumulativeDarken = 0x03,
		AlphaMixAverage = 0x04,
	};
	static void setMixMode(AlphaMixingMode mode);
	PixelAttribute() = default;
//	PixelAttribute(const PixelAttribute &p);
	PixelAttribute(const Color &color, double height);
	PixelAttribute(const ColorEntry &entry, double height);
	bool nextEmpty{true};
	double h() const { return m_h / (m_n ? m_n : 1); }
	double t() const { return m_t / (m_n ? m_n : 1); }
	double a() const { return m_a / (m_n ? m_n : 1); }
	double r() const { return m_r / (m_n ? m_a : 1); }
	double g() const { return m_g / (m_n ? m_a : 1); }
	double b() const { return m_b / (m_n ? m_a : 1); }
	uint8_t red() const { return int(r() * 255 + 0.5); }
	uint8_t green() const { return int(g() * 255 + 0.5); }
	uint8_t blue() const { return int(b() * 255 + 0.5); }
	uint8_t alpha() const { return int(a() * 255 + 0.5); }
	uint8_t thicken() const { return int(t() * 255 + 0.5); }
	unsigned height() const { return unsigned(h() + 0.5); }
	bool isNormalized() const { return !m_n; }
	Color color() const { return Color(red(), green(), blue(), alpha()); }

	inline bool is_valid() const { return !std::isnan(m_h); }
	PixelAttribute &operator=(const PixelAttribute &p);
	void normalize(double count = 0, Color defaultColor = Color(127, 127, 127));
	void add(const PixelAttribute &p);
	void mixUnder(const PixelAttribute &p);
private:
	static AlphaMixingMode m_mixMode;
	double m_n{0};
	double m_h{ std::numeric_limits<double>::quiet_NaN() };
	double m_t{0};
	double m_a{0};
	double m_r{0};
	double m_g{0};
	double m_b{0};

friend class PixelAttributes;
};

class PixelAttributes
{
public:
	PixelAttributes() = default;
	virtual ~PixelAttributes();
	void setParameters(int width, int lines, int nextY, int scale, bool defaultEmpty);
	void scroll(int keepY);
	PixelAttribute &attribute(int y, int x);
	void renderShading(double emphasis, bool drawAlpha);
	int getNextY() { return m_nextY; }
	void setLastY(int y);
	int getLastY() { return m_lastY; }

private:
	int yCoord2Line(int y) { return y - m_firstY + m_firstLine; }
	void freeAttributes();

private:
	int m_previousLine{};
	const int m_firstLine{1};
	int m_lastLine{};
	int m_emptyLine{};
	int m_lineCount{};
	PixelAttribute **m_pixelAttributes{nullptr};
	int m_width{};
	int m_firstY{};
	int m_nextY{};
	int m_lastY{};
	int m_firstUnshadedY{};
	int m_scale{};
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

