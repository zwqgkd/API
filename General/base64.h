#pragma once
#include <string>
#include <vector>
typedef unsigned char BYTE;

std::string base64_encode(BYTE const *buf, unsigned int bufLen);
std::vector<BYTE> base64_decode(std::string const &);