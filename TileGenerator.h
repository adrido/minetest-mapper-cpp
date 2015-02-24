/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  23.08.2012 12:35:59
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#ifndef TILEGENERATOR_H_JJNUCARH
#define TILEGENERATOR_H_JJNUCARH

#include <gd.h>
#include <climits>
#include <iosfwd>
#include <list>
#if __cplusplus >= 201103L
#include <unordered_map>
#else
#include <map>
#endif
#include <set>
#include <list>
#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include "types.h"
#include "PixelAttributes.h"
#include "BlockPos.h"
#include "Color.h"
#include "db.h"

#define TILESIZE_CHUNK			(INT_MIN)
#define TILECENTER_AT_WORLDCENTER	(INT_MAX)
#define TILECORNER_AT_WORLDCENTER	(INT_MAX - 1)
#define TILECENTER_AT_CHUNKCENTER	(INT_MAX - 2)
#define TILECENTER_AT_MAPCENTER		(INT_MIN)
#define TILECORNER_AT_MAPCENTER		(INT_MIN + 1)

#define DRAWSCALE_NONE			0x00
#define DRAWSCALE_MASK			0x0f
#define DRAWSCALE_LEFT			0x01
#define DRAWSCALE_RIGHT			0x02
#define DRAWSCALE_TOP			0x04
#define DRAWSCALE_BOTTOM		0x08
#define DRAWHEIGHTSCALE_MASK		0xf0
#define DRAWHEIGHTSCALE_LEFT		0x10
#define DRAWHEIGHTSCALE_RIGHT		0x20
#define DRAWHEIGHTSCALE_TOP		0x40
#define DRAWHEIGHTSCALE_BOTTOM		0x80

#define SCALESIZE_HOR			40
#define SCALESIZE_VERT			50
#define HEIGHTSCALESIZE			60

class TileGenerator
{
private:
#if __cplusplus >= 201103L
	typedef std::unordered_map<std::string, ColorEntry> NodeColorMap;
	typedef std::unordered_map<int, std::string> NodeID2NameMap;
#else
	typedef std::map<std::string, ColorEntry> NodeColorMap;
	typedef std::map<int, std::string> NodeID2NameMap;
#endif
public:
	struct HeightMapColor
	{
		HeightMapColor(int h0, Color c0, int h1, Color c1) : height{h0, h1}, color{c0, c1} {}
		int height[2];
		Color color[2];
	};
	typedef std::list<HeightMapColor> HeightMapColorList;
	struct DrawObject {
		void setCenter(const NodeCoord &c) { haveCenter = true; center = c; }
		void setCorner1(const NodeCoord &c) { haveCenter = false; corner1 = c; }
		void setDimensions(const NodeCoord &d) { haveDimensions = true; dimensions = d; }
		void setCorner2(const NodeCoord &c) { haveDimensions = false; corner2 = c; }
		enum Type {
			Unknown,
			Point,
			Line,
			Ellipse,
			Rectangle,
			Text
		};
		bool world;
		Type type;
		bool haveCenter;
		NodeCoord corner1;
		NodeCoord center;
		bool haveDimensions;
		NodeCoord corner2;
		NodeCoord dimensions;
		Color color;
		std::string text;
	};

	struct UnpackError
	{
		BlockPos pos;
		const char *type;
		size_t offset;
		size_t length;
		size_t dataLength;
		UnpackError(const char *t, size_t o, size_t l, size_t dl) : type(t), offset(o), length(l), dataLength(dl) {}
	};

	TileGenerator();
	~TileGenerator();
	void setGenerateNoPrefetch(int enable);
	void setHeightMap(bool enable);
	void setHeightMapYScale(float scale);
	void setSeaLevel(int level);
	void setBgColor(const Color &bgColor);
	void setBlockDefaultColor(const Color &olor);
	void setScaleColor(const Color &scaleColor);
	void setOriginColor(const Color &originColor);
	void setPlayerColor(const Color &playerColor);
	void setHeightMapColor(const Color &color0, const Color &color1);
	Color parseColor(const Color &color);
	void setDrawOrigin(bool drawOrigin);
	void setDrawPlayers(bool drawPlayers);
	void setDrawScale(int scale);
	void setDrawHeightScale(int scale);
	void setSideScaleInterval(int major, int minor);
	void setHeightScaleInterval(int major, int minor);
	void setDrawAlpha(bool drawAlpha);
	void setDrawAir(bool drawAir);
	void drawObject(const DrawObject &object) { m_drawObjects.push_back(object); }
	void setShading(bool shading);
	void setGeometry(const NodeCoord &corner1, const NodeCoord &corner2);
	void setMinY(int y);
	void setMaxY(int y);
	void setShrinkGeometry(bool shrink);
	void setBlockGeometry(bool block);
	void setTileBorderColor(const Color &tileBorderColor);
	void setTileBorderSize(int size);
	void setTileSize(int width, int heigth);
	void setTileOrigin(int x, int y);
	void setTileCenter(int x, int y);
	void setScaleFactor(int f);
	void enableProgressIndicator(void);
	void parseNodeColorsFile(const std::string &fileName);
	void parseHeightMapNodesFile(const std::string &fileName);
	void parseHeightMapColorsFile(const std::string &fileName);
	void setBackend(std::string backend);
	void setChunkSize(int size);
	void generate(const std::string &input, const std::string &output);
	Color computeMapHeightColor(int height);

private:
	std::string getWorldDatabaseBackend(const std::string &input);
	int getMapChunkSize(const std::string &input);
	void openDb(const std::string &input);
	void sanitizeParameters(void);
	void loadBlocks();
	void createImage();
	void computeMapParameters(const std::string &input);
	void computeTileParameters(
		// Input parameters
		int minPos,
		int maxPos,
		int mapStartNodeOffset,
		int mapEndNodeOffset,
		int tileOrigin,
		int tileSize,
		// Output parameters
		int &tileBorderCount,
		int &tileMapOffset,
		int &tileMapExcess,
		// Behavior selection
		bool ascending);
	void renderMap();
	std::list<int> getZValueList() const;
	void pushPixelRows(PixelAttributes &pixelAttributes, int zPosLimit);
	void scalePixelRows(PixelAttributes &pixelAttributes, PixelAttributes &pixelAttributesScaled, int zPosLimit);
	void processMapBlock(const DB::Block &block);
	void renderMapBlock(const ustring &mapBlock, const BlockPos &pos, int version);
	void renderScale();
	void renderHeightScale();
	void renderOrigin();
	void renderPlayers(const std::string &inputPath);
	void renderDrawObjects();
	void writeImage(const std::string &output);
	void printUnknown();
	int mapX2ImageX(int val) const;
	int mapY2ImageY(int val) const;
	int worldX2ImageX(int val) const;
	int worldZ2ImageY(int val) const;
	int worldBlockX2StoredX(int xPos) const { return (xPos - m_xMin) * 16; }
	int worldBlockZ2StoredY(int zPos) const { return (m_zMax - zPos) * 16; }
	int borderTop() const { return ((m_drawScale & DRAWSCALE_TOP) ? SCALESIZE_HOR : 0) + (m_heightMap && (m_drawScale & DRAWHEIGHTSCALE_TOP) ? HEIGHTSCALESIZE : 0); }
	int borderBottom() const { return ((m_drawScale & DRAWSCALE_BOTTOM) ? SCALESIZE_HOR : 0) + (m_heightMap && (m_drawScale & DRAWHEIGHTSCALE_BOTTOM) ? HEIGHTSCALESIZE : 0); }
	int borderLeft() const { return ((m_drawScale & DRAWSCALE_LEFT) ? SCALESIZE_VERT : 0) + (m_heightMap && (m_drawScale & DRAWHEIGHTSCALE_LEFT) ? HEIGHTSCALESIZE : 0); }
	int borderRight() const { return ((m_drawScale & DRAWSCALE_RIGHT) ? SCALESIZE_VERT : 0) + (m_heightMap && (m_drawScale & DRAWHEIGHTSCALE_RIGHT) ? HEIGHTSCALESIZE : 0); }

	void parseDataFile(const std::string &fileName, int depth, const char *type,
		void (TileGenerator::*parseLine)(const std::string &line, std::string name,
			std::istringstream &iline, int linenr, const std::string &filename));
	void parseDataStream(std::istream &in, const std::string &filename, int depth, const char *type,
		void (TileGenerator::*parseLine)(const std::string &line, std::string name,
			std::istringstream &iline, int linenr, const std::string &filename));
	void parseNodeColorsLine(const std::string &line, std::string name, std::istringstream &iline,
		int linenr, const std::string &filename);
	void parseHeightMapNodesLine(const std::string &line, std::string name, std::istringstream &iline,
		int linenr, const std::string &filename);
	void parseHeightMapColorsLine(const std::string &line, std::string name, std::istringstream &iline,
		int linenr, const std::string &filename);

public:
	int verboseCoordinates;
	int verboseReadColors;
	int verboseStatistics;
	bool progressIndicator;

private:
	bool m_heightMap;
	float m_heightMapYScale;
	int m_seaLevel;
	Color m_bgColor;
	Color m_blockDefaultColor;
	Color m_scaleColor;
	Color m_originColor;
	Color m_playerColor;
	Color m_tileBorderColor;
	bool m_drawOrigin;
	bool m_drawPlayers;
	int m_drawScale;
	bool m_drawAlpha;
	bool m_drawAir;
	bool m_shading;
	std::string m_backend;
	bool m_shrinkGeometry;
	bool m_blockGeometry;
	int m_scaleFactor;
	int m_chunkSize;
	int m_sideScaleMajor;
	int m_sideScaleMinor;
	int m_heightScaleMajor;
	int m_heightScaleMinor;

	DB *m_db;
	bool m_generateNoPrefetch;
	long long m_databaseFormatFound[BlockPos::STRFORMAT_MAX];
	gdImagePtr m_image;
	PixelAttributes m_blockPixelAttributes;
	PixelAttributes m_blockPixelAttributesScaled;
	int m_xMin;
	int m_xMax;
	int m_zMin;
	int m_zMax;
	int m_yMin;
	int m_yMax;
	int m_reqXMin;
	int m_reqXMax;
	int m_reqYMin;
	int m_reqYMax;
	int m_reqZMin;
	int m_reqZMax;
	int m_reqYMinNode;		// Node offset within a map block
	int m_reqYMaxNode;		// Node offset within a map block
	int m_YMinMapped;		// Lowest block number mapped (not empty or air)
	int m_YMaxMapped;		// Higher block number mapped (not empty or air)
	long long m_emptyMapArea;	// Number of blocks that are partly empty in the map
	long long m_worldBlocks;	// Number of blocks in the world (if known)
	int m_storedWidth;
	int m_storedHeight;
	int m_mapXStartNodeOffset;
	int m_mapYStartNodeOffset;
	int m_mapXEndNodeOffset;
	int m_mapYEndNodeOffset;
	int m_mapXStartNodeOffsetOrig;
	int m_mapYStartNodeOffsetOrig;
	int m_mapXEndNodeOffsetOrig;
	int m_mapYEndNodeOffsetOrig;
	int m_tileXOrigin;
	int m_tileZOrigin;
	int m_tileXCentered;
	int m_tileYCentered;
	int m_tileWidth;
	int m_tileHeight;
	int m_tileBorderSize;
	int m_tileMapXOffset;
	int m_tileMapYOffset;
	int m_tileBorderXCount;
	int m_tileBorderYCount;
	int m_pictWidth;
	int m_pictHeight;
	int m_surfaceHeight;
	int m_surfaceDepth;
	std::list<BlockPos> m_positions;
	NodeID2NameMap m_nameMap;
	static const ColorEntry *NodeColorNotDrawn;
	const ColorEntry *m_nodeIDColor[MAPBLOCK_MAXCOLORS];
	NodeColorMap m_nodeColors;
	HeightMapColorList m_heightMapColors;
	uint16_t m_readedPixels[16];
	std::set<std::string> m_unknownNodes;
	std::vector<DrawObject> m_drawObjects;
}; /* -----  end of class TileGenerator  ----- */

#endif /* end of include guard: TILEGENERATOR_H_JJNUCARH */

