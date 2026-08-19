#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <io.h>
#include <windows.h>
#include <lmcons.h>
#include <ws2tcpip.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

inline std::string fixchar(std::string_view name) {
	std::string ret;
	ret.reserve(name.length());
	for (size_t i = 0; i < name.length(); i++) {
		char c = name[i];
		if (c == '`') {
			i++;
			continue;
		}
		if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
			ret.push_back(c);
		}
	}
	return ret;
}