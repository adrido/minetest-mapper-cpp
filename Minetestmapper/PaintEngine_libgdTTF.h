#pragma once
#include "PaintEngine_libgd.h"
#include <gd.h>
#include <string>

class PaintEngine_libgdTTF :
	public PaintEngine_libgd
{
public:
	PaintEngine_libgdTTF();
	~PaintEngine_libgdTTF() override;

	//bool create(int w, int h);

	void drawText(int x, int y, Font font, const std::string &text, const Color &color) override;

private:
	//gdImagePtr image = nullptr;
	//int width = 0;
	//int height = 0;
	double getFontSize(PaintEngine::Font font) const;
	std::string getFont(PaintEngine::Font font) const;
};

