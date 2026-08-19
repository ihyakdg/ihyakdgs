#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <thread>
#include <functional>

#pragma comment(lib, "winhttp.lib")
std::string UrlEncode(const std::string& value) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;
    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        }
        else {
            encoded << '%' << std::setw(2) << std::uppercase << (int)static_cast<unsigned char>(c);
        }
    }
    return encoded.str();
}
std::wstring StringToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}
class Chat_Ai {
public:
    static std::string Response(const std::string& Api, const std::string& prompt, const std::string& userName, const std::string& rid) {
        std::wstring url = std::format(
            L"/ai-gtps?apikey={}&playerSpecialID={}&playerName={}&ask={}",
            StringToWString(Api),
            StringToWString(UrlEncode(rid)),
            StringToWString(UrlEncode(userName)),
            StringToWString(UrlEncode(prompt))
        );
        HINTERNET hSession = WinHttpOpen(L"A WinHTTP Example Program/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) {
            std::cerr << "Failed to open WinHTTP session.\n";
            return "Failed to open session!";
        }
        HINTERNET hConnect = WinHttpConnect(hSession, L"api.akk.bar", NULL, 0);
        if (!hConnect) {
            std::cerr << "Failed to connect.\n";
            WinHttpCloseHandle(hSession);
            return "Failed to connect!";
        }
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", url.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        if (!hRequest) {
            std::cerr << "Failed to open request.\n";
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "Failed to open request!";
        }
        BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        if (!bResults) {
            std::cerr << "Failed to send request.\n";
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "Failed to send request!";
        }
        bResults = WinHttpReceiveResponse(hRequest, NULL);
        if (!bResults) {
            std::cerr << "Failed to receive response.\n";
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "Failed to receive response!";
        }
        DWORD dwSize = 0;
        std::string response;
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                std::cerr << "Error in WinHttpQueryDataAvailable.\n";
                break;
            }
            char* buffer = new char[dwSize + 1];
            if (!buffer) {
                std::cerr << "Out of memory.\n";
                break;
            }
            ZeroMemory(buffer, dwSize + 1);
            DWORD dwDownloaded = 0;
            if (!WinHttpReadData(hRequest, (LPVOID)buffer, dwSize, &dwDownloaded)) {
                std::cerr << "Error in WinHttpReadData.\n";
                delete[] buffer;
                break;
            }
            response.append(buffer, dwDownloaded);
            delete[] buffer;
        }
        while (dwSize > 0);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        try {
            auto jsonData = nlohmann::json::parse(response);
            auto data = jsonData["data"];
            return data["response"];
        }
        catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            return "Failed to parse JSON response!";
        }
    }
};
