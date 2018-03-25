/*
 * =====================================================================
 *        Version:  1.0
 *        Created:  01.09.2012 14:38:05
 *         Author:  Miroslav Bendík
 *        Company:  LinuxOS.sk
 * =====================================================================
 */

#include <dirent.h>
//#include <experimental/filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include "PlayerAttributes.h"

using namespace std;
//namespace fs = std::experimental::filesystem::v1;

PlayerAttributes::PlayerAttributes(const std::string &sourceDirectory)
{
	string playersPath = sourceDirectory + "players";
	DIR *dir;
	dir = opendir(playersPath.c_str());
	if (dir == NULL) {
		return;
	}

	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.') {
			continue;
		}

		string path = playersPath + '/' + ent->d_name;

		ifstream in;
		in.open(path.c_str(), ifstream::in);
		string buffer;
		string name;
		string position;
		while (getline(in, buffer)) {
			if (buffer.find("name = ") == 0) {
				name = buffer.substr(7);
			}
			else if (buffer.find("position = ") == 0) {
				position = buffer.substr(12, buffer.length() - 13);
			}
		}
		char comma;
		Player player;
		istringstream positionStream(position, istringstream::in);
		positionStream >> player.x;
		positionStream >> comma;
		positionStream >> player.y;
		positionStream >> comma;
		positionStream >> player.z;
		player.name = name;

		m_players.push_back(player);
	}
	closedir(dir);
	//for (auto& dirEntry : fs::directory_iterator(playersPath)) {
	//	cout << dirEntry << std::endl;
	//	//dirEntry.path().filename();

	//	ifstream in;
	//	in.open(dirEntry.path().string(), ifstream::in);
	//	string buffer;
	//	string name;
	//	string position;
	//	while (getline(in, buffer)) {
	//		if (buffer.find("name = ") == 0) {
	//			name = buffer.substr(7);
	//		}
	//		else if (buffer.find("position = ") == 0) {
	//			position = buffer.substr(12, buffer.length() - 13);
	//		}
	//	}
	//	char comma;
	//	Player player;
	//	istringstream positionStream(position, istringstream::in);
	//	positionStream >> player.x;
	//	positionStream >> comma;
	//	positionStream >> player.y;
	//	positionStream >> comma;
	//	positionStream >> player.z;
	//	player.name = name;

	//	m_players.push_back(player);
	//}
}

PlayerAttributes::Players::iterator PlayerAttributes::begin()
{
	return m_players.begin();
}

PlayerAttributes::Players::iterator PlayerAttributes::end()
{
	return m_players.end();
}

