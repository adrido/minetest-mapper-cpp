
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdexcept>
#include <sstream>
#include <fstream>

class Settings {
public:
	Settings(std::string filename, std::string msgName = "");
	~Settings(void);
	std::string get(std::string key);
	std::string get(std::string key, std::string defaultValue);
	bool check(std::string key) { return getGeneric(key); }
	bool check(std::string key, std::string &value) { return getGeneric(key, &value); }
private:
	std::string m_filename;
	std::string m_messageName;
	std::ifstream m_file;

	bool getGeneric(std::string key, std::string *pvalue = NULL);
};

#endif // _SETTINGS_H_
