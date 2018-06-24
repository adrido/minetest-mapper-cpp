#include "Mapper.h"
#include <string>

int main(int argc, char *argv[])
{
	std::string executableName;
	std::string executablePath;
	if (argc) {
		std::string argv0 = argv[0];
		size_t pos = argv0.find_last_of(PATH_SEPARATOR);
		if (pos == std::string::npos) {
			if (!argv0.empty())
				executableName = argv0;
			else
				executableName = DEFAULT_PROGRAM_NAME;
			executablePath = "";

		}
		else {
			executableName = argv0.substr(pos + 1);
			executablePath = argv0.substr(0, pos);
		}
	}

	Mapper m(executablePath, executableName);
	return m.start(argc, argv);

}
