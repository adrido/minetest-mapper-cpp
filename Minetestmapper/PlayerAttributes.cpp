#if __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#if __has_include(<filesystem>)
#define HAVE_FILESYSTEM
#endif
#endif

#ifdef HAVE_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <dirent.h>
#endif // HAVE_FILESYSTEM

#include <fstream>
#include <sstream>
#include <iostream>
#include "PlayerAttributes.h"

using namespace std;


PlayerAttributes::PlayerAttributes(const std::string &sourceDirectory)
{
	const string playersPath = sourceDirectory + "players";

#ifdef HAVE_FILESYSTEM
	for (const auto &dirEntry : fs::directory_iterator(playersPath)) {
		cout << dirEntry << std::endl;
		//dirEntry.path().filename();

		extractPlayer(dirEntry.path().string());
	}
#else
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

		const string path = playersPath + '/' + ent->d_name;

		extractPlayer(path);
	}
	closedir(dir);
#endif // HAVE_FILESYSTEM

	
}

void PlayerAttributes::extractPlayer(const std::string &path)
{
	ifstream in;
	in.open(path, ifstream::in);
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

PlayerAttributes::Players::iterator PlayerAttributes::begin()
{
	return m_players.begin();
}

PlayerAttributes::Players::iterator PlayerAttributes::end()
{
	return m_players.end();
}

