/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  22.08.2012 15:15:54
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include "TileGenerator.h"

using namespace std;

#define OPT_SQLITE_CACHEWORLDROW	0x81

void usage()
{
	const char *usage_text = "minetestmapper [options]\n"
			"  -i/--input <world_path>\n"
			"  -o/--output <output_image.png>\n"
			"  --bgcolor <color>\n"
			"  --scalecolor <color>\n"
			"  --playercolor <color>\n"
			"  --origincolor <color>\n"
			"  --tilebordercolor <color>\n"
			"  --drawscale\n"
			"  --drawplayers\n"
			"  --draworigin\n"
			"  --drawalpha\n"
			"  --noshading\n"
			"  --min-y <y>\n"
			"  --max-y <y>\n"
			"  --backend <sqlite3/leveldb>\n"
			"  --geometry x:y+w+h\n"
		        "  --forcegeometry\n"
			"  --sqlite-cacheworldrow\n"
			"  --tiles <tilesize>[+<border>]\n"
			"  --tileorigin x:y|center-world|center-map\n"
			"  --verbose\n"
			"Color format: '#000000'\n";
	std::cout << usage_text;
}

int main(int argc, char *argv[])
{
	static struct option long_options[] =
	{
		{"help", no_argument, 0, 'h'},
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"bgcolor", required_argument, 0, 'b'},
		{"scalecolor", required_argument, 0, 's'},
		{"origincolor", required_argument, 0, 'r'},
		{"playercolor", required_argument, 0, 'p'},
		{"draworigin", no_argument, 0, 'R'},
		{"drawplayers", no_argument, 0, 'P'},
		{"drawscale", no_argument, 0, 'S'},
		{"drawalpha", no_argument, 0, 'e'},
		{"noshading", no_argument, 0, 'H'},
		{"geometry", required_argument, 0, 'g'},
		{"forcegeometry", no_argument, 0, 'G'},
		{"min-y", required_argument, 0, 'a'},
		{"max-y", required_argument, 0, 'c'},
		{"backend", required_argument, 0, 'd'},
		{"sqlite-cacheworldrow", no_argument, 0, OPT_SQLITE_CACHEWORLDROW},
		{"tiles", required_argument, 0, 't'},
		{"tileorigin", required_argument, 0, 'T'},
		{"tilebordercolor", required_argument, 0, 'B'},
		{"verbose", no_argument, 0, 'v'},
	};

	string input;
	string output;

	TileGenerator generator;
	generator.parseColorsFile("colors.txt");
	int option_index = 0;
	int c = 0;
	while (1) {
		c = getopt_long(argc, argv, "hi:o:", long_options, &option_index);
		if (c == -1) {
			if (input.empty() || output.empty()) {
				usage();
				return 0;
			}
			break;
		}
		switch (c) {
			case 'h':
				usage();
				return 0;
				break;
			case 'i':
				input = optarg;
				break;
			case 'o':
				output = optarg;
				break;
			case 'b':
				generator.setBgColor(optarg);
				break;
			case 's':
				generator.setScaleColor(optarg);
				break;
			case 'r':
				generator.setOriginColor(optarg);
				break;
			case 'p':
				generator.setPlayerColor(optarg);
				break;
			case 'B':
				generator.setTileBorderColor(optarg);
				break;
			case 'R':
				generator.setDrawOrigin(true);
				break;
			case 'P':
				generator.setDrawPlayers(true);
				break;
			case 'S':
				generator.setDrawScale(true);
				break;
			case 'v':
				generator.verboseCoordinates = true;
				generator.verboseStatistics = true;
				break;
			case 'e':
				generator.setDrawAlpha(true);
				break;
			case 'H':
				generator.setShading(false);
				break;
			case OPT_SQLITE_CACHEWORLDROW:
				generator.setSqliteCacheWorldRow(true);
				break;
			case 'a': {
					istringstream iss;
					iss.str(optarg);
					int miny;
					iss >> miny;
					generator.setMinY(miny);
				}
				break;
			case 'c': {
					istringstream iss;
					iss.str(optarg);
					int maxy;
					iss >> maxy;
					generator.setMaxY(maxy);
				}
				break;
			case 't': {
					istringstream tilesize;
					tilesize.str(optarg);
					int size, border;
					char c;
					tilesize >> size;
					if (tilesize.fail() || size<1) {
						usage();
						exit(1);
					}
					generator.setTileSize(size, size);
					tilesize >> c >> border;
					if (!tilesize.fail()) {
						if (c != '+' || border < 1) {
							usage();
							exit(1);
						}
						generator.setTileBorderSize(border);
					}
				}
				break;
			case 'T': {
					istringstream origin;
					origin.str(optarg);
					int x, y;
					char c;
					origin >> x >> c >> y;
					if (origin.fail() || c != ':') {
						if (string("center-world") == optarg)
							generator.setTileOrigin(TILECENTER_IS_WORLDCENTER, TILECENTER_IS_WORLDCENTER);
						else if (string("center-map") == optarg)
							generator.setTileOrigin(TILECENTER_IS_MAPCENTER, TILECENTER_IS_MAPCENTER);
						else {
							usage();
							exit(1);
						}
					}
					else {
						generator.setTileOrigin(x, y);
					}
				}
				break;
			case 'g': {
					istringstream geometry;
					geometry.str(optarg);
					int x, y, w, h;
					char c;
					geometry >> x >> c >> y >> w >> h;
					if (geometry.fail() || c != ':' || w < 1 || h < 1) {
						usage();
						exit(1);
					}
					generator.setGeometry(x, y, w, h);
				}
				break;
			case 'G':
				generator.setForceGeom(true);
				break;
			case 'd':
				generator.setBackend(optarg);
				break;
			default:
				exit(1);
		}
	}
	try {
		generator.generate(input, output);
	} catch(std::runtime_error e) {
		std::cout<<"Exception: "<<e.what()<<std::endl;
		return 1;
	}
	return 0;
}
