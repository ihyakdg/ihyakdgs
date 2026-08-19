#pragma once
#include <fstream>
#include <vector>
#include <cstdint>
#ifdef _WIN32
#elif defined(__linux__)
#define BYTE unsigned char
#define DWORD unsigned int
#define __int64 int64_t
#define __int32 int32_t
#define __int16 int16_t
#define __int8 int8_t
#endif
namespace ItemDataUtils {
    inline BYTE* linux_item_data = nullptr;
    inline BYTE* ios_item_data = nullptr;
    inline int64_t linux_item_data_size = 0;
    inline int64_t ios_item_data_size = 0;
    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_RED = "\033[31m";
    const std::string COLOR_GREEN = "\033[32m";
    inline void loadLinux() {
        struct tm newtime;
        time_t now = time(0);
#ifdef _WIN32
        localtime_s(&newtime, &now);
#elif defined(__linux__)
        localtime_r(&now, &newtime);
#endif
        std::ifstream file("database/itemdata_linux.dat", std::ios::binary | std::ios::ate);
        __int64 size = file.tellg();
        if (size == -1) {
            file.close();
            file.open("database/items.dat", std::ios::binary | std::ios::ate);
            size = file.tellg();
        }
        if (size == -1) {
            std::cout << COLOR_RESET << "[" << newtime.tm_hour << ":" << newtime.tm_min << "] " << COLOR_RED << " [ERROR]: " << COLOR_RESET << "Failed to read real linux data at 'database'" << std::endl;
#ifdef _WIN32
            system("pause");
#endif
            exit(EXIT_FAILURE);
        }
        linux_item_data_size = size;
        linux_item_data = new BYTE[size];
        file.seekg(0, std::ios::beg);
        file.read((char *)(linux_item_data), size);
        file.close();
    }
    inline void loadIOS() {
        struct tm newtime;
        time_t now = time(0);
#ifdef _WIN32
        localtime_s(&newtime, &now);
#elif defined(__linux__)
        localtime_r(&now, &newtime);
#endif
        std::ifstream file("database/itemdata_ios.dat", std::ios::binary | std::ios::ate);
        __int64 size = file.tellg();
        if (size == -1) {
            file.close();
            file.open("database/items.dat", std::ios::binary | std::ios::ate);
            size = file.tellg();
        }
        if (size == -1) {
            std::cout << COLOR_RESET << "[" << newtime.tm_hour << ":" << newtime.tm_min << "] " << COLOR_RED << " [ERROR]: " << COLOR_RESET << "Failed to read real ios data at 'database'" << std::endl;
#ifdef _WIN32
            system("pause");
#endif
            exit(EXIT_FAILURE);
        }
        ios_item_data_size = size;
        ios_item_data = new BYTE[size];
        file.seekg(0, std::ios::beg);
        file.read((char *)(ios_item_data), size);
        file.close();
    }
    inline BYTE* getLinuxPacket() {
        return linux_item_data;
    }
    inline int64_t getLinuxPacketSize() {
        return linux_item_data_size;
    }
    inline BYTE* getIOSPacket() {
        return ios_item_data;
    }
    inline int64_t getIOSPacketSize() {
        return ios_item_data_size;
    }
};