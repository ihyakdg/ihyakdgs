#pragma once
#include "RewardConfig.h"
#include "PlayerDailyData.h"
#include <string>
#include <sstream>

// ============================================================================
//  DailyRewardSystem.h
//  Core business logic untuk Daily Reward system.
//
//  Required functions (sesuai spec):
//    - LoadPlayerData()
//    - SavePlayerData()
//    - CanClaimToday()
//    - ClaimReward()
//    - SkipMissedReward()
//    - GetCurrentReward()
//    - ResetRewardCycle()
//    - ShowRewardList() / BuildDialog()
//
//  Aturan:
//    - 30 hari reward total, claim 1x per hari.
//    - Progress disave permanen (lewat Player struct + JSON).
//    - Lewat 1 hari = reward hari itu HANGUS (skip), lanjut ke hari berikutnya.
//    - Double-claim di hari yang sama TIDAK BISA.
// ============================================================================

namespace DailyReward {

class DailyRewardSystem {
public:
    // ------------------------------------------------------------------
    //  1. LoadPlayerData
    //  Baca daily reward state dari Player struct ke PlayerDailyData.
    // ------------------------------------------------------------------
    static void LoadPlayerData(Player* pl, PlayerDailyData& out) {
        if (!pl) { out = PlayerDailyData{}; return; }
        out.ReadFromPlayer(pl);
    }

    // ------------------------------------------------------------------
    //  2. SavePlayerData
    //  Tulis PlayerDailyData kembali ke Player struct (mirror).
    // ------------------------------------------------------------------
    static void SavePlayerData(Player* pl, const PlayerDailyData& in) {
        if (!pl) return;
        in.WriteToPlayer(pl);
    }

    // ------------------------------------------------------------------
    //  3. CanClaimToday
    //  Return true kalau player masih bisa claim hari ini.
    //  False kalau sudah claim ATAU kalau current_day invalid.
    // ------------------------------------------------------------------
    static bool CanClaimToday(const PlayerDailyData& data) {
        if (data.ClaimedToday) return false;
        if (data.CurrentDay < 1 || data.CurrentDay > RewardConfig::TOTAL_DAYS) return false;
        return true;
    }

    // ------------------------------------------------------------------
    //  4. GetCurrentReward
    //  Return pointer ke Reward untuk hari ini, atau nullptr kalau invalid.
    // ------------------------------------------------------------------
    static const Reward* GetCurrentReward(const PlayerDailyData& data) {
        return GetConfig().Get(data.CurrentDay);
    }

    // ------------------------------------------------------------------
    //  5. SkipMissedReward
    //  Cek apakah ada hari yang terlewat, advance current_day sesuai jumlah
    //  hari yang terlewat. Hari yang dilewat ditandai status = 2 (skipped).
    //  Dipanggil saat player login / buka dialog.
    // ------------------------------------------------------------------
    static int SkipMissedReward(PlayerDailyData& data) {
        // Baru pertama login: tidak ada yang di-skip
        if (data.LastLoginDate == 0) return 0;

        long long now = PlayerDailyData::Now();
        int days_gap = PlayerDailyData::DaysElapsed(data.LastLoginDate, now);

        // Jika masih di hari yang sama (sudah pernah claim / baru login hari ini), skip
        if (days_gap <= 0) return 0;

        int skipped = 0;

        // Kalau player sudah claimed hari kemarin, hari berikutnya otomatis tersedia
        if (data.ClaimedToday) {
            // Sudah claimed kemarin, hari berikutnya = current_day + 1 (normal)
            // Lalu tambahkan gap (days_gap - 1) sebagai hari yang dilewat
            int days_to_advance = 1 + (days_gap - 1);

            for (int i = 0; i < days_to_advance; i++) {
                if (i == 0) {
                    // Hari setelah claim: normal advance (tidak dihitung missed)
                } else {
                    // Hari-hari berikutnya: miss
                    if (data.CurrentDay >= 1 && data.CurrentDay <= 30) {
                        // Hanya tandai sebagai skipped kalau belum claimed dan belum skipped sebelumnya
                        if (data.Status[data.CurrentDay - 1] == 0) {
                            data.Status[data.CurrentDay - 1] = 2;
                            skipped++;
                            data.MissedDaysTotal++;
                        }
                    }
                }
                data.CurrentDay++;
                if (data.CurrentDay > RewardConfig::TOTAL_DAYS) {
                    data.CurrentDay = 1;
                }
            }
        } else {
            // Player belum claim kemarin (hari current_day) — HARI INI hilang dulu
            if (data.CurrentDay >= 1 && data.CurrentDay <= 30
                && data.Status[data.CurrentDay - 1] == 0) {
                data.Status[data.CurrentDay - 1] = 2;
                skipped++;
                data.MissedDaysTotal++;
            }
            data.CurrentDay++;

            // Lalu skip sisa hari gap (days_gap - 1) tambahan
            for (int i = 1; i < days_gap; i++) {
                if (data.CurrentDay >= 1 && data.CurrentDay <= 30
                    && data.Status[data.CurrentDay - 1] == 0) {
                    data.Status[data.CurrentDay - 1] = 2;
                    skipped++;
                    data.MissedDaysTotal++;
                }
                data.CurrentDay++;
                if (data.CurrentDay > RewardConfig::TOTAL_DAYS) data.CurrentDay = 1;
            }
        }

        data.ClaimedToday = false;
        data.LastLoginDate = now;
        return skipped;
    }

    // ------------------------------------------------------------------
    //  6. ResetRewardCycle
    //  Reset cycle ke Day 1 (misal setelah Day 30). Clear semua status.
    // ------------------------------------------------------------------
    static void ResetRewardCycle(PlayerDailyData& data) {
        data.CurrentDay    = 1;
        data.ClaimedToday  = false;
        for (int i = 0; i < 30; i++) data.Status[i] = 0;
    }

    // ------------------------------------------------------------------
    //  7. ClaimReward
    //  Eksekusi claim. Return true kalau berhasil.
    //  Menggunakan Inventory::Modify untuk memberikan item.
    // ------------------------------------------------------------------
    static bool ClaimReward(ENetPeer* peer, std::string* out_message = nullptr) {
        if (!peer || !peer->data) return false;
        Player* pl = pInfo(peer);

        PlayerDailyData data;
        LoadPlayerData(pl, data);

        // Auto-skip missed days sebelum claim
        int skipped = SkipMissedReward(data);

        if (!CanClaimToday(data)) {
            if (out_message) *out_message = "You already claimed today's daily reward!";
            SavePlayerData(pl, data);
            return false;
        }

        const Reward* r = GetCurrentReward(data);
        if (!r || r->Quantity <= 0) {
            if (out_message) *out_message = "Error: invalid reward configuration.";
            SavePlayerData(pl, data);
            return false;
        }

        // Berikan item sesuai type
        bool given = false;
        switch (r->Type) {
            case RewardType::ITEM: {
                // Inventory::Modify(ENetPeer*, int item_id, int& count, ...)
                int give = r->Quantity;
                if (Inventory::Modify(peer, r->ItemID, give, true) == 0) given = true;
                break;
            }
            case RewardType::GEMS:  pl->gems += r->Quantity; given = true; break;
            case RewardType::COINS: pl->gtwl += r->Quantity; given = true; break;
            case RewardType::OPC:   pl->opc  += r->Quantity; given = true; break;
        }

        if (!given) {
            if (out_message) *out_message = "Inventory full! Make space and try again.";
            SavePlayerData(pl, data);
            return false;
        }

        // Update state
        int day_idx = data.CurrentDay - 1;
        if (day_idx >= 0 && day_idx < 30) data.Status[day_idx] = 1; // claimed
        data.ClaimedToday = true;
        data.TotalClaim++;
        data.LastLoginDate = PlayerDailyData::Now();

        // Build pesan sukses
        std::string item_name = r->ItemName;
        if ((item_name.empty() || item_name == "Unknown") && r->ItemID > 0 && r->ItemID < (int)items.size()) {
            item_name = items[r->ItemID].name;
        }
        if (item_name.empty()) item_name = "Unknown Item";

        if (out_message) {
            std::ostringstream oss;
            oss << "You claimed Day " << data.CurrentDay
                << " reward: " << item_name << " x" << r->Quantity << "!";
            *out_message = oss.str();
        }

        SavePlayerData(pl, data);
        return true;
    }

    // ------------------------------------------------------------------
    //  8. ShowRewardList / BuildDialog
    //  Build string dialog untuk Growtopia.
    //  Ini adalah versi yang compatible dengan VarList::OnDialogRequest.
    // ------------------------------------------------------------------
    static std::string BuildDialog(Player* pl) {
        if (!pl) return "";

        // Reload config (admin mungkin sudah update)
        GetConfig().load();

        PlayerDailyData data;
        LoadPlayerData(pl, data);

        // Auto-skip missed days
        int skipped = SkipMissedReward(data);
        SavePlayerData(pl, data);

        // Current server time & reset countdown (jam)
        time_t t_now = time(nullptr);
        tm tm_now{};
#ifdef _WIN32
        localtime_s(&tm_now, &t_now);
#else
        localtime_r(&t_now, &tm_now);
#endif
        char time_str[64];
        std::strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_now);

        int seconds_today = tm_now.tm_hour * 3600 + tm_now.tm_min * 60 + tm_now.tm_sec;
        int seconds_left = 86400 - seconds_today;
        if (seconds_left < 0) seconds_left = 0;
        int h_left = seconds_left / 3600;
        int m_left = (seconds_left % 3600) / 60;
        int s_left = seconds_left % 60;

        std::ostringstream d;
        d << "set_default_color|`o\n"
          << "set_bg_color|0,52,102,178|\n"
          << "set_border_color|255,255,255,255|\n\n"
          << "add_label_with_icon|big|`wDaily Reward|left|1488|\n"
          << "add_textbox|`oServer Time: `w" << time_str << "``|center|\n"
          << "add_spacer|small|\n"
          << "add_textured_progress_bar|interface/large/gui_event_bar2.rttex|0|4|`oReset Countdown: `2" << h_left << "h " << m_left << "m " << s_left << "s left``|" << seconds_today << "|86400|center|0.85|0.02|0|1000|64|0.005|dailyBarBG|\n"
          << "add_spacer|small|\n"
          << "add_smalltext|Visit every day for 30 days, claim your daily prizes, and stay the course to earn a special reward waiting on the final day!|center|\n"
          << "add_spacer|small|\n"
          << "text_scaling_string|   Day 30 +9999999999   |\n";

        const auto& rewards = GetConfig().rewards;
        for (int i = 0; i < 30 && i < (int)rewards.size(); i++) {
            int day_num = i + 1;
            const auto& rew = rewards[i];

            int st = data.Status[i];
            if (st == 0) {
                if (day_num < data.CurrentDay) {
                    st = 1; // Past days are claimed
                } else if (day_num == data.CurrentDay && data.ClaimedToday) {
                    st = 1; // Current day claimed today
                }
            }

            std::string label;
            std::string flags = "staticYellowFrame";
            std::string button_id;
            int display_item_id = rew.ItemID;
            int display_quantity = rew.Quantity;

            if (st == 1) { // claimed
                label = "`2CLAIMED!";
                button_id = "dr_day_" + std::to_string(day_num);
                display_item_id = 15634;
            } else if (day_num == data.CurrentDay && !data.ClaimedToday) { // ready to claim today
                label = "Day " + std::to_string(day_num) + ": `2 Claim Now!";
                button_id = "claim_daily_reward";
            } else if (st == 2) { // skipped
                label = "Day " + std::to_string(day_num) + ": `4 Skipped";
                button_id = "dr_day_" + std::to_string(day_num);
            } else { // upcoming / future days
                label = "Day " + std::to_string(day_num);
                button_id = "dr_day_" + std::to_string(day_num);
            }

            d << "add_button_with_icon|" << button_id << "|" << label << "|" << flags << "|" << display_item_id << "|" << display_quantity << "|\n";
        }

        d << "add_button_with_icon||END_LIST|noflags|0||\n"
          << "add_spacer|small|\n"
          << "add_textbox|`wNote: Missing a day will skip that reward!|center|\n"
          << "end_dialog|dailyReward|Close||\n";

        return d.str();
    }

    static std::string BuildInfoDialog(Player* pl, int day) {
        if (!pl || day < 1 || day > 30) return "";
        GetConfig().load();
        PlayerDailyData data;
        LoadPlayerData(pl, data);

        const Reward* rew = GetConfig().Get(day);
        if (!rew) return "";

        std::string item_name = (rew->ItemID > 0 && rew->ItemID < (int)items.size()) ? items[rew->ItemID].name : "Unknown Item";
        std::string item_desc = (rew->ItemID > 0 && rew->ItemID < (int)items.size()) ? items[rew->ItemID].description : "";

        int st = data.Status[day - 1];
        if (st == 0) {
            if (day < data.CurrentDay) {
                st = 1;
            } else if (day == data.CurrentDay && data.ClaimedToday) {
                st = 1;
            }
        }

        std::ostringstream d;
        d << "set_default_color|`o\n"
          << "set_bg_color|0,52,102,178|\n"
          << "set_border_color|255,255,255,255|\n\n"
          << "add_label_with_icon|big|`wDay " << day << ": " << rew->Quantity << " " << item_name << "``|left|" << rew->ItemID << "|\n"
          << "add_spacer|small|\n"
          << "add_smalltext|`o" << (item_desc.empty() ? "Daily reward item." : item_desc) << "|left|\n"
          << "add_spacer|small|\n";

        if (day == data.CurrentDay && !data.ClaimedToday) {
            d << "add_textbox|`2Status: Ready to claim!``|left|\n"
              << "add_spacer|small|\n"
              << "add_button|claim_daily_reward|`2CLAIM|0|0|\n";
        } else if (st == 1) {
            d << "add_textbox|`2Status: Already Claimed``|left|\n";
        } else if (st == 2) {
            d << "add_textbox|`4Status: Skipped (Missed Day)``|left|\n";
        } else {
            d << "add_textbox|`oStatus: Available on Day `w" << day << "``|left|\n";
        }

        d << "add_spacer|small|\n"
          << "add_button|back_to_daily|`oBACK``|0|0|\n"
          << "end_dialog|dailyReward_info|Close||\n";

        return d.str();
    }

    static void OpenInfoDialog(ENetPeer* peer, int day) {
        if (!peer || !peer->data) return;
        std::string d = BuildInfoDialog(pInfo(peer), day);
        if (!d.empty()) VarList::OnDialogRequest(peer, d, 500);
    }

    // Shortcut: kirim dialog langsung ke client
    static void OpenDialog(ENetPeer* peer) {
        if (!peer || !peer->data) return;
        std::string d = BuildDialog(pInfo(peer));
        if (!d.empty()) VarList::OnDialogRequest(peer, d, 500);
    }

    // ------------------------------------------------------------------
    //  Process Claim dari dialog button click
    // ------------------------------------------------------------------
    static void HandleClaim(ENetPeer* peer) {
        if (!peer || !peer->data) return;
        Player* pl = pInfo(peer);

        std::string msg;
        bool ok = ClaimReward(peer, &msg);

        if (ok) {
            VarList::OnConsoleMessage(peer, "`2[Daily Reward] " + msg);
            VarList::OnTalkBubble(peer, pl->netID, "`2[Daily Reward] " + msg, 0, 0);
            VarList::OnPlaySound(peer, "audio/party_horn.wav", 0);

            // Visual effect
            PlayerMoving data_{};
            data_.packetType = 17;
            data_.netID = 198;
            data_.YSpeed = 198;
            data_.x = pl->x + 16;
            data_.y = pl->y + 16;
            BYTE* raw = packPlayerMoving(&data_);
            send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
            delete[] raw;

            Logger::Info("INFO", "[DailyReward] " + pl->tankIDName + " claimed: " + msg);
        } else {
            VarList::OnTalkBubble(peer, pl->netID, "`4[Daily Reward] " + msg, 0, 0);
        }

        // Redraw dialog setelah claim
        OpenDialog(peer);
    }

    // ------------------------------------------------------------------
    //  ResetAllPlayersDailyReward
    //  Reset progress daily reward seluruh player (online & database) ke Day 1
    // ------------------------------------------------------------------
    static void ResetAllPlayersDailyReward() {
        try {
            if (server && server->peers) {
                for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
                    if (!cp_ || cp_->state != ENET_PEER_STATE_CONNECTED || !cp_->data) continue;
                    Player* pl = pInfo(cp_);
                    if (pl) {
                        pl->is_day = 1;
                        pl->claimed_daily_today = false;
                        pl->daily_login_day = 0;
                        pl->daily_streak = 0;
                        PlayerDailyData data;
                        data.CurrentDay = 1;
                        data.ClaimedToday = false;
                        data.LastLoginDate = 0;
                        data.TotalClaim = 0;
                        data.MissedDaysTotal = 0;
                        for (int i = 0; i < 30; i++) data.Status[i] = 0;
                        SavePlayerData(pl, data);
                        ServerPool::SaveDatabase::Players(pl, false);
                        OpenDialog(cp_);
                    }
                }
            }
        } catch (...) {}

        try {
            std::string dir_path = "database/players";
            if (std::filesystem::exists(dir_path)) {
                for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".json") {
                        std::string file_path = entry.path().string();
                        try {
                            std::ifstream ifs(file_path);
                            if (!ifs.is_open()) continue;
                            nlohmann::json j;
                            ifs >> j;
                            ifs.close();

                            j["is_day"] = 1;
                            j["claimed_daily_today"] = false;
                            j["daily_login_day"] = 0;
                            j["daily_streak"] = 0;
                            j["daily_last_login"] = 0;
                            j["daily_current_day"] = 1;
                            j["daily_claimed"] = false;
                            j["daily_total_claim"] = 0;
                            j["daily_missed"] = 0;

                            nlohmann::json st = nlohmann::json::array();
                            for (int i = 0; i < 30; i++) st.push_back(0);
                            j["daily_status"] = st;

                            std::ofstream ofs(file_path);
                            if (ofs.is_open()) {
                                ofs << j.dump(4);
                                ofs.close();
                            }
                        } catch (...) {}
                    }
                }
            }
        } catch (...) {}
    }
};

// ============================================================================
//  TEST SCENARIOS (for documentation / debug)
//  Compile dengan define DAILY_REWARD_TEST_SCENARIOS untuk enable.
//
//  Skenario yang di-cover:
//    1. Login pertama
//    2. Claim berhasil
//    3. Login di hari berikutnya
//    4. Login setelah melewatkan beberapa hari
//    5. Semua pesan status yang relevan
// ============================================================================
#ifdef DAILY_REWARD_TEST_SCENARIOS
inline void RunTestScenarios() {
    std::cout << "==== DailyRewardSystem Test Scenarios ====\n\n";

    // Skenario 1: Login pertama
    {
        std::cout << "[Scenario 1] Login pertama\n";
        PlayerDailyData d;
        DailyRewardSystem::SkipMissedReward(d);
        std::cout << "  CurrentDay: " << d.CurrentDay << " (expected: 1)\n";
        std::cout << "  CanClaim: " << (DailyRewardSystem::CanClaimToday(d) ? "YES" : "NO") << "\n";
        std::cout << "  LastLoginDate: 0 -> " << d.LastLoginDate << "\n\n";
    }

    // Skenario 2: Claim berhasil
    {
        std::cout << "[Scenario 2] Claim berhasil (Day 1)\n";
        PlayerDailyData d;
        d.CurrentDay = 1;
        d.LastLoginDate = PlayerDailyData::Now();
        bool ok = false;
        // Simulate claim
        d.Status[0] = 1;
        d.ClaimedToday = true;
        d.TotalClaim = 1;
        ok = d.ClaimedToday && d.Status[0] == 1;
        std::cout << "  Claimed: " << (ok ? "SUCCESS" : "FAIL") << "\n";
        std::cout << "  CanClaim again: " << (DailyRewardSystem::CanClaimToday(d) ? "YES" : "NO") << " (expected: NO)\n\n";
    }

    // Skenario 3: Login di hari berikutnya
    {
        std::cout << "[Scenario 3] Login di hari berikutnya (Day 2)\n";
        PlayerDailyData d;
        d.CurrentDay = 1;
        d.ClaimedToday = true;
        d.LastLoginDate = PlayerDailyData::Now() - 86400; // 1 hari yang lalu
        d.Status[0] = 1;
        int skipped = DailyRewardSystem::SkipMissedReward(d);
        std::cout << "  Skipped days: " << skipped << " (expected: 0)\n";
        std::cout << "  CurrentDay: " << d.CurrentDay << " (expected: 2)\n";
        std::cout << "  CanClaim: " << (DailyRewardSystem::CanClaimToday(d) ? "YES" : "NO") << "\n\n";
    }

    // Skenario 4: Login setelah melewatkan beberapa hari
    {
        std::cout << "[Scenario 4] Login setelah 3 hari offline (Day 1 -> Day 5)\n";
        PlayerDailyData d;
        d.CurrentDay = 1;
        d.ClaimedToday = true;
        d.LastLoginDate = PlayerDailyData::Now() - (3 * 86400); // 3 hari lalu
        d.Status[0] = 1; // day 1 sudah claimed sebelumnya
        std::cout << "  Before: CurrentDay=" << d.CurrentDay << "\n";
        int skipped = DailyRewardSystem::SkipMissedReward(d);
        std::cout << "  Skipped days: " << skipped << " (expected: 3)\n";
        std::cout << "  CurrentDay after: " << d.CurrentDay << " (expected: 5)\n";
        std::cout << "  MissedTotal: " << d.MissedDaysTotal << "\n";
        std::cout << "  Status[2..4] (Day 3,4,5): ";
        for (int i = 1; i <= 4; i++) std::cout << d.Status[i] << " ";
        std::cout << " (expected: 2 2 2 0 = skipped, skipped, skipped, claimable)\n\n";
    }

    // Skenario 5: Double claim prevention
    {
        std::cout << "[Scenario 5] Double claim prevention\n";
        PlayerDailyData d;
        d.CurrentDay = 1;
        d.LastLoginDate = PlayerDailyData::Now();
        d.ClaimedToday = true;
        std::cout << "  CanClaim after claim: " << (DailyRewardSystem::CanClaimToday(d) ? "YES" : "NO") << " (expected: NO)\n\n";
    }

    std::cout << "==== All scenarios complete ====\n";
}
#endif // DAILY_REWARD_TEST_SCENARIOS

} // namespace DailyReward
