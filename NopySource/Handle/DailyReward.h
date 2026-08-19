#pragma once

// ============================================================================
//  DailyReward.h
//  Main entry point untuk Daily Reward System.
//  File ini mengintegrasikan struktur baru:
//    - DailyReward/RewardConfig.h       (data konfigurasi reward)
//    - DailyReward/PlayerDailyData.h    (data harian per player)
//    - DailyReward/DailyRewardSystem.h  (business logic)
//
//  Juga menyediakan shim backward-compatible untuk existing call sites:
//    - DailyReward_BuildDialog(peer)
//    - DailyReward_Claim(peer)
//    - DailyReward_BuildEditDialog(peer)       [admin /setdaily]
//    - DailyReward_BuildEditDayDialog(peer,d)  [admin edit per-hari]
// ============================================================================

#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "NopySource/Handle/PacketHandler.h"

using json = nlohmann::json;

#include "NopySource/Handle/DailyReward/RewardConfig.h"
#include "NopySource/Handle/DailyReward/PlayerDailyData.h"
#include "NopySource/Handle/DailyReward/DailyRewardSystem.h"

// ============================================================================
//  Legacy struct & functions (untuk backward compatibility code lama)
// ============================================================================

// Alias lama supaya code yang masih pakai DailyRewardItem tetap compile
using DailyRewardItem = DailyReward::Reward;

// Legacy DailyRewardConfig struct (wrapper ke DailyReward::RewardConfig)
struct DailyRewardConfig {
    std::string title       = "Daily Rewards";
    std::string description = "Claim your daily reward every day! Missed days are permanently skipped.";
    std::string footer_note = "Note: Missed days are lost permanently. After Day 30, cycle resets to Day 1.";

    // Legacy rewards vector: vector of pairs (item_id, quantity).
    // Di-back oleh DailyReward::RewardConfig::rewards.
    std::vector<std::pair<int,int>> rewards;

    void load() {
        DailyReward::GetConfig().load();
        SyncFromNew();
    }

    void save() {
        // Update DailyReward::GetConfig() dari legacy rewards pair
        DailyReward::GetConfig().rewards.clear();
        int day = 1;
        for (auto& p : rewards) {
            DailyReward::Reward r;
            r.Day      = day++;
            r.ItemID   = p.first;
            r.Quantity = p.second;
            DailyReward::GetConfig().rewards.push_back(r);
        }
        DailyReward::GetConfig().save();
    }

    void SyncFromNew() {
        rewards.clear();
        for (auto& r : DailyReward::GetConfig().rewards) {
            rewards.push_back({ r.ItemID, r.Quantity });
        }
    }
};

inline DailyRewardConfig g_dailyRewardConfig;

// Legacy function: cek apakah hari baru
inline bool DailyReward_IsNewDay(long long last_claim_timestamp) {
    if (last_claim_timestamp == 0) return true;
    return DailyReward::PlayerDailyData::IsDifferentCalendarDay(
        last_claim_timestamp, DailyReward::PlayerDailyData::Now());
}

// Legacy function: hitung hari yang terlewat
inline int DailyReward_DaysSince(long long last_claim_timestamp) {
    if (last_claim_timestamp == 0) return 999;
    return DailyReward::PlayerDailyData::DaysElapsed(last_claim_timestamp);
}

// ============================================================================
//  Backward-compatible public API
// ============================================================================

// Build dialog Daily Reward (untuk player)
inline std::string DailyReward_BuildDialog(ENetPeer* peer) {
    if (!peer || !peer->data) return "";
    return DailyReward::DailyRewardSystem::BuildDialog(pInfo(peer));
}

// Proses claim daily reward (dari dialog button click)
inline void DailyReward_Claim(ENetPeer* peer) {
    if (!peer || !peer->data) return;
    DailyReward::DailyRewardSystem::HandleClaim(peer);
}

// Open dialog Daily Reward (untuk player)
inline void DailyReward_OpenDialog(ENetPeer* peer) {
    if (!peer || !peer->data) return;
    DailyReward::DailyRewardSystem::OpenDialog(peer);
}

// ============================================================================
//  Admin dialogs (untuk /setdaily)
//  Tetap inline di sini karena admin UI adalah concern terpisah dari core logic.
// ============================================================================

// Build dialog untuk /setdaily (admin config)
inline std::string DailyReward_BuildEditDialog(ENetPeer* peer) {
    DailyReward::GetConfig().load();
    auto& cfg = DailyReward::GetConfig();

    std::string d = SetColor(peer);
    d += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
    d += "add_label_with_icon|big|`wSet Daily Rewards``|left|7188|\n";
    d += "add_spacer|small|\n";
    d += "add_textbox|`oEdit reward for each day. Click a day to modify.``|left|\n";
    d += "add_spacer|small|\n";

    d += "add_textbox|`wTitle:``|left|\n";
    d += "add_text_input|dr_title||" + cfg.title + "|30|\n";
    d += "add_spacer|small|\n";

    d += "add_textbox|`wDescription:``|left|\n";
    d += "add_text_input|dr_desc||" + cfg.description + "|80|\n";
    d += "add_spacer|small|\n";

    d += "add_textbox|`wFooter Note:``|left|\n";
    d += "add_text_input|dr_note||" + cfg.footer_note + "|80|\n";
    d += "add_spacer|small|\n";

    d += "add_custom_break|\n";
    d += "add_spacer|small|\n";

    for (int i = 0; i < 30 && i < (int)cfg.rewards.size(); i++) {
        int day_num = i + 1;
        auto& reward = cfg.rewards[i];
        std::string item_name = "Unknown";
        if (reward.ItemID > 0 && reward.ItemID < (int)items.size()) {
            item_name = items[reward.ItemID].name;
        }
        d += "add_button_with_icon|dr_edit_" + std::to_string(day_num) + "|`wDay "
             + std::to_string(day_num) + ": " + item_name + " x" + std::to_string(reward.Quantity)
             + "``|staticBlueFrame|" + std::to_string(reward.ItemID) + "|0|\n";
    }

    d += "add_spacer|small|\n";
    d += "add_button|dr_save_text|`2Save Title/Desc/Note``|noflags|0|0|\n";
    d += "add_button|dr_reset|`4Reset to Default``|noflags|0|0|\n";
    d += "add_quick_exit|\n";
    d += "end_dialog|setDaily|Close||\n";

    return d;
}

// Edit reward untuk hari tertentu
inline std::string DailyReward_BuildEditDayDialog(ENetPeer* peer, int day) {
    DailyReward::GetConfig().load();
    if (day < 1 || day > 30) day = 1;
    int idx = day - 1;
    auto& cfg = DailyReward::GetConfig();
    if (idx >= (int)cfg.rewards.size()) idx = 0;

    auto& reward = cfg.rewards[idx];
    std::string item_name = "Unknown";
    if (reward.ItemID > 0 && reward.ItemID < (int)items.size()) {
        item_name = items[reward.ItemID].name;
    }

    std::string d = SetColor(peer);
    d += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
    d += "add_label_with_icon|big|`wEdit Day " + std::to_string(day) + " Reward``|left|"
         + std::to_string(reward.ItemID) + "|\n";
    d += "add_spacer|small|\n";
    d += "add_textbox|`oCurrent: `w" + item_name + " x" + std::to_string(reward.Quantity) + "``|left|\n";
    d += "add_spacer|small|\n";
    d += "add_textbox|`wItem ID:``|left|\n";
    d += "add_text_input|dr_item_id||" + std::to_string(reward.ItemID) + "|10|\n";
    d += "add_textbox|`wAmount:``|left|\n";
    d += "add_text_input|dr_amount||" + std::to_string(reward.Quantity) + "|10|\n";
    d += "add_spacer|small|\n";
    d += "embed_data|dr_day|" + std::to_string(day) + "\n";
    d += "add_button|dr_save_day|`2Save``|noflags|0|0|\n";
    d += "add_button|dr_back|`oBack to List``|noflags|0|0|\n";
    d += "add_quick_exit|\n";
    d += "end_dialog|setDaily_edit|Close||\n";

    return d;
}
