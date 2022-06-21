#ifndef OCTOON_BASE64_H_
#define OCTOON_BASE64_H_

#include <string>

namespace octoon
{
	std::string base64_encode(const unsigned char* data, std::size_t len);
	std::string base64_encode(const std::string& s);
	std::string base64_encode(const std::u8string& s);

	std::string base64_decode(const std::string& s);
}

#endif