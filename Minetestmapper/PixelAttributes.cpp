/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  25.08.2012 10:55:27
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#include "PixelAttributes.h"
#include <cstring> // memcpy

using namespace std;

PixelAttribute::AlphaMixingMode PixelAttribute::m_mixMode = PixelAttribute::AlphaMixCumulative;


void PixelAttributes::setParameters(int width, int lines, int nextY, int scale, bool defaultEmpty)
{
	freeAttributes();
	m_width = width + 1; // 1px gradient calculation
	m_previousLine = 0;
	//m_firstLine = 1;
	m_lastLine = m_firstLine + lines - 1;
	m_emptyLine = m_lastLine + 1;
	m_lineCount = m_emptyLine + 1;
	m_firstY = 0;
	m_nextY = nextY;
	m_lastY = -1;
	m_firstUnshadedY = 0;
	m_scale = scale;

	m_pixelAttributes = vector<vector<PixelAttribute>>(m_lineCount, vector<PixelAttribute>(m_width, PixelAttribute()));

	if (defaultEmpty)
		for (int i = 0; i < m_lineCount; i++)
			for (int j = 0; j < m_width; j++) {
				m_pixelAttributes[i][j].nextEmpty = (j - 1) % (16 / scale) == 0;
			}
}

void PixelAttributes::scroll(int keepY)
{
	int scroll = keepY - m_firstY;
	if (scroll > 0) {
		int i;
		for (i = m_previousLine; i + scroll <= m_lastLine; i++) {
			m_pixelAttributes[i].swap(m_pixelAttributes[i + scroll]);
		}

		size_t lineLength = m_width * sizeof(PixelAttribute);
		for (; i <= m_lastLine; ++i) {
			// TODO: get rid of memcpy. This seems non-trivial.
			memcpy(m_pixelAttributes[i].data(), m_pixelAttributes[m_emptyLine].data(), lineLength);
		}

		m_firstY += scroll;
		m_nextY = m_firstY;
		m_firstUnshadedY -= scroll;
		if (m_firstUnshadedY < m_firstY)
			m_firstUnshadedY = m_firstY;
	}
}

void PixelAttributes::freeAttributes()
{
	m_pixelAttributes.clear();
}


static inline double colorSafeBounds(double color)
{
	if (color > 1) {
		return 1;
	}
	else if (color < 0) {
		return 0;
	}
	else {
		return color;
	}
}


void PixelAttributes::renderShading(double emphasis, bool drawAlpha)
{
	int y;
	for (y = yCoord2Line(m_firstUnshadedY); y <= yCoord2Line(m_lastY); y++) {
		for (int x = 1; x < m_width; x++) {
			if (m_pixelAttributes[y][x].nextEmpty) {
				x += 16 / m_scale - 1;
				continue;
			}
			if (!m_pixelAttributes[y][x].isNormalized())
				m_pixelAttributes[y][x].normalize();
			if (!m_pixelAttributes[y][x].is_valid()) {
				if (x + 1 < m_width && !m_pixelAttributes[y][x + 1].isNormalized())
					m_pixelAttributes[y][x + 1].normalize();
				x++;
				continue;
			}
			if (!m_pixelAttributes[y - 1][x].is_valid() || !m_pixelAttributes[y][x - 1].is_valid())
				continue;
			if (!m_pixelAttributes[y][x].m_a)
				continue;
			double h = m_pixelAttributes[y][x].m_h;
			double h1 = m_pixelAttributes[y][x - 1].m_a ? m_pixelAttributes[y][x - 1].m_h : h;
			double h2 = m_pixelAttributes[y - 1][x].m_a ? m_pixelAttributes[y - 1][x].m_h : h;
			double d = (h - h1) + (h - h2);
			if (d > 3) {
				d = 3;
			}
			d = d * 12 / 255 * emphasis;
#define pixel (m_pixelAttributes[y][x])
			//PixelAttribute &pixel = m_pixelAttributes[y][x];
			if (drawAlpha)
				d = d * (1 - pixel.m_t);
			pixel.m_r = colorSafeBounds(pixel.m_r + d);
			pixel.m_g = colorSafeBounds(pixel.m_g + d);
			pixel.m_b = colorSafeBounds(pixel.m_b + d);
#undef pixel
		}
	}
	m_firstUnshadedY = y - yCoord2Line(0);
}

// Meaning and usage of parameter 'n'.
//
// When n==0, all other values should be interpreted as
// plain color / height / etc.
//
// When n is positive, some kind of pixel average is being
// computed, and the other values represent a sum, with
// n being the number of items summed.
//
// There is a twist when summing colors: Transparent colors
// should not contribute the same amount to the final average
// color as opaque colors do.
// For that reason, the color values (r, g, b) are not simply
// summed, but they are multiplied by their alpha values
// first.
//
// Conversion from pixelattributes with n>0 to pixelattributes
// with n==0 is performed as follows:
//	<color-value> = <sum-of-weighed-color-values> / <sum-of-alpha-values>
//	<alpha-value> = <sum-of-alpha-values> / n
//	<height> = <sum-of-heights> / n
//	<thick> = <sum-of-thick-values> / n
//	n = 0
// The converse is (n would normally be set to 1):
//	<weighed-color-value> = <color-value> * <alpha-value>
//	n = 1
// Color values with n>0 can be summed.

// normalize() converts from n>0 to n==0 representation
void PixelAttribute::normalize(double count, Color defaultColor)
{
	if (!m_n) {
		// Already normalized
		return;
	}
	if (m_n < count) {
		m_r += (defaultColor.r / 255.0) * (defaultColor.a / 255.0) * (count - m_n);
		m_g += (defaultColor.g / 255.0) * (defaultColor.a / 255.0) * (count - m_n);
		m_b += (defaultColor.b / 255.0) * (defaultColor.a / 255.0) * (count - m_n);
		m_a += (defaultColor.a / 255.0) * (count - m_n);
		m_h *= double(count) / m_n;
		m_t *= double(count) / m_n;
		m_n = count;
	}
	if (m_n != 1) {
		m_r /= m_a;
		m_g /= m_a;
		m_b /= m_a;
		m_a /= m_n;
		m_t /= m_n;
		m_h /= m_n;
	}
	m_n = 0;
}

void PixelAttribute::add(const PixelAttribute &p)
{
	if (!m_n) {
		m_r *= m_a;
		m_g *= m_a;
		m_b *= m_a;
		m_n = 1;
	}
	if (!is_valid()) {
		m_r = p.m_r;
		m_g = p.m_g;
		m_b = p.m_b;
		m_a = p.m_a;
		m_t = 0;
		m_h = p.m_h;
		m_n = p.m_n;
	}
	else if (!p.m_n) {
		m_r += p.m_r * p.m_a;
		m_g += p.m_g * p.m_a;
		m_b += p.m_b * p.m_a;
		m_a += p.m_a;
		m_t += p.m_t;
		m_h += p.m_h;
		m_n++;
	}
	else {
		m_r += p.m_r;
		m_g += p.m_g;
		m_b += p.m_b;
		m_a += p.m_a;
		m_t += p.m_t;
		m_h += p.m_h;
		m_n += p.m_n;
	}
}

void PixelAttribute::mixUnder(const PixelAttribute &p)
{
	if (!is_valid() || m_a == 0) {
		if (!is_valid() || p.m_a != 0) {
			m_n = p.m_n;
			m_r = p.m_r;
			m_g = p.m_g;
			m_b = p.m_b;
			m_a = p.m_a;
			m_t = 0;
		}
		m_h = p.m_h;
	}
	else if (m_a == 1)
		; // Nothing to do: pixel is already fully opaque.
	else if ((m_mixMode & AlphaMixCumulative) == AlphaMixCumulative || (m_mixMode == AlphaMixAverage && p.m_a == 1)) {
		PixelAttribute pp(p);
#ifdef DEBUG
		assert(pp.isNormalized());
#else
		if (!pp.isNormalized())
			pp.normalize();
#endif
		if (!isNormalized())
			normalize();
		int prev_alpha = alpha();
		m_r = (m_a * m_r + pp.m_a * (1 - m_a) * pp.m_r);
		m_g = (m_a * m_g + pp.m_a * (1 - m_a) * pp.m_g);
		m_b = (m_a * m_b + pp.m_a * (1 - m_a) * pp.m_b);
		m_a = (m_a + (1 - m_a) * pp.m_a);
		if (pp.m_a != 1)
			m_t = (m_t + pp.m_t) / 2;
		else
			m_h = pp.m_h;
		if ((m_mixMode & AlphaMixDarkenBit) && prev_alpha >= 254 && pp.alpha() < 255) {
			// Darken
			// Parameters make deep water look good :-)
			// (maybe this setting should be per-node-type, and obtained from the colors file ?)
			m_r = m_r * 0.95;
			m_g = m_g * 0.95;
			m_b = m_b * 0.95;
		}
	}
#ifdef DEBUG
	else if (m_mixMode == AlphaMixAverage && p.m_a != 1) {
#else
	else {
#endif
		if (p.m_a == 1)
			normalize();
		double h = p.m_h;
		double t = m_t;
		add(p);
		if (p.m_a == 1) {
			normalize();
			m_t = t;
			m_a = 1;
			m_h = m_n * h;
		}
	}
#ifdef DEBUG
	else {
		// Internal error
		assert(1 && m_mixMode);
	}
#endif
}
