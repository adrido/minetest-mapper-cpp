#pragma once

#include <list>
#include <string>

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
}; 

