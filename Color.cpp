
#include <cctype>
#include <stdexcept>
#include <cstdlib>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cctype>
#include "Color.h"
#include "util.h"

class ColorTable : public std::map<std::string, Color>
{
public:
	ColorTable(void)
	{
	map<std::string, Color> &table = *this;
	table["white"] =	Color(0xff, 0xff, 0xff);
	table["black"] =	Color(0, 0, 0);
	table["gray"] =		Color(0x7f, 0x7f, 0x7f);
	table["grey"] =		Color(0x7f, 0x7f, 0x7f);

	table["red"] =		Color(0xff, 0, 0);
	table["green"] =	Color(0, 0xff, 0);
	table["blue"] =		Color(0, 0, 0xff);

	table["yellow"] =	Color(0xff, 0xff, 0);
	table["magenta"] =	Color(0xff, 0, 0xff);
	table["fuchsia"] =	Color(0xff, 0, 0xff);
	table["cyan"] =		Color(0, 0xff, 0xff);
	table["aqua"] =		Color(0, 0xff, 0xff);

	table["orange"] =	Color(0xff, 0x7f, 0);
	table["chartreuse"] =	Color(0x7f, 0xff, 0);
	table["pink"] =		Color(0xff, 0, 0x7f);
	table["violet"] =	Color(0x7f, 0, 0xff);
	table["springgreen"] =	Color(0, 0xff, 0x7f);
	table["azure"] =	Color(0, 0x7f, 0xff);

	table["brown"] =	Color(0x7f, 0x3f, 0);
	}
};
ColorTable colorTable;


// alpha:
//	 0: don't expect/allow alpha
//	 1: allow alpha; defaults to 255 (0xff)
//	-1: allow alpha but ignore - set to to 255 (0xff)
Color::Color(std::string color, int alpha)
{
	size_t pos = color.find_first_of("+-");
	std::string basecolor = color.substr(0, pos);
	std::string colormod;
	if (pos != std::string::npos)
		colormod = color.substr(pos);
	do {
		if (basecolor[0] != '#') {
			basecolor = strlower(basecolor);
			if (colorTable.count(basecolor) > 0) {
				*this = colorTable[basecolor];
				break;
			}
			throw std::runtime_error(std::string("Symbolic color '") + color + "' not known, or color does not begin with #");
		}
		if (std::string::npos != basecolor.find_first_not_of("0123456789abcdefABCDEF",1)) {
			throw std::runtime_error("Color value has invalid digits (expected: [0-9a-zA-Z])");
		}
		if (alpha) {
			if (basecolor.length() != 4 && basecolor.length() != 5 && basecolor.length() != 7 && basecolor.length() != 9)
				throw std::runtime_error(std::string("Color '") + color + "' not in the expected format (#xxx, #xxxx, #xxxxxx or #xxxxxxxx)");
		}
		else {
			if (basecolor.length() == 5 || basecolor.length() == 9)
				throw std::runtime_error(std::string("Color '") + color + "' not in the expected format (#xxx or #xxxxxx) - alpha not allowed");
			if (basecolor.length() != 4 && basecolor.length() != 7)
				throw std::runtime_error(std::string("Color '") + color + "' not in the expected format (#xxx or #xxxxxx)");
		}
		unsigned col = strtoul(basecolor.c_str() + 1, NULL, 16);
		if (basecolor.length() < 6) {
			if (basecolor.length() == 5)
				a = (col >> 12) & 0x0f;
			else
				a = 0x0f;
			a |= a << 4;
			r = (col >> 8) & 0x0f;
			r |= r << 4;
			g = (col >> 4) & 0x0f;
			g |= g << 4;
			b = (col >> 0) & 0x0f;
			b |= b << 4;
		}
		else {
			if (basecolor.length() == 9)
				a = (col >> 24) & 0xff;
			else
				a = 0xff;
			r = (col >> 16) & 0xff;
			g = (col >> 8) & 0xff;
			b = (col >> 0) & 0xff;
		}
		if (alpha <= 0)
			a = 0xff;
	} while(false);
	if (colormod.length() == 0)
		return;
	std::istringstream iss(colormod);
	iss >> std::ws;
	while (!iss.eof()) {
		double w;
		char operation;
		char ccode='w';
		iss >> operation;
		iss >> ccode;
		iss >> w >> std::ws;
		if (w > 1)
			w = w / 255;
		if (!strchr("+-",operation) || !strchr("wkrgbcmy", ccode) || w < 0 || w > 1 || iss.fail())
			throw std::runtime_error(std::string("Invalid color modification(s) (") + color + "); expected one or more of [+-][wkrgbcmy]<n> (n = 0..1)");
		double v;
		if (operation == '+' || operation == '-') {
			// Mix in a color
			int mixr = r;
			int mixg = g;
			int mixb = b;
			if (operation == '+') {
				if (ccode == 'w')
					mixr = mixg = mixb = 255;
				else if (operation == 'k')
					mixr = mixg = mixb = 0;
				else if (ccode == 'r')
					mixr = 255;
				else if (ccode == 'g')
					mixg = 255;
				else if (ccode == 'b')
					mixb = 255;
				else if (ccode == 'c')
					mixg = mixb = 255;
				else if (ccode == 'm')
					mixr = mixb = 255;
				else if (ccode == 'y')
					mixr = mixg = 255;
			}
			else {
				if (ccode == 'w')
					mixr = mixg = mixb = 0;
				else if (operation == 'k')
					mixr = mixg = mixb = 255;
				else if (ccode == 'r')
					mixr = 0;
				else if (ccode == 'g')
					mixg = 0;
				else if (ccode == 'b')
					mixb = 0;
				else if (ccode == 'c')
					mixg = mixb = 0;
				else if (ccode == 'm')
					mixr = mixb = 0;
				else if (ccode == 'y')
					mixr = mixg = 0;
			}

			v = r * (1 - w) + mixr * w;
			if (v < 0) v = 0;
			else if (v > 255) v = 255;
			r = int(v + 0.5);
			v = g * (1 - w) + mixg * w;
			if (v < 0) v = 0;
			else if (v > 255) v = 255;
			g = int(v + 0.5);
			v = b * (1 - w) + mixb * w;
			if (v < 0) v = 0;
			else if (v > 255) v = 255;
			b = int(v + 0.5);
		}
	}
}

