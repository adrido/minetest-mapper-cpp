#pragma once

#include <fstream>
#include <sstream>
#include <stdexcept>

class Settings
{
public:
	Settings(const std::string &filename, const std::string &msgName = "");
	~Settings() = default;
	std::string get(const std::string &key);
	std::string get(const std::string &key, const std::string &defaultValue);
	bool check(const std::string &key) { return getGeneric(key); }
	bool check(const std::string &key, std::string &value) { return getGeneric(key, &value); }

private:
	std::string m_filename;
	std::string m_messageName;
	std::ifstream m_file;

	bool getGeneric(const std::string &key, std::string *pvalue = nullptr);
};
