#ifndef UTIL_H
#define UTIL_H

#include <string>

namespace Util {
	std::string PathBaseName(std::string const & path);
	bool IsPng(std::string file);
}

#endif