/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  23.08.2012 12:35:59
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#pragma once

#include <climits>
#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "BlockPos.h"
#include "Color.h"
#include "PaintEngine.h"
#include "PixelAttributes.h"
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

#define SUGGESTION_ALL			0xffffffff
#define SUGGESTION_PREFETCH		0x00000001

class TileGenerator
{
private:
	typedef std::unordered_map<std::string, ColorEntry> NodeColorMap;
	typedef std::unordered_map<int, std::string> NodeID2NameMap;

public:
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
	void setSilenceSuggestion(unsigned flags);
	void setGenerateNoPrefetch(int enable);
	void setDBFormat(BlockPos::StrFormat format, bool query);
	void setHeightMap(bool enable);
	void setHeightMapYScale(float scale);
	void setSeaLevel(int level);
	void setBgColor(const Color &bgColor);
	void setBlockDefaultColor(const Color &color);
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
	void setDrawIgnore(bool drawIgnore);
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
	void setScanEntireWorld(bool enable);
	void setChunkSize(int size);
	void generate(const std::string &input, const std::string &output);
	Color computeMapHeightColor(int height);

private:
	std::string getWorldDatabaseBackend(const std::string &input);
	int getMapChunkSize(const std::string &input);
	void openDb(const std::string &input);
	void closeDb();
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
	void renderMapBlock(const std::vector<unsigned char> &mapBlock, const BlockPos &pos, int version);
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

public:
	static const BlockPos BlockPosLimitMin;
	static const BlockPos BlockPosLimitMax;

	int verboseCoordinates{ 0 };
	int verboseReadColors{ 0 };
	int verboseStatistics{ 0 };
	bool progressIndicator{ false };

private:
	unsigned m_silenceSuggestions{ 0 };
	bool m_heightMap{ false };
	float m_heightMapYScale{ 1 };
	int m_seaLevel{ 0 };
	Color m_bgColor{ 255, 255, 255 };
	Color m_blockDefaultColor{ 0, 0, 0, 0 };
	Color m_scaleColor{ 0, 0, 0 };
	Color m_originColor{ 255, 0, 0 };
	Color m_playerColor{ 255, 0, 0 };
	Color m_tileBorderColor{ 0, 0, 0 };
	bool m_drawOrigin{ false };
	bool m_drawPlayers{ false };
	int m_drawScale{ DRAWSCALE_NONE };
	bool m_drawAlpha{ false };
	bool m_drawAir{ false };
	bool m_drawIgnore{ false };
	bool m_shading{ true };
	std::string m_backend{ DEFAULT_BACKEND };
	std::string m_requestedBackend{ DEFAULT_BACKEND };
	bool m_scanEntireWorld{ false };
	bool m_shrinkGeometry{ true };
	bool m_blockGeometry{ false };
	int m_scaleFactor{ 1 };
	int m_chunkSize{ 0 };
	int m_sideScaleMajor{ 0 };
	int m_sideScaleMinor{ 0 };
	int m_heightScaleMajor{ 0 };
	int m_heightScaleMinor{ 0 };

	DB *m_db = nullptr;
	int m_generateNoPrefetch{ 0 };
	bool m_databaseFormatSet{ false };
	BlockPos::StrFormat m_databaseFormat{ BlockPos::Unknown };
	std::string m_recommendedDatabaseFormat;
	long long m_databaseFormatFound[BlockPos::STRFORMAT_MAX];
	bool m_reportDatabaseFormat{ false };
	PaintEngine *paintEngine = nullptr;
	PixelAttributes m_blockPixelAttributes;
	PixelAttributes m_blockPixelAttributesScaled;
	int m_xMin{ INT_MAX / 16 - 1 };
	int m_xMax{ INT_MIN / 16 + 1 };
	int m_zMin{ INT_MAX / 16 - 1 };
	int m_zMax{ INT_MIN / 16 + 1 };
	int m_yMin{ INT_MAX / 16 - 1 };
	int m_yMax{ INT_MIN / 16 + 1 };
	int m_reqXMin{ MAPBLOCK_MIN };
	int m_reqXMax{ MAPBLOCK_MAX };
	int m_reqYMin{ MAPBLOCK_MIN };
	int m_reqYMax{ MAPBLOCK_MAX };
	int m_reqZMin{ MAPBLOCK_MIN };
	int m_reqZMax{ MAPBLOCK_MAX };
	int m_reqYMinNode{ 0 };		// Node offset within a map block
	int m_reqYMaxNode{ 15 };		// Node offset within a map block
	int m_YMinMapped{ MAPBLOCK_MAX };		// Lowest block number mapped (not empty or air)
	int m_YMaxMapped{ MAPBLOCK_MIN };		// Higher block number mapped (not empty or air)
	long long m_emptyMapArea{ 0 };	// Number of blocks that are partly empty in the map
	long long m_worldBlocks;	// Number of blocks in the world (if known)
	int m_storedWidth;
	int m_storedHeight;
	int m_mapXStartNodeOffset{ 0 };
	int m_mapYStartNodeOffset{ 0 };
	int m_mapXEndNodeOffset{ 0 };
	int m_mapYEndNodeOffset{ 0 };
	int m_mapXStartNodeOffsetOrig;
	int m_mapYStartNodeOffsetOrig;
	int m_mapXEndNodeOffsetOrig;
	int m_mapYEndNodeOffsetOrig;
	int m_tileXOrigin{ TILECENTER_AT_WORLDCENTER };
	int m_tileZOrigin{ TILECENTER_AT_WORLDCENTER };
	int m_tileXCentered;
	int m_tileYCentered;
	int m_tileWidth{ 0 };
	int m_tileHeight{ 0 };
	int m_tileBorderSize{ 1 };
	int m_tileMapXOffset{ 0 };
	int m_tileMapYOffset{ 0 };
	int m_tileBorderXCount{ 0 };
	int m_tileBorderYCount{ 0 };
	int m_pictWidth;
	int m_pictHeight;
	int m_surfaceHeight{ INT_MIN };
	int m_surfaceDepth{ INT_MAX };
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

