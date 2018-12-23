#pragma once

#include <list>
#include <string>
#include <sqlite3.h>

struct Player
{
	std::string name;
	double x;
	double y;
	double z;
};

class PlayerAttributes
{
public:
	typedef std::list<Player> Players;

	PlayerAttributes(const std::string &sourceDirectory);

	Players::iterator begin();
	Players::iterator end();

private:
	Players m_players;
	void extractPlayer(const std::string &path);

	bool extractPlayersSqlite(const std::string &playersPath); //returns false if database could not found
	sqlite3_stmt *preparedStatement = nullptr;
	sqlite3 *db = nullptr;
}; 

