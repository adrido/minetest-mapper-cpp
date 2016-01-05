
#ifndef COLOR_H
#define COLOR_H

#include <string>

struct Color {
	Color(): r(0), g(0), b(0), a(0) {};
	Color(uint32_t c): r((c >> 16) & 0xff), g((c >> 8) & 0xff), b((c >> 0) & 0xff ), a((c >> 24) & 0xff) {};
	Color(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b), a(0xff) {};
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a): r(r), g(g), b(b), a(a) {};
	Color(std::string s, int alpha = 1);
	Color &operator=(const Color &c);
	unsigned to_uint() const { return (unsigned(a) << 24) + (unsigned(r) << 16) + (unsigned(g) << 8) + unsigned(b); }
	//libgd treats 127 as transparent, and 0 as opaque ...
	int to_libgd() const { return (((0xff - int(a)) >> 1) << 24) + (int(r) << 16) + (int(g) << 8) + int(b); }
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct ColorEntry {
	enum flags {
		FlagNone	= 0x00,
		FlagIgnore	= 0x01,
		FlagAir		= 0x02,
	};
	ColorEntry(): r(0), g(0), b(0), a(0), t(0), f(0) {};
	ColorEntry(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint8_t t, uint8_t f): r(r), g(g), b(b), a(a), t(t), f(f) {};
	inline Color to_color() const { return Color(r, g, b, a); }
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	uint8_t t;
	uint8_t f;
};

inline Color &Color::operator=(const Color &c)
{
	r = c.r;
	g = c.g;
	b = c.b;
	a = c.a;
	return *this;
}

#endif // COLOR_H
