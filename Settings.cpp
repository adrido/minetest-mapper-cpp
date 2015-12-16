
#include "Settings.h"

static std::string trim(const std::string s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, (end-start+1));
}

Settings::Settings(std::string filename, std::string msgName)
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

std::string Settings::get(std::string key)
{
	std::string value;
	if (getGeneric(key, &value))
		return value;
	else
		throw std::runtime_error(std::string("Failed to read key '") + key + "' from file '" + m_messageName + "'");
}

std::string Settings::get(std::string key, std::string defaultValue)
{
	std::string value;
	if (getGeneric(key, &value))
		return value;
	else
		return defaultValue;
}

bool Settings::getGeneric(std::string key, std::string *pvalue)
{
	std::string line;
	int linenr = 0;
	bool found = false;
	m_file.clear();
	m_file.seekg(0);
	for (std::getline(m_file,line); m_file.good(); std::getline(m_file,line)) {
		linenr++;
		std::istringstream iline;
		iline.str(line);
		iline >> std::skipws;
		std::string variable;
		std::string eq;
		iline >> variable;
		if (variable != key)
			continue;
		found = true;
		iline >> eq;
		if (m_file.fail() || eq != "=") {
			std::ostringstream oss;
			oss << "Error parsing '" << key << "' in file " << m_messageName << " at line " << linenr << " (missing '=')";
			throw std::runtime_error(oss.str());
		}
		if (pvalue) {
			std::string value;
			iline >> value;
			if (m_file.fail()) {
				std::ostringstream oss;
				oss << "Error parsing value for '" << key << "' in file " << m_messageName << " at line " << linenr;
				throw std::runtime_error(oss.str());
			}
			*pvalue = trim(value);
		}
	}
	return found;
}

