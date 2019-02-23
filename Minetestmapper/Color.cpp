#include "Color.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

static const std::unordered_map<std::string, const Color> colorTable{
	{"white", Color(0xff, 0xff, 0xff)},
	{"black", Color(0, 0, 0)},
	{"gray", Color(0x7f, 0x7f, 0x7f)},
	{"grey", Color(0x7f, 0x7f, 0x7f)},

	{"red", Color(0xff, 0, 0)},
	{"green", Color(0, 0xff, 0)},
	{"blue", Color(0, 0, 0xff)},

	{"yellow", Color(0xff, 0xff, 0)},
	{"magenta", Color(0xff, 0, 0xff)},
	{"fuchsia", Color(0xff, 0, 0xff)},
	{"cyan", Color(0, 0xff, 0xff)},
	{"aqua", Color(0, 0xff, 0xff)},

	{"orange", Color(0xff, 0x7f, 0)},
	{"chartreuse", Color(0x7f, 0xff, 0)},
	{"pink", Color(0xff, 0, 0x7f)},
	{"violet", Color(0x7f, 0, 0xff)},
	{"springgreen", Color(0, 0xff, 0x7f)},
	{"azure", Color(0, 0x7f, 0xff)},

	{"brown", Color(0x7f, 0x3f, 0)},
};

// alpha:
//	 0: don't expect/allow alpha
//	 1: allow alpha; defaults to 255 (0xff)
//	-1: allow alpha but ignore - set to to 255 (0xff)
Color::Color(const std::string &color, int alpha)
{
	size_t pos = color.find_first_of("+-");
	std::string basecolor = color.substr(0, pos);
	std::string colormod;
	if (pos != std::string::npos) {
		colormod = color.substr(pos);
	}

	if (basecolor[0] != '#') { // Color name
		std::transform(basecolor.begin(), basecolor.end(), basecolor.begin(), ::tolower);
		auto colorTableIter = colorTable.find(basecolor);
		if (colorTableIter != colorTable.end()) {
			*this = colorTableIter->second;
		}
		else {
			throw std::runtime_error(std::string("Symbolic color '") + color + "' not known, or color does not begin with #");
		}
	}
	else { // Html color notation
		if (std::string::npos != basecolor.find_first_not_of("0123456789abcdefABCDEF", 1)) {
			throw std::runtime_error("Color value has invalid digits (expected: [0-9a-zA-Z])");
		}
		if (alpha) {
			if (basecolor.length() != 4 && basecolor.length() != 5 && basecolor.length() != 7 && basecolor.length() != 9) {
				throw std::runtime_error(std::string("Color '") + color + "' not in the expected format (#xxx, #xxxx, #xxxxxx or #xxxxxxxx)");
			}
		}
		else {
			if (basecolor.length() == 5 || basecolor.length() == 9) {
				throw std::runtime_error(std::string("Color '") + color + "' not in the expected format (#xxx or #xxxxxx) - alpha not allowed");
			}
			if (basecolor.length() != 4 && basecolor.length() != 7) {
				throw std::runtime_error(std::string("Color '") + color + "' not in the expected format (#xxx or #xxxxxx)");
			}
		}
		unsigned col = strtoul(basecolor.c_str() + 1, nullptr, 16);
		if (basecolor.length() < 6) {
			if (basecolor.length() == 5) {
				a = (col >> 12) & 0x0f;
			}
			else {
				a = 0x0f;
			}
			a |= a << 4;
			r = (col >> 8) & 0x0f;
			r |= r << 4;
			g = (col >> 4) & 0x0f;
			g |= g << 4;
			b = (col >> 0) & 0x0f;
			b |= b << 4;
		}
		else {
			if (basecolor.length() == 9) {
				a = (col >> 24) & 0xff;
			}
			else {
				a = 0xff;
			}
			r = (col >> 16) & 0xff;
			g = (col >> 8) & 0xff;
			b = (col >> 0) & 0xff;
		}
		if (alpha <= 0) {
			a = 0xff;
		}
	}
	if (colormod.length() == 0) {
		return;
	}
	std::istringstream iss(colormod);
	iss >> std::ws;
	while (!iss.eof()) {
		double w;
		char operation;
		char ccode = 'w';
		iss >> operation;
		iss >> ccode;
		iss >> w >> std::ws;
		if (w > 1) {
			w = w / 255;
		}
		if (!strchr("+-", operation) || !strchr("wkrgbcmy", ccode) || w < 0 || w > 1 || iss.fail()) {
			throw std::runtime_error(std::string("Invalid color modification(s) (") + color + "); expected one or more of [+-][wkrgbcmy]<n> (n = 0..1)");
		}
		double v;
		if (operation == '+' || operation == '-') {
			// Mix in a color
			int mixr = r;
			int mixg = g;
			int mixb = b;
			if (operation == '+') {
				if (ccode == 'w') {
					mixr = mixg = mixb = 255;
				}
				else if (ccode == 'k') {
					mixr = mixg = mixb = 0;
				}
				else if (ccode == 'r') {
					mixr = 255;
				}
				else if (ccode == 'g') {
					mixg = 255;
				}
				else if (ccode == 'b') {
					mixb = 255;
				}
				else if (ccode == 'c') {
					mixg = mixb = 255;
				}
				else if (ccode == 'm') {
					mixr = mixb = 255;
				}
				else if (ccode == 'y') {
					mixr = mixg = 255;
				}
			}
			else {
				if (ccode == 'w') {
					mixr = mixg = mixb = 0;
				}
				else if (ccode == 'k') {
					mixr = mixg = mixb = 255;
				}
				else if (ccode == 'r') {
					mixr = 0;
				}
				else if (ccode == 'g') {
					mixg = 0;
				}
				else if (ccode == 'b') {
					mixb = 0;
				}
				else if (ccode == 'c') {
					mixg = mixb = 0;
				}
				else if (ccode == 'm') {
					mixr = mixb = 0;
				}
				else if (ccode == 'y') {
					mixr = mixg = 0;
				}
			}

			v = r * (1 - w) + mixr * w;
			if (v < 0) {
				v = 0;
			}
			else if (v > 255) {
				v = 255;
			}
			r = int(v + 0.5);
			v = g * (1 - w) + mixg * w;
			if (v < 0) {
				v = 0;
			}
			else if (v > 255) {
				v = 255;
			}
			g = int(v + 0.5);
			v = b * (1 - w) + mixb * w;
			if (v < 0) {
				v = 0;
			}
			else if (v > 255) {
				v = 255;
			}
			b = int(v + 0.5);
		}
	}
}
