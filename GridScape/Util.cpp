#include "util.h"

std::string Util::PathBaseName(std::string const & path) {
	return path.substr(path.find_last_of("/\\") + 1);
}

bool Util::IsPng(std::string file) {
	return (file.substr(file.find_last_of(".") + 1) == "png");
}