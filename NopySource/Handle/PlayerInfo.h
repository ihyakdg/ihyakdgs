#pragma once
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <random>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

struct QuizQuestion {
	std::string question;
	std::string options[4];
	int correct;
};

static QuizQuestion GenerateRandomQuiz() {
	QuizQuestion q{};
	int tier = rand() % 3;
	int type = rand() % 2;

	if (tier == 0) {
		int x = rand() % 20 + 1;
		int y = rand() % 20 + 1;
		int ans = x * y;
		q.question = "Jika x = " + std::to_string(x) + " dan y = " + std::to_string(y) + ", berapa nilai x × y?";
		int base = ans - 10;
		for (int i = 0; i < 4; i++) q.options[i] = std::to_string(base + (i * 5));
		q.correct = (ans - base) / 5;
	}
	else if (tier == 1) {
		int x = rand() % 10 + 2;
		int y = rand() % 5 + 1;
		int ans = (x * x) + y;
		q.question = "Jika x = " + std::to_string(x) + ", hitung x² + " + std::to_string(y);
		q.options[0] = std::to_string(ans);
		q.options[1] = std::to_string(ans + 3);
		q.options[2] = std::to_string(ans - 2);
		q.options[3] = std::to_string(ans + 5);
		q.correct = 0;
	}
	else {
		int a = rand() % 5 + 2;
		int b = rand() % 10 + 1;
		int ans = a * b;
		q.question = "Sebuah toko menjual " + std::to_string(a) + " kotak, masing-masing berisi " + std::to_string(b) + " barang. Total barang?";
		q.options[0] = std::to_string(ans);
		q.options[1] = std::to_string(ans + b);
		q.options[2] = std::to_string(ans - a);
		q.options[3] = std::to_string(ans + 10);
		q.correct = 0;
	}

	return q;
}
struct GhostPriceData {
	int sell_mode;
	int sell_price;
	int buy_mode;
	int buy_price;
};
struct PlayerBot {
	std::string world_name;
	int netID;
	float x;
	float y;
	std::string text;
	bool active;
	long long last_spam_time;
};

std::map<std::string, PlayerBot> bots;
struct AskAllEvent {
	bool active = false;
	std::string question;
	std::string answer;
	int winners = 1;
	std::unordered_set<std::string> winnerList;
	std::vector<std::pair<int, int>> prizes; // itemID, amount
};

AskAllEvent g_askall;


static const std::string liveFile = "database/json/live.json";

json loadLiveDB() {
	std::ifstream f(liveFile);
	if (!f.good()) return json{ {"players", json::object()} };
	json j; f >> j; return j;
}

void saveLiveDB(const json& j) {
	static std::mutex live_db_mtx;
	std::lock_guard<std::mutex> lock(live_db_mtx);
	std::ofstream f(liveFile);
	if (f.is_open()) {
		f << j.dump(4);
		f.close();
	}
}

inline int get_next_uid() {
	static std::mutex uid_counter_mtx;
	std::lock_guard<std::mutex> lock(uid_counter_mtx);
	int next_uid = 1;
	std::string path = "database/json/uid_counter.json";
	json j;
	std::ifstream file(path);
	if (file.is_open()) {
		try {
			file >> j;
			if (j.contains("next_uid") && j["next_uid"].is_number_integer()) {
				next_uid = j["next_uid"].get<int>();
			}
		} catch (...) {}
		file.close();
	}
	j["next_uid"] = next_uid + 1;
	std::ofstream out(path);
	if (out.is_open()) {
		out << j.dump(4);
		out.close();
	}
	return next_uid;
}
struct ExchangeEventData {
	bool fish_exchange = false;
	bool crystal_exchange = false;

	void save() {
		json j;
		j["fish"] = fish_exchange;
		j["crystal"] = crystal_exchange;

		std::ofstream file("database/json/eventexchange.json");
		if (file.is_open()) {
			file << j.dump(4);
			file.close();
		}
	}

	void load() {
		std::ifstream file("database/json/eventexchange.json");
		if (!file.is_open()) return;

		json j;
		file >> j;

		fish_exchange = j.value("fish", false);
		crystal_exchange = j.value("crystal", false);
	}
};

ExchangeEventData exchangeData;

struct KitItem {
	int item_id = 0;
	int count = 1;
	int level_req = 0;
};

struct KitsConfig {
	std::vector<KitItem> kits;

	void save() {
		json j = json::array();
		for (const auto& kit : kits) {
			json kj;
			kj["item_id"] = kit.item_id;
			kj["count"] = kit.count;
			kj["level_req"] = kit.level_req;
			j.push_back(kj);
		}
		std::ofstream file("database/json/kits_config.json");
		if (file.is_open()) {
			file << j.dump(4);
			file.close();
		}
	}

	void load() {
		kits = {
			{14896, 1, 5},
			{5196, 10, 10},
			{14360, 1, 15},
			{9266, 15, 20},
			{6282, 1, 25},
			{14990, 1, 30},
			{14500, 1, 35},
			{11050, 1, 40},
			{3134, 1, 45},
			{14552, 50, 50},
			{14358, 1, 55},
			{13198, 1, 60},
			{14084, 15, 65},
			{14498, 1, 70},
			{10944, 1, 75},
			{7960, 15, 80},
			{10382, 10, 90},
			{5930, 1, 100}
		};

		std::ifstream file("database/json/kits_config.json");
		if (!file.is_open()) {
			save();
			return;
		}

		json j;
		try {
			file >> j;
			if (j.is_array() && j.size() == 18) {
				for (size_t i = 0; i < 18; ++i) {
					kits[i].item_id = j[i].value("item_id", kits[i].item_id);
					kits[i].count = j[i].value("count", kits[i].count);
					kits[i].level_req = j[i].value("level_req", kits[i].level_req);
				}
			}
		} catch (...) {
			save();
		}
	}
};

KitsConfig kitsConfig;

inline std::vector<std::pair<int, std::string>> LoadBlastDesigner() {
	std::vector<std::pair<int, std::string>> result;
	std::string path = "database/json/blastdesigner.json";
	if (!std::filesystem::exists(path)) return result;

	try {
		std::ifstream file(path);
		if (!file.is_open()) return result;

		json j;
		file >> j;
		file.close();

		if (!j.is_object() || !j.contains("blasts")) return result;
		if (!j["blasts"].is_array()) return result;

		for (auto& b : j["blasts"]) {
			if (!b.is_object()) continue;
			int id = 0;
			std::string world;
			if (b.contains("id") && b["id"].is_number_integer()) id = b["id"].get<int>();
			if (b.contains("world") && b["world"].is_string()) world = b["world"].get<std::string>();
			if (id > 0 && !world.empty()) result.push_back({ id, world });
		}
	}
	catch (...) {
		return {};
	}
	return result;
}

inline void SaveBlastDesigner(const std::vector<std::pair<int, std::string>>& blasts) {
	json j;
	j["blasts"] = json::array();
	for (auto& p : blasts) {
		json b;
		b["id"] = p.first;
		b["world"] = p.second;
		j["blasts"].push_back(b);
	}
	std::filesystem::create_directories("database");
	std::ofstream file("database/json/blastdesigner.json");
	if (!file.is_open()) return;
	file << j.dump(4);
	file.close();
}
inline bool IsBlastItem(int id) {
	// Cek blastdesigner JSON dulu
	std::vector<std::pair<int, std::string>> blasts = LoadBlastDesigner();
	for (auto& p : blasts) {
		if (p.first == id) return true;
	}
	// Fallback: blast ID bawaan server yang hardcoded di create_world_blast
	// Kalau belum di-override di blastdesigner, tetap bisa dipakai
	static const std::vector<int> DEFAULT_BLASTS = {
		9164,  // World Locket Blast
		1402,  // Basic Blast
		1060,  // Bountiful Blast
		8738,  // Sandy Blast
		8556,  // Hospital Blast
		6420,  // Another default
		1532,  // Undersea Blast
		830,   // Summer Blast
		10380, // Custom Blast
		9602,  // Farm Blast
		1136,  // Mars Blast
	};
	for (int def : DEFAULT_BLASTS) {
		if (def == id) return true;
	}
	return false;
}
inline bool IsBlastWorldFromJson(std::string worldName) {
	if (worldName.empty()) return false;

	std::transform(worldName.begin(), worldName.end(), worldName.begin(), ::toupper);

	auto blasts = LoadBlastDesigner();
	for (auto& p : blasts) {
		std::string w = p.second;
		std::transform(w.begin(), w.end(), w.begin(), ::toupper);
		if (w == worldName) return true;
	}
	return false;
}
inline bool GetCustomBlastWorld(int blastId, std::string& worldName) {
	auto blasts = LoadBlastDesigner();
	for (auto& p : blasts) {
		if (p.first == blastId) {
			worldName = p.second;
			return true;
		}
	}
	return false;
}

// Mengembalikan default world template name untuk blast bawaan server.
// Blast procedural (9164, 1402, dll) mengembalikan string kosong (tidak pakai template world).
inline std::string GetDefaultBlastWorldName(int blastId) {
	static const std::unordered_map<int, std::string> DEFAULT_BLAST_WORLDS = {
		{10380, "GENERATE_HUGE_WORLD"},
		{1532,  "GENERATED_UNDERSEA_BLAST99971"},
		{830,   "GENERATED_SUMMER_BLAST464"},
		{9602,  "GENERATED_FARM_BLAST111"},
		{1136,  "GENERATED_MARS_BLAST_15"},
		{3562,  "GENERATED_CAVE_BLAST_1554"},
		{9582,  "GENERATED_MINECRAFT_WORLD_1647"},
		{14896, "GENERATED_CANDYLAND_BLAST"},
		{7588,  "GENERATED_TREASURE_BLAST_156"},
		{942,   "GENERATED_DESRT_BLAST"},
		{4774,  "GENERATE_JUNGLE_BLAST_156"},
		{7380,  "GENERATE_MONOCHROME_BL_15"},
	};
	auto it = DEFAULT_BLAST_WORLDS.find(blastId);
	if (it != DEFAULT_BLAST_WORLDS.end()) return it->second;
	return "";
}
struct AdventureItem {
	int id = 0;
	int pos = -1;
	int uid = -1;
};
struct Recipe {
	std::vector<std::pair<int, int>> bahan;
	std::pair<int, int> hasil;
};
struct PlayMods {
	uint16_t id = 0;
	string user = "";
	long long time = 0;
};
struct Friends {
	string name = "";
	long long last_seen = 0;
	bool mute = false, block_trade = false;
};
struct PlayerRodData { int rod_level = 0; };

static std::unordered_map<int, PlayerRodData> rodData; static std::unordered_map<int, int> rodUpgradeCosts;
struct Market_Place {
	int notifications = 0;
	string fg = "", storeName = "", whose = "";
};
// Moderation
struct {
	bool CURSED = false;
	bool BANNED = false;
	long long CURSE_EXPIRE = 0;
	long long BAN_EXPIRE = 0;
} Role;
struct RedeemCode_ {
	std::string code = "";
	long long int expired = 0, created = 0;
};
void to_json(json& j, const RedeemCode_& p) {
	j = json{ {"code", p.code}, {"expired", p.expired}, {"created", p.created} };
}
void to_json(json& j, const PlayMods& p) {
	j = json{ {"id", p.id}, {"time", p.time}, {"user", p.user} };
}
void to_json(json& j, const Friends& p) {
	j = json{ {"name", p.name}, {"mute", p.mute}, {"block_trade", p.block_trade}, {"last_seen", p.last_seen} };
}
void from_json(const json& j, RedeemCode_& p) {
	j["code"].get_to(p.code);
	j["expired"].get_to(p.expired);
	j["created"].get_to(p.created);
}
void from_json(const json& j, Friends& p) {
	j["name"].get_to(p.name);
	j["mute"].get_to(p.mute);
	j["last_seen"].get_to(p.last_seen);
	j["block_trade"].get_to(p.block_trade);
}
void from_json(const json& j, PlayMods& p) {
	j["id"].get_to(p.id);
	j["time"].get_to(p.time);
	j["user"].get_to(p.user);
}
enum GTPS_NPC { NPC_EXCHANGE = -777, NPC_MARKETPLACE = -888 };
std::vector<int> getNpcSpawnPos(GTPS_NPC npcID) {
	std::vector<int> npcPos = { 0, 0 };
	switch (npcID) {
	case NPC_EXCHANGE: npcPos = { 1668, 834 }; break;
	case NPC_MARKETPLACE: npcPos = { 1262, 1154 }; break;
	}
	return npcPos;
}
std::string getNPCNickname(GTPS_NPC npcID) {
	switch (npcID) {
	case NPC_EXCHANGE: return "`w[`2NPC`w] Exchange Place";
	case NPC_MARKETPLACE: return "`w[`2NPC`w] MarketPlace";
	default: return "";
	}
}
struct BackpackItem {
	int itemID;
	int amount;
};
struct MathQuestion {
	std::string question;
	std::vector<std::string> options;
	int correctIndex = 0;
};

struct MathReward {
	int itemID;
	int chance;
};

std::vector<MathReward> mathRewards = {
	{7188, 50},
	{13158, 50},
	{9350, 30},
	{14676, 10},
	{14682, 25},
	{19220, 5},
	{1458, 1}
};
static MathQuestion GenerateMathQuestion() {
	int a = rand() % 10 + 1;
	int b = rand() % 10 + 1;
	int op = rand() % 3;
	MathQuestion q;
	int ans = 0;

	if (op == 0) {
		q.question = "Berapakah hasil " + std::to_string(a) + " + " + std::to_string(b) + "?";
		ans = a + b;
	}
	else if (op == 1) {
		q.question = "Berapakah hasil " + std::to_string(a) + " x " + std::to_string(b) + "?";
		ans = a * b;
	}
	else {
		b = (b == 0 ? 1 : b);
		q.question = "Berapakah hasil " + std::to_string(a * b) + " / " + std::to_string(b) + "?";
		ans = a;
	}

	q.correctIndex = rand() % 4;
	q.options.resize(4);
	for (int i = 0; i < 4; i++) {
		if (i == q.correctIndex) q.options[i] = std::to_string(ans);
		else q.options[i] = std::to_string(ans + ((rand() % 5) + 1));
	}
	return q;
}
struct ShopItem {
	int id;
	int price_id;
	int price_amt;
	int chance;
	int min_stock;
	int max_stock;
};

static std::map<int, int> shopStock;
static time_t nextReset = 0;

std::vector<ShopItem> shopItems = {
	{13158, 1796, 15, 100, 1, 20},
	{10400, 1796, 30, 50, 1, 1},
	{10716, 1796, 30, 100, 1, 15},
	{5136, 1796, 30, 100, 1, 15},
	{14084, 1796, 30, 100, 1, 15},
	{14596, 1796, 30, 100, 1, 15},
	{7960, 1796, 30, 100, 1, 15},
	{9350, 1796, 30, 100, 1, 15},
	{14676, 7188, 10, 20, 1, 5},
	{14682, 7188, 10, 20, 1, 5},
	{19220, 8470, 10, 2, 1, 1},
	{1458, 7188, 20, 5, 1, 3},
	{15874, 8470, 1, 1, 1, 1}
};

static bool discoActive = false;
static std::string discoWorld = "";


struct Player {
	// FIX EXPLOIT (anti-spoof dialog_return)
	// Set dialog name yang sedang valid (server baru saja kirim OnDialogRequest dengan
	// `end_dialog|<name>|...`). Diisi otomatis di VarList::OnDialogRequest /
	// OnDialogRequestRML / on_dialog. Diperiksa di Action::dialog_return.
	// TTL 60 detik supaya executor tidak bisa "menyimpan" dialog open lama.
	std::unordered_map<std::string, long long> pending_dialogs; // name -> expiry_ms
	long long last_refresh_item_data_ms = 0; // FIX DOS: rate-limit action refresh_item_data
	int pending_gem_collected = 0;
	long long last_gem_collect_time = 0;
	int gem_collect_queue_delay = 0;
	long long last_gem_collect_ms = 0;

	bool just_registered = false;
	bool new_get_claimed = false;
	bool infinite_mode = false;
	int sawit_tree = 0;
	int sawit_fruit = 0;
	int sawit_rewarded = 0;
	int last_puasa_day = 0;
	int last_teraweh_day = 0;
	int puasa_streak = 0;
	int teraweh_streak = 0;
	bool usedReferral = false;
	std::string last_claim_ip;
	std::string last_claim_rid;
	std::string partner_name = "";
	int hp = 100;
	long long the_world_time = 0;
	bool autosellfish = false;
	bool autosellfish_processing = false;
	bool autoConvert = false;
	bool autoConvertProcessing = false;
	bool autoCvModeGemsToDl = false;
	bool autoCvModeWlToDl = false;
	bool autoCvModeDlToBgl = false;
	bool autoCvModeBglToGgl = false;
	bool autoCvModeGglTo20298 = false;   // 8470 (mgl) -> 20298 (direct conversion)
	int backpackLevel = 1;
	bool backpack_untrade_unlocked = false; // Unlocked by paying 10 LGL
	bool is_paired = false;
	int pair_quest_type = 0;
	int pair_quest_goal = 0;
	int pair_quest_progress = 0;
	int pair_count = 0;
	int pair_charisma = 0;
	long long pair_daily_reset = 0; // Untuk cek hari
	int pair_daily_count = 0;
	std::vector<std::pair<int, int>> backpackcool; // {itemID, count}
	bool is_live = false;
	std::string live_platform = "";
	std::string live_name = "";
	std::vector<BackpackItem> backpack;
	long long csn_buff_end_time = 0;
	bool has_csn_buff = false;
	std::vector<std::string> roles;
	// WinterFest Bingo
	bool Has_WinBingo = false;
	std::map<std::string, std::pair<int, int>> WinterBingo_Prize{ {"r0", {0, 0}}, {"r1", {0, 0}}, {"r2", {0, 0}}, {"r3", {0, 0}}, {"r4", {0, 0}}, {"c0", {0, 0}}, {"c1", {0, 0}}, {"c2", {0, 0}}, {"c3", {0, 0}}, {"c4", {0, 0}} };
	std::map<std::string, int> WinterBingo_Task{ {"0", 0},{"1", 0},{"2", 0},{"3", 0},{"4", 0},{"5", 0},{"6", 0},{"7", 0},{"8", 0},{"9", 0},{"10", 0},{"11", 0},{"12", 0},{"13", 0},{"14", 0},{"15", 0},{"16", 0},{"17", 0},{"18", 0},{"19", 0},{"20", 0},{"21", 0},{"22", 0},{"23", 0},{"24", 0} };
	vector<int> Has_BingoTask{};
	std::vector<std::string> Has_ClaimBingo{};
	std::vector<std::tuple<std::string, int, int>> Has_BingoPrize;

	steady_clock::time_point lastPacketTime;
	int cDailyDay = -1;
	int daily_streak = 0;          // jumlah hari berturut-turut claim (1-30)
	time_t daily_last_claim = 0;   // timestamp claim terakhir (untuk cek 24 jam)
	bool daily_loop = true;        // loop reward setelah hari 30 ke hari 1
	bool daily_notif_shown = false; // flag session: notifikasi pengingat daily sudah ditampilkan
	// Account Notes
	std::vector<std::string> Account_Notes{};

	// Gems Storage
	long long int Gems_Storage = 0;
	//dc
	//discord things
	bool discordVerified = false;
	bool accaap = false;
	string discordId = "";
	string discordDis = "";
	string discordTag = "";
	bool discordPending = false;
	// Warning System by LyChie
	int Warning = 0, Warning_Spam = 0;
	std::vector<std::string> Warning_Message{};
	std::vector<std::string> WM{};

	// Mod Setting by LyChie
	bool npc_summon = false, effworld_ = false;
	int x_npc = 0, y_npc = 0, npc_netID = 0;
	int effworld = 0, cheat_bot_spam = 0, cheat_pull = 0, cheat_qq = 0, cheat_reme = 0, cheat_kick = 0, cheat_ban = 0, cheat_detect = 0, cheat_unacc = 0, cheat_hideother = 0, cheat_hidechat = 0, cheat_spam = 0;
	int Cheat_Spam_Delay = 1, Cheat_Last_Spam = 0;
	std::string Cheat_Spam_Text = "", npc_text = "", npc_name = "", npc_world = "";

	// Title
	struct {
		bool Mentor = false, Grow4Good = false, Doctor = false, OfLegend = false, TiktokBadge = false, ContentCBadge = false, ThanksGiving = false, OldTimer = false, WinterSanta = false, PartyAnimal = false, GrowPass_Bronze = false, GrowPass_Silver = false, GrowPass_Gold = false, Award_Winning = false;
	} Title;

	// Role
	struct {
		int Role_Level = 0;
		std::string custom_role_name = "";
		long long int SUPER_BOOST_TIME = 0, BOOST_TIME = 0;
		bool Owner_Server = false, Unlimited = false, God = false, Developer = false, Administrator = false, Moderator = false, Vip = false, Cheats = false, BOOST = false, SUPER_BOOST = false, Coder = false, Streamers = false, Staff = false, has_config_access = false, Donatur = false;
	} Role;

	// Kit by LyChie
	bool startedKit = false;
	std::vector<std::string> KitPass_Prize;
	int lvlKit = 0, lvlKit_S2 = 0, xpKit = 0, xpKit_S2 = 0;

	// Marketplace by LyChie
	int buyp = 0, store_price = 0, store_id = 0;
	std::string store_pos = "", store_desc = "";

	// Spotify
	int spotify = 0;

	//dialogdatattrash
	std::map<std::string, std::string> dialog_data;

	//find next
	std::string last_find = "";
	std::map<uint16_t, std::vector<std::pair<uint32_t, bool>>> items_match = {};

	// battle pet by LyChie
	std::pair<std::vector<uint16_t>, std::vector<uint16_t>> battle_pet{ {0, 0, 0, 1}, { 0,0,0,0 } };

	// adventure
	std::vector<AdventureItem> adventure_item;

	// Redeem Code by LyChie
	long long last_redeem = 0;
	std::vector<std::string> Has_Claim{};
	std::vector<std::pair<int, int>> r_items{};
	std::vector<RedeemCode_> redeem_code{};

	// New Get
	std::vector<std::pair<int, int>> n_items{};

	// Tutorial by LyChie
	bool new_player = false;
	bool isStopTime = false;
	//spambyalbin

	struct SpamData {
		std::string text;
		int delay;
		bool active = false;
	};
	SpamData spam_data;

	//allaalaalala
	std::vector < std::string> args;

	//jsjjds
	int lwiz_complate = 0;

	//alala
	int currentItem = 18; // default ke fist jika belum ada

	// Pet Ai by LyChie
	std::string pet_name = "";
	int pet_type = -1;
	float pet_move = 0.0f;
	long long int pet_xp = 0;
	int Pets_Death_Times = 0, Pets_hunger = 150, Pets_health = 100, Pets_Builder_Lvl = 0, Pets_Farmer_Lvl = 0, Pets_Gems_Lvl = 0, Pets_Xp_Lvl = 0, pet_netID = 0, pet_ID = 0, pet_level = 0, ability_xgems = 0, ability_xxp = 0;
	bool Pets_Not = false, Pets_Not2 = false, Pets_Not3 = false, Pets_Dead = false, show_pets = false, pet_ClothesUpdated = false, hidden_pets_name = false, master_pet = false, active_bluename = false, active_1hit = false, LiyueHtouFly = false, AbyssMageFly = false, UuuzzFly = false, RandomSentences = false;

	// Guild Event
	std::vector<std::string> Guild_Prize;
	int last_personal_contri = 0, last_rank_per = 0, last_guild_contri = 0, last_rank_guild = 0;
	int spring_token = 0, summer_token = 0, winter_token = 0, Legendary_Keys = 0;
	bool next_guild_event = false, raw5_legendary_claim = false, participated_personal = false;
	bool row1_items1_claim = false, row1_items2_claim = false, row1_items3_claim = false, row1_items4_claim = false, row1_items5_claim = false, row1_items6_claim = false;
	bool row2_items1_claim = false, row2_items2_claim = false, row2_items3_claim = false, row2_items4_claim = false, row2_items5_claim = false, row2_items6_claim = false;
	bool row3_items1_claim = false, row3_items2_claim = false, row3_items3_claim = false, row3_items4_claim = false, row3_items5_claim = false, row3_items6_claim = false;
	bool row4_items1_claim = false, row4_items2_claim = false, row4_items3_claim = false, row4_items4_claim = false, row4_items5_claim = false, row4_items6_claim = false;
	bool claim_limited_clash = false, claim_top1 = false, claim_top2 = false, claim_top3 = false, claim_top4 = false, claim_top5 = false, claim_top6 = false, claim_top7 = false, claim_top8 = false, claim_top9 = false, claim_top10 = false, claim_top11 = false, claim_top12 = false, claim_top13 = false, claim_top14 = false, claim_top15 = false, claim_top16 = false, claim_top17 = false, claim_top18 = false, claim_top19 = false, claim_top20 = false, claim_top1g = false, claim_top2g = false, claim_top3g = false, claim_top4g = false, claim_top5g = false, claim_top6g = false, claim_top7g = false, claim_top8g = false, claim_top9g = false, claim_top10g = false, claim_top11g = false, claim_top12g = false, claim_top13g = false, claim_top14g = false, claim_top15g = false, claim_top16g = false, claim_top17g = false, claim_top18g = false, claim_top19g = false, claim_top20g = false, claim_top21g = false, claim_top22g = false, claim_top23g = false, claim_top24g = false, claim_top25g = false;

	// Punch Event
	bool top_punch_1 = false, top_punch_2 = false, top_punch_3 = false, top_punch_4 = false, top_punch_5 = false;

	//TUTORIAL BY AZZAM
	string last_tutorial_world = "";
	int Tutorial_Number = 1, Need_To_Required = 0, Requiring = 1;
	bool completed_tutorial = false, in_world_tutorial = false;
	// Winter
	std::vector<std::string> WinterTask_Prize, Winter_Branch_Rewards;
	long long int Winter_Daily_Time = 0, Present_Goblin = 0;
	bool Already_Tried_Gob = false, ability_upgrade_ances = false, winter_power = false, Bronze_Donatur = false, Silver_Donatur = false, Gold_Donatur = false, Diamond_Donatur = false;
	int winter_seal = 0, winter_seal_branch_1 = 0, winter_seal_branch_2 = 0, winter_seal_branch_3 = 0, winter_seal_branch_4 = 0, staged = 0, s4tb = 0, winterfest_wishes = 0, winterfest_quest_1 = 0, winterfest_quest_2 = 0, winterfest_quest_3 = 0, reset_winterfest_quest_1 = 0, reset_winterfest_quest_2 = 0, reset_winterfest_quest_3 = 0, reset_winterfest_quest_4 = 0, reset_winterfest_quest_5 = 0, reset_winterfest_quest_6 = 0, reset_winterfest_quest_7 = 0, reset_winterfest_quest_8 = 0, reset_winterfest_quest_9 = 0;

	// Halloween
	std::vector<std::string> HalloweenTask_Prize;
	int DarkKing_Sacrifices = 0, halloween_q1 = 0, halloween_q2 = 0, halloween_q3 = 0, halloween_q4 = 0, halloween_q5 = 0, halloween_q6 = 0, halloween_q7 = 0, halloween_q8 = 0, halloween_q9 = 0;

	// Marvelous Mission
	int last_choose_tab = 0;
	bool claim_myth_1 = false, claim_myth_2 = false, claim_myth_3 = false, claim_myth_4 = false, claim_myth_5 = false, claim_myth_6 = false;
	bool claim_seven_seas_1 = false, claim_seven_seas_2 = false, claim_seven_seas_3 = false, claim_seven_seas_4 = false, claim_seven_seas_5 = false, claim_seven_seas_6 = false;

	// Achievments
	int achievement_total = 0;
	int builder_achi = 0, farmer_achi = 0, demolition_achi = 0, packrat_achi = 0, big_spender_achi = 0, trashman_achi = 0, embiggened_achi = 0, all_mine_achi = 0, ding_achi = 0, milkin_achi = 0, surgeon_achi = 0, legen_achi = 0, social_b_achi = 0;
	int pro_builder_achi = 0, farmer_itd_achi = 0, wrecking_achi = 0, hoarder_achi = 0, moneybags_achi = 0, givin_ah_achi = 0, space_cmd_achi = 0, let_troi_achi = 0, long_tf_achi = 0, moo_juice_achi = 0, chief_os_achi = 0, wait_fi_achi = 0;
	int expert_builder = 0, farmasaurus_rex = 0, annihilator = 0, filthy_rich = 0, one_meelion = 0, ocd = 0, master_of_space = 0, this_is_my_land = 0, obsessive_growtopian = 0, supporter_achi = 0, kevin_bacon = 0, wotw_achi = 0, collector_extra = 0, surgeon_general = 0, DARY_achi = 0, guild_leader_achi = 0;
	int berserk = 0, the_special = 0, crazy_farmer = 0, millionare = 0, void_achi = 0, mad_shopper = 0, show_off = 0, ssup_achi = 0, providers_wp = 0, the_doctor = 0;
	int ances_being = 0, a_highter_p = 0, power_overwhelming = 0, big_shows = 0;

	// epic quest
	int epic_quest_total = 0;
	int epicq_stonefish = 0, epicq_woolcraft = 0, epicq_octopushead = 0, epicq_goldenaxe = 0, epicq_ddrag = 0, epicq_riding = 0, epicq_compactdress = 0, epicq_dressup = 0, epicq_lvl100 = 0, epicq_plant_wiz = 0, epicq_emeraldlock = 0, epicq_ghostcharm = 0, epicq_teddybear = 0, epicq_splice_wm_comet = 0;

	// Sewing Machine
	std::string sew1 = "", sew2 = "", sew3 = "";
	int lastsew = 0;
	std::string sewtype = "";
	int sewunlocked = 0;
	int totalsew = 0;

	// Buy
	std::string last_name_item = "";
	int last_price_item = 0, last_id_item = 0;

	std::unordered_map<std::string, std::function<void(ENetPeer*, const std::string&, int)>> defaultCommandHandlers;

	// Gacha by LyChie
	std::string gacha_type = "", Has_Type = "";
	int Intertwined_Fate = 0, Acquaint_Fate = 0, Pity = 0;
	bool Pity_On = false, Has_In = true;
	std::vector<std::string> Gacha_History{};

	// Npc by LyChie
	int last_netid = 0;
	bool isNpcPromotionCalling = false, isNpcTipsCall = false, isNpcBfg = false, isNpcWelcomeLoad = false, isNpcTrading = false;

	// Daily Login by LyChie
	int is_day = 0;
	bool claimed_daily_today = false;
	long long int daily_login_day = 0;
	std::vector<uint8_t> daily_login;
	std::vector<int> daily_status = std::vector<int>(30, 0);

	std::unordered_map<int, int> blockBreaks;

	std::string rawName = "";


	int StopTime = 0;
	long long frozenUntil = 0;

	bool isFrozen = false;
	bool isInvisible = false;
	time_t freezeEnd = 0;

	// Vip Daily Rewards
	std::vector<uint8_t> vip_daily;
	bool vip_claimed_daily = false;
	long long int vip_daily_day = 0;
	int vip_day = 0;

	// Level Rewards
	std::vector<uint8_t> lvl_rewards;

	static bool isNumeric(const std::string& s) {
		return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
	}

	// Bank
	int bank_password = 0;
	bool Has_Enter_Bank = false;
	std::vector<std::string> Bank_History{};
	bool bank_in_transaction = false;    // [ANTI-DUPE] flag untuk cegah concurrent bank ops
	time_t bank_last_op = 0;             // [ANTI-SPAM] timestamp operasi bank terakhir
	int32_t wl_bank_amount = 0, dl_bank_amount = 0, bgl_bank_amount = 0, ggl_bank_amount = 0, mgl_bank_amount = 0;
	long long int donated_wl = 0; // total WL donated in Donation Server Event

	// Life Goals
	long long int lifegoals_reset_day = 0;
	int awesomeness = 0, seed_goal = 0, gem_goal = 0, exp_goal = 0;
	/*GIVENUP*/bool lifegoals1 = true, lifegoals2 = true, lifegoals3 = true;
	/*COMPLETED*/bool lifegoals_1 = false, lifegoals_2 = false, lifegoals_3 = false;

	// Role Quest
	long long int rolequest_reset_day = 0;
	bool role_quest = false, complete_rolequest_day = false;
	std::string role_quest_type = "", specialization_track = "", quest_text = "";
	int total_quest_selected = 0, quest_required = 0, need_required = 0, quest_number = 0, need_sumbit_req_id = 0, need_item_req_id = 0;
	uint8_t roleIcon = 6, roleSkin = 6;
	struct {
		bool Farmer = false, Builder = false, Surgeon = false, Fisher = false, Chef = false, Startopian = false;
	} RolesTitle;

	// Biweekly Quest
	bool biweekly_completed = false;
	long long int biweekly_reset_day = 0;
	int BiweeklyQ_1 = 0, BiweeklyQ_2 = 0, BiweeklyQ_3 = 0;

	// Anti Cheat
	float last_x = 0.0f;
	float last_y = 0.0f;
	int warningCount = 0;
	int64_t last_movement_time = 0;
	bool is_afk_sleeping = false;
	float speed = 1.0f;

	// Anti PNB Spam
	struct AntiFarmSpam {
		std::uint64_t lastBreakTile = 0;
		int totalWarning = 0;
		std::uint64_t lastTimedout = 0;
	} antiFarmSpam;

	// Autofarm
	bool autofarm = false;
	int autofarm_slot = 1, autofarm_x = -1, autofarm_y = -1, Slot_Far = 1;


	// Chat Spammer
	bool chat_spammer = false;
	std::string spam_message = "Hello World";
	int spam_interval = 3000;
	std::int64_t last_spam_time = 0;

	// Adventure 
	int lives = 0;
	std::int64_t timers = 0;
	std::int64_t totalTime = 0;
	bool timerActive = false, adventure_begins = false, adventure_ends = false;

	// Mines Game
	int foundg = 0, minesleft = 0;
	bool lostt = false;
	bool endedit = true;
	std::vector<int> mines{};
	std::vector<int> clickm{};
	int bM = 0;

	// Mining
	int Mines_Score = 0, Number_License = 1, mines_coin = 0, Has_Reads = 0;
	long long int MiningTime = 0;
	bool In_World_Mining = false, Has_Read_ = false, Has_Read_2 = false;

	// Change Address
	bool AddressHasTrue = false;
	std::string AddressName = "";

	// Masterless Starglitterb Quest
	int Masterless_Starglitter = 0;
	long long int St_Regen_Time = 0;
	std::vector<std::string> StarglitterTask_Prize;
	bool starglitter_enter = false;
	uint16_t starglitter_seed = 0;
	uint8_t starglitter_30mins = 0, starglitter_seed2 = 0, starglitter_combine = 0, starglitter_crystal = 0;
	int starglitter_points = 0, starglitter_email = 0, starglitter_surgery = 0, starglitter_fish = 0, starglitter_break = 0, starglitter_place = 0, starglitter_trade = 0, starglitter_sb = 0, starglitter_provider = 0, starglitter_provider2 = rand() % 450 + 1, starglitter_geiger = 0, starglitter_geiger2 = 0;
	long long int starglitter_reset_day = 0, starglitter_reset_week = 0, starglitter_reset_month = 0;

	// Dr. Desctruto
	bool DrDes = false;
	int Health_DrDes = 100;

	// Proxy
	struct Proxys {
		bool reme = false;
		bool roulette = false;
		bool fastspin = false;
		bool spin = false;
		bool real = false;
	} proxy;

	// Verify Email
	bool AccountSecured = false, Has_Verifed = false;

	bool isGhost = false; // Tambahan untuk menyimpan status ghost

	//morph
	int morphed_item_id = 0; // 0 = tidak morph

	// Custom Wrench
	std::string TitleTexture = "", TitleCoordinate = "";
	std::vector<int> Wrench_Style, Wrench_Decoration;
	int wrench_foreground_custom = 0, wrench_custom = 0, Set_Wrench_Fg = 0, Set_Wrench_Bg = 0;

	// Event Wls
	bool has_claim_event_wls = false;

	// Background Color Settings
	std::string border_color = "", bg_color = "";

	// Earn Free Gems
	struct {
		long long int Time = 0;
		int Quest_1 = 0, Quest_2 = 0, Quest_3 = 0;
		std::vector<int> Claim;
	} EarnFreeGems;

	int chem_price_wl = 0, chem_price_dl = 0, chem_price_bgl = 0;

	// Anniversary Event
	bool anniversary_participated = false;

	// Worlds
	struct AntiWorlds {
		bool loadAssets = false;
		std::mutex loadAssetsMutex;
	} worldSecurity;

	struct { int lastActionSent = 0, totalActionSent = 0; } packetSent;
	std::vector<std::pair<std::string, long long int>> Used_VipDoor;
	std::string page_type = "eng", page_music = "1", login_pass = "", recovery_token = "", player_token = "", last_choose_music = "", tb_1or2 = "", nm = "", ds = "", fg = "", last_name = "", last_pos = "", lastFind = "", newname = "", ip = "", fa_ip = "", name_color = "`0", last_world = "", last_online = "", note = "", CapeStyleColor_1 = "147,56,143", CapeCollarColor_1 = "147,56,143", CapeStyleColor_2 = "137,30,43", CapeCollarColor_2 = "34,35,36", skin_c = "0,0,0", _CapeStyleColor_1 = "93,22,200", _CapeCollarColor_1 = "220,72,255", _CapeStyleColor_2 = "137,30,43", _CapeCollarColor_2 = "34,35,63", tankIDName = "", tankIDPass = "", requestedName = "", world = "", email = "", country = "", home_world = "", last_wrenched = "", last_msg_wrenched = "", info_new_ = "", d_name = "", temporary_vault = "", Carnival_GameType = "None", b_r = "", b_b = "", m_b = "", DeathRaceWinner = "", world_owner = "", page_item = "", musical_note = "C-F-G#G-F-B#A#G-F-G#G-D#G-C-", surgery_world = "", surged_person = "", surged_display = "", surgery_name = "", pretext = "", fixtext = "", postext = "", scantext = "", tooltext = "The patient is prepped for surgery.", endtext = "", player_device = "", device_name = "", wolf_world = "", chat_prefix = "", ban_reason = "", last_home_world = "", game_version = "", custom_title = "";
	bool show_custom_title = true, In_World_Clash = false, is_guest = false, Has_Login = false, in_enter_game = false, isindialog = false, Is_In_Dialog = true, ignore_cheats = false, isFlagged = false, Has_Reads_ = false, unlock_card_license = false, trade_accept = false, accept_the_offer = false, growid = false, wiped_invalid_worlds = false, ttBadge = false, ccBadge = false, has_logon = false, is_verified = false, ghost = false, no_clip = false, invis = false, DeathRaceBeaten = false, BrutalBeaten = false, SpikyBeaten = false, NotSame = false, mini_you = false, no_body = false, no_hands = false, Crown_Floating_Effect_0 = false, Crown_Laser_Beam_0 = true, Crown_Crystals_0 = true, Crown_Rays_0 = true, Crown_Time_Change = true, hider = false, backwards = false, seeker = false, pinata_prize = false, pinata_claimed = false, pc_player = false, bypass = false, update_player = false, bypass2 = false, WorldTimed = false, Crown_Floating_Effect_1 = true, Crown_Laser_Beam_1 = true, Crown_Crystals_1 = true, Crown_Rays_1 = true, temp_transmute = false, C_QuestActive = false, CQ = false, bb = false, show_location_ = true, show_friend_notifications_ = true;

	int adminLevel = 0, fa2_entered = 0, PosX = 0, PosY = 0, Slot_Credits = 0, Slot_Points = 0, tscanc = 0, item_scan = 0, FlyHackDetectPoints = 0, FlyHackTimer = 0, FlyHackPosX = 0, FlyHackPosY = 0, ignore_cheatss = 0, flagged = 0, Banked_Piggy = 0;
	long long total_locks = 0, total_all_score = 0;

	std::vector<std::string> last_visited_worlds, worlds_owned, worlds_favorite, trade_history, ignored, last_rated, custom_titles;
	vector<int> seed_diary{};

	std::vector<std::map<int, int>> trade_items{};
	long long int race_flag = 0, xp = 0, card_xp = 0;
	std::uint32_t hair_color = 0xFFFFFFFF, eye_drop = 0xFFFFFFFF, eye_lenses = 0xFFFFFFFF, xenonite = 0, cape_t = 2402849791, cape_c = 2402849791, cape_t2 = 723421695, cape_c2 = 1059267327, wings_t = 3356909055, wings_c = 4282965247, wings_t2 = 723421695, wings_c2 = 1059267327;
	int id = 0, netID = 0, last_netID = 0, state = 0, trading_with = -1;
	int x = -1, y = -1;
	int gems = 0, voucher = 0;
	int usedmegaphone = 0;
	std::map<int, int> access_offers{};
	bool hit1 = false;
	int skin = 0x8295C3FF;
	int exitwarn = 0;
	std::uint16_t punched = 0;
	int enter_game = 0;
	int lock = 0;
	int flagmay = 256;
	int lockeitem = 0;
	std::uint8_t n = 0; //newbie passed save
	std::uint8_t supp = 0, hs = 0, m_h = 0, d_s = 0;

	int flags = 19451, TimeDilation = 30, _flags = 104912, _TimeDilation = 30;
	float strong_punch = 200, high_jump = 1000, player_speed = 250, water_speed = 125;
	int build_range = 128, punch_range = 128;
	int state_player = 0;
	int punch_effect = 0;
	int type_player = 0;
	int guild_role = -1;
	int flag_may_form = 0;
	int punch_modifier = 0;
	bool punch_decrease = false;
	bool GhostCommandAccess = false;
	int findPage;

	int rr_ = rand() % 6 + 1;
	int growpass_points = 0;
	std::vector<std::string> growpass_prizes, growpass_quests;
	int rr_used = 0;
	int wls = 0;
	std::uint8_t csn = -1;
	long long int b_t = 0, b_s = 0; // ban seconds
	int last_infected = 0;
	std::uint16_t hair = 0, shirt = 0, pants = 0, feet = 0, face = 0, hand = 0, back = 0, mask = 0, necklace = 0, ances = 0, hiden_clothing = 0; /*clothes*/
	// mirrorMaze struct (used by carnivalEnter Mirror_Maze)
	struct mirrorMaze_t {
		vector<pair<int, int>> Carnival_Mirror_Maze;
		bool Carnival_Mirror_Reset = true;
	} mirrorMaze;

	// cardGame struct (used by carnivalEnter Card_Left/Right)
	struct cardGame_t {
		int Cards_1 = 0, Cards_2 = 0, Cards_3 = 0, Cards_4 = 0;
		int Cards_5 = 0, Cards_6 = 0, Cards_7 = 0, Cards_8 = 0;
		int Cards_9 = 0, Cards_10 = 0, Cards_11 = 0, Cards_12 = 0;
		int Cards_13 = 0, Cards_14 = 0, Cards_15 = 0, Cards_16 = 0;
		int vars[16] = {};
		int XCard_1 = 0, YCard_1 = 0, XCard_2 = 0, YCard_2 = 0;
		int Card_1 = 0, Card_2 = 0;
		int CardOpened = 0, TotalCardSuccess = 0;
		vector<int> Cards = { 742, 744, 746, 748, 1918, 1920, 1922, 1924 };
	} cardGame;

	int CarnivalPoint = 0;
	vector<pair<int, int>>Carnival_Mirror_Maze;
	long long Carnival_GameTime = 0;
	int Shooting_Punch = 0;
	bool unban = false;
	int checkX = 0;
	int checkY = 0;
	int Shooting_Point = 0;
	string carnival_playing = "";
	int carnival_entered = 0;
	int carnivalscore = 0;
	int carniwait = 0;
	int t_x = 0;
	int t_y = 0;
	string carniwaitgame = "";
	int BrutalX = 0;
	int BrutalY = 0;
	int SpikyX = 0;
	int SpikyY = 0;
	int XCard_1 = 0;
	int YCard_1 = 0;
	int XCard_2 = 0;
	int YCard_2 = 0;
	int CardOpened = 0;
	int Card_1 = 0;
	int Card_2 = 0;
	int TotalCardSucces = 0;
	int Carnival_Games_Won = 0;
	int FunTimeRewards = 0;
	std::vector<Friends> friends;
	std::vector<int> last_friends_selection{};
	std::unordered_set<int> find_selected_items{}; // FIX: simpan item yang dicentang di /find & /getitem lintas halaman
	bool welcome_reward_claimed = false; // Welcome Reward: true jika starter pack sudah di-claim
	std::vector<std::string> pending_friends{};
	std::vector<std::string> bans{};
	std::string last_edit = "";
	int summer_surprise = 0, summer_total = 0;
	std::vector<int> summer_milestone;
	int pinata_day = 0;
	int pure_shadow = 0;
	int wrong2fa = 0;
	int new_packets = 0;
	int random_geiger_time = 0;
	std::uint8_t i_11818_1 = 0, i_11818_2 = 0;
	int8_t random_fossil = rand() % 3 + 4;
	int page_number = 0;
	int Aura_Season = 2, Trail_Season = 2;
	int Banner_Item = 0, CBanner_Item = 0, Banner_Flag = 0, CBanner_Flag = 0;
	int Crown_Cycle_Time = 15;
	int Base_R_0 = 255, Base_G_0 = 200, Base_B_0 = 37;
	int Gem_R_0 = 255, Gem_G_0 = 0, Gem_B_0 = 64;
	int Crystal_R_0 = 26, Crystal_G_0 = 45, Crystal_B_0 = 140;

	long long World_Timed = 0;
	int Base_R_1 = 255, Base_G_1 = 255, Base_B_1 = 255;
	int Gem_R_1 = 255, Gem_G_1 = 0, Gem_B_1 = 255;
	int Crystal_R_1 = 0, Crystal_G_1 = 45, Crystal_B_1 = 140;
	int Crown_Value = 1768716607;
	long long int Crown_Value_0_0 = 4294967295, Crown_Value_0_1 = 4278255615, Crown_Value_0_2 = 4190961919;
	long long int Crown_Value_1_0 = 633929727, Crown_Value_1_1 = 1073807359, Crown_Value_1_2 = 2351766271;
	int transmute_item1 = 0, transmute_item2 = 0, remove_transmute = 0;
	string transmuted = "";
	int C_QuestStep = 1, C_QuestProgress = 0, C_ProgressNeeded = 0, C_QuestKind = 0, C_DeliverID = 0;
	uint16_t opc = 0;
	int cc = 0;
	uint8_t rb = 0;
	uint8_t gp = 0, glo = 0, g4p = 0, pg_bank = 0;
	uint8_t w_w = 0;
	int offergems = 0;
	int confirm_reset = 0;
	int carnivalgameswon = 0;
	int level = 1, card_level = 1;
	long long playtime = 0;
	vector<int> playtime_items;
	long long int account_created = 0, seconds = 0;
	double gver = 0.0;
	int protocol = 0, user = 0, token = 0;
	string rid = "", mac = "", vid = "", platformid = "", wk = "", f = "", fz = "", lmode = "", cbits = "", player_age = "", GDPR = "", meta = "", fhash = "", deviceVersion = "", hash = "", gid = "", zf = "";
	string lastmsg = "", lastmsgworld = "";
	string b_bill = "0,0";
	string last_input_text = "";
	string personal_event = "", guild_event = "";
	string growmoji = "(wl)|ā|1&(oops)|ą|1&(sleep)|Ċ|1&(punch)|ċ|1&(bheart)|Ĕ|1&(grow)|Ė|1&(gems)|ė|1&(gtoken)|ę|1&(cry)|ĝ|1&(vend)|Ğ|1&(bunny)|ě|1&(cactus)|ğ|1&(pine)|Ĥ|1&(peace)|ģ|1&(terror)|ġ|1&(troll)|Ġ|1&(evil)|Ģ|1&(fireworks)|Ħ|1&(football)|ĥ|1&(alien)|ħ|1&(party)|Ĩ|1&(pizza)|ĩ|1&(clap)|Ī|1&(song)|ī|1&(ghost)|Ĭ|1&(nuke)|ĭ|1&(halo)|Į|1&(turkey)|į|1&(gift)|İ|1&(cake)|ı|1&(heartarrow)|Ĳ|1&(lucky)|ĳ|1&(shamrock)|Ĵ|1&(grin)|ĵ|1&(ill)|Ķ|1&(eyes)|ķ|1&(weary)|ĸ|1&(moyai)|ļ|1&(plead)|Ľ|1&";
	map<string, int> achievements{};
	long long int lastRESP = 0, lastPDOOR = 0, lastJOINWORLD = 0;
	long long int lastTRAVEL = 0, lastLEAVE = 0;
	int WarnPDoor = 0, WarnRespawn = 0;
	int gtwl = 0;
	uint16_t c_x = 0, c_y = 0;
	uint8_t lavaeffect = 0;
	long long int i240 = 0, i756 = 0, i758 = 0;
	bool tmod = 0;
	int b_i = 0, b_w = 0, b_p = 0;
	int same_input = 0, not_same = 0, pickup_limits = 0, hack = 0, walk = 0;
	bool world_enter = false;
	long long int pickup_time = 0, walk_time = 0, teleport_time = 0, last_inf = 0, last_spam_detection = 0, last_world_enter = 0, last_personal_update = 0, last_fish_catch = 0, respawn_time = 0, hand_torch = 0, punch_time = 0, lava_time = 0, world_time = 0, geiger_time = 0, valentine_time = 0, remind_time = 0, warp_time = 0, name_time = 0, anticheat_cooldown = 0, anticheat_cooldown2 = 0, chat_cmd_time = 0, last_sdb_cmd_time = 0;
	int lastwrenchx = 0, lastwrenchy = 0, lastwrenchb = 0, lastchoosenitem = 0, lastchoosennr = 0;
	int spray_x = 0, spray_y = 0;
	int pps = 0, pps24 = 0;
	long long lpps = 0;
	long long lpps23 = 0, lpps24 = 0;
	long long int punch_count = 0;
	int carnival_credit = 0;
	int total_credit_card = 0, credit_card = rand() % 10000000000000;
	int Magnet_Item = 0;
	int AncesID = 0, Upgradeto = 0, HowmuchSoulStone = 0, IDCeles = 0, JumlahCeles = 0, IDCrystalized = 0, JumlahCrystalized = 0;
	int DailyRiddles = 0;
	uint8_t  grow_air_ballon = 0;
	bool subscriber = false;
	int mailbox = 0;
	bool wiping_time = false;
	uint8_t geiger_ = 0;
	uint8_t all_in = 0;
	int fishing_used = 0, f_x = 0, f_y = 0, move_warning = 0, f_xy = 0, punch_warning = 0, fish_seconds = 0;
	float fish_speed_mult = 1.0f;
	int fish_upgrade_cost_bgl = 50;
	vector<int> glo_p;
	vector<int> lvl_p;
	int geiger_x = -1, geiger_y = -1;
	int t_xp = 0, bb_xp = 0, g_xp = 0, p_xp = 0, ff_xp = 0, s_xp = 0;
	uint8_t t_lvl = 0, bb_lvl = 0, g_lvl = 0, p_lvl = 0, ff_lvl = 0, s_lvl = 0;
	vector<uint8_t> t_p, bb_p, p_p, g_p, ff_p, surg_p;
	int booty_broken = 0;
	bool ghc = false;
	uint8_t dd = 0, dq_day = 0;
	vector<PlayMods> playmods{};
	bool new_pass = false;
	uint16_t b_l = 1;
	vector<pair<uint16_t, uint8_t>> bp;
	vector<pair<int, int>> inv, transmute;
	bool auth_ = true;
	int choosenitem = 0;
	uint8_t radio = 0;
	bool radio2 = false;
	bool temp_radio = true;
	int b_ra = 0, b_lvl = 1;
	int magnetron_id = 0;
	int magnetron_x = 0;
	int magnetron_y = 0;
	long long int save_time = 0, text_time = 0, basher_time = 0;
	vector<vector<long long>> completed_blarneys{ {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, };
	bool block_trade = false;
	int p_x = 0;
	int p_y = 0;
	int guild_id = 0;
	int uid = 0;
	int pending_guild = 0;
	bool is_legend = false;
	bool ios_player = false;
	uint16_t pps23 = 0, all_packets = 0;
	int temp_x = 0, temp_y = 0;
	long long lpps2 = 0, all_packets_time = 0;
	bool valentine = false;
	long long int ban_seconds = 0, last_exit = 0;
	uint8_t fires = 0;
	uint32_t total_pineapple = 0;
	uint16_t stage_pineapple = 0;
	uint8_t garuda = 0;
	uint16_t eq_aura = 0;
	int balloon_faction = rand() % 3 + 1, balloon_hit = 0, hit_by = 0, balloon_donated = 0, balloon_score = 0;
	int fruit_1 = 0, fruit_2 = 0, fruit_3 = 0;
	int fruit_1_c = 0, fruit_2_c = 0, fruit_3_c = 0;
	bool disable_cheater = 0;
	int egg_carton = 0;
	int magic_egg = 0;
	int egg_chance = 0;
	int p_status = 0;
	int participated = 0;
	int lwiz_quest = 0, lwiz_step = 1, lwiz_notification = 0;
	vector<vector<int>> legendary_quest, set = { {} };
	int guild_fg = 0;
	int guild_bg = 0;
	int set_guild_bg = 0, set_guild_fg = 0;
	int cheater_ = 0, cheater_settings = 0;
	int support101_ = 0;
	bool received_balloon_warz = false, black_color = false;
	bool logger = 0;
	int last_used_block = 0;
	int search_page = 20;
	vector<uint16_t> available_surg_items;
	bool sounded = false;
	bool labworked = false;
	bool fixed = false;
	bool fixable = false;
	bool flu = false;
	int pulse = 40, site = 0, sleep = 0, dirt = 0, broken = 0, shattered = 0, incisions = 0, bleeding = 0, incneeded = 0, heart = 0;
	double temp = 98.6;
	double fever = 0.0;
	bool update = true;
	bool s = true;
	// SURG
	uint8_t surgery_skill = 0, spongUsed = 0, scalpUsed = 0, stitcUsed = 0, antibUsed = 0, antisUsed = 0, ultraUsed = 0, labkiUsed = 0, anestUsed = 0, defibUsed = 0, splinUsed = 0, pinsUsed = 0, clampUsed = 0, transUsed = 0;
	uint16_t surgery_done = 0, su_8552_1 = 0, su_8552_2 = 0, su_8552_3 = 0;
	bool antibs = false, surgery_started = false;
	bool hospital_bed = false;
	int fa2 = 0;
	uint8_t surgery_type = -1, started_type = 0, mercy = 0, drtitle = 0, drlegend = 0, drt = 0, lvl125 = 0, donor = 0, master = 0, tgiv = 0, anni_old = 0, santa = 0, party_animal = 0, gp_bronze = 0, gp_silver = 0, gp_gold = 0, aw_winning = 0;
	bool spotlight = false;
	bool mentor = false;
	uint8_t minokawa_wings = 0, panda_spirit = 0, mask_dragon = 0, growformer = 0;
	int won_seeker = 0, won_hider = 0;
	bool emoji_gg = false;
	int musical_volume = 100;
	std::vector<std::string> growtoken_worlds;

	bool new_version = false;
	bool wls_participated = false;
	bool punch_aura = false;
	int punch_aura_id = 0;
	bool halloween_prize_receive = false;
	bool updated_clothes = false;
	bool display_age = false, display_home = false;
	vector<int>Fav_Items;
	uint8_t sprite = 0, wild = 6, golem = 6;
	bool Respawning = false;
	bool ONGame = false, Carried_A_Flag = false;
	int Game_Goal_Egg = 0, Game_Player_Hits = 0, Game_Live = 0, Game_Flag = 0;
	int Flag_X = 0, Flag_Y = 0;
	int pearl = 0, rubble = 0;
	bool received_recycle_prize = false;
	int last_state = 0;
	int automatic_bot = 0;
	bool collect_allowed = false;
	int collect_total = 0;
	bool onetimecollect = false;
	bool load_item2 = false;
	bool admin_event_opt_in = false;

	// Builder's Quest (Jobs)
	struct PlayerJobs {
		int level = 0;
		long long int xp = 0;
		int questType = 0;
		int questItems = 0;
		int questProgress = 0;
		bool activeNotifier = false;
		bool activeQuest = false;
		std::int64_t point = 0;

		bool claimRewards1 = false;
		bool claimRewards2 = false;
		bool claimRewards3 = false;
		bool claimRewards4 = false;
		bool claimRewards5 = false;
	} jobs;

	// Quest For Gauntlet Stone
	int QuestForStone = 0, onQuestStone = 0, StoneQuestStep = 0;
	int QuestXP = 0;

	// Gauntlet
	vector<int> swapping_index = { -1, -1 };
	vector<int> available_swap = {};
};
#define pInfo(peer) ((Player*)(peer->data))
struct PlayerMoving {
	int32_t netID = 0, effect_flags_check = 0;
	int packetType = 0, characterState = 0, plantingTree = 0, punchX = 0, punchY = 0, secondnetID = 0;
	float x = 0.0f, y = 0.0f, XSpeed = 0.0f, YSpeed = 0.0f;
	int packet_3 = 0, packet_int_40 = 0;
};
BYTE* packBlockType(int packetType, int plantingTree, int x, int y, int state = 0) {
	BYTE* data = new BYTE[56];
	Memory_Set(data, 0, 56);
	Memory_Copy(data + 0, &packetType, 4);
	Memory_Copy(data + 12, &state, 4);
	Memory_Copy(data + 20, &plantingTree, 4);
	Memory_Copy(data + 44, &x, 4);
	Memory_Copy(data + 48, &y, 4);
	return data;
}
class kranken_pattern {
public:
	enum : uint8_t { NONE, PT1, PT2, PT3, PT4, PT5, PT6, PT7, PT8, PT9, PT10, PT11, PT12, };
	static std::pair<uint32_t, uint32_t> get_pattern(const uint8_t& pattern) {
		switch (pattern) {
		case NONE:
			return { (uint32_t)0, (uint32_t)10962 };
		case PT1:
			return { (uint32_t)1, (uint32_t)10964 };
		case PT2:
			return { (uint32_t)2, (uint32_t)10966 };
		case PT3:
			return { (uint32_t)3, (uint32_t)10968 };
		case PT4:
			return { (uint32_t)4, (uint32_t)10970 };
		case PT5:
			return { (uint32_t)5, (uint32_t)10972 };
		case PT6:
			return { (uint32_t)6, (uint32_t)10974 };
		case PT7:
			return { (uint32_t)7, (uint32_t)10976 };
		case PT8:
			return { (uint32_t)8, (uint32_t)10978 };
		case PT9:
			return { (uint32_t)9, (uint32_t)10980 };
		case PT10:
			return { (uint32_t)10, (uint32_t)10982 };
		case PT11:
			return { (uint32_t)11, (uint32_t)10984 };
		case PT12:
			return { (uint32_t)12, (uint32_t)10986 };
		default:
			return { (uint32_t)0, (uint32_t)10962 };
		}
	}
};
BYTE* PackBlockUpdate(int packetType, int characterState, float x, float y, float XSpeed, float YSpeed, float rotation, int netID, int plantingTree, int punchX, int punchY, int packet_1, int packet_2, int packet_3) {
	BYTE* data = new BYTE[56];
	Memory_Set(data, 0, 56);
	Memory_Copy(data + 0, &packetType, 4);
	if (packet_1 != 0) Memory_Copy(data + 1, &packet_1, 4);
	if (packet_2 != 0) Memory_Copy(data + 2, &packet_2, 4);
	if (packet_3 != 0) Memory_Copy(data + 3, &packet_3, 4);
	if (netID != 0)Memory_Copy(data + 4, &netID, 4);
	if (characterState != 0)Memory_Copy(data + 12, &characterState, 4);
	if (plantingTree != 0)Memory_Copy(data + 20, &plantingTree, 4);
	Memory_Copy(data + 24, &x, 4);
	Memory_Copy(data + 28, &y, 4);
	if (XSpeed != 0) Memory_Copy(data + 32, &XSpeed, 4);
	if (YSpeed != 0) Memory_Copy(data + 36, &YSpeed, 4);
	Memory_Copy(data + 40, &rotation, 4);
	if (punchX != 0) Memory_Copy(data + 44, &punchX, 4);
	if (punchY != 0)Memory_Copy(data + 48, &punchY, 4);
	return data;
}
BYTE* packPlayerMoving(PlayerMoving* dataStruct, int size_ = 56, int some_extra = 0) {
	BYTE* data = new BYTE[size_];
	Memory_Set(data, 0, size_);
	Memory_Copy(data + 0, &dataStruct->packetType, sizeof(dataStruct->packetType));
	Memory_Copy(data + 3, &some_extra, sizeof(some_extra));
	Memory_Copy(data + 4, &dataStruct->netID, sizeof(dataStruct->netID));
	Memory_Copy(data + 12, &dataStruct->characterState, sizeof(dataStruct->characterState));
	Memory_Copy(data + 20, &dataStruct->plantingTree, sizeof(dataStruct->plantingTree));
	Memory_Copy(data + 24, &dataStruct->x, sizeof(dataStruct->x));
	Memory_Copy(data + 28, &dataStruct->y, sizeof(dataStruct->y));
	Memory_Copy(data + 32, &dataStruct->XSpeed, sizeof(dataStruct->XSpeed));
	Memory_Copy(data + 36, &dataStruct->YSpeed, sizeof(dataStruct->YSpeed));
	Memory_Copy(data + 40, &dataStruct->packet_int_40, sizeof(dataStruct->packet_int_40));
	Memory_Copy(data + 44, &dataStruct->punchX, sizeof(dataStruct->punchX));
	Memory_Copy(data + 48, &dataStruct->punchY, sizeof(dataStruct->punchY));
	return data;
}

// Performance optimization: stack-based packing for standard 56-byte packets
inline void packPlayerMovingStack(BYTE* data, PlayerMoving* dataStruct) {
	Memory_Set(data, 0, 56);
	Memory_Copy(data + 0, &dataStruct->packetType, sizeof(dataStruct->packetType));
	Memory_Copy(data + 4, &dataStruct->netID, sizeof(dataStruct->netID));
	Memory_Copy(data + 12, &dataStruct->characterState, sizeof(dataStruct->characterState));
	Memory_Copy(data + 20, &dataStruct->plantingTree, sizeof(dataStruct->plantingTree));
	Memory_Copy(data + 24, &dataStruct->x, sizeof(dataStruct->x));
	Memory_Copy(data + 28, &dataStruct->y, sizeof(dataStruct->y));
	Memory_Copy(data + 32, &dataStruct->XSpeed, sizeof(dataStruct->XSpeed));
	Memory_Copy(data + 36, &dataStruct->YSpeed, sizeof(dataStruct->YSpeed));
	Memory_Copy(data + 40, &dataStruct->packet_int_40, sizeof(dataStruct->packet_int_40));
	Memory_Copy(data + 44, &dataStruct->punchX, sizeof(dataStruct->punchX));
	Memory_Copy(data + 48, &dataStruct->punchY, sizeof(dataStruct->punchY));
}
BYTE* packFishMoving(PlayerMoving* dataStruct, int size_ = 56) {
	BYTE* data = new BYTE[size_];
	Memory_Set(data, 0, size_);
	*(__int8*)(data + 0) = 31;
	*(__int8*)(data + 3) = dataStruct->packet_3;
	*(__int16*)(data + 4) = dataStruct->netID;
	*reinterpret_cast<int32_t*>(data + 44) = static_cast<int32_t>(dataStruct->x);
	*reinterpret_cast<int32_t*>(data + 48) = static_cast<int32_t>(dataStruct->y);
	return data;
}
void SendPacketRaw(int a1, void* packetData, size_t packetDataSize, void* a4, ENetPeer* peer, int packetFlag) {
	ENetPacket* p;
	if (peer) {
		if (a1 == 4 && *((BYTE*)packetData + 12) & 8) {
			p = enet_packet_create(0, packetDataSize + *((DWORD*)packetData + 13) + 5, packetFlag);
			int four = 4;
			Memory_Copy(p->data, &four, 4);
			Memory_Copy((char*)p->data + 4, packetData, packetDataSize);
			Memory_Copy((char*)p->data + packetDataSize + 4, a4, *((DWORD*)packetData + 13));
			if (enet_peer_send(peer, 0, p) != 0) {
				enet_packet_destroy(p);
			}
		}
		else {
			if (a1 == 192) {
				a1 = 4;
				p = enet_packet_create(0, packetDataSize + 5, packetFlag);
				Memory_Copy(p->data, &a1, 4);
				Memory_Copy((char*)p->data + 4, packetData, packetDataSize);
				if (enet_peer_send(peer, 0, p) != 0) {
					enet_packet_destroy(p);
				}
			}
			else {
				p = enet_packet_create(0, packetDataSize + 5, packetFlag);
				Memory_Copy(p->data, &a1, 4);
				Memory_Copy((char*)p->data + 4, packetData, packetDataSize);
				if (enet_peer_send(peer, 0, p) != 0) {
					enet_packet_destroy(p);
				}
			}
		}
	}
	delete[] (char*)packetData;
}
void send_raw(ENetPeer* peer, int a1, const void* packetData, int packetDataSize, int packetFlag, int delay = 0) {
	if (!peer || packetData == nullptr || packetDataSize <= 0) return;

	const uint8_t* src = reinterpret_cast<const uint8_t*>(packetData);

	// extraDataLength hanya di-read jika a1==4 dan buffer cukup panjang untuk offset 13+4
	uint32_t extraDataLength = 0;
	constexpr int EXTRA_LEN_OFFSET = 13; // offset byte dari mana kita baca len (sesuai asumsi lama)
	if (a1 == 4) {
		// pas	tikan kita tidak membaca di luar packetData
		if (packetDataSize >= EXTRA_LEN_OFFSET + static_cast<int>(sizeof(uint32_t))) {
			// baca little-endian uint32 dari src + EXTRA_LEN_OFFSET
			uint32_t val;
			std::memcpy(&val, src + EXTRA_LEN_OFFSET, sizeof(val));
			extraDataLength = val;
		}
		else {
			// jika tidak ada field ini, anggap 0 agar aman
			extraDataLength = 0;
		}
	}

	// total size yang akan dialokasikan: 4 bytes header + packetDataSize + extraDataLength + (optional padding)
	int headerSize = 4; // kita tulis 'a1' atau '4' di awal
	size_t finalSize = static_cast<size_t>(headerSize) + static_cast<size_t>(packetDataSize) + static_cast<size_t>(extraDataLength);

	// safety cap: hindari alokasi gila
	const size_t MAX_PACKET_SIZE = 1u << 24; // adjust sesuai kebutuhan (16MB)
	if (finalSize > MAX_PACKET_SIZE) return;

	ENetPacket* p = enet_packet_create(nullptr, finalSize, packetFlag);
	if (!p) return;

	uint8_t* dst = reinterpret_cast<uint8_t*>(p->data);

	// tulis header (4 bytes)
	int headerValue = a1;
	std::memcpy(dst, &headerValue, sizeof(headerValue));

	// copy packet payload setelah header
	std::memcpy(dst + headerSize, src, static_cast<size_t>(packetDataSize));

	// zero-fill extra area (jika ada)
	if (extraDataLength > 0) {
		std::memset(dst + headerSize + packetDataSize, 0, extraDataLength);
	}

	// jika ada delay => tulis ke offset yang aman saja (cek bounds)
	if (delay != 0) {
		// OFFSETS YANG KAMU PAKAI SEBELUMNYA: 24 untuk delay, 56 untuk flag
		// pastikan finalSize cukup untuk menulis ke posisi-posisi ini
		if (finalSize >= 24 + sizeof(int)) {
			std::memcpy(dst + 24, &delay, sizeof(int));
		}
		int deathFlag = 0x19;
		if (finalSize >= 56 + sizeof(int)) {
			std::memcpy(dst + 56, &deathFlag, sizeof(int));
		}
	}

	if (enet_peer_send(peer, 0, p) != 0) {
		enet_packet_destroy(p);
	}
}
void send_raw2(ENetPeer* peer, int a1, void* packetData, int packetDataSize, int packetFlag, int delay = 0) {
	ENetPacket* p;
	if (peer) {
		if (a1 == 4 && *((BYTE*)packetData + 12) & 8) {
			p = enet_packet_create(0, packetDataSize + *((DWORD*)packetData + 13) + 5, packetFlag);
			int four = 4;
			Memory_Copy(p->data, &four, 4);
			Memory_Copy(p->data + 24, &delay, 4);
			Memory_Copy((char*)p->data + 4, packetData, packetDataSize);
			Memory_Set((char*)p->data + packetDataSize + 4, 0, *((DWORD*)packetData + 13));
		}
		else {
			p = enet_packet_create(0, packetDataSize + 5, packetFlag);
			Memory_Copy(p->data, &a1, 4);
			Memory_Copy((char*)p->data + 4, packetData, packetDataSize);
		}
		if (enet_peer_send(peer, 0, p) != 0) {
			enet_packet_destroy(p);
		}
	}
}
void SendInventory(ENetPeer* peer, Player* player) {
	// serialize player->inv and send packet to client
}
void send_packet(ENetPeer* peer, const void* data, uintmax_t data_size, uint32_t flags) {
	if (!peer) return;
	ENetPacket* packet = enet_packet_create(data, data_size, flags);
	if (!packet) return;
	if (enet_peer_send(peer, 0, packet) != 0) enet_packet_destroy(packet);
}
void send_packet(ENetPeer* peer, uint32_t type, const void* data, uintmax_t data_size, uint32_t flags) {
	if (!peer) return;
	ENetPacket* packet = enet_packet_create(nullptr, 5 + data_size, flags);
	if (!packet) return;
	Memory_Copy(packet->data, &type, 4);
	packet->data[data_size + 4] = 0;
	if (data) Memory_Copy(packet->data + 4, data, data_size);
	if (enet_peer_send(peer, 0, packet) != 0) enet_packet_destroy(packet);
}
string CooldownText(int seconds) {
	string x;
	x.reserve(64);
	int day = seconds / (24 * 3600);
	if (day != 0) x.append(to_string(day) + " days, ");
	seconds = seconds % (24 * 3600);
	int hour = seconds / 3600;
	if (hour != 0) x.append(to_string(hour) + " hours, ");
	seconds %= 3600;
	int minutes = seconds / 60;
	if (minutes != 0) x.append(to_string(minutes) + " mins, ");
	seconds %= 60;
	int secondsz = seconds;
	if (secondsz != 0) x.append(to_string(secondsz) + " secs ");
	return x;
}
std::string get_player_custom_wrench(ENetPeer* peer) {
	if (!peer || !pInfo(peer)) return "";
	return "{\"PlayerWorldID\":" + to_string(pInfo(peer)->netID) + ",\"TitleTexture\":\"" + pInfo(peer)->TitleTexture + "\",\"TitleTextureCoordinates\":\"" + pInfo(peer)->TitleCoordinate + "\",\"WrenchCustomization\":{\"WrenchForegroundCanRotate\":true,\"WrenchForegroundID\":" + to_string(pInfo(peer)->wrench_foreground_custom) + ",\"WrenchIconID\":" + to_string(pInfo(peer)->wrench_custom) + "}}";
}
std::vector<std::pair<int, int>> getOfflineInventory(const std::string& username, const std::string& usingFile = "") {
	const std::string filePath = usingFile != "" ? usingFile : "database/players/" + username + "_.json";
	if (std::filesystem::exists(filePath)) {
		std::ifstream i(filePath);
		json jsonData;
		i >> jsonData;
		i.close();
		return jsonData["inventory"].get<std::vector<std::pair<int, int>>>();
	}
	return {};
}
bool saveDatabaseOffline(const string& username, const string& path, const string& type) {
	const string filePath = "database/players/" + username + "_.json";
	if (filesystem::exists(filePath)) {
		ifstream inputFile(filePath);
		json jsonData;
		inputFile >> jsonData;
		if (type == "int") jsonData[path] = 0;
		if (type == "string") jsonData[path] = "";
		if (type == "bool") jsonData[path] = false;
		inputFile.close();
		ofstream outputFile(filePath);
		outputFile << jsonData << endl;
		outputFile.close();
		return true;
	}
	return false;
}
bool saveOfflineInventory(const string& username, const vector<pair<int, int>>& inventory, bool reset = false, const string& usingFile = "") {
	const string filePath = usingFile != "" ? usingFile : "database/players/" + username + "_.json";
	if (filesystem::exists(filePath)) {
		ifstream inputFile(filePath);
		json jsonData;
		inputFile >> jsonData;
		jsonData["inventory"] = inventory;
		if (reset) {
			jsonData["wl_bank_amount"] = 0;
			jsonData["dl_bank_amount"] = 0;
			jsonData["bgl_bank_amount"] = 0;
		jsonData["mgl_bank_amount"] = 0;

		jsonData["mgl_bank_amount"] = 0;
		jsonData["welcome_reward_claimed"] = false;
		}
		inputFile.close();
		ofstream outputFile(filePath);
		outputFile << jsonData << endl;
		outputFile.close();
		return true;
	}
	return false;
}
std::string GetDeviceName(string ID_) {
	string name = "";
	if (ID_ == "-1") name = "UNKNOWN";
	else if (ID_ == "0" or ID_ == "0,0,1") name = "WINDOWS";
	else if (ID_ == "1") name = "IOS";
	else if (ID_ == "2") name = "OSX";
	else if (ID_ == "3") name = "LINUX";
	else if (ID_ == "4") name = "ANDROID";
	else if (ID_ == "5") name = "WINDOWS_MOBILE";
	else if (ID_ == "6") name = "WEBOS";
	else if (ID_ == "7") name = "BBX";
	else if (ID_ == "8") name = "FLASH";
	else if (ID_ == "9") name = "HTML5";
	else if (ID_ == "10") name = "MAXVAL";
	else name = "UNKNOWN";
	return name;
}
namespace player::algorithm {
	bool adventureTimers(ENetPeer* peer) {
		if (!pInfo(peer)->adventure_begins) return false;
		if (pInfo(peer)->timers < +date_time::get_epoch_time()) return false;
		return true;
	}
	std::string second_adventure(int n) {
		int second = n % 60;
		n /= 60;
		int minute = n % 60;
		n /= 60;
		std::string str;
		if (minute != 9) {
			str.append(std::to_string(minute) + ":");
		}
		else if (minute == 0) {
			str.append("0:");
		}
		if (second < 10) {
			str.append("0" + std::to_string(second));
		}
		else {
			str.append(std::to_string(second));
		}
		return str;
	}
}
// Tambahkan ini di bagian bawah PlayerInfo.h agar dikenali semua file

// ─── /editrole infrastructure ─────────────────────────────────────────────────
#include <mutex>
#include <vector>
struct CustomRoleEntry {
	std::string name;
	int         level = 0;
	std::string prefix;
	std::vector<std::string> commands;
	bool        can_access_world = false;
	bool        can_punish = false;
};
inline std::vector<CustomRoleEntry>        g_customRoles;
inline std::mutex                          g_customRoles_mtx;
inline std::map<std::string, int>          commandLevelOverrides;
inline int GetCmdEffectiveLevel(const std::string& cmd, int defaultLevel) noexcept {
    auto it = commandLevelOverrides.find(cmd);
    return (it != commandLevelOverrides.end()) ? it->second : defaultLevel;
}
#define LVL_ROLE_EDIT(cmd, def) GetCmdEffectiveLevel((cmd), (def))
inline std::map<std::string, std::string>  g_builtinStringOverrides;
inline const std::string CUSTOM_ROLES_FILE = "database/json/editrole.json";
inline const std::string CMD_LEVEL_OVERRIDE_FILE = "database/json/editrole.json";
inline const std::string BUILTIN_STRING_OVERRIDE_FILE = "database/json/builtin_role_overrides.json";
// ─────────────────────────────────────────────────────────────────────────────
void UpdatePairProgress(ENetPeer* peer, int type, int amount);
void SavePairLeaderboard(std::string name1, std::string name2, int charisma, int count);
void GeneratePairQuest(ENetPeer* peer);
void GivePairRewards(ENetPeer* peer);
void SaveDropItemMoreTimes(ENetPeer* peer, int itemId, int coint);