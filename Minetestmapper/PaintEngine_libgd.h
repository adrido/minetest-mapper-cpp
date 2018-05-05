#pragma once
#include "CharEncodingConverter.h"
#include "PaintEngine.h"
#include <gd.h>

class PaintEngine_libgd :
	public PaintEngine
{
public:
	PaintEngine_libgd();
	~PaintEngine_libgd() override;
	bool create(int w, int h) override;
	void fill(const Color &color) override;
	void drawText(int x, int y, Font font, const std::string &text, const Color &color) override;
	void drawChar(int x, int y, Font font, char ch, const Color &color) override;
	void drawRect(int x1, int y1, int x2, int y2, const Color &color) override;
	void drawLine(int x1, int y1, int x2, int y2, const Color &color) override;
	void drawCircle(int x, int y, int r, const Color &color) override;
	void drawArc(int x, int y, int w, int h, int start, int end, const Color &color) override;
	void drawFilledRect(int x1, int y1, int x2, int y2, const Color &color) override;
	void drawPixel(int x, int y, const Color &color) override;
	bool save(const std::string &filename, const std::string &format, int quality) override;
	void clean() override;
protected:
	gdImagePtr image = nullptr;
	int width = 0;
	int height = 0;
	gdFontPtr getGdFont(Font font) const;
	CharEncodingConverter *gdStringConv = nullptr;
};

