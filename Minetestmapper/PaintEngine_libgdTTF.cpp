#include "PaintEngine_libgdTTF.h"
#include <iostream>

using namespace std;

PaintEngine_libgdTTF::PaintEngine_libgdTTF()
{
	//gdFTUseFontConfig(1);
}


PaintEngine_libgdTTF::~PaintEngine_libgdTTF()
= default;

void PaintEngine_libgdTTF::drawText(int x, int y, Font font, const std::string & text, const Color & color)
{
	int brect[8];
	string f = getFont(font);
	double fontsize = getFontSize(font);
	char *ret = gdImageStringTTF(PaintEngine_libgd::image, &brect[0], 
		                         color.to_libgd(), const_cast<char *>(f.c_str()), 
		                         fontsize, 0.0,
		                         x, static_cast<int>(y + fontsize + 2), 
		                         const_cast<char*>(text.c_str()));
	if (ret) {
		std::cerr << ret << std::endl;
	}
}

double PaintEngine_libgdTTF::getFontSize(PaintEngine::Font font) const
{
	double size = 0.0;
	switch (font)
	{
	case PaintEngine::Font::Tiny:
		size = 8.0;
		break;
	case PaintEngine::Font::Small:
		size = 13.0;
		break;
	case PaintEngine::Font::MediumBold:
		size = 13.0;
		break;
	case PaintEngine::Font::large:
		size = 16.0;
		break;
	case PaintEngine::Font::Giant:
		size = 15.0;
		break;
	default:
		size = 8.0;
		break;
	}

	// 1 px = 0.75 point
	return size * 0.75;
}

std::string PaintEngine_libgdTTF::getFont(PaintEngine::Font font) const
{

	string s = "arial";
	switch (font)
	{
	case PaintEngine::Font::MediumBold:
	case PaintEngine::Font::Giant:
		s.append("bd");
		break;
	case PaintEngine::Font::Tiny:
	case PaintEngine::Font::Small:
	case PaintEngine::Font::large:
	default:
		break;
	}
	return s;
}
