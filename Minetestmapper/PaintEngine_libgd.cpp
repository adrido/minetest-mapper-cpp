#include "PaintEngine_libgd.h"

#include <sstream>
#include <cstring>
#include <stdexcept>

#include <gdfontmb.h>
#include <gdfontl.h>
#include <gdfontg.h>
#include <gdfonts.h>
#include <gdfontt.h>

#include "porting.h"


PaintEngine_libgd::PaintEngine_libgd()
{
	// Libgd requires ISO8859-2 :-(
	// Internally, we use UTF-8. Assume minetest does the same... (if not, it's probably broken)
	gdStringConv = CharEncodingConverter::createStandardConverter("ISO8859-2", "UTF-8");
}


PaintEngine_libgd::~PaintEngine_libgd()
{
	clean();
}

bool PaintEngine_libgd::create(int w, int h)
{
	width = w;
	height = h;
	image = gdImageCreateTrueColor(w, h);
	return image != nullptr;
}

void PaintEngine_libgd::fill(const Color &color)
{
	gdImageFilledRectangle(image, 0, 0, width - 1, height - 1, color.to_libgd());
}

void PaintEngine_libgd::drawText(int x, int y, Font font, const std::string &text, const Color &color)
{
	const std::string str = gdStringConv->convert(text);
	gdImageString(image, getGdFont(font), x, y, 
		reinterpret_cast<unsigned char *>(const_cast<char *>(str.c_str())), color.to_libgd());
}

void PaintEngine_libgd::drawChar(int x, int y, Font font, char ch, const Color &color)
{

	gdImageChar(image, getGdFont(font), x, y, ch, color.to_libgd());
}

void PaintEngine_libgd::drawRect(int x1, int y1, int x2, int y2, const Color &color)
{
	gdImageRectangle(image, x1, y1, x2, y2, color.to_libgd());
}

void PaintEngine_libgd::drawLine(int x1, int y1, int x2, int y2, const Color &color)
{
	gdImageLine(image, x1, y1, x2, y2, color.to_libgd());
}

void PaintEngine_libgd::drawCircle(int x, int y, int r, const Color &color)
{
	gdImageArc(image, x, y, r, r, 0, 360, color.to_libgd());
}

void PaintEngine_libgd::drawArc(int x, int y, int w, int h, int start, int end, const Color &color)
{
	gdImageArc(image, x, y, w, h, start, end, color.to_libgd());
}

void PaintEngine_libgd::drawFilledRect(int x1, int y1, int x2, int y2, const Color &color)
{
	gdImageFilledRectangle(image, x1, y1, x2, y2, color.to_libgd());
}

void PaintEngine_libgd::drawPixel(int x, int y, const Color &color)
{
	image->tpixels[y][x] = color.to_libgd();
}

bool PaintEngine_libgd::save(const std::string & filename, const std::string & format, int quality)
{
	FILE *out;
	out = porting::fopen(filename.c_str(), "wb");
	if (!out) {
		std::ostringstream oss;
		oss << "Error opening '" << filename << "': " << porting::strerror(errno);
		throw std::runtime_error(oss.str());
	}
	gdImagePng(image, out);
	fclose(out);
	gdImageDestroy(image);
	return true;
}

void PaintEngine_libgd::clean()
{
	gdImageDestroy(image);
}

gdFontPtr PaintEngine_libgd::getGdFont(Font font) const
{
	gdFontPtr f = nullptr;
	switch (font)
	{
	case PaintEngine::Font::Tiny:
		f = gdFontGetTiny();
		break;
	case PaintEngine::Font::Small:
		f = gdFontGetSmall();
		break;
	case PaintEngine::Font::MediumBold:
		f = gdFontGetMediumBold();
		break;
	case PaintEngine::Font::large:
		f = gdFontGetLarge();
		break;
	case PaintEngine::Font::Giant:
		f = gdFontGetGiant();
		break;
	default:
		f = gdFontGetSmall();
		break;
	}
	return f;
}
