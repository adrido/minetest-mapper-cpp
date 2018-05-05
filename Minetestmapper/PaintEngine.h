#pragma once
#include <string>

#include "Color.h"

class PaintEngine
{
public:
	enum class Font{
		Tiny, // 8px
		Small, // 13px
		MediumBold, // 13px bold
		large, // 16px
		Giant // 15px bold?
	};
	virtual ~PaintEngine() = default;
	virtual bool create(int w, int h) = 0;
	virtual void fill(const Color &color) = 0;
	virtual void drawText(int x, int y, Font font, const std::string &text, const Color &color) = 0;
	virtual void drawChar(int x, int y, Font font, char ch, const Color &color) = 0;
	virtual void drawRect(int x1, int y1, int x2, int y2, const Color &color) = 0;
	virtual void drawLine(int x1, int y1, int x2, int y2, const Color &color) = 0;
	virtual void drawCircle(int x, int y, int r, const Color &color) = 0;
	virtual void drawArc(int x, int y, int w, int h, int start, int end, const Color &color) = 0;
	virtual void drawFilledRect(int x1, int y1, int x2, int y2, const Color &color) = 0;
	virtual void drawPixel(int x, int y, const Color &color) = 0;
	virtual bool save(const std::string &filename, const std::string &format, int quality) = 0;
	virtual void clean() = 0;
};
