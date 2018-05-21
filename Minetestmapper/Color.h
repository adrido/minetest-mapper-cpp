#pragma once

#include <cstdint>
#include <string>

struct Color {
	constexpr Color() = default;
	constexpr Color(uint32_t c) : r((c >> 16) & 0xff), g((c >> 8) & 0xff), b((c >> 0) & 0xff), a((c >> 24) & 0xff){};
	constexpr Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), a(0xff){};
	constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a){};
	Color(const std::string &color, int alpha = 1);
	Color &operator=(const Color &c) = default;
	constexpr unsigned to_uint() const { return (unsigned(a) << 24) + (unsigned(r) << 16) + (unsigned(g) << 8) + unsigned(b); }
	//libgd treats 127 as transparent, and 0 as opaque ...
	constexpr int to_libgd() const { return (((0xff - int(a)) >> 1) << 24) + (int(r) << 16) + (int(g) << 8) + int(b); }
	uint8_t r{0};
	uint8_t g{0};
	uint8_t b{0};
	uint8_t a{0};
};

struct ColorEntry {
	enum flags {
		FlagNone = 0x00,
		FlagIgnore = 0x01,
		FlagAir = 0x02,
	};
	constexpr ColorEntry() = default;
	constexpr ColorEntry(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint8_t t, uint8_t f) : r(r), g(g), b(b), a(a), t(t), f(f){};
	constexpr Color to_color() const { return Color(r, g, b, a); }
	uint8_t r{0};
	uint8_t g{0};
	uint8_t b{0};
	uint8_t a{0};
	uint8_t t{0};
	uint8_t f{0};
};

struct HeightMapColor {
	constexpr HeightMapColor(int h0, Color c0, int h1, Color c1) : height{h0, h1}, color{c0, c1} {}
	int height[2];
	Color color[2];
};
