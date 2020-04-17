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
#include <stdexcept>
#include <iostream>
#include <ostream>
#include "PlayerAttributes.h"

using namespace std;


PlayerAttributes::PlayerAttributes(const std::string &sourceDirectory)
{
	const string playersPath = sourceDirectory + "players";

	// if players.sqlite, could not be opened, then process the playerfiles
	if (!extractPlayersSqlite(playersPath)) {
#ifdef HAVE_FILESYSTEM
		for (const auto &dirEntry : fs::directory_iterator(playersPath)) {
			cout << dirEntry.path().string() << std::endl;
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
	
}

bool PlayerAttributes::extractPlayersSqlite(const std::string &playersPath)
{
	const string playersDB = playersPath + ".sqlite";
	const string statement = "SELECT name, posX, posY, posZ FROM player";

	int sqlite_state = sqlite3_open_v2(playersDB.c_str(), &db, SQLITE_OPEN_READONLY | SQLITE_OPEN_PRIVATECACHE, nullptr);
	if (sqlite_state == SQLITE_CANTOPEN) {
		return false;
	}

	if (sqlite_state != SQLITE_OK) {
		throw std::runtime_error(std::string(sqlite3_errmsg(db)) + ", Database file: " + playersDB);
	}
	if (SQLITE_OK != sqlite3_prepare_v2(db, statement.c_str(), -1, &preparedStatement, nullptr)) {
		throw std::runtime_error("Failed to prepare SQL statement (dataVersionStatement)");
	}

	while (sqlite3_step(preparedStatement) == SQLITE_ROW) {
		Player player;
		player.name = reinterpret_cast<const char*>(sqlite3_column_text(preparedStatement, 0));
		player.x = sqlite3_column_double(preparedStatement, 1);
		player.y = sqlite3_column_double(preparedStatement, 2);
		player.z = sqlite3_column_double(preparedStatement, 3);
		m_players.push_back(player);
	};
	sqlite3_finalize(preparedStatement);
	sqlite3_close(db);
	return true;
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

