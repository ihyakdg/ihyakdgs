#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <array>
#include <sstream>
#include <cctype>
#include <format>
#include "NopySource/Handle/WorldInfo.h"
#include "NopySource/Handle/PlayerInfo.h"
#include "include/base64.hpp"
#include <array>
#include <algorithm>
#include <cctype>


static std::string base64_decode_safe(const std::string& in) {
    // urlsafe base64 -> normal base64
    static const auto T = []() {
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        std::array<int, 256> table{};
        table.fill(-1);
        for (int i = 0; i < 64; i++) table[(unsigned char)base64_chars[i]] = i;
        return table;
    }();

    std::string out;
    out.reserve(in.size());
    int val = 0, valb = -8;

    for (char ch : in) {
        unsigned char c = (unsigned char)ch;
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
        if (c == '=') break;
        int d = T[c];
        if (d == -1) continue;
        val = (val << 6) + d;
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

static int hexval(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return -1;
}

static std::string url_decode(const std::string& s) {
    std::string o;
    o.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int hi = hexval(s[i + 1]);
            int lo = hexval(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                o.push_back(char((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        o.push_back(s[i]);
    }
    return o;
}

bool isValidBase64(const std::string& input) {
    if (input.length() % 4 != 0) return false;
    for (char c : input) {
        if (!isalnum(c) && c != '+' && c != '/' && c != '=') {
            return false;
        }
    }
    return true;
}

namespace LoginHandle {
    class Parser {
        std::vector<std::pair<std::string, std::string>> contents;
    public:
        Parser(std::string const& str) {
            size_t start = 0;
            while (start < str.length()) {
                size_t end = str.find('\n', start);
                if (end == std::string::npos) end = str.length();
                size_t lineLen = (end > start && str[end - 1] == '\r') ? end - 1 - start : end - start;
                if (lineLen > 0) {
                    size_t pipe = str.find('|', start);
                    if (pipe != std::string::npos && pipe < start + lineLen) {
                        contents.emplace_back(str.substr(start, pipe - start), str.substr(pipe + 1, start + lineLen - (pipe + 1)));
                    }
                }
                start = end + 1;
            }
        }
        std::pair<std::string, std::string> get_value_starts_with(std::string const& key) {
            for (auto const& [contentKey, value] : contents) {
                if (contentKey.starts_with(key)) return { contentKey, value };
            }
            return {};
        }
        std::string get_value(std::string const& key) {
            for (auto const& [contentKey, value] : contents) {
                if (contentKey == key) return value;
            }
            return "";
        }
    };
    class HandleReturnParser {
        std::vector<std::pair<std::string, std::string>> contents;
    public:
        HandleReturnParser(std::string const& str) {
            size_t start = 0;
            while (start < str.length()) {
                size_t end = str.find('\n', start);
                if (end == std::string::npos) end = str.length();
                size_t lineLen = (end > start && str[end - 1] == '\r') ? end - 1 - start : end - start;
                if (lineLen > 0) {
                    size_t underscore = str.find('_', start);
                    if (underscore != std::string::npos && underscore < start + lineLen) {
                        contents.emplace_back(str.substr(start, underscore - start), str.substr(underscore + 1, start + lineLen - (underscore + 1)));
                    }
                }
                start = end + 1;
            }
        }
        std::pair<std::string, std::string> get_value_starts_with(std::string const& key) {
            for (auto const& [contentKey, value] : contents) {
                if (contentKey.starts_with(key)) return { contentKey, value };
            }
            return {};
        }
        std::string get_value(std::string const& key) {
            for (auto const& [contentKey, value] : contents) {
                if (contentKey == key) return value;
            }
            return "";
        }
    };

    std::string getValue(const std::string& str, const std::string& key) {
        std::string searchKey = key + "=";
        size_t startPos = str.find(searchKey);
        if (startPos != std::string::npos) {
            startPos += searchKey.length();
            size_t endPos = str.find('&', startPos);
            if (endPos == std::string::npos) {
                endPos = str.length();
            }
            return str.substr(startPos, endPos - startPos);
        }
        return "";
    }

    std::string GetContents(ENetPeer* peer, std::string const& cch) {
        std::cout << "[INFO][LOGIN_DEBUG] Received CCH from peer " << peer->connectID << ". CCH data: [" << cch << "]" << std::endl;

        std::string content = "";
        std::vector<std::string> result_token = explode("|", cch);

        if (result_token.size() < 3) {
            std::cerr << "[ERROR][LOGIN_ERROR] Invalid CCH format: Not enough parts separated by '|'. CCH: [" << cch << "]" << std::endl;
            return "ERROR_INVALID_CCH_FORMAT";
        }

        std::vector<std::string> line_parts = explode("\n", result_token[2]);
        if (line_parts.empty()) {
            std::cerr << "[ERROR][LOGIN_ERROR] Missing token line in CCH. result_token[2] was empty/malformed. CCH: [" << cch << "]" << std::endl;
            return "ERROR_MISSING_TOKEN_LINE";
        }

        std::string token = line_parts[0];
        std::cout << "[INFO][LOGIN_DEBUG] Extracted Base64 token from CCH: [" << token << "]" << std::endl;

        if (!isValidBase64(token)) {
            std::cerr << "[ERROR][LOGIN_ERROR] Received invalid Base64 token (isValidBase64 returned false). Input: [" << token << "]" << std::endl;
            return "ERROR_INVALID_BASE64";
        }

        std::string decoded_token;
        try {
            decoded_token = base64::from_base64(token);
            std::cout << "[INFO][LOGIN_DEBUG] Successfully decoded token. Content: [" << decoded_token << "]" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR][LOGIN_ERROR] Failed to Base64 decode token (exception caught). Error: " << e.what() << ". Token: [" << token << "]" << std::endl;
            return "ERROR_DECODE_FAILED";
        }
        catch (...) {
            std::cerr << "[ERROR][LOGIN_ERROR] Failed to Base64 decode token (unknown exception). Token: [" << token << "]" << std::endl;
            return "ERROR_DECODE_FAILED_UNKNOWN";
        }

        std::string user = getValue(decoded_token, "growId");
        std::string pass = getValue(decoded_token, "password");
        std::string ltoken = getValue(decoded_token, "_token");

        std::cout << "[INFO][LOGIN_DEBUG] Extracted GrowID: [" << user << "]" << std::endl;
        std::cout << "[INFO][LOGIN_DEBUG] Extracted Password: [" << pass << "]" << std::endl;
        std::cout << "[INFO][LOGIN_DEBUG] Extracted _token (ltoken): [" << ltoken << "]" << std::endl;

        if (user.empty() && pass.empty()) {
            pInfo(peer)->is_guest = true;
            std::cout << "[WARNING][LOGIN_INFO] GrowID and Password are empty. Treating as guest login for peer " << peer->connectID << "." << std::endl;
            return "GUEST_LOGIN_HANDLED";
        }

        if (ltoken.empty()) {
            std::cerr << "[ERROR][LOGIN_ERROR] _token (ltoken) is empty. Cannot proceed with login for user: [" << user << "]" << std::endl;
            return "ERROR_LTOKEN_EMPTY";
        }

        // -----------------------------------------------------------
        // BAGIAN INI HARUS BERISI LOGIKA AUTENTIKASI AKUN ANDA!
        // Misalnya, membandingkan 'user' dan 'pass' dengan database.
        // -----------------------------------------------------------

        /*
        // Contoh Pseudo-code untuk Autentikasi (Ganti dengan implementasi database Anda)
        if (authenticateUserAgainstDatabase(user, pass)) {
            std::cout << "[INFO][LOGIN_SUCCESS] Authentication SUCCESS for GrowID: [" << user << "]! Proceeding to player initialization." << std::endl;
            // ... Tambahkan kode untuk memuat data pemain, mengirim paket awal dunia, dll.
            content.append("action|logon\n");
            content.append("server_version|2.99\n");
            return content;
        } else {
            std::cerr << "[WARNING][LOGIN_FAIL] Authentication FAILED for GrowID: [" << user << "]. Invalid credentials." << std::endl;
            return "ERROR_AUTHENTICATION_FAILED";
        }
        */

        std::cout << "[WARNING][LOGIN_INFO] GrowID: [" << user << "] and Password: [" << pass << "] parsed. ACTUAL AUTHENTICATION LOGIC IS MISSING HERE!" << std::endl;

        content.append("tankIDName|" + user + "|\n");
        content.append("tankIDPass|" + pass + "|\n");
        content.append(ltoken + "|\n");

        return content;
    }
}