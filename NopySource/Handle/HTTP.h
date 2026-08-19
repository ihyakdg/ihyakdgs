#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp> // Include for JSON handling
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include "include/nlohmann/json.hpp"

class HTTPServer {
public:
    HTTPServer() = default;
    ~HTTPServer() {
        if (m_pServer) {
            Stop();
        }
    }

    bool Listen();
    void Stop();
    std::thread& GetThread();

private:
    std::string SERVER_IP;
    int SERVER_PORT;
    std::string CURRENT_VERSION;

    struct ClientInfo {
        std::chrono::steady_clock::time_point lastRequest;
        int requestCount;
        int burstCount;
        std::chrono::steady_clock::time_point burstStart;
        bool isProxy;
        float cpuUsage;
    };

    std::unique_ptr<httplib::Server> m_pServer;
    // std::thread m_serviceThread;
    std::unordered_set<std::string> m_blacklist;
    std::unordered_map<std::string, ClientInfo> m_rateLimit;
    std::mutex m_blacklistMutex;
    std::mutex m_rateLimitMutex;

    std::atomic<float> m_currentCpuUsage{ 0.0f };
    std::chrono::steady_clock::time_point m_lastCpuCheck;
    static constexpr int BURST_THRESHOLD = 20;
    static constexpr std::chrono::seconds BURST_WINDOW{ 1 };
    static constexpr float CPU_THRESHOLD_NORMAL = 2.1f;
    static constexpr float CPU_THRESHOLD_FLOOD = 3.0f;
    static constexpr float CPU_THRESHOLD_PROXY = 10.0f;

    void LoadConfig();
    void LoadBlacklist();
    bool IsBlacklisted(const std::string& ip);
    bool IsRateLimited(const std::string& ip);
    void ServicePoll();
    std::string GenerateServerResponse(const std::string& clientVersion) const;
    void HandlePost(const httplib::Request& req, httplib::Response& res);
    void LogRequest(const std::string& ip, const std::string& path, const std::string& response);

    float GetCPUUsage();
    bool IsFloodAttack(const std::string& ip);
    bool IsProxyRequest(const httplib::Request& req);
    void UpdateClientMetrics(const std::string& ip, const httplib::Request& req);
    void MonitorSystemLoad();
};

using namespace std;
using json = nlohmann::json;

bool validateLicense(const std::string& domain, const std::string& licenseKey) {
    httplib::Client client(domain.c_str());
    std::string endpoint = "/apikeys/" + licenseKey;

    auto response = client.Get(endpoint.c_str());

    if (!response || response->status != 200) {
        std::cerr << "Failed to validate license. HTTP Status: "
            << (response ? response->status : 0) << std::endl;
        return false;
    }

    try {
        json responseJson = json::parse(response->body);

        if (responseJson.contains("error")) {
            std::cerr << "License validation failed: " << responseJson["error"].get<std::string>() << std::endl;
            return false;
        }

        std::cout << "License is valid!" << std::endl;
        std::cout << "Expires at: " << responseJson["expire"].get<std::string>() << std::endl;

        if (responseJson.contains("whitelist")) {
            std::cout << "Whitelist: ";
            for (const auto& ip : responseJson["whitelist"]) {
                std::cout << ip.get<std::string>() << " ";
            }
            std::cout << std::endl;
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
        return false;
    }
}
bool writeToFile(const std::string& filename, const std::string& data) {
    std::ofstream out(filename, std::ios::out | std::ios::binary);
    if (!out.is_open()) {
        return false;
    }
    out.write(data.c_str(), data.size());
    out.close();
    return true;
}

bool readFromFile(const std::string& filename, std::string& data) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        return false;
    }
    in.seekg(0, std::ios::end);
    data.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&data[0], data.size());
    in.close();
    return true;
}

std::string getCurrentDateISO() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", now);
    return std::string(buf);
}

bool checkAccountValidity(const std::string& username, std::string& secretFileContent) {
    httplib::Client cli("https://privateserverid.my.id");
    auto res = cli.Get(("/account/" + username + ".json").c_str());

    if (!res || res->status != 200) {
        return false;
    }

    try {
        json jsonData = json::parse(res->body);
        std::string expirationDate = jsonData["expirationDate"];
        std::string currentDate = getCurrentDateISO();

        if (expirationDate <= currentDate) {
            return false;
        }

        std::string secretFileData = "username:" + username + "\nexpirationDate:" + expirationDate;
        if (!writeToFile("secret_data.txt", secretFileData)) {
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

bool isFileExpired(const std::string& secretFileData) {
    size_t pos = secretFileData.find("expirationDate:");
    if (pos == std::string::npos) {
        return true;
    }
    std::string expirationDate = secretFileData.substr(pos + 16);
    std::string currentDate = getCurrentDateISO();
    return expirationDate <= currentDate;
}

bool isUsernameStillValid(const std::string& username) {
    httplib::Client cli("https://privateserverid.my.id");
    auto res = cli.Get(("/account/" + username + ".json").c_str());

    if (!res || res->status != 200) {
        return false;
    }

    try {
        json jsonData = json::parse(res->body);
        std::string expirationDate = jsonData["expirationDate"];
        std::string currentDate = getCurrentDateISO();

        return expirationDate > currentDate;
    }
    catch (const std::exception& e) {
        return false;
    }
}

bool HTTPServer::Listen() {
    try {
        m_pServer = std::make_unique<httplib::Server>();
        int listenPort = 8888;
        if (!m_pServer->bind_to_port("0.0.0.0", listenPort)) {
            listenPort = 17096;
            if (!m_pServer->bind_to_port("0.0.0.0", listenPort)) {
                listenPort = 8080;
                if (!m_pServer->bind_to_port("0.0.0.0", listenPort)) {
                    return false;
                }
            }
        }

        LoadBlacklist();

        ServicePoll();

        std::thread([this]() {
            m_pServer->listen_after_bind();
        }).detach();

        std::thread([this]() {
            while (true) {
                MonitorSystemLoad();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }).detach();

        Logger::Info("HTTP", "Web Status API Server initialized on http://0.0.0.0:" + std::to_string(listenPort));
        return true;
    }
    catch (const std::exception& e) {
        Logger::Info("HTTP", std::string("HTTP Server failed: ") + e.what());
        return false;
    }
}

void HTTPServer::LoadBlacklist() {
    std::ifstream blacklistFile("database/blacklist.txt");
    std::string ip;

    while (std::getline(blacklistFile, ip)) {
        std::lock_guard<std::mutex> lock(m_blacklistMutex);
        m_blacklist.insert(ip);
    }
}
void HTTPServer::Stop() {
    if (m_pServer) {
        m_pServer->stop(); // Assuming `stop()` is the correct method to stop the server
        m_pServer.reset(); // Clean up the server instance
        std::cout << "HTTP Server stopped." << std::endl;
    }
}
void HTTPServer::LogRequest(const std::string& ip, const std::string& path, const std::string& response) {
    std::ofstream logFile("server_log.txt", std::ios_base::app);
    if (logFile.is_open()) {
        logFile << "[" << std::chrono::steady_clock::now().time_since_epoch().count() << "] "
            << "IP: " << ip << ", Path: " << path << ", Response: " << response << "\n";
        logFile.close();
    }
    else {
        std::cerr << "Failed to open log file." << std::endl;
    }
}

bool HTTPServer::IsBlacklisted(const std::string& ip) {
    std::lock_guard<std::mutex> lock(m_blacklistMutex);
    return m_blacklist.find(ip) != m_blacklist.end();
}

bool HTTPServer::IsRateLimited(const std::string& ip) {
    std::lock_guard<std::mutex> lock(m_rateLimitMutex);

    auto now = std::chrono::steady_clock::now();
    auto& client = m_rateLimit[ip];

    if (now - client.lastRequest > std::chrono::seconds(60)) {
        client.requestCount = 1;
        client.lastRequest = now;
        return false;
    }

    client.requestCount++;

    int rateLimit = 100;
    if (client.isProxy && m_currentCpuUsage > CPU_THRESHOLD_NORMAL) {
        rateLimit = 30;
    }
    else if (m_currentCpuUsage > CPU_THRESHOLD_FLOOD) {
        rateLimit = 50;
    }

    return client.requestCount > rateLimit;
}

float HTTPServer::GetCPUUsage() {
    static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

    std::ifstream stat_file("/proc/stat");
    std::string line;
    std::getline(stat_file, line);
    std::istringstream ss(line);

    std::string cpu;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, totalIrq;
    ss >> cpu >> totalUser >> totalUserLow >> totalSys >> totalIdle >> totalIrq;

    if (lastTotalUser == 0) {
        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;
        return 0.0f;
    }

    unsigned long long totalDelta = (totalUser - lastTotalUser) +
        (totalUserLow - lastTotalUserLow) +
        (totalSys - lastTotalSys);
    unsigned long long idleDelta = totalIdle - lastTotalIdle;

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    float cpuUsage = (totalDelta * 100.0f) / (totalDelta + idleDelta);
    return cpuUsage;
}

bool HTTPServer::IsFloodAttack(const std::string& ip) {
    std::lock_guard<std::mutex> lock(m_rateLimitMutex);
    auto& client = m_rateLimit[ip];
    auto now = std::chrono::steady_clock::now();

    if (now - client.burstStart > BURST_WINDOW) {
        client.burstCount = 1;
        client.burstStart = now;
        return false;
    }

    client.burstCount++;
    return client.burstCount > BURST_THRESHOLD;
}

bool HTTPServer::IsProxyRequest(const httplib::Request& req) {
    const auto& headers = req.headers;
    return headers.find("X-Forwarded-For") != headers.end() ||
        headers.find("Via") != headers.end() ||
        headers.find("Proxy-Connection") != headers.end();
}

void HTTPServer::UpdateClientMetrics(const std::string& ip, const httplib::Request& req) {
    std::lock_guard<std::mutex> lock(m_rateLimitMutex);
    auto& client = m_rateLimit[ip];
    client.isProxy = IsProxyRequest(req);
    client.cpuUsage = m_currentCpuUsage;
}

void HTTPServer::MonitorSystemLoad() {
    m_currentCpuUsage = GetCPUUsage();

    if (m_currentCpuUsage > CPU_THRESHOLD_PROXY) {
        std::cout << "WARNING: High CPU usage detected (" << m_currentCpuUsage
            << "%) - Possible proxy attack" << std::endl;
    }
    else if (m_currentCpuUsage > CPU_THRESHOLD_FLOOD) {
        std::cout << "WARNING: Elevated CPU usage (" << m_currentCpuUsage
            << "%) - Possible flood attack" << std::endl;
    }
}

void makeHttpRequestAsync(const std::string& url, std::function<void(const std::string&)> callback) {
    std::thread([url, callback]() {
        try {
            std::regex url_regex(R"(^(http[s]?://)?([^:/\s]+)(:[0-9]+)?(/.*)?$)");
            std::smatch url_match;

            if (std::regex_match(url, url_match, url_regex)) {
                std::string host = url_match[2];  // Host
                std::string path = url_match[4];  // Path

                httplib::Client cli(host.c_str());

                cli.set_connection_timeout(10);
                cli.set_read_timeout(10);

                auto result = cli.Get(path.c_str());

                if (result) {
                    if (result->status == 200) {
                        callback(result->body);
                    }
                    else {
                        callback("Error: HTTP " + std::to_string(result->status));
                    }
                }
                else {
                    callback("Error: Failed to connect to server");
                }
            }
            else {
                callback("Error: Invalid URL");
            }
        }
        catch (const std::exception& e) {
            // Handle any exceptions
            callback("Error: " + std::string(e.what()));
        }
        }).detach();
}

void HTTPServer::ServicePoll() {
    m_pServer->Post("/growtopia/server_data.php", [&](const httplib::Request& request, httplib::Response& response) {
        // Cek apakah alamat IP diblacklist
        if (IsBlacklisted(request.remote_addr)) {
            response.status = 403; // Forbidden
            response.set_content("Forbidden", "text/plain");
            return;
        }

        if (IsRateLimited(request.remote_addr)) {
            response.status = 429;
            response.set_content("Too Many Requests", "text/plain");
            LogRequest(request.remote_addr, request.path, "Blocked - Rate Limited");
            return;
        }

        std::cout << "A request from " << request.remote_addr << " | " << request.path << std::endl;

        const std::string version = "5.10"; // Versi server
        std::string client_version = request.get_param_value("version"); // Dapatkan versi klien dari parameter

        std::string str;
        if (client_version == version) {
            std::cout << "Version matched: " << client_version << std::endl;

            str = "server|" + Environment()->server_ip + "\n";
            str += "port|17095\n";
            str += "loginurl|gtps.playserver.me\n";
            str += "type|1\n";
            str += "#maint|Lagi Maintenance GOblok sbr\n";
            str += "meta|growly.com\n";
            str += "RTENDMARKERBS1001";
        }
        else {
            std::cerr << "Version mismatch: Client " << client_version << " != Server " << version << std::endl;

            str = "error|1000|Update is now available for your device. Go get it!\n";
            str += "url|https://growtopiagame.com/Growtopia-Installer.exe\n";
            str += "port|" + std::to_string(SERVER_PORT) + "\n";
            str += "loginurl|tron.craftjava.net\n";
            str += "type|1\n";
            str += "#maint|Lagi MT ASU\n";
            str += "meta|growly.com\n";
            str += "RTENDMARKERBS1001";
        }
        response.set_content(str, "text/html");
        std::cout << "Response sent: \n" << str << std::endl;
        });
    m_pServer->listen_after_bind();
}
