#pragma once

#include "TileGenerator.h"
#include <istream>
#include <string>

#define OPT_SQLITE_CACHEWORLDROW	0x81
#define OPT_PROGRESS_INDICATOR		0x82
#define OPT_DRAW_OBJECT			0x83
#define OPT_BLOCKCOLOR			0x84
#define OPT_DRAWAIR			0x85
#define OPT_VERBOSE_SEARCH_COLORS	0x86
#define OPT_CHUNKSIZE			0x87
#define OPT_HEIGHTMAP			0x88
#define OPT_HEIGHTMAPYSCALE		0x89
#define OPT_HEIGHT_LEVEL0		0x8a
#define OPT_HEIGHTMAPNODESFILE		0x8b
#define OPT_HEIGHTMAPCOLORSFILE		0x8c
#define OPT_DRAWHEIGHTSCALE		0x8d
#define OPT_SCALEFACTOR			0x8e
#define OPT_SCALEINTERVAL		0x8f
#define OPT_NO_BLOCKLIST_PREFETCH	0x90
#define OPT_DATABASE_FORMAT		0x91
#define OPT_SILENCE_SUGGESTIONS		0x92
#define OPT_PRESCAN_WORLD		0x93
#define OPT_DRAWNODES			0x94
#define OPT_SQLITE_LIMIT_PRESCAN_QUERY	0x95

#define DRAW_ARROW_LENGTH		10
#define DRAW_ARROW_ANGLE		30

// Will be used in error messages if nothing better is available.
#define DEFAULT_PROGRAM_NAME		"minetestmapper"

class FuzzyBool {
private:
	int m_value{0};
	FuzzyBool(int i) : m_value(i) {}
public:
	FuzzyBool()  = default;
	FuzzyBool(bool b) : m_value(b ? Yes.m_value : No.m_value) {}
	static const FuzzyBool Yes;
	static const FuzzyBool Maybe;
	static const FuzzyBool No;
	inline friend bool operator==(FuzzyBool f1, FuzzyBool f2) { return f1.m_value == f2.m_value; }
	inline friend bool operator!=(FuzzyBool f1, FuzzyBool f2) { return f1.m_value != f2.m_value; }
	inline friend bool operator>=(FuzzyBool f1, FuzzyBool f2) { return f1.m_value >= f2.m_value; }
	inline friend bool operator<=(FuzzyBool f1, FuzzyBool f2) { return f1.m_value <= f2.m_value; }
	inline friend bool operator<(FuzzyBool f1, FuzzyBool f2) { return f1.m_value < f2.m_value; }
	inline friend bool operator>(FuzzyBool f1, FuzzyBool f2) { return f1.m_value < f2.m_value; }
};


class Mapper 
{
public:
	Mapper(const std::string &executablePath, const std::string &executableName);
	int start(int argc, char *argv[]);

private:
	TileGenerator generator;
	const std::string executableName;
	const std::string executablePath;

	void usage();

	std::string input;
	std::string output;
	bool heightMap = false;
	bool loadHeightMapColorsFile = false;
	std::string nodeColorsFile;
	std::string heightMapColorsFile;
	std::string heightMapNodesFile;
	bool foundGeometrySpec = false;
	bool setFixedOrShrinkGeometry = false;

	const std::string nodeColorsDefaultFile = "colors.txt";
	const std::string heightMapNodesDefaultFile = "heightmap-nodes.txt";
	const std::string heightMapColorsDefaultFile = "heightmap-colors.txt";

	static bool validStreamAtWsOrEof(std::istream & is)
	{
		// May need to peek before knowing it's EOF.
		// However, peeking twice results in fail()...
		return !is.fail() && (is.eof() || is.peek() == ' ' || is.eof() || is.peek() == '\t');
	}

	static bool validStreamAtEof(std::istream & is)
	{
		// May need to peek before knowing it's EOF.
		// However, peeking twice at eof results in fail()...
		return !is.fail() && (is.eof() || (is.peek(), is.eof()));
	}

	static int safePeekStream(std::istream & is)
	{
		// Don't peek at EOF, else stream fails...
		return is.eof() ? EOF : is.peek();
	}

	void parseDataFile(TileGenerator &generator, const std::string &input, std::string dataFile, const std::string& defaultFile,
		void (TileGenerator::*parseFile)(const std::string &fileName));

	// is: stream to read from
	// coord: set to coordinate value that was read
	// isBlockCoord: set to true if the coordinate read was a block coordinate
	// wildcard: if non-zero, accept '*' as a coordinate, and return this value instead.
	//	(suggested values for 'wildcard': INT_MIN or INT_MAX)
	//
	// Accepted coordinate syntax:
	// 	[+-]<n>:	node coordinate:  node +/- n
	// 	[+-]<b>#:	block coordinate: block +/- b	(isBlockCoord will be set to true)
	// 	[+-]<b>#<n>:	node coordinate:  node <n> in block +/- <b>
	// 	[+-]<b>.<n>:	node coordinate:  node +/- (b * 16 + n)
	// As a special feature, double signs are also supported. E.g.:
	//	+-3
	// Which allows shell command-lines like the following
	//	${width}x${height}+$xoffs+$yoffs
	// (which otherwise require special measures to cope with xoffs or yoffs being negative...)
	// Other uses of this feature are left as an excercise to the reader.
	// Hint: --3.5 is *not* the same as 3.5
	static bool parseNodeCoordinate(std::istream &is, int &coord, bool &isBlockCoord, int wildcard);

	static bool parseCoordinates(std::istream &is, NodeCoord &coord, int n, int wildcard = 0, char separator = ',');

	static void convertBlockToNodeCoordinates(NodeCoord &coord, int offset, int n);

	static void convertBlockToNodeCoordinates(NodeCoord &coord1, NodeCoord &coord2, int n);

	static void convertCenterToCornerCoordinates(NodeCoord &coord, NodeCoord &dimensions, int n);

	static void convertDimensionToCornerCoordinates(NodeCoord &coord1, NodeCoord &coord2, NodeCoord &dimensions, int n);

	static void convertCornerToDimensionCoordinates(NodeCoord &coord1, NodeCoord &coord2, NodeCoord &dimensions, int n);

	static void convertPolarToCartesianCoordinates(NodeCoord &coord1, NodeCoord &coord2, double angle, double length);

	static void convertCartesianToPolarCoordinates(NodeCoord &coord1, NodeCoord &coord2, double &angle, double &length);

	static void orderCoordinateDimensions(NodeCoord &coord1, NodeCoord &coord2, int n);


	// Parse the following geometry formats:
	// <w>x<h>[+<x>+<y>]
	//	(dimensions, and position)
	//	(if x and y are omitted, they default to -w/2 and -h/2)
	// <x1>,<y1>:<x2>,<y2>
	//	(2 corners of the area)
	// <x>,<y>:<w>x<h>
	//	(center of the area, and dimensions)
	// <x>[,:]<y>+<w>+<h>
	//	(corner of the area, and dimensions)
	// <x>,<y>@<angle>+<length>
	static bool parseGeometry(std::istream &is, NodeCoord &coord1, NodeCoord &coord2, NodeCoord &dimensions, bool &legacy, bool &centered, int n, FuzzyBool expectDimensions, int wildcard = 0);

	static bool parseMapGeometry(std::istream &is, NodeCoord &coord1, NodeCoord &coord2, bool &legacy, FuzzyBool interpretAsCenter);

};



