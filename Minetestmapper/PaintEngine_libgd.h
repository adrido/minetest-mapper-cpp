#pragma once
#include "paintEngine.h"
#include "CharEncodingConverter.h"
#include <gd.h>

class PaintEngine_libgd :
	public PaintEngine
{
public:
	PaintEngine_libgd();
	~PaintEngine_libgd();
	bool create(int w, int h);
	void fill(const Color &color);
	void drawText(int x, int y, Font font, const std::string &text, const Color &color);
	void drawChar(int x, int y, Font font, char ch, const Color &color);
	void drawRect(int x1, int y1, int x2, int y2, const Color &color);
	void drawLine(int x1, int y1, int x2, int y2, const Color &color);
	void drawCircle(int x, int y, int r, const Color &color);
	void drawArc(int x, int y, int w, int h, int start, int end, const Color &color);
	void drawFilledRect(int x1, int y1, int x2, int y2, const Color &color);
	void drawPixel(int x, int y, const Color &color);
	bool save(const std::string &filename, const std::string &format, int quality);
	void clean();
private:
	gdImagePtr image = nullptr;
	int width = 0;
	int height = 0;
	gdFontPtr getGdFont(Font font);
	CharEncodingConverter *gdStringConv = nullptr;
};

