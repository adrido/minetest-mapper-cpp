
#include <iostream>
#include "Settings.h"

static std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, (end-start+1));
}

Settings::Settings(const std::string &filename, const std::string &msgName)
	:  m_filename(filename),
	   m_messageName(msgName)
{
	if (m_messageName == "")
		m_messageName = m_filename;
	m_file.open(m_filename.c_str(), std::ifstream::in);
	if (!m_file.is_open())
		throw std::runtime_error(std::string("Failed to open file '") + m_messageName + "'");
}

Settings::~Settings(void)
{
	m_file.close();
}

std::string Settings::get(const std::string &key)
{
	std::string value;
	if (getGeneric(key, &value))
		return value;
	else
		throw std::runtime_error(std::string("Failed to read key '") + key + "' from file '" + m_messageName + "'");
}

std::string Settings::get(const std::string &key, const std::string &defaultValue)
{
	std::string value;
	if (getGeneric(key, &value))
		return value;
	else
		return defaultValue;
}

bool Settings::getGeneric(const std::string &key, std::string *pvalue)
{
	std::string line;
	int linenr = 0;
	bool found = false;
	m_file.clear();
	m_file.seekg(0);
	for (std::getline(m_file,line); m_file.good(); std::getline(m_file,line)) {
		linenr++;
		size_t keylen = line.find_first_of('=');
		if (keylen == std::string::npos) {
			std::cerr << "Error parsing config line at " << m_filename << ":" << linenr << ": expected: <name> = <value> ('=' not found)";
			continue;
		}
		if (trim(line.substr(0, keylen)) != key)
			continue;
		found = true;
		if (pvalue)
			*pvalue = trim(line.substr(keylen + 1));
	}
	return found;
}

