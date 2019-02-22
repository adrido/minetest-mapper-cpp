
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdexcept>
#include <sstream>
#include <fstream>

class Settings {
public:
	Settings(const std::string &filename, const std::string &msgName = "");
	~Settings(void);
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

#endif // _SETTINGS_H_
