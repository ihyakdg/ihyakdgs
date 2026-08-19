#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

// ============================================================================
//  RewardConfig.h
//  Struktur data reward untuk system Daily Reward.
//  Data disimpan di database/json/dailyreward.json
// ============================================================================

extern std::map<int, std::pair<int, int>> dailyReward;

namespace DailyReward {

// Type of reward the player receives
enum class RewardType {
    ITEM,       // regular item from item database
    GEMS,       // gems currency (reserved)
    COINS,      // server coins (reserved)
    OPC,        // Online Point Currency (reserved)
};

inline const char* RewardTypeToString(RewardType t) {
    switch (t) {
        case RewardType::GEMS:  return "gems";
        case RewardType::COINS: return "coins";
        case RewardType::OPC:   return "opc";
        default: return "item";
    }
}

inline RewardType RewardTypeFromString(const std::string& s) {
    if (s == "gems")  return RewardType::GEMS;
    if (s == "coins") return RewardType::COINS;
    if (s == "opc")   return RewardType::OPC;
    return RewardType::ITEM;
}

// One reward entry (Day + Item + Quantity + Type)
struct Reward {
    int        Day       = 0;
    int        ItemID    = 0;        // item id dari items.dat (jika Item)
    std::string ItemName;            // nama item (display only, source of truth = ItemID)
    int        Quantity  = 1;
    RewardType Type      = RewardType::ITEM;
};

// ---------------------------------------------------------------------------
//  RewardConfig
//  Singleton-like struct yang menyimpan konfigurasi 30 hari reward.
//  Load/Save dari disk.
// ---------------------------------------------------------------------------
struct RewardConfig {
    static constexpr int TOTAL_DAYS = 30;

    std::string title       = "Daily Reward";
    std::string description = "Anyone can log in once, but the consistent are rewarded. Visit every day for 30 days, claim your daily prizes, and stay the course to earn a special reward waiting on the final day!";
    std::string footer_note = "Note: Missing a day will skip that reward!";
    std::vector<Reward> rewards;     // index 0 = Day 1, index 29 = Day 30

    void SyncFromDailyRewardMap() {
        if (!dailyReward.empty()) {
            rewards.clear();
            rewards.reserve(TOTAL_DAYS);
            for (int d = 1; d <= TOTAL_DAYS; ++d) {
                Reward r;
                r.Day = d;
                if (dailyReward.count(d)) {
                    r.ItemID   = dailyReward[d].first;
                    r.Quantity = dailyReward[d].second;
                    if (r.ItemID > 0 && r.ItemID < (int)items.size()) {
                        r.ItemName = items[r.ItemID].name;
                    }
                } else {
                    r.ItemID   = 1796;
                    r.Quantity = 1;
                    r.ItemName = "Diamond Lock";
                }
                r.Type = RewardType::ITEM;
                rewards.push_back(r);
            }
        }
    }

    void SyncToDailyRewardMap() {
        int day = 1;
        for (auto& r : rewards) {
            dailyReward[day++] = { r.ItemID, r.Quantity };
        }
    }

    // Default rewards kalau file belum ada (persis seperti di foto)
    void ApplyDefaultRewards() {
        rewards.clear();
        rewards.reserve(TOTAL_DAYS);
        // Format: {Day, ItemID, ItemName, Quantity, Type}
        rewards.push_back({  1, 1424,  "Toolbox",        15,  RewardType::ITEM });
        rewards.push_back({  2, 7188,  "Magic Potion",   50,  RewardType::ITEM });
        rewards.push_back({  3, 10428, "Golden Ticket",   3,  RewardType::ITEM });
        rewards.push_back({  4, 1488,  "Surprise Box",   25,  RewardType::ITEM });
        rewards.push_back({  5, 8148,  "Dark Soul",      20,  RewardType::ITEM });
        rewards.push_back({  6, 12228, "Green Laurel",    3,  RewardType::ITEM });
        rewards.push_back({  7, 12230, "Biosphere",       1,  RewardType::ITEM });
        rewards.push_back({  8, 1424,  "Toolbox",        30,  RewardType::ITEM });
        rewards.push_back({  9, 7188,  "Magic Potion",  100,  RewardType::ITEM });
        rewards.push_back({ 10, 10428, "Golden Ticket",  10,  RewardType::ITEM });
        rewards.push_back({ 11, 1488,  "Surprise Box",   35,  RewardType::ITEM });
        rewards.push_back({ 12, 1796,  "Diamond Lock",    1,  RewardType::ITEM });
        rewards.push_back({ 13, 12228, "Green Laurel",    3,  RewardType::ITEM });
        rewards.push_back({ 14, 9850,  "Stasis Chamber",  1,  RewardType::ITEM });
        rewards.push_back({ 15, 1424,  "Toolbox",        50,  RewardType::ITEM });
        rewards.push_back({ 16, 7188,  "Magic Potion",  150,  RewardType::ITEM });
        rewards.push_back({ 17, 10428, "Golden Ticket",  25,  RewardType::ITEM });
        rewards.push_back({ 18, 1488,  "Surprise Box",   45,  RewardType::ITEM });
        rewards.push_back({ 19, 8148,  "Dark Soul",      30,  RewardType::ITEM });
        rewards.push_back({ 20, 12228, "Green Laurel",    4,  RewardType::ITEM });
        rewards.push_back({ 21, 9850,  "Stasis Chamber",  1,  RewardType::ITEM });
        rewards.push_back({ 22, 1424,  "Toolbox",        75,  RewardType::ITEM });
        rewards.push_back({ 23, 7188,  "Magic Potion",  200,  RewardType::ITEM });
        rewards.push_back({ 24, 10428, "Golden Ticket",  50,  RewardType::ITEM });
        rewards.push_back({ 25, 1488,  "Surprise Box",   75,  RewardType::ITEM });
        rewards.push_back({ 26, 8148,  "Dark Soul",      50,  RewardType::ITEM });
        rewards.push_back({ 27, 12228, "Green Laurel",    4,  RewardType::ITEM });
        rewards.push_back({ 28, 1796,  "Diamond Lock",    1,  RewardType::ITEM });
        rewards.push_back({ 29, 9850,  "Stasis Chamber",  2,  RewardType::ITEM });
        rewards.push_back({ 30, 9850,  "Stasis Chamber",  4,  RewardType::ITEM });
    }

    void load(const std::string& path = "database/json/daily_reward.json") {
        rewards.clear();
        std::string actual_path = std::filesystem::exists(path) ? path : (std::filesystem::exists("database/json/dailyreward.json") ? "database/json/dailyreward.json" : "");

        if (!actual_path.empty()) {
            try {
                std::ifstream file(actual_path);
                nlohmann::json j;
                file >> j;
                file.close();

                if (j.contains("dailyreward")) {
                    auto map_data = j["dailyreward"].get<std::map<int, std::pair<int, int>>>();
                    dailyReward = map_data;
                    SyncFromDailyRewardMap();
                    return;
                }

                if (j.contains("title"))         title         = j["title"].get<std::string>();
                if (j.contains("description"))   description   = j["description"].get<std::string>();
                if (j.contains("footer_note"))   footer_note   = j["footer_note"].get<std::string>();
                if (j.contains("rewards") && j["rewards"].is_array()) {
                    int day_counter = 1;
                    for (auto& r : j["rewards"]) {
                        Reward rd;
                        rd.Day = day_counter++;
                        if (r.contains("item_id"))    rd.ItemID   = r["item_id"].get<int>();
                        if (r.contains("item_name"))  rd.ItemName = r["item_name"].get<std::string>();
                        if (r.contains("quantity"))   rd.Quantity = r["quantity"].get<int>();
                        if (r.contains("type"))       rd.Type     = RewardTypeFromString(r["type"].get<std::string>());
                        rewards.push_back(rd);
                    }
                }
            }
            catch (...) {}
        }

        if (!dailyReward.empty()) {
            SyncFromDailyRewardMap();
            return;
        }

        if (rewards.empty()) {
            ApplyDefaultRewards();
            SyncToDailyRewardMap();
            save(path);
        }
    }

    void save(const std::string& path = "database/json/daily_reward.json") {
        SyncToDailyRewardMap();

        nlohmann::json j;
        j["title"]       = title;
        j["description"] = description;
        j["footer_note"] = footer_note;
        j["dailyreward"] = dailyReward;
        j["rewards"]     = nlohmann::json::array();
        int day = 1;
        for (auto& r : rewards) {
            nlohmann::json item;
            item["day"]       = day++;
            item["item_id"]   = r.ItemID;
            item["item_name"] = r.ItemName;
            item["quantity"]  = r.Quantity;
            item["type"]      = RewardTypeToString(r.Type);
            j["rewards"].push_back(item);
        }
        std::filesystem::create_directories("database/json");
        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
    }

    // Helper: ambil reward by day (1-indexed, wrap around > 30 ke 1)
    const Reward* Get(int day) const {
        if (rewards.empty()) return nullptr;
        day = ((day - 1) % TOTAL_DAYS);
        if (day < 0) day += TOTAL_DAYS;
        if (day >= (int)rewards.size()) return nullptr;
        return &rewards[day];
    }
};

// Global config instance
inline RewardConfig& GetConfig() {
    static RewardConfig cfg;
    return cfg;
}

} // namespace DailyReward
