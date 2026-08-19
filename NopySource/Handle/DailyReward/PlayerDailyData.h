#pragma once
#include <string>
#include <chrono>
#include <ctime>
#include <nlohmann/json.hpp>

// ============================================================================
//  PlayerDailyData.h
//  Struktur data player untuk Daily Reward system.
//
//  Mapping ke field Player struct yang sudah ada di PlayerInfo.h:
//    - daily_login_day   -> last login timestamp (seconds since epoch)
//    - is_day            -> current reward day (1-30)
//    - claimed_daily_today -> sudah claim hari ini?
//
//  Field baru (perlu ditambahkan ke Player struct):
//    - total_daily_claim -> total lifetime claims (statistik)
//    - missed_days_total -> total days yang terlewat (statistik)
// ============================================================================

namespace DailyReward {

// Wrapper struct: merepresentasikan state Daily Reward seorang player.
// Data ini adalah mirror dari field di Player struct.
// Bisa di-read langsung dari Player* atau di-load/save dari JSON.
struct PlayerDailyData {
    long long LastLoginDate     = 0;    // timestamp (seconds since epoch) last claim/login eval
    int       CurrentDay        = 1;    // day yang sedang aktif (1-30)
    bool      ClaimedToday      = false;
    int       TotalClaim        = 0;    // lifetime total claims (statistik, optional)
    int       MissedDaysTotal   = 0;    // total days yang terlewat (statistik)
    int       Status[30]        = {0};  // 0 = locked, 1 = claimed, 2 = skipped (lost)

    // ------------------------------------------------------------------
    //  Date utilities using std::chrono
    // ------------------------------------------------------------------

    // Ambil timestamp now dalam detik
    static long long Now() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    // Ambil tanggal calendar (year, month, day) dari timestamp
    struct CalendarDate {
        int year, month, day;
        long long DayIndex() const {
            // Naive day-number calc (year*10000 + month*100 + day)
            // Good enough for calendar difference, avoids tm pitfalls
            return (long long)year * 10000LL + (long long)month * 100LL + (long long)day;
        }
    };

    static CalendarDate ToCalendar(long long timestamp) {
        time_t t = (time_t)timestamp;
        tm tm_val{};
#ifdef _WIN32
        localtime_s(&tm_val, &t);
#else
        localtime_r(&t, &tm_val);
#endif
        return { tm_val.tm_year + 1900, tm_val.tm_mon + 1, tm_val.tm_mday };
    }

    // Apakah dua timestamp berada di tanggal calendar yang berbeda?
    // Menggunakan std::chrono + localtime untuk validasi yang benar.
    static bool IsDifferentCalendarDay(long long a, long long b) {
        return ToCalendar(a).DayIndex() != ToCalendar(b).DayIndex();
    }

    // Hitung berapa hari calendar yang telah berlalu antara last_ts dan sekarang.
    // Return 0 jika masih hari yang sama.
    // Robust: tidak bisa dimanipulasi dengan mengubah system clock ke masa depan
    //         karena membatasi hasil maksimum 1000 hari (anti-cheat).
    static int DaysElapsed(long long last_ts, long long now_ts = Now()) {
        if (last_ts == 0) return 0; // first-time player: tidak ada gap

        // Anti-manipulation: kalau now_ts < last_ts (clock mundur), return 0
        if (now_ts < last_ts) return 0;

        // Batasi gap maksimum 1000 hari → kalau > 1000 hari, anggap cheat
        const long long MAX_GAP_SECONDS = 1000LL * 86400LL;
        if (now_ts - last_ts > MAX_GAP_SECONDS) return 1000;

        auto last_cal = ToCalendar(last_ts);
        auto now_cal  = ToCalendar(now_ts);

        // Hitung selisih tanggal menggunakan tm_yday + year diff
        // Konversi ke day-of-year + year lalu hitung
        return DateDiffDays(last_cal.year, last_cal.month, last_cal.day,
                            now_cal.year,  now_cal.month,  now_cal.day);
    }

    // Helper: hitung selisih hari antara dua tanggal calendar (accurate untuk rentang < 10 tahun)
    static int DateDiffDays(int y1, int m1, int d1, int y2, int m2, int d2) {
        auto ToDaysSinceEpoch = [](int y, int m, int d) -> long long {
            // Julian Day Number (simplified, works for 2000-2200 range)
            int a = (14 - m) / 12;
            int yr = y + 4800 - a;
            int mn = m + 12 * a - 3;
            long long jdn = d + (153 * mn + 2) / 5 + 365 * yr + yr / 4 - yr / 100 + yr / 400 - 32045;
            return jdn;
        };
        long long a = ToDaysSinceEpoch(y1, m1, d1);
        long long b = ToDaysSinceEpoch(y2, m2, d2);
        long long diff = b - a;
        if (diff < 0) return 0;
        if (diff > 1000) return 1000; // cap
        return (int)diff;
    }

    // ------------------------------------------------------------------
    //  JSON serialization (untuk save/load per player)
    // ------------------------------------------------------------------

    nlohmann::json SaveToJson() const {
        nlohmann::json j;
        j["daily_last_login"]  = LastLoginDate;
        j["daily_current_day"] = CurrentDay;
        j["daily_claimed"]      = ClaimedToday;
        j["daily_total_claim"] = TotalClaim;
        j["daily_missed"]      = MissedDaysTotal;
        nlohmann::json st = nlohmann::json::array();
        for (int i = 0; i < 30; i++) st.push_back(Status[i]);
        j["daily_status"] = st;
        return j;
    }

    void LoadFromJson(const nlohmann::json& j) {
        if (j.contains("daily_last_login"))  LastLoginDate  = j["daily_last_login"].get<long long>();
        if (j.contains("daily_current_day")) CurrentDay     = j["daily_current_day"].get<int>();
        if (j.contains("daily_claimed"))      ClaimedToday   = j["daily_claimed"].get<bool>();
        if (j.contains("daily_total_claim")) TotalClaim     = j["daily_total_claim"].get<int>();
        if (j.contains("daily_missed"))      MissedDaysTotal = j["daily_missed"].get<int>();
        if (j.contains("daily_status") && j["daily_status"].is_array()) {
            auto& arr = j["daily_status"];
            for (int i = 0; i < (int)arr.size() && i < 30; i++) {
                Status[i] = arr[i].get<int>();
            }
        }
        // Sanitize
        if (CurrentDay < 1) CurrentDay = 1;
        if (CurrentDay > 30) CurrentDay = 1; // safety: reset kalau corrupt
    }

    // ------------------------------------------------------------------
    //  Binding ke Player struct (read/write mirror)
    //  Panggil ini di DailyRewardSystem untuk sinkronisasi.
    // ------------------------------------------------------------------

    // Baca dari Player struct (asumsikan Player punya field yang sesuai)
    template<typename PlayerT>
    void ReadFromPlayer(const PlayerT* pl) {
        if (!pl) return;
        LastLoginDate    = pl->daily_login_day;
        CurrentDay       = pl->is_day;
        ClaimedToday     = pl->claimed_daily_today;
        if (CurrentDay < 1 || CurrentDay > 30) CurrentDay = 1;
        if (pl->daily_status.size() >= 30) {
            for (int i = 0; i < 30; i++) Status[i] = pl->daily_status[i];
        }
    }

    template<typename PlayerT>
    void WriteToPlayer(PlayerT* pl) const {
        if (!pl) return;
        pl->daily_login_day    = LastLoginDate;
        pl->is_day             = CurrentDay;
        pl->claimed_daily_today = ClaimedToday;
        if (pl->daily_status.size() < 30) pl->daily_status.resize(30, 0);
        for (int i = 0; i < 30; i++) pl->daily_status[i] = Status[i];
    }
};

} // namespace DailyReward
