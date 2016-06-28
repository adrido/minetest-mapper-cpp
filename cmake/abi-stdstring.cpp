#include <string>
#include <leveldb/status.h>
int main(void) {
	leveldb::Status status;
	std::string str = status.ToString();
	return 0;
}
