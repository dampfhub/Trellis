#include "util.h"

std::string Util::PathBaseName(std::string const &path) {
    return path.substr(path.find_last_of("/\\") + 1);
}

bool Util::IsPng(std::string file) {
    return (file.substr(file.find_last_of(".") + 1) == "png");
}

template<>
std::string Util::deserialize<std::string>(const std::vector<std::byte> &bytes) {
    using std::byte;
    const char *ptr = reinterpret_cast<const char *>(bytes.data());
    return std::string(ptr);
}

std::vector<std::byte> Util::serialize(const std::string &str) {
    using std::byte;
    size_t s = str.size() + 1;
    const char *c_str = str.c_str();
    const byte *begin = reinterpret_cast<const byte *>(c_str);
    const byte *end = begin + s;
    std::vector<byte> bytes(begin, end);
    return bytes;
}
