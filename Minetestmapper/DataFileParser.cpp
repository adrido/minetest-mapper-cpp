#include "DataFileParser.h"

#include <climits>
#include <fstream>
#include <iostream>

using namespace std;

DataFileParser::DataFileParser(int verboseReadColors, bool drawAlpha) :
	verboseReadColors(verboseReadColors),
	m_drawAlpha(drawAlpha)
{
};


DataFileParser::~DataFileParser()
= default;

void DataFileParser::parseDataFile(const std::string & fileName, const char * type, int depth)
{
	if (depth > 100) {
		throw std::runtime_error(std::string("Excessive inclusion depth of ") + type + " files - suspected recursion (i.e. cycle); current file: '" + fileName + "'");
	}
	if (depth == 0 && verboseReadColors >= 2) {
		cout << "Checking for " << type << " file: " << fileName << std::endl;
	}
	ifstream in;
	in.open(fileName.c_str(), ifstream::in);
	if (!in.is_open()) {
		throw std::runtime_error(std::string("Failed to open ") + type + " file '" + fileName + "'");
		return;
	}
	if (verboseReadColors >= 1) {
		cout << "Reading " << type << " file:  " << fileName << std::endl;
	}
	parseDataStream(in, fileName, depth, type);
	in.close();
}

void DataFileParser::parseDataStream(std::istream & in, const std::string & filename, int depth, const char * type)
{
	string line;
	int linenr = 0;
	for (std::getline(in, line); in.good(); std::getline(in, line)) {
		linenr++;
		size_t comment = line.find_first_of('#');
		if (comment != string::npos) {
			line.erase(comment);
		}
		istringstream iline;
		iline.str(line);
		iline >> std::skipws;
		string name;
		iline >> name >> std::ws;
		if (name.empty()) {
			continue;
		}
		if (name == "@include") {
			string includeFile;
			getline(iline, includeFile);
			size_t lastChar = includeFile.find_last_not_of(" \t\r\n");
			if (lastChar != string::npos) {
				includeFile.erase(lastChar + 1);
			}
			if (includeFile.empty()) {
				std::cerr << filename << ":" << linenr << ": include filename missing in colors file (" << line << ")" << std::endl;
				continue;
			}

			if (includeFile[0] != '/') {
				string includePath = filename;
				size_t offset = includePath.find_last_of('/');
				if (offset != string::npos) {
					includePath.erase(offset);
					includePath.append("/").append(includeFile);
					includeFile = includePath;
				}
			}

			parseDataFile(includeFile, type, depth + 1);
		}
		else {
			parseLine(line, name, iline, linenr, filename);
		}
	}
	if (!in.eof()) {
		std::cerr << filename << ": error reading colors file after line " << linenr << std::endl;
	}
}

void ColorsFileParser::parseLine(const std::string & line, std::string name, std::istringstream & iline, int linenr, const std::string & filename)
{
	iline >> std::ws >> std::skipws;
	if (iline.good() && iline.peek() == '-') {
		char c;
		iline >> c >> std::ws;
		if (iline.fail() || !iline.eof()) {
			std::cerr << filename << ":" << linenr << ": bad line in colors file (" << line << ")" << std::endl;
			return;
		}
		m_nodeColors.erase(name);
	}
	else {
		int r, g, b, a, t, f;
		std::string flags;
		ColorEntry color;
		iline >> r;
		iline >> g;
		iline >> b;
		if (iline.fail()) {
			std::cerr << filename << ":" << linenr << ": bad line in colors file (" << line << ")" << std::endl;
			return;
		}
		a = 0xff;
		iline >> std::ws;
		if (iline.good() && isdigit(iline.peek())) {
			iline >> a >> std::ws;
		}
		t = 0;
		if (iline.good() && isdigit(iline.peek())) {
			iline >> t >> std::ws;
		}
		if (iline.good() && !isdigit(iline.peek())) {
			iline >> flags >> std::ws;
		}
		f = 0;
		if (!iline.fail() && !flags.empty()) {
			for (char & flag : flags) {
				if (flag == ',') {
					flag = ' ';
				}
			}
			istringstream iflags(flags);
			std::string flag;
			iflags >> flag;
			while (!iflags.fail()) {
				if (flag == "ignore") {
					f |= ColorEntry::FlagIgnore;
				}
				else if (flag == "air") {
					f |= ColorEntry::FlagAir;
				}
				iflags >> flag;
			}
		}
		color = ColorEntry(r, g, b, a, t, f);
		if ((m_drawAlpha && a == 0xff) || (!m_drawAlpha && a != 0xff)) {
			// If drawing alpha, and the colors file contains both
			// an opaque entry and a non-opaque entry for a name, prefer
			// the non-opaque entry
			// If not drawing alpha, and the colors file contains both
			// an opaque entry and a non-opaque entry for a name, prefer
			// the opaque entry
			// Otherwise, any later entry overrides any previous entry
			auto it = m_nodeColors.find(name);
			if (it != m_nodeColors.end()) {
				if (m_drawAlpha && (a == 0xff && it->second.a != 0xff)) {
					// drawing alpha: don't use opaque color to override
					// non-opaque color
					return;
				}
				if (!m_drawAlpha && (a != 0xff && it->second.a == 0xff)) {
					// not drawing alpha: don't use non-opaque color to
					// override opaque color
					return;
				}
			}
		}
		m_nodeColors[name] = color;
	}
}

void HeightMapColorsFileParser::parseLine(const std::string & line, std::string name, std::istringstream & iline, int linenr, const std::string & filename)
{
	(void)name;
	int height[2];
	Color color[2];
	iline.str(line);		// Reset
	for (int & i : height) {
		iline >> std::ws;
		char c = iline.peek();
		iline >> i;
		if (iline.fail()) {
			std::string value;
			iline.clear();
			iline >> std::ws;
			iline >> value >> std::ws;
			if (!iline.fail()) {
				if (value == "-oo" || (c == '-' && value == "oo")) {
					i = INT_MIN;
				}
				else if (value == "oo" || value == "+oo") {
					i = INT_MAX;
				}
				else {
					iline.clear(ios::failbit);	// Set to failed
					break;
				}
			}
		}
	}
	for (auto & i : color) {
		int r, g, b;
		iline >> r;
		iline >> g;
		iline >> b;
		i = Color(r, g, b);
	}
	if (height[0] > height[1]) {
		{
			int tmp = height[0];
			height[0] = height[1];
			height[1] = tmp;
		}
		{
			Color tmp = color[0];
			color[0] = color[1];
			color[1] = tmp;
		}
	}
	iline >> std::ws;
	if (iline.fail() || !iline.eof()) {
		std::cerr << filename << ":" << linenr << ": bad line in heightmap colors file (" << line << ")" << std::endl;
		return;
	}
	m_heightMapColors.push_back(HeightMapColor(height[0], color[0], height[1], color[1]));

}

void HeightMapNodesFileParser::parseLine(const std::string & line, std::string name, std::istringstream & iline, int linenr, const std::string & filename)
{
	if (name == "-") {
		iline >> std::ws >> name >> std::ws;
		m_nodeColors.erase(name);
	}
	else {
		m_nodeColors[name] = ColorEntry(0, 0, 0, 255, 1, 0);		// Dummy entry - but must not be transparent
	}
	// Don't care if not at eof (== really eol). We might be reading a colors.txt file...
	if (iline.fail()) {
		std::cerr << filename << ":" << linenr << ": bad line in heightmap nodes file (" << line << ")" << std::endl;
		return;
	}
}
