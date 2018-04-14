#pragma once

#include <istream>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>

#include "Color.h"

class DataFileParser
{
public:
	using NodeColorMap = std::unordered_map<std::string, ColorEntry>;
	using HeightMapColorList = std::list<HeightMapColor>;

	DataFileParser(int verboseReadColors = 0, bool drawAlpha = false);
	~DataFileParser();
	void parseDataFile(const std::string &fileName, const char *type, int depth = 0);

	void parseDataStream(std::istream &in, const std::string &filename, int depth, const char *type);

	virtual void parseLine(const std::string &line, std::string name,
		std::istringstream &iline, int linenr, const std::string &filename) = 0;

protected:
	int verboseReadColors = 0;
	bool m_drawAlpha = false;
};

class ColorsFileParser : public DataFileParser
{
public:
	ColorsFileParser(int verboseReadColors = 0, bool drawAlpha = false) : DataFileParser(verboseReadColors, drawAlpha) {}
	NodeColorMap getNodeColors() { return m_nodeColors; }

private:
	const std::string type = "map colors";
	void parseLine(const std::string &line, std::string name, std::istringstream &iline, int linenr, const std::string &filename) override;
	NodeColorMap m_nodeColors;
};

class HeightMapColorsFileParser : public DataFileParser
{
public:
	HeightMapColorsFileParser(int verboseReadColors = 0, bool drawAlpha = false) : DataFileParser(verboseReadColors, drawAlpha) {}
	HeightMapColorList getHeightMapColors() { return m_heightMapColors; }

private:
	void parseLine(const std::string &line, std::string name, std::istringstream &iline, int linenr, const std::string &filename) override;
	HeightMapColorList m_heightMapColors;
};

class HeightMapNodesFileParser : public DataFileParser
{
public:
	HeightMapNodesFileParser(int verboseReadColors = 0, bool drawAlpha = false) : DataFileParser(verboseReadColors, drawAlpha) {}
	NodeColorMap getNodeColors() { return m_nodeColors; }

private:
	void parseLine(const std::string &line, std::string name, std::istringstream &iline, int linenr, const std::string &filename) override;
	NodeColorMap m_nodeColors;
};
