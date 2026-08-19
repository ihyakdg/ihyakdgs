#pragma once
#include <string>
#include "NopySource/Handle/WorldInfo.h"
#include "NopySource/Handle/DailyReward.h"
#include <ctime>
#include <cstdlib>
#include <curl/curl.h>
#include <string>
#include <thread>
#include "NopySource/Handle/PlayerInfo.h"
#include <filesystem>
#define NOMINMAX
#include <Windows.h>
#include "NopySource/Handle/LoginHandler.h"
#include <lm.h>
#include <lmcons.h>
#include <fstream>
#include <nlohmann/json.hpp>
#pragma comment(lib, "Netapi32.lib")
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>
#include <algorithm>
#undef min
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <fstream>
#include <sstream>
#include <filesystem>
#define NOMINMAX
#include <algorithm> 
#include <string>
#include <cstdlib>  
#include <cctype>
#include <mutex>

inline std::mutex worlds_mutex;

using json = nlohmann::json;
std::string copiedWorldName = "";
json copiedWorldData;
std::unordered_map<int, int> ghostSellPrices;
std::unordered_map<int, int> CarnivalPointExchange;
inline std::unordered_map<int, std::vector<std::tuple<int, int, int>>> customExchangeList;
std::unordered_map<std::string, std::function<void(ENetPeer*, const std::string&, int)>> defaultCommandHandlers;
static std::map<std::string, std::map<std::string, int>> roleCommandAccess;
inline std::unordered_map<ENetPeer*, std::string> pending_world_paste;
static std::mutex g_claimrole_mtx;
static std::vector<std::pair<std::string, std::string>> top_donation_str;

namespace PartyEvent {
	static std::mutex g_mtx;
	static bool g_active = false;
	static long long g_next_spawn = 0;
	static long long g_next_weather = 0;
	static int g_weather_idx = 0;

	static inline long long now() { return (long long)time(nullptr); }

	static inline void broadcast_banner(const std::string& banner_rttex) {
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
			gamepacket_t p(5000);
			p.Insert("OnAddNotification");
			p.Insert(banner_rttex);
			p.Insert("");
			p.Insert("audio/cumbia_horns.wav");
			p.Insert(1);
			p.CreatePacket(currentPeer);
		}
	}

	static inline void broadcast_music() {
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
			CAction::Log(currentPeer, "action|play_music\nfile|audio/tacos.ogg\ndelayMS|0");
		}
	}

	static inline std::vector<ENetPeer*> list_peers_in_world(const std::string& wname) {
		std::vector<ENetPeer*> list;
		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
			if (pInfo(cp)->world != wname) continue;
			list.push_back(cp);
		}
		return list;
	}

	static inline int roll_gacha_id() {
		static const int gacha_ids[] = { 10716, 3402, 14084, 5136, 14596, 7960, 9350 };
		int id = gacha_ids[rand() % (int)(sizeof(gacha_ids) / sizeof(gacha_ids[0]))];
		int roll = rand() % 1000;
		if (roll < 5) id = 15874;
		return id;
	}

	static inline void spawn_in_world_spread(const std::string& wname, int drops_per_world) {
		auto wit = std::find_if(worlds.begin(), worlds.end(), [&](const World& w) { return w.name == wname; });
		if (wit == worlds.end()) return;

		std::vector<ENetPeer*> ppl = list_peers_in_world(wname);
		if (ppl.empty()) return;

		if (drops_per_world < 1) drops_per_world = 1;
		if (drops_per_world > 25) drops_per_world = 25;

		for (int k = 0; k < drops_per_world; k++) {
			ENetPeer* base = ppl[rand() % (int)ppl.size()];
			int px = pInfo(base)->x + (rand() % 241) - 120;
			int py = pInfo(base)->y + (rand() % 241) - 120;
			if (px < 0) px = 0;
			if (py < 0) py = 0;

			WorldDrop d{};
			d.id = roll_gacha_id();
			d.count = 1;
			d.x = px;
			d.y = py;

			VisualHandle::Drop(&(*wit), d);

			for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
				if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
				if (pInfo(cp)->world != wname) continue;
				VarList::OnConsoleMessage(cp, "`6PARTY``: `oSomething dropped from the sky!``");
				VarList::OnParticleEffect(cp, px + 16, py + 16, 420, 90, 0);
				VarList::OnPlaySound(cp, "audio/phoenix.wav", 0);
			}
		}
	}

	static inline void set_world_weather(const std::string& wname, int weather_id) {
		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
			if (pInfo(cp)->world != wname) continue;
			gamepacket_t p(5000);
			p.Insert("OnSetCurrentWeather");
			p.Insert(weather_id);
			p.CreatePacket(cp);
		}
	}

	static inline std::vector<std::string> list_online_worlds() {
		std::unordered_map<std::string, bool> seen;
		std::vector<std::string> out;
		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
			if (pInfo(cp)->world.empty()) continue;
			if (!seen[pInfo(cp)->world]) {
				seen[pInfo(cp)->world] = true;
				out.push_back(pInfo(cp)->world);
			}
		}
		return out;
	}

	static inline void spawn_all_worlds_instant(int drops_per_world) {
		std::vector<std::string> ws = list_online_worlds();
		for (auto& w : ws) spawn_in_world_spread(w, drops_per_world);
	}

	static inline long long now_ms() { return (long long)enet_time_get(); }

	static inline void Tick() {
		std::lock_guard<std::mutex> lk(g_mtx);
		if (!g_active) return;

		long long t = now_ms();

		if (t >= g_next_weather) {
			static const int weather_list[] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
			g_weather_idx++;
			if (g_weather_idx >= (int)(sizeof(weather_list) / sizeof(weather_list[0]))) g_weather_idx = 0;

			int wid = weather_list[g_weather_idx];
			std::vector<std::string> ws = list_online_worlds();
			for (auto& w : ws) set_world_weather(w, wid);

			g_next_weather = t + 5000;
		}

		if (t >= g_next_spawn) {
			std::vector<std::string> ws = list_online_worlds();
			for (auto& w : ws) spawn_in_world_spread(w, 6);
			g_next_spawn = t + 5000;
		}
	}

	static inline void Start(ENetPeer* peer) {
		std::lock_guard<std::mutex> lk(g_mtx);
		g_active = true;
		g_weather_idx = 0;

		broadcast_music();
		broadcast_banner("interface/large/tianvan/partystart_banner.rttex");

		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
			VarList::OnConsoleMessage(cp, "`6PARTY``: `2Party has started!``");
		}

		spawn_all_worlds_instant(12);

		long long t = now_ms();
		g_next_weather = t + 100;
		g_next_spawn = t + 100;
	}


	static inline void Stop(ENetPeer* peer) {
		std::lock_guard<std::mutex> lk(g_mtx);
		g_active = false;
		broadcast_banner("interface/large/tianvan/partyend_banner.rttex");
		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
			VarList::OnConsoleMessage(cp, "`6PARTY``: `4Party has ended.``");
		}
	}
}
static std::string ToUpperStr(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

namespace GachaDarkstone {
	static const std::vector<int> pool = {
		7960,
		7960,
		9350,
		3402,
		14084,
		5136,
		10716,
		14596,
	};
	static inline int roll() {
		return pool[rand() % (int)pool.size()];
	}
	static constexpr long long COST = 100000LL;  // 100k gems
}

static ENetPeer* FindPeerByGrowID(const std::string& growid) {
	std::string target = ToUpperStr(growid);
	for (ENetPeer* currentPeer = server->peers;
		currentPeer < &server->peers[server->peerCount];
		++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED || !currentPeer->data) continue;
		Player* pl = pInfo(currentPeer);
		if (!pl) continue;
		if (pl->tankIDName.empty()) continue;

		std::string nameUpper = ToUpperStr(pl->tankIDName);
		if (nameUpper == target) {
			return currentPeer;
		}
	}
	return nullptr;
}

static bool PayWorldLock(ENetPeer* peer, int price_wl = 20) {
	const int WL_ID = 1796;
	if (!Inventory::Has(peer, WL_ID, price_wl)) {
		VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Need `w20 Diamonds Lock`` to use this command!", 0, 0);
		return false;
	}
	int minus = -price_wl;
	if (Inventory::Modify(peer, WL_ID, minus, true) != 0) {
		VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Failed to deduct Diamonds Lock.``", 0, 0);
		return false;
	}
	return true;
}
static inline int imin2(int a, int b) { return (a < b) ? a : b; }
static inline int imin3(int a, int b, int c) { return imin2(imin2(a, b), c); }

static int LevenshteinDistance(const std::string& a, const std::string& b) {
	const int n = (int)a.size();
	const int m = (int)b.size();
	if (n == 0) return m;
	if (m == 0) return n;

	std::vector<int> prev(m + 1);
	std::vector<int> cur(m + 1);

	for (int j = 0; j <= m; ++j) prev[j] = j;

	for (int i = 1; i <= n; ++i) {
		cur[0] = i;
		const char ca = a[i - 1];
		for (int j = 1; j <= m; ++j) {
			const char cb = b[j - 1];
			const int cost = (ca == cb) ? 0 : 1;
			cur[j] = imin3(
				prev[j] + 1,
				cur[j - 1] + 1,
				prev[j - 1] + cost
			);
		}
		prev.swap(cur);
	}
	return prev[m];
}

static std::string NormalizeCmd(std::string s) {
	if (!s.empty() && s[0] == '/') s.erase(0, 1);
	s = to_lower(s);
	return s;
}

static bool FindClosestCommand(const std::string& typed, std::string& outCmd) {
	if (typed.empty()) return false;

	std::string t = NormalizeCmd(typed);
	if (t.empty()) return false;

	int bestDist = 999999;
	int bestLen = 0;
	std::string best;

	for (const auto& kv : commandMap) {
		const std::string& cmd = kv.first;
		if (cmd.empty() || cmd[0] != '/') continue;

		std::string c = NormalizeCmd(cmd);
		if (c.empty()) continue;

		int dist = LevenshteinDistance(t, c);

		int lenDiff = (int)c.size() - (int)t.size();
		if (lenDiff < 0) lenDiff = -lenDiff;

		int score = dist * 3 + lenDiff;

		if (score < bestDist || (score == bestDist && (int)c.size() > bestLen)) {
			bestDist = score;
			bestLen = (int)c.size();
			best = cmd;
		}
	}

	if (best.empty()) return false;

	int typedLen = (int)t.size();
	if (typedLen < 1) return false;

	int threshold = 0;
	if (typedLen <= 4) threshold = 3;
	else if (typedLen <= 7) threshold = 6;
	else threshold = 9;

	if (bestDist > threshold) return false;

	outCmd = best;
	return true;
}
namespace CommandActiveConfig {
	static std::unordered_map<std::string, bool> g_cache;
	static bool g_loaded = false;
	static const char* kPath = "database/json/cmdactive.json";

	static std::string trim(std::string s) {
		while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
		while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
		return s;
	}

	static std::string normalize(std::string cmd) {
		cmd = trim(cmd);
		std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
		if (!cmd.empty() && cmd[0] != '/') cmd = "/" + cmd;
		return cmd;
	}

	static bool is_protected_command(const std::string& raw_cmd) {
		const std::string cmd = normalize(raw_cmd);
		return cmd == "/editcommand";
	}

	static void save() {
		nlohmann::json j = nlohmann::json::object();
		for (const auto& [cmd, enabled] : g_cache) {
			j[cmd] = enabled;
		}

		std::ofstream out(kPath);
		if (out.is_open()) {
			out << j.dump(4);
			out.close();
		}
	}

	static void load() {
		if (g_loaded) return;
		g_loaded = true;
		g_cache.clear();

		std::ifstream in(kPath);
		if (!in.is_open()) {
			save();
			return;
		}

		try {
			nlohmann::json j;
			in >> j;
			in.close();

			if (j.is_object()) {
				for (auto it = j.begin(); it != j.end(); ++it) {
					if (it.value().is_boolean()) {
						g_cache[normalize(it.key())] = it.value().get<bool>();
					}
				}
			}
		}
		catch (...) {
			g_cache.clear();
			save();
		}
	}

	static bool is_enabled(const std::string& raw_cmd) {
		load();
		const std::string cmd = normalize(raw_cmd);

		if (is_protected_command(cmd)) {
			return true;
		}

		auto it = g_cache.find(cmd);
		if (it == g_cache.end()) {
			return true;
		}
		return it->second;
	}

	static void set_enabled(const std::string& raw_cmd, bool enabled) {
		load();
		const std::string cmd = normalize(raw_cmd);

		if (is_protected_command(cmd)) {
			return;
		}

		g_cache[cmd] = enabled;
		save();
	}
}

class Commands {
public:

	static void SETSHOP(ENetPeer* peer, const std::string& param = "") {
		if (!peer || !peer->data) return;
		if (!Role::Clist(pInfo(peer)->tankIDName)) {
			VarList::OnConsoleMessage(peer, "`4You don't have permission to use /setshop.");
			return;
		}

		std::string dialog =
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wShop Pack Configuration|left|6016|\n"
			"add_spacer|small|\n"
			"add_textbox|`oConfigure shop pack JSON filename and item contents:``|left|\n"
			"add_textbox|`9File saved to: `wdatabase/shop/-[Nama_File].json``|left|\n"
			"add_spacer|small|\n"
			"add_text_input|shop_name|File Name .json (e.g. venom_set -> -venom_set.json):||32|\n"
			"add_text_input|pack_title|Display Title (e.g. Venom Set):||32|\n"
			"add_text_input|gems_price|Price Gems (e.g. 500000):|500000|10|\n"
			"add_text_input|item_id|Item ID (e.g. 14624):||10|\n"
			"add_text_input|item_count|Item Count:|1|5|\n"
			"add_spacer|small|\n"
			"add_button|setshop_save|`2Save Configuration|noflags|0|0|\n"
			"end_dialog|setshop_dialog|Cancel||\n"
			"add_quick_exit|";

		VarList::OnDialogRequest(peer, dialog);
	}

	static void EDIT_COMMAND(ENetPeer* peer, const std::string& param, int level) {
		if (!peer || !peer->data) return;

		std::stringstream ss(param);
		std::string target_cmd;
		std::string action;
		ss >> target_cmd >> action;

		if (target_cmd.empty() || action.empty()) {
			VarList::OnConsoleMessage(peer, "`4Usage: /editcommand <nama cmd> <enable/disable>``");
			VarList::OnConsoleMessage(peer, "`oExample: /editcommand exchange disable");
			return;
		}

		target_cmd = CommandActiveConfig::normalize(target_cmd);
		std::transform(action.begin(), action.end(), action.begin(), ::tolower);

		if (CommandActiveConfig::is_protected_command(target_cmd)) {
			VarList::OnConsoleMessage(peer, "`4You cannot edit this command.``");
			return;
		}

		auto it = commandMap.find(target_cmd);
		if (it == commandMap.end()) {
			VarList::OnConsoleMessage(peer, "`4Command not found: `w" + target_cmd + "``");
			return;
		}

		bool enabled = false;
		if (action == "enable") {
			enabled = true;
		}
		else if (action == "disable") {
			enabled = false;
		}
		else {
			VarList::OnConsoleMessage(peer, "`oUsage: /editcommand <command> <enable/disable>``");
			return;
		}

		CommandActiveConfig::set_enabled(target_cmd, enabled);
		VarList::OnConsoleMessage(peer, "`2Command `w" + target_cmd + "`2 is now " + std::string(enabled ? "`cenabled``" : "`4disabled``") + ".");
	}
	static void Execute(ENetPeer* peer, string cmd, bool msg = false) {
		std::string act_cmd = cmd;
		std::string param;
		std::string level;
		bool handled = false;
		transform(act_cmd.begin(), act_cmd.end(), act_cmd.begin(), ::tolower);
		if (act_cmd.find("player_chat=") != string::npos) return;
		if (act_cmd.substr(0, 5) == "/msg " || act_cmd.substr(0, 3) == "/r " || act_cmd == "/msg" || act_cmd == "/r") VarList::OnConsoleMessage(peer, "CT:[MSG]_ `6" + cmd + "``");
		else if (act_cmd.substr(0, 4) == "/sb " || act_cmd == "/sb") {
			if (cmd.empty()) { VarList::OnConsoleMessage(peer, "`oUsage: /sb <text>"); return; }
			if (!PayWorldLock(peer, 1)) return;
			VarList::OnConsoleMessage(peer, "CT:[SB]_ `6" + cmd + "``");
		}
		else if (act_cmd.substr(0, 4) == "/bc " || act_cmd == "/bc") {
			if (cmd.empty()) { VarList::OnConsoleMessage(peer, "`oUsage: /bc <text>"); return; }
			if (!PayWorldLock(peer, 1)) return;
			VarList::OnConsoleMessage(peer, "CT:[BC]_ `6" + cmd + "``");
		}
		else if (act_cmd.substr(0, 4) == "/gc " || act_cmd == "/gc") {
			if (cmd.empty()) { VarList::OnConsoleMessage(peer, "`oUsage: /gc <text>"); return; }
			if (!PayWorldLock(peer, 1)) return;
			VarList::OnConsoleMessage(peer, "CT:[GC]_ `6" + cmd + "``");
		}
		else if (not msg) {
			VarList::OnConsoleMessage(peer, " `6" + cmd + "``");
		}
		bool matched = false;
		std::string command = act_cmd;
		for (const auto& [key, value] : commandMap) {
			const auto& [func, requiredLevel] = value;
			if (act_cmd.find(key) == 0 && (act_cmd.size() == key.size() || act_cmd[key.size()] == ' ')) {
				if (!CommandActiveConfig::is_enabled(key)) {
					VarList::OnConsoleMessage(peer, "`4This command is currently disabled by the server.``");
					return;
				}

				if (pInfo(peer)->Role.Role_Level < requiredLevel) {
					matched = false;
					break;
				}

				std::string param = cmd.substr(key.length());
				if (!param.empty() && param[0] == ' ') {
					param = param.substr(1);
				}

				func(peer, param, pInfo(peer)->Role.Role_Level);
				matched = true;
				return;
			}
		}
		if (!matched) {
			VarList::OnConsoleMessage(peer, "`4Unknown command.``  Enter `$/?`` for a list of valid commands.");
			return;
		}
	}

	static void Command() {
		Commands::InitEditRole();
		commandMap["/editrole"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EDITROLE(peer); }, LVL_ROLE_EDIT("/editrole",  999) };
		commandMap["/setshop"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETSHOP(peer, param); }, LVL_ROLE_EDIT("/setshop", 13) };
		commandMap["/startdonationevent"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::StartDonationEvent(peer); }, LVL_ROLE_EDIT("/startdonationevent",  999) };
		commandMap["/stopdonationevent"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::StopDonationEvent(peer); }, LVL_ROLE_EDIT("/stopdonationevent",  999) };
		commandMap["/donate"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DonateWL(peer, param); }, LVL_ROLE_EDIT("/donate", 0) }; // /donate <type> <amount>
		commandMap["/editcommand"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EDIT_COMMAND(peer, param, level); }, LVL_ROLE_EDIT("/editcommand",  999) };
		commandMap["/buy"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CMD_BUY(peer, param); }, LVL_ROLE_EDIT("/buy", 0) };
		commandMap["/autocv"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::AUTOCV(peer); }, LVL_ROLE_EDIT("/autocv", 0) };
		commandMap["/cv"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CMD_CV(peer); }, LVL_ROLE_EDIT("/cv", 0) };
		commandMap["/playsound"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PLAYSOUND(peer, param); }, LVL_ROLE_EDIT("/playsound", 0) };
		commandMap["/playsong"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PLAYSOUND(peer, param); }, LVL_ROLE_EDIT("/playsong", 0) };
		commandMap["/setsound"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PLAYSOUND(peer, param); }, LVL_ROLE_EDIT("/setsound", 0) };
		commandMap["/autoptht"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::AUTOPTHT(peer); }, LVL_ROLE_EDIT("/autoptht", 0) };
		commandMap["/infinite"] = { [](ENetPeer* peer, const std::string& param, int level) { CMD_INFINITE(peer, param, level); }, LVL_ROLE_EDIT("/infinite",  999) };
		commandMap["/exchangenew"] = { [](ENetPeer* peer, std::string param, int level) { CMD_EXCHANGENEW(peer, param, level); }, LVL_ROLE_EDIT("/exchangenew", 0) };
		commandMap["/setexchangenew"] = { [](ENetPeer* peer, std::string param, int level) { CMD_SETEXCHANGENEW(peer, param, level); }, LVL_ROLE_EDIT("/setexchangenew",  999) };
		commandMap["/setconsum"] = { [](ENetPeer* peer, const std::string& param, int level) { CMD_SETCONSUM(peer, param, level); }, LVL_ROLE_EDIT("/setconsum",  999) };
		commandMap["/sawit"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SAWIT(peer); }, LVL_ROLE_EDIT("/sawit", 0) };
		commandMap["/ustadz"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::USTADZ(peer); }, LVL_ROLE_EDIT("/ustadz", 0) };
		commandMap["/editdrop"] = { [](ENetPeer* peer, const std::string& cmd, int) { CMD_EDITDROP(peer, cmd); }, LVL_ROLE_EDIT("/editdrop",  999) };
		commandMap["/e"] = { [](ENetPeer* peer, const std::string& param, int) { command_e(peer); }, LVL_ROLE_EDIT("/e", 0) };
		commandMap["/pair"] = { [](ENetPeer* peer, const std::string& param, int) { cmd_pair(peer, param); }, LVL_ROLE_EDIT("/pair", 0) };
		commandMap["/leaderboardpair"] = { [](ENetPeer* peer, const std::string& param, int) { cmd_leaderboardpair(peer, param); }, LVL_ROLE_EDIT("/leaderboardpair", 0) };
		commandMap["/setrecipe"] = { [](ENetPeer* peer, const std::string& param, int) { cmd_setrecipe(peer, param); }, LVL_ROLE_EDIT("/setrecipe",  999) };
		commandMap["/autosellfish"] = { [](ENetPeer* peer, const std::string& param, int) { AUTOSELLFISH(peer, param); }, LVL_ROLE_EDIT("/autosellfish", 0) };
		commandMap["/particle"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CMD_PARTICLE(peer, param); }, LVL_ROLE_EDIT("/particle", 0) };
		commandMap["/config"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands:ConfigDialog(peer); }, LVL_ROLE_EDIT("/config",  999) };
		commandMap["/cekalt"] = { [](ENetPeer* peer, const std::string& arg, int level) { Commands::Command_CekAlt(peer, arg); }, LVL_ROLE_EDIT("/cekalt", 13) };
		commandMap["/help"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HELP(peer); }, LVL_ROLE_EDIT("/help", 0) };
		commandMap["/?"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HELP(peer); }, LVL_ROLE_EDIT("/?", 0) };
		std::vector<std::string> commands = { "/wave", "/dance", "/love", "/sleep", "/facepalm", "/fp", "/smh", "/yes", "/no", "/cheer", "/wink", "/troll", "/shower", "/mad", "/omg", "/lol", "/laugh", "/cry", "/sad", "/idk", "/shrug", "/furious", "/rolleyes", "/dab", "/sassy", "/dance2", "/march", "/favrm", "/grumpy", "/shy", "/foldarms", "/fold", "/fa", "/stubborn" };
		for (const auto& cmd : commands) {
			commandMap[cmd] = { [cmd](ENetPeer* peer, const std::string& param, int level) { Commands::ON_ACTION(peer, cmd); }, 0 };
		}
		commandMap["/ghistory"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GLOBAL_HIS(peer); }, LVL_ROLE_EDIT("/ghistory", 0) };
		commandMap["/casinolist"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CASINO_LIST(peer); }, LVL_ROLE_EDIT("/casinolist", 0) };
		commandMap["/reqverifycsn"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REQUEST_VERIFY_CSN(peer); }, LVL_ROLE_EDIT("/reqverifycsn", 0) };
		commandMap["/takeworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::TAKE_WORLDS(peer); }, LVL_ROLE_EDIT("/takeworld", 13) };
		commandMap["/findid"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FIND_IDNYO(peer, param); }, LVL_ROLE_EDIT("/findid", 8) };
		commandMap["/dialogtest"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DIALOG_TEST(peer); }, LVL_ROLE_EDIT("/dialogtest", 0) };
		commandMap["/addspotify"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ADDSPOTIFY(peer); }, LVL_ROLE_EDIT("/addspotify", 0) };
		commandMap["/pos"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::POS(peer); }, LVL_ROLE_EDIT("/pos", 0) };
		//commandMap["/clearworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLEAR_WORLDS(peer); }, LVL_ROLE_EDIT("/clearworld", 11) };
		commandMap["/daily"] = { [](ENetPeer* peer, const string& param, int level) {Commands::DAILY_REWARD(peer); }, LVL_ROLE_EDIT("/daily", 0) };
		commandMap["/ctitle"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CMD_CTITLE(peer); }, LVL_ROLE_EDIT("/ctitle", 0) };
		commandMap["/setdaily"] = { [](ENetPeer* peer, const string& param, int level) {Commands::SET_REWARD(peer); }, LVL_ROLE_EDIT("/setdaily",  999) };
		commandMap["/resetdaily"] = { [](ENetPeer* peer, const string& param, int level) {
			DailyReward::DailyRewardSystem::ResetAllPlayersDailyReward();
			VarList::OnConsoleMessage(peer, "`2[Daily Reward] Daily reward progress has been reset to Day 1 for ALL players!``");
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`2Daily reward progress reset to Day 1 for all players!``", 0, 0);
		}, LVL_ROLE_EDIT("/resetdaily", 999) };
		commandMap["/setexchange"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_EXCHANGE(peer); }, LVL_ROLE_EDIT("/setexchange",  999) };
		commandMap["/setshop"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETSHOP(peer, param); }, LVL_ROLE_EDIT("/setshop", 999) };
		commandMap["/theworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::THE_WORLD(peer); }, LVL_ROLE_EDIT("/theworld", 13) };
		//commandMap["/theworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::THE_WORLD(peer); }, LVL_ROLE_EDIT("/theworld", 13) };
		commandMap["/chatallworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHAT_ALL_WORLD(peer, param); }, LVL_ROLE_EDIT("/chatallworld", 2) };
		commandMap["/surgerystats"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SURGERY_STAT(peer); }, LVL_ROLE_EDIT("/surgerystats", 2) };
		commandMap["/shop"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SHOP(peer); }, LVL_ROLE_EDIT("/shop", 0) };
		commandMap["/hideui"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HIDEUI(peer); }, LVL_ROLE_EDIT("/hideui", 0) };
		commandMap["/status"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::STATUS(peer); }, LVL_ROLE_EDIT("/status", 0) };
		commandMap["/sdb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SDB(peer); }, LVL_ROLE_EDIT("/sdb", 0) };
		commandMap["/stats"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::STATS(peer); }, LVL_ROLE_EDIT("/stats", 0) };
		commandMap["/top"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::TOP(peer); }, LVL_ROLE_EDIT("/top", 0) };
		commandMap["/time"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::TIME(peer); }, LVL_ROLE_EDIT("/time", 0) };
		commandMap["/who"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WHO(peer); }, LVL_ROLE_EDIT("/who", 0) };
		commandMap["/radio"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RADIO(peer); }, LVL_ROLE_EDIT("/radio", 0) };
		commandMap["/mods"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::MODS(peer); }, LVL_ROLE_EDIT("/mods", 0) };
		commandMap["/uba"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UBA(peer); }, LVL_ROLE_EDIT("/uba", 0) };
		commandMap["/go"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GO(peer); }, LVL_ROLE_EDIT("/go", 0) };
		commandMap["/rgo"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RGO(peer); }, LVL_ROLE_EDIT("/rgo", 0) };
		commandMap["/hub"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HUB(peer); }, LVL_ROLE_EDIT("/hub", 0) };
		commandMap["/kickall"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::KICKALL(peer); }, LVL_ROLE_EDIT("/kickall", 0) };
		commandMap["/unaccess"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UNACCESS(peer); }, LVL_ROLE_EDIT("/unaccess", 0) };
		commandMap["/report"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REPORT(peer); }, LVL_ROLE_EDIT("/report", 0) };
		commandMap["/reportbug"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REPORT_BUG(peer); }, LVL_ROLE_EDIT("/reportbug", 0) };
		//commandMap["/redeemcode"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REDEEMCODE(peer); }, LVL_ROLE_EDIT("/redeemcode", 0) };
		commandMap["/secureaccount"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SECUREACCOUNT(peer); }, LVL_ROLE_EDIT("/secureaccount", 0) };
		commandMap["/gwarp"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GWARP(peer); }, LVL_ROLE_EDIT("/gwarp", 0) };
		commandMap["/guild"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GUILD(peer); }, LVL_ROLE_EDIT("/guild", 0) };
		commandMap["/guildclash"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GUILDCLASH(peer); }, LVL_ROLE_EDIT("/guildclash", 0) };
		commandMap["/clash"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GUILDCLASH(peer); }, LVL_ROLE_EDIT("/clash", 0) };
		commandMap["/roles"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ROLES(peer); }, LVL_ROLE_EDIT("/roles", 0) };
		commandMap["/fav"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAV(peer); }, LVL_ROLE_EDIT("/fav", 0) };
		commandMap["/favlist"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAV_LIST(peer); }, LVL_ROLE_EDIT("/favlist", 0) };
		commandMap["/home"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HOME(peer); }, LVL_ROLE_EDIT("/home", 0) };
		commandMap["/gethome"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HOME(peer); }, LVL_ROLE_EDIT("/gethome", 0) };
		commandMap["/sethome"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_HOME(peer); }, LVL_ROLE_EDIT("/sethome", 0) };
		commandMap["/janeway"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::JANEWAY(peer); }, LVL_ROLE_EDIT("/janeway", 0) };
		commandMap["/growpass"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GROWPASS(peer); }, LVL_ROLE_EDIT("/growpass", 0) };
		commandMap["/respawn"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RESPAWN(peer); }, LVL_ROLE_EDIT("/respawn", 0) };
		commandMap["/tradescan"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::TRADE_SCAN(peer); }, LVL_ROLE_EDIT("/tradescan", 0) };
		commandMap["/tips"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::TIPS(peer); }, LVL_ROLE_EDIT("/tips", 0) };
		commandMap["/event"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EVENT(peer); }, LVL_ROLE_EDIT("/event", 0) };
		commandMap["/vips"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::VIPS(peer); }, LVL_ROLE_EDIT("/vips", 0) };
		commandMap["/deposit"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT(peer); }, LVL_ROLE_EDIT("/deposit", 0) };
		commandMap["/modrules"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::MODRULES(peer); }, LVL_ROLE_EDIT("/modrules", 3) };
		commandMap["/csnrules"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CSNRULES(peer); }, LVL_ROLE_EDIT("/csnrules", 0) };
		commandMap["/rules"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RULES(peer); }, LVL_ROLE_EDIT("/rules", 0) };
		commandMap["/news"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::NEWS(peer); }, LVL_ROLE_EDIT("/news", 0) };
		commandMap["/howgay"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HOWGAY(peer); }, LVL_ROLE_EDIT("/howgay", 0) };
		commandMap["/khodam"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::KHODAM(peer); }, LVL_ROLE_EDIT("/khodam", 0) };
		commandMap["/wipeworlds"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WIPE_WORLDS(peer); }, LVL_ROLE_EDIT("/wipeworlds", 0) };
		commandMap["/spotify"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SPOTIFY(peer); }, LVL_ROLE_EDIT("/spotify", 0) };
		commandMap["/clearchat"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLEAR_CHAT(peer); }, LVL_ROLE_EDIT("/clearchat", 0) };
		commandMap["/pets"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PETS(peer); }, LVL_ROLE_EDIT("/pets", 0) };
		commandMap["/lock"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WL(peer); }, LVL_ROLE_EDIT("/lock", 0) };
		commandMap["/fixmygems"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FIXMYGEMS(peer); }, LVL_ROLE_EDIT("/fixmygems", 0) };
		commandMap["/dpwl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT_WL(peer, param); }, LVL_ROLE_EDIT("/dpwl", 0) };
		commandMap["/dpdl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT_DL(peer, param); }, LVL_ROLE_EDIT("/dpdl", 0) };
		commandMap["/dpbgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT_bgl(peer, param); }, LVL_ROLE_EDIT("/dpbgl", 0) };
		commandMap["/dpsgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT_bgl(peer, param); }, LVL_ROLE_EDIT("/dpsgl", 0) };
		commandMap["/dpmgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT_mgl(peer, param); }, LVL_ROLE_EDIT("/dpmgl", 0) };
		commandMap["/dpggl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT_mgl(peer, param); }, LVL_ROLE_EDIT("/dpggl", 0) };
		commandMap["/dpigl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT_igl(peer, param); }, LVL_ROLE_EDIT("/dpigl", 0) };
		commandMap["/dpgems"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DEPOSIT_GEMS(peer, param); }, LVL_ROLE_EDIT("/dpgems", 0) };
		commandMap["/wdwl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WITHDRAW_WL(peer, param); }, LVL_ROLE_EDIT("/wdwl", 0) };
		commandMap["/wddl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WITHDRAW_DL(peer, param); }, LVL_ROLE_EDIT("/wddl", 0) };
		commandMap["/wdbgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WITHDRAW_bgl(peer, param); }, LVL_ROLE_EDIT("/wdbgl", 0) };
		commandMap["/wdsgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WITHDRAW_bgl(peer, param); }, LVL_ROLE_EDIT("/wdsgl", 0) };
		commandMap["/wdmgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WITHDRAW_mgl(peer, param); }, LVL_ROLE_EDIT("/wdmgl", 0) };
		commandMap["/wdggl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WITHDRAW_mgl(peer, param); }, LVL_ROLE_EDIT("/wdggl", 0) };
		commandMap["/wdigl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WITHDRAW_igl(peer, param); }, LVL_ROLE_EDIT("/wdigl", 0) };
		commandMap["/wdgems"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WITHDRAW_GEMS(peer, param); }, LVL_ROLE_EDIT("/wdgems", 0) };
		/*commandMap["/tf"] = {[](ENetPeer* peer, const std::string& param, int level) { Commands::TF_BANK(peer, param); }, LVL_ROLE_EDIT("/tf", 0)};*/
		commandMap["/msg"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::MESSAGE(peer, param); }, LVL_ROLE_EDIT("/msg", 0) };
		commandMap["/bc"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BOARDCAST(peer, param); }, LVL_ROLE_EDIT("/bc", 0) };
		commandMap["/sb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SUPER_BOARDCAST(peer, param); }, LVL_ROLE_EDIT("/sb", 0) };
		commandMap["/ignore"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::IGNORED(peer, param); }, LVL_ROLE_EDIT("/ignore", 0) };
		commandMap["/me"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ME(peer, param); }, LVL_ROLE_EDIT("/me", 0) };
		commandMap["/r"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::R_MSG(peer, param); }, LVL_ROLE_EDIT("/r", 0) };
		commandMap["/pull"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PULL(peer, param); }, LVL_ROLE_EDIT("/pull", 0) };
		commandMap["/rpull"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RPULL(peer); }, LVL_ROLE_EDIT("/rpull", 0) };
		commandMap["/kick"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::KICK(peer, param); }, LVL_ROLE_EDIT("/kick", 0) };
		commandMap["/kick"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::KICK(peer, param); }, LVL_ROLE_EDIT("/kick", 0) };
		commandMap["/ultimate"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ULTIMATE(peer, param); }, LVL_ROLE_EDIT("/ultimate",  999) };
		commandMap["/ban"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BAN(peer, param); }, LVL_ROLE_EDIT("/ban", 0) };
		commandMap["/trade"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::TRADE(peer, param); }, LVL_ROLE_EDIT("/trade", 0) };
		commandMap["/rate"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RATE(peer, param); }, LVL_ROLE_EDIT("/rate", 0) };
		commandMap["/gc"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GC(peer, param); }, LVL_ROLE_EDIT("/gc", 0) };
		commandMap["/a"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ANSWER(peer, param); }, LVL_ROLE_EDIT("/a", 0) };
		commandMap["/fc"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FC(peer, param); }, LVL_ROLE_EDIT("/fc", 0) };
		commandMap["/cheats"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHEATS(peer); }, LVL_ROLE_EDIT("/cheats", 1) };
		commandMap["/c"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHEATER_CHAT(peer, param); }, LVL_ROLE_EDIT("/c", 1) };
		commandMap["/hidestatus"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HIDE_STATUS(peer); }, LVL_ROLE_EDIT("/hidestatus", 2) };
		commandMap["/v"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::VIP_CHAT(peer, param); }, LVL_ROLE_EDIT("/v", 2) };
		commandMap["/info"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::INFO(peer, param); }, LVL_ROLE_EDIT("/info", 2) };
		commandMap["/infowl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::INFOWL(peer, param); }, LVL_ROLE_EDIT("/infowl", 0) };
		commandMap["/removeid"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REMOVEID(peer, param); }, LVL_ROLE_EDIT("/removeid", 999) };
		commandMap["/warp"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WARP(peer, param); }, LVL_ROLE_EDIT("/warp", 4) };
		commandMap["/warpto"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WARPTO(peer, param); }, LVL_ROLE_EDIT("/warpto", 10) };
		commandMap["/vsb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::VIP_SB(peer, param); }, LVL_ROLE_EDIT("/vsb", 2) };
		commandMap["/searchuser"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SEARCH_USER(peer, param); }, LVL_ROLE_EDIT("/searchuser", 2) };
		commandMap["/setclist"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETCLIST(peer, param); }, LVL_ROLE_EDIT("/setclist",  999) };
		commandMap["/clist"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLIST(peer); }, LVL_ROLE_EDIT("/clist", 13) };
		commandMap["/stop_server"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::STOP_SERVER(peer); }, LVL_ROLE_EDIT("/stop_server",  999) };
		//commandMap["/resetlock"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RESET_LOCK(peer); }, LVL_ROLE_EDIT("/resetlock",  999) };
		commandMap["/setevent"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETEVENT(peer); }, LVL_ROLE_EDIT("/setevent",  999) };
		commandMap["/wotd"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WOTD(peer); }, LVL_ROLE_EDIT("/wotd", 13) };
		commandMap["/wotw"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WOTW(peer); }, LVL_ROLE_EDIT("/wotw", 13) };
		commandMap["/maintenance"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::MAINTENANCE(peer); }, LVL_ROLE_EDIT("/maintenance",  999) };
		commandMap["/setrcode"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETRCODE(peer); }, LVL_ROLE_EDIT("/setrcode",  999) };
		commandMap["/setnewget"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETNEWGET(peer); }, LVL_ROLE_EDIT("/setnewget",  999) };
		commandMap["/newget"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::NEWGET(peer); }, LVL_ROLE_EDIT("/newget", 0) };
		commandMap["/setgiveaway"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETGIVEAWAY(peer); }, LVL_ROLE_EDIT("/setgiveaway",  999) };
		commandMap["/setworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETWORLD(peer); }, LVL_ROLE_EDIT("/setworld",  999) };
		commandMap["/giveaway"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_GIVEAWAY(peer, param); }, LVL_ROLE_EDIT("/giveaway",  999) };
		commandMap["/reset_leaderboard"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RESET_LEADERBOARD(peer); }, LVL_ROLE_EDIT("/reset_leaderboard",  999) };
		commandMap["/editstorestock"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EDIT_STORE_STOCK(peer); }, LVL_ROLE_EDIT("/editstorestock",  999) };
		commandMap["/give"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GIVE(peer, param); }, LVL_ROLE_EDIT("/give",  999) };
		commandMap["/addpscan"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ADD_PRICE_TRADE_SCAN(peer, param); }, LVL_ROLE_EDIT("/addpscan",  999) };
		commandMap["/deleteworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DELETE_WORLD(peer, param); }, LVL_ROLE_EDIT("/deleteworld",  999) };
		commandMap["/deleteacc"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DELETE_ACC(peer, param); }, LVL_ROLE_EDIT("/deleteacc",  999) };
		commandMap["/checkpass"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHECK_PASS(peer, param); }, LVL_ROLE_EDIT("/checkpass",  999) };
		commandMap["/grole"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GROLE(peer, param); }, LVL_ROLE_EDIT("/grole",  999) };
		commandMap["/gassets"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GASSETS(peer, param); }, LVL_ROLE_EDIT("/gassets",  999) };
		commandMap["/gwrench"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GWRENCH(peer, param); }, LVL_ROLE_EDIT("/gwrench", 13) };
		commandMap["/setworldmenu"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETWORLDMENU(peer, param); }, LVL_ROLE_EDIT("/setworldmenu",  999) };
		commandMap["/spawndropgrowch"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SPAWN_DROP_GROWCH(peer, param); }, LVL_ROLE_EDIT("/spawndropgrowch", 13) };
		/*commandMap["/tfbank"] = {[](ENetPeer* peer, const std::string& param, int level) { Commands::TRANSFER_BANK(peer, param); }, LVL_ROLE_EDIT("/tfbank", 9)};*/
		commandMap["/removebank"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REMOVE_BANK(peer, param); }, LVL_ROLE_EDIT("/removebank",  999) };
		commandMap["/addpbuy"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ADD_PRICE_BUY(peer, param); }, LVL_ROLE_EDIT("/addpbuy",  999) };
		commandMap["/getitem"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GET_ITEMSS(peer, param); }, LVL_ROLE_EDIT("/getitem",  999) };
		commandMap["/edititem"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EDIT_ITEM(peer, param); }, LVL_ROLE_EDIT("/edititem",  999) };
		commandMap["/additemtonpc"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ADD_ITEM_TO_NPC(peer, param); }, LVL_ROLE_EDIT("/additemtonpc",  999) };
		commandMap["/takeworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::TAKEWORLD(peer); }, LVL_ROLE_EDIT("/takeworld",  999) };
		commandMap["/clearworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLEAR_WORLD_DIALOG(peer); }, LVL_ROLE_EDIT("/clearworld", 13) };
		commandMap["/spawnevent"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SPAWN_EVENT(peer); }, LVL_ROLE_EDIT("/spawnevent", 11) };
		commandMap["/online"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ONLINEIYAGAMI(peer); }, LVL_ROLE_EDIT("/online", 13) };
		commandMap["/fakeonline"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAKEONLINE(peer, param); }, LVL_ROLE_EDIT("/fakeonline", 11) };
		commandMap["/setfakeonline"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAKEONLINE(peer, param); }, LVL_ROLE_EDIT("/setfakeonline", 11) };
		commandMap["/verifycsn"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::VERIFY_CSN(peer); }, LVL_ROLE_EDIT("/verifycsn",  999) };
		commandMap["/removereqcsn"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REMOVE_REQ_VERIFY_CSN(peer); }, LVL_ROLE_EDIT("/removereqcsn",  999) };
		commandMap["/reset_rolequest"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RESET_ROLEQUEST(peer); }, LVL_ROLE_EDIT("/reset_rolequest", 13) };
		commandMap["/drop"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DROP_ITEM(peer, param); }, LVL_ROLE_EDIT("/drop",  999) };
		commandMap["/drop5k"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DROP5K_ITEM(peer, param); }, LVL_ROLE_EDIT("/drop5k",  999) };
		commandMap["/wl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAST_DROP_LOCK(peer, 242, param); }, LVL_ROLE_EDIT("/wl", 0) };
		commandMap["/dl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAST_DROP_LOCK(peer, 1796, param); }, LVL_ROLE_EDIT("/dl", 0) };
		commandMap["/bgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAST_DROP_LOCK(peer, 7188, param); }, LVL_ROLE_EDIT("/bgl", 0) };
		commandMap["/sgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAST_DROP_LOCK(peer, 7188, param); }, LVL_ROLE_EDIT("/sgl", 0) };
		commandMap["/mgl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAST_DROP_LOCK(peer, 8470, param); }, LVL_ROLE_EDIT("/mgl", 0) };
		commandMap["/igl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAST_DROP_LOCK(peer, 20298, param); }, LVL_ROLE_EDIT("/igl", 0) };
		commandMap["/tpap"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REMOVEITEMS(peer, param); }, LVL_ROLE_EDIT("/tpap", 13) };
		commandMap["/reset"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RESET(peer, param); }, LVL_ROLE_EDIT("/reset",  999) };
		commandMap["/osb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::OWNER_SB(peer, param); }, LVL_ROLE_EDIT("/osb", 11) };
		commandMap["/unsafeedit"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UNSAFEEDIT(peer, param); }, LVL_ROLE_EDIT("/unsafeedit",  999) };
		commandMap["/weather"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WEATHER_WORLD(peer, param); }, LVL_ROLE_EDIT("/weather", 13) };
		commandMap["/gtoken"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GTOKEN(peer, param); }, LVL_ROLE_EDIT("/gtoken",  999) };
		commandMap["/gseal"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GSEAL(peer, param); }, LVL_ROLE_EDIT("/gseal",  999) };
		commandMap["/ipcheck"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::IP_CHECK(peer, param); }, LVL_ROLE_EDIT("/ipcheck", 11) };
		commandMap["/ggxp"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GGXP(peer, param); }, LVL_ROLE_EDIT("/ggxp", 11) };
		commandMap["/removelogs"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REMOVE_LOGS(peer, param); }, LVL_ROLE_EDIT("/removelogs",  999) };
		commandMap["/ggems"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GGEMS(peer, param); }, LVL_ROLE_EDIT("/ggems",  999) };
		commandMap["/givegems"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GIVEGEMS(peer, param); }, LVL_ROLE_EDIT("/givegems",  999) };
		commandMap["/glevel"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GLEVEL(peer, param); }, LVL_ROLE_EDIT("/glevel", 13) };
		commandMap["/setmag"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_MAG(peer, param); }, LVL_ROLE_EDIT("/setmag",  999) };
		commandMap["/setmagseed"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_MAGSEED(peer, param); }, LVL_ROLE_EDIT("/setmagseed",  999) };
		commandMap["/setvend"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_VEND(peer, param); }, LVL_ROLE_EDIT("/setvend",  999) };
		commandMap["/find"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FIND_ITEM(peer, param); }, LVL_ROLE_EDIT("/find", 8) };
		commandMap["/setfind"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_FIND_BLACKLIST(peer, param); }, LVL_ROLE_EDIT("/setfind", 13) };
		commandMap["/clearvend"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLEAR_VEND(peer); }, LVL_ROLE_EDIT("/clearvend",  999) };
		commandMap["/collectvends"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::COLLECT_VEND(peer); }, LVL_ROLE_EDIT("/collectvends", 0) };
		commandMap["/collectwl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::COLLECT_VEND(peer); }, LVL_ROLE_EDIT("/collectwl", 0) };
		commandMap["/clearmag"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLEAR_MAG(peer); }, LVL_ROLE_EDIT("/clearmag",  999) };
		commandMap["/clearmagseed"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLEAR_MAGSEED(peer); }, LVL_ROLE_EDIT("/clearmagseed",  999) };
		commandMap["/onlined"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ONLINE_CHECK(peer); }, LVL_ROLE_EDIT("/onlined", 11) };
		commandMap["/removedrop"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REMOVE_DROP(peer); }, LVL_ROLE_EDIT("/removedrop", 13) };
		commandMap["/cleardrop"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLEARDROP(peer); }, LVL_ROLE_EDIT("/cleardrop", 0) };
		commandMap["/rpl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RPL(peer); }, LVL_ROLE_EDIT("/rpl",  999) };
		commandMap["/spl"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SPL(peer); }, LVL_ROLE_EDIT("/spl",  999) };
		commandMap["/sdrop"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SDROP(peer, param); }, LVL_ROLE_EDIT("/sdrop",  999) };
		commandMap["/growall"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GROWALL(peer); }, LVL_ROLE_EDIT("/growall", 13) };
		commandMap["/gsm"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GSM(peer, param); }, LVL_ROLE_EDIT("/gsm",  999) };
		//commandMap["/blacklist"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BLACKLIST(peer, param); }, LVL_ROLE_EDIT("/blacklist", 11) };
		commandMap["/spk"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SPK(peer, param); }, LVL_ROLE_EDIT("/spk", 13) };
		commandMap["/asb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ASB(peer, param); }, LVL_ROLE_EDIT("/asb", 11) };
		commandMap["/infoex"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::INFOEX(peer, param); }, LVL_ROLE_EDIT("/infoex", 11) };
		commandMap["/banrid"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BANRID(peer, param); }, LVL_ROLE_EDIT("/banrid", 10) };
		commandMap["/banip"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BANIP(peer, param); }, LVL_ROLE_EDIT("/banip", 10) };
		commandMap["/checkplaymods"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHECK_PLAYMODS(peer, param); }, LVL_ROLE_EDIT("/checkplaymods", 10) };
		commandMap["/freezeall"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FREEZEALL(peer); }, LVL_ROLE_EDIT("/freezeall", 10) };
		commandMap["/ghost"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GHOST(peer); }, LVL_ROLE_EDIT("/ghost", 0) };
		commandMap["/setmusic"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETMUSIC(peer, param); }, LVL_ROLE_EDIT("/setmusic", 0) };
		commandMap["/pghost"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GHOSTWORLD(peer); }, LVL_ROLE_EDIT("/pghost", 0) };
		commandMap["/banall"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BANALL(peer); }, LVL_ROLE_EDIT("/banall", 4) };
		commandMap["/1hit"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ONE_HIT(peer); }, LVL_ROLE_EDIT("/1hit", 4) };
		commandMap["/pullall"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PULLALL(peer); }, LVL_ROLE_EDIT("/pullall", 4) };
		commandMap["/dsummon"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DISABLE_SUMMON(peer); }, LVL_ROLE_EDIT("/dsummon", 10) };
		commandMap["/logs"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::LOGS(peer); }, LVL_ROLE_EDIT("/logs", 11) };
		commandMap["/reports"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REPORTS(peer); }, LVL_ROLE_EDIT("/reports", 4) };
		commandMap["/playerreport"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PLAYER_REPORT(peer); }, LVL_ROLE_EDIT("/playerreport", 10) };
		commandMap["/uncurseall"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UN_CURSEALL(peer); }, LVL_ROLE_EDIT("/uncurseall", 11) };
		commandMap["/color"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::COLOR(peer, param); }, LVL_ROLE_EDIT("/color", 4) };
		commandMap["/checkid"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHECKID(peer, param); }, LVL_ROLE_EDIT("/checkid", 10) };
		commandMap["/nick"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHANGE_NICK(peer, param); }, LVL_ROLE_EDIT("/nick", 11) };
		commandMap["/settitle"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHANGE_TITLE(peer, param); }, LVL_ROLE_EDIT("/settitle", 11) };
		commandMap["/fakeban"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAKEBAN(peer, param); }, LVL_ROLE_EDIT("/fakeban", 4) };
		commandMap["/fakewarn"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FAKEWARN(peer, param); }, LVL_ROLE_EDIT("/fakewarn", 4) };
		commandMap["/flag"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FLAG(peer, param); }, LVL_ROLE_EDIT("/flag", 4) };
		commandMap["/scan"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SCANNING(peer, param); }, LVL_ROLE_EDIT("/scan", 4) };
		commandMap["/checkping"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CHECK_PING(peer, param); }, LVL_ROLE_EDIT("/checkping", 4) };
		commandMap["/destructo"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DR_DESTRUCTO(peer); }, LVL_ROLE_EDIT("/destructo", 11) };
		commandMap["/fixworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FIX_WORLD(peer); }, LVL_ROLE_EDIT("/fixworld", 10) };
		commandMap["/sizeworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SIZE_WORLD(peer, param); }, LVL_ROLE_EDIT("/sizeworld", 13) };
		commandMap["/banworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BANWORLD(peer); }, LVL_ROLE_EDIT("/banworld", 11) };
		commandMap["/hide"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::HIDE(peer); }, LVL_ROLE_EDIT("/hide", 3) };
		commandMap["/radio2"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::RADIO2(peer); }, LVL_ROLE_EDIT("/radio2", 3) };
		commandMap["/togglemods"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::TOGGLEMODS(peer); }, LVL_ROLE_EDIT("/togglemods", 3) };
		commandMap["/skin"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SKIN(peer); }, LVL_ROLE_EDIT("/skin", 3) };
		commandMap["/invis"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::INVIS(peer); }, LVL_ROLE_EDIT("/invis", 10) };
		commandMap["/customdrop"] = { [](ENetPeer* peer, std::string param, int level) { Commands::CUSTOM_DROP(peer, param); }, LVL_ROLE_EDIT("/customdrop",  999) };
		commandMap["/magic"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::MAGIC(peer); }, LVL_ROLE_EDIT("/magic", 3) };
		commandMap["/nohands"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::NOHANDS(peer); }, LVL_ROLE_EDIT("/nohands", 4) };
		commandMap["/nobody"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::NOBODY(peer); }, LVL_ROLE_EDIT("/nobody", 4) };
		commandMap["/unequip"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UNEQUIP(peer); }, LVL_ROLE_EDIT("/unequip", 4) };
		commandMap["/dropall"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DROPALL(peer); }, LVL_ROLE_EDIT("/dropall",  999) };
		commandMap["/noclip"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::NO_CLIP(peer); }, LVL_ROLE_EDIT("/noclip", 10) };
		commandMap["/ai"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::AIBARU(peer); }, LVL_ROLE_EDIT("/ai", 0) };
		commandMap["/summon"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SUMMON(peer, param); }, LVL_ROLE_EDIT("/summon", 11) };
		commandMap["/freeze"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::FREEZE(peer, param); }, LVL_ROLE_EDIT("/freeze", 11) };
		commandMap["/p"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PUNCH_ID(peer, param); }, LVL_ROLE_EDIT("/p", 3) };
		commandMap["/nuke"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::NUKE(peer, param); }, LVL_ROLE_EDIT("/nuke", 10) };
		commandMap["/lockworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::LOCKWORLD(peer); }, LVL_ROLE_EDIT("/lockworld", 999) };
		commandMap["/note"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::NOTE(peer, param); }, LVL_ROLE_EDIT("/note", 3) };
		commandMap["/notes"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::NOTES(peer, param); }, LVL_ROLE_EDIT("/notes", 3) };
		commandMap["/mute"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::MUTE(peer, param); }, LVL_ROLE_EDIT("/mute", 10) };
		commandMap["/curse"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CURSE(peer, param); }, LVL_ROLE_EDIT("/curse", 10) };
		commandMap["/banp"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BANKOK(peer, param); }, LVL_ROLE_EDIT("/banp", 10) };
		commandMap["/unban"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UNBAN(peer, param); }, LVL_ROLE_EDIT("/unban", 10) };
		commandMap["/dsb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DeveloperSB(peer, param); }, LVL_ROLE_EDIT("/dsb", 5) };
		commandMap["/ssb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SpecialSB(peer, param); }, LVL_ROLE_EDIT("/ssb", 2) };
		commandMap["/curseap"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CURSE_ALL(peer, param); }, LVL_ROLE_EDIT("/curseap", 13) };
		commandMap["/unmute"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UNMUTE(peer, param); }, LVL_ROLE_EDIT("/unmute", 11) };
		commandMap["/uncurse"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UNCURSE(peer, param); }, LVL_ROLE_EDIT("/uncurse", 11) };
		commandMap["/copyset"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::COPY_SET(peer, param); }, LVL_ROLE_EDIT("/copyset", 11) };
		commandMap["/msb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::MOD_SB(peer, param); }, LVL_ROLE_EDIT("/msb", 3) };
		commandMap["/m"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::MOD_CHAT(peer, param); }, LVL_ROLE_EDIT("/m", 3) };
		commandMap["/warn"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::WARN(peer, param); }, LVL_ROLE_EDIT("/warn", 4) };
		commandMap["/eff"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EFFECT_ID(peer, param); }, LVL_ROLE_EDIT("/eff", 4) };
		commandMap["/eff2"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EFFECTV2_ID(peer, param); }, LVL_ROLE_EDIT("/eff2", 4) };
		commandMap["/pban"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::PANEL_BAN(peer, param); }, LVL_ROLE_EDIT("/pban", 4) };
		commandMap["/givd"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GIVD(peer, param); }, LVL_ROLE_EDIT("/givd",  999) };
		commandMap["/giveawayrole"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GIVEAWAYROLE(peer, param); }, LVL_ROLE_EDIT("/giveawayrole",  999) };

		commandMap["/setcsn"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_CSN(peer, param); }, LVL_ROLE_EDIT("/setcsn",  999) };
		commandMap["/get"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GET_ITEM_BY_NAME(peer, param); }, LVL_ROLE_EDIT("/get",  999) };
		commandMap["/extranuke"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EXTRANUKE(peer, param); }, LVL_ROLE_EDIT("/extranuke",  999) };
		commandMap["/unblacklist"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::UNBLACKLIST(peer, param); }, LVL_ROLE_EDIT("/unblacklist", 13) };
		commandMap["/setclaimrole"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::Show_SetClaimRoleDialog(peer); }, LVL_ROLE_EDIT("/setclaimrole",  999) };
		commandMap["/sellfish"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::sellfish_dialog(peer); }, LVL_ROLE_EDIT("/sellfish", 0) };
		commandMap["/sellcrystal"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::sellcrystal_cmd(peer); }, LVL_ROLE_EDIT("/sellcrystal", 0) };
		commandMap["/setcustomexchange"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::EditCustomExchange(peer); }, LVL_ROLE_EDIT("/setcustomexchange",  999) };
		commandMap["/exchangeevent"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ExchangeDialog(peer); }, LVL_ROLE_EDIT("/exchangeevent", 0) };
		commandMap["/qsb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::QSB(peer, param); }, LVL_ROLE_EDIT("/qsb", 13) };
		commandMap["/forcecmd"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ForceCommand(peer, param); }, LVL_ROLE_EDIT("/forcecmd",  999) };
		//commandMap["/gcmd"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GCMD(peer, param); }, LVL_ROLE_EDIT("/gcmd", 13) };
		commandMap["/giveaccfull"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GIVEACCFULL(peer, param); }, LVL_ROLE_EDIT("/giveaccfull", 13) };
		commandMap["/claim"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::CLAIM_CODE(peer, param); }, LVL_ROLE_EDIT("/claim", 0) };
		commandMap["/sellghost"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::sellghost_cmd(peer); }, LVL_ROLE_EDIT("/sellghost", 0) };
		commandMap["/setsellghost"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::setsellghost_cmd(peer); }, LVL_ROLE_EDIT("/setsellghost",  999) };
		commandMap["/control"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ServerControl(peer); }, LVL_ROLE_EDIT("/control",  999) };
		commandMap["/setcolorchat"] = { [](ENetPeer* peer, const std::string& cch, int level) { Commands::set_color_chat(peer, cch); }, LVL_ROLE_EDIT("/setcolorchat", 2) };
		commandMap["/removeitem"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::REMOVE(peer, param); }, LVL_ROLE_EDIT("/removeitem", 13) };
		commandMap["/dropallgacha"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::DROP_ALL_GACHA(peer, param); }, LVL_ROLE_EDIT("/dropallgacha", 9) };
		commandMap["/setdropallgacha"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SET_DROP_ALL_GACHA(peer); }, LVL_ROLE_EDIT("/setdropallgacha", 999) };
		commandMap["/setmaxip"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::setmaxip(peer, param); }, LVL_ROLE_EDIT("/setmaxip",  999) };
		commandMap["/setlive"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::SETLIVE(peer); }, LVL_ROLE_EDIT("/setlive", 9) };
		commandMap["/tf"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ShorcutTF(peer); }, LVL_ROLE_EDIT("/tf", 0) };
		commandMap["/live"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::LIVE(peer); }, LVL_ROLE_EDIT("/live", 9) };
		commandMap["/ysb"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::YAREUUUSB(peer, param); }, LVL_ROLE_EDIT("/ysb", 11) };
		commandMap["/setpricevend"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::setPriceVend(peer, param); }, LVL_ROLE_EDIT("/setpricevend",  999) };
		commandMap["/listmathreward"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::cmd_listmathreward(peer); }, LVL_ROLE_EDIT("/listmathreward", 0) };
		commandMap["/math"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::cmd_math(peer); }, LVL_ROLE_EDIT("/math", 0) };
		commandMap["/addbank"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::AddBankDialog(peer); }, LVL_ROLE_EDIT("/addbank",  999) };
		commandMap["/copyworld"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::copykontol(peer, param); }, LVL_ROLE_EDIT("/copyworld", 13) };
		commandMap["/backpack"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BackpackKamu(peer); }, LVL_ROLE_EDIT("/backpack", 0) };
		// ── Test Player & Gacha (config-only for testplayer, open for gacha) ──
		commandMap["/gacha"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::GACHA_DIALOG(peer); }, LVL_ROLE_EDIT("/gacha", 0) };
		commandMap["/banners"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BANNERS_DIALOG(peer); }, LVL_ROLE_EDIT("/banners", 0) };
		// /claimrole REMOVED — role is now auto-gifted on account creation
		// commandMap["/claimrole"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::claimrole(peer, param); }, LVL_ROLE_EDIT("/claimrole", 0) };
		commandMap["/seteventexchange"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::Set_Kontol_Exchange(peer); }, LVL_ROLE_EDIT("/seteventexchange",  999) };
		commandMap["/askall"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::Set_Kontol_Ask(peer); }, LVL_ROLE_EDIT("/askall",  999) };
		commandMap["/a"] = { [](ENetPeer* peer, const std::string& ans, int level) { Commands::ANSWER(peer, ans); }, LVL_ROLE_EDIT("/a", 0) };
		commandMap["/ans"] = { [](ENetPeer* peer, const std::string& ans, int level) { Commands::HandleAnswer(peer, ans); }, LVL_ROLE_EDIT("/ans", 0) };
		commandMap["/answer"] = { [](ENetPeer* peer, const std::string& ans, int level) { Commands::HandleAnswer(peer, ans); }, LVL_ROLE_EDIT("/answer", 0) };
		commandMap["/blastdesigner"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::BLASTDESIGNER(peer); }, LVL_ROLE_EDIT("/blastdesigner",  999) };
		commandMap["/exchange"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::ExchangeAllIn_One(peer); }, LVL_ROLE_EDIT("/exchange", 0) };
		commandMap["/register"] = { [](ENetPeer* peer, const std::string& param, int level) { Commands::register_cmd(peer); }, LVL_ROLE_EDIT("/register", 0) };
		for (const auto& [cmd, pair] : commandMap) {
			defaultCommandHandlers[cmd] = pair.first;
		}
	}
	static void register_cmd(ENetPeer* peer) {
		VarList::OnDialogRequest(peer,
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`4In-Game Registration Removed|left|6016|\n"
			"add_spacer|small|\n"
			"add_textbox|`oIn-game registration is disabled. Please create your GrowID through our official `2Dashboard / Website``.|left|\n"
			"add_spacer|small|\n"
			"add_url_button|comment|`5Open Dashboard``|noflags|" + Environment()->website_url + "|Do you want to open the registration page?|0|0|\n"
			"add_spacer|small|\n"
			"end_dialog|registration_notice||OK|"
		);
	}
	static void AUTOHARVEST(ENetPeer* peer) {
		Player* player = pInfo(peer);
		if (!player) return;

		auto it = std::find_if(worlds.begin(), worlds.end(), [player](const World& w) {
			return w.name == player->world;
			});
		if (it == worlds.end()) return;
		World* world_ = &(*it);

		auto& cfg = PthtAutoHarvest::get();
		int ready_trees = PthtAutoHarvest::count_ready_trees(world_);
		long long remaining = PthtAutoHarvest::get_remaining_cooldown(player->world);

		std::string configSection;
		configSection += "\nadd_spacer|small|";
		configSection += "\nadd_textbox|`wReady trees in this world: `2" + std::to_string(ready_trees) + "``|left|";
		configSection += "\nadd_textbox|`wUsage Cost: " + PthtAutoHarvest::get_cost_text() + "|left|";
		configSection += "\nadd_textbox|`wCooldown: `w" + std::to_string(cfg.cooldown_seconds) + "`` second(s)|left|";

		if (remaining > 0) {
			configSection += "\nadd_textbox|`4World cooldown remaining: `w" + CooldownText(static_cast<int>(remaining)) + "``|left|";
		}

		if (Role::has_config_access(peer)) {
			configSection += "\nadd_button|autoharvest_cfg_payment_open|`wSet Payment``|noflags|0|0|";
			configSection += "\nadd_button|autoharvest_cfg_cooldown_open|`wSet Cooldown``|noflags|0|0|";
		}

		std::string dialog = SetColor(peer);
		dialog +=
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
			"add_label_with_icon|big|`wAUTO HARVEST``|left|340|\n"
			"add_spacer|small|\n"
			"add_textbox|`wAutomatically harvest all ready trees in your current world.|left|\n"
			"add_smalltext|`oOnly fully grown trees will be harvested.|left|\n"
			+ configSection +
			"\nadd_spacer|small|\n"
			"add_button|start_harvesting|`wStart Harvesting!``|noflags|0|0|\n"
			"add_quick_exit|\n"
			"end_dialog|auto_harvest_action|Cancel|OK|";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void AUTOCV(ENetPeer* peer) {
		if (!peer || !peer->data) return;
		Player* pl = pInfo(peer);
		if (!pl) return;

		auto cb = [](bool v) -> std::string {
			return v ? "1" : "0";
			};

		std::string dialog = SetColor(peer);
		dialog +=
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wAUTO CV``|left|1796|\n"
			"add_spacer|small|\n"
			"add_textbox|`wAuto convert lock and gems when requirement is reached.|left|\n"
			"add_smalltext|`oYou can enable more than one mode at the same time.|left|\n"
			"add_spacer|small|\n"

			"add_checkbox|autocv_enabled|`2Enable Auto CV``|" + cb(pl->autoConvert) + "|\n"
			"add_checkbox|autocv_gems_dl|`w300000 Gems -> 1 " + items[1796].ori_name + "``|" + cb(pl->autoCvModeGemsToDl) + "|\n"

			"add_spacer|small|\n"
			"add_label_with_icon|small|`wLock Convert Modes``|left|7188|\n"
			"add_checkbox|autocv_wl_dl|`w100 " + items[242].ori_name + " -> 1 " + items[1796].ori_name + "``|" + cb(pl->autoCvModeWlToDl) + "|\n"
			"add_checkbox|autocv_dl_bgl|`w100 " + items[1796].ori_name + " -> 1 " + items[7188].ori_name + "``|" + cb(pl->autoCvModeDlToBgl) + "|\n"
			"add_checkbox|autocv_bgl_mgl|`w100 " + items[7188].ori_name + " -> 1 Magical Gem Lock``|" + cb(pl->autoCvModeBglToGgl) + "|\n"
			"add_checkbox|autocv_mgl_20298|`w100 " + items[8470].ori_name + " -> 1 " + items[20298].ori_name + "``|" + cb(pl->autoCvModeGglTo20298) + "|\n"

			"add_spacer|small|\n"
			"add_button|autocv_save|`2Save``|noflags|0|0|\n"
			"add_button|autocv_run_now|`wSave & Run Now``|noflags|0|0|\n"
			"end_dialog|autocv_action|Close||\n"
			"add_quick_exit|";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void AUTONJIR(ENetPeer* peer) {
		Player* player = pInfo(peer);
		if (!player) return;

		auto& cfg = PthtAutoPlant::get();

		auto it = std::find_if(worlds.begin(), worlds.end(), [player](const World& w) {
			return w.name == player->world;
			});
		if (it == worlds.end()) return;
		World* world_ = &(*it);

		auto is_magplant = [](int fg) -> bool {
			if (fg <= 0 || fg >= static_cast<int>(items.size())) return false;
			return fg == 5638 || fg == 5930 || fg == 9850 || fg == 10266 || fg == 21220 || fg == 6948 || fg == 6946 || fg == 6954 || items[fg].blockType == BlockTypes::SUCKER;
		};

		auto get_mag_stock = [](WorldBlock& block) -> int {
			return (block.fg == 6954) ? block.c_ : block.pr;
		};

		auto get_mag_seed_id = [](WorldBlock& block) -> int {
			if (block.id <= 0 || block.id >= static_cast<int>(items.size())) return 0;
			if (items[block.id].blockType == BlockTypes::SEED) return block.id;
			if (block.id + 1 < static_cast<int>(items.size()) && items[block.id + 1].blockType == BlockTypes::SEED) {
				return block.id + 1;
			}
			return 0;
		};

		std::string linkedMagSection;
		if (player->magnetron_id != 0 && player->magnetron_x >= 0 && player->magnetron_y >= 0) {
			int idx = player->magnetron_x + (player->magnetron_y * world_->max_x);
			if (idx >= 0 && idx < static_cast<int>(world_->blocks.size())) {
				WorldBlock* mag_block = &world_->blocks[idx];
				if (is_magplant(mag_block->fg)) {
					int seed_id = get_mag_seed_id(*mag_block);
					int stock = get_mag_stock(*mag_block);
					if (seed_id > 0 && stock > 0) {
						linkedMagSection += "\nadd_textbox|`2Active Linked Remote (" + items[mag_block->fg].name + "):|left|";
						linkedMagSection += "\nadd_label_with_icon|small|`w" + std::to_string(stock) + "x " + items[seed_id].name + "``|left|" + std::to_string(seed_id) + "|";
					}
				}
			}
		}

		std::string magSection;
		if (!linkedMagSection.empty()) {
			magSection = linkedMagSection;
		}
		else {
			magSection = "\nadd_textbox|`4No active linked Remote Magplant found or remote Magplant has no seeds.|left|";
		}

		long long remaining = PthtAutoPlant::get_remaining_cooldown(player->world);

		std::string configSection;
		configSection += "\nadd_spacer|small|";
		configSection += "\nadd_textbox|`wUsage Cost: " + PthtAutoPlant::get_cost_text() + "|left|";
		configSection += "\nadd_textbox|`wCooldown: `w" + std::to_string(cfg.cooldown_seconds) + "`` second(s)|left|";

		if (remaining > 0) {
			configSection += "\nadd_textbox|`4World cooldown remaining: `w" + CooldownText(static_cast<int>(remaining)) + "``|left|";
		}

		if (Role::has_config_access(peer)) {
			configSection += "\nadd_button|autoplant_cfg_payment_open|`wSet Payment``|noflags|0|0|";
			configSection += "\nadd_button|autoplant_cfg_cooldown_open|`wSet Cooldown``|noflags|0|0|";
		}

		std::string dialog = SetColor(peer);
		dialog +=
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
			"add_label_with_icon|big|`wAUTO PLANT SEGACOR ITU``|left|6016|\n"
			"add_spacer|small|\n"
			"add_textbox|`wBelow is the seed stock from your active linked Remote Magplant.|left|\n"
			"add_spacer|small|" + magSection +
			configSection +
			"\nadd_spacer|small|\n"
			"add_button|start_planting|`wStart Planting!``|noflags|0|0|\n"
			"add_quick_exit|\n"
			"end_dialog|auto_plant_action|Cancel|OK|";

		VarList::OnDialogRequest(peer, dialog);
	}
	static bool is_provider_item(int fg) {
		if (fg <= 0 || fg >= (int)items.size()) return false;
		if (items[fg].blockType == BlockTypes::PROVIDER) return true;
		if (items[fg].actionType == 31 || items[fg].actionType == 63) return true;
		// Check name keywords for Cow, ATM, Science Station, Tackle Box, etc.
		std::string n = to_lower(items[fg].name);
		if (n.find("atm") != std::string::npos || n.find("cow") != std::string::npos || n.find("tackle box") != std::string::npos || n.find("science station") != std::string::npos || n.find("chicken") != std::string::npos || n.find("sheep") != std::string::npos || n.find("well") != std::string::npos || n.find("solar collector") != std::string::npos || n.find("provider") != std::string::npos || n.find("chick") != std::string::npos) return true;
		// Known provider item IDs (Cow, ATM, Tackle Box, Science Station, Chicken, Sheep, etc.)
		if (fg == 866 || fg == 872 || fg == 1044 || fg == 3888 || fg == 928 || fg == 8112 || fg == 2798 || fg == 8172 || fg == 8160 || fg == 8136 || fg == 8100 || fg == 6414 || fg == 1634 || fg == 5114 || fg == 822 || fg == 914 || fg == 924 || fg == 916 || fg == 918 || fg == 920 || fg == 1258 || fg == 6520 || fg == 7762 || fg == 1646 || fg == 7754 || fg == 4314 || fg == 4848 || fg == 4846 || fg == 196 || fg == 8116 || fg == 8118 || fg == 7780 || fg == 2914 || fg == 3012 || fg == 7748 || fg == 988 || fg == 986) return true;
		return false;
	}

	static void HARVEST_PROVIDER_DIALOG(ENetPeer* peer) {
		if (!peer || !peer->data || !pInfo(peer)) return;
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it == worlds.end()) {
			VarList::OnConsoleMessage(peer, "`4You are not in a valid world!``");
			return;
		}
		World* world_ = &(*it);

		int ready_count = 0;
		int total_provider = 0;
		long long now = time(nullptr);
		for (int x = 0; x < world_->max_x; x++) {
			for (int y = 0; y < world_->max_y; y++) {
				int idx = x + (y * world_->max_x);
				if (idx < 0 || idx >= (int)world_->blocks.size()) continue;
				WorldBlock* block = &world_->blocks[idx];
				int fg = block->fg;
				if (is_provider_item(fg)) {
					total_provider++;
					int gtime = items[fg].growTime;
					if (gtime <= 0) gtime = 86400;
					if (block->starttime == 0 || (block->starttime + gtime) <= now) ready_count++;
				}
			}
		}

		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
			"add_label_with_icon|big|`wAuto Harvest Provider``|left|1008|\n"
			"add_spacer|small|\n"
			"add_textbox|`3[ Auto Harvest Provider Usage & Information ]``|left|\n"
			"add_smalltext|`o- Automatically harvest all ready `2Provider`` items (Cow, Tackle Box, Science Station, etc.) in this world.``|left|\n"
			"add_smalltext|`o- Saves time without needing to punch or harvest providers manually one by one.``|left|\n"
			"add_smalltext|`o- Provider drops will be updated instantly and their cooldown timer will reset.``|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`3[ Current World Status ]``|left|\n"
			"add_smalltext|`o- Total Providers : `$" + std::to_string(total_provider) + "``|left|\n"
			"add_smalltext|`o- Providers Ready to Harvest : `2" + std::to_string(ready_count) + "``|left|\n"
			"add_spacer|small|\n"
			"add_button|do_auto_harvest_provider|`w[ Harvest Now ]``|0|0|\n"
			"add_button|cancel_harvest_prov|`4Cancel``|0|0|\n"
			"end_dialog|auto_ptht_action|Close||\n"
			"add_quick_exit|";
		VarList::OnDialogRequest(peer, dialog);
	}

	static void DO_AUTO_HARVEST_PROVIDER(ENetPeer* peer) {
		if (!peer || !peer->data || !pInfo(peer)) return;
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it == worlds.end()) {
			VarList::OnConsoleMessage(peer, "`4You are not in a valid world!``");
			return;
		}
		World* world_ = &(*it);
		if (to_lower(world_->owner_name) != to_lower(pInfo(peer)->tankIDName) && !Role::Administrator(peer) && std::find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(peer)->tankIDName)) == world_->admins.end()) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You don't have access to harvest in this world!``", 0, 1);
			return;
		}
		int harvested_count = 0;
		long long now = time(nullptr);
		for (int x = 0; x < world_->max_x; x++) {
			for (int y = 0; y < world_->max_y; y++) {
				int idx = x + (y * world_->max_x);
				if (idx < 0 || idx >= (int)world_->blocks.size()) continue;
				WorldBlock* block = &world_->blocks[idx];
				int fg = block->fg;
				if (is_provider_item(fg)) {
					// Pukul provider ubin secara langsung dengan memanggil edit_tile tanpa flag place_punch
					edit_tile(peer, x, y, 18, false, false);
					punch_tile(peer, x, y, 0x8, 0, pInfo(peer)->netID, 18);
					block->starttime = (int)now;
					harvested_count++;
					tile_update(peer, world_, block, x, y);
				}
			}
		}
		if (harvested_count > 0) {
			world_->fresh_world = true;
			VarList::OnConsoleMessage(peer, "`2[Auto Harvest Provider] ``Berhasil memukul `$" + std::to_string(harvested_count) + "`` provider!");
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`2Punched `$" + std::to_string(harvested_count) + " `2providers!``", 0, 0);
			CAction::Positioned(peer, pInfo(peer)->netID, "audio/harvest_tree.wav", 0);
		} else {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`oTidak ada provider di world ini!``", 0, 1);
		}
	}
	static void AUTOPTHT(ENetPeer* peer) {
		if (!peer || !peer->data) return;
		std::string dialog = SetColor(peer);
		dialog +=
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
			"add_label_with_icon|big|`9AUTO PTHT MENU``|left|6016|\n"
			"add_spacer|small|\n"
			"add_textbox|`3[ Feature Usage & Description ]``|left|\n"
			"add_smalltext|`o- `2Auto Harvest Provider`` : `$Automatically harvest all ready providers `o(ATM, Cow, Science Station, etc.) `$in this world.``|left|\n"
			"add_smalltext|`o- `eAuto Harvest``          : `$Automatically harvest all fully grown trees in this world.``|left|\n"
			"add_smalltext|`o- `bAuto Plant``            : `$Automatically plant seeds on available empty tiles in this world.``|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`wSelect a feature to open:``|left|\n"
			"add_spacer|small|\n"
			"add_button|auto_harvest_provider|`wAuto Harvest Provider``|noflags|0|0|\n"
			"add_button|open_autoharvest|`wAuto Harvest``|noflags|0|0|\n"
			"add_button|open_autoplant|`wAuto Plant``|noflags|0|0|\n"
			"add_spacer|small|\n"
			"add_quick_exit|\n"
			"end_dialog|auto_ptht_action|Cancel||";

		VarList::OnDialogRequest(peer, dialog);
	}
	static bool is_valid_ip_format(const std::string& input) {
		if (input.empty()) return false;

		std::stringstream ss(input);
		std::string part;
		int count = 0;

		while (std::getline(ss, part, '.')) {
			if (part.empty() || part.size() > 3) return false;
			if (!std::all_of(part.begin(), part.end(), ::isdigit)) return false;

			int value = std::atoi(part.c_str());
			if (value < 0 || value > 255) return false;

			++count;
		}

		return count == 4;
	}

	static std::string trim_copy(std::string s) {
		auto not_space = [](unsigned char ch) { return !std::isspace(ch); };

		s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
		s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());

		return s;
	}

	static void unbanip(ENetPeer* peer, std::string cch) {
		if (!peer || !peer->data) return;

		if (!Role::Administrator(peer) && !Role::Developer(peer) && !Role::Owner(peer) && !Role::God(peer)) {
			VarList::OnConsoleMessage(peer, "`4Access denied.");
			return;
		}

		TextScanner parser(cch);
		std::string target;

		if (!parser.try_get("text", target) || target.empty()) {
			size_t pos = cch.find("/unbanip");
			if (pos != std::string::npos) {
				target = trim_copy(cch.substr(pos + 8));
			}
		}

		target = trim_copy(target);

		if (target.empty()) {
			VarList::OnConsoleMessage(peer, "`4Usage: `/unbanip <IP/GrowID>");
			return;
		}
		if (is_valid_ip_format(target)) {
			auto& bannedIps = Environment()->Rid_Bans;
			auto it = std::find(bannedIps.begin(), bannedIps.end(), target);

			if (it == bannedIps.end()) {
				VarList::OnConsoleMessage(peer, "`4IP `w" + target + "`4 is not in ban list.");
				return;
			}

			bannedIps.erase(it);

			VarList::OnConsoleMessage(peer, "`oSuccessfully removed IP ban for `w" + target + "``.");
			Logger::Info("ADMIN", pInfo(peer)->tankIDName + " unbanned IP: " + target);
			return;
		}
		const std::string growid = target;
		const std::string path = "database/players/" + growid + "_.json";

		if (!std::filesystem::exists(path)) {
			VarList::OnConsoleMessage(peer, "`4GrowID `w" + growid + "`4 was not found.");
			return;
		}

		try {
			json data;
			{
				std::ifstream in(path, std::ios::binary);
				if (!in.is_open()) {
					VarList::OnConsoleMessage(peer, "`4Failed to open player data.");
					return;
				}
				in >> data;
			}

			data["b_t"] = 0;
			data["b_s"] = 0;
			data["b_r"] = "";
			data["b_b"] = "";
			if (data.contains("ban_seconds")) data["ban_seconds"] = 0;
			if (data.contains("ban_reason")) data["ban_reason"] = "";

			{
				std::ofstream out(path, std::ios::binary | std::ios::trunc);
				if (!out.is_open()) {
					VarList::OnConsoleMessage(peer, "`4Failed to save player data.");
					return;
				}
				out << data.dump(4);
			}
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
				if (to_lower(pInfo(cp_)->tankIDName) != to_lower(growid)) continue;

				pInfo(cp_)->b_t = 0;
				pInfo(cp_)->b_s = 0;
				pInfo(cp_)->b_r.clear();
				pInfo(cp_)->b_b.clear();
				pInfo(cp_)->ban_seconds = 0;
			}

			VarList::OnConsoleMessage(peer, "`oSuccessfully removed ban for GrowID `w" + growid + "``.");
			Logger::Info("ADMIN", pInfo(peer)->tankIDName + " unbanned GrowID: " + growid);
		}
		catch (const std::exception& e) {
			VarList::OnConsoleMessage(peer, "`4Failed to unban target.");
			Logger::Info("ERROR", std::string("unbanip exception: ") + e.what());
		}
		catch (...) {
			VarList::OnConsoleMessage(peer, "`4Failed to unban target.");
			Logger::Info("ERROR", "unbanip unknown exception");
		}
	}
	static void CMD_INFINITE(ENetPeer* peer, const std::string& param, int level) {
		if (!peer || !peer->data) return;

		Player* pl = pInfo(peer);
		if (!pl) return;

		pl->infinite_mode = !pl->infinite_mode;

		if (pl->infinite_mode) {
			VarList::OnConsoleMessage(peer, "`2[INFINITE] `oMode enabled. Dropping / placing / consuming items will no longer reduce inventory.");
			VarList::OnTalkBubble(peer, pl->netID, "`2Infinite mode enabled!");
		}
		else {
			VarList::OnConsoleMessage(peer, "`4[INFINITE] `oMode disabled. Inventory will be reduced normally.");
			VarList::OnTalkBubble(peer, pl->netID, "`4Infinite mode disabled!");
		}
	}
	static void CMD_PARTY_START(ENetPeer* peer, const std::string& param, int level) {
		if (!peer || !peer->data) return;
		PartyEvent::Start(peer);
	}

	static void CMD_PARTY_STOP(ENetPeer* peer, const std::string& param, int level) {
		if (!peer || !peer->data) return;
		PartyEvent::Stop(peer);
	}
	static void Commands_spotify2(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, dialog_build());
	}
	static void CMD_EXCHANGENEW(ENetPeer* peer, std::string param, int level) {
		int page = 1;
		if (!param.empty() && is_number(param)) page = atoi(param.c_str());
		Commands::Commands_exchangenew(peer, page);
	}

	static void CMD_SETEXCHANGENEW(ENetPeer* peer, std::string param, int level) {
		int page = 1;
		if (!param.empty() && is_number(param)) page = atoi(param.c_str());
		Commands::Commands_setexchangenew(peer, page);
	}
	static void Commands_exchangenew(ENetPeer* peer, int page = 1) {
		{
			std::lock_guard<std::mutex> lk(g_exchNewStateMtx);
			g_exchNewState[peer].page = page < 1 ? 1 : page;
		}
		VarList::OnDialogRequest(peer, exn_exchange_dialog(peer, page));
	}

	static void Commands_setexchangenew(ENetPeer* peer, int page = 1) {
		ExchNewEditState st;
		{
			std::lock_guard<std::mutex> lk(g_exchNewStateMtx);
			st = g_exchNewState[peer];
			st.page = page < 1 ? 1 : page;
			g_exchNewState[peer] = st;
		}
		VarList::OnDialogRequest(peer, exn_set_dialog(peer, st));
	}
	static void CMD_SETCONSUM(ENetPeer* peer, const std::string& param, int level) {
		if (!Role::has_config_access(peer)) return;

		std::string s = param;

		// trim kiri
		while (!s.empty() && (s[0] == ' ' || s[0] == '\t')) s.erase(s.begin());
		// trim kanan
		while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();

		// kadang dispatcher ngirim "/setconsum 958" ke param, kita amankan
		if (s.rfind("/setconsum", 0) == 0) {
			s.erase(0, 9);
			while (!s.empty() && (s[0] == ' ' || s[0] == '\t')) s.erase(s.begin());
		}

		if (s.empty()) {
			VarList::OnConsoleMessage(peer, "Usage: /setconsum <id>");
			return;
		}

		// parse aman tanpa exception
		const char* c = s.c_str();
		char* endp = 0;
		long v = strtol(c, &endp, 10);

		// skip trailing spaces
		while (endp && (*endp == ' ' || *endp == '\t')) endp++;

		if (endp == c || (endp && *endp != '\0')) {
			VarList::OnConsoleMessage(peer, "Usage: /setconsum <id>");
			return;
		}

		int id = (int)v;
		if (id <= 0 || id >= (int)items.size()) {
			VarList::OnConsoleMessage(peer, "Invalid item id");
			return;
		}
		if (items[id].blockType != BlockTypes::CONSUMABLE) {
			VarList::OnConsoleMessage(peer, "Item is not consumable");
			return;
		}

		SetConsum_GetOrCreate(id);
		SetConsum_Save();

		VarList::OnDialogRequest(peer, SetConsum_BuildMainDialog(peer, id));
	}
	static void SAWIT(ENetPeer* peer) {
		if (peer->state != ENET_PEER_STATE_CONNECTED) return;
		if (peer->data == nullptr) return;

		int tree = pInfo(peer)->sawit_tree;
		int fruit = pInfo(peer)->sawit_fruit;

		// clamp aman (kalau file lama/korup)
		if (tree < 0) tree = 0;
		if (fruit < 0) fruit = 0;
		if (tree > 999) tree = 999;
		if (fruit > 999) fruit = 999;
		pInfo(peer)->sawit_tree = tree;
		pInfo(peer)->sawit_fruit = fruit;

		std::string d = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		d += "add_label_with_icon|big|Want to Citem Mr. Prabowo?|left|19676|\n";
		d += "add_spacer|small|\n";
		d += "text_scaling_string|Subscribtions++++++++|\n";
		d += "add_smalltext|Collect 999 palm trees and palm fruit, enter palm fruit and palm trees via this button|\n";
		d += "add_button|add_sawit|Add Sawit|\n";

		d += "add_spacer|small|\n";
		d += "add_smalltext|List Your Palm Tress:|\n";
		d += "add_button_with_icon|sawit_tree_show|" + std::to_string(tree) + "||854|\n";

		d += "add_custom_break|\nadd_spacer|small|\n";
		d += "add_smalltext|List Your Palm Fruit|\n";
		d += "add_button_with_icon|sawit_fruit_show|" + std::to_string(fruit) + "||1328|\n";
		d += "add_custom_break|\n";
		d += "add_spacer|small|\n";
		d += "add_smalltext|If you have collected 999x palm trees and palm fruit you will get Mr Prabowo's item|\n";
		d += "end_dialog|sawit_main|Close|\n";
		VarList::OnDialogRequest(peer, d);
	}
	static void REMOVE_DROP_ALL(ENetPeer* peer) {
		if (pInfo(peer)->world == "GROWCH" and not Role::Clist(pInfo(peer)->tankIDName)) return;

		int removed_loaded = 0;
		int removed_files = 0;

		PlayerMoving data_{};
		data_.packetType = 14;
		data_.netID = 0;

		for (int w = 0; w < (int)worlds.size(); ++w) {
			World* world_ = &worlds[w];
			if (!world_ || world_->drop_new.empty()) continue;

			const int drop_count = (int)world_->drop_new.size();

			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
				if (pInfo(cp_)->world != world_->name) continue;

				for (int i_ = 0; i_ < drop_count; ++i_) {
					data_.plantingTree = (int)world_->drop_new[i_][2];
					BYTE* raw = packPlayerMoving(&data_);
					send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
					delete[] raw;
				}
			}

			removed_loaded += drop_count;
			world_->drop_new.clear();
			world_->fresh_world = true;
			save_world(world_->name, false);
		}

		try {
			const std::string folder = "database/worlds/";
			if (std::filesystem::exists(folder)) {
				for (auto& entry : std::filesystem::directory_iterator(folder)) {
					if (!entry.is_regular_file()) continue;

					const std::string path = entry.path().string();
					if (path.size() < 6) continue;
					if (path.find("_.json") == std::string::npos) continue;

					json j;
					std::ifstream in(path, std::ifstream::binary);
					if (!in.is_open()) continue;

					try {
						in >> j;
					}
					catch (...) {
						in.close();
						continue;
					}
					in.close();

					if (j.find("drop_new") == j.end()) continue;
					if (!j["drop_new"].is_array()) continue;

					int cnt = (int)j["drop_new"].size();
					if (cnt <= 0) continue;

					j["drop_new"] = json::array();
					if (j.find("total_drop_uid") != j.end()) j["total_drop_uid"] = 0;

					std::ofstream out(path, std::ofstream::trunc);
					if (!out.is_open()) continue;
					out << j << std::endl;
					out.close();

					removed_files += cnt;
				}
			}
		}
		catch (...) {
		}

		VarList::OnConsoleMessage(peer,
			">> Removed all dropped items (ALL WORLDS). Loaded removed: " + std::to_string(removed_loaded) +
			" | File removed: " + std::to_string(removed_files) + ".");
	}
	static void STATSFISH(ENetPeer* peer) {
		Player* pl = pInfo(peer);
		float mult = pl->fish_speed_mult;
		if (mult < 1.0f) mult = 1.0f;
		if (mult > 3.0f) mult = 3.0f;

		float next = mult;
		if (mult < 3.0f) {
			next = mult + 0.2f;
			if (next > 3.0f) next = 3.0f;
		}

		std::ostringstream ss_mult;
		ss_mult.setf(std::ios::fixed);
		ss_mult << std::setprecision(1) << mult;

		std::ostringstream ss_next;
		ss_next.setf(std::ios::fixed);
		ss_next << std::setprecision(1) << next;

		int cost = pl->fish_upgrade_cost_bgl;
		if (cost < 50) cost = 50;

		std::string d = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		d += "add_label_with_icon|big|`oFishing Speed``|left|9472|\n";
		d += "add_spacer|\n";
		d += "add_smalltext|Current speed: `w" + ss_mult.str() + "x``|\n";
		d += "add_smalltext|Next speed: `w" + ss_next.str() + "x``|\n";
		d += "add_smalltext|Upgrade cost: `w" + FormatCostBGL(cost) + "``|\n";
		d += "add_button|upgrade|`8Upgrade!!``|0|0|\n";
		if (mult >= 3.0f) {
			d += "add_smalltext|`4Max speed reached.``|\n";
			d += "end_dialog|statsfish|Close||\n";
		}
		else {
			d += "end_dialog|statsfish|Close|Upgrade|\n";
		}

		VarList::OnDialogRequest(peer, d);
	}
	static void USTADZ(ENetPeer* peer) {
		if (!peer || !peer->data) return;

		// Set ustadz
		// Chef's Cap 4502
		// Slick Black Hair 1168
		// Presidential Bread 10256
		// Polo Shirt - Blue 8592
		// Blue Skirt 1860
		const int set_ids[] = { 4502, 1168, 10256, 8592, 1860 };

		// Track already-used slots to avoid conflicts (e.g. 2x HAIR)
		bool used_hair = false, used_shirt = false, used_pants = false, used_feet = false, used_face = false,
			used_hand = false, used_back = false, used_mask = false, used_neck = false, used_ances = false;

		int equipped = 0;
		std::string skipped;

		for (int i = 0; i < (int)(sizeof(set_ids) / sizeof(set_ids[0])); ++i) {
			const int id = set_ids[i];
			if (id <= 0 || id >= (int)items.size()) {
				skipped += "`4- Invalid item id: `w" + std::to_string(id) + "``\n";
				continue;
			}

			if (items[id].blockType != BlockTypes::CLOTHING) {
				skipped += "`4- Not clothing: `w" + std::to_string(id) + " (" + items[id].ori_name + ")``\n";
				continue;
			}

			// clothType routing (following the switch used in Inventory::Modify)
			switch (items[id].clothType) {
			case ClothTypes::HAIR:
				if (used_hair) { skipped += "`4- Slot HAIR already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->hair = (std::uint16_t)id; used_hair = true; equipped++; break;

			case ClothTypes::SHIRT:
				if (used_shirt) { skipped += "`4- Slot SHIRT already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->shirt = (std::uint16_t)id; used_shirt = true; equipped++; break;

			case ClothTypes::PANTS:
				if (used_pants) { skipped += "`4- Slot PANTS already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->pants = (std::uint16_t)id; used_pants = true; equipped++; break;

			case ClothTypes::FEET:
				if (used_feet) { skipped += "`4- Slot FEET already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->feet = (std::uint16_t)id; used_feet = true; equipped++; break;

			case ClothTypes::FACE:
				if (used_face) { skipped += "`4- Slot FACE already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->face = (std::uint16_t)id; used_face = true; equipped++; break;

			case ClothTypes::HAND:
				if (used_hand) { skipped += "`4- Slot HAND already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->hand = (std::uint16_t)id; used_hand = true; equipped++; break;

			case ClothTypes::BACK:
				if (used_back) { skipped += "`4- Slot BACK already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->back = (std::uint16_t)id; used_back = true; equipped++; break;
			case ClothTypes::MASK:
				if (used_mask) { skipped += "`4- Slot MASK already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->mask = (std::uint16_t)id; used_mask = true; equipped++; break;

			case ClothTypes::NECKLACE:
				if (used_neck) { skipped += "`4- Slot NECKLACE already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->necklace = (std::uint16_t)id; used_neck = true; equipped++; break;

			case ClothTypes::ANCES:
				if (used_ances) { skipped += "`4- Slot ANCES already used, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n"; break; }
				pInfo(peer)->ances = (std::uint16_t)id; used_ances = true; equipped++; break;

			default:
				skipped += "`4- Unsupported clothType, skipped: `w" + items[id].ori_name + " (" + std::to_string(id) + ")``\n";
				break;
			}
		}

		// Apply visuals + stats
		Clothing_V2::Update_Value(peer);
		Clothing_V2::Update(peer, false, true);       // force OnSetClothing
		Clothing_V2::Update_Value(peer, true);        // kalau ada dependensi guild-mode

		CAction::Positioned(peer, pInfo(peer)->netID, "audio/change_clothes.wav", 0);

		VarList::OnConsoleMessage(peer, "`2/ustadz applied. Equipped: `w" + std::to_string(equipped) + "`2 item(s).``");
		if (!skipped.empty()) {
			VarList::OnConsoleMessage(peer, "`oSkipped:\n" + skipped);
		}
	}
	static void CMD_STARTRAMADHAN(ENetPeer* peer, const std::string& cmd, int level) {
		if (!pInfo(peer)->Role.Owner_Server) return;
		SaveRamadhanActive(true);
		VarList::OnConsoleMessage(peer, "`2[Ramadhan] `oRamadhan has been started.");
	}

	static void CMD_RAMADHAN(ENetPeer* peer, const std::string& cmd, int level) {
		LoadRamadhanActive();
		if (!g_ramadhan_active) {
			VarList::OnConsoleMessage(peer, "`4[Ramadhan] `oRamadhan is not active yet.");
			return;
		}
		int current_day_id = RamadhanDayId();
		bool is_puasa_done = (pInfo(peer)->last_puasa_day == current_day_id);
		bool is_teraweh_done = (pInfo(peer)->last_teraweh_day == current_day_id);
		std::string d = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		d += "add_label_with_icon|big|`2Ramadhan Tracker|left|1366|\n";
		d += "text_scaling_string|Subscribtions++++++++|\n";
		d += "add_spacer|small|\n";
		d += "add_label_with_icon|small|`wTarawih Streak (Day " + std::to_string(pInfo(peer)->teraweh_streak) + ")|left|6292|\n";
		if (is_teraweh_done) d += "add_button_with_icon||`2Stamped!|staticBlueFrame,no_padding_x|6292|0|\n";
		else d += "add_button_with_icon||`4Not Stamped|staticGreyFrame,no_padding_x|0|0|\n";
		d += "add_spacer|small|\n";
		d += "add_custom_break|\n";
		d += "add_label_with_icon|small|`wPuasa Streak (Day " + std::to_string(pInfo(peer)->puasa_streak) + ")|left|6292|\n";
		if (is_puasa_done) d += "add_button_with_icon||`2Checked In!|staticBlueFrame,no_padding_x|6292|0|\n";
		else d += "add_button_with_icon|claim_puasa|`2I'm Fasting|staticYellowFrame,no_padding_x|0|0|\n";
		d += "add_spacer|small|\n";
		d += "add_textbox|`oRewards:\n- `wPuasa Day 30`: 3x Aku Berlutut Box + 20x mgl\n- `wTarawih Day 10`: 10x Aku Berlutut Box + 50x mgl|left|\n";
		d += "add_quick_exit|\n";
		d += "end_dialog|ramadhan_view|||\n";
		VarList::OnDialogRequest(peer, d);
	}
	static void CMD_TERAWEH(ENetPeer* peer, const std::string& param, int level) {
		LoadRamadhanActive();
		if (!g_ramadhan_active) {
			VarList::OnConsoleMessage(peer, "`4[Ramadhan] `oRamadhan is not active yet.");
			return;
		}
		if (!pInfo(peer)->Role.Owner_Server) return;
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "Usage: /teraweh <growid>");
			return;
		}
		ENetPeer* targetPeer = nullptr;
		for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
			if (p->state != ENET_PEER_STATE_CONNECTED || p->data == NULL) continue;
			if (to_lower(pInfo(p)->tankIDName) == to_lower(param)) {
				targetPeer = p;
				break;
			}
		}
		if (!targetPeer) {
			VarList::OnConsoleMessage(peer, "`4Player not found or offline.");
			return;
		}
		int current_day_id = RamadhanDayId();
		if (pInfo(targetPeer)->last_teraweh_day == current_day_id) {
			VarList::OnConsoleMessage(peer, "`4Player already received Tarawih stamp today.");
			return;
		}
		pInfo(targetPeer)->last_teraweh_day = current_day_id;
		pInfo(targetPeer)->teraweh_streak++;
		VarList::OnConsoleMessage(targetPeer, "`2[Ramadhan] `oYou received a Tarawih Stamp from Owner! Streak: " + std::to_string(pInfo(targetPeer)->teraweh_streak));
		VarList::OnConsoleMessage(peer, "`2Success given Tarawih stamp to " + pInfo(targetPeer)->tankIDName);
		VarList::OnPlaySound(targetPeer, "audio/secret.wav", 0);
		if (pInfo(targetPeer)->teraweh_streak == 10) {
			int c1 = 10;
			int c2 = 50;
			Inventory::Modify(targetPeer, 15874, c1, true);
			Inventory::Modify(targetPeer, 8470, c2, true);
			VarList::OnConsoleMessage(targetPeer, "`2[Ramadhan] `oCongratulations on 10 Tarawih! You received 10x Aku Berlutut Box and 50x mgl!");
			VarList::OnPlaySound(targetPeer, "audio/party_horn.wav", 0);
		}
	}
	static void CMD_EDITDROP(ENetPeer* peer, const std::string& cmd) {
		VarList::OnDialogRequest(peer, DropEditV1_BuildMainDialog(peer));
	}
	static void DL_PLANT(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`$Plant Your Seed!!``|left|7188|\nadd_spacer|small|\nadd_button_with_icon|additempicker|Pick Seed Your Seed|staticYellowFrame|2|0|\nadd_button|plant|Plant|left|\nadd_quick_exit|\nend_dialog|plantSeed|Nevermind||");
	}

	static void DL_HARVEST(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`$Harvest All Tree``|left|7188|\nadd_spacer|small|\nadd_button|harvest|Harvest!!|left|\nadd_quick_exit|\nend_dialog|harvestAll|Nevermind||");
	}
	static void command_e(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, ex_dialog_build(peer));
	}
	static void cmd_pair(ENetPeer* peer, const std::string& param) {
		if (!param.empty()) return;

		std::string status = "Not Paired";
		if (pInfo(peer)->is_paired) status = "Paired with " + pInfo(peer)->partner_name;

		std::string q_info = "None";
		if (pInfo(peer)->is_paired) {
			std::string q_type = "None";
			if (pInfo(peer)->pair_quest_type == 1) q_type = "Smash Blocks";
			if (pInfo(peer)->pair_quest_type == 2) q_type = "Plant Seeds";
			if (pInfo(peer)->pair_quest_type == 3) q_type = "Collect Gems";
			if (pInfo(peer)->pair_quest_type == 4) q_type = "Surgery";
			q_info = q_type + " (" + std::to_string(pInfo(peer)->pair_quest_progress) + "/" + std::to_string(pInfo(peer)->pair_quest_goal) + ")";
		}

		std::string dlg = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wPair System``|left|13158|\nadd_spacer|small|\n";
		dlg += "add_textbox|Status: " + status + "|left|\n";
		dlg += "add_textbox|Charisma: " + std::to_string(pInfo(peer)->pair_charisma) + "|left|\n";
		dlg += "add_textbox|Pairs Finished: " + std::to_string(pInfo(peer)->pair_count) + "|left|\n";
		dlg += "add_textbox|Current Quest: " + q_info + "|left|\nadd_spacer|small|\n";

		if (!pInfo(peer)->is_paired) {
			dlg += "add_text_input|pair_target|Target Name||20|\n";
			dlg += "add_button|pair_invite|Invite|noflags|0|0|\n";
		}
		else {
			dlg += "add_button|pair_divorce|Divorce|noflags|0|0|\n";
		}

		dlg += "add_quick_exit|\nend_dialog|pair_menu|||\n";
		VarList::OnDialogRequest(peer, dlg);
	}
	static void cmd_leaderboardpair(ENetPeer* peer, const std::string& param) {
		std::string dlg = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wPair Leaderboard``|left|13158|\nadd_spacer|small|\n";
		std::ifstream i("database/json/pair_leaderboard.json");
		if (i.good()) {
			json j;
			i >> j;
			int rank = 1;
			for (const auto& entry : j) {
				dlg += "add_label|small|`9" + std::to_string(rank) + ". `2" + entry["p1"].get<std::string>() + " `o& `2" + entry["p2"].get<std::string>() + " `o|left|\n";
				dlg += "add_smalltext|Charisma: " + std::to_string(entry["charisma"].get<int>()) + " | Finished: " + std::to_string(entry["count"].get<int>()) + "|left|\n";
				rank++;
			}
		}
		else {
			dlg += "add_textbox|No pairs yet.|left|\n";
		}
		dlg += "add_quick_exit|\nend_dialog|lb_pair|||\n";
		VarList::OnDialogRequest(peer, dlg);
	}
	static inline std::string BuildRecipeDialog(ENetPeer* peer, int page) {
		LoadSetRecipeJson(); // always sync from json

		if (page < 0) page = 0;

		const int per_page = 10;
		const int total = (int)receptai.size();
		const int max_page = (total <= 0) ? 0 : (total - 1) / per_page;
		if (page > max_page) page = max_page;

		int start = page * per_page;
		int end = start + per_page;
		if (end > total) end = total;

		std::string dlg = SetColor(peer);
		dlg += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n";
		dlg += "add_label_with_icon|big|`wRECIPE BOOK|left|9474|\n";
		dlg += "add_spacer|small|\n";

		dlg += "add_textbox|`oRecipes: `w" + std::to_string(total) +
			" `o(Page `w" + std::to_string(page + 1) + "`o/`w" + std::to_string(max_page + 1) + "`o)|left|\n";
		dlg += "add_spacer|small|\n";

		// embed current page
		dlg += "embed_data|rp_page|" + std::to_string(page) + "\n";

		if (total <= 0) {
			dlg += "add_textbox|`4No recipes found.|left|\n";
			dlg += "add_spacer|small|\n";
			dlg += "add_custom_button|rp_close|textLabel:`wClose;middle_colour:80543231;border_colour:80543231;|\n";
			dlg += "end_dialog|recipe_view|||\n";
			return dlg;
		}
		for (int i = start; i < end; i++) {
			auto& r = receptai[i];
			if ((int)r.size() < 4) continue;

			int ing1_id = r[0][0], ing1_ct = r[0][1];
			int ing2_id = r[1][0], ing2_ct = r[1][1];
			int ing3_id = r[2][0], ing3_ct = r[2][1];
			int res_id = r[3][0], res_ct = r[3][1];
			if (ing1_id <= 0 || ing1_id >= (int)items.size()) continue;
			if (ing2_id <= 0 || ing2_id >= (int)items.size()) continue;
			if (ing3_id <= 0 || ing3_id >= (int)items.size()) continue;
			if (res_id <= 0 || res_id >= (int)items.size()) continue;
			dlg += "add_button_with_icon||" + items[ing1_id].name + "|staticGreyFrame|" +
				std::to_string(ing1_id) + "|" + std::to_string(ing1_ct) + "\n";
			dlg += "add_custom_margin|x:10;y:0;|\n";

			dlg += "add_button_with_icon||" + items[ing2_id].name + "|staticGreyFrame|" +
				std::to_string(ing2_id) + "|" + std::to_string(ing2_ct) + "\n";
			dlg += "add_custom_margin|x:10;y:0;|\n";

			dlg += "add_button_with_icon||" + items[ing3_id].name + "|staticGreyFrame|" +
				std::to_string(ing3_id) + "|" + std::to_string(ing3_ct) + "\n";
			dlg += "add_custom_margin|x:10;y:0;|\n";
			dlg += "add_button_with_icon|||noflags|482||\n";
			dlg += "add_custom_margin|x:10;y:0;|\n";
			dlg += "add_button_with_icon||" + items[res_id].name + "|staticYellowFrame|" +
				std::to_string(res_id) + "|" + std::to_string(res_ct) + "\n";
			dlg += "reset_placement_x|\n";
			dlg += "add_custom_break|\n";
			dlg += "add_spacer|small|\n";
		}
		dlg += "add_spacer|big|\n";
		dlg += "add_custom_break|\n";
		dlg += "add_custom_button|rp_prev|textLabel:`wSearch;middle_colour:80543231;border_colour:80543231;|\n";
		if (page > 0) {
			dlg += "add_custom_button|rp_prev|textLabel:`wBack Page <<<;middle_colour:80543231;border_colour:80543231;|\n";
		}
		if (page < max_page) {
			dlg += "add_custom_button|rp_next|textLabel:`wNext Page >>>;anchor:_button_rp_prev;left:1;margin:60,0;middle_colour:80543231;border_colour:80543231;|\n";
		}
		dlg += "add_custom_button|rp_close|textLabel:`wClose;middle_colour:80543231;border_colour:80543231;|\n";
		dlg += "end_dialog|recipe_view|||\n";

		return dlg;
	}

	static void cmd_setrecipe(ENetPeer* peer, const std::string& param) {
		if (!peer || peer->data == NULL) return;
		if (!Role::Clist(pInfo(peer)->tankIDName)) return;
		LoadSetRecipeJson();

		std::string dlg = SetColor(peer);
		dlg += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n";
		dlg += "add_label_with_icon|big|`wSET RECIPE|left|9474|\n";
		dlg += "add_spacer|small|\n";
		dlg += "add_textbox|`oTotal recipe: `w" + std::to_string((int)receptai.size()) + "|left|\n";
		dlg += "add_spacer|small|\n";

		for (int i = 0; i < (int)receptai.size(); i++) {
			int res_id = 0, res_ct = 0;

			if ((int)receptai[i].size() >= 4 && (int)receptai[i][3].size() >= 2) {
				res_id = receptai[i][3][0];
				res_ct = receptai[i][3][1];
			}

			if (res_id <= 0 || res_id >= (int)items.size()) continue;

			dlg += "add_button_with_icon|sr_edit_" + std::to_string(i) +
				"|`w" + items[res_id].name + " `2x" + std::to_string(res_ct) +
				"|left|" + std::to_string(res_id) + "|\n";
		}

		dlg += "add_spacer|big|\n";
		dlg += "add_custom_break|\n";
		dlg += "add_custom_button|sr_add|textLabel:`w+ Add Recipe;middle_colour:80543231;border_colour:80543231;|\n";
		dlg += "add_custom_button|sr_close|textLabel:`wClose;anchor:_button_sr_add;left:1;margin:60,0;middle_colour:80543231;border_colour:80543231;|\n";
		dlg += "end_dialog|setrecipe_main|||\n";

		VarList::OnDialogRequest(peer, dlg);
	}
	static void AUTOSELLFISH(ENetPeer* peer, const std::string& param) {
		if (!peer || !peer->data) return;

		std::string p = param;
		std::transform(p.begin(), p.end(), p.begin(), ::tolower);

		if (p == "on" || p == "1" || p == "true") {
			pInfo(peer)->autosellfish = true;
			VarList::OnConsoleMessage(peer, "`2[AutoSellFish] ON``");
			Inventory::ProcessAutoSellFishInventory(peer, true, true);
			return;
		}

		if (p == "off" || p == "0" || p == "false") {
			pInfo(peer)->autosellfish = false;
			VarList::OnConsoleMessage(peer, "`4[AutoSellFish] OFF``");
			return;
		}

		VarList::OnConsoleMessage(peer, "`oUsage: `w/autosellfish on `oor `w/autosellfish off``");
	}
	static void ADD_P_EFFECT_CITEM(ENetPeer* peer, const std::string& param) {
		if (not Role::Clist(pInfo(peer)->tankIDName)) return;

		auto is_num = [](const std::string& s) -> bool {
			if (s.empty()) return false;
			for (char c : s) if (!std::isdigit((unsigned char)c)) return false;
			return true;
			};

		std::string punch_s = param;
		if (punch_s.empty() || !is_num(punch_s)) return;
		int punch_id = std::atoi(punch_s.c_str());
		if (punch_id < 0) return;

		for (int item_id = 14001; item_id < (int)items.size(); item_id++) {
			if (items[item_id].blockType != BlockTypes::CLOTHING) continue;
			if (items[item_id].clothType != ClothTypes::FEET) continue;

			auto it = std::find_if(EditItem.begin(), EditItem.end(), [item_id](const Edit_ItemV2& v) {
				return v.ID == item_id;
				});

			if (it == EditItem.end()) {
				Edit_ItemV2 rev;
				rev.ID = item_id;
				rev.Name = items[item_id].name;
				rev.Desc = items[item_id].description;
				rev.rarity = items[item_id].rarity;
				rev.Punch_Id = punch_id;
				rev.Far_Punch = 0;
				rev.Punch_Hit = 0;
				rev.Punch_Place = 0;
				rev.Gems = 0;
				rev.Xp = 0;
				rev.Bonus = 0;
				rev.Item_Price = 0;
				rev.property_untradeable = false;
				rev.property_blacklist = false;
				rev.property_gacha = false;
				rev.property_unobtainable = true;
				rev.property_blocked = false;
				rev.property_farmable = false;
				EditItem.push_back(rev);
			}
			else {
				it->Punch_Id = punch_id;
				it->property_unobtainable = true;
			}
		}

		{
			std::ofstream over_write("database/json/edit_itemv2.json");
			json j;
			json rev_ = json::array();
			for (int i_2 = 0; i_2 < (int)EditItem.size(); i_2++) {
				json it_;
				it_["ID"] = EditItem[i_2].ID;
				it_["Name"] = EditItem[i_2].Name;
				it_["Desc"] = EditItem[i_2].Desc;
				it_["rarity"] = EditItem[i_2].rarity;
				it_["Change_Drop_Seeds"] = EditItem[i_2].Change_Drop_Seeds;
				it_["Punch_Id"] = EditItem[i_2].Punch_Id;
				it_["Far_Punch"] = EditItem[i_2].Far_Punch;
				it_["Punch_Hit"] = EditItem[i_2].Punch_Hit;
				it_["Punch_Place"] = EditItem[i_2].Punch_Place;
				it_["Gems"] = EditItem[i_2].Gems;
				it_["Xp"] = EditItem[i_2].Xp;
				it_["Bonus"] = EditItem[i_2].Bonus;
				it_["Item_Price"] = EditItem[i_2].Item_Price;
				it_["Extra_Drops"] = EditItem[i_2].Extra_Drops;
				it_["property_untradeable"] = EditItem[i_2].property_untradeable;
				it_["property_blacklist"] = EditItem[i_2].property_blacklist;
				it_["property_gacha"] = EditItem[i_2].property_gacha;
				it_["property_unobtainable"] = EditItem[i_2].property_unobtainable;
				it_["property_blocked"] = EditItem[i_2].property_blocked;
				it_["property_farmable"] = EditItem[i_2].property_farmable;
				rev_.push_back(it_);
			}
			j["items"] = rev_;
			over_write << j.dump(4) << endl;
			over_write.close();
		}

		for (int i_ = 0; i_ < (int)items.size(); i_++) {
			int item_id = items[i_].id;
			for (int i = 0; i < (int)EditItem.size(); i++) {
				if (EditItem[i].ID == item_id) {
					items[EditItem[i].ID].name = EditItem[i].Name;
					items[EditItem[i].ID].ori_name = EditItem[i].Name;
					items[EditItem[i].ID].description = EditItem[i].Desc;
					items[EditItem[i].ID].rarity = EditItem[i].rarity;
					items[EditItem[i].ID].newdropchance = EditItem[i].Change_Drop_Seeds;
					items[EditItem[i].ID].max_gems = EditItem[i].Gems;
					items[EditItem[i].ID].untradeable = (EditItem[i].property_untradeable ? 1 : 0);
					items[EditItem[i].ID].unobtainable = (EditItem[i].property_unobtainable ? 1 : 0);
					items[EditItem[i].ID].blocked_place = (EditItem[i].property_blocked ? 1 : 0);
					items[EditItem[i].ID].farmable = (EditItem[i].property_farmable ? true : false);
					if (EditItem[i].property_blacklist) {
						if (not Gtps_Shop::Disable_Items(EditItem[i].ID)) Disable_Item.id.push_back(EditItem[i].ID);
					}
					else {
						if (Gtps_Shop::Disable_Items(EditItem[i].ID)) Disable_Item.id.erase(std::remove(Disable_Item.id.begin(), Disable_Item.id.end(), EditItem[i].ID), Disable_Item.id.end());
					}
					if (Gtps_Shop::Get_Price(EditItem[i].ID) == 0) Environment()->buy_item_list.push_back(std::make_pair(EditItem[i].ID, EditItem[i].ID));
					Environment()->shop_data[EditItem[i].ID] = EditItem[i].Item_Price;
				}
			}
		}

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			Clothing_V2::Update_Value(cp_);
			Clothing_V2::Update(cp_, true);
		}
	}

	static void ConfigDialog(ENetPeer* peer) {
		if (!Role::has_config_access(peer)) {
			VarList::OnConsoleMessage(peer, "`oAccess denied, please enter password to access these commands usage / giveaccfull <password>.`");
			return;
		}
		if (!Role::Clist(pInfo(peer)->tankIDName)) return;

		int TotalPlayer = TotalJson("database/players/");
		int TotalWorld = TotalJson("database/worlds/");
		int TotalGuild = TotalJson("database/guilds/");
		int total_online = 0, total_pc = 0, total_ios = 0, total_android = 0;

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			total_online += 1;
			if (pInfo(cp_)->player_device == "0,1,1") total_pc += 1;
			else if (pInfo(cp_)->player_device == "1") total_ios += 1;
			else total_android += 1;
		}

		std::string dialog;
		dialog += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|Server Configuration|left|32|\n";
		dialog += "add_smalltext|`oThis panel lets you manage core server and game settings that are stored in your config.json file.|left|\n";
		dialog += "embed_data|action|apply_config|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_label_with_icon|small|Server Information|left|32|\n";
		dialog += "add_smalltext|Total Accounts: " + std::to_string(TotalPlayer) + "  |left|\n";
		dialog += "add_smalltext|Total Worlds: " + std::to_string(TotalWorld) + "  |left|\n";
		dialog += "add_smalltext|Total Guilds: " + std::to_string(TotalGuild) + "  |left|\n";
		dialog += "add_smalltext|Online: " + std::to_string(total_online) + " (`oPC: " + std::to_string(total_pc) + "  iOS: " + std::to_string(total_ios) + "  Android: " + std::to_string(total_android) + "``) |left|\n";
		dialog += "add_small_font_button|server_information|Server Statistic|noflags|0|0|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_label_with_icon|small|Game Protection|left|32|\n";
		dialog += "add_checkbox|anti_gl|Anti-Growlauncher|";
		dialog += (Environment()->Anti_GL ? "1" : "0");
		dialog += "|\n";
		dialog += "add_checkbox|anti_proxy|Anti-Proxy|";
		dialog += (Environment()->Anti_Proxy ? "1" : "0");
		dialog += "|\n";
		dialog += "add_checkbox|joystick|Player Joystick|";
		dialog += (Environment()->Enable_Joystick ? "1" : "0");
		dialog += "|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_label_with_icon|small|Server Settings (SERVER)|left|32|\n";
		dialog += "add_smalltext|Server Name (NAME)|left|\n";
		dialog += "add_text_input|servername||" + Environment()->server_name + "|20|\n";
		dialog += "add_smalltext|Server Port (PORT)|left|\n";
		dialog += "add_text_input|server_port||" + std::to_string(Environment()->Server_Port) + "|5|\n";
		dialog += "add_smalltext|Protocol (PROTOCOL)|left|\n";
		dialog += "add_text_input|server_proto||" + Environment()->proto + "|10|\n";
		dialog += "add_smalltext|Client Version (CLIENT_VERSION)|left|\n";
		dialog += "add_text_input|client_version||" + Environment()->GT_Version + "|10|\n";
		dialog += "add_smalltext|Server Version (SERVER_VERSION)|left|\n";
		dialog += "add_text_input|server_version||" + Environment()->server_version + "|10|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_label_with_icon|small|Game Settings (GAME)|left|32|\n";
		dialog += "add_smalltext|Autofarm Delay ms (AUTOFARM_DELAY, min 100ms)|left|\n";
		dialog += "add_text_input|autofarm_delay||" + std::to_string(Environment()->AutoFarm_Delay) + "|4|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_smalltext|World Foreground ID (WORLD_FG)|left|\n";
		dialog += "add_text_input|world_fg||" + std::to_string(WorldDefault.fg) + "|5|\n";
		dialog += "add_smalltext|World Background ID (WORLD_BG)|left|\n";
		dialog += "add_text_input|world_bg||" + std::to_string(WorldDefault.bg) + "|5|\n";
		dialog += "add_smalltext|World Rock ID (WORLD_ROCK)|left|\n";
		dialog += "add_text_input|world_rock||" + std::to_string(WorldDefault.rock) + "|5|\n";
		dialog += "add_smalltext|World Lava ID (WORLD_LAVA)|left|\n";
		dialog += "add_text_input|world_lava||" + std::to_string(WorldDefault.lava) + "|5|\n";
		dialog += "add_smalltext|World Weather ID (WORLD_WEATHER)|left|\n";
		dialog += "add_text_input|world_weather||" + std::to_string(WorldDefault.weather) + "|3|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_label_with_icon|small|Store Stock (GAME)|left|32|\n";
		dialog += "add_smalltext|Store Stock ERay (STORE_STOCK_ERAY)|left|\n";
		dialog += "add_text_input|stock_eray||" + std::to_string(Environment()->Stock_ERay) + "|5|\n";
		dialog += "add_smalltext|Store Stock GRay (STORE_STOCK_GRAY)|left|\n";
		dialog += "add_text_input|stock_gray||" + std::to_string(Environment()->Stock_GRay) + "|5|\n";
		dialog += "add_smalltext|Store Stock Mag (STORE_STOCK_MAG)|left|\n";
		dialog += "add_text_input|stock_mag||" + std::to_string(Environment()->Stock_Mag) + "|5|\n";
		dialog += "add_small_font_button|editstorestock|Edit Store Stock|noflags|0|0|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_label_with_icon|small|Server URL & OSM (SERVER)|left|32|\n";
		dialog += "add_smalltext|Deposit World (DEPOSIT_WORLD)|left|\n";
		dialog += "add_text_input|depositname||" + Environment()->Deposit_World + "|20|\n";
		dialog += "add_smalltext|Website URL (WEBSITE_URL)|left|\n";
		dialog += "add_text_input|websiteurl||" + Environment()->website_url + "|50|\n";
		dialog += "add_smalltext|Discord URL (DISCORD_URL)|left|\n";
		dialog += "add_text_input|discordurl||" + Environment()->discord_url + "|50|\n";
		dialog += "add_smalltext|WhatsApp URL (WHATSAPP_URL)|left|\n";
		dialog += "add_text_input|whatsappurl||" + Environment()->whatsapp_url + "|50|\n";
		dialog += "add_smalltext|AI API Key (APIKEY_AI)|left|\n";
		dialog += "add_text_input|apikey_ai||" + Environment()->apikey_ai + "|50|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_label_with_icon|small|Misc|left|32|\n";
		dialog += "add_small_font_button|open_event|Game Events|noflags|0|0|\n";
		dialog += "add_small_font_button|open_rcode|Create a Redeem Code|noflags|0|0|\n";
		dialog += "add_small_font_button|open_newbie|Newbie Rewards|noflags|0|0|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_label_with_icon|small|Creator List (GAME: CREATOR_LIST)|left|32|\n";
		dialog += "add_smalltext|Add/Remove player in Creator List. Enter GrowID.|left|\n";
		dialog += "add_text_input|clist_name|||20|\n";
		dialog += "add_small_font_button|add_clist|Add to Creator List|noflags|0|0|\n";
		dialog += "add_small_font_button|remove_clist|Remove from Creator List|noflags|0|0|\n";
		dialog += "add_spacer|small|\n";
		{
			auto bscfg = LoadBuyShopConfig();
			dialog += "add_label_with_icon|small|Buy Command Settings (/buy)|left|6016|\n";
			dialog += "add_smalltext|Payment Mode: 0=Gems, 1=Item|left|\n";
			dialog += "add_text_input|buy_payment_mode||" + std::to_string(bscfg.buy.payment_mode) + "|2|\n";
			dialog += "add_smalltext|Gems Price (if mode=0)|left|\n";
			dialog += "add_text_input|buy_price_gems||" + std::to_string(bscfg.buy.price_gems) + "|10|\n";
			dialog += "add_smalltext|Items Per Purchase|left|\n";
			dialog += "add_text_input|buy_items_per_purchase||" + std::to_string(bscfg.buy.items_per_purchase) + "|5|\n";
			dialog += "add_smalltext|Payment Item ID (if mode=1)|left|\n";
			dialog += "add_text_input|buy_payment_item_id||" + std::to_string(bscfg.buy.payment_item_id) + "|6|\n";
			dialog += "add_smalltext|Payment Item Amount (if mode=1)|left|\n";
			dialog += "add_text_input|buy_payment_item_amount||" + std::to_string(bscfg.buy.payment_item_amount) + "|6|\n";
			dialog += "add_spacer|small|\n";

			// /shop Role Settings - button-based like /grole
			dialog += "add_label_with_icon|small|Shop Roles (/shop)|left|8470|\n";
			dialog += "add_smalltext|`oCurrent roles in shop (paid with Coins):|left|\n";
			for (int ri = 0; ri < (int)bscfg.shop_roles.size(); ri++) {
				auto& sr = bscfg.shop_roles[ri];
				dialog += "add_smalltext|`w" + sr.role_name + " - `2" + std::to_string(sr.price_coins) + " Coins``|left|\n";
			}
			dialog += "add_spacer|small|\n";
			dialog += "add_small_font_button|cfg_shop_roles_manage|Manage Shop Roles|noflags|0|0|\n";
			dialog += "add_spacer|small|\n";
		}

		dialog += "end_dialog|apply_config|Cancel|Apply|\n";
		VarList::OnDialogRequest(peer, dialog);
	}
	static void Command_CekAlt(ENetPeer* peer, const std::string& arg) {
		std::string target = arg;
		if (target.empty()) {
			VarList::OnConsoleMessage(peer, "Usage: /cekalt <GrowID>");
			return;
		}

		std::string targetPath = "database/players/" + target + "_.json";
		if (!fs::exists(targetPath)) {
			VarList::OnConsoleMessage(peer, "Account `" + target + "` not found in database.");
			return;
		}

		std::string baseIp;
		std::string baseMac;

		try {
			std::ifstream f(targetPath, std::ios::binary);
			if (!f.is_open()) {
				VarList::OnConsoleMessage(peer, "Failed to open data for `" + target + "`.");
				return;
			}

			json data;
			f >> data;

			if (data.contains("ip") && data["ip"].is_string())
				baseIp = data["ip"].get<std::string>();
			if (data.contains("last_ip") && data["last_ip"].is_string())
				baseIp = data["last_ip"].get<std::string>();

			if (data.contains("mac") && data["mac"].is_string())
				baseMac = data["mac"].get<std::string>();

			if (baseIp.empty() && baseMac.empty()) {
				VarList::OnConsoleMessage(peer, "No IP/MAC data stored for `" + target + "`.");
				return;
			}
		}
		catch (...) {
			VarList::OnConsoleMessage(peer, "Error while reading data for `" + target + "`.");
			return;
		}

		std::vector<std::string> alts;

		try {
			for (const auto& entry : fs::directory_iterator("database/players")) {
				if (!entry.is_regular_file()) continue;

				std::string fname = entry.path().filename().string();
				if (fname.size() <= 6) continue;
				if (fname.rfind("_.json") == std::string::npos) continue;

				std::string accName = fname.substr(0, fname.size() - 6);
				if (accName == target) continue;

				std::string ip, mac;
				try {
					std::ifstream ff(entry.path(), std::ios::binary);
					if (!ff.is_open()) continue;
					json d;
					ff >> d;

					if (d.contains("ip") && d["ip"].is_string())
						ip = d["ip"].get<std::string>();
					if (d.contains("last_ip") && d["last_ip"].is_string())
						ip = d["last_ip"].get<std::string>();
					if (d.contains("mac") && d["mac"].is_string())
						mac = d["mac"].get<std::string>();
				}
				catch (...) {
					continue;
				}

				bool same = false;
				if (!baseIp.empty() && !ip.empty() && ip == baseIp)
					same = true;
				if (!same && !baseMac.empty() && !mac.empty() && mac == baseMac)
					same = true;

				if (same) {
					alts.push_back(accName);
				}
			}
		}
		catch (...) {
			VarList::OnConsoleMessage(peer, "Error while scanning player database.");
			return;
		}

		if (alts.empty()) {
			VarList::OnConsoleMessage(peer, "No alt accounts found for `" + target + "` (based on IP/MAC).");
		}
		else {
			std::string list;
			for (size_t i = 0; i < alts.size(); ++i) {
				if (i > 0) list += ", ";
				list += alts[i];
			}
			VarList::OnConsoleMessage(peer, "Alt accounts for ``" + target + "``: " + list + " (IP/MAC match).");
		}
	}
	static void Cmd_AutoConvertOn(ENetPeer* peer, const std::string& param, int level) {
		Player* pl = pInfo(peer);
		if (!pl) return;

		pl->autoConvert = true;
		VarList::OnConsoleMessage(peer, "`2[AutoConvert]`o AutoConvert enabled!");
	}

	// /autoconvertoff
	static void Cmd_AutoConvertOff(ENetPeer* peer, const std::string& param, int level) {
		Player* pl = pInfo(peer);
		if (!pl) return;

		pl->autoConvert = false;
		VarList::OnConsoleMessage(peer, "`4[AutoConvert]`o AutoConvert disabled!");
	}
	static void HELP(ENetPeer* peer) {
		int userLevel = Role::GetLevel(peer);
		std::map<int, std::vector<std::string>> cmdsByLevel;

		int maxLevel = 0;
		for (const auto& kv : commandMap) {
			if (kv.second.second > maxLevel) maxLevel = kv.second.second;
		}

		for (const auto& kv : commandMap) {
			const std::string& cmd = kv.first;
			int requiredLevel = kv.second.second;
			if (requiredLevel <= userLevel) {
				cmdsByLevel[requiredLevel].push_back(cmd);
			}
		}

		for (auto& kv : cmdsByLevel) {
			std::sort(kv.second.begin(), kv.second.end());
		}

		auto headerForLevel = [&](int lvl) -> std::string {
			switch (lvl) {
			case 0:  return "`5Player Commands";
			case 1:  return "`1[Cheater] Commands";
			case 2:  return "`1[VIP] Commands";
			case 3:  return "`#@Moderator Commands";
			case 4:  return "`e@Administrator Commands";
			case 5:  return "`6@Developer Commands";
			case 6:  return "`2@Donatur Commands";
			case 7:  return "`9@God Commands";
			case 8:  return "`8@Unlimited Commands";
			case 9:  return "`4@Streamers Commands";
			case 10: return "`4@Staff Commands";
			case 11: return "`b@Owner Commands";
			case 12: return "`^@Coder Commands";
			case 13: return "`c@Creator Commands";
			case 14: return "`c@Albin Commands";
			case 999: return "`c@Config Commands";
			default: {
				std::string role_label;
				{ std::lock_guard<std::mutex> lk(g_customRoles_mtx); for (auto& r : g_customRoles) { if (r.level == lvl) { role_label = r.prefix + r.name + "`` Commands"; break; } } }
				if (!role_label.empty()) return role_label;
				return "Commands (level " + std::to_string(lvl) + ")";
			}
			}
			};

		std::string dialog = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wCommand List|left|982|\nadd_spacer|small|\nadd_textbox|`oYour role level: `2" + std::to_string(userLevel) + "`o — Showing commands you can use.|left|\nadd_spacer|small|";

		bool hasCmds = false;
		for (int lvl = 0; lvl <= maxLevel; ++lvl) {
			auto it = cmdsByLevel.find(lvl);
			if (it == cmdsByLevel.end() || it->second.empty()) continue;
			if (userLevel < lvl) continue;

			hasCmds = true;
			dialog += "\nadd_smalltext|`o>> " + headerForLevel(lvl) + "``|left|";

			std::string cmdList;
			bool first = true;
			for (const auto& cmd : it->second) {
				if (!first) cmdList += " ";
				cmdList += cmd;
				first = false;
			}
			dialog += "\nadd_textbox|" + cmdList + "|left|";
			dialog += "\nadd_spacer|small|";
		}

		if (!hasCmds) {
			dialog += "\nadd_textbox|`4No commands available for your role.|left|";
		}

		dialog += "\nend_dialog|Help_Dialog||Close|\nadd_quick_exit|";
		VarList::OnDialogRequest(peer, dialog);
	}
	static void AIBARU(ENetPeer* peer) {
		//stdnplaysound:string command = act_cmd;
		string text = "/ai ";
		if (text.empty()) {
			VarList::OnConsoleMessage(peer, "Please input text for AI processing.");
			return;
		}
		ENetPeer* current_peer = peer;
		thread([text, current_peer]() {
			try {
				string jsonResponse = fancy::ai::sendRequest(text);
				string message = fancy::ai::extractMessage(jsonResponse);

				if (message.empty()) {
					VarList::OnConsoleMessage(current_peer, "`2AI`0: Sorry, no response received.");
				}
				else {
					VarList::OnConsoleMessage(current_peer, "`2AI`0: " + message);
				}
			}
			catch (const std::exception& e) {
				VarList::OnConsoleMessage(current_peer, "`4Error`0: Failed to process your request.");
			}
			}).detach();

		return;
	}
	static void ONLINEIYAGAMI(ENetPeer* peer) {
		static const std::unordered_map<std::string, std::string> country_names = {
		 {"af", "Afghanistan"}, {"al", "Albania"}, {"dz", "Algeria"}, {"as", "American Samoa"}, {"ad", "Andorra"},
		 {"ao", "Angola"}, {"ai", "Anguilla"}, {"aq", "Antarctica"}, {"ag", "Antigua and Barbuda"}, {"ar", "Argentina"},
		 {"am", "Armenia"}, {"aw", "Aruba"}, {"au", "Australia"}, {"at", "Austria"}, {"az", "Azerbaijan"},
		 {"bs", "Bahamas"}, {"bh", "Bahrain"}, {"bd", "Bangladesh"}, {"bb", "Barbados"}, {"by", "Belarus"},
		 {"be", "Belgium"}, {"bz", "Belize"}, {"bj", "Benin"}, {"bm", "Bermuda"}, {"bt", "Bhutan"},
		 {"bo", "Bolivia"}, {"ba", "Bosnia and Herzegovina"}, {"bw", "Botswana"}, {"br", "Brazil"}, {"bn", "Brunei"},
		 {"bg", "Bulgaria"}, {"bf", "Burkina Faso"}, {"bi", "Burundi"}, {"cv", "Cabo Verde"}, {"kh", "Cambodia"},
		 {"cm", "Cameroon"}, {"ca", "Canada"}, {"ky", "Cayman Islands"}, {"cf", "Central African Republic"}, {"td", "Chad"},
		 {"cl", "Chile"}, {"cn", "China"}, {"co", "Colombia"}, {"km", "Comoros"}, {"cg", "Congo"},
		 {"cd", "Congo, Democratic Republic of the"}, {"ck", "Cook Islands"}, {"cr", "Costa Rica"}, {"ci", "C�te d'Ivoire"}, {"hr", "Croatia"},
		 {"cu", "Cuba"}, {"cy", "Cyprus"}, {"cz", "Czech Republic"}, {"dk", "Denmark"}, {"dj", "Djibouti"},
		 {"dm", "Dominica"}, {"do", "Dominican Republic"}, {"ec", "Ecuador"}, {"eg", "Egypt"}, {"sv", "El Salvador"},
		 {"gq", "Equatorial Guinea"}, {"er", "Eritrea"}, {"ee", "Estonia"}, {"sz", "Eswatini"}, {"et", "Ethiopia"},
		 {"fj", "Fiji"}, {"fi", "Finland"}, {"fr", "France"}, {"ga", "Gabon"}, {"gm", "Gambia"},
		 {"ge", "Georgia"}, {"de", "Germany"}, {"gh", "Ghana"}, {"gi", "Gibraltar"}, {"gr", "Greece"},
		 {"gl", "Greenland"}, {"gd", "Grenada"}, {"gu", "Guam"}, {"gt", "Guatemala"}, {"gn", "Guinea"},
		 {"gw", "Guinea-Bissau"}, {"gy", "Guyana"}, {"ht", "Haiti"}, {"hn", "Honduras"}, {"hk", "Hong Kong"},
		 {"hu", "Hungary"}, {"is", "Iceland"}, {"in", "India"}, {"id", "Indonesia"}, {"ir", "Iran"},
		 {"iq", "Iraq"}, {"ie", "Ireland"}, {"il", "Israel"}, {"it", "Italy"}, {"jm", "Jamaica"},
		 {"jp", "Japan"}, {"jo", "Jordan"}, {"kz", "Kazakhstan"}, {"ke", "Kenya"}, {"ki", "Kiribati"},
		 {"kp", "Korea, Democratic People's Republic of"}, {"kr", "Korea, Republic of"}, {"kw", "Kuwait"}, {"kg", "Kyrgyzstan"}, {"la", "Lao People's Democratic Republic"},
		 {"lv", "Latvia"}, {"lb", "Lebanon"}, {"ls", "Lesotho"}, {"lr", "Liberia"}, {"ly", "Libya"},
		 {"li", "Liechtenstein"}, {"lt", "Lithuania"}, {"lu", "Luxembourg"}, {"mo", "Macao"}, {"mg", "Madagascar"},
		 {"mw", "Malawi"}, {"my", "Malaysia"}, {"mv", "Maldives"}, {"ml", "Mali"}, {"mt", "Malta"},
		 {"mh", "Marshall Islands"}, {"mr", "Mauritania"}, {"mu", "Mauritius"}, {"mx", "Mexico"}, {"fm", "Micronesia"},
		 {"md", "Moldova"}, {"mc", "Monaco"}, {"mn", "Mongolia"}, {"me", "Montenegro"}, {"ma", "Morocco"},
		 {"mz", "Mozambique"}, {"mm", "Myanmar"}, {"na", "Namibia"}, {"nr", "Nauru"}, {"np", "Nepal"},
		 {"nl", "Netherlands"}, {"nz", "New Zealand"}, {"ni", "Nicaragua"}, {"ne", "Niger"}, {"ng", "Nigeria"},
		 {"mk", "North Macedonia"}, {"no", "Norway"}, {"om", "Oman"}, {"pk", "Pakistan"}, {"pw", "Palau"},
		 {"ps", "Palestine"}, {"pa", "Panama"}, {"pg", "Papua New Guinea"}, {"py", "Paraguay"}, {"pe", "Peru"},
		 {"ph", "Philippines"}, {"pl", "Poland"}, {"pt", "Portugal"}, {"qa", "Qatar"}, {"ro", "Romania"},
		 {"ru", "Russian Federation"}, {"rw", "Rwanda"}, {"kn", "Saint Kitts and Nevis"}, {"lc", "Saint Lucia"}, {"vc", "Saint Vincent and the Grenadines"},
		 {"ws", "Samoa"}, {"sm", "San Marino"}, {"st", "Sao Tome and Principe"}, {"sa", "Saudi Arabia"}, {"sn", "Senegal"},
		 {"rs", "Serbia"}, {"sc", "Seychelles"}, {"sl", "Sierra Leone"}, {"sg", "Singapore"}, {"sk", "Slovakia"},
		 {"si", "Slovenia"}, {"sb", "Solomon Islands"}, {"so", "Somalia"}, {"za", "South Africa"}, {"ss", "South Sudan"},
		 {"es", "Spain"}, {"lk", "Sri Lanka"}, {"sd", "Sudan"}, {"sr", "Suriname"}, {"se", "Sweden"},
		 {"ch", "Switzerland"}, {"sy", "Syrian Arab Republic"}, {"tw", "Taiwan"}, {"tj", "Tajikistan"}, {"tz", "Tanzania"},
		 {"th", "Thailand"}, {"tl", "Timor-Leste"}, {"tg", "Togo"}, {"to", "Tonga"}, {"tt", "Trinidad and Tobago"},
		 {"tn", "Tunisia"}, {"tr", "Turkey"}, {"tm", "Turkmenistan"}, {"tv", "Tuvalu"}, {"ug", "Uganda"},
		 {"ua", "Ukraine"}, {"ae", "United Arab Emirates"}, {"gb", "United Kingdom"}, {"us", "United States of America"}, {"uy", "Uruguay"},
		 {"uz", "Uzbekistan"}, {"vu", "Vanuatu"}, {"ve", "Venezuela"}, {"vn", "Viet Nam"}, {"ye", "Yemen"},
		 {"zm", "Zambia"}, {"zw", "Zimbabwe"}
		};

		int total_online = 0, total_pc = 0, total_ios = 0, total_android = 0, total_macbook = 0, total_linux = 0;
		std::unordered_map<std::string, int> country_count;
		std::string names = "";
		names.reserve(2048);
		std::unordered_set<std::string> active_usernames;

		struct FormattedPlayerEntry {
			std::string formatted;
			std::string country;
			std::string device;
			bool is_real;
		};
		std::vector<FormattedPlayerEntry> all_entries;
		all_entries.reserve(256);

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			std::string player_world = pInfo(cp_)->world.empty() ? "EXIT" : pInfo(cp_)->world;
			std::string fmt = pInfo(cp_)->name_color + pInfo(cp_)->tankIDName + " ``[`2" + player_world + "``], ";
			active_usernames.insert(pInfo(cp_)->tankIDName);
			total_online++;

			const std::string& device = pInfo(cp_)->player_device;
			if (device == "0,1,1") total_pc++;
			else if (device == "1")     total_ios++;
			else if (device == "2")     total_macbook++;
			else if (device == "3")     total_linux++;
			else                        total_android++;

			country_count[pInfo(cp_)->country]++;
			all_entries.push_back({ fmt, pInfo(cp_)->country, device, true });
		}

		if (Environment()->Fake_Online) {
			int target_total = ServerPool::PlayerCountServer();
			if (target_total > total_online) {
				int needed = target_total - total_online;
				std::vector<ServerPool::RealisticFakePlayer> fakes = ServerPool::GetFakePlayerList(needed, active_usernames);
				for (const auto& fp : fakes) {
					std::string fmt = "`#@" + fp.name + " ``[`2" + fp.world + "``], ";
					total_online++;

					if (fp.device == "0,1,1")      total_pc++;
					else if (fp.device == "1")     total_ios++;
					else if (fp.device == "2")     total_macbook++;
					else if (fp.device == "3")     total_linux++;
					else                           total_android++;

					country_count[fp.country]++;
					all_entries.push_back({ fmt, fp.country, fp.device, false });
				}
			}
		}

		if (all_entries.size() > 1) {
			for (size_t i = all_entries.size() - 1; i > 0; --i) {
				size_t j = (i * 17 + 3) % (i + 1);
				std::swap(all_entries[i], all_entries[j]);
			}
		}

		for (const auto& entry : all_entries) {
			names += entry.formatted;
		}

		if (!names.empty() && names.substr(names.size() - 2) == ", ") {
			names = names.substr(0, names.size() - 2);
		}

		std::string country_flags = "";
		country_flags.reserve(512);
		for (const auto& [country, count] : country_count) {
			if (count > 0) {
				auto it = country_names.find(country);
				const std::string& cname = (it != country_names.end()) ? it->second : country;
				country_flags += "\nadd_custom_button||image:interface/flags/" + country + ".rttex;image_size:16,12;width:0.03;state:visibled;|left|\nadd_smalltext|`w" + cname + " - `2" + std::to_string(count) + " `$User Online|left|\nreset_placement_x|";
			}
		}
		int bar_max = (total_online < 200) ? 200 : ((total_online / 50 + 1) * 50);
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wServer Statistics|left|3802|\nadd_label_with_icon|small|`$Info|left|7190|\nadd_progress_bar|`o  Server: " + Environment()->server_name + "``|small|`o  Online:``|" + std::to_string(total_online) + "|" + std::to_string(bar_max) + "|(" + std::to_string(total_online) + "/" + std::to_string(bar_max) + ")|-3669761|\nadd_smalltext|Uptime: `^" + Time::Playmod(time(nullptr) - Environment()->UpTime) + "`w.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`$Player Device:|left|572|\nadd_smalltext|`wPC Users: `2" + std::to_string(total_pc) + "`w.<CR>Android Users: `2" + std::to_string(total_android) + "`w.<CR>IOS Users: `2" + std::to_string(total_ios) + "`w.<CR>Macbook Users: `2" + std::to_string(total_macbook) + "`w.<CR>Linux Users: `2" + std::to_string(total_linux) + "`w.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`$Player Country|left|3394|" + country_flags + "\nadd_spacer|small|\nadd_label_with_icon|small|`$Player Username|left|1280|\nadd_textbox| " + names + " |left|\nadd_quick_exit|");
	}

	static void FAKEONLINE(ENetPeer* peer, const std::string& param) {
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "`6Usage: `/fakeonline <on|off|count> `o(e.g., `/fakeonline 150` or `/fakeonline on` or `/fakeonline off`)``");
			VarList::OnConsoleMessage(peer, "Current Status: " + std::string(Environment()->Fake_Online ? "`2ACTIVE" : "`4INACTIVE") + " `o(Real: `" + std::to_string(ServerPool::PlayerCountReal()) + "`o | Base Target: `" + std::to_string(Environment()->Fake_Online_Count) + "`o | Displayed Total: `" + std::to_string(ServerPool::PlayerCountServer()) + "``)");
			return;
		}
		std::string p = param;
		std::transform(p.begin(), p.end(), p.begin(), ::tolower);
		if (p == "on" || p == "1" || p == "enable" || p == "true") {
			Environment()->Fake_Online = true;
			EventPool::Save::Config();
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Fake Online is now `2ACTIVE!", 0, 0);
			VarList::OnConsoleMessage(peer, "`2[Fake Online] Dynamic Fake Online status set to ACTIVE. Displayed Total: `" + std::to_string(ServerPool::PlayerCountServer()) + "``");
		} else if (p == "off" || p == "0" || p == "disable" || p == "false") {
			Environment()->Fake_Online = false;
			EventPool::Save::Config();
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Fake Online is now `4INACTIVE!", 0, 0);
			VarList::OnConsoleMessage(peer, "`4[Fake Online] Dynamic Fake Online status set to INACTIVE. Real players showing: `" + std::to_string(ServerPool::PlayerCountReal()) + "``");
		} else {
			try {
				int count = std::stoi(p);
				if (count < 0) count = 0;
				Environment()->Fake_Online = true;
				Environment()->Fake_Online_Count = count;
				EventPool::Save::Config();
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Fake Online base target set to `2" + std::to_string(count) + "`w!", 0, 0);
				VarList::OnConsoleMessage(peer, "`2[Fake Online] Dynamic Fake Online active with base target `" + std::to_string(count) + "`2 (Displayed Total: `" + std::to_string(ServerPool::PlayerCountServer()) + "``)");
			} catch (...) {
				VarList::OnConsoleMessage(peer, "`4Invalid input! Use: /fakeonline <on|off|count>");
			}
		}
	}

	static void GIVEGEMS(ENetPeer* peer, string cmd) {
		string act_cmd = cmd;
		string msg_info = act_cmd;
		size_t extra_space = msg_info.find("  ");
		if (extra_space != std::string::npos) {
			msg_info.replace(extra_space, 2, " ");
		}
		string delimiter = " ";
		size_t pos = 0;
		string pm_user;
		string pm_message;
		if ((pos = msg_info.find(delimiter)) != std::string::npos) msg_info.erase(0, pos + delimiter.length());
		else {
			VarList::OnConsoleMessage(peer, "`oPlease specify a Player's you want to give Gems.");
		}
		if ((pos = msg_info.find(delimiter)) != std::string::npos) {
			pm_user = msg_info.substr(0, pos);
			msg_info.erase(0, pos + delimiter.length());
		}
		else VarList::OnConsoleMessage(peer, "`oPlease enter Player's name.");
		pm_message = msg_info;
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->m_h or pInfo(peer)->growid == false) continue;
			if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pm_user)) {
				variants::on_bux_gems(currentPeer, atoi(pm_message.c_str()));
				variants::on_notif(currentPeer, "`wYou received `2" + pm_message + " `wGems from Owner!", "interface/cash_icon_overlay.rttex", "audio/hub_open.wav");
				variants::on_msg(peer, "`oSuccesfully give " + pm_message + " Gems to " + pInfo(currentPeer)->tankIDName + ".");
				break;
			}
		}
	}
	static void TAKE_WORLDS(ENetPeer* peer) {
		string name_ = pInfo(peer)->world;
		vector<World>::iterator paa = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (paa != worlds.end()) {
			World* world_ = &worlds[paa - worlds.begin()];
			world_->fresh_world = true;
			string owner_name = to_lower(world_->owner_name), player_name = to_lower(pInfo(peer)->tankIDName);
			if (world_->owner_name.empty()) return;
			if (world_->guild_id != 0) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`wYou can't use that commands here.");
				return;
			}
			if (world_->owner_name == pInfo(peer)->tankIDName) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`wYou already own of this World.");
				return;
			}
			int lock_id = 0, lock_x = -1, lock_y = -1;
			int ySize = (int)world_->blocks.size() / 100, xSize = (int)world_->blocks.size() / ySize;
			{
				vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [&](const WorldBlock& a) { return items[a.fg].blockType == BlockTypes::LOCK && a.fg != 202 && a.fg != 204 && a.fg != 206 && a.fg != 4994 && a.fg != 10000; });
				if (p != world_->blocks.end()) {
					int i_ = p - world_->blocks.begin();
					WorldBlock* block__ = &world_->blocks[p - world_->blocks.begin()];
					uint32_t id_ = block__->fg;
					lock_id = id_, lock_x = i_ % xSize, lock_y = i_ / xSize;
				}
			}
			if (lock_id == 0) return;
			string before_name = world_->owner_name;
			world_->owner_name = pInfo(peer)->tankIDName;
			if (pInfo(peer)->name_color != "`0" or Role::Moderator(peer) || Role::Administrator(peer) && pInfo(peer)->name_color != "`2")  world_->owner_named = pInfo(peer)->name_color + pInfo(peer)->tankIDName;
			else world_->owner_named = pInfo(peer)->tankIDName;
			pInfo(peer)->worlds_owned.push_back(world_->name);
			PlayerMoving data_{};
			data_.packetType = 15, data_.punchX = lock_x, data_.punchY = lock_y, data_.characterState = 0x8, data_.netID = name_to_number(to_lower(pInfo(peer)->tankIDName)), data_.plantingTree = lock_id;
			BYTE* raw = packPlayerMoving(&data_, 56);
			VisualHandle::Nick(peer, NULL);
			pInfo(peer)->name_color = Role::Prefix2(peer, *world_);
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != world_->name) continue;
				pInfo(currentPeer)->world_owner = (world_->owner_named.empty() ? "`#" + world_->owner_name : world_->owner_named);
				VarList::OnPlaySound(currentPeer, "audio/use_lock.wav");
				send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
				VarList::OnConsoleMessage(currentPeer, "`5[```w" + world_->name + "`` has been `$World Locked`` by " + pInfo(peer)->name_color + world_->owner_name + "```5]``");
				VarList::OnTalkBubble(currentPeer, pInfo(peer)->netID, "`5[```w" + world_->name + "`` has been `$World Locked`` by " + pInfo(peer)->name_color + world_->owner_name + "```5]``", 0, 0);
			}
			delete[] raw;
			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " take world : " + world_->name + ".", "Take Worlds");
		}
	}
	static void FIND_IDNYO(ENetPeer* peer, string cmd) {
		string error_message = "Usage: /findid <`$item name``> - This will show you the mentioned item price and purchase option if any.";
		vector<string> a_ = explode(" ", cmd);
		if (a_.size() <= 1) {
			gamepacket_t p;
			p.Insert("OnConsoleMessage");
			p.Insert(error_message);
			p.CreatePacket(peer);
			return;
		}
		if (a_.size() >= 2) {
			a_.erase(a_.begin());
			string find_target = to_lower(join(a_, " "));
			if (find_target.size() < 3) {
				gamepacket_t p;
				p.Insert("OnConsoleMessage");
				p.Insert("Item name must be more than 2 letters!");
				p.CreatePacket(peer);
				return;
			}
			string buy_list_ = "";
			if (Role::Developer) {
				for (int i_ = 0; i_ < items.size(); i_++) {
					uint32_t item_id = items[i_].id;
					if (items[item_id].blockType == SEED) continue;
					if (to_lower(items[i_].ori_name).find(find_target) != string::npos) {
						buy_list_ += "\nadd_label_with_icon|big||left|" + to_string(item_id) + "|\n\nadd_textbox|`$" + items[i_].ori_name + "``|\n\nadd_textbox|Item ID : `9" + to_string(i_) + "|\nadd_textbox|Rarity : " + to_string(items[i_].rarity) + "|\nadd_spacer|small|";
					}
				}
			}
			else {
				for (int i_ = 0; i_ < kainos.size(); i_++) {
					uint32_t item_id = kainos[i_][0];
					if (to_lower(items[item_id].ori_name).find(find_target) != string::npos) {
						buy_list_ += "\nadd_label_with_icon|big||left|" + to_string(item_id) + "|\n\nadd_textbox|`$" + items[i_].ori_name + "``|\n\nadd_textbox|Item ID : `9" + to_string(i_) + "|\nadd_textbox|Rarity : " + to_string(items[i_].rarity) + "|\nadd_spacer|small|";
					}
				}
			}
			if (buy_list_.empty()) {
				CAction::Log(peer, "action|log\nmsg| `4Oops: `oThere is no item starting with `w" + find_target + "`o.", "");
				return;
			}
			VarList::OnDialogRequest(peer, "add_label_with_icon|big|`wFound item : " + find_target + "``|left|6016|\nadd_spacer|small|\nembed_data|search|" + find_target + "\nend_dialog|buy_option|Cancel|\nadd_spacer|big|\nadd_spacer|small|\n" + buy_list_ + "|\nadd_textbox|`1You can get those item with `$/give <itemid> <count>|\nend_dialog|finddialog||OK|\nadd_quick_exit|\n");
		}
	}
	static void CASINO_LIST(ENetPeer* peer) {
		std::string csn_list = "", wait_list = ""; int total = 0, total_ = 0;
		for (int w_ = 0; w_ < Environment()->Verifed_Csn_World.size(); w_++) {
			total++;
			csn_list += "\nadd_button|warp_to_" + Environment()->Verifed_Csn_World[w_] + "|`w[`2#" + to_string(total) + "``] `b" + Environment()->Verifed_Csn_World[w_] + "|0|0|";
		}
		for (int t_ = 0; t_ < Environment()->Waiting_Verif_Csn.size(); t_++) {
			total_++;
			wait_list += "\nadd_button|warp_to_" + Environment()->Waiting_Verif_Csn[t_] + "|`w[`5#" + to_string(total_) + "``] `o" + Environment()->Waiting_Verif_Csn[t_] + "|0|0|";
		}
		if (csn_list.empty()) csn_list = "\nadd_textbox|`oThere are no verifed casino worlds!|left|";
		if (wait_list.empty()) wait_list = "\nadd_textbox|`oThere are no waiting world to verify!|left|";
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wVerifed `b[Casino] ``World|left|758|\nadd_spacer|small|\nadd_textbox|`oList of verifed `b[Casino] `` worlds:|left|" + csn_list + "|" + (Role::Owner(peer) ? "\nadd_spacer|small|\nadd_textbox|`oList of world requesting verification:|left|" + wait_list + "|\nadd_spacer|small|" : "") + "\nend_dialog|top|Close||\nand_quick_exit|");
	}
	static void REQUEST_VERIFY_CSN(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (to_lower(world_->owner_name) == to_lower(pInfo(peer)->tankIDName)) {
				if (find(Environment()->Waiting_Verif_Csn.begin(), Environment()->Waiting_Verif_Csn.end(), pInfo(peer)->world) != Environment()->Waiting_Verif_Csn.end()) {
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`wYour World is already on the waiting list.", 0, 1);
				}
				else {
					VarList::OnDialogRequest(peer, SetColor(peer) +
						"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
						"add_label|big|Request to Verification Casino World|left|\n"
						"add_spacer|small|\n"
						"add_smalltext|`o- Every world owner must use donation box to avoid user genta.|left|\n"
						"add_smalltext|`o- Every admin must have a guarantee and pay every (day/week) depending on the world owner to solve the refund problem.|left|\n"
						"add_smalltext|`o- Gambling using items is strictly prohibited, only required locks (wl, dl, bgl, mgl).|left|\n"
						"add_smalltext|`o- Minimum player and hoster must be above level 10 if below the specified level it will be banned immediately.|left|\n"
						"add_smalltext|`o- World gambling owners for verification pay around 30bgl and every week pay 10 bgl to @Owner_Server as an emergency fund if there is a problem.|left|\n"
						"add_smalltext|`o- (non verif) World without verification will immediately get a penalty in the form of a nuke and all users, admins, and owners are banned (if the slightest problem occurs).|left|\n"
						"add_smalltext|`o- every staff on the server only helps or only receives reports will not provide refunds.|left|\n"
						"add_spacer|small|\n"
						"add_smalltext|`4NOTE: `oWe will review the world that you want to be verified as a verified world casino after you pay the price explained above.|left|\n"
						"add_smalltext|`4NOTE: `oworld you will be removed from the waiting list to be verified if you do not pay the fees described above.|left|\n"
						"end_dialog|Req_Verify_Csn|Cancel|Send|\n"
						"add_quick_exit|"
					);
				}
			}
			else {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Only the owner of this world can request to verify this casino world.", 0, 1);
			}
		}
	}
	static void DIALOG_TEST(ENetPeer* peer) {
		DialogHandle::DTesting(peer);
	}
	static void ADDSPOTIFY(ENetPeer* peer) {
		if (!pInfo(peer)->Role.has_config_access && !pInfo(peer)->Role.Owner_Server) {
			VarList::OnConsoleMessage(peer, "`4You do not have permission to use this command!");
			return;
		}
		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
			"add_label_with_icon|big|`wAdd / Delete Spotify Music|left|11432|\n"
			"add_smalltext|Tambahkan lagu (Nama & Path) atau Hapus lagu (pake Nomor Lagu / Path):|left|\n"
			"add_spacer|small|\n"
			"add_text_input|song_name|Song Name (untuk Add)||50|\n"
			"add_text_input|audio_path|Audio Path / Nomor Lagu (contoh: 1)|audio/test.ogg|100|\n"
			"add_spacer|small|\n"
			"add_button|Add|`oAdd``|noflags|0|0|\n"
			"add_button|Delete|`4Delete``|noflags|0|0|\n"
			"end_dialog|addspotify_logic|Cancel||\n";
		VarList::OnDialogRequest(peer, dialog);
	}
	static void POS(ENetPeer* peer) {
		VarList::OnConsoleMessage(peer, "x: " + to_string(pInfo(peer)->x) + " y:" + to_string(pInfo(peer)->y));
	}
	static void SET_EXCHANGE(ENetPeer* peer) {
		std::string editedCrystal, editedFish, editedGhost;
		for (const auto& it : exchangeSetPrice) {
			if (it.first <= 0 || it.first >= (int)items.size()) continue;

			if (items[it.first].blockType == BlockTypes::FISH) {
				editedFish += "\nadd_button_with_icon|rem_" + std::to_string(it.first) + "|" +
					items[it.first].name + "|staticBlueFrame|" + std::to_string(it.first) + "||";
			}
			if (items[it.first].blockType == BlockTypes::CRYSTAL) {
				editedCrystal += "\nadd_button_with_icon|rem_" + std::to_string(it.first) + "|" +
					items[it.first].name + "|staticPurpleFrame|" + std::to_string(it.first) + "||";
			}
		}
		for (int id : kGhostItems) {
			if (id <= 0 || id >= (int)items.size()) continue;
			std::string name = items[id].name.empty()
				? "Item " + std::to_string(id)
				: items[id].name;
			editedGhost += "\nadd_button_with_icon|editghost_" + std::to_string(id) + "|" +
				name + "|staticBlueFrame|" + std::to_string(id) + "||";
		}
		std::string dialog;
		dialog = SetColor(peer);
		dialog += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|`wSet Exchange & Ghost Item Price|left|32|\n";
		dialog += "add_smalltext|This menu allows you to set exchange item prices and ghost item prices as you want.|left|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_button|set_all_fish|`^Set Price All Fish``|0|0|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_textbox|`$Edited Fish:|left|\n";
		dialog += "text_scaling_string|99,99999ZB|" + editedFish + "\n";
		dialog += "add_button_with_icon|add_fish||frame|9412||\n";
		dialog += "add_button_with_icon||END_LIST|noflags|0||\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_textbox|`$Edited Crystal:|left|\n";
		dialog += "text_scaling_string|99,99999ZB|" + editedCrystal + "\n";
		dialog += "add_button_with_icon|add_crystal||frame|9412||\n";
		dialog += "add_button_with_icon||END_LIST|noflags|0||\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_textbox|`$Edited Ghost Items:|left|\n";
		dialog += "text_scaling_string|99,99999ZB|" + editedGhost + "\n";
		dialog += "add_button_with_icon||END_LIST|noflags|0||\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_button|setexchangenew|`^Set New Exchange``|0|0|\n";
		dialog += "add_button|exchangeevent|`8Set Exchange Custom``|0|0|\n";
		dialog += "add_quick_exit|\n";
		dialog += "end_dialog|setexchange|Close tab||";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void SET_KITS(ENetPeer* peer) {
		if (!Role::has_config_access(peer)) return;
		std::string dialog = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wKit Settings``|left|14896|\n"
			"add_spacer|small|\n"
			"add_textbox|`oConfigure items, quantities, and levels for Kit Pass. Click any kit to modify:||left|\n"
			"add_spacer|small|\n"
			"text_scaling_string|Subscription++++++++|\n";
		
		for (int i = 0; i < 18; ++i) {
			int item_id = kitsConfig.kits[i].item_id;
			int count = kitsConfig.kits[i].count;
			int req_lvl = kitsConfig.kits[i].level_req;
			std::string item_name = (item_id > 0 && item_id < (int)items.size()) ? items[item_id].name : "None";
			dialog += "add_button_with_icon|edit_kit_" + to_string(i) + "|Kit " + to_string(i + 1) + ": " + to_string(count) + "x " + item_name + " (Lvl " + to_string(req_lvl) + ")|staticYellowFrame|" + to_string(item_id) + "||\n";
		}
		dialog += "add_spacer|small|\nend_dialog|setkits|Close||\nadd_quick_exit|";
		VarList::OnDialogRequest(peer, dialog);
	}
	static void THE_WORLD(ENetPeer* peer) {
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != pInfo(peer)->world) continue;
			VarList::OnAddNotification(cp_, "`#Star Platinum: `9The World!!!", "interface/gtps/zawardo_ban.rttex", "audio/zawardo_sfx.wav");
			if (pInfo(cp_)->tankIDName != pInfo(peer)->tankIDName) VarList::OnSetFreezeState(cp_, pInfo(cp_)->netID, 4000, 2), VarList::OnSetFreezeState(cp_, pInfo(cp_)->netID, 9000, 0);
			VarList::OnSetCurrentWeather(cp_, 43, 4050), VarList::OnSetCurrentWeather(cp_, 0, 9050);
		}
		pInfo(peer)->the_world_time = time(nullptr) + 9;
	}

	static void CHAT_ALL_WORLD(ENetPeer* peer, const std::string& param) {
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "`4Usage: `w/chatallworld <teks>``");
			VarList::OnConsoleMessage(peer, "`oContoh: `w/chatallworld Halo semua! Selamat datang di world ini!``");
			return;
		}
		std::string world = pInfo(peer)->world;
		if (world.empty()) {
			VarList::OnConsoleMessage(peer, "`4You must be inside a world first!``");
			return;
		}

		// FIX: Cooldown 30 detik — shared dengan chat_cmd_time
		long long now_caw = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
		if (pInfo(peer)->chat_cmd_time + 30000 > now_caw) {
			long long rem = (pInfo(peer)->chat_cmd_time + 30000 - now_caw) / 1000 + 1;
			VarList::OnConsoleMessage(peer, "`4>> Cooldown! Tunggu " + std::to_string(rem) + " detik sebelum pakai /chatallworld lagi.");
			return;
		}
		pInfo(peer)->chat_cmd_time = now_caw;

		// Kumpulan Growmoji Growtopia biar ga garing
		static const std::vector<std::string> emojis = {
			"(smile)", "(lol)", "(cool)", "(heart)", "(love)",
			"(wink)", "(wow)", "(dance)", "(yes)", "(no)",
			"(mad)", "(sigh)", "(shy)", "(tongue)", "(kiss)",
			"(agree)", "(music)", "(build)", "(megaphone)", "(see-no-evil)"
		};

		std::string emojiLeft = emojis[rand() % (int)emojis.size()];
		std::string emojiRight = emojis[rand() % (int)emojis.size()];

		// FIX 1: Pakai CT:[FC]_ prefix supaya pesan tampil sebagai chat beneran (bukan system msg)
		//         biar keliatan sama semua player di world, bukan cuma yang kirim
		// FIX 2: Pakai pInfo(cp_)->netID bukan pInfo(peer)->netID
		//         → bubble muncul di atas kepala TIAP player, bukan cuma kepala admin
		//         (kalau pakai netID admin, bubble hanya muncul kalau admin ada di world & terload)
		std::string consoleMsg = "CT:[FC]_ `o>> [`$World Chat`` in `w" + world + "`o] " +
			pInfo(peer)->name_color + pInfo(peer)->tankIDName + " `o: `$" + param + "``";
		std::string bubbleMsg = emojiLeft + " `w" + param + "`` " + emojiRight;

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL || pInfo(cp_)->world != world) continue;
			VarList::OnConsoleMessage(cp_, consoleMsg);              // keliatan semua sebagai chat
			VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, bubbleMsg, 0, 0); // bubble di atas kepala masing-masing
		}

		VarList::OnConsoleMessage(peer, "`2[ChatAllWorld] `oPesan berhasil dikirim ke semua player di world `w" + world + "``!");
	}

	static void CMD_CTITLE(ENetPeer* peer) {
		if (!peer || !peer->data) return;
		std::string d = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		d += "add_label_with_icon|big|`9Custom Title Manager|left|11816|\n";
		d += "add_spacer|small|\n";
		d += "add_text_input|new_title|Add New Title (supports color codes):||30|\n";
		d += "add_spacer|small|\n";
		d += "add_label_with_icon|small|`wYour Titles list:|left|11816|\n";
		
		int i = 0;
		for (const auto& title : pInfo(peer)->custom_titles) {
			bool is_active = (pInfo(peer)->custom_title == title && pInfo(peer)->show_custom_title);
			std::string active_str = is_active ? "`2[ACTIVE] " : "`9[INACTIVE] ";
			
			d += "add_checkbox|active_" + std::to_string(i) + "|" + active_str + title + "|" + (is_active ? "1" : "0") + "|\n";
			d += "add_checkbox|delete_" + std::to_string(i) + "|`4Delete Title: `o" + title + "|0|\n";
			d += "add_spacer|small|\n";
			i++;
		}
		if (pInfo(peer)->custom_titles.empty()) {
			d += "add_textbox|`9You don't have any custom titles yet. Create one above!|left|\n";
		}
		
		d += "end_dialog|custom_titles_manager|Cancel|Update/Save|\n";
		d += "add_quick_exit|\n";
		VarList::OnDialogRequest(peer, d);
	}

	static void DAILY_REWARD(ENetPeer* peer) {
		if (!peer || !peer->data) return;
		DailyReward_OpenDialog(peer);
	}
	static void SET_REWARD(ENetPeer* peer) {
		if (!peer || !peer->data) return;
		VarList::OnDialogRequest(peer, DailyReward_BuildEditDialog(peer));
	}
	static void EXCHANGE_ITEMS(ENetPeer* peer) {
		std::string fish = "", crystal = "";
		for (int i_ = 0; i_ < items.size(); i_++) {
			int item_id = items[i_].id;
			if (items[item_id].blockType == BlockTypes::FISH) {
				fish += "\nadd_button_with_icon|item_" + to_string(item_id) + "|" + items[item_id].name + "|staticPurpleFrame|" + to_string(item_id) + "||\n";
			}
			if (items[item_id].blockType == BlockTypes::CRYSTAL) {
				crystal += "\nadd_button_with_icon|item_" + to_string(item_id) + "|" + items[item_id].name + "|staticPurpleFrame|" + to_string(item_id) + "||\n";
			}
		}
		VarList::OnDialogRequest(peer, "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wExchange Items|left|9922|\nadd_spacer|small|\nadd_textbox|`wGreetings, Traveler! Welcome to the Exchange Place. I am here to assist you in Trading your treasures for valuable rewards<CR>If you'd like to strike a dead, simply browse the list below and choose the item you wish to offer. I'll ensure you get a fair trade in return!|left|\nadd_spacer|small|\nadd_textbox|`wItems available for exchange:|left|\nadd_spacer|small|\nadd_textbox|`5Fish:|left|\ntext_scaling_string|Subscribtions++++++++|\nadd_spacer|small|" + fish + "\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nadd_textbox|`5Crystal:|left|\nadd_spacer|small|" + crystal + "\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nend_dialog|Exchange_Npc|Close||", 700);
	}
	static void SURGERY_STAT(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSurgery Stats``|left|1270|\nadd_spacer|small|\nadd_textbox|`oSurgery Skill:`` `2" + to_string(pInfo(peer)->surgery_skill) + "``    `oSurgery Completed:`` `2" + to_string(pInfo(peer)->surgery_done) + "``|left|\nadd_spacer|small|\nadd_textbox|`oRoad to Angel of Mercy:`` `2" + to_string(pInfo(peer)->su_8552_1) + "/2000``    `oRoad to Dr.Title:`` `2" + to_string(pInfo(peer)->su_8552_2) + "/3000``    `oRoad to Legend title:`` `2" + to_string(pInfo(peer)->su_8552_3) + "/50000``|left|\nadd_spacer|small|\nadd_button|close|`wThanks!``|noflags|0|0|end_dialog|generic|||");
	}
	static void SHOP(ENetPeer* peer) {
		Commands::SHOP_NEW(peer);
	}
	static void HIDEUI(ENetPeer* peer) {
		CAction::ScreenShotMode(peer);
	}
	static void STATUS(ENetPeer* peer) {
		std::string recently_visited = "";
		for (auto it = pInfo(peer)->last_visited_worlds.rbegin(); it != pInfo(peer)->last_visited_worlds.rend(); ++it) {
			std::string a_ = *it + (next(it) != pInfo(peer)->last_visited_worlds.rend() ? "``, " : "``");
			recently_visited += "`#" + a_;
		}
		VarList::OnConsoleMessage(peer, get_player_nick(peer) + "'s Status:\nCurrent world: `w" + pInfo(peer)->world + "`` (`w" + to_string(pInfo(peer)->x / 32) + "``, `w" + to_string(pInfo(peer)->y / 32) + "``) (`w" + to_string(ServerPool::PlayerCountWorld(pInfo(peer)->world)) + "`` people)\nBackpack slots: `w" + to_string(pInfo(peer)->inv.size() - 1) + "``\nLast visited: " + recently_visited + form_mods(pInfo(peer), 1) + "");
	}
	static void SDB(ENetPeer* peer) {
		// Cek shared SB cooldown (playmod 122)
		if (Playmods::HasById(pInfo(peer), 122)) {
			int time_ = 0;
			for (PlayMods peer_playmod : pInfo(peer)->playmods) {
				if (peer_playmod.id == 122) {
					time_ = peer_playmod.time - time(nullptr);
					break;
				}
			}
			VarList::OnTalkBubble(peer, pInfo(peer)->netID,
				"`4SDB cooldown! " + Time::Playmod(time_) + " before you can broadcast again.", 0, 1);
			return;
		}
		// Cooldown: 10 menit (600 detik)
		long long now_sdb = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
		constexpr long long SDB_COOLDOWN_MS = 600000LL; // 10 menit
		if (pInfo(peer)->last_sdb_cmd_time + SDB_COOLDOWN_MS > now_sdb) {
			long long rem_sec = (pInfo(peer)->last_sdb_cmd_time + SDB_COOLDOWN_MS - now_sdb) / 1000 + 1;
			long long rem_min = rem_sec / 60;
			long long rem_s = rem_sec % 60;
			std::string rem_str = (rem_min > 0)
				? std::to_string(rem_min) + " minute" + (rem_min > 1 ? "s" : "") +
				(rem_s > 0 ? " " + std::to_string(rem_s) + "s" : "")
				: std::to_string(rem_sec) + " second" + (rem_sec > 1 ? "s" : "");
			VarList::OnTalkBubble(peer, pInfo(peer)->netID,
				"`4SDB cooldown! Wait " + rem_str + " before sending again.", 0, 1);
			return;
		}
		pInfo(peer)->last_sdb_cmd_time = now_sdb;
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`2Super Duper Broadcast|left|660|\nadd_textbox|Super Duper Broadcasts pop up a box with your message on every player's screen. The box even includes a button to visit your world! It costs 200 Growtokens to send.|left|\nadd_spacer|small|\nadd_smalltext|Enter up to 3 lines of text!|left|\nadd_text_input|sdb_1|||128|\nadd_text_input|sdb_2|||128|\nadd_text_input|sdb_3|||128||\nadd_spacer|small|\nend_dialog|sdbsend|Cancel|Send|");
	}
	static void STATS(ENetPeer* peer) {
		const char* months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
		struct tm newtime;
		time_t now = time(0);
#ifdef _WIN32
		localtime_s(&newtime, &now);
#elif defined(__linux__)
		localtime_r(&now, &newtime);
#endif
		std::string month = months[newtime.tm_mon];
		int add_amount = 0, total_online = 0, total_pc = 0, total_ios = 0, total_android = 0, total_ping = 0, total_peers = 0, total_packet_loss = 1;
		auto start = high_resolution_clock::now();
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			enet_peer_ping(cp_);
			total_ping += cp_->roundTripTime;
			total_packet_loss += cp_->packetLoss;
			add_amount = 1;
			total_online += add_amount;
			if (pInfo(cp_)->player_device == "0,1,1")total_pc += add_amount;
			else if (pInfo(cp_)->player_device == "1") total_ios += add_amount;
			else total_android += add_amount;
		}
		double cpuUsage = GetCpuUsage();
		int average_ping = (total_peers > 0) ? total_ping / total_peers : 0, average_packet_loss = (total_peers > 0) ? total_packet_loss / total_peers : 0;
		auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
		VarList::OnConsoleMessage(peer, "S1 Uptime: " + Time::Playmod(time(nullptr) - Environment()->UpTime) + " - `$" + Set_Count(ServerPool::PlayerCountServer()) + "`` players on.  Stats for this node: `$" + Set_Count(ServerPool::PlayerCountServer()) + "`` players. (" + to_string(total_pc) + " PC, " + to_string(total_android) + " Android, " + to_string(total_ios) + " iOS) and `$" + to_string(worlds.size()) + "`` Worlds active. Server Load: " + to_string(duration.count()) + "``\n`2Growtopia Time (EDT/UTC-5): " + month + " " + to_string(today_day) + "th, " + to_string(newtime.tm_hour) + ":" + to_string(newtime.tm_min) + "");
		VarList::OnConsoleMessage(peer, "Average Ping: " + to_string(average_ping) + " ms, Packet Loss: " + to_string(average_packet_loss) + "%" + " CPU Usage :" + to_string(cpuUsage) + "%");
	}
	static void TOP(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n\nadd_label_with_icon|big|`oWorld Rankings``|left|394|\nadd_spacer|\nadd_textbox|World Rankings are based on unique daily visitors. Where do your worlds stack up?|\nadd_button|toprated|`8Top Rated Worlds``|noflags|0|0|\nadd_button|toplist|`8Top worlds today``|noflags|0|0|\nadd_button|yesterdaylist|`5Top worlds yesterday``|noflags|0|0|\nadd_button|overalllist|`0Top worlds overall``|noflags|0|0|\nadd_button|wotd|`$World of the Day winners``|noflags|0|0|\nend_dialog|top|Close||\n");
	}
	static void TIME(ENetPeer* peer) {
		const char* months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
		struct tm newtime;
		time_t now = time(0);
#ifdef _WIN32
		localtime_s(&newtime, &now);
#elif defined(__linux__)
		localtime_r(&now, &newtime);
#endif
		VarList::OnConsoleMessage(peer, "`2Growtopia Time (EDT/UTC-5): " + a + months[newtime.tm_mon] + " " + to_string(newtime.tm_mday) + "th, " + (newtime.tm_hour < 10 ? "0" + to_string(newtime.tm_hour) + "" : "" + to_string(newtime.tm_hour) + "") + ":" + (newtime.tm_min < 10 ? "0" + to_string(newtime.tm_min) + "" : "" + to_string(newtime.tm_min) + "") + ".");
	}
	static void WHO(ENetPeer* peer) {
		std::string who = "`oWho's in `$" + pInfo(peer)->world + "``:`` " + get_player_nick(peer);
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->tankIDName == pInfo(peer)->tankIDName or pInfo(cp_)->invis or pInfo(cp_)->world != pInfo(peer)->world) continue;
			who += ", " + get_player_nick(cp_);
			VarList::OnTalkBubble(peer, pInfo(cp_)->netID, get_player_nick(cp_), 1, 1);
			VarList::OnConsoleMessage(peer, who);
		}
	}
	static void RADIO(ENetPeer* peer) {
		pInfo(peer)->radio = (pInfo(peer)->radio == 0 ? 1 : 0);
		VarList::OnConsoleMessage(peer, "Radio " + a + (pInfo(peer)->radio ? "disabled, `4you now won't" : "Radio enabled, `2you can now") + a + " see`` public broadcasts.");
	}
	static void MODS(ENetPeer* peer) {
		std::string mods = "", a = "";
		int count = 0;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL || pInfo(cp_)->m_h || pInfo(cp_)->invis) continue;
			if (Role::Moderator(cp_) || Role::Administrator(cp_) || Role::Developer(cp_)) {
				count++;
				mods += "" + a + (count == 1 ? "" : ", ") + "" + pInfo(cp_)->name_color + pInfo(cp_)->tankIDName + "``";
			}
		}
		VarList::OnConsoleMessage(peer, "Mods online: " + (mods != "" ? mods : "(All are hidden)") + "`o.``");
	}
	static void UBA(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (world_->owner_name.empty() || to_lower(world_->owner_name) != to_lower(pInfo(peer)->tankIDName) and not Role::Administrator(peer) and (!guild_access(peer, world_->guild_id) and find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(peer)->tankIDName)) == world_->admins.end())) {
				VarList::OnConsoleMessage(peer, "You don't have access to the world lock, so you can't unban all!");
				return;
			}
			world_->bannedPlayers.clear();
			VarList::OnConsoleMessage(peer, "CP:_PL:0_OID:_CT:[S]_ You've unbanned everybody from this world.");
		}
	}
	static void GO(ENetPeer* peer) {
		std::string enter_world = Environment()->Sb_World;
		if (enter_world == pInfo(peer)->world || enter_world.empty()) {
			VarList::OnTextOverlay(peer, "Unable to track down the location of the last broadcast!");
		}
		else if (enter_world.find("GROWMINES") != std::string::npos) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "can't enter this world...", 0, 0);
		}
		else {
			VarList::OnTextOverlay(peer, "Moving to location of the last broadcast (`2" + enter_world + "``) ...");
			CAction::Positioned(peer, pInfo(peer)->netID, "audio/object_spawn.wav", 0);
			Enter_World(peer, enter_world);
			Clothing_V2::Update(peer);
		}
	}
	static void RGO(ENetPeer* peer) {
		if (pInfo(peer)->lastmsgworld == pInfo(peer)->world) {
			VarList::OnConsoleMessage(peer, "You magically appear at the world you were already in.  Great job!");
			return;
		}
		if (pInfo(peer)->lastmsgworld.empty()) {
			VarList::OnTextOverlay(peer, "Unable to track down the location of the last broadcast!");
			return;
		}
		if (pInfo(peer)->lastmsgworld.find("GROWMINES") != std::string::npos) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "can't enter this world...", 0, 0);
			return;
		}
		VarList::OnTextOverlay(peer, "Moving to where `w/" + pInfo(peer)->lastmsg + "`` sent that last `5/msg`` from... (`2" + pInfo(peer)->lastmsgworld + "``) ...");
		CAction::Positioned(peer, pInfo(peer)->netID, "audio/object_spawn.wav", 0);
		Enter_World(peer, pInfo(peer)->lastmsgworld);
		Clothing_V2::Update(peer);
	}
	static void HUB(ENetPeer* peer) {
		std::string enter_world = "TRADE";
		VarList::OnTextOverlay(peer, "Moving to location of the Global Trading World (`2TRADE`) ...");
		CAction::Positioned(peer, pInfo(peer)->netID, "audio/object_spawn.wav", 0);
		Enter_World(peer, enter_world);
		Clothing_V2::Update(peer);
	}
	static void KICKALL(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (to_lower(world_->owner_name) != to_lower(pInfo(peer)->tankIDName) and not Role::Administrator(peer) and (!guild_access(peer, world_->guild_id) and find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(peer)->tankIDName)) == world_->admins.end())) {
				VarList::OnTextOverlay(peer, "Can't `4kick``, is not in a locked area you control!");
				return;
			}
			if (not Role::Administrator(peer) && world_->kickall + 600000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
				VarList::OnConsoleMessage(peer, "You can use `5/kickall`` again in few minutes.");
			}
			else {
				world_->kickall = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->world != pInfo(cp_)->world or pInfo(peer)->tankIDName == pInfo(cp_)->tankIDName or Role::Administrator(cp_)) continue;
					pInfo(cp_)->c_x = 0, pInfo(cp_)->c_y = 0;
					Player_Respawn(cp_, true, 0, 1);
					VarList::OnConsoleMessage(cp_, "`4(KICKALL ACTIVATED!)``");
				}
			}
		}
	}
	static void UNACCESS(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|small|Remove Your Access From World|left|242|\nadd_textbox|Are you sure you want to remove yourself from all locks in this world?|left|\nadd_textbox|Any locks you placed will still be owned by you, but you will be removed from any other locks that you have access on.|left|\nadd_spacer|small|\nend_dialog|unaccess|No|Yes|");
	}
	static void REPORT(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wReport this world as a scam``|left|3732|\nadd_textbox|If this world is being used to scam or bully players, you can press `3Report`` to flag it for moderators to check.|left|\nadd_smalltext|- This feature is for reporting the `2world``, not the players. If the world is fine, but people are being inappropriate, send a /msg to a mod instead.|left|\nadd_smalltext|- We record who uses this feature. You will be banned if you file false reports.|left|\nadd_smalltext|- Reporting multiple times doesn't do anything - just report once, and the world will be on our list to check.|left|\nadd_smalltext|- There is no way to un-report, so don't report unless you are sure the world is bad!|left|\nadd_smalltext|- Provide a short 32 character description of why you are reporting the world below.|left|\nadd_text_input|report_reason|Reason:||32|\nadd_textbox|`1If you are sure you want to report this world as a scam, press Report below!``|left|\nend_dialog|worldreport|Cancel|Report|");
	}
	static void REPORT_BUG(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wReport Bug``|left|1432|\nadd_smalltext|A bug report contains dupe, crash, and lag diagnostic information to help you find and fix bugs in our Server. To capture a bug report from your device, use the commands '/reportbug'.|left|\nadd_spacer|small|\nadd_smalltext|Bug Name:|left|\nadd_text_input|report_name|||20|\nadd_smalltext|How does it work:|left|\nadd_text_box_input|howit|||500|2|\nend_dialog|report_bug|Cancel|Report Bug!|");
	}
	static void REDEEMCODE(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wRedeem Your Code!``|left|982|\nadd_spacer|small|\nadd_textbox|ABOUT CODE REDEMPTION|left|\nadd_smalltext|- Pay attention to the redemption conditions and validity period. A code cannot be redeemed after it expires.|left|\nadd_smalltext|- Each redemption code can only be used once.|left|\nadd_smalltext|- After successful code redemption, your redeemed item will be delivered to your Inventory. For issues, contact Owner Server.|left|\nadd_smalltext|- The Redemption Code has 10 characters, a combination of capital letters and numbers.|left|\nadd_spacer|small|\nadd_text_input|redeemcode|||16|\nadd_spacer|small|\nadd_button|cancel|`wCancel|noflags|\nadd_custom_button|redeem_code_button|textLabel:`wRedeem;anchor:_button_cancel;left:1;margin:40,0;|\nadd_spacer|small|\nend_dialog|redeem_code|||");
	}
	static void SECUREACCOUNT(ENetPeer* peer) {
		DialogHandle::Acc_Security(peer);
	}
	static void GWARP(ENetPeer* peer) {
		if (pInfo(peer)->guild_id == 0) {
			VarList::OnConsoleMessage(peer, "You are not in a Guild!");
		}
		else {
			std::uint32_t guild_id = pInfo(peer)->guild_id;
			std::vector<Guild>::iterator pz = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
			if (pz != guilds.end()) {
				Guild* guild_information = &guilds[pz - guilds.begin()];
				pInfo(peer)->update = true;
				VarList::OnConsoleMessage(peer, "Magically warping to world `5" + guild_information->guild_world + "``...");
				Enter_World(peer, guild_information->guild_world);
			}
		}
	}
	static void GUILD(ENetPeer* peer) {
		std::string guild_info = get_guild_info(peer);
		if (guild_info == "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wGrow Guild ``|left|5814|\nadd_textbox|Something went wrong!|left|\nend_dialog|guildalreadyjoined|Close||") set_Guilds(peer);
		VarList::OnDialogRequest(peer, guild_info);
	}
	static void ROLES(ENetPeer* peer) {
		Role_Quest::Dialog(peer);
	}
	static void FAV(ENetPeer* peer) {
		if (pInfo(peer)->worlds_favorite.size() < 10) {
			if (find(pInfo(peer)->worlds_favorite.begin(), pInfo(peer)->worlds_favorite.end(), pInfo(peer)->world) != pInfo(peer)->worlds_favorite.end()) {
				pInfo(peer)->worlds_favorite.erase(remove(pInfo(peer)->worlds_favorite.begin(), pInfo(peer)->worlds_favorite.end(), pInfo(peer)->world), pInfo(peer)->worlds_favorite.end());
				VarList::OnConsoleMessage(peer, "`oWorld " + pInfo(peer)->world + " has been removed from your favorite worlds. Now you have " + to_string(pInfo(peer)->worlds_favorite.size()) + "/10.");
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "World " + pInfo(peer)->world + " has been removed from your favorite worlds. Now you have " + to_string(pInfo(peer)->worlds_favorite.size()) + "/10.", 0, 1);
			}
			else {
				pInfo(peer)->worlds_favorite.push_back(pInfo(peer)->world);
				VarList::OnConsoleMessage(peer, "`oWorld " + pInfo(peer)->world + " has been added to your favorite worlds. Now you have " + to_string(pInfo(peer)->worlds_favorite.size()) + "/10.");
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "World " + pInfo(peer)->world + " has been added to your favorite worlds. Now you have " + to_string(pInfo(peer)->worlds_favorite.size()) + "/10.", 0, 1);
			}
		}
		else {
			VarList::OnConsoleMessage(peer, "`oYou have used all your favorite worlds limit, try un-favorite some.");
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have used all your favorite worlds limit, try un-favorite some.", 0, 1);
		}
	}
	static void FAV_LIST(ENetPeer* peer) {
		std::string fav_list = "";
		for (int w_ = 0; w_ < pInfo(peer)->worlds_favorite.size(); w_++) {
			fav_list += pInfo(peer)->worlds_favorite[w_] + " ";
		}
		VarList::OnConsoleMessage(peer, "`oYour favorite worlds: " + fav_list);
	}
	static void GLOBAL_HIS(ENetPeer* peer) {
		std::string trade_history = "";

		if (globalTradeHistory.empty()) {
			trade_history = "\nadd_spacer|small|\nadd_smalltext|Nothing to show yet.|left|";
		}
		else {
			for (const auto& trade : globalTradeHistory) {
				char timeBuffer[80];
				std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", std::localtime(&trade.timestamp));

				trade_history += "\nadd_spacer|small|\nadd_smalltext|`2" + trade.seller + "`` traded `9" +
					std::to_string(trade.quantity) + " " + trade.item + "`` to `2" + trade.buyer +
					"`` at `o" + timeBuffer + "|left|";
			}
		}

		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`2GrowRabid Global Trade History|left|13816|\n"
			"add_smalltext|`4Disclaimer: `oRisk of Manipulation from players using `wAlts ``And `wSpamming Trades `` is possible.|left|\n"
			"add_smalltext|if we catch a player strongly spamming Trades and filling the Global Trade History to `5manipulate``, we will `4Ban ``the. Also, if they are in a world (ex.:`5BUY``), you can `4Vote-Ban ``them from the world.|left|\n"
			"add_smalltext|For more protection, we only record Trades from `eDiscord Verified Players``, but there is `&NO GUARANTEE``. Read the Global Trade History `4**at your own risk** `o!|left|\n" +
			"add_spacer|small|\n" +
			"add_textbox|Showing all `2Global `9Trade `7History `osince last restart, Server Uptime (" + Time::Playmod(time(nullptr) - Environment()->UpTime) + ")|left|" +
			trade_history +
			"\nadd_spacer|small|\nadd_button|back|Back|noflags|0|0|\nadd_button||Close|noflags|0|0|\nadd_quick_exit|\nend_dialog|friends|||";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void HOME(ENetPeer* peer) {
		if (pInfo(peer)->home_world.empty()) {
			VarList::OnConsoleMessage(peer, "You haven't set a home world yet!");
			return;
		}
		for (std::pair<std::string, long long int> p : Environment()->Home_Timed) {
			if (p.first == pInfo(peer)->tankIDName) {
				if (p.second + HOME_TIME > (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
					VarList::OnConsoleMessage(peer, "`4Cooldown active`` - You can use `5/home`` in `5" + to_string(((p.second + HOME_TIME - (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) / 1000) - ((((p.second + HOME_TIME - (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) / 1000) / 60) * 60)) + " ``seconds.");
					return;
				}
			}
		}
		Environment()->Home_Timed.push_back(make_pair(pInfo(peer)->tankIDName, (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()));
		VarList::OnConsoleMessage(peer, "Magically warping to home world `5" + pInfo(peer)->home_world + "``...");
		Exit_World(peer, true);
		Enter_World(peer, pInfo(peer)->home_world);
	}
	static void SET_HOME(ENetPeer* peer) {
		if (pInfo(peer)->home_world != "") {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, pInfo(peer)->home_world + " has been removed as your home world!", 0, 0);
			pInfo(peer)->home_world = "";
		}
		else if (pInfo(peer)->home_world.empty()) {
			pInfo(peer)->home_world = pInfo(peer)->world;
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, pInfo(peer)->world + " has been set as your home world!", 0, 0);
		}
	}
	static void JANEWAY(ENetPeer* peer) {
		DialogHandle::Janeway(peer);
	}
	static void GROWPASS(ENetPeer* peer) {
		DialogHandle::GrowPass(peer);
	}
	static void RESPAWN(ENetPeer* peer) {
		if (pInfo(peer)->WarnRespawn == 5) {// Anti Spam Respawn By LyChie
			VarList::OnConsoleMessage(peer, "`4SPAM RESPAWN DETECTED!`oPleasee Don't Spam Respawn, have a nice day.");
		}
		using namespace std::chrono;
		if (pInfo(peer)->lastRESP + 1800 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
			pInfo(peer)->lastRESP = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
		}
		else pInfo(peer)->WarnRespawn++;
		Player_Respawn(peer, false, 0, true);
	}
	static void TRADE_SCAN(ENetPeer* peer) {
		DialogHandle::Trade_Scan(peer);
	}
	static void TIPS(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|Tips for New Players|left|18|\nadd_textbox|`oI will show you how to profit in " + Environment()->server_name + "|left|\nadd_spacer|small|\nadd_label_with_icon|small|Break for Gems (BFG)|left|5638|\nadd_smalltext|`oYou can BFG and get extra gems, You can go to world BFG to rent a room. in this world we provide a Pot'o Gems which produces a lot of gems.|left|\nadd_spacer|small|\nadd_label_with_icon|small|Guild Event|left|6012|\nadd_smalltext|`oTo do this, you will need to join a guild to contribute to ongoing events.|left|\nadd_smalltext|`oAfter joining the guild, you can immediately take part in existing events, using the procedures as explained in the events.|left|\nadd_spacer|small|\nadd_label_with_icon|small|ATM Machine|left|1008|\nadd_smalltext|`oYou can farm using an ATM Machine and if the Owner-Server holds a Gems Event, you can get more gems there.|left|\nadd_spacer|small|\nadd_label_with_icon|small|Giveaway|left|10716|\nadd_smalltext|`oYou can join Daily Giveaway because " + Environment()->server_name + " Giveaway 3 times a week.|left|\nadd_spacer|small|\nend_dialog||Thanks for info!||\nadd_quick_exit|");
	}
	static void EVENT(ENetPeer* peer) {
		VarList::OnConsoleMessage(peer, "`o" + a + (not Event()->Gems_Event and not Event()->Xp_Event and not Event()->Exchange_Event ? "There is no events." : Event()->Gems_Event ? "Gems Event: [" + to_string(Event()->X_Gems) + "x] Time: [" + Time::Gacha(Event()->x_gems_time - time(nullptr)) + "]" : "") + (Event()->Xp_Event ? "\nXp Event: [" + to_string(Event()->X_Xp) + "x] Time: [" + Time::Gacha(Event()->x_xp_time - time(nullptr)) + "]" : "") + (Event()->Exchange_Event ? "\nExchange Event: [" + to_string(Event()->X_Exchange) + "x] Time: [" + Time::Gacha(Event()->x_exchange_time - time(nullptr)) + "]" : ""));
	}
	static void VIPS(ENetPeer* peer) {
		string vips = "", a = "";
		int count = 0;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL || pInfo(cp_)->m_h) continue;
			if (Role::Vip(cp_)) {
				count++;
				vips += a + (count == 1 ? "" : ", ") + "`0" + pInfo(cp_)->tankIDName + "``";
			}
		}
		VarList::OnConsoleMessage(peer, "Vips online: " + (vips != "" ? vips : "(All are hidden)") + "`o.``");
	}
	static void DEPOSIT(ENetPeer* peer) {
		DialogHandle::Deposit(peer);
	}
	static void MODRULES(ENetPeer* peer) {
		DialogHandle::ModRules(peer);
	}
	static void CSNRULES(ENetPeer* peer) {
		DialogHandle::CsnRules(peer);
	}
	static void RULES(ENetPeer* peer) {
		DialogHandle::Rules(peer);
	}
	static void NEWS(ENetPeer* peer) {
		DialogHandle::News(peer);
	}
	static void HOWGAY(ENetPeer* peer) {
		if (Playmods::HasById(pInfo(peer), 11)) {
			VarList::OnConsoleMessage(peer, "6>> That's sort of hard to do while duct-taped.`");
			return;
		}
		int rate = rand() % 101;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (!cp_ || cp_->state != ENET_PEER_STATE_CONNECTED || !cp_->data) continue;
			if (pInfo(peer)->world != pInfo(cp_)->world) continue;
			VarList::OnConsoleMessage(cp_, "`cGay Detector: `0" + pInfo(peer)->tankIDName + " is `5" + std::to_string(rate) + "`%`0 gay");
			VarList::OnTalkBubble(cp_, pInfo(peer)->netID, "`cGay Detector: `" + pInfo(peer)->tankIDName + " is `5" + std::to_string(rate) + "`%`0 gay", 0, 0);
		}
	}
	static void KHODAM(ENetPeer* peer) {
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or pInfo(peer)->world != pInfo(cp_)->world) continue;
			string khodamList = GenerateRandomKhodam(pInfo(peer)->tankIDName);
			VarList::OnTalkBubble(cp_, pInfo(peer)->netID, pInfo(peer)->tankIDName + " Khodam is: [" + khodamList + "]", 0, 0);
			VarList::OnConsoleMessage(cp_, pInfo(peer)->tankIDName + " Your Khodam is: [" + khodamList + "]");
		}
	}
	static void WIPE_WORLDS(ENetPeer* peer) {
		sort(pInfo(peer)->worlds_owned.begin(), pInfo(peer)->worlds_owned.end());
		pInfo(peer)->worlds_owned.erase(unique(pInfo(peer)->worlds_owned.begin(), pInfo(peer)->worlds_owned.end()), pInfo(peer)->worlds_owned.end());
		VarList::OnConsoleMessage(peer, "Removed worlds that you don't own from your list.");
	}
	static void SPOTIFY(ENetPeer* peer) {
		std::string t = pInfo(peer)->page_type;
		std::string p = pInfo(peer)->page_music;
		if (t.empty()) t = "eng";
		if (p.empty()) p = "1";
		DialogHandle::Spotify_Menu(peer, t, p);
	}
	static void CLEAR_CHAT(ENetPeer* peer) {
		VarList::OnConsoleMessage(peer, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
		VarList::OnTextOverlay(peer, "`wCleared chat!");
	}
	static void SERVER_INFO(ENetPeer* peer) {
		DialogHandle::Server_Info(peer);
	}
	static void PETS(ENetPeer* peer) {
		if (not pInfo(peer)->LiyueHtouFly and not pInfo(peer)->AbyssMageFly) {
			VarList::OnConsoleMessage(peer, "`oYou don't have pets! Buy Pets from Store");
		}
		else {
			if (pInfo(peer)->show_pets) {
				pInfo(peer)->pet_ID = 0;
				pInfo(peer)->show_pets = false;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (pInfo(peer)->world == pInfo(cp_)->world) {
						VarList::OnKilled(cp_, pInfo(peer)->pet_netID, 0);
						VarList::OnRemove(cp_, pInfo(peer)->pet_netID, 0, 500);
					}
				}
			}
			else {
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wCustom Pet|left|9474|\nadd_smalltext|`oThe conversation around Pet AI may have slowed down due to various factors, including ethical concerns about AI's impact on real animals, the novelty wearing off, or shifts in public interest towards other technological advancements. Additionally, discussions about AI's role in pet care and companionship continue, but they may not be as prominent in the media as before.|left|\nadd_spacer|small|\nadd_textbox|`$Choose your own Pet!|left|\nadd_custom_margin|x:10;y:0|\ntext_scaling_string|Subscribtions++++++++|" + a + (pInfo(peer)->LiyueHtouFly ? "\nadd_custom_button|hutao_pet|icon:9502;border:white;display:inline_free;width:0.40;|" : "") + (pInfo(peer)->AbyssMageFly ? "\nadd_custom_button|abyssmage_pet|icon:9504;border:white;display:inline_free;width:0.40;|" : "") + "\nadd_custom_break|\nadd_spacer|small|\nend_dialog|My_PetAi|Close||");
			}
			ServerPool::SaveDatabase::Players(pInfo(peer), false);
		}
	}
	static void WL(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (to_lower(world_->owner_name) == to_lower(pInfo(peer)->tankIDName)) {
				std::vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [&](const WorldBlock& a) { return items[a.fg].blockType == BlockTypes::LOCK && a.fg != 202 && a.fg != 204 && a.fg != 206 && a.fg != 4994 && a.fg != 10000; });
				if (p != world_->blocks.end()) {
					int x_ = int(p - world_->blocks.begin()) % 100, y_ = int(p - world_->blocks.begin()) / 100;
					DialogHandle::Lock(peer, world_, world_->blocks[x_ + (y_ * 100)].fg, x_, y_);
				}
			}
		}
	}
	static void FIXMYGEMS(ENetPeer* peer) {
		if (to_string(pInfo(peer)->gems).find("-") != std::string::npos) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "The system has successfully repaired/fix your gems!");
			pInfo(peer)->gems = 0;
			gamepacket_t p;
			p.Insert("OnSetBux");
			p.Insert(0), p.Insert(0), p.Insert((pInfo(peer)->supp >= 1 || pInfo(peer)->subscriber ? 1 : 0));
			if (pInfo(peer)->supp >= 2 || pInfo(peer)->subscriber) p.Insert((float)33796, (float)1, (float)0);
			p.CreatePacket(peer);
		}
		else VarList::OnTalkBubble(peer, pInfo(peer)->netID, "The system does not detect errors/minus in your gems!");
	}
	static void TF_BANK(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /tf <player name> <type> <count>");
			return;
		}
		std::istringstream ss(cmd);
		std::string playerName, type;
		int quantity;
		try {
			if (ss >> playerName >> type >> quantity) {
				if (to_lower(type) == "wl" or to_lower(type) == "dl" or to_lower(type) == "bgl" or to_lower(type) == "mgl" or to_lower(type) == "ggl" or to_lower(type) == "igl" or to_lower(type) == "gems") {

				}
				else {
					VarList::OnConsoleMessage(peer, "`oInvalid input type! Please input (WL, DL, BGL, MGL, IGL, GEMS)");
					return;
				}
				if (type == "" || quantity <= 0) {
					VarList::OnConsoleMessage(peer, (quantity <= 0 ? "`oQuantity must be more than 0!" : "Please input type (WL, DL, BGL, MGL, IGL, GEMS)"));
					return;
				}
				bool found = false;
				for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
					if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
					if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(playerName)) {
						found = true;
						int itemID = 0;
						// [ANTI-DUPE] Validasi balance sebelum transfer, cegah overdraft/negative balance
						if (to_lower(type) == "wl") {
							itemID = 242;
							if (quantity > pInfo(peer)->wl_bank_amount) {
								VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Insufficient WL balance in bank!", 0, 1);
								return;
							}
							pInfo(peer)->wl_bank_amount -= quantity;
							pInfo(currentPeer)->wl_bank_amount += quantity;
						}
						else if (to_lower(type) == "dl") {
							itemID = 1796;
							if (quantity > pInfo(peer)->dl_bank_amount) {
								VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Insufficient DL balance in bank!", 0, 1);
								return;
							}
							pInfo(peer)->dl_bank_amount -= quantity;
							pInfo(currentPeer)->dl_bank_amount += quantity;
						}
						else if (to_lower(type) == "bgl" || to_lower(type) == "sgl") {
							itemID = 7188;
							if (quantity > pInfo(peer)->bgl_bank_amount) {
								VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Insufficient SGL balance in bank!", 0, 1);
								return;
							}
							pInfo(peer)->bgl_bank_amount -= quantity;
							pInfo(currentPeer)->bgl_bank_amount += quantity;
						}
						else if (to_lower(type) == "mgl" || to_lower(type) == "ggl") {
							itemID = 8470;
							if (quantity > pInfo(peer)->ggl_bank_amount) {
								VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Insufficient MGL balance in bank!", 0, 1);
								return;
							}
							pInfo(peer)->ggl_bank_amount -= quantity;
							pInfo(currentPeer)->ggl_bank_amount += quantity;
						}
						else if (to_lower(type) == "igl") {
							itemID = 20298;
							if (quantity > pInfo(peer)->mgl_bank_amount) {
								VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Insufficient IGL balance in bank!", 0, 1);
								return;
							}
							pInfo(peer)->mgl_bank_amount -= quantity;
							pInfo(currentPeer)->mgl_bank_amount += quantity;
						}
						else if (to_lower(type) == "gems") {
							itemID = 112;
							if (quantity > pInfo(peer)->Gems_Storage) {
								VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Insufficient Gems balance in bank!", 0, 1);
								return;
							}
							pInfo(peer)->Gems_Storage -= quantity;
							pInfo(currentPeer)->Gems_Storage += quantity;
						}
						VarList::OnConsoleMessage(peer, "Working...");
						VarList::OnConsoleMessage(currentPeer, "You receive a transfer in the form of " + Set_Count(quantity) + " " + items[itemID].name + " from " + pInfo(peer)->tankIDName + "");
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Transfer your " + Set_Count(quantity) + " " + items[itemID].name + " to " + pInfo(currentPeer)->tankIDName + ".", 0, 1);
						pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Transfer " + Set_Count(quantity) + " " + items[itemID].name + " to " + pInfo(currentPeer)->tankIDName + "");
						PlayerMoving data_{};
						data_.packetType = 19, data_.plantingTree = 500, data_.netID = pInfo(currentPeer)->netID;
						data_.punchX = itemID, data_.punchY = itemID;
						int32_t to_netid = pInfo(peer)->netID;
						BYTE* raw = packPlayerMoving(&data_);
						raw[3] = 3;
						Memory_Copy(raw + 8, &to_netid, 4);
						for (ENetPeer* cp_2 = server->peers; cp_2 < &server->peers[server->peerCount]; ++cp_2) {
							if (cp_2->state != ENET_PEER_STATE_CONNECTED or cp_2->data == NULL) continue;
							if (pInfo(cp_2)->world == pInfo(peer)->world) {
								send_raw(cp_2, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
							}
						}
						delete[]raw;
					}
				}
				if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
			}
			else {
				VarList::OnConsoleMessage(peer, "`oUsage: /tf <player name> <type> <count>");
				return;
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /tf <player name> <type> <count>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /tf <player name> <type> <count>");
		}
	}
	static void DEPOSIT_WL(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpwl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int rem = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > Inventory::Contains(peer, 242)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough World Locks to Deposit", 0, 1);
				return;
			}
			if (Inventory::Modify(peer, 242, rem -= amount) == 0) {
				pInfo(peer)->wl_bank_amount += amount;
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Deposit your World Lock to Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Deposit " + to_string(amount) + " World Lock to Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpwl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpwl <amount>");
		}
	}
	static void DEPOSIT_DL(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpdl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int rem = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > Inventory::Contains(peer, 1796)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Diamond Locks to Deposit", 0, 1);
				return;
			}
			if (Inventory::Modify(peer, 1796, rem -= amount) == 0) {
				pInfo(peer)->dl_bank_amount += amount;
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Deposit your Diamond Lock to Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Deposit " + to_string(amount) + " Diamond Lock to Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpdl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpdl <amount>");
		}
	}
	static void DEPOSIT_bgl(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpbgl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int rem = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > Inventory::Contains(peer, 7188)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Silver Gem Lock to Deposit", 0, 1);
				return;
			}
			if (Inventory::Modify(peer, 7188, rem -= amount) == 0) {
				pInfo(peer)->bgl_bank_amount += amount;
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Deposit your Silver Gem Lock to Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Deposit " + to_string(amount) + " Silver Gem Lock to Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpbgl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpbgl <amount>");
		}
	}
	static void DEPOSIT_mgl(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpmgl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int rem = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > Inventory::Contains(peer, 8470)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Magical Gem Lock to Deposit", 0, 1);
				return;
			}
			if (Inventory::Modify(peer, 8470, rem -= amount) == 0) {
				pInfo(peer)->ggl_bank_amount += amount;
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Deposit your Magical Gem Lock to Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Deposit " + to_string(amount) + " Magical Gem Lock to Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpmgl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpmgl <amount>");
		}
	}
	static void DEPOSIT_igl(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpigl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int rem = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > Inventory::Contains(peer, 20298)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Infinity Gem Lock to Deposit", 0, 1);
				return;
			}
			if (Inventory::Modify(peer, 20298, rem -= amount) == 0) {
				pInfo(peer)->mgl_bank_amount += amount;
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Deposit your Infinity Gem Lock to Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Deposit " + to_string(amount) + " Infinity Gem Lock to Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpigl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpigl <amount>");
		}
	}
	static void DEPOSIT_GEMS(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpgems <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int rem = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 2000000000) return;
			if (amount > pInfo(peer)->gems) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Gems to Deposit", 0, 1);
				return;
			}
			if (pInfo(peer)->gems >= amount) {
				pInfo(peer)->Gems_Storage += amount;
				VarList::OnMinGems(peer, amount);
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Deposit your " + Set_Count(amount) + " Gems to Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Deposit " + Set_Count(amount) + " Gems to Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpgems <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dpgems <amount>");
		}
	}
	static void WITHDRAW_WL(ENetPeer* peer, std::string cmd) {
		// [ANTI-SPAM] cooldown 1 detik antar operasi bank
		time_t now = time(nullptr);
		if (now - pInfo(peer)->bank_last_op < 1) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Too fast! Wait 1 second between bank operations.", 0, 1);
			return;
		}
		if (pInfo(peer)->bank_in_transaction) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Transaction in progress, please wait.", 0, 1);
			return;
		}
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdwl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int free_slots = Inventory::Get_Slots(pInfo(peer)), add = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > pInfo(peer)->wl_bank_amount) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough World Locks to Withdraw", 0, 1);
				return;
			}
			if (free_slots == 0) {
				VarList::OnConsoleMessage(peer, "You don't have room in your backpack!");
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have room in your backpack!", 0, 1);
				return;
			}
			if (Inventory::Check_Max(peer, 242, amount)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "That wouldnt fit into my inventory!", 0, true);
				return;
			}
			pInfo(peer)->bank_in_transaction = true;
			if (Inventory::Modify(peer, 242, add += amount) == 0) {
				pInfo(peer)->wl_bank_amount -= amount;
				if (pInfo(peer)->wl_bank_amount < 0) pInfo(peer)->wl_bank_amount = 0; // underflow guard
				pInfo(peer)->bank_last_op = time(nullptr);
				VisualHandle::Trade(peer, 242, 500);
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Withdraw your World Lock from Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Withdraw " + to_string(amount) + " World Lock from Bank Central");
				pInfo(peer)->bank_in_transaction = false;
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdwl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdwl <amount>");
		}
	}
	static void WITHDRAW_DL(ENetPeer* peer, std::string cmd) {
		time_t now = time(nullptr);
		if (now - pInfo(peer)->bank_last_op < 1 || pInfo(peer)->bank_in_transaction) { VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Wait before next bank operation.", 0, 1); return; }
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wddl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int free_slots = Inventory::Get_Slots(pInfo(peer)), add = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > pInfo(peer)->dl_bank_amount) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Diamond Locks to Withdraw", 0, 1);
				return;
			}
			if (free_slots == 0) {
				VarList::OnConsoleMessage(peer, "You don't have room in your backpack!");
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have room in your backpack!", 0, 1);
				return;
			}
			if (Inventory::Check_Max(peer, 1796, amount)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "That wouldnt fit into my inventory!", 0, true);
				return;
			}
			if (Inventory::Modify(peer, 1796, add += amount) == 0) {
				pInfo(peer)->dl_bank_amount -= amount;
				if (pInfo(peer)->dl_bank_amount < 0) pInfo(peer)->dl_bank_amount = 0;
				pInfo(peer)->bank_last_op = time(nullptr);
				VisualHandle::Trade(peer, 1796, 500);
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Withdraw your Diamond Lock from Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Withdraw " + to_string(amount) + " Diamond Lock from Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wddl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wddl <amount>");
		}
	}
	static void WITHDRAW_bgl(ENetPeer* peer, std::string cmd) {
		time_t now = time(nullptr);
		if (now - pInfo(peer)->bank_last_op < 1 || pInfo(peer)->bank_in_transaction) { VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Wait before next bank operation.", 0, 1); return; }
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdbgl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int free_slots = Inventory::Get_Slots(pInfo(peer)), add = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > pInfo(peer)->bgl_bank_amount) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Silver Gem Locks to Withdraw", 0, 1);
				return;
			}
			if (free_slots == 0) {
				VarList::OnConsoleMessage(peer, "You don't have room in your backpack!");
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have room in your backpack!", 0, 1);
				return;
			}
			if (Inventory::Check_Max(peer, 7188, amount)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "That wouldnt fit into my inventory!", 0, true);
				return;
			}
			if (Inventory::Modify(peer, 7188, add += amount) == 0) {
				pInfo(peer)->bgl_bank_amount -= amount;
				if (pInfo(peer)->bgl_bank_amount < 0) pInfo(peer)->bgl_bank_amount = 0;
				pInfo(peer)->bank_last_op = time(nullptr);
				VisualHandle::Trade(peer, 7188, 500);
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Withdraw your Silver Gem Lock from Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Withdraw " + to_string(amount) + " Silver Gem Lock from Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdbgl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdbgl <amount>");
		}
	}
	static void WITHDRAW_mgl(ENetPeer* peer, std::string cmd) {
		time_t now = time(nullptr);
		if (now - pInfo(peer)->bank_last_op < 1 || pInfo(peer)->bank_in_transaction) { VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Wait before next bank operation.", 0, 1); return; }
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdmgl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int free_slots = Inventory::Get_Slots(pInfo(peer)), add = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > pInfo(peer)->ggl_bank_amount) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Magical Gem Locks to Withdraw", 0, 1);
				return;
			}
			if (free_slots == 0) {
				VarList::OnConsoleMessage(peer, "You don't have room in your backpack!");
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have room in your backpack!", 0, 1);
				return;
			}
			if (Inventory::Check_Max(peer, 8470, amount)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "That wouldnt fit into my inventory!", 0, true);
				return;
			}
			if (Inventory::Modify(peer, 8470, add += amount) == 0) {
				pInfo(peer)->ggl_bank_amount -= amount;
				if (pInfo(peer)->ggl_bank_amount < 0) pInfo(peer)->ggl_bank_amount = 0;
				pInfo(peer)->bank_last_op = time(nullptr);
				VisualHandle::Trade(peer, 8470, 500);
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Withdraw your Magical Gem Lock from Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Withdraw " + to_string(amount) + " Magical Gem Lock from Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdmgl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdmgl <amount>");
		}
	}
	static void WITHDRAW_igl(ENetPeer* peer, std::string cmd) {
		time_t now = time(nullptr);
		if (now - pInfo(peer)->bank_last_op < 1 || pInfo(peer)->bank_in_transaction) { VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Wait before next bank operation.", 0, 1); return; }
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdigl <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int free_slots = Inventory::Get_Slots(pInfo(peer)), add = 0, amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200) return;
			if (amount > pInfo(peer)->mgl_bank_amount) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Infinity Gem Locks to Withdraw", 0, 1);
				return;
			}
			if (free_slots == 0) {
				VarList::OnConsoleMessage(peer, "You don't have room in your backpack!");
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have room in your backpack!", 0, 1);
				return;
			}
			if (Inventory::Check_Max(peer, 20298, amount)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "That wouldnt fit into my inventory!", 0, true);
				return;
			}
			if (Inventory::Modify(peer, 20298, add += amount) == 0) {
				pInfo(peer)->mgl_bank_amount -= amount;
				if (pInfo(peer)->mgl_bank_amount < 0) pInfo(peer)->mgl_bank_amount = 0;
				pInfo(peer)->bank_last_op = time(nullptr);
				VisualHandle::Trade(peer, 20298, 500);
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Withdraw your Infinity Gem Lock from Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Withdraw " + to_string(amount) + " Infinity Gem Lock from Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdigl <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdigl <amount>");
		}
	}
	static void WITHDRAW_GEMS(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdgems <amount>");
			return;
		}
		if (pInfo(peer)->bank_password == 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have not created a PIN for your bank account.", 0, 1);
			return;
		}
		try {
			int amount = std::stoi(cmd.c_str());
			if (amount < 1 or amount > 200000000) return;
			if (amount > pInfo(peer)->Gems_Storage) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough Gems to Withdraw", 0, 1);
				return;
			}
			if (pInfo(peer)->gems >= 2100000000) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`wYour gems have reached the maximum limit", 0, 1);
				return;
			}
			if (pInfo(peer)->Gems_Storage >= amount) {
				pInfo(peer)->Gems_Storage -= amount;
				if (pInfo(peer)->Gems_Storage < 0) pInfo(peer)->Gems_Storage = 0;
				pInfo(peer)->bank_last_op = time(nullptr);
				VarList::OnBuxGems(peer, amount);
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Withdraw your " + Set_Count(amount) + " Gems from Bank.", 0, 1);
				pInfo(peer)->Bank_History.push_back(currentDateTime() + " You Withdraw " + Set_Count(amount) + " Gems from Bank Central");
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdgems <amount>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /wdgems <amount>");
		}
	}
	static void MESSAGE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "Usage: /msg <`$full or first part of a name``> <`$your message``> - This will send a private message to someone anywhere in the universe.  If you don't include a message, you can just see if he/she is online or not.");
			return;
		}
		bool found = false;
		std::istringstream ss(cmd);
		std::string pm_user, pm_message;
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (ss >> pm_user >> pm_message) {
				if (find(pInfo(peer)->ignored.begin(), pInfo(peer)->ignored.end(), to_lower(pm_user)) != pInfo(peer)->ignored.end()) {
					VarList::OnConsoleMessage(peer, "(Sent to nobody) `0(`4Note:`` `0You can't send messages to people you are currently ignoring)``");
					return;
				}
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (pInfo(cp_)->m_h == 1 or pInfo(cp_)->invis == true) continue;
					if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pm_user)) {
						if (find(pInfo(cp_)->ignored.begin(), pInfo(cp_)->ignored.end(), to_lower(pInfo(peer)->tankIDName)) != pInfo(cp_)->ignored.end()) {
							VarList::OnConsoleMessage(peer, "`6>> `4SYSTEM``: That user is ignoring you.");
							return;
						}
						bool msg = false, sdg = false;
						for (int c_ = 0; c_ < pInfo(peer)->friends.size(); c_++) {
							if (to_lower(pInfo(peer)->friends[c_].name) == to_lower(pInfo(cp_)->tankIDName)) {
								if (pInfo(peer)->friends[c_].mute) sdg = true;
							}
						}
						for (int c_ = 0; c_ < pInfo(cp_)->friends.size(); c_++) {
							if (to_lower(pInfo(cp_)->friends[c_].name) == to_lower(pInfo(peer)->tankIDName)) {
								if (pInfo(cp_)->friends[c_].mute) msg = true;
							}
						}
						if (sdg) {
							VarList::OnConsoleMessage(peer, "(`4Note: `6: That user is muted you)");
							return;
						}
						else if (msg) {
							VarList::OnConsoleMessage(peer, "(`4Note: ```6You can't send messages to people you muted)");
							return;
						}
						else {
							pInfo(cp_)->lastmsg = pInfo(peer)->tankIDName;
							pInfo(cp_)->lastmsgworld = pInfo(peer)->world;
							string p0;
							if (not Role::Moderator(peer) and Role::Moderator(cp_) || not Role::Administrator(peer) and Role::Administrator(cp_) || not Role::Developer(peer) and Role::Developer(cp_)) {
								p0 = "`6>> (Sent to `$" + pInfo(cp_)->tankIDName + "`6) `o(`4Note: ``Message a mod `4ONLY ONCE `oabout an issue. Mods dont fix scams or replace gems, they punish players who break the `5/rules`o. For issues related to account recovery or purchasing, send message to creators on discord.)";
							}
							else {
								p0 = "`6>> (Sent to `$" + pInfo(cp_)->tankIDName + "`6)";
							}
							VarList::OnConsoleMessage(peer, p0);
							found = true;
							VarList::OnConsoleMessage(cp_, "CP:0_PL:0_OID:_CT:[MSG]_>> `6>> Private msg from " + get_player_nick(peer) + " `6(`&in " + (pInfo(peer)->m_h || find(world_->active_jammers.begin(), world_->active_jammers.end(), 226) != world_->active_jammers.end() ? "`4JAMMED!``" : pInfo(peer)->world) + "`6) : ```$" + pm_message + "``");
							CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/pay_time.wav", 0);
							break;
						}
					}
				}
				if (found == false) {
					VarList::OnConsoleMessage(peer, "`6>> No one online who has a name starting with " + pm_user + "`8.");
				}
			}
		}
	}
	static void BOARDCAST(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, ">> This lets you send a message to `5100`` random users online.  Costs `$250 Gems`` to use.  Example:`` `w/broadcast Hey guys, come to my world!");
				return;
			}
			if (pInfo(peer)->level < 5) {
				VarList::OnConsoleMessage(peer, " >> You must be level 5 to broadcast!");
			}
			if (Playmods::HasById(pInfo(peer), 11)) {
				VarList::OnConsoleMessage(peer, "`6>> That's sort of hard to do while duct-taped.``");
				return;
			}
			if (Playmods::HasById(pInfo(peer), 13)) {
				int time_ = 0;
				for (PlayMods peer_playmod : pInfo(peer)->playmods) {
					if (peer_playmod.id == 13) {
						time_ = peer_playmod.time - time(nullptr);
						break;
					}
				}
				VarList::OnConsoleMessage(peer, ">> (" + Time::Playmod(time_) + " before you can broadcast again)");
				return;
			}
			int s_c = 1000;
			if (pInfo(peer)->gems >= s_c) {
				VarList::OnBuxGems(peer, s_c * -1);
				if (not Role::Administrator(peer) and not Role::Developer(peer)) { //change
					PlayMods new_playmod{};
					new_playmod.id = 13, new_playmod.time = time(nullptr) + 600;
					pInfo(peer)->playmods.push_back(new_playmod);
				}
				if (pInfo(peer)->m_h != 1 and find(world_->active_jammers.begin(), world_->active_jammers.end(), 226) == world_->active_jammers.end()) {
					Environment()->Sb_World = pInfo(peer)->world;
				}
				int sentto = 0;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->radio) continue;
					if (sentto > 100) continue;
					sentto++;
					VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[BC]_ `p** from (`0" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + "`````p) in [```$" + (pInfo(peer)->m_h == 1 || find(world_->active_jammers.begin(), world_->active_jammers.end(), 226) != world_->active_jammers.end() ? "`4JAMMED!``" : pInfo(peer)->world) + "```p] ** : ``" + (Role::Moderator(peer) and not Role::Developer(peer) ? "`$" : "`^") + "" + cmd + "``");
				}
			}
			else {
				VarList::OnConsoleMessage(peer, ">> You don't have `$" + Set_Count(s_c) + " Gems``, you can't broadcast a message!");
				return;
			}
		}
	}
	static void SUPER_BOARDCAST(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, ">> This lets you send a message to " + Set_Count(ServerPool::PlayerCountServer()) + "`` random users online and make an annoying beep noise.  Costs `$50,000 Gems`` to use. Example:`` `w/sb Hey guys, come to my world!");
				return;
			}
			if (pInfo(peer)->level < 5) {
				VarList::OnConsoleMessage(peer, ">> You must be level 5 to broadcast!");
			}
			if (Playmods::HasById(pInfo(peer), 11)) {
				VarList::OnConsoleMessage(peer, "`6>> That's sort of hard to do while duct-taped.``");
				return;
			}
			if (Playmods::HasById(pInfo(peer), 122)) {
				int time_ = 0;
				for (PlayMods peer_playmod : pInfo(peer)->playmods) {
					if (peer_playmod.id == 122) {
						time_ = peer_playmod.time - time(nullptr);
						break;
					}
				}
				VarList::OnConsoleMessage(peer, ">> (" + Time::Playmod(time_) + " before you can broadcast again)");
				return;
			}
			int s_c = 1000 * 50;
			if (pInfo(peer)->gems >= s_c or pInfo(peer)->usedmegaphone) {
				if (pInfo(peer)->starglitter_sb < 10 && pInfo(peer)->starglitter_sb != -1) DailyEvent_Task::MStarglitter(peer, "sb", 1);
				int removemegaphone = -1, megacount = 0;
				if (pInfo(peer)->usedmegaphone == 0) {
					VarList::OnBuxGems(peer, s_c * -1);
				}
				else {
					Inventory::Modify(peer, 2480, removemegaphone);
					Inventory::Modify(peer, 2480, megacount);
				}
			if (not Role::Administrator(peer) and not Role::Developer(peer)) {
				Playmods::Add(peer, 122, 90);
			}
				CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
				VarList::OnConsoleMessage(peer, "Broadcasting to ALL! (`$Megaphone!`` mod added)");
				VarList::OnConsoleMessage(peer, a + "`#Super-Broadcast sent`$. Used " + (pInfo(peer)->usedmegaphone ? "1 Megaphone. (" + to_string(megacount) : Set_Count(s_c) + " Gems. (" + Set_Count(pInfo(peer)->gems)) + " left) (1 min 30 sec before you can broadcast again)");
				pInfo(peer)->usedmegaphone = 0;
				if (pInfo(peer)->m_h != 1 and find(world_->active_jammers.begin(), world_->active_jammers.end(), 226) == world_->active_jammers.end()) {
					Environment()->Sb_World = pInfo(peer)->world;
				}
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->radio) continue;
					VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[SB]_ `w** `5Super-Broadcast `wfrom `0" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + "```` (in ```$" + (pInfo(peer)->m_h == 1 || find(world_->active_jammers.begin(), world_->active_jammers.end(), 226) != world_->active_jammers.end() ? "`4JAMMED!``" : pInfo(peer)->world) + "```w) ** : ``" + (Role::Moderator(peer) and not Role::Developer(peer) ? "`$" : "`^") + "" + cmd + "``");
				}
			}
			else {
				VarList::OnConsoleMessage(peer, ">> You don't have `$" + Set_Count(s_c) + " Gems``, you can't broadcast a message!");
				return;
			}
		}
	}
	static void IGNORED(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "Type `5/ignore`` <`5name``> to mute them.  (You can unmute people using the `5Friends List``)");
			return;
		}
		bool playerfound = false;
		std::string name = cmd;
		if (to_lower(pInfo(peer)->tankIDName) == name or find(pInfo(peer)->ignored.begin(), pInfo(peer)->ignored.end(), name) != pInfo(peer)->ignored.end()) VarList::OnConsoleMessage(peer, (to_lower(pInfo(peer)->tankIDName) == name ? "Don't look now, but that person is `5IN YOUR HOUSE``!" : "That jerk `5" + name + "`` is already being ignored!"));
		else {
			std::string current_name = "";
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (to_lower(pInfo(cp_)->tankIDName.substr(0, 3)) == to_lower(name.substr(0, 3)) || name == to_lower(pInfo(cp_)->tankIDName)) {
					if (find(pInfo(peer)->ignored.begin(), pInfo(peer)->ignored.end(), to_lower(pInfo(cp_)->tankIDName)) != pInfo(peer)->ignored.end()) {
						VarList::OnConsoleMessage(peer, "You removed `5" + pInfo(cp_)->tankIDName + "`` from ignore list!");
						pInfo(peer)->ignored.erase(remove(pInfo(peer)->ignored.begin(), pInfo(peer)->ignored.end(), pInfo(cp_)->tankIDName), pInfo(peer)->ignored.end());
						return;
					}
					pInfo(peer)->ignored.push_back(to_lower(pInfo(cp_)->tankIDName));
					current_name = pInfo(cp_)->tankIDName;
					playerfound = true;
					break;
				}
			}
			if (playerfound) VarList::OnConsoleMessage(peer, "`4PERSON IGNORED!`` Ok, you won't see messages from `5" + current_name + "``. If you change your mind, you can remove him/her from ignore by using the `5Friends List``.");
			else VarList::OnConsoleMessage(peer, "Hmm, nobody is currently online with a name starting with `5" + name + "``.");
		}
	}
	static void ME(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "`6>> Type `5/me looks around suspiciously`` or any other text, useful for role playing.");
				return;
			}
			if (pInfo(peer)->world.empty()) return;
			if (Playmods::HasById(pInfo(peer), 11)) {
				VarList::OnConsoleMessage(peer, "`6>> That's sort of hard to do while duct-taped.``");
				return;
			}
			std::string text = cmd;
			if (text.length() <= 0 || text.empty()) return;
			{
				if (world_->world_settings & SETTINGS::SETTINGS_8 and Role::Administrator(peer) != 1 and to_lower(world_->owner_name) != to_lower(pInfo(peer)->tankIDName) and find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(peer)->tankIDName)) == world_->admins.end()) {
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`1(Peasants must not speak)", 0, 1);
					return;
				}
			}
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == pInfo(peer)->world) {
					VarList::OnConsoleMessage(cp_, "CP:0_PL:0_OID:__CT:[W]_ `6<``" + get_player_nick(peer) + "`` `#" + text + "```6>``");
					VarList::OnTalkBubble(cp_, pInfo(peer)->netID, "player_chat= `6<```0" + get_player_nick(peer) + "`` `#" + text + "```6>``", 0, 0);
				}
			}
		}
	}
	static void R_MSG(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "Usage: /r <`$your message``> - This will send a private message to the last person who sent you a message. Use /msg to talk to somebody new!");
				return;
			}
			std::string pm_user = pInfo(peer)->lastmsg, pm_message = cmd;
			if (pm_user.empty()) {
				VarList::OnConsoleMessage(peer, "Usage: /r <`$your message``> - This will send a private message to the last person who sent you a message. Use /msg to talk to somebody new!");
				return;
			}
			else {
				bool found = false;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (pInfo(cp_)->m_h == 1 or pInfo(cp_)->invis == true) continue;
					if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pm_user)) {
						pInfo(cp_)->lastmsg = pInfo(peer)->tankIDName;
						pInfo(cp_)->lastmsgworld = pInfo(peer)->world;
						string p0;
						if (not Role::Moderator(peer) and Role::Moderator(cp_) || not Role::Administrator(peer) and Role::Administrator(cp_) || not Role::Developer(peer) and Role::Developer(cp_)) {
							p0 = "`6>> (Sent to `$" + pInfo(cp_)->tankIDName + "`6) `o(`4Note: ``Message a mod `4ONLY ONCE `oabout an issue. Mods dont fix scams or replace gems, they punish players who break the `5/rules`o. For issues related to account recovery or purchasing, send message to creators on discord.)";
						}
						else {
							p0 = "`6>> (Sent to `$" + pInfo(cp_)->tankIDName + "`6)";
						}
						VarList::OnConsoleMessage(peer, p0);
						found = true;
						VarList::OnConsoleMessage(cp_, "CP:0_PL:0_OID:_CT:[MSG]_>> `c>> from (``" + get_player_nick(peer) + "`c) in [`$" + (pInfo(peer)->m_h || find(world_->active_jammers.begin(), world_->active_jammers.end(), 226) != world_->active_jammers.end() ? "`4JAMMED!``" : pInfo(peer)->world) + "``] > ```$" + pm_message + "``");
						CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/pay_time.wav", 0);
						break;
					}
				}
				if (found == false) {
					VarList::OnConsoleMessage(peer, "`6>> No one online who has a name starting with " + pm_user + "`8.");
				}
			}
		}
	}
	static void PULL(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "You'll need to enter at least the first three characters of the person's name.");
				return;
			}
			std::string username = cmd;
			if (to_lower(pInfo(peer)->tankIDName) == to_lower(username)) {
				VarList::OnConsoleMessage(peer, "`oYou give yourself a solid boot to the backside. But what did that really accomplish?");
				return;
			}
			if (username.size() < 3) {
				VarList::OnConsoleMessage(peer, "You'll need to enter at least the first three characters of the person's name.");
				return;
			}
			std::string by_name = "";
			if ((pInfo(peer)->name_color == "`0" || pInfo(peer)->name_color == "`2" || pInfo(peer)->name_color == "`^") && not pInfo(peer)->d_name.empty()) by_name = "`o" + pInfo(peer)->tankIDName + "``";
			else by_name = get_player_nick(peer);
			bool playerFound = false;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(peer)->world == pInfo(cp_)->world) {
					if (to_lower(pInfo(peer)->tankIDName) == to_lower(world_->owner_name) || find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(peer)->tankIDName)) != world_->admins.end() or Role::Moderator(peer)) {
						if ((to_lower(pInfo(cp_)->tankIDName).find(to_lower(username)) != std::string::npos && username.length() >= 3 && username.length() != pInfo(cp_)->tankIDName.length()) || to_lower(pInfo(cp_)->tankIDName) == to_lower(username) || (not pInfo(cp_)->d_name.empty() && (to_lower(pInfo(cp_)->d_name).find(to_lower(username)) != std::string::npos && username.length() >= 3 && username.length() != pInfo(cp_)->d_name.length()) || to_lower(pInfo(cp_)->d_name) == to_lower(username))) {
							int x_ = pInfo(peer)->x / 32, y_ = pInfo(peer)->y / 32, x__ = pInfo(cp_)->x / 32, y__ = pInfo(cp_)->y / 32;
							if (x_ < 0 or x_ >= world_->max_x or y_ < 0 or y_ >= world_->max_y or x__ < 0 or x__ >= world_->max_x or y__ < 0 or y__ >= world_->max_y) {
								VarList::OnTextOverlay(peer, "Can't `5pull`` Player with a higher role than you!");
								break;
							}
							playerFound = true;
							if (not pInfo(cp_)->d_name.empty()) {
								username = pInfo(cp_)->d_name;
							}
							else {
								if (not pInfo(cp_)->d_name.empty() && (pInfo(cp_)->name_color == "`0" || pInfo(cp_)->name_color == "`2" || pInfo(cp_)->name_color == "`^")) username = "`o" + pInfo(cp_)->tankIDName + "``";
								else username = get_player_nick(cp_);
							}
							pInfo(cp_)->x = pInfo(peer)->x, pInfo(cp_)->y = pInfo(peer)->y;
							VarList::OnSetPos(cp_, pInfo(peer)->x, pInfo(peer)->y);
							VarList::OnTextOverlay(cp_, "You were pulled by " + by_name);
						}
					}
					else {
						VarList::OnTextOverlay(peer, "`wCan't `5Pull`w, that player is not standing in an area you control!");
						return;
					}
				}
			}
			if (playerFound) {
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->world != pInfo(cp_)->world) continue;
					VarList::OnConsoleMessage(cp_, by_name + "`` `5pulls`` `o" + username + "``!``");
					CAction::Positioned(cp_, pInfo(peer)->netID, "audio/teleport.wav");
				}
			}
		}
	}
	static void KICK(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "You'll need to enter at least the first three characters of the person's name.");
				return;
			}
			std::string username = cmd;
			if (to_lower(pInfo(peer)->tankIDName) == to_lower(username)) {
				VarList::OnConsoleMessage(peer, "`oOuch!");
				return;
			}
			if (username.size() < 3) {
				VarList::OnConsoleMessage(peer, "You'll need to enter at least the first three characters of the person's name.");
				return;
			}
			std::string by_name = "";
			if ((pInfo(peer)->name_color == "`0" || pInfo(peer)->name_color == "`2" || pInfo(peer)->name_color == "`^") && not pInfo(peer)->d_name.empty()) by_name = "`o" + pInfo(peer)->tankIDName + "``";
			else by_name = get_player_nick(peer);
			bool playerFound = false;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(peer)->world == pInfo(cp_)->world) {
					if (to_lower(pInfo(peer)->tankIDName) == to_lower(world_->owner_name) || find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(peer)->tankIDName)) != world_->admins.end() or Role::Moderator(peer)) {
						if ((to_lower(pInfo(cp_)->tankIDName).find(to_lower(username)) != std::string::npos && username.length() >= 3 && username.length() != pInfo(cp_)->tankIDName.length()) || to_lower(pInfo(cp_)->tankIDName) == to_lower(username) || (not pInfo(cp_)->d_name.empty() && (to_lower(pInfo(cp_)->d_name).find(to_lower(username)) != std::string::npos && username.length() >= 3 && username.length() != pInfo(cp_)->d_name.length()) || to_lower(pInfo(cp_)->d_name) == to_lower(username))) {
							int x_ = pInfo(peer)->x / 32, y_ = pInfo(peer)->y / 32, x__ = pInfo(cp_)->x / 32, y__ = pInfo(cp_)->y / 32;
							if (not Role::Administrator(peer) and Role::Administrator(cp_)) {
								VarList::OnTextOverlay(peer, "Can't `4kick`` Player with a higher role than you!");
								return;
							}
							if (x_ < 0 or x_ >= world_->max_x or y_ < 0 or y_ >= world_->max_y or x__ < 0 or x__ >= world_->max_x or y__ < 0 or y__ >= world_->max_y) {
								VarList::OnTextOverlay(peer, "Can't `4kick`` Player with a higher role than you!");
								return;
							}
							playerFound = true;
							if (not pInfo(cp_)->d_name.empty()) {
								username = pInfo(cp_)->d_name;
							}
							else {
								if (not pInfo(cp_)->d_name.empty() && (pInfo(cp_)->name_color == "`0" || pInfo(cp_)->name_color == "`2" || pInfo(cp_)->name_color == "`^")) username = "`o" + pInfo(cp_)->tankIDName + "``";
								else username = get_player_nick(cp_);
							}
							Player_Respawn(cp_, true, 0, 1);
						}
					}
					else {
						VarList::OnTextOverlay(peer, "`wCan't `4kick`w, is not in a world you control!");
						return;
					}
				}
			}
			if (playerFound) {
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->world != pInfo(cp_)->world) continue;
					VarList::OnConsoleMessage(cp_, by_name + "`` `4kicks`` `o" + username + "``!``");
				}
			}
		}
	}
	static void ULTIMATE(ENetPeer* peer, std::string targetName) {
		if (targetName.empty()) {
			VarList::OnConsoleMessage(peer, "4Usage: /ultimate <player name>");
			return;
		}

		// [1] Cari target player
		ENetPeer* targetPeer = nullptr;
		for (ENetPeer* target = server->peers; target < &server->peers[server->peerCount]; ++target) {
			if (target->state != ENET_PEER_STATE_CONNECTED || !target->data) continue;
			if (to_lower(pInfo(target)->tankIDName) == to_lower(targetName)) {
				targetPeer = target;
				break;
			}
		}

		if (!targetPeer) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Player not found!", 0, 0);
			return;
		}

		// [2] Copy set dari target ke admin
		pInfo(peer)->hair = pInfo(targetPeer)->hair;
		pInfo(peer)->shirt = pInfo(targetPeer)->shirt;
		pInfo(peer)->pants = pInfo(targetPeer)->pants;
		pInfo(peer)->feet = pInfo(targetPeer)->feet;
		pInfo(peer)->face = pInfo(targetPeer)->face;
		pInfo(peer)->hand = pInfo(targetPeer)->hand;
		pInfo(peer)->back = pInfo(targetPeer)->back;
		pInfo(peer)->mask = pInfo(targetPeer)->mask;
		pInfo(peer)->necklace = pInfo(targetPeer)->necklace;
		pInfo(peer)->ances = pInfo(targetPeer)->ances;
		Clothing_V2::Update(peer);

		// [3] Pesan "I am you, and you are me"
		VarList::OnTalkBubble(peer, pInfo(peer)->netID, "I am `4you`0, and you are me", 0, 0);
		VarList::OnTalkBubble(targetPeer, pInfo(targetPeer)->netID, "I am `4you`0, and you are me", 0, 0);

		// [4] Bekukan target + efek INVIS
		Playmods::Add(targetPeer, 2); // Freeze
		VisualHandle::State(pInfo(targetPeer));
		Clothing_V2::Update(targetPeer, true);

		// [5] Set weather ke ID 9 (Black Hole)
		WEATHER_WORLD(targetPeer, "9");

		// [6] Efek partikel INVIS
		std::thread([targetPeer]() {
			for (int i = 0; i < 30; i++) {
				if (targetPeer->state != ENET_PEER_STATE_CONNECTED) break;
				for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
					if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data || pInfo(cp)->world != pInfo(targetPeer)->world) continue;
					if (rand() % 100 <= 75) VarList::OnParticleEffect(cp, pInfo(targetPeer)->x - 15 * (rand() % 6), pInfo(targetPeer)->y - 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			}).detach();

		// [7] Summon target ke dekat admin (1 block jarak)
		std::thread([peer, targetPeer]() {
			std::this_thread::sleep_for(std::chrono::seconds(2));

			// Teleport ke 1 block di depan admin
			float teleportX = pInfo(peer)->x + 32;
			float teleportY = pInfo(peer)->y;
			pInfo(targetPeer)->x = teleportX;
			pInfo(targetPeer)->y = teleportY;

			// Update posisi player
			PlayerMoving data;
			data.packetType = 1;
			data.netID = pInfo(targetPeer)->netID;
			data.x = teleportX;
			data.y = teleportY;
			BYTE* raw = packPlayerMoving(&data);
			send_raw(targetPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
			delete[] raw;

			// [8] Hitung mundur 3, 2, 1
			for (int countdown = 3; countdown >= 1; countdown--) {
				VarList::OnDialogRequest(targetPeer,
					"set_default_color|`4\n"
					"add_label_with_icon|big|`4RESPAWN IN|left|206|\n"
					"add_textbox|#" + std::to_string(countdown) + "|\n"
					"add_quick_exit|\n"
					"end_dialog|countdown_" + std::to_string(countdown) + "|`wClose||"
				);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}

			// [9] RESPAWN target
			if (pInfo(targetPeer)->WarnRespawn == 5) {
				VarList::OnConsoleMessage(peer, "`4SPAM RESPAWN DETECTED! Skipping respawn.");
			}
			else {
				using namespace std::chrono;
				if (pInfo(targetPeer)->lastRESP + 1800 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
					pInfo(targetPeer)->lastRESP = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
				}
				else {
					pInfo(targetPeer)->WarnRespawn++;
				}
				Player_Respawn(targetPeer, false, 0, true);
			}

			// [10] Kembalikan weather ke normal
			WEATHER_WORLD(targetPeer, "0");
			VarList::OnConsoleMessage(peer, "`4Ultimate punishment completed!");
			}).detach();
	}
	static void BAN(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "You'll need to enter at least the first three characters of the person's name.");
				return;
			}
			std::string username = cmd;
			if (to_lower(pInfo(peer)->tankIDName) == to_lower(username)) {
				VarList::OnConsoleMessage(peer, "`oOuch!");
				return;
			}
			if (username.size() < 3) {
				VarList::OnConsoleMessage(peer, "You'll need to enter at least the first three characters of the person's name.");
				return;
			}
			std::string by_name = "";
			if ((pInfo(peer)->name_color == "`0" || pInfo(peer)->name_color == "`2" || pInfo(peer)->name_color == "`^") && not pInfo(peer)->d_name.empty()) by_name = "`o" + pInfo(peer)->tankIDName + "``";
			else by_name = get_player_nick(peer);
			bool playerFound = false;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(peer)->world == pInfo(cp_)->world) {
					if (to_lower(pInfo(peer)->tankIDName) == to_lower(world_->owner_name) || find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(peer)->tankIDName)) != world_->admins.end() or Role::Moderator(peer)) {
						if ((to_lower(pInfo(cp_)->tankIDName).find(to_lower(username)) != std::string::npos && username.length() >= 3 && username.length() != pInfo(cp_)->tankIDName.length()) || to_lower(pInfo(cp_)->tankIDName) == to_lower(username) || (not pInfo(cp_)->d_name.empty() && (to_lower(pInfo(cp_)->d_name).find(to_lower(username)) != std::string::npos && username.length() >= 3 && username.length() != pInfo(cp_)->d_name.length()) || to_lower(pInfo(cp_)->d_name) == to_lower(username))) {
							int x_ = pInfo(peer)->x / 32, y_ = pInfo(peer)->y / 32, x__ = pInfo(cp_)->x / 32, y__ = pInfo(cp_)->y / 32;
							if (pInfo(cp_)->world == "HELL") {
								VarList::OnConsoleMessage(peer, "`4You can't ban players in this world.");
								continue;
							}
							if (not Role::Administrator(peer) and Role::Administrator(cp_)) {
								VarList::OnTextOverlay(peer, "Can't `4ban`` Player with a higher role than you!");
								return;
							}
							if (world_->owner_name.empty() || find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(cp_)->tankIDName)) != world_->admins.end() || to_lower(pInfo(cp_)->tankIDName) == to_lower(world_->owner_name) || to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->tankIDName) || Role::Administrator(cp_) or x_ < 0 or x_ >= world_->max_x or y_ < 0 or y_ >= world_->max_y or x__ < 0 or x__ >= world_->max_x or y__ < 0 or y__ >= world_->max_y) {
								VarList::OnTextOverlay(peer, "Can't `4ban``, is not in a locked area you control!");
								return;
							}
							playerFound = true;
							VarList::OnConsoleMessage(cp_, by_name + "`` `4world bans```` " + username + "`` `ofrom`` `0" + pInfo(peer)->world + "``!");
							world_->bannedPlayers.push_back(make_pair(to_lower(pInfo(cp_)->tankIDName), (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()));
							Exit_World(cp_);
						}
					}
					else {
						VarList::OnTextOverlay(peer, "`wCan't `4ban`w, is not in a world you control!");
						return;
					}
				}
			}
			if (playerFound) {
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->world != pInfo(cp_)->world) continue;
					VarList::OnConsoleMessage(cp_, by_name + "`` `4world bans```` " + username + "`` `ofrom`` `0" + pInfo(peer)->world + "``!");
					CAction::Positioned(cp_, pInfo(peer)->netID, "audio/repair.wav");
				}
			}
		}
	}
	static void TRADE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "To trade with a specific person in this world, do `2/trade <``full or partial name`2>``");
			return;
		}
		pInfo(peer)->last_wrenched = cmd;
		if (to_lower(pInfo(peer)->last_wrenched) == to_lower(pInfo(peer)->tankIDName)) {
			VarList::OnConsoleMessage(peer, "You trade all your stuff to yourself in exchange for all your stuff.");
			return;
		}
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != pInfo(peer)->world or pInfo(cp_)->tankIDName == pInfo(peer)->tankIDName) continue;
			if ((to_lower(pInfo(cp_)->tankIDName).find(to_lower(pInfo(peer)->last_wrenched)) != std::string::npos && pInfo(peer)->last_wrenched.length() >= 3 && pInfo(peer)->last_wrenched.length() != pInfo(cp_)->tankIDName.length()) || to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched) || (not pInfo(cp_)->d_name.empty() && (to_lower(pInfo(cp_)->d_name).find(to_lower(pInfo(peer)->last_wrenched)) != std::string::npos && pInfo(peer)->last_wrenched.length() >= 3 && pInfo(peer)->last_wrenched.length() != pInfo(cp_)->d_name.length()) || to_lower(pInfo(cp_)->d_name) == to_lower(pInfo(peer)->last_wrenched))) {
				for (int c_ = 0; c_ < pInfo(cp_)->friends.size(); c_++) {
					if (to_lower(pInfo(cp_)->friends[c_].name) == to_lower(pInfo(peer)->tankIDName)) {
						if (pInfo(cp_)->friends[c_].block_trade)
							return;
					}
				}
				if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->tankIDName)) {
					VarList::OnConsoleMessage(peer, "You trade all your stuff to yourself in exchange for all your stuff.");
					return;
				}
				if (pInfo(cp_)->trading_with != -1 and pInfo(cp_)->trading_with != pInfo(peer)->netID) {
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "That person is busy.", 0, 0);
					pInfo(peer)->block_trade = false, pInfo(peer)->trading_with = -1;
					VarList::OnForceTradeEnd(peer);
					return;
				}
				pInfo(peer)->trading_with = pInfo(cp_)->netID;
				pInfo(peer)->trade_accept = 0;
				pInfo(cp_)->trade_accept = 0;
				gamepacket_t p;
				p.Insert("OnStartTrade");
				p.Insert(fixchar2(get_player_nick(cp_))), p.Insert(pInfo(cp_)->netID), p.CreatePacket(peer);
				if (pInfo(cp_)->trading_with != -1) {
					VarList::OnTradeStatus(peer, pInfo(peer)->netID, "", "`o" + get_player_nick(peer) + "'s offer.``", "locked|0\nreset_locks|1\naccepted|0");
					VarList::OnTradeStatus(peer, pInfo(peer)->netID, "", "`o" + get_player_nick(peer) + "'s offer.``", "locked|0\naccepted|1");
					VarList::OnTradeStatus(peer, pInfo(peer)->netID, "", "`o" + get_player_nick(peer) + "'s offer.``", "locked|0\nreset_locks|1\naccepted|0");
					VarList::OnTradeStatus(peer, pInfo(cp_)->netID, "", "`o" + get_player_nick(cp_) + "'s offer.``", make_trade_offer(pInfo(cp_), true) + "locked|0\nreset_locks|1\naccepted|0");
					VarList::OnTradeStatus(cp_, pInfo(cp_)->netID, "", "`o" + get_player_nick(cp_) + "'s offer.``", "locked|0\nreset_locks|1\naccepted|0");
					VarList::OnTradeStatus(cp_, pInfo(cp_)->netID, "", "`o" + get_player_nick(cp_) + "'s offer.``", "locked|0\naccepted|1");
					VarList::OnTradeStatus(cp_, pInfo(cp_)->netID, "", "`o" + get_player_nick(cp_) + "'s offer.``", make_trade_offer(pInfo(cp_), true) + "locked|0\nreset_locks|1\naccepted|0");
					VarList::OnTradeStatus(cp_, pInfo(peer)->netID, "", "`o" + get_player_nick(peer) + "'s offer.``", make_trade_offer(pInfo(peer), true) + "locked|0\nreset_locks|1\naccepted|0");
					break;
				}
				CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/cash_register.wav", 0);
				VarList::OnConsoleMessage(cp_, "`#TRADE ALERT:`` `w" + get_player_nick(peer) + "`` wants to trade with you!  To start, use the `wWrench`` on that person's wrench icon, or type `w/trade " + get_player_nick(peer));
				break;
			}
		}
	}
	static void RATE(ENetPeer* peer, std::string cmd) {
		int Rating = 0;
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
				VarList::OnConsoleMessage(peer, "Type \"/rate X\" to rate a world. You can rate a world 1-5! A 1 or 2 means you don't like it. 3 is average. 4 or 5 means you liked it! You can't rate a world of your own, our use alts to rate up your world. Nobody will ever see how you rated, so don't pay people to rate your world!");
				return;
			}
			if (world_->category.empty()) {
				VarList::OnConsoleMessage(peer, "A world needs to have a category set on the World Lock to be rated.");
			}
			else {
				try {
					Rating = std::stoi(cmd.c_str());
				}
				catch (const std::invalid_argument&) {
					VarList::OnConsoleMessage(peer, "Type \"/rate X\" to rate a world. You can rate a world 1-5! A 1 or 2 means you don't like it. 3 is average. 4 or 5 means you liked it! You can't rate a world of your own, our use alts to rate up your world. Nobody will ever see how you rated, so don't pay people to rate your world!");
				}
				catch (const std::out_of_range&) {
					VarList::OnConsoleMessage(peer, "Type \"/rate X\" to rate a world. You can rate a world 1-5! A 1 or 2 means you don't like it. 3 is average. 4 or 5 means you liked it! You can't rate a world of your own, our use alts to rate up your world. Nobody will ever see how you rated, so don't pay people to rate your world!");
				}
				if (world_->owner_name == pInfo(peer)->tankIDName) {
					VarList::OnConsoleMessage(peer, "You can't rate your own world!");
					return;
				}
				else if (Rating < 1 or Rating > 5) {
					VarList::OnConsoleMessage(peer, "Type \"/rate X\" to rate a world. You can rate a world 1-5! A 1 or 2 means you don't like it. 3 is average. 4 or 5 means you liked it! You can't rate a world of your own, or use alts to rate up your world. Nobody will ever see how you rated, so don't pay people to rate your world!");
					return;
				}
				else {
					VarList::OnConsoleMessage(peer, "Thanks for rating! World ratings are updated once a day. If you rate the same world again, it will replace this rating.");
					if (find(pInfo(peer)->last_rated.begin(), pInfo(peer)->last_rated.end(), pInfo(peer)->world) == pInfo(peer)->last_rated.end()) {
						double rating = std::stod(cmd.c_str());
						if (rating < 1) rating = 1;
						else if (rating > 5) rating = 5;
						rating *= 100;
						ServerPool::Ratings::Add(pInfo(peer)->world, pInfo(peer)->world_owner, world_->category, rating);
						pInfo(peer)->last_rated.push_back(pInfo(peer)->world);
					}
				}
			}
		}
	}
	static void GC(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`6>> Guildcast! Use /gc <message> to send messages to everyone who's online in your guild list. (they must have `5Show Guild Member Notifications`` checked to see them!)``");
			return;
		}
		if (pInfo(peer)->guild_id == 0) {
			VarList::OnConsoleMessage(peer, "`2You are not in guild. Join guild for use Guildcast!");
		}
		else {
			string text = cmd;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->guild_id == pInfo(peer)->guild_id) VarList::OnConsoleMessage(cp_, "`5[GUILD CHAT]`` " + pInfo(peer)->name_color + pInfo(peer)->tankIDName + "```5 in [```$" + pInfo(peer)->world + "```5] > " + text);
			}
		}
	}
	static void ANSWER(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "usage : /a <answer>");
			return;
		}
		int ans = 0, additem = 0;
		try {
			ans = std::stoi(cmd.c_str());
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "usage : /a <answer>");
			return;
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "usage : /a <answer>");
			return;
		}
		if (Event()->DailyMaths == false) return;
		if (ans != Event()->Math_Result) {
			VarList::OnConsoleMessage(peer, "`oYour Answer " + to_string(ans) + " is Wrong!");
			return;
		}
		else {
			VarList::OnBuxGems(peer, Event()->Math_Prize);
			VarList::OnConsoleMessage(peer, "`9** Congratulations! You won Daily Math. (party)");
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/pinata_lasso.wav", 0);
				VarList::OnConsoleMessage(cp_, "`9** " + Environment()->server_name + " Daily Math: (party) Party Math Event Winner is `w" + pInfo(peer)->tankIDName + "`9!");
			}
			Event()->DailyMaths = false; Event()->Math_Num1 = 0; Event()->Math_Num2 = 0; Event()->Math_Result = 0; Event()->Math_Prize = 0;
		}
	}
	static void FC(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "usage : /fc <text>");
			return;
		}
		int fonline = 0;
		std::string message = cmd;
		std::vector<std::string> friends_;
		for (int c_ = 0; c_ < pInfo(peer)->friends.size(); c_++) friends_.emplace_back(to_lower(pInfo(peer)->friends[c_].name));
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (find(friends_.begin(), friends_.end(), to_lower(pInfo(cp_)->tankIDName)) != friends_.end()) {
				fonline++;
				if (fonline != 0) VarList::OnConsoleMessage(cp_, "CT:[FC]_>> `3>> from (``" + get_player_nick(peer) + "`3) in [```$" + pInfo(peer)->world + "```3] > ```$" + message + "``");
			}
		}
		if (fonline == 0) VarList::OnConsoleMessage(peer, "`3>> There isn't any friends online.");
		else VarList::OnConsoleMessage(peer, "`3>> You friendcasted to `0" + to_string(fonline) + "`` person online.");
	}
	static void CHEATS(ENetPeer* peer) {
		DialogHandle::Cheats(peer);
	}
	static void CHEATER_CHAT(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "usage : /c <text>");
			return;
		}
		// 30 second cooldown
		long long now_cc = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
		if (pInfo(peer)->chat_cmd_time + 30000 > now_cc) {
			long long rem = (pInfo(peer)->chat_cmd_time + 30000 - now_cc) / 1000 + 1;
			VarList::OnConsoleMessage(peer, "`4>> Cooldown! Wait " + to_string(rem) + " seconds before using /c again.");
			return;
		}
		pInfo(peer)->chat_cmd_time = now_cc;
		string text = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or not Role::Cheater(cp_) or pInfo(cp_)->radio) continue;
			VarList::OnConsoleMessage(cp_, "CT:[FC]_>> `o>> [`bCHEATER-CHAT`o] from (``" + pInfo(peer)->name_color + pInfo(peer)->tankIDName + "```o) in [```$" + pInfo(peer)->world + "```o] > ```$`3" + text + "`p");
		}
	}
	static void HIDE_STATUS(ENetPeer* peer) {
		pInfo(peer)->hs = (pInfo(peer)->hs ? 0 : 1);
		VarList::OnConsoleMessage(peer, "Super Supporter status " + a + (pInfo(peer)->hs == 1 ? "visible" : "hidden") + "!");
		VisualHandle::State(pInfo(peer));
		Clothing_V2::Update(peer, true);
	}
	static void VIP_CHAT(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /v <text>");
			return;
		}
		// 30 second cooldown
		long long now_vip = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
		if (pInfo(peer)->chat_cmd_time + 30000 > now_vip) {
			long long rem = (pInfo(peer)->chat_cmd_time + 30000 - now_vip) / 1000 + 1;
			VarList::OnConsoleMessage(peer, "`4>> Cooldown! Wait " + to_string(rem) + " seconds before using /v again.");
			return;
		}
		pInfo(peer)->chat_cmd_time = now_vip;
		std::string text = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or not Role::Vip(cp_) or pInfo(cp_)->radio) continue;
			VarList::OnConsoleMessage(cp_, "CT:[FC]_>> `o>> [`1VIP-CHAT`o] from (``" + pInfo(peer)->name_color + pInfo(peer)->tankIDName + "```o) in [```$" + pInfo(peer)->world + "```o] > ```$`3" + text + "`p");
		}
	}
	static void INFO(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /info <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd;
		bool foundacc = true;
		if (pInfo(peer)->Role.Role_Level >= 4) {
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false) continue;
				if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
					foundacc = false;
					time_t s__;
					s__ = time(NULL);
					int days_ = int(s__) / (60 * 60 * 24);
					double hours_ = (double)((s__ - pInfo(cp_)->playtime) + pInfo(cp_)->seconds) / 3600;
					string num_text = to_string(hours_); string notes = "", crid = "";
					string rounded = num_text.substr(0, num_text.find(".") + 3);
					if (not pInfo(cp_)->Account_Notes.empty()) {
						for (int i = 0; i < pInfo(cp_)->Account_Notes.size(); i++) notes += "\nadd_textbox|`o" + pInfo(cp_)->Account_Notes[i] + "|left|\n";
					}
					else notes += "\nadd_textbox|`oThere's no account notes for this user yet.|left|";
					string bans = "", warning = "";
					for (string a : pInfo(cp_)->bans) bans += "\nadd_textbox|`o" + a + ", |left|\n";
					if (bans == "") bans = "\nadd_textbox|`oThis player has clear records / no bans.|left|";
					for (string a : pInfo(cp_)->Warning_Message) warning += "\nadd_textbox|`o" + a + ", |left|\n";
					if (warning == "") warning = "\nadd_textbox|`oThis player has clear records / no warnings.|left|";
					
					string d_ = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
						"add_label_with_icon|big|`wPlayer Information: `2ONLINE``|left|658|\n"
						"add_textbox|`oEditing: `w" + pInfo(cp_)->tankIDName + " `o(" + pInfo(cp_)->requestedName + ") `o| #`w" + to_string(pInfo(cp_)->id) + "``|left|\n"
						"add_textbox|`oPlaytime: `w" + rounded + " hours``   `oIP Address: `w" + pInfo(cp_)->ip + "``|left|\n"
						"add_spacer|small|\n"
						"add_textbox|`5─── Quick Actions ──────────────────────────────────────────|left|\n"
						"add_button|warp_to_" + pInfo(cp_)->world + "|`oWarp to Player (in `5" + pInfo(cp_)->world + "``)|noflags|0|0|\n"
						"add_button|SkillAssets_Info|`oAssets Info / Edit Role|noflags|0|0|\n"
						"add_button|view_inventory|`oView Inventory|noflags|0|0|\n"
						"add_spacer|small|\n"
						"add_textbox|`5─── Account Role & Assets ──────────────────────────────────|left|\n"
						"add_smalltext|`oRole: [" + (Role::Vip(cp_) ? "`2VIP``/" : "`4NO_VIP``/") + (Role::Moderator(cp_) ? "`2MOD``/" : "`4NO_MOD``/") + (Role::Administrator(cp_) ? "`2ADMIN``/" : "`4NO_ADMIN``/") + (Role::Developer(cp_) ? "`2DEVELOPER``/" : "`4NO_DEVELOPER``/") + (Role::God(cp_) ? "`2RESELLER``/" : "`4NO_RESELLER``/") + (Role::Unlimited(cp_) ? "`2UNLIMITED``/" : "`4NO_UNLIMITED``/") + (Role::Owner(cp_) ? "`2OWNER``/" : "`4NO_OWNER``/") + (Role::Cheater(cp_) or Playmods::HasById(pInfo(cp_), 143) ? "`2CHEATER``" : "`4NO_CHEATER``") + "]|left|\n"
						"add_smalltext|`oLevel: `w" + to_string(pInfo(cp_)->level) + "`` | Gems: `2" + Set_Count(pInfo(cp_)->gems) + "`` | OPC: `e" + Set_Count(pInfo(cp_)->opc) + "`` | XP: `9" + Set_Count(pInfo(cp_)->xp) + "``|left|\n"
						"add_spacer|small|\n"
						"add_textbox|`5─── Records ────────────────────────────────────────────────|left|\n"
						"add_textbox|`oAccount Warnings:``" + warning + "|\n"
						"add_textbox|`oAccount Bans:``" + bans + "|\n"
						"add_textbox|`oAccount Notes:``" + notes + "|\n"
						"add_spacer|small|\n"
						"add_textbox|`5─── Moderation Controls ────────────────────────────────────|left|\n"
						"add_smalltext|`oYou must have strong evidence (Hacks, Spammer, Scammer, etc.) and proof must be sent to Discord #Ban-Proof. Banning without solid evidence will result in demotion. Read /modrules before acting.|left|\n"
						+ (Role::Administrator(peer) ? "\nadd_label_with_icon_button|small|`4Blacklist from Server|left|278|blacklist_|" : "") +
						"\nadd_label_with_icon_button|small|`4Ban IP Address|left|278|ban_ip|"
						"\nadd_label_with_icon_button|small|`4Ban: 1 Week (DGH)|left|732|ban_604800|"
						"\nadd_label_with_icon_button|small|`4Ban: 4 Weeks (DGH)|left|732|ban_31|"
						"\nadd_label_with_icon_button|small|`4Ban: Permanent (DGH)|left|732|ban_729|"
						"\nadd_label_with_icon_button|small|`6Fake Auto-Ban (Online Troll)|left|1908|fake_ban|\n"
						"add_spacer|small|\n"
						"end_dialog|punish_view|Close||";
					VarList::OnDialogRequest(peer, d_);
					break;
				}
			}
			/*offline*/
			if (foundacc && (Role::Moderator(peer) or Role::Administrator(peer))) {
				bool found = false;
				try {
					string name = pInfo(peer)->last_wrenched;
					std::ifstream ifs("database/players/" + name + "_.json");
					if (ifs.is_open()) {
						json j;
						ifs >> j;
						found = true;
						pInfo(peer)->login_pass = j["pass"].get<string>();
						string add_unban = "";
						if (find(Environment()->Rid_Bans.begin(), Environment()->Rid_Bans.end(), j["ip"].get<string>()) != Environment()->Rid_Bans.end()) {
							add_unban += "\nadd_button|unbanip_" + j["name"].get<string>() + "|`4Unban (IP)``|noflags|0|0|";
						}
						time_t s__;
						s__ = time(NULL);
						int days_ = int(s__) / (60 * 60 * 24);
						double hours_ = (double)((s__ - time(NULL)) + j["playtime"].get<long long int>()) / 3600;
						string num_text = to_string(hours_); string notes = "", crid = "";
						string rounded = num_text.substr(0, num_text.find(".") + 3);
						string bans = "", warning = "";
						for (string a : j["Account_Notes"].get<vector<string>>()) notes += "\nadd_textbox|`o" + a + "|left|\n";
						if (notes == "") notes = "\nadd_textbox|`oThere's no account notes for this user yet.|left|";
						for (string a : j["7bans"].get<vector<string>>()) bans += "\nadd_textbox|`o" + a + ", |left|\n";
						if (bans == "") bans = "\nadd_textbox|`oThis player has clear records / no bans.|left|";
						for (string a : j["Warning_Message"].get<vector<string>>()) warning += "\nadd_textbox|`o" + a + ", |left|\n";
						if (warning == "") warning = "\nadd_textbox|`oThis player has clear records / no warnings.|left|";
						
						string d_ = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
							"add_label_with_icon|big|`wPlayer Information: `4OFFLINE``|left|658|\n"
							"add_textbox|`oEditing: `w" + j["name"].get<string>() + " `o(" + j["requestedName"].get<string>() + ") `o| #`w" + to_string(j["id"].get<int>()) + (j["b_s"].get<long long int>() == 0 or not add_unban.empty() ? "" : " (`4BANNED``)") + "``|left|\n"
							"add_textbox|`oPlaytime: `w" + rounded + " hours``   `oIP Address: `w" + j["ip"].get<string>() + "``|left|\n"
							"add_smalltext|Last online: `5" + j["lo"].get<string>() + "``|left|\n"
							"add_spacer|small|\n"
							"add_textbox|`5─── Quick Actions ──────────────────────────────────────────|left|\n"
							"add_button||`oWarp to User (in `4EXIT``)|off|0|0|\n"
							"add_button|SkillAssets_Info|`oAssets Info / Edit Role|noflags|0|0|\n"
							"add_button|view_inventory|`oView Inventory|noflags|0|0|\n"
							+ (Role::Clist(pInfo(peer)->tankIDName) ? "add_button|login_as|`5Login as`` `2" + name + "```5...``|noflags|0|0|\n" : "")
							+ (not add_unban.empty() or j["b_s"].get<long long int>() != 0 ? "\nadd_textbox|`5─── Ban Info ────────────────────────────────────────────────|left|" : "")
							+ (j["b_s"].get<long long int>() == 0 ? "" : "\nadd_smalltext|Banned by: `5" + j["b_b"].get<string>() + "``|left|\nadd_smalltext|Ban reason: `5" + j["b_r"].get<string>() + "``|left|\nadd_button|ban_0|`4Unban``|noflags|0|0|\n")
							+ (add_unban.empty() ? "" : add_unban + "\n") +
							"add_spacer|small|\n"
							"add_textbox|`5─── Account Role & Assets ──────────────────────────────────|left|\n"
							"add_smalltext|`oRole: [" + (j["Role.Vip"] == true ? "`2VIP``/" : "`4NO_VIP``/") + (j["Role.Moderator"] == true ? "`2MOD``/" : "`4NO_MOD``/") + (j["Role.Administrator"] == true ? "`2ADMIN``/" : "`4NO_ADMIN``/") + (j["Role.Developer"] == true ? "`2DEVELOPER``/" : "`4NO_DEVELOPER``/") + (j["Role.God"] == true ? "`2RESELLER``/" : "`4NO_RESELLER``/") + (j["Role.Unlimited"] == true ? "`2UNLIMITED``/" : "`4NO_UNLIMITED``/") + (j["Role.Owner_Server"] == true ? "`2OWNER``/" : "`4NO_OWNER``/") + (j["Role.Cheats"] == true ? "`2CHEATER``" : "`4NO_CHEATER``") + "]|left|\n"
							"add_smalltext|`oLevel: `w" + to_string(j["level"].get<int>()) + "`` | Gems: `2" + Set_Count(j["gems"].get<int>()) + "`` | OPC: `e" + Set_Count(j["opc"].get<uint16_t>()) + "`` | XP: `9" + Set_Count(j["xp"].get<long long int>()) + "``|left|\n"
							"add_spacer|small|\n"
							"add_textbox|`5─── Records ────────────────────────────────────────────────|left|\n"
							"add_textbox|`oAccount Warnings:``" + warning + "|\n"
							"add_textbox|`oAccount Bans:``" + bans + "|\n"
							"add_textbox|`oAccount Notes:``" + notes + "|\n"
							"add_spacer|small|\n"
							"add_textbox|`5─── Moderation Controls ────────────────────────────────────|left|\n"
							"add_smalltext|`oYou must have strong evidence (Hacks, Spammer, Scammer, etc.) and proof must be sent to Discord #Ban-Proof. Banning without solid evidence will result in demotion. Read /modrules before acting.|left|\n"
							+ (Role::Administrator(peer) ? "\nadd_label_with_icon_button|small|`4Blacklist from Server|left|278|blacklist_|" : "") +
							"\nadd_label_with_icon_button|small|`4Ban IP Address|left|278|ban_ip|"
							"\nadd_label_with_icon_button|small|`4Ban: 1 Week (DGH)|left|732|ban_604800|"
							"\nadd_label_with_icon_button|small|`4Ban: 4 Weeks (DGH)|left|732|ban_31|"
							"\nadd_label_with_icon_button|small|`4Ban: Permanent (DGH)|left|732|ban_729|\n"
							"add_spacer|small|\n"
							"end_dialog|punish_view|Close||";
						VarList::OnDialogRequest(peer, d_);
					}
				}
				catch (exception) {
					return;
				}
				if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
			}
		}
		else if (pInfo(peer)->Role.Role_Level == 2) DialogHandle::View_Inventory(peer, pInfo(peer)->last_wrenched);
	}
	static void REMOVEID(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /removeid <item id>");
			return;
		}
		int id = std::atoi(cmd.c_str());
		if (id <= 0) {
			VarList::OnConsoleMessage(peer, "`oInvalid item ID.");
			return;
		}
		World* world_ = find_world(pInfo(peer)->world);
		if (!world_) {
			VarList::OnConsoleMessage(peer, "`oYou are not in a world.");
			return;
		}
		int removed = 0;
		world_->drop_new.erase(std::remove_if(world_->drop_new.begin(), world_->drop_new.end(), [&](const std::vector<int>& drop) {
			if (drop.size() < 5) return false;
			if (drop[0] == id) { removed++; return true; }
			return false;
			}), world_->drop_new.end());
		for (int i = 0; i < (int)world_->drop_new.size(); ++i) {
			world_->drop_new[i][2] = i;
		}
		world_->total_drop_uid = (int)world_->drop_new.size();
		VarList::OnConsoleMessage(peer, "`oRemoved `2" + to_string(removed) + "`` drop(s) of item `2" + to_string(id) + "`` from `" + world_->name + "``.");
	}
	static void INFOWL(ENetPeer* peer, std::string cmd) {
		ENetPeer* target = peer;
		string name = pInfo(peer)->tankIDName;
		if (!cmd.empty()) {
			bool found = false;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
				if (to_lower(pInfo(cp_)->tankIDName) == to_lower(cmd)) {
					target = cp_;
					name = pInfo(cp_)->tankIDName;
					found = true;
					break;
				}
			}
			if (!found) {
				VarList::OnConsoleMessage(peer, "`oPlayer `4" + cmd + "`` not found (offline/typo).");
				return;
			}
		}
		int wl = Inventory::Contains(target, 242);
		int dl = Inventory::Contains(target, 1796);
		int bgl = Inventory::Contains(target, 7188);
		int ggl = Inventory::Contains(target, 8470);
		int mgl = Inventory::Contains(target, 20298);
		string dialog = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`w" + name + "`` - Lock Info|left|242|\n"
			"add_spacer|small|\n"
			"add_textbox|`5`o-- Backpack Locks --``|left|\n"
			"add_label_with_icon|small|`oWorld Lock: `2" + Set_Count(wl) + "|left|242|\n"
			"add_label_with_icon|small|`oDiamond Lock: `2" + Set_Count(dl) + "|left|1796|\n"
			"add_label_with_icon|small|`oSilver Gem Lock: `2" + Set_Count(bgl) + "|left|7188|\n"
			"add_label_with_icon|small|`oMagical Gem Lock: `2" + Set_Count(mgl) + "|left|8470|\n"
			"add_label_with_icon|small|`o" + items[20298].ori_name + ": `2" + Set_Count(mgl) + "|left|20298|\n"
			"add_spacer|small|\n"
			"add_textbox|`5`o-- Bank Locks --``|left|\n"
			"add_label_with_icon|small|`oWorld Lock: `2" + Set_Count(pInfo(target)->wl_bank_amount) + "|left|242|\n"
			"add_label_with_icon|small|`oDiamond Lock: `2" + Set_Count(pInfo(target)->dl_bank_amount) + "|left|1796|\n"
			"add_label_with_icon|small|`oSilver Gem Lock: `2" + Set_Count(pInfo(target)->bgl_bank_amount) + "|left|7188|\n"
			"add_label_with_icon|small|`oMagical Gem Lock: `2" + Set_Count(pInfo(target)->mgl_bank_amount) + "|left|8470|\n"
			"add_label_with_icon|small|`o" + items[20298].ori_name + ": `2" + Set_Count(pInfo(target)->mgl_bank_amount) + "|left|20298|\n"
			"add_spacer|small|\n"
			"add_button|Close|`4Close``|noflags|0|0|\n"
			"end_dialog|infowl_dialog|||";
		VarList::OnDialogRequest(peer, dialog, 0);
	}
	static void WARP(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /warp <world name>");
			return;
		}
		std::string world = cmd;
		transform(world.begin(), world.end(), world.begin(), ::toupper);
		bool passed = true;
		if (pInfo(peer)->Role.Role_Level <= 4) {
			if (pInfo(peer)->warp_time + 15000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) pInfo(peer)->warp_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			else {
				VarList::OnConsoleMessage(peer, "`4Cooldown active`` - You can use  `5/warp`` in `5" + to_string((pInfo(peer)->warp_time + HOME_TIME - (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) / 1000) + "`` seconds.");
				passed = false;
			}
		}
		if (passed) {
			if (pInfo(peer)->world == world) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Sorry, but you are already in the world!");
				return;
			}
			else {
				pInfo(peer)->update = true;
				VarList::OnConsoleMessage(peer, "Magically warping to world `5" + world + "``...");
				Enter_World(peer, world);
			}
		}
	}
	static void WARPTO(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /warpto <player name>");
			return;
		}
		if (Playmods::HasById(pInfo(peer), 139)) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, you can't do that while cursed.", 0, 0);
		}
		else {
			bool found_ = false;
			pInfo(peer)->last_wrenched = cmd;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->invis) continue;
				if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
					if (pInfo(cp_)->world.empty()) {
						found_ = true;
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, this person isn't in a world right now.", 0, 0);
					}
					else if (pInfo(cp_)->world.find("GROWMINES") != std::string::npos) {
						found_ = true;
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, this person can't be bothered", 0, 0);
					}
					else if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->tankIDName)) {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, You can't warpto yourself!", 0, 0);
						found_ = true;
					}
					else {
						found_ = true;
						pInfo(peer)->update = true;
						VarList::OnTextOverlay(peer, "Moving to location of " + get_player_nick(cp_) + " (`2" + pInfo(cp_)->world + "``) ...");
						if (Role::Administrator(peer)) Enter_World(peer, pInfo(cp_)->world, pInfo(cp_)->x / 32, pInfo(cp_)->y / 32);
						else Enter_World(peer, pInfo(cp_)->world);
					}
					break;
				}
			}
			if (found_ == false) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, player was not found.", 0, 0);
			}
		}
	}
	static void VIP_SB(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /vsb <text>");
			return;
		}

		long long now = time(nullptr);
		int remaining = 0;

		for (auto& pm : pInfo(peer)->playmods) {
			if (pm.id == 122) {
				remaining = pm.time - now;
				break;
			}
		}

		if (remaining > 0) {
			VarList::OnConsoleMessage(peer, ">> (" + Time::Playmod(remaining) + " before you can broadcast again)");
			return;
		}

		for (auto it = pInfo(peer)->playmods.begin(); it != pInfo(peer)->playmods.end();) {
			if (it->id == 122) it = pInfo(peer)->playmods.erase(it);
			else ++it;
		}

		PlayMods new_pm{};
		new_pm.id = 122;
		new_pm.time = now + 600;
		pInfo(peer)->playmods.push_back(new_pm);

		CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
		VarList::OnConsoleMessage(peer, "Broadcasting to ALL! (`$VIP SB!``)");

		std::string text = cmd;

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			if (pInfo(cp_)->radio) continue;

			VarList::OnConsoleMessage(
				cp_,
				"CP:_PL:0_OID:_CT:[SB]_ `e[VIP-SB] `5** from (`$" +
				get_player_nick(peer) +
				"`5) in [`4HIDDEN!`5] ** : `$" +
				text + "``"
			);
		}
	}
	static void SEARCH_USER(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /searchuser <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd; bool found = false;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label|big|`w" + pInfo(cp_)->name_color + pInfo(cp_)->tankIDName + " `2Found|left|\nadd_spacer|small|\nadd_textbox|`oStatus: `2Online|left|\nadd_textbox|`o" + pInfo(cp_)->tankIDName + " is now in World: `2" + (pInfo(cp_)->world.empty() ? "EXIT" : pInfo(cp_)->world) + ".|left|\nadd_button|warp_to_" + pInfo(cp_)->world + "|`oWarp To User in `w(`5" + pInfo(cp_)->world + "`o)|0|0|\nadd_quick_exit|\nadd_spacer|small|\nend_dialog|top|Close||");
				found = true;
				break;
			}
		}
		if (not found) VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label|big|`w" + pInfo(peer)->last_wrenched + " `4Not Found|left|\nadd_spacer|small|\nadd_textbox|`oStatus: `4Offline|left|\nadd_textbox|`o" + pInfo(peer)->last_wrenched + " is now in World: `4EXIT.|left|\nadd_quick_exit|\nadd_spacer|small|\nend_dialog|cl0se|Close||");
	}
	static void SETCLIST(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /setclist <player name>");
			return;
		}

		std::string name = cmd;
		std::ifstream ifs("database/players/" + name + "_.json");
		if (!ifs.is_open()) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
			return;
		}

		json j;
		ifs >> j;
		ifs.close();

		if (!j.contains("name") || !j["name"].is_string()) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players data invalid!", 0, 0);
			return;
		}

		const std::string targetName = j["name"].get<std::string>();

		auto apply_live_clist_state = [&](ENetPeer* targetPeer, bool added) {
			if (!targetPeer || targetPeer->state != ENET_PEER_STATE_CONNECTED || targetPeer->data == NULL) return;

			pInfo(targetPeer)->Role.Coder = added;
			pInfo(targetPeer)->name_color = Role::Prefix(targetPeer);

			VisualHandle::Nick(targetPeer, NULL);
			Clothing_V2::Update_Value(targetPeer);
			Clothing_V2::Update(targetPeer);

			if (!pInfo(targetPeer)->world.empty()) {
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
					if (pInfo(cp_)->world != pInfo(targetPeer)->world) continue;

					VisualHandle::Nick(cp_, targetPeer);
				}
			}

			VarList::OnAddNotification(
				targetPeer,
				added
				? "You has been `2added ``to Creator List by System"
				: "You has been `4removed ``from Creator List by System",
				added ? "interface/science_button.rttex" : "interface/atomic_button.rttex",
				"audio/hub_open.wav"
			);
			};

		auto it = std::find(C_List.clist.begin(), C_List.clist.end(), targetName);
		const bool alreadyAdded = (it != C_List.clist.end());

		if (alreadyAdded) {
			C_List.clist.erase(std::remove(C_List.clist.begin(), C_List.clist.end(), targetName), C_List.clist.end());
			VarList::OnConsoleMessage(peer, "`o>> Successfully removed " + targetName + " from list.");
			EventPool::Save::Config();

			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
				if (pInfo(cp_)->tankIDName != targetName) continue;

				apply_live_clist_state(cp_, false);
			}
		}
		else {
			C_List.clist.push_back(targetName);
			VarList::OnConsoleMessage(peer, "`o>> Successfully added " + targetName + " to list.");
			EventPool::Save::Config();

			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
				if (pInfo(cp_)->tankIDName != targetName) continue;

				apply_live_clist_state(cp_, true);
			}
		}

		j.clear();
	}
	static void CLIST(ENetPeer* peer) {
		std::string clist = "";
		for (int w_ = 0; w_ < C_List.clist.size(); w_++) clist += C_List.clist[w_] + ", ";
		if (clist.empty()) clist = "None!";
		VarList::OnConsoleMessage(peer, "`oCreator List: `2" + clist + "");
	}
	static void STOP_SERVER(ENetPeer* peer) {
		ServerPool::Trigger();
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			ServerPool::Trigger();
			VarList::OnConsoleMessage(cp_, "`5Server is saving and shutting down...");
			Peer_Disconnect(cp_, 0);
		}
	}
	static void RESET_LOCK(ENetPeer* peer) {
		namespace fs = std::filesystem;
		int onlineReset = 0, offlineReset = 0, worldReset = 0;

		// helper: check if a string exists in a vector (replaces std::set::count)
		auto vecContains = [](const std::vector<std::string>& v, const std::string& s) -> bool {
			for (const auto& x : v) if (x == s) return true;
			return false;
			};
		// helper: check if item ID is a lock type (safe bounds check)
		auto isLockItem = [](int id) -> bool {
			return id > 0 && id < (int)items.size() && items[id].blockType == BlockTypes::LOCK;
			};

		// ============================================================
		// STEP 1 : Online players – inventory, extra backpack, bank
		// ============================================================
		std::vector<std::string> onlineNames;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;

			// --- bank central ---
			pInfo(cp_)->wl_bank_amount = 0;
			pInfo(cp_)->dl_bank_amount = 0;
			pInfo(cp_)->bgl_bank_amount = 0;
		pInfo(cp_)->mgl_bank_amount = 0;

		pInfo(cp_)->mgl_bank_amount = 0;


			// --- inventory: collect lock IDs first, then remove ---
			if (!pInfo(cp_)->inv.empty()) {
				std::vector<int> lockIds;
				for (auto& inv : pInfo(cp_)->inv) {
					if (isLockItem(inv.first)) lockIds.push_back(inv.first);
				}
				for (int lid : lockIds) {
					int cnt = Inventory::Contains(cp_, lid);
					if (cnt > 0) {
						int neg = -cnt;
						Inventory::Modify(cp_, lid, neg);
					}
				}
			}

			// --- extra backpack ---
			auto& bp = pInfo(cp_)->backpack;
			std::vector<BackpackItem> cleanBp;
			for (auto& b : bp) {
				if (!isLockItem(b.itemID)) cleanBp.push_back(b);
			}
			bp = cleanBp;

			// --- persist ---
			ServerPool::SaveDatabase::Players(pInfo(cp_), false);
			onlineNames.push_back(to_lower(pInfo(cp_)->tankIDName));
			onlineReset++;
		}

		// ============================================================
		// STEP 2 : Offline player JSON files
		// ============================================================
		const char* bankKeys[] = {
		"wl_bank_amount","dl_bank_amount","bgl_bank_amount","mgl_bank_amount",
		"mgl_bank_amount",
		};
		const int bankKeyCount = 18;

		if (fs::exists("database/players/")) {
			for (const auto& entry : fs::directory_iterator("database/players/")) {
				if (!entry.is_regular_file()) continue;
				std::string fname = entry.path().filename().string();
				if (fname.size() < 7) continue;
				std::string ext6 = fname.substr(fname.size() - 6);
				if (ext6 != "_.json") continue;
				std::string pname = fname.substr(0, fname.size() - 6);
				if (vecContains(onlineNames, to_lower(pname))) continue;

				try {
					std::ifstream ifs(entry.path().string());
					if (!ifs.is_open()) continue;
					json jdata = json::parse(ifs);
					ifs.close();

					bool modified = false;

					// -- inventory: array of [itemId, count] --
					if (jdata.contains("inventory") && jdata["inventory"].is_array()) {
						json newInv = json::array();
						for (auto& slot : jdata["inventory"]) {
							if (slot.is_array() && slot.size() >= 2) {
								int itemId = slot[0].get<int>();
								if (!isLockItem(itemId)) {
									newInv.push_back(slot);
								}
								else { modified = true; }
							}
							else { newInv.push_back(slot); }
						}
						if (modified) jdata["inventory"] = newInv;
					}

					// -- extra backpack: array of {"itemID":x,"amount":y} --
					if (jdata.contains("backpack") && jdata["backpack"].is_array()) {
						json newBp = json::array();
						for (auto& bslot : jdata["backpack"]) {
							int itemId = bslot.value("itemID", 0);
							if (!isLockItem(itemId)) {
								newBp.push_back(bslot);
							}
							else { modified = true; }
						}
						jdata["backpack"] = newBp;
					}

					// -- bank central --
					for (int ki = 0; ki < bankKeyCount; ki++) {
						if (jdata.contains(bankKeys[ki])) {
							int32_t val = jdata[bankKeys[ki]].get<int32_t>();
							if (val != 0) { jdata[bankKeys[ki]] = 0; modified = true; }
						}
					}

					if (modified) {
						std::ofstream ofs(entry.path().string());
						if (ofs.is_open()) {
							ofs << jdata.dump(4);
							ofs.close();
							offlineReset++;
						}
					}
				}
				catch (...) {}
			}
		}

		// ============================================================
		// STEP 3 : Loaded worlds – drop_new & lock blocks (in-memory)
		// ============================================================
		std::vector<std::string> loadedWorldNames;
		for (auto& world_ : worlds) {
			loadedWorldNames.push_back(world_.name);

			bool worldModified = false;

			// Hapus lock item yang di-drop di world
			auto& drops = world_.drop_new;
			size_t beforeSz = drops.size();
			drops.erase(std::remove_if(drops.begin(), drops.end(),
				[](const std::vector<int>& d) {
					return !d.empty() && d[0] > 0 && d[0] < (int)items.size() &&
						items[d[0]].blockType == BlockTypes::LOCK;
				}), drops.end());
			if (drops.size() != beforeSz) worldModified = true;

			// Ganti lock block yang terpasang di world -> item 242 (World Lock)
			for (auto& block : world_.blocks) {
				if (block.fg > 0 && block.fg < (int)items.size() &&
					items[block.fg].blockType == BlockTypes::LOCK) {
					block.fg = 242;
					block.owner_name = "";
					block.owner_named = "";
					block.admins.clear();
					block.locked = false;
					block.lock_origin = -1;
					worldModified = true;
				}
			}

			// Tulis langsung ke file world agar perubahan persisten
			if (worldModified) {
				try {
					std::string safe_name = sanitize_world_name(world_.name);
					std::string wpath = "database/worlds/" + safe_name + "_.json";
					// Serialisasi ulang lewat json (bukan save_world agar tidak erase)
					// Baca file lama lalu replace blok & drop_new saja
					std::ifstream ifs2(wpath);
					if (ifs2.is_open()) {
						json wj = json::parse(ifs2);
						ifs2.close();

						// Rebuild drop_new dari in-memory (sudah bersih)
						wj["drop_new"] = world_.drop_new;

						// Rebuild blocks array dari in-memory
						if (wj.contains("blocks") && wj["blocks"].is_array()) {
							for (size_t bi = 0; bi < wj["blocks"].size() && bi < world_.blocks.size(); bi++) {
								auto& jblk = wj["blocks"][bi];
								const auto& mblk = world_.blocks[bi];
								if (jblk.contains("f") && mblk.fg == 242) {
									int origFg = jblk["f"].get<int>();
									if (origFg > 0 && origFg < (int)items.size() &&
										items[origFg].blockType == BlockTypes::LOCK) {
										jblk["f"] = 242;
										jblk.erase("lo");
										jblk.erase("l");
										jblk.erase("lq");
										jblk.erase("a");
									}
								}
							}
						}

						std::ofstream ofs2(wpath);
						if (ofs2.is_open()) {
							ofs2 << wj.dump(4) << std::endl;
							ofs2.close();
						}
					}
				}
				catch (...) {}
				worldReset++;
			}
		}

		// ============================================================
		// STEP 4 : Unloaded world JSON files
		// ============================================================
		if (fs::exists("database/worlds/")) {
			for (const auto& entry : fs::directory_iterator("database/worlds/")) {
				if (!entry.is_regular_file()) continue;
				std::string fname = entry.path().filename().string();
				if (fname.size() < 7) continue;
				std::string ext6 = fname.substr(fname.size() - 6);
				if (ext6 != "_.json") continue;
				std::string wname = fname.substr(0, fname.size() - 6);
				// Skip world yang sudah di-proses (loaded)
				if (vecContains(loadedWorldNames, wname)) continue;

				try {
					std::ifstream ifs(entry.path().string());
					if (!ifs.is_open()) continue;
					json wj = json::parse(ifs);
					ifs.close();

					bool modified = false;

					// Hapus lock item dari drop_new
					if (wj.contains("drop_new") && wj["drop_new"].is_array()) {
						json newDrops = json::array();
						for (auto& drop : wj["drop_new"]) {
							if (drop.is_array() && !drop.empty()) {
								int itemId = drop[0].get<int>();
								if (itemId <= 0 || itemId >= (int)items.size() ||
									items[itemId].blockType != BlockTypes::LOCK) {
									newDrops.push_back(drop);
								}
								else { modified = true; }
							}
							else { newDrops.push_back(drop); }
						}
						if (modified) wj["drop_new"] = newDrops;
					}

					// Ganti lock block -> 242
					if (wj.contains("blocks") && wj["blocks"].is_array()) {
						for (auto& blk : wj["blocks"]) {
							if (blk.contains("f")) {
								int fg = blk["f"].get<int>();
								if (fg > 0 && fg < (int)items.size() &&
									items[fg].blockType == BlockTypes::LOCK) {
									blk["f"] = 242;
									blk.erase("lo");
									blk.erase("l");
									blk.erase("lq");
									blk.erase("a");
									modified = true;
								}
							}
						}
					}

					if (modified) {
						std::ofstream ofs(entry.path().string());
						if (ofs.is_open()) {
							ofs << wj.dump(4) << std::endl;
							ofs.close();
							worldReset++;
						}
					}
				}
				catch (...) {}
			}
		}

		// ============================================================
		// Laporan akhir
		// ============================================================
		VarList::OnConsoleMessage(peer,
			"`2[RESETLOCK]`` Selesai! Online: `w" + to_string(onlineReset) +
			"`` | Offline: `w" + to_string(offlineReset) +
			"`` | Worlds: `w" + to_string(worldReset) + "``");
		ServerPool::Logs::Add(
			pInfo(peer)->tankIDName + " used /resetlock (online=" + to_string(onlineReset) +
			" offline=" + to_string(offlineReset) + " worlds=" + to_string(worldReset) + ")",
			"ResetLock");
	}
	static void SETEVENT(ENetPeer* peer) {
		DialogHandle::SetEvent(peer);
	}
	static void WOTD(ENetPeer* peer) {
		std::string world_owner = get_world(pInfo(peer)->world).owner_name;
		if (world_owner.empty()) VarList::OnConsoleMessage(peer, "`oCannot add new WOTD (owner name is empty)");
		else {
			std::vector<std::string> wotd_found;
			for (const auto& wlist : Environment()->WOTD) {
				wotd_found.push_back(wlist.first);
			}
			if (find(wotd_found.begin(), wotd_found.end(), pInfo(peer)->world) != wotd_found.end()) {
				VarList::OnConsoleMessage(peer, "`oCannot add new WOTD (this world already existed)");
				return;
			}
			std::string name_ = pInfo(peer)->world;
			auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (it != worlds.end()) {
				World* world_ = &(*it);
				world_->world_settings &= ~SETTINGS::SETTINGS_1;
				world_->world_settings |= SETTINGS::SETTINGS_1;
			}
			save_world(name_, false);
			gamepacket_t p2, p3;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				VarList::OnConsoleMessage(cp_, "WOTW Winner has been announncement, `0" + pInfo(peer)->world + "`o is the winner of `9WORLD OF THE DAY!");
				if (pInfo(cp_)->world == pInfo(peer)->world) {
					VarList::OnAddNotification(cp_, "`0Congratulation!`o, this world is the winner of `9WORLD OF THE DAY", "interface/large/special_event.rttex", "");
					CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/cumbia_horns.wav", 0);
				}
			}
			Environment()->WOTD.clear();
			Environment()->WOTD.push_back(make_pair(pInfo(peer)->world, world_owner));
		}
	}
	static void WOTW(ENetPeer* peer) {
		std::string world_owner = get_world(pInfo(peer)->world).owner_name;
		if (world_owner.empty()) VarList::OnConsoleMessage(peer, "`oCannot add new WOTD (owner name is empty)");
		else {
			std::vector<std::string> wotw_found;
			for (const auto& wlist : Environment()->WOTW) {
				wotw_found.push_back(wlist.first);
			}
			if (find(wotw_found.begin(), wotw_found.end(), pInfo(peer)->world) != wotw_found.end()) {
				VarList::OnConsoleMessage(peer, "`oCannot add new WOTW (this world already existed)");
				return;
			}
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				VarList::OnConsoleMessage(cp_, "WOTW Winner has been announncement, `0" + pInfo(peer)->world + "`o is the winner of `9WORLD OF THE WEEK!");
				if (pInfo(cp_)->world == pInfo(peer)->world) {
					VarList::OnAddNotification(cp_, "`0Congratulation!`o, this world is the winner of `9WORLD OF THE WEEK", "interface/large/special_event.rttex", "");
					CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/cumbia_horns.wav", 0);
				}
			}
			Environment()->WOTW.clear();
			Environment()->WOTW.push_back(make_pair(pInfo(peer)->world, world_owner));
		}
	}
	static void MAINTENANCE(ENetPeer* peer) {
		if (Environment()->Maintenance or Environment()->Restart_Status) {
			Environment()->Maintenance = false;
			Environment()->Restart_Status = false;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				VarList::OnAddNotification(cp_, "Maintenance Status : False", "interface/science_button.rttex", "audio/hub_open.wav");
				VarList::OnConsoleMessage(cp_, "`2[Server]`o Maintenance has ended! Server is open.");
			}
		}
		else {
			Environment()->Maintenance = true;
			Environment()->Restart_Status = true;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->tankIDName.empty()) {
					enet_peer_disconnect_later(cp_, 0);
					continue;
				}
				if (Role::Clist(pInfo(cp_)->tankIDName) or Role::Administrator(cp_) or world_access_bypass(cp_)) {
					VarList::OnAddNotification(cp_, "Maintenance Status : True", "interface/science_button.rttex", "audio/hub_open.wav");
					VarList::OnConsoleMessage(cp_, "`5[Server]`o Maintenance is ACTIVE. Non-admin players have been disconnected.");
				}
				else {
					VarList::OnConsoleMessage(cp_, "`5[Server]`o Server is under Maintenance. Disconnecting...");
					enet_peer_disconnect_later(cp_, 0);
				}
			}
		}
	}
	static void SETRCODE(ENetPeer* peer) {
		DialogHandle::RedeemCode(peer);
	}
	static void SETNEWGET(ENetPeer* peer) {
		DialogHandle::NewbieGet(peer);
	}
	static void NEWGET(ENetPeer* peer) {
		std::string list_items = "";
		std::string list_details = "";
		int total_items = 0;
		int total_gems = 0;
		int total_coins = 0;
		int total_tokens = 0;

		for (int c_ = 0; c_ < (int)new_get.list.size(); c_++) {
			total_gems += new_get.list[c_].Gemss;
			total_coins += new_get.list[c_].Coin;
			total_tokens += new_get.list[c_].Seasonals_Token;
			for (const auto& it_ : new_get.list[c_].items) {
				std::string item_name = items[it_.first].name;
				list_items += "\nadd_button_with_icon||`2" + item_name + "|staticYellowFrame|" + to_string(it_.first) + "|" + to_string(it_.second) + "|";
				total_items++;
				if (total_items % 5 == 0) {
					list_items += "\nadd_custom_break|";
				}
				
				std::string name_color = "`w";
				if (item_name.find("Legendary") != std::string::npos || item_name.find("Blood Wings") != std::string::npos) {
					name_color = "`4";
				}
				list_details += "add_textbox|- `2" + to_string(it_.second) + "x " + name_color + item_name + "|left|\n";
			}
		}

		if (total_gems > 0) {
			list_details += "add_textbox|- `2" + to_string(total_gems) + "x `wGems|left|\n";
		}
		if (total_coins > 0) {
			list_details += "add_textbox|- `2" + to_string(total_coins) + "x `w" + Environment()->server_name + " Coin|left|\n";
		}
		if (total_tokens > 0) {
			list_details += "add_textbox|- `2" + to_string(total_tokens) + "x `w" + guild_event_type + " Tokens|left|\n";
		}

		std::string dialog = SetColor(peer) + 
			"set_default_color|`o\n"
			"set_bg_color|0,52,102,178|\n"
			"set_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wNewbie Supplies``|left|12436|\n"
			"add_spacer|small|\n"
			"add_textbox|Here is the list of starter items given to every new player who registers on `2" + Environment()->server_name + "`o:|left|\n"
			"add_spacer|small|\n";

		if (total_items > 0) {
			dialog += "add_textbox|`wStarter Items:``|left|\n"
				"text_scaling_string|Day 30 +9999999999|\n" + list_items + "\nadd_button_with_icon||END_LIST|noflags|0||\n"
				"add_spacer|small|\n";
		}

		if (!list_details.empty()) {
			dialog += "add_textbox|`wItem List Details:``|left|\n" + list_details + "add_spacer|small|\n";
		} else if (total_items == 0) {
			dialog += "add_textbox|`4No starter items configured.``|left|\n";
		}

		dialog += "end_dialog|newget_display|Close||\nadd_quick_exit|";
		VarList::OnDialogRequest(peer, dialog);
	}
	static void SETGIVEAWAY(ENetPeer* peer) {
		DialogHandle::Giveaway(peer);
	}
	static void SETWORLD(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSet World Default|left|32|\nadd_spacer|small|\nadd_textbox|Foreground:|left|\nadd_label_with_icon|small|`2" + items[WorldDefault.fg].name + "|left|" + to_string(WorldDefault.fg) + "|\nadd_text_input|fg||" + to_string(WorldDefault.fg) + "|5|\nadd_spacer|small|\nadd_textbox|Background:|left|\nadd_label_with_icon|small|`2" + items[WorldDefault.bg].name + "|left|" + to_string(WorldDefault.bg) + "|\nadd_text_input|bg||" + to_string(WorldDefault.bg) + "|5|\nadd_spacer|small|\nadd_textbox|Rock:|left|\nadd_label_with_icon|small|`2" + items[WorldDefault.rock].name + "|left|" + to_string(WorldDefault.rock) + "|\nadd_text_input|rock||" + to_string(WorldDefault.rock) + "|5|\nadd_spacer|small|\nadd_textbox|Lava:|left|\nadd_label_with_icon|small|`2" + items[WorldDefault.lava].name + "|left|" + to_string(WorldDefault.lava) + "|\nadd_text_input|lava||" + to_string(WorldDefault.lava) + "|5|\nadd_spacer|small|\nadd_textbox|Weather:|left|\nadd_text_input|weather||" + to_string(WorldDefault.weather) + "|3|\nend_dialog|World_Default|Close|Update|\nadd_quick_exit|");
	}
	static void RESET_LEADERBOARD(ENetPeer* peer) {
		/*Punch Leaderboard*/top_punch_list.clear(), top_punch.clear();
		/*Rich Leaderboard*/top_rich_list.clear(), top_richest.clear();
		/*Online Leaderboard*/top_online.clear(), top_on_list.clear();
		/*Daily Challange Leaderboard*/Event()->top_dailyc.clear(), Event()->top_dailyc_list.clear();
		/*Party Animal Leaderboard*/Event()->top_anniversary.clear(), Event()->top_anniversary_list.clear();
		VarList::OnConsoleMessage(peer, "`2Successfully reset all Leaderboard except (Guilds).");
	}
	static void EDIT_STORE_STOCK(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wEdit Store Stock|left|5956|\nadd_spacer|small|\nmax_checks|3|\ntext_scaling_string|DEFIBRILLATOR|\nadd_checkicon|item1||staticYellowFrame,no_padding_x|5930||0|\nadd_custom_margin|x:20;y:0|\nadd_textbox|'LEGENDARY DEVIL MAGPLANT'|left|\nadd_textbox|CURRENT STOCK: [`2" + Set_Count(Environment()->Stock_Mag) + "``]|left|\nreset_placement_x|\nadd_button_with_icon||END_ROW|noflags|0||\nadd_checkicon|item2||staticYellowFrame,no_padding_x|9770||0|\nadd_custom_margin|x:20;y:0|\nadd_textbox|'GOLDEN RAYMAN'S FIST'|left|\nadd_textbox|CURRENT STOCK: [`2" + Set_Count(Environment()->Stock_GRay) + "``]|left|\nreset_placement_x|\nadd_button_with_icon||END_ROW|noflags|0||\nadd_checkicon|item3||staticYellowFrame,no_padding_x|9778||0|\nadd_custom_margin|x:20;y:0|\nadd_textbox|'DARK EVIL RAYMAN'S FIST'|left|\nadd_textbox|CURRENT STOCK: [`2" + Set_Count(Environment()->Stock_ERay) + "``]|left|\nreset_placement_x|\nadd_button_with_icon||END_ROW|noflags|0||\nadd_text_input|Amount|Amount:||3|\nend_dialog|EditStore_Stock|Cancel|Update|\nadd_quick_exit|");
	}
	static void GIVE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /give <player name> <item name or ID> <quantity>");
			return;
		}

		std::istringstream stream(cmd);
		std::string playerName, itemStr;
		int quantity = 0, itemID = -1;

		if (!(stream >> playerName >> std::ws)) {
			VarList::OnConsoleMessage(peer, "`4Invalid command format. Usage: /give <name> <item name or ID> <amount>");
			return;
		}

		std::string remaining;
		getline(stream, remaining);
		size_t pos = remaining.find_last_of(' ');
		if (pos == std::string::npos) {
			VarList::OnConsoleMessage(peer, "`4Invalid command format. Usage: /give <name> <item name or ID> <amount>");
			return;
		}

		itemStr = remaining.substr(0, pos);
		std::string amountStr = remaining.substr(pos + 1);

		try {
			quantity = std::stoi(amountStr.c_str());
			if (quantity <= 0) {
				VarList::OnConsoleMessage(peer, "`oQuantity must be more than 0!");
				return;
			}

			if (is_number(itemStr)) {
				itemID = std::stoi(itemStr);
				if (itemID < 0 || itemID >= items.size()) {
					VarList::OnConsoleMessage(peer, "`4Item ID not valid.");
					return;
				}
			}
			else {
				for (int i = 0; i < items.size(); ++i) {
					if (to_lower(items[i].name) == to_lower(itemStr)) {
						itemID = i;
						break;
					}
				}
				if (itemID == -1) {
					VarList::OnConsoleMessage(peer, "`4Item not found! Use a more complete name or ID.");
					return;
				}
			}

			bool found = false;
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
				if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(playerName)) {
					found = true;
					if (Inventory::Modify(currentPeer, itemID, quantity) == -1) {
						VarList::OnConsoleMessage(peer, "`4Failed to give item, inventory full.");
					}
					else {
						VarList::OnConsoleMessage(currentPeer, "`2You received `w" + std::to_string(quantity) + " " + items[itemID].name + "`.");
						VarList::OnConsoleMessage(peer, "`oGave `w" + std::to_string(quantity) + " " + items[itemID].name + "` to `" + pInfo(currentPeer)->tankIDName + "`");

						PlayerMoving data_{};
						data_.packetType = 19, data_.plantingTree = 500, data_.netID = pInfo(currentPeer)->netID;
						data_.punchX = itemID, data_.punchY = itemID;
						int32_t to_netid = pInfo(peer)->netID;
						BYTE* raw = packPlayerMoving(&data_);
						raw[3] = 3;
						Memory_Copy(raw + 8, &to_netid, 4);
						for (ENetPeer* cp_2 = server->peers; cp_2 < &server->peers[server->peerCount]; ++cp_2) {
							if (cp_2->state != ENET_PEER_STATE_CONNECTED || cp_2->data == NULL) continue;
							if (pInfo(cp_2)->world == pInfo(peer)->world) {
								send_raw(cp_2, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
							}
						}
						delete[] raw;
					}
				}
			}

			if (!found) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Player not found!", 0, 0);
			}

		}
		catch (...) {
			VarList::OnConsoleMessage(peer, "`4Invalid quantity input.");
		}
	}
	static void ADD_PRICE_TRADE_SCAN(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /addpscan <itemid> <price>");
			return;
		}
		std::istringstream ss(cmd);
		int i_, c_;
		try {
			if (ss >> i_ >> c_) {
				if (i_ <= 0 || i_ >= items.size() || items[i_].blockType == BlockTypes::SEED || i_ == 242 || i_ == 1796 || i_ == 7188 || i_ == 5980) {
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Invaild item id", 0, 0);
					return;
				}
				int previous_price = items[i_].scan_price;
				items[i_].scan_price = c_;
				if (previous_price == 0) VarList::OnConsoleMessage(peer, "Added item `2" + items[i_].ori_name + "`` with price `2" + Set_Count(items[i_].scan_price) + "`` World Locks to trade-scan.");
				else VarList::OnConsoleMessage(peer, "Change Price item `2" + items[i_].ori_name + "`` to >> `$" + Set_Count(items[i_].scan_price) + "`` World Locks. (previous price `$" + Set_Count(previous_price * 3) + "``!)");
				ServerPool::Load::TradeScan_Price(true);
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /addpscan <itemid> <price>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /addpscan <itemid> <price>");
		}
	}
	static void DELETE_WORLD(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /deleteworld <name world>");
			return;
		}
		std::string world = cmd;
		transform(world.begin(), world.end(), world.begin(), ::toupper);
		std::string folderPath = "database/worlds/";
		fs::path filePath = folderPath + world + "_.json";
		if (fs::exists(filePath)) {
			std::erase_if(worlds, [world](World const& worldInfo) { return worldInfo.name == world; });
			std::filesystem::remove(filePath);
			VarList::OnConsoleMessage(peer, "`oSuccessfully deleted world " + world + "!");
			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " Delete World: " + world + ".", "Delete World");
			ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, " Delete World " + world + " from database.", "");
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == world) {
					Exit_World(cp_);
					VarList::OnAddNotification(cp_, "`wWarning from `4System`w: This world has been `4DELETED", "interface/atomic_button.rttex", "audio/hub_open.wav");
				}
			}
		}
	}
	static void DELETE_ACC(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /deleteacc <player name>");
			return;
		}
		std::string player = cmd;
		std::string folderPath = "database/players/";
		fs::path filePath = folderPath + player + "_.json";
		if (fs::exists(filePath)) {
			fs::remove(filePath);
			VarList::OnConsoleMessage(peer, "`oSuccessfully deleted " + player + " from the database");
			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " Delete Players: " + player + ".", "Delete Players");
			ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, " Delete Player " + player + " from database.", "");
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->tankIDName == player) {
					VarList::OnAddNotification(cp_, "`wWarning from `4System`w: Your account has been `4DELETED", "interface/atomic_button.rttex", "audio/hub_open.wav");
					Peer_Disconnect(cp_, 0);
				}
			}
		}
	}
	static void CHECK_PASS(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /checkpass <player name>");
			return;
		}
		bool foundacc = true;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				VarList::OnConsoleMessage(peer, "`o>> Player Password [`2" + pInfo(cp_)->tankIDPass + "`o], 2FA CODE: [`2" + to_string(pInfo(cp_)->fa2) + "`o] STATUS: `2ONLINE.");
				foundacc = false;
				break;
			}
			if (foundacc) {
				bool found = false;
				try {
					std::string name = pInfo(peer)->last_wrenched;
					std::ifstream ifs("database/players/" + name + "_.json");
					if (ifs.is_open()) {
						json j;
						ifs >> j;
						found = true;
						VarList::OnConsoleMessage(peer, "`o>> Player Password [`2" + j["pass"].get<string>() + "`o], 2FA CODE: [`2" + to_string(j["2fa"].get<int>()) + "`o] STATUS: `4OFFLINE.");
						j.clear();
						break;
					}
				}
				catch (exception) {
					return;
				}
				if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
			}
		}
	}
	static void GROLE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /grole <player name>");
			return;
		}
		bool found = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				found = true;
				pInfo(peer)->last_wrenched = pInfo(cp_)->tankIDName;
				std::string grole_dlg = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wGive / Remove - Role (" + pInfo(cp_)->tankIDName + ")|left|15042|\nadd_spacer|small|\nadd_textbox|`5- Role Info -``|left|\nadd_smalltext|`o- VIP: `5[" + (Role::Vip(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- MODERATOR: `5[" + (Role::Moderator(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- ADMINISTRATOR: `5[" + (Role::Administrator(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- DEVELOPER: `5[" + (Role::Developer(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- Donatur: `5[" + (Role::Donatur(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- UNLIMITED: `5[" + (Role::Unlimited(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- Coder: `5[" + (Role::Coder(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- STAFF: `5[" + (Role::Staff(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- Streamers: `5[" + (Role::Streamers(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- GOD: `5[" + (Role::God(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- OWNER SERVER: `5[" + (Role::Owner(cp_) ? "`2YES" : "`4NO") + "`5]<CR>`o- CHEATER: `5[" + (Role::Cheater(cp_) or Playmods::HasById(pInfo(cp_), 143) ? "`2YES``" : "`4NO") + "`5]<CR>`o- BOOST: `5[" + (pInfo(cp_)->Role.BOOST ? "`2YES``" : "`4NO") + "`5]<CR>`o- SUPER-BOOST: `5[" + (pInfo(cp_)->Role.SUPER_BOOST ? "`2YES``" : "`4NO") + "`5]|left|\nadd_spacer|small|\nadd_textbox|`5Role Checkbox:|left|\nadd_checkbox|role_1|`w[`1VIP`w] ROLE|" + to_string(Role::Vip(cp_)) + "|\nadd_checkbox|role_2|`#@MODERATOR ROLE|" + to_string(Role::Moderator(cp_)) + "|\nadd_checkbox|role_3|`e@ADMINISTRATOR ROLE|" + to_string(Role::Administrator(cp_)) + "|\nadd_checkbox|role_4|`6@DEVELOPER ROLE|" + to_string(Role::Donatur(cp_)) + "|\nadd_checkbox|role_baik|`c@Donatur Role|" + to_string(Role::Donatur(cp_)) + "|\nadd_checkbox|role_9|`9@GOD ROLE|" + to_string(Role::God(cp_)) + "|\nadd_checkbox|role_10|`8@UNLIMITED ROLE|" + to_string(Role::Unlimited(cp_)) + "|\nadd_checkbox|role_11|`4@STAFF ROLE|" + to_string(Role::Staff(cp_)) + "|\nadd_checkbox|role_12|`0[`4Streamers`0]ROLE``|" + to_string(Role::Streamers(cp_)) + "|\nadd_checkbox|role_13|`^@CODER ROLE|" + to_string(Role::Coder(cp_)) + "|\nadd_checkbox|role_5|`b@OWNER SERVER ROLE|" + to_string(Role::Owner(cp_)) + "|\nadd_checkbox|role_6|CHEAT ROLE|" + to_string(Role::Cheater(cp_)) + "|\nadd_checkbox|role_7|`5[BOOST]`` ROLE|" + to_string(pInfo(cp_)->Role.BOOST) + "|\nadd_checkbox|role_8|`8[SUPER-BOOST]`` ROLE|" + to_string(pInfo(cp_)->Role.SUPER_BOOST) + "|\nadd_spacer|small|\nadd_textbox|`5Custom Role (/editrole):|left|\nadd_spacer|tiny|";
				{
					std::lock_guard<std::mutex> lk(g_customRoles_mtx);
					if (g_customRoles.empty()) {
						grole_dlg += "\nadd_textbox|`4No custom roles found. Use /editrole to create one.|left|";
					}
					else {
						for (size_t _ri = 0; _ri < g_customRoles.size(); _ri++) {
							auto& _r = g_customRoles[_ri];
							int _chk = (to_lower(pInfo(cp_)->Role.custom_role_name) == to_lower(_r.name)) ? 1 : 0;
							grole_dlg += "\nadd_checkbox|custom_role_" + std::to_string(_ri) + "|" + _r.prefix + _r.name + "|" + std::to_string(_chk) + "|";
						}
					}
					int _clr = pInfo(cp_)->Role.custom_role_name.empty() ? 1 : 0;
					grole_dlg += "\nadd_checkbox|custom_role_clear|`4[None / Clear]|" + std::to_string(_clr) + "|";
				}
				grole_dlg += "\nadd_custom_button|Nevermind|textLabel:`wNevermind;middle_colour:2415764;border_colour:2415764;|"
					"\nadd_custom_button|Apply Changes|textLabel:`wApply Changes;anchor:_button_Nevermind;left:1;margin:60,0;middle_colour:434431310;border_colour:434438350;|"
					"\nend_dialog|give_role|||\nadd_quick_exit|";
				VarList::OnDialogRequest(peer, grole_dlg);
				break;
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void GASSETS(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /gassets <player name>");
			return;
		}
		bool found = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				found = true;
				pInfo(peer)->last_wrenched = pInfo(cp_)->tankIDName;
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wGive / Remove - Assets (" + pInfo(cp_)->tankIDName + ")|left|15054|\nadd_spacer|small|\nadd_textbox|`5- Assets Info -``|left|\nadd_smalltext|`o- Gems: `5[`2" + Set_Count(pInfo(cp_)->gems) + "``]|left|\nadd_smalltext|`o- Level: `5[`2" + Set_Count(pInfo(cp_)->level) + "``]|left|\nadd_smalltext|`o- Online Point Currency: `5[`2" + Set_Count(pInfo(cp_)->opc) + "``]|left|\nadd_smalltext|`o- " + Environment()->server_name + " Coins: `5[`2" + Set_Count(pInfo(cp_)->gtwl) + "``]|left|\nadd_smalltext|`o- Supporter: `5[" + (pInfo(cp_)->supp >= 1 ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- Super Supporter: `5[" + (pInfo(cp_)->supp >= 2 ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- GrowPass: `5[" + (pInfo(cp_)->gp ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- Road to Glory: `5[" + (pInfo(cp_)->glo ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- Piggy Bank: `5[" + (pInfo(cp_)->pg_bank ? "`2YES" : "`4NO") + "``]|left|\nadd_spacer|small|\nadd_textbox|`5- Bank Info -``|left|\nadd_smalltext|`o- WL Bank: `5[`2" + Set_Count(pInfo(cp_)->wl_bank_amount) + "``]|left|\nadd_smalltext|`o- DL Bank: `5[`2" + Set_Count(pInfo(cp_)->dl_bank_amount) + "``]|left|\nadd_smalltext|`o- bgl Bank: `5[`2" + Set_Count(pInfo(cp_)->bgl_bank_amount) + "``]|left|\nadd_smalltext|`o- mgl Bank: `5[`2" + Set_Count(pInfo(cp_)->mgl_bank_amount) + "``]|left|\nadd_smalltext|`o- mgl Bank: `5[`2" + Set_Count(pInfo(cp_)->mgl_bank_amount) + "``]|left|\nadd_spacer|small|\nadd_textbox|`5- Title info -|left|\nadd_smalltext|`o- 'Of Legend': `5[" + (pInfo(cp_)->Title.OfLegend ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Dr.': `5[" + (pInfo(cp_)->Title.Doctor ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Grow4Good Title': `5[" + (pInfo(cp_)->Title.Grow4Good ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Bluename': `5[" + (pInfo(cp_)->level >= 125 ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Blackname': `5[" + (pInfo(cp_)->level >= 250 ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Mentor Title': `5[" + (pInfo(cp_)->Title.Mentor ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Tiktok Creator Badge': `5[" + (pInfo(cp_)->Title.TiktokBadge ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Content Creator Badge': `5[" + (pInfo(cp_)->Title.ContentCBadge ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Party Animal': `5[" + (pInfo(cp_)->Title.PartyAnimal ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'ThanksGiving': `5[" + (pInfo(cp_)->Title.ThanksGiving ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Old Timer': `5[" + (pInfo(cp_)->Title.OldTimer ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Santa Claus': `5[" + (pInfo(cp_)->Title.WinterSanta ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'GrowPass Gold': `5[" + (pInfo(cp_)->Title.GrowPass_Gold ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'GrowPass Silver': `5[" + (pInfo(cp_)->Title.GrowPass_Silver ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'GrowPass Bronze': `5[" + (pInfo(cp_)->Title.GrowPass_Bronze ? "`2YES" : "`4NO") + "``]|left|\nadd_smalltext|`o- 'Award Winning': `5[" + (pInfo(cp_)->Title.Award_Winning ? "`2YES" : "`4NO") + "``]|left|\nadd_spacer|small|\nadd_textbox|`5- Roles Title Info -|left|\nadd_smalltext|'" + (pInfo(cp_)->RolesTitle.Farmer ? "`2FARMER ?``/" : "`4FARMER ?``/") + (pInfo(cp_)->RolesTitle.Builder ? "`2BUILDER ?``/" : "`4BUILDER ?``/") + (pInfo(cp_)->RolesTitle.Surgeon ? "`2SURGEON ?``/" : "`4SURGEON ?``/") + (pInfo(cp_)->RolesTitle.Fisher ? "`2FISHER ?``/" : "`4FISHER ?``/") + (pInfo(cp_)->RolesTitle.Chef ? "`2CHEF ?``/" : "`4CHEF ?``/") + (pInfo(cp_)->RolesTitle.Startopian ? "`2STARTOPIAN ?``/" : "`4STARTOPIAN ?``/") + "'|left|"
					"\nadd_spacer|small|\nadd_textbox|`5Give Assets:|left|\nadd_spacer|small|\ntext_scaling_string|DEFIBRILLATOR|\nadd_checkicon|gems|`oGEMS||14590||0||\nadd_checkicon|level|`oLEVEL||1488||0||\nadd_checkicon|coin|`oCOIN||244||0||\nadd_checkicon|opc|`oOPC||10668||0||\nadd_checkicon|token|`oGUILD TOKEN||" + to_string(guild_event_id) + "||0||\nadd_checkicon|seals|`oSEALS||9186||0||\nadd_checkicon|growpass|`oGROWPASS||11304||" + to_string(pInfo(cp_)->gp) + "||\nadd_checkicon|roadtoglory|`oROAD TO GLORY||9436||" + to_string(pInfo(cp_)->glo) + "||\nadd_checkicon|piggybank|`oPIGGY BANK||0||" + to_string(pInfo(cp_)->pg_bank) + "||" + (pInfo(cp_)->supp == 0 ? "\nadd_checkicon|supp_1|`oSUPPORTER||10860||0||" : pInfo(cp_)->supp == 1 ? "\nadd_checkicon|supp_2|`oSUPER-SUPP||10862||0||" : "\nadd_checkicon|supp_2|`oSUPER-SUPP||10862||1||") + "\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nadd_textbox|`5Give Title:|left|\nadd_spacer|small|\nadd_checkicon|oflegend|`oOf Legend||1794||" + to_string(pInfo(cp_)->Title.OfLegend) + "||\nadd_checkicon|doctor|`oDr.Name||7068||" + to_string(pInfo(cp_)->Title.Doctor) + "||\nadd_checkicon|grow4good|`oGrow4Good||11816||" + to_string(pInfo(cp_)->Title.Grow4Good) + "||\nadd_checkicon|mentor|`oMentor||9472||" + to_string(pInfo(cp_)->Title.Mentor) + "||\nadd_checkicon|tiktok|`oTiktok Badge||9530||" + to_string(pInfo(cp_)->Title.TiktokBadge) + "||\nadd_checkicon|content|`oContent Badge||10866||" + to_string(pInfo(cp_)->Title.ContentCBadge) + "||\nadd_checkicon|partyanimal|`oParty Animal||14186||" + to_string(pInfo(cp_)->Title.PartyAnimal) + "||\nadd_checkicon|tgt|`oThanksGiving||10320||" + to_string(pInfo(cp_)->Title.ThanksGiving) + "||\nadd_checkicon|oldtimer|`oOld Timer||4370||" + to_string(pInfo(cp_)->Title.OldTimer) + "||\nadd_checkicon|santa|`oSanta Claus||1360||" + to_string(pInfo(cp_)->Title.WinterSanta) + "||\nadd_checkicon|gp_bronze|`oGPass Bronze||14412||" + to_string(pInfo(cp_)->Title.GrowPass_Bronze) + "||\nadd_checkicon|gp_silver|`oGPass Silver||14410||" + to_string(pInfo(cp_)->Title.GrowPass_Silver) + "||\nadd_checkicon|gp_gold|`oGPass Golden||14408||" + to_string(pInfo(cp_)->Title.GrowPass_Gold) + "||\nadd_checkicon|aw_win|`oAward Winning||15114||" + to_string(pInfo(cp_)->Title.Award_Winning) + "||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nadd_textbox|`5Give Roles Title:|left|\nadd_spacer|small|\nadd_checkicon|roles_farmer|`oFarmer||7064||" + to_string(pInfo(cp_)->RolesTitle.Farmer) + "||\nadd_checkicon|roles_builder|`oBuilder||7070||" + to_string(pInfo(cp_)->RolesTitle.Builder) + "||\nadd_checkicon|roles_surgeon|`oSurgeon||7068||" + to_string(pInfo(cp_)->RolesTitle.Surgeon) + "||\nadd_checkicon|roles_fisher|`oFisher||7072||" + to_string(pInfo(cp_)->RolesTitle.Fisher) + "||\nadd_checkicon|roles_chef|`oChef||7076||" + to_string(pInfo(cp_)->RolesTitle.Chef) + "||\nadd_checkicon|roles_startopian|`oStartopian||7074||" + to_string(pInfo(cp_)->RolesTitle.Startopian) + "||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nadd_smalltext|`oNOTE: The column below is only useful for (Gems, Level, Coin, Opc, Guild Token, and Seals).|left|\nadd_text_input|amount|`oAmount:||15|\nadd_spacer|small|\nadd_custom_button|Nevermind|textLabel:`wNevermind;middle_colour:2415764;border_colour:2415764;|\nadd_custom_button|Apply Changes|textLabel:`wApply Changes;anchor:_button_Nevermind;left:1;margin:60,0;middle_colour:434431310;border_colour:434438350;|\nend_dialog|give_assets|||\nadd_quick_exit|");
				break;
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void GWRENCH(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /gwrench <player name>");
			return;
		}
		bool found = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				found = true;
				pInfo(peer)->last_wrenched = pInfo(cp_)->tankIDName;
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wGive / Remove - Custom Wrench (" + pInfo(cp_)->tankIDName + ")|left|32|\nadd_spacer|small|\ntext_scaling_string|DEFIBRILLATOR|\nadd_textbox|`$Wrench Style:|left|\nadd_checkicon|prismatic_style|`oPrismatic Style||14360||" + to_string(Has_Claimed::W_Style(peer, 14360)) + "||\nadd_checkicon|shiny_style|`oShiny Style||14492||" + to_string(Has_Claimed::W_Style(peer, 14492)) + "||\nadd_checkicon|wrecked_style|`oWrecked Style||14496||" + to_string(Has_Claimed::W_Style(peer, 14496)) + "||\nadd_checkicon|fresh_style|`oFresh Style||14500||" + to_string(Has_Claimed::W_Style(peer, 14500)) + "||\nadd_checkicon|beautiful_style|`oBeautiful Style||14504||" + to_string(Has_Claimed::W_Style(peer, 14504)) + "||\nadd_checkicon|shocking_style|`oShocking Style||14824||" + to_string(Has_Claimed::W_Style(peer, 14824)) + "||\nadd_checkicon|musical_style|`oMusical Style||14560||" + to_string(Has_Claimed::W_Style(peer, 14560)) + "||\nadd_checkicon|runic_style|`oRunic Style||14714||" + to_string(Has_Claimed::W_Style(peer, 14714)) + "||\nadd_checkicon|mechanical_style|`oMechanical Style||14726||" + to_string(Has_Claimed::W_Style(peer, 14726)) + "||\nadd_checkicon|icy_style|`oIcy Style||15014||" + to_string(Has_Claimed::W_Style(peer, 15014)) + "||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nadd_textbox|`$Wrench Decoration:|left|\nadd_checkicon|prismatic_deco|`oPrismatic Deco||14358||" + to_string(Has_Claimed::W_Deco(peer, 14358)) + "||\nadd_checkicon|shiny_deco|`oShiny Deco||14490||" + to_string(Has_Claimed::W_Deco(peer, 14490)) + "||\nadd_checkicon|wrecked_deco|`oWrecked Deco||14494||" + to_string(Has_Claimed::W_Deco(peer, 14494)) + "||\nadd_checkicon|fresh_deco|`oFresh Deco||14498||" + to_string(Has_Claimed::W_Deco(peer, 14498)) + "||\nadd_checkicon|beautiful_deco|`oBeautiful Deco||14502||" + to_string(Has_Claimed::W_Deco(peer, 14502)) + "||\nadd_checkicon|shocking_deco|`oShocking Deco||14822||" + to_string(Has_Claimed::W_Deco(peer, 14822)) + "||\nadd_checkicon|musical_deco|`oMusical Deco||14558||" + to_string(Has_Claimed::W_Deco(peer, 14558)) + "||\nadd_checkicon|runic_deco|`oRunic Deco||14712||" + to_string(Has_Claimed::W_Deco(peer, 14712)) + "||\nadd_checkicon|mechanical_deco|`oMechanical Deco||14724||" + to_string(Has_Claimed::W_Deco(peer, 14724)) + "||\nadd_checkicon|icy_deco|`oIcy Deco||15012||" + to_string(Has_Claimed::W_Deco(peer, 15012)) + "||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nadd_custom_button|Nevermind|textLabel:`wNevermind;middle_colour:2415764;border_colour:2415764;|\nadd_custom_button|Apply Changes|textLabel:`wApply Changes;anchor:_button_Nevermind;left:1;margin:60,0;middle_colour:434431310;border_colour:434438350;|\nend_dialog|give_cwrench|||\nadd_quick_exit|");
				break;
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void SETWORLDMENU(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /setworldmenu <name world>");
			return;
		}
		std::string name = cmd;
		transform(name.begin(), name.end(), name.begin(), ::toupper);
		if (find(Environment()->World_Menu.begin(), Environment()->World_Menu.end(), name) != Environment()->World_Menu.end()) {
			Environment()->World_Menu.erase(remove(Environment()->World_Menu.begin(), Environment()->World_Menu.end(), name), Environment()->World_Menu.end());
			VarList::OnConsoleMessage(peer, "`o>> Successfully removed " + name + " from world menu list.");
			EventPool::Save::All();
		}
		else {
			Environment()->World_Menu.push_back(name);
			VarList::OnConsoleMessage(peer, "`o>> Successfully added " + name + " to world menu list.");
			EventPool::Save::All();
		}
	}
	static void SPAWN_DROP_GROWCH(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
				VarList::OnConsoleMessage(peer, "`oUsage: /spawndropgrowch <number 1-11>");
				return;
			}
			int count = std::atoi(cmd.c_str());
			if (count < 1 or count > 11) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Size must between 1-11!", 0, 1);
				return;
			}
			if (not WinterFest.Active or pInfo(peer)->world != "GROWCH") {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, (not WinterFest.Active ? "Can only be used if the Winterfest event is active!" : "Can only be used in world GROWCH!"), 0, 1);
				return;
			}
			if (count == 1) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(2226, 1318, 1538), list.emplace_back(1374, 1256, 1538), list.emplace_back(1380, 1190, 1538), list.emplace_back(7444, 1253, 1474), list.emplace_back(11518, 1193, 1474);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 2) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(1358, 1128, 1442), list.emplace_back(2222, 1092, 1378), list.emplace_back(3206, 1155, 1378), list.emplace_back(10522, 1155, 1314);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 3) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(1368, 1221, 1282), list.emplace_back(2234, 1221, 1346), list.emplace_back(1370, 1221, 1410), list.emplace_back(7408, 1285, 1378), list.emplace_back(7410, 1285, 1314);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 4) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(1392, 1702, 1506), list.emplace_back(1372, 1767, 1506), list.emplace_back(7430, 1828, 1506), list.emplace_back(9198, 1892, 1538);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 5) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(1388, 2019, 1506), list.emplace_back(2240, 1960, 1506), list.emplace_back(1362, 1990, 1442), list.emplace_back(7464, 1959, 1378), list.emplace_back(7466, 1892, 1474);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 6) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(1382, 1444, 1154), list.emplace_back(1386, 1444, 1218), list.emplace_back(2230, 1444, 1282), list.emplace_back(9200, 1477, 1442);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 7) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(1384, 1892, 1346), list.emplace_back(2232, 1892, 1410), list.emplace_back(3256, 1827, 1378), list.emplace_back(7450, 1827, 1442), list.emplace_back(11512, 1766, 1442);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 8) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(2236, 1635, 1122), list.emplace_back(5442, 1705, 1154), list.emplace_back(1378, 1705, 1218), list.emplace_back(5444, 1705, 1282);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 9) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(1376, 1510, 1122), list.emplace_back(2224, 1510, 1185), list.emplace_back(3250, 1510, 1249), list.emplace_back(9174, 1510, 1313), list.emplace_back(11488, 1510, 1377);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 10) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(2218, 1638, 1185), list.emplace_back(5414, 1638, 1249), list.emplace_back(10438, 1638, 1313), list.emplace_back(1390, 1638, 1377);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
			if (count == 11) {
				std::vector<std::tuple<int, int, int>> list;
				list.emplace_back(7416, 1574, 1089), list.emplace_back(7432, 1574, 1153), list.emplace_back(9176, 1574, 1217), list.emplace_back(11466, 1574, 1281), list.emplace_back(12892, 1574, 1345), list.emplace_back(14092, 1574, 1409);
				for (const auto& pos : list) {
					WorldDrop drop_block_{};
					drop_block_.id = std::get<0>(pos), drop_block_.count = 1, drop_block_.x = std::get<1>(pos), drop_block_.y = std::get<2>(pos);
					VisualHandle::Drop(world_, drop_block_);
				}
			}
		}
	}
	static void TRANSFER_BANK(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /tfbank <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd; bool found = false;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				pInfo(peer)->last_wrenched = pInfo(cp_)->tankIDName;
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wTransfer Bank - (" + pInfo(cp_)->tankIDName + ")|left|13808|\nadd_spacer|small|\nadd_textbox|`2Balance User:|left|\nadd_custom_textbox|`o" + Set_Count(pInfo(cp_)->wl_bank_amount) + " World Locks.|size:small;icon:242;|\nadd_custom_textbox|`o" + Set_Count(pInfo(cp_)->dl_bank_amount) + " Diamond Locks.|size:small;icon:1796;|\nadd_custom_textbox|`o" + Set_Count(pInfo(cp_)->bgl_bank_amount) + " Silver Gem Locks.|size:small;icon:7188;|\nadd_custom_textbox|`o" + Set_Count(pInfo(cp_)->mgl_bank_amount) + " " + "Magical Gem Lock" + "s.|size:small;icon:8470;|\nadd_custom_textbox|`o" + Set_Count(pInfo(cp_)->mgl_bank_amount) + " " + items[20298].ori_name + "s.|size:small;icon:20298;|\nadd_spacer|small|\ntext_scaling_string|DEFIBRILLATOR|\nadd_checkicon|tf_wl|`oWL||242||0||\nadd_checkicon|tf_dl|`oDL||1796||0||\nadd_checkicon|tf_bgl|`oBGL||7188||0||\nadd_checkicon|tf_mgl|`oMagical Gem Lock||8470||0||\nadd_checkicon|tf_mgl|`oMGL||20298||0||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_text_input|amount|Amount:||7|\nadd_spacer|small|\nadd_custom_button|Nevermind|textLabel:`wClose;middle_colour:252135623;border_colour:252135623;|\nadd_custom_button|Confirm_Transfer|textLabel:`wTransfer;anchor:_button_Nevermind;left:1;margin:60,0;middle_colour:-15132296;border_colour:-15132296;|\nend_dialog|creator_bank_option|||");
				found = true;
				break;
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void REMOVE_BANK(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "oUsage: /removebank <player name>");
			return;
		}

		pInfo(peer)->last_wrenched = cmd;
		bool found = false;

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == nullptr) continue;

			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				pInfo(peer)->last_wrenched = pInfo(cp_)->tankIDName;

				auto safe_count = [](int64_t value) -> std::string {
					try {
						return Set_Count(value);
					}
					catch (...) {
						return "0";
					}
					};

				std::string dialog = SetColor(peer) +
					"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"

					"add_label_with_icon|big|`wRemove Balance Bank - (" + pInfo(cp_)->tankIDName + ")|left|13810|\n"
					"add_spacer|small|\n"
					"add_textbox|`2Balance User:|left|\n"
					"add_custom_textbox|`o" + safe_count(pInfo(cp_)->wl_bank_amount) + " World Locks.|size:small;icon:242;|\n"
					"add_custom_textbox|`o" + safe_count(pInfo(cp_)->dl_bank_amount) + " Diamond Locks.|size:small;icon:1796;|\n"
					"add_custom_textbox|`o" + safe_count(pInfo(cp_)->bgl_bank_amount) + " Silver Gem Locks.|size:small;icon:7188;|\n"
				"add_custom_textbox|`o" + safe_count(pInfo(cp_)->mgl_bank_amount) + " " + "Magical Gem Lock" + "s.|size:small;icon:8470;|\n"
				"add_custom_textbox|`o" + safe_count(pInfo(cp_)->mgl_bank_amount) + " " + items[20298].ori_name + "s.|size:small;icon:20298;|\n"
					"add_spacer|small|\n"
					"text_scaling_string|DEFIBRILLATOR|\n"
					"add_checkicon|tf_wl|`oWL||242||0||\n"
					"add_checkicon|tf_dl|`oDL17960||\n"
					"add_checkicon|tf_bgl|`oBGL||7188||0||\n"
					"add_checkicon|tf_mgl|`oGGL84700||\n"
					"add_checkicon|tf_mgl|`oMGL||20298||0||\n"
					"add_button_with_icon||END_LIST|noflags|0||\n"
					"add_spacer|small|\n"
					"add_custom_button|Nevermind|textLabel:`wClose;middle_colour:252135623;border_colour:252135623;|\n"
					"add_custom_button|Confirm_Remove|textLabel:`wRemove;anchor:_button_Nevermind;left:1;margin:60,0;middle_colour:-15132296;border_colour:-15132296;|\n"
					"end_dialog|creator_bank_option|||";

				VarList::OnDialogRequest(peer, dialog);
				found = true;
				break;
			}
		}

		if (!found)
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void ADD_PRICE_BUY(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /addpbuy <itemid> <price>");
			return;
		}
		std::istringstream ss(cmd);
		int item_id, harga;
		try {
			if (ss >> item_id >> harga) {
				if (item_id < 1 || item_id > items.size()) {
					VarList::OnConsoleMessage(peer, ">> ItemID is not valid!");
					return;
				}
				if (Gtps_Shop::Get_Price(item_id) == 0) Environment()->buy_item_list.push_back(std::make_pair(item_id, item_id));
				Environment()->shop_data[item_id] = harga;
				VarList::OnConsoleMessage(peer, ">> Succes modified `2" + items[item_id].name + " `w(" + to_string(item_id) + ") `oprice to `4" + Set_Count(harga) + " `ogems (gems)");
				Gtps_Shop::Save();
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /addpbuy <itemid> <price>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /addpbuy <itemid> <price>");
		}
	}
	static void GET_ITEMSS(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /getitem <item name>");
			return;
		}
		// FIX: mulai pencarian baru = reset checkbox yang tersimpan dari pencarian sebelumnya
		pInfo(peer)->find_selected_items.clear();
		DialogHandle::FINDITEMSCLIST(peer, cmd, 1);
	}
	static void EDIT_ITEM(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /edititem <item id>");
			return;
		}
		int id = std::atoi(cmd.c_str());
		if (id == 0 or id < 2 or id > items.size() or items[id].blockType == SEED || items[id].name.find("Blank") != string::npos || items[id].name.find("Phoenix") != string::npos || items[id].name.find("Wrench") != string::npos or items[id].name.find("Data Bedrock") != string::npos || items[id].name.find("null_item") != string::npos || items[id].name.find("null") != string::npos || items[id].name.find("Guild Entrance") != string::npos || items[id].name.find("Guild Banner") != string::npos || items[id].name.find("Guild Flag") != string::npos || items[id].name.find("Guild Key") != string::npos || items[id].name.find("World Key") != string::npos || id == 5640 || id == 9158 || id == 5814 || id == 5816 || items[id].actionType == 127 || items[id].actionType == 126 || items[id].actionType == 118 || items[id].actionType == 117 || items[id].actionType == 116 || items[id].actionType == 115 || items[id].actionType == 113 || items[id].actionType == 109 || items[id].actionType == 106 || items[id].actionType == 105 || items[id].actionType == 104 || items[id].actionType == 102 || items[id].actionType == 99 || items[id].actionType == 96 || items[id].actionType == 86 || items[id].actionType == 79 || items[id].actionType == 75 || items[id].actionType == 72 || items[id].actionType == 71 || items[id].actionType == 68 || items[id].actionType == 66 || items[id].actionType == 65 || items[id].actionType == 53 || items[id].actionType == 52 || items[id].actionType == 50 || items[id].actionType == 43 || items[id].actionType == 91 || items[id].id == 5818 || items[id].id == 5820) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "These items cannot be edited!");
			return;
		}
		DialogHandle::EditItemPro(peer, id);
	}
	static void ADD_ITEM_TO_NPC(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /additemtonpc <item id>");
			return;
		}
		int id = std::atoi(cmd.c_str());
		if (id == 0 or id < 2 or id > items.size() or items[id].blockType == BlockTypes::LOCK || items[id].name.find("Blank") != string::npos || items[id].name.find("Wrench") != string::npos or items[id].name.find("Data Bedrock") != string::npos || items[id].name.find("null_item") != string::npos || items[id].name.find("null") != string::npos || items[id].name.find("Guild Entrance") != string::npos || items[id].name.find("Guild Banner") != string::npos || items[id].name.find("Guild Flag") != string::npos || items[id].name.find("Guild Key") != string::npos || items[id].name.find("World Key") != string::npos || id == 5640 || id == 9158 || id == 5814 || id == 5816 || items[id].actionType == 127 || items[id].actionType == 126 || items[id].actionType == 118 || items[id].actionType == 117 || items[id].actionType == 116 || items[id].actionType == 115 || items[id].actionType == 113 || items[id].actionType == 109 || items[id].actionType == 106 || items[id].actionType == 105 || items[id].actionType == 104 || items[id].actionType == 102 || items[id].actionType == 99 || items[id].actionType == 96 || items[id].actionType == 86 || items[id].actionType == 79 || items[id].actionType == 75 || items[id].actionType == 72 || items[id].actionType == 71 || items[id].actionType == 68 || items[id].actionType == 66 || items[id].actionType == 65 || items[id].actionType == 53 || items[id].actionType == 52 || items[id].actionType == 50 || items[id].actionType == 43 || items[id].actionType == 91 || items[id].id == 5818 || items[id].id == 5820) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "These items cannot be added!");
			return;
		}
		bool has_ = false;
		if (Npc_MarketPlace::Has(id)) {
			for (const auto& item : Environment()->Npc_MarketPlace) {
				if (std::get<0>(item) == id) {
					if (Npc_MarketPlace::Remove(id)) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Items has been Removed from Npc!");
					has_ = true;
					break;
				}
			}
		}
		if (not has_) VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wAdd `2" + items[id].name + " ``to [NPC] MarketPlace|left|" + to_string(id) + "|\nembed_data|id|" + to_string(id) + "\nadd_spacer|small|\nmax_checks|1|\ntext_scaling_string|DEFIBRILLATOR|\nadd_smalltext|`oSelect type Price:|left|\nadd_checkicon|wl|WL|noflags|242||0|\nadd_checkicon|dl|DL|noflags|1796||0|\nadd_checkicon|bgl|bgl|noflags|7188||0|\nadd_checkicon|mgl|Magical Gem Lock|noflags|8470||0|0|\nadd_checkicon|gems|GEMS|noflags|14590||0|\nadd_button_with_icon||END_ROW|noflags|0||\nadd_spacer|small|\nadd_smalltext|`oPrice:|left|\nadd_text_input|price|||13|\nadd_spacer|small|\nend_dialog|MarketPlace_Npc|Cancel|Update|\nadd_quick_exit|");
	}
	static void TAKEWORLD(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (world_->owner_name.empty()) return;
			if (world_->guild_id != 0) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`wYou can't use that commands here.");
				return;
			}
			if (world_->owner_name == pInfo(peer)->tankIDName) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`wYou already own of this World.");
				return;
			}
			int lock_id = 0, lock_x = -1, lock_y = -1;
			int ySize = (int)world_->blocks.size() / 100, xSize = (int)world_->blocks.size() / ySize;
			for (int i_ = 0; i_ < world_->blocks.size(); i_++) {
				if (items[world_->blocks[i_].fg].blockType == BlockTypes::LOCK) {
					if (world_->blocks[i_].fg == 202 || world_->blocks[i_].fg == 204 || world_->blocks[i_].fg == 206 || world_->blocks[i_].fg == 4994) {
						world_->blocks[i_].owner_name = pInfo(peer)->tankIDName;
					}
				}
			}
			{
				std::vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [&](const WorldBlock& a) { return items[a.fg].blockType == BlockTypes::LOCK && a.fg != 202 && a.fg != 204 && a.fg != 206 && a.fg != 4994 && a.fg != 10000; });
				if (p != world_->blocks.end()) {
					int i_ = p - world_->blocks.begin();
					WorldBlock* block__ = &world_->blocks[p - world_->blocks.begin()];
					std::uint32_t id_ = block__->fg;
					lock_id = id_, lock_x = i_ % xSize, lock_y = i_ / xSize;
				}
			}
			if (lock_id == 0) return;
			std::string before_name = world_->owner_name;
			world_->owner_name = pInfo(peer)->tankIDName;
			if (pInfo(peer)->name_color != "`0" or Role::Moderator(peer) || Role::Administrator(peer) && pInfo(peer)->name_color != "`2")  world_->owner_named = pInfo(peer)->name_color + pInfo(peer)->tankIDName;
			else world_->owner_named = pInfo(peer)->tankIDName;
			pInfo(peer)->worlds_owned.push_back(world_->name);
			PlayerMoving data_{};
			data_.packetType = 15, data_.punchX = lock_x, data_.punchY = lock_y, data_.characterState = 0x8, data_.netID = name_to_number(to_lower(pInfo(peer)->tankIDName)), data_.plantingTree = lock_id;
			BYTE* raw = packPlayerMoving(&data_, 56);
			VisualHandle::Nick(peer, NULL);
			pInfo(peer)->name_color = Role::Prefix2(peer, *world_);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != world_->name) continue;
				pInfo(cp_)->world_owner = (world_->owner_named.empty() ? "`#" + world_->owner_name : world_->owner_named);
				VarList::OnPlaySound(cp_, "audio/use_lock.wav");
				send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
				VarList::OnConsoleMessage(cp_, "`5[```w" + world_->name + "`` has been `$World Locked`` by " + pInfo(peer)->name_color + world_->owner_name + "```5]``");
				VarList::OnTalkBubble(cp_, pInfo(peer)->netID, "`5[```w" + world_->name + "`` has been `$World Locked`` by " + pInfo(peer)->name_color + world_->owner_name + "```5]``", 0, 0);
			}
			delete[] raw;
			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " take world : " + world_->name + ".", "Take Worlds");
		}
	}
	static void CLEAR_WORLD_DIALOG(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (world_->worldLocked && !Role::Owner(peer)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4This world is locked by the server.", 0, 1);
				return;
			}
			if (not world_->owner_name.empty() and (pInfo(peer)->world == "REC" or pInfo(peer)->world == "PROMOTE" or pInfo(peer)->world == "START" or pInfo(peer)->world == "TRADE" or pInfo(peer)->world == "BFG" or pInfo(peer)->world == "TIPS")) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You can't do it here!");
				return;
			}
			if (pInfo(peer)->Role.Role_Level >= 5) {
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`4BEWARE``|left|112|\nadd_spacer|small|\nadd_textbox|`4WARNING: You are about to CLEAR the world `w" + pInfo(peer)->world + "``!|left|\nadd_spacer|small|\nadd_textbox|`8All blocks and settings in this world will be permanently reset!|left|\nadd_textbox|`4This action CANNOT be undone!|left|\nadd_spacer|small|\nadd_textbox|`oAre you absolutely sure you want to clear this world?|left|\nadd_spacer|small|\nend_dialog|clearworld_confirm|Cancel|Yes, CLEAR IT!|\nadd_quick_exit|");
			}
		}
	}
	static void CLEAR_WORLD(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (not world_->owner_name.empty() and (pInfo(peer)->world == "REC" or pInfo(peer)->world == "PROMOTE" or pInfo(peer)->world == "START" or pInfo(peer)->world == "TRADE" or pInfo(peer)->world == "BFG" or pInfo(peer)->world == "TIPS")) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You can't do it here!");
				return;
			}
			else {
				if (pInfo(peer)->Role.Role_Level >= 5) {
					if (not world_->owner_name.empty() and (pInfo(peer)->world == "REC" or pInfo(peer)->world == "PROMOTE" or pInfo(peer)->world == "START" or pInfo(peer)->world == "TRADE" or pInfo(peer)->world == "BFG" or pInfo(peer)->world == "TIPS")) {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You can't do it here!");
						return;
					}
					ServerPool::Logs::Add(pInfo(peer)->tankIDName + " clear world [" + pInfo(peer)->world + "]", "Clear World");
					VarList::OnTextOverlay(peer, "Cleared the world");
					world_->fresh_world = true;
					world_->world_settings = 0;
					world_->nuked_by = "";
					world_->admins.clear();
					world_->drop_new.clear();
					world_->sbox1.clear();
					world_->bulletin.clear();
					world_->weather = 0;
					world_->bannedPlayers.clear();
					world_->music_bpm = 100;
					world_->entry_level = 1;
					world_->active_jammers.clear();
					world_->machines.clear();
					for (int i_ = 0; i_ < world_->blocks.size(); i_++) {
						world_->blocks[i_].flags = 0;
						world_->blocks[i_].id = 0;
						if (i_ <= 5400) {
							if (world_->blocks[i_].fg != 0 and world_->blocks[i_].fg != 6 and world_->blocks[i_].fg != 8 and items[world_->blocks[i_].fg].blockType != BlockTypes::LOCK) world_->blocks[i_].fg = 0, world_->blocks[i_].locked = false;
							if (world_->blocks[i_].bg != 0) world_->blocks[i_].bg = 0;
						}
					}
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						if (pInfo(cp_)->world == pInfo(peer)->world) {
							Exit_World(cp_);
						}
					}
				}
			}
		}
	}
	static void SPAWN_EVENT(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (world_->special_event == false) {
				int event_item = 0;
				vector<int> list = { 263,942,942,1396,4774,4522, 4774, 5002, 121, 1636, 2798, 2704, 3786, 4354 , 4354, 2992, 2992, 10016, 10016, 1396 };
				world_->special_event_item = (WinterFest.Active ? 9186 : list[rand() % list.size()]);
				event_item = world_->special_event_item;
				world_->special_event = true;
				for (ENetPeer* cp__event = server->peers; cp__event < &server->peers[server->peerCount]; ++cp__event) {
					if (cp__event->state != ENET_PEER_STATE_CONNECTED or cp__event->data == NULL or pInfo(cp__event)->world != pInfo(peer)->world) continue;
					VarList::OnConsoleMessage(cp__event, "`2" + items[world_->special_event_item].event_name + ":`` " + (items[world_->special_event_item].event_total == 1 ? "`oYou have`` `030`` `oseconds to find and grab the`` `#" + items[world_->special_event_item].name + "```o.``" : "`#" + to_string(items[world_->special_event_item].event_total) + " " + items[world_->special_event_item].name + "`` `ospawn in your world, you have`` `030`` `oseconds to collect them.``") + "");
					VarList::OnAddNotification(cp__event, "`2" + items[world_->special_event_item].event_name + ":`` " + (items[world_->special_event_item].event_total == 1 ? "`oYou have`` `030`` `oseconds to find and grab the`` `#" + items[world_->special_event_item].name + "```o.``" : "`#" + to_string(items[world_->special_event_item].event_total) + " " + items[world_->special_event_item].name + "`` `ospawn in your world, you have`` `030`` `oseconds to collect them.``") + "", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
				}
				world_->last_special_event = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
				if (find(Environment()->Another_Worlds.begin(), Environment()->Another_Worlds.end(), world_->name) == Environment()->Another_Worlds.end()) Environment()->Another_Worlds.push_back(world_->name);
				server_event_spawn = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
				WorldDrop drop_block_{};
				drop_block_.count = 1;
				for (int i_ = 0; i_ < items[event_item].event_total; i_++) {
					drop_block_.id = event_item == 2034 ? (rand() % 100 < 25 ? 2036 : 2034) : world_->special_event_item, drop_block_.x = rand() % 99 * 32, drop_block_.y = rand() % 54 * 32;
					world_->world_event_items.push_back(drop_block_.id);
					VisualHandle::Drop(world_, drop_block_, true);
				}
			}
		}
	}
	static void VERIFY_CSN(ENetPeer* peer) {
		if (find(Environment()->Verifed_Csn_World.begin(), Environment()->Verifed_Csn_World.end(), pInfo(peer)->world) != Environment()->Verifed_Csn_World.end()) {
			Environment()->Verifed_Csn_World.erase(remove(Environment()->Verifed_Csn_World.begin(), Environment()->Verifed_Csn_World.end(), pInfo(peer)->world), Environment()->Verifed_Csn_World.end());
			EventPool::Save::All();
			VarList::OnConsoleMessage(peer, "`oWorld " + pInfo(peer)->world + " has been removed from verified casino worlds.");
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "World " + pInfo(peer)->world + " has been removed from verified casino worlds.", 0, 1);
		}
		else {
			std::string name_ = pInfo(peer)->world;
			auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (it != worlds.end()) {
				it->fresh_world = true;
			}

			if (find(Environment()->Waiting_Verif_Csn.begin(), Environment()->Waiting_Verif_Csn.end(), pInfo(peer)->world) != Environment()->Waiting_Verif_Csn.end()) {
				Environment()->Waiting_Verif_Csn.erase(remove(Environment()->Waiting_Verif_Csn.begin(), Environment()->Waiting_Verif_Csn.end(), pInfo(peer)->world), Environment()->Waiting_Verif_Csn.end());
			}

			Environment()->Verifed_Csn_World.push_back(pInfo(peer)->world);
			EventPool::Save::All();
			VarList::OnConsoleMessage(peer, "`oWorld " + pInfo(peer)->world + " has been added to verified casino worlds.");
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "World " + pInfo(peer)->world + " has been added to verified casino worlds.", 0, 1);
		}
	}
	static void REMOVE_REQ_VERIFY_CSN(ENetPeer* peer) {
		if (find(Environment()->Waiting_Verif_Csn.begin(), Environment()->Waiting_Verif_Csn.end(), pInfo(peer)->world) != Environment()->Waiting_Verif_Csn.end()) {
			Environment()->Waiting_Verif_Csn.erase(remove(Environment()->Waiting_Verif_Csn.begin(), Environment()->Waiting_Verif_Csn.end(), pInfo(peer)->world), Environment()->Waiting_Verif_Csn.end());
			EventPool::Save::All();
			VarList::OnConsoleMessage(peer, "`oWorld " + pInfo(peer)->world + " has been removed from waiting list request.");
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "World " + pInfo(peer)->world + " has been removed from waiting list request.", 0, 1);
		}
	}
	static void RESET_ROLEQUEST(ENetPeer* peer) {
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->tankIDName.empty()) continue;
			pInfo(cp_)->role_quest = false, pInfo(cp_)->complete_rolequest_day = false;
			pInfo(cp_)->specialization_track = pInfo(cp_)->quest_text;
			pInfo(cp_)->role_quest_type = "", pInfo(cp_)->quest_text = "";
			pInfo(cp_)->total_quest_selected = 0, pInfo(cp_)->quest_required = 0, pInfo(cp_)->need_required = 0, pInfo(cp_)->quest_number = 0, pInfo(cp_)->need_sumbit_req_id = 0, pInfo(cp_)->need_item_req_id = 0;
		}
		VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully Reset all Role Quest!");
	}
	static void DROP_ITEM(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;  // Added missing parenthesis here
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const auto& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "`oUsage: /drop <id> <count>");
				return;
			}
			std::istringstream ss(cmd);
			int itemId, quantity;
			try {
				if (ss >> itemId >> quantity) {
					if (itemId < 1 || itemId > items.size()) {
						VarList::OnConsoleMessage(peer, "`8>> `4Oops: `8Invalid items id, make sure it's was correct!");
						return;
					}
					if (quantity < 1 || quantity > 10000) {
						VarList::OnConsoleMessage(peer, "`8>> `4Oops: `8Minimum is 1 and the maximum is 10,000 items dropped!");
						return;
					}
					if (items[itemId].name.find("null_item") != string::npos || items[itemId].name.find("null") != string::npos || items[itemId].name.find("Guild Entrance") != string::npos || items[itemId].name.find("Guild Key") != string::npos || items[itemId].name.find("World Key") != string::npos || itemId == 5640 || itemId == 9158 || itemId == 5814 || itemId == 5816) {
						VarList::OnConsoleMessage(peer, "`8> `4Oops: `8You can't drop this items, please try with others items!");
						return;
					}
					WorldDrop drop_block_{};
					while (quantity > 0) {
						if (quantity >= 200) {
							drop_block_.x = (pInfo(peer)->state == 16 ? pInfo(peer)->x - ((rand() % 12) + 18) : pInfo(peer)->x + ((rand() % 12) + 22)), drop_block_.y = pInfo(peer)->y + rand() % 16, drop_block_.id = itemId, drop_block_.count = 200;
							VisualHandle::Drop(world_, drop_block_);
							quantity -= 200;
						}
						else {
							drop_block_.x = (pInfo(peer)->state == 16 ? pInfo(peer)->x - ((rand() % 12) + 18) : pInfo(peer)->x + ((rand() % 12) + 22)), drop_block_.y = pInfo(peer)->y + rand() % 16, drop_block_.id = itemId, drop_block_.count = quantity;
							VisualHandle::Drop(world_, drop_block_);
							quantity = 0;
						}
					}
				}
				else {
					VarList::OnConsoleMessage(peer, "`oUsage: /drop <id> <count>");
					return;
				}
			}
			catch (const std::invalid_argument&) {
				VarList::OnConsoleMessage(peer, "`oUsage: /drop <id> <count>");
			}
			catch (const std::out_of_range&) {
				VarList::OnConsoleMessage(peer, "`oUsage: /drop <id> <count>");
			}
		}
	}
	static void DROP5K_ITEM(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const auto& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "`oUsage: /drop5k <item_id>");
				return;
			}

			std::istringstream ss(cmd);
			int itemId;

			try {
				if (ss >> itemId) {
					// Validate item ID range
					if (itemId < 1 || itemId > items.size()) {
						VarList::OnConsoleMessage(peer, "`4>> Error: Invalid item ID!");
						return;
					}

					// Check restricted items
					if (items[itemId].name.find("null_item") != std::string::npos ||
						items[itemId].name.find("null") != std::string::npos ||
						items[itemId].name.find("Guild Entrance") != std::string::npos ||
						items[itemId].name.find("Guild Key") != std::string::npos ||
						items[itemId].name.find("World Key") != std::string::npos ||
						itemId == 5640 || itemId == 9158 || itemId == 5814 || itemId == 5816) {
						VarList::OnConsoleMessage(peer, "`4>> Error: You can't drop this item!");
						return;
					}
					// Drop 5000 items in chunks of 200
					int remaining = 5000;
					WorldDrop drop_block;

					while (remaining > 0) {
						int dropCount = (remaining > 200) ? 200 : remaining;

						drop_block.x = (pInfo(peer)->state == 16 ?
							pInfo(peer)->x - ((rand() % 12) + 18) :
							pInfo(peer)->x + ((rand() % 12) + 22));
						drop_block.y = pInfo(peer)->y + (rand() % 16);
						drop_block.id = itemId;
						drop_block.count = dropCount;

						VisualHandle::Drop(world_, drop_block);
						remaining -= dropCount;
					}

					VarList::OnConsoleMessage(peer, "`2>> Successfully dropped 5000 items!");
				}
				else {
					VarList::OnConsoleMessage(peer, "`oUsage: /drop5k <item_id>");
				}
			}
			catch (const std::exception& e) {
				VarList::OnConsoleMessage(peer, "`4>> Error: Invalid input!");
			}
		}
	}
	// Fast drop lock — drop lock dari inventory ke world dengan jumlah tertentu.
	// /wl <amount>, /dl <amount>, /bgl <amount>, dst.
	static void FAST_DROP_LOCK(ENetPeer* peer, int itemId, const std::string& param) {
		if (!peer || !peer->data) return;
		// Validasi parameter amount.
		if (param.empty() || param.find_first_not_of("0123456789") != std::string::npos) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`oUsage: /" + to_lower(items[itemId].ori_name).substr(0, 3) + " <amount>", 0, 1);
			return;
		}
		int amount = std::atoi(param.c_str());
		if (amount < 1 || amount > 10000) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Amount must be 1-10000!", 0, 1);
			return;
		}
		int have = Inventory::Contains(peer, itemId);
		if (have <= 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You don't have any " + items[itemId].ori_name + " to drop!", 0, 1);
			return;
		}
		if (amount > have) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You only have " + to_string(have) + " " + items[itemId].ori_name + "s!", 0, 1);
			return;
		}
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const auto& a) { return a.name == name_; });
		if (it == worlds.end()) return;
		World* world_ = &(*it);
		int rem = 0;
		Inventory::Modify(peer, itemId, rem -= amount);
		WorldDrop drop_block_{};
		while (amount > 0) {
			int dropCount = (amount >= 200) ? 200 : amount;
			drop_block_.x = (pInfo(peer)->state == 16 ? pInfo(peer)->x - ((rand() % 12) + 18) : pInfo(peer)->x + ((rand() % 12) + 22));
			drop_block_.y = pInfo(peer)->y + (rand() % 16);
			drop_block_.id = itemId;
			drop_block_.count = dropCount;
			VisualHandle::Drop(world_, drop_block_);
			amount -= dropCount;
		}
		VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`2Dropped " + to_string(std::atoi(param.c_str())) + " " + items[itemId].ori_name + "s!", 0, 1);
	}
	static void REMOVEITEMS(ENetPeer* peer, std::string worldName) {
		if (worldName.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /allteleport <worldname>");
			return;
		}

		// Convert to uppercase (GT world name standard)
		std::transform(worldName.begin(), worldName.end(), worldName.begin(), ::toupper);

		// Admin confirmation
		VarList::OnConsoleMessage(peer, "Teleporting all players to `2" + worldName + "``...");

		int teleportedCount = 0;

		// Process all connected players
		for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
			if (p->state != ENET_PEER_STATE_CONNECTED || p->data == NULL) continue;

			// Skip if already in target world
			if (pInfo(p)->world == worldName) continue;

			// Teleport process
			pInfo(p)->update = true;
			Enter_World(p, worldName);

			// Send notification
			VarList::OnAddNotification(
				p,
				"You were moved by the owner",
				"interface/atomic_button.rttex",
				"audio/hub_open.wav"
			);

			// Additional message
			VarList::OnConsoleMessage(
				p,
				"You were teleported to `2" + worldName + "`` by `5" + pInfo(peer)->tankIDName + "``"
			);

			teleportedCount++;
		}

		// Admin summary
		VarList::OnConsoleMessage(
			peer,
			"Successfully teleported `2" + std::to_string(teleportedCount) + "``players to " + worldName
		);
	}
	static void RESET(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /reset <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd; bool found = false;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				if (not Role::Clist(pInfo(peer)->tankIDName) and Role::Clist(pInfo(cp_)->tankIDName)) {
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You can't use this to Creator List!", 0, 0);
					found = true;
					break;
				}
				pInfo(peer)->last_wrenched = pInfo(cp_)->tankIDName;
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wReset Panel - (" + pInfo(cp_)->tankIDName + ")|left|32|\nadd_spacer|small|\nadd_smalltext|`7Fast Button:|left|\ntext_scaling_string|DEFIBRILLATOR|\nadd_checkicon|reset_all|`oALL-IN||5494||0||\nadd_smalltext|Include: [Inventory, Gems, Level, Storage<CR>Coin and Cooldown/Playmods]|left|\nadd_button_with_icon||END_LIST|noflags|0||\nadd_checkicon|reset_lock|`oALL LOCKS||242||0||\nadd_smalltext|Include: [All Locks in Inventory and Bank]|left|\nadd_button_with_icon||END_LIST|noflags|0||\nadd_checkicon|reset_inv|`oINVENTORY||9412||0||\nadd_checkicon|reset_gems|`oGEMS||14590||0||\nadd_checkicon|reset_level|`oLEVEL||1488||0||\nadd_checkicon|reset_strgems|`oSTRG GEMS||14592||0||\nadd_checkicon|reset_coin|`oCOIN||244||0||\nadd_checkicon|reset_cooldown|`oCOOLDOWN||3804||0||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nadd_custom_button|Nevermind|textLabel:`wClose;middle_colour:252135623;border_colour:252135623;|\nadd_custom_button|Confirm_Reset|textLabel:`wReset;anchor:_button_Nevermind;left:1;margin:60,0;middle_colour:-15132296;border_colour:-15132296;|\nend_dialog|reset_option|||");
				found = true;
				break;
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void OWNER_SB(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /osb <text>");
			return;
		}
		if (Playmods::HasById(pInfo(peer), 122)) {
			int time_ = 0;
			for (PlayMods peer_playmod : pInfo(peer)->playmods) {
				if (peer_playmod.id == 122) {
					time_ = peer_playmod.time - time(nullptr);
					break;
				}
			}
			VarList::OnConsoleMessage(peer, ">> (" + Time::Playmod(time_) + " before you can broadcast again)");
		}
		else {
			CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
			Playmods::Add(peer, 122, 90);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[SB]_ `b[OWNER-SB] `5** from (`$" + (get_player_nick(peer)) + "`5) in [`4HIDDEN!`5] ** : `$" + cmd + "``");
			}
		}
	}
	static void UNSAFEEDIT(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "`oUsage: /unsafeedit <action> <radius> <item_id");
				return;
			}
			std::istringstream ss(cmd);
			std::string action;
			int radius, item_id;
			if (ss >> action >> radius >> item_id) {
				if (action == "add" || action == "erase" || action == "addobject" || action == "clearobject");
				else {
					VarList::OnConsoleMessage(peer, "`o<action> (add -> add block), (erase -> remove block), (addobject -> drop), (clearobject -> clear drop).");
					return;
				}
				if (radius < 1 or radius > 20) {
					VarList::OnConsoleMessage(peer, "`4Oops``, the given radius must between 1 - 20.");
					return;
				}
				CL_Vec2i devided_position = CL_Vec2i{ pInfo(peer)->x / 32, pInfo(peer)->y / 32 };
				if (item_id > items.size() || items[item_id].name.find("null_item") != string::npos || items.at(item_id).blockType == BlockTypes::SEED || items.at(item_id).blockType == BlockTypes::CLOTHING || items[item_id].blockType == BlockTypes::LOCK || item_id == 8470 || items[item_id].untradeable == 1 || items[item_id].rarity == 999 || items[item_id].block_possible_put == false || items.at(item_id).blockType == BlockTypes::MAIN_DOOR) {
					VarList::OnConsoleMessage(peer, "`4Oops``, couldn't find item with given keyword '`w" + to_string(item_id) + "``', please make sure this is valid item ID.");
					return;
				}
				VarList::OnConsoleMessage(peer, "Terraforming world...");
				UnsafeEdit(peer, world_, action, devided_position, radius, item_id);
			}
		}
	}
	static void WEATHER_WORLD(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
				VarList::OnConsoleMessage(peer, "`oUsage: /weather <weather id>");
				return;
			}
			int x = std::atoi(cmd.c_str());
			world_->weather = x;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == pInfo(peer)->world) {
					VarList::OnSetCurrentWeather(cp_, x);
				}
			}
		}
	}
	static void GTOKEN(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /gtoken <amount>");
			return;
		}
		int token = std::atoi(cmd.c_str());
		if (guild_event_type == "Spring Clash") pInfo(peer)->spring_token += token;
		if (guild_event_type == "Summer Clash") pInfo(peer)->summer_token += token;
		if (guild_event_type == "Winter Clash") pInfo(peer)->winter_token += token;
		VarList::OnAddNotification(peer, "`wYou received `2" + Set_Count(token) + " `w" + guild_event_type + " Tokens from System!", "interface/token_pve_icon_overlay.rttex", "audio/hub_open.wav");
		ServerPool::SaveDatabase::Players(pInfo(peer), false);
	}
	static void GSEAL(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /gseal <amount>");
			return;
		}
		int seal = std::atoi(cmd.c_str());
		pInfo(peer)->winter_seal = pInfo(peer)->winter_seal + seal;
		VarList::OnAddNotification(peer, "`wYou received `2" + Set_Count(seal) + " `wWinter Seal from System!", "interface/token_pve_icon_overlay.rttex", "audio/hub_open.wav");
		ServerPool::SaveDatabase::Players(pInfo(peer), false);
	}
	static void IP_CHECK(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /ipcheck <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd; bool found = false;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				VarList::OnConsoleMessage(peer, ">> IP CHECK on player " + pInfo(cp_)->name_color + pInfo(cp_)->tankIDName + "`` IP: " + pInfo(cp_)->ip);
				found = true;
				break;
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void GGXP(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
				VarList::OnConsoleMessage(peer, "`oUsage: /ggxp <amount>");
				return;
			}
			int xp = std::atoi(cmd.c_str());
			std::uint32_t guild_id = world_->guild_id;
			std::vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
			if (p != guilds.end()) {
				Guild* guild_information = &guilds[p - guilds.begin()];
				guild_information->guild_xp = xp;
				VarList::OnAddNotification(peer, "`wYou received `2" + std::to_string(xp) + " `wGuild Xp from System!", "interface/guild_button.rttex", "audio/hub_open.wav");
			}
		}
	}
	static void REMOVE_LOGS(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /removelogs <player name>");
			return;
		}
		bool foundacc = true;
		std::string username = cmd;
		pInfo(peer)->last_wrenched = username;
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
			if (to_lower(username) == to_lower(pInfo(currentPeer)->tankIDName)) {
				foundacc = false;
				pInfo(currentPeer)->bans = {}, pInfo(currentPeer)->Account_Notes = {}, pInfo(currentPeer)->Warning_Message = {};
				VarList::OnConsoleMessage(peer, ">> Removed account logs for user " + Role::Prefix(currentPeer) + pInfo(currentPeer)->tankIDName);
			}
		}
		if (foundacc) {
			bool found = false;
			try {
				std::string name = pInfo(peer)->last_wrenched;
				const string filePath = "database/players/" + name + "_.json";
				if (filesystem::exists(filePath)) {
					found = true;
					ifstream inputFile(filePath);
					json jsonData;
					inputFile >> jsonData;
					jsonData["Account_Notes"] = json::array();
					jsonData["7bans"] = json::array();
					jsonData["Warning_Message"] = json::array();
					inputFile.close();
					ofstream outputFile(filePath);
					outputFile << jsonData << endl;
					outputFile.close();
					VarList::OnConsoleMessage(peer, ">> Removed account logs for user " + name);
				}
			}
			catch (exception) {
				return;
			}
			if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
		}
	}
	static void GGEMS(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /ggems <amount>");
			return;
		}
		int gems = std::atoi(cmd.c_str());
		if (pInfo(peer)->gems >= MAX_GEMS) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Warning! Your gems have reached the maximum limit", 0, 0);
			return;
		}
		VarList::OnBuxGems(peer, gems);
		VarList::OnAddNotification(peer, "`wYou received `2" + std::to_string(gems) + " `wGems from System!", "interface/cash_icon_overlay.rttex", "audio/hub_open.wav");
	}
	static void GLEVEL(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /glevel <amount>");
			return;
		}
		int lvl_ = std::atoi(cmd.c_str());
		if (pInfo(peer)->level + lvl_ > 1000) {
			VarList::OnConsoleMessage(peer, "`oCount level exceeds maximum limit");
			return;
		}
		pInfo(peer)->level += lvl_;
		VarList::OnAddNotification(peer, "`wYou received `2" + std::to_string(lvl_) + " `wLevel from System!", "interface/token_pve_icon_overlay.rttex", "audio/hub_open.wav");
		ServerPool::SaveDatabase::Players(pInfo(peer), false);
	}
	static void SET_MAG(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /setmag <amount>");
			return;
		}
		int count = std::atoi(cmd.c_str());
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (int i = 0; i < world_->blocks.size(); i++) {
				int id = world_->blocks[i].fg;
				if (id == 5638 or id == 5930 or id == 10266 or id == 9850 or id == 21220) {
					if (world_->blocks[i].id == 0) continue;
					if (items.at(world_->blocks[i].id).blockType == BlockTypes::SEED) continue;
					if (items.at(world_->blocks[i].id).blockType == BlockTypes::CLOTHING) continue;
					if (items.at(world_->blocks[i].id).blockType == BlockTypes::LOCK) continue;
					world_->blocks[i].pr = count;
				}
			}
			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " restock magplant in [" + pInfo(peer)->world + "] count [" + to_string(count) + "]", "Restock Magplant");
			VarList::OnConsoleMessage(peer, "all number of items in the magplant has been set to (" + to_string(count) + ")");
		}
	}
	static void SET_MAGSEED(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /setmagseed <amount>");
			return;
		}
		int count = std::atoi(cmd.c_str());
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (int i = 0; i < world_->blocks.size(); i++) {
				int id = world_->blocks[i].fg;
				if (id == 5638 or id == 5930 or id == 10266 or id == 9850 or id == 21220) {
					if (world_->blocks[i].id == 0) continue;
					if (items.at(world_->blocks[i].id).blockType == BlockTypes::SEED) {
						world_->blocks[i].pr = count;
					}
				}
			}
			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " restock magplant seed in [" + pInfo(peer)->world + "] count [" + to_string(count) + "]", "Restock Magplant");
			VarList::OnConsoleMessage(peer, "all number of seed in the magplant has been set to (" + to_string(count) + ")");
		}
	}
	static void SET_VEND(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /setvend <amount>");
			return;
		}
		int count = std::atoi(cmd.c_str());
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (int i = 0; i < world_->blocks.size(); i++) {
				int id = world_->blocks[i].fg;
				if (id == 2978 or id == 9268) {
					if (world_->blocks[i].id == 0) continue;
					if (items.at(world_->blocks[i].id).blockType == BlockTypes::SEED) continue;
					if (items.at(world_->blocks[i].id).blockType == BlockTypes::LOCK) continue;
					world_->blocks[i].c_ = count;
				}
			}
			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " restock vending in [" + pInfo(peer)->world + "] count [" + to_string(count) + "]", "Restock Vending");
			VarList::OnConsoleMessage(peer, "all number of items in the vending machine has been set to (" + to_string(count) + ")");
		}
	}
	static void FIND_ITEM(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /find <item name>");
			return;
		}
		// FIX: mulai pencarian baru = reset checkbox yang tersimpan dari pencarian sebelumnya
		pInfo(peer)->find_selected_items.clear();
		DialogHandle::FINDITEMS(peer, cmd, 1);
	}
	static void SET_FIND_BLACKLIST(ENetPeer* peer, std::string cmd) {
		if (!Role::has_config_access(peer)) {
			VarList::OnConsoleMessage(peer, "`4[ERROR]`` `oYou don't have permission to use this command.");
			return;
		}
		if (cmd.empty()) {
			DialogHandle::FINDBLACKLISTLIST(peer, 1);
			return;
		}
		DialogHandle::FINDITEMSBLACKLIST(peer, cmd, 1);
	}
	static void CLEAR_VEND(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (int i = 0; i < world_->blocks.size(); i++) {
				int id = world_->blocks[i].fg;
				if (id == 2978 or id == 9268) {
					if (world_->blocks[i].id == 0) continue;
					world_->blocks[i].c_ = 0;
					world_->blocks[i].id = 0;
				}
			}
			VarList::OnConsoleMessage(peer, "Remove all items from Vending.");
		}
	}
	static void CMD_CV(ENetPeer* peer) {
		if (!peer || !peer->data) return;
		Inventory::AutoConvert(peer);
	}
	static void PLAYSOUND(ENetPeer* peer, std::string param) {
		if (!peer || !peer->data) return;

		DialogHandle::LoadCustomSongs();

		if (DialogHandle::g_custom_songs.empty()) {
			VarList::OnConsoleMessage(peer, "`4[ERROR]`` `oTidak ada lagu di database/spotify_custom.json!");
			return;
		}

		if (param.empty()) {
			std::string msg = "`2[Global Radio - Spotify Music List]``\n";
			for (size_t i = 0; i < DialogHandle::g_custom_songs.size(); ++i) {
				msg += "`w" + std::to_string(i + 1) + ". `o" + DialogHandle::g_custom_songs[i].name + "\n";
			}
			msg += "`7Gunakan: `/playsound <nomor>` (contoh: `/playsound 1`) untuk memutar ke SEMUA player, atau `/playsound 0` untuk stop.";
			VarList::OnConsoleMessage(peer, msg);
			return;
		}

		if (param == "0" || to_lower(param) == "stop" || to_lower(param) == "off") {
			for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
				if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
				pInfo(cp)->spotify = 0;
				CAction::Log(cp, "action|play_music\nfile|\ndelayMS|0");
				VarList::OnConsoleMessage(cp, "`4[Global Radio]`` `oGlobal music stopped by `$" + pInfo(peer)->tankIDName);
			}
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Global music stopped!``", 0, 1);
			return;
		}

		if (param.find_first_not_of("0123456789") != std::string::npos) {
			VarList::OnConsoleMessage(peer, "`4[ERROR]`` `oGunakan nomor lagu (1 - " + std::to_string(DialogHandle::g_custom_songs.size()) + ") atau `/playsound 0` untuk stop.");
			return;
		}

		int num = std::atoi(param.c_str());
		if (num < 1 || num > (int)DialogHandle::g_custom_songs.size()) {
			VarList::OnConsoleMessage(peer, "`4[ERROR]`` `oNomor lagu tidak valid! Pilih nomor antara 1 - " + std::to_string(DialogHandle::g_custom_songs.size()));
			return;
		}

		int idx = num - 1;
		auto& song = DialogHandle::g_custom_songs[idx];

		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
			pInfo(cp)->spotify = 100 + idx;
			CAction::Log(cp, "action|play_music\nfile|" + song.path + "\ndelayMS|0");
			VarList::OnConsoleMessage(cp, "`2[Global Radio]`` `oNow playing `w#" + std::to_string(num) + " - " + song.name + "`` (`7Broadcasted by `$" + pInfo(peer)->tankIDName + "``)");
		}
		VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`2Broadcasting #" + std::to_string(num) + " - " + song.name + " to ALL players!``", 0, 1);
	}
	static void COLLECT_VEND(ENetPeer* peer) {
		if (!peer || !peer->data) return;
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);

			if (to_lower(world_->owner_name) != to_lower(pInfo(peer)->tankIDName)) {
				VarList::OnConsoleMessage(peer, "`4[ERROR]`` `oOnly the world owner can collect locks from vending machines!");
				return;
			}

			long long total_wl = 0;
			int vend_count = 0;

			for (size_t i = 0; i < world_->blocks.size(); i++) {
				WorldBlock* block_ = &world_->blocks[i];
				int id = block_->fg;
				if (items.at(id).blockType == BlockTypes::VENDING || id == 2978 || id == 9268) {
					if (block_->wl > 0) {
						total_wl += block_->wl;
						block_->wl = 0;
						vend_count++;
					}
				}
			}

			if (total_wl <= 0) {
				VarList::OnConsoleMessage(peer, "`4[INFO]`` `oNo World Locks earned in any vending machine to collect.");
				return;
			}

			long long initial_wl = total_wl;
			std::string collected_str = "";

			// 1. Infinity Gem Lock (IGL - ID 20298) -> 1 IGL = 100 MGL = 100,000,000 WL
			if (total_wl >= 100000000) {
				int igl_cnt = (int)(total_wl / 100000000);
				total_wl %= 100000000;
				int c_ = igl_cnt;
				if (Inventory::Modify(peer, 20298, c_) != 0) {
					pInfo(peer)->mgl_bank_amount += c_;
				}
				collected_str += std::to_string(igl_cnt) + " IGL";
			}

			// 2. Magical Gem Lock (MGL - ID 8470) -> 1 MGL = 100 SGL = 1,000,000 WL
			if (total_wl >= 1000000) {
				int mgl_cnt = (int)(total_wl / 1000000);
				total_wl %= 1000000;
				int c_ = mgl_cnt;
				if (Inventory::Modify(peer, 8470, c_) != 0) {
					pInfo(peer)->mgl_bank_amount += c_;
				}
				if (!collected_str.empty()) collected_str += ", ";
				collected_str += std::to_string(mgl_cnt) + " MGL";
			}

			// 3. Silver Gem Lock / Platinum Lock (SGL/BGL - ID 7188) -> 1 SGL = 100 DL = 10,000 WL
			if (total_wl >= 10000) {
				int sgl_cnt = (int)(total_wl / 10000);
				total_wl %= 10000;
				int c_ = sgl_cnt;
				if (Inventory::Modify(peer, 7188, c_) != 0) {
					pInfo(peer)->bgl_bank_amount += c_;
				}
				if (!collected_str.empty()) collected_str += ", ";
				collected_str += std::to_string(sgl_cnt) + " SGL";
			}

			// 4. Diamond Lock (DL - ID 1796) -> 1 DL = 100 WL
			if (total_wl >= 100) {
				int dl_cnt = (int)(total_wl / 100);
				total_wl %= 100;
				int c_ = dl_cnt;
				if (Inventory::Modify(peer, 1796, c_) != 0) {
					pInfo(peer)->dl_bank_amount += c_;
				}
				if (!collected_str.empty()) collected_str += ", ";
				collected_str += std::to_string(dl_cnt) + " DL";
			}

			// 5. World Lock (WL - ID 242)
			if (total_wl > 0) {
				int wl_cnt = (int)total_wl;
				int c_ = wl_cnt;
				if (Inventory::Modify(peer, 242, c_) != 0) {
					pInfo(peer)->wl_bank_amount += c_;
				}
				if (!collected_str.empty()) collected_str += ", ";
				collected_str += std::to_string(wl_cnt) + " WL";
			}

			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " collected " + std::to_string(initial_wl) + " WLs from " + std::to_string(vend_count) + " Vends in [" + world_->name + "]", "Collect Vends");
			VarList::OnConsoleMessage(peer, "`2[SUCCESS]`` `oCollected `w" + std::to_string(initial_wl) + " `oWorld Locks (`2" + collected_str + "``) from `w" + std::to_string(vend_count) + " `oVending Machine(s)!");
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`2Collected " + std::to_string(initial_wl) + " WLs (" + collected_str + ") from " + std::to_string(vend_count) + " Vends!``", 0, 0);
			VarList::OnPlaySound(peer, "audio/gold.wav", 0);
		}
	}
	static void CLEAR_MAG(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (int i = 0; i < world_->blocks.size(); i++) {
				int id = world_->blocks[i].fg;
				if (id == 5638 or id == 5930 or id == 10266 or id == 9850 or id == 21220) {
					if (world_->blocks[i].id == 0) continue;
					world_->blocks[i].pr = 0;
					world_->blocks[i].id = 0;
				}
			}
			VarList::OnConsoleMessage(peer, "Remove all items from Magplant.");
		}
	}
	static void CLEAR_MAGSEED(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (int i = 0; i < world_->blocks.size(); i++) {
				int id = world_->blocks[i].fg;
				if (id == 5638 or id == 9850 or id == 10266 or id == 5930 or id == 21220) {
					if (world_->blocks[i].id == 0) continue;
					if (items.at(world_->blocks[i].id).blockType == BlockTypes::SEED) {
						world_->blocks[i].pr = 0;
						world_->blocks[i].id = 0;
					}
				}
			}
			VarList::OnConsoleMessage(peer, "Remove all Seeds from Magplant.");
		}
	}
	static void SET_GIVEAWAY(ENetPeer* peer, std::string param) {
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "Usage: /giveaway <item_id> <amount> <hour>");
			return;
		}

		int item_id, amount, hour;
		if (sscanf(param.c_str(), "%d %d %d", &item_id, &amount, &hour) != 3 || hour < 0 || hour > 23) {
			VarList::OnConsoleMessage(peer, "`4[ERROR] INVALID PARAM `&Usage: /giveaway <item_id> <amount> <hour>");
			return;
		}
		bool is_lock = items[item_id].blockType == BlockTypes::LOCK;
		if (is_lock) {
			VarList::OnConsoleMessage(peer, "`4Oops: `8You can't giveaway Lock");
			return;
		}

		giveaways = { item_id, amount, hour, true };
		VarList::OnConsoleMessage(peer, "[`6G`7I`8VE`2A`pW`8A`4Y] ``Scheduled! Item ID: " + std::to_string(item_id) + " Amount: " + std::to_string(amount) + " at " + std::to_string(hour) + ":00 WIB.");
	}
	static void ONLINE_CHECK(ENetPeer* peer) {
		std::string list = "";
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->tankIDName.empty()) continue;
			list += "" + Role::Prefix(cp_) + pInfo(cp_)->tankIDName + ", ";
		}
		int add_amount = 0, total_online = 0, total_pc = 0, total_ios = 0, total_android = 0, TotalPlayer = TotalJson("database/players/");
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			add_amount = 1;
			total_online += add_amount;
			if (pInfo(cp_)->player_device == "0,1,1") total_pc += add_amount;
			else if (pInfo(cp_)->player_device == "1") total_ios += add_amount;
			else total_android += add_amount;
		}
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wOnline Players [`2" + Environment()->server_name + "`w]|left|5138|\nadd_textbox|Online: `2" + Set_Count(ServerPool::PlayerCountServer()) + "``/`4" + Set_Count(TotalPlayer) + "``.|left|\nadd_spacer|small|\nadd_label_with_icon|small|Player Device:|left|3802|\nadd_smalltext|PC Users: `2" + to_string(total_pc) + "``.<CR>Android Users: `2" + to_string(total_android) + "``.<CR>IOS Users: `2" + to_string(total_ios) + "``.|left|\nadd_spacer|small|\nadd_smalltext|Uptime: `2" + Time::Playmod(time(nullptr) - Environment()->UpTime) + "|left|\nadd_spacer|small|\nadd_label_with_icon|small|Player Name:|left|1280|\nadd_textbox|" + list + "|left|\nadd_spacer|small|\nend_dialog||Close||\nadd_quick_exit|");
	}
	static void SAVE_WORLD(ENetPeer* peer) {
		VarList::OnConsoleMessage(peer, "saved all worlds: " + to_string(worlds.size()) + "!");
		Environment()->Auto_Saving = true;
		for (int i = 0; i < static_cast<int>(worlds.size()); i++) {
			save_world(worlds[i].name, false);
		}
		Environment()->Auto_Saving = false;
		VarList::OnConsoleMessage(peer, "inactive worlds cleared: " + to_string(worlds.size()) + "!");
	}
	static void REMOVE_DROP(ENetPeer* peer) {
		if (pInfo(peer)->world == "GROWCH" and not Role::Clist(pInfo(peer)->tankIDName)) return;
		VarList::OnConsoleMessage(peer, ">> Removed all dropped items.");
		PlayerMoving data_{};
		data_.packetType = 14, data_.netID = 0;
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != pInfo(peer)->world) continue;
				for (int i_ = 0; i_ < world_->drop_new.size(); i_++) {
					data_.plantingTree = world_->drop_new[i_][2];
					BYTE* raw = packPlayerMoving(&data_);
					send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
					delete[]raw;
				}
			}
			world_->drop_new.clear();
		}
	}
	static void CLEARDROP(ENetPeer* peer) {
		if (pInfo(peer)->world.empty()) {
			VarList::OnConsoleMessage(peer, "`4Error: You must be in a world to use this command.");
			return;
		}

		std::string world_name = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [world_name](const World& a) { return a.name == world_name; });
		if (it == worlds.end()) {
			VarList::OnConsoleMessage(peer, "`4Error: World data not found.");
			return;
		}

		World* world_ = &(*it);
		if (to_lower(world_->owner_name) != to_lower(pInfo(peer)->tankIDName)) {
			VarList::OnConsoleMessage(peer, "`4Error: You are not the owner of this world.");
			return;
		}

		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n\n"
			"add_label_with_icon|big|`wAre You Serious?|left|1456|\n"
			"add_smalltext|`5NOTE: `oThis action will permanently remove `wALL dropped items `of this world. Once cleared, it `ocannot be undone!|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`oWorld: `w" + world_->name + "|left|\n"
			"add_textbox|`oOwner: `w" + world_->owner_name + "|left|\n"
			"add_spacer|small|\n"
			"add_button|clear_confirm|`4Clear All Drops|noflags|0|0|1|\n"
			"add_smalltext|`o>> Click `4Clear All Drops `oto permanently delete all " + std::to_string(world_->drop_new.size()) + " items dropped in this world.|left|\n"
			"add_spacer|small|\n"
			"end_dialog|cleardrop_confirm||";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void RPL(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == pInfo(peer)->world) {
					pInfo(cp_)->last_world = pInfo(cp_)->world;
					for (int i_ = 0; i_ < world_->blocks.size(); i_++) {
						if (world_->blocks[i_].fg == 1000) {
							world_->blocks[i_].fg = 0;
						}
					}
					VarList::OnAddNotification(cp_, "`2Public Lava!: `wis Ended!", "interface/large/special_event.rttex", "audio/pinata_lasso.wav");
					Exit_World(cp_, true);
					Enter_World(cp_, pInfo(cp_)->last_world);
				}
			}
		}
	}
	static void SPL(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == pInfo(peer)->world) {
					pInfo(cp_)->last_world = pInfo(cp_)->world;
					for (int i_ = 0; i_ < world_->blocks.size(); i_++) {
						if (world_->blocks[i_].fg == 0) {
							world_->blocks[i_].fg = 1000;
						}
					}
					VarList::OnAddNotification(cp_, "`2Public Lava!: `wLava is being placed in world `4" + pInfo(peer)->world + "`w, go break it and grab some extra `5Gems!", "interface/large/special_event.rttex", "audio/pinata_lasso.wav");
					Exit_World(cp_, true);
					Enter_World(cp_, pInfo(cp_)->last_world);
				}
			}
		}
	}
	static void SDROP(ENetPeer* peer, std::string cmd) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const auto& a) { return a.name == name_; });

		if (it != worlds.end()) {
			World* world_ = &(*it);

			if (cmd.empty()) {
				VarList::OnConsoleMessage(peer, "`oUsage: /sdrop <id> <count_per_block>");
				return;
			}

			std::istringstream ss(cmd);
			int itemId, countPerBlock;

			try {
				if (ss >> itemId >> countPerBlock) {
					if (itemId < 1 || itemId > items.size()) {
						VarList::OnConsoleMessage(peer, "`8>> `4Oops: `8Invalid items id!");
						return;
					}

					if (countPerBlock < 1 || countPerBlock > 200) {
						VarList::OnConsoleMessage(peer, "`8>> `4Oops: `8Count must be 1-200 per block!");
						return;
					}

					if (items[itemId].name.find("null_item") != string::npos ||
						items[itemId].name.find("null") != string::npos ||
						items[itemId].name.find("Guild Entrance") != string::npos ||
						items[itemId].name.find("Guild Key") != string::npos ||
						items[itemId].name.find("World Key") != string::npos ||
						itemId == 5640 || itemId == 9158 || itemId == 5814 || itemId == 5816) {
						VarList::OnConsoleMessage(peer, "`8> `4Oops: `8You can't drop this item!");
						return;
					}
					const int WORLD_WIDTH = 100;
					const int WORLD_HEIGHT = 60;
					const int BLOCK_SIZE = 32;

					for (int x = 0; x < WORLD_WIDTH; x++) {
						for (int y = 0; y < WORLD_HEIGHT; y++) {
							WorldDrop drop{};
							drop.x = x * BLOCK_SIZE + (BLOCK_SIZE / 2);
							drop.y = y * BLOCK_SIZE + (BLOCK_SIZE / 2);
							drop.id = itemId;
							drop.count = countPerBlock;
							VisualHandle::Drop(world_, drop);
						}
					}

					VarList::OnConsoleMessage(peer, "Filled world with " + std::to_string(countPerBlock) +
						"x of item " + std::to_string(itemId) + " on every block!");
				}
				else {
					VarList::OnConsoleMessage(peer, "`oUsage: /sdrop <id> <count_per_block>");
				}
			}
			catch (const std::exception&) {
				VarList::OnConsoleMessage(peer, "`oInvalid command format!");
			}
		}
	}
	static void GROWALL(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			int ySize = world_->blocks.size() / 100, xSize = world_->blocks.size() / ySize;
			for (int f = 0; f < world_->blocks.size(); f++) {
				int xx = f % xSize, yy = f / xSize;
				WorldBlock* block_ = &world_->blocks[f];
				if (items[world_->blocks[f].fg].blockType == PROVIDER || items[world_->blocks[f].fg].blockType == SEED) {
					world_->blocks[f].planted = 0;
				}
			}
			std::string name = pInfo(peer)->world;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == name) {
					VarList::OnAddNotification(cp_, "`wWarning from `4System``: All Provider and seeds in this world has been Grow!", "interface/atomic_button.rttex", "audio/hub_open.wav");
					pInfo(cp_)->spray_x = pInfo(cp_)->x;
					pInfo(cp_)->spray_y = pInfo(cp_)->y;
					Exit_World(cp_, true);
					pInfo(cp_)->x = pInfo(cp_)->spray_x;
					pInfo(cp_)->y = pInfo(cp_)->spray_y;
					Enter_World(cp_, name, pInfo(cp_)->spray_x / 32, pInfo(cp_)->spray_y / 32);
					pInfo(cp_)->x = pInfo(cp_)->spray_x;
					pInfo(cp_)->y = pInfo(cp_)->spray_y;
					pInfo(cp_)->spray_x = 0;
					pInfo(cp_)->spray_y = 0;
				}
			}
		}
	}
	static void GSM(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /gsm <text>");
			return;
		}
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[S]_ `o**`` `4Global System Message:`` `$" + cmd + "``");
			CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/sungate.wav\ndelayMS|0\n");
		}
	}
	static void SPK(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /spk <GrowID> <message>`");
			return;
		}
		std::istringstream ss(cmd);
		std::string targetName;
		ss >> targetName;
		std::string message;
		std::getline(ss, message);
		if (targetName.empty() || message.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /spk <GrowID> <message>`");
			return;
		}
		if (!message.empty() && message[0] == ' ')
			message.erase(0, 1);
		bool playerFound = false;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL)
				continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(targetName)) {
				chat_message(cp_, message);
				playerFound = true;
				break;
			}
		}
		if (!playerFound) {
			VarList::OnConsoleMessage(peer, "`4Player with GrowID `" + targetName + "` not found or offline.`");
		}
	}
	static void ASB(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /gsm <text>");
			return;
		}
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			VarList::OnConsoleMessage(cp_, cmd);
			VarList::OnAddNotification(cp_, cmd, "interface/atomic_button.rttex", "audio/hub_open.wav");
		}
	}
	static void INFOEX(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /infoex <player name>");
			return;
		}
		bool found = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				found = true;
				VarList::OnConsoleMessage(peer, "`oTankIDName|" + pInfo(cp_)->tankIDName + "\nrequestedName|" + pInfo(cp_)->requestedName + "\nf|" + pInfo(cp_)->f + "\nprotocol|" + to_string(pInfo(cp_)->protocol) + "\ngame_version|" + pInfo(cp_)->game_version + "\nfz|" + pInfo(cp_)->fz + "\nlmode|" + pInfo(cp_)->lmode + "\ncbits|" + pInfo(cp_)->cbits + "\nplayer_age|" + pInfo(cp_)->player_age + "\nGDPR|" + pInfo(cp_)->GDPR + "\nmeta|" + pInfo(cp_)->meta + "\nfhash|" + pInfo(cp_)->fhash + "\nrid|" + pInfo(cp_)->rid + "\nplatfromID|" + pInfo(cp_)->player_device + "\nDeviceVersion|" + pInfo(cp_)->deviceVersion + "\ncountry|" + pInfo(cp_)->country + "\nhash|" + pInfo(cp_)->hash + "\nmac|" + pInfo(cp_)->mac + "\nwk|" + pInfo(cp_)->wk + "\ngid|" + pInfo(cp_)->gid + "\nzf|" + pInfo(cp_)->zf + "");
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static bool is_valid_ip(const std::string& ip) {
		if (ip.empty()) return false;

		int dots = 0;
		int parts = 0;
		std::stringstream ss(ip);
		std::string part;

		while (std::getline(ss, part, '.')) {
			if (part.empty() || part.size() > 3) return false;
			if (!std::all_of(part.begin(), part.end(), ::isdigit)) return false;

			int value = std::atoi(part.c_str());
			if (value < 0 || value > 255) return false;

			parts++;
		}

		for (char c : ip) {
			if (c == '.') dots++;
			else if (!std::isdigit(static_cast<unsigned char>(c))) return false;
		}

		return dots == 3 && parts == 4;
	}
	static void BANRID(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /banrid <player name / ip>");
			return;
		}

		// trim sederhana
		cmd.erase(0, cmd.find_first_not_of(" \t\r\n"));
		cmd.erase(cmd.find_last_not_of(" \t\r\n") + 1);

		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /banrid <player name / ip>");
			return;
		}

		// support ban langsung by IP
		if (is_valid_ip(cmd)) {
			auto it = std::find(Environment()->Rid_Bans.begin(), Environment()->Rid_Bans.end(), cmd);
			if (it != Environment()->Rid_Bans.end()) {
				Environment()->Rid_Bans.erase(
					std::remove(Environment()->Rid_Bans.begin(), Environment()->Rid_Bans.end(), cmd),
					Environment()->Rid_Bans.end()
				);

				VarList::OnConsoleMessage(peer, "`o>> Unbanned IP " + cmd);
				ServerPool::Logs::Add(pInfo(peer)->tankIDName + " has un-banned ip " + cmd + ".", "BAN RID");
			}
			else {
				Environment()->Rid_Bans.push_back(cmd);

				VarList::OnConsoleMessage(peer, "`o>> Banned IP " + cmd);
				ServerPool::Logs::Add(pInfo(peer)->tankIDName + " has banned ip " + cmd + ".", "BAN RID");
				VarList::OnConsoleMessage(peer, "`#**`` `$The Gods `ohave used `5ban`` on IP " + cmd + "`o from " + Environment()->server_name + " `#**`` (`4/rules`` to see the rules!)", true);

				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
					if (pInfo(cp_)->ip == cmd) {
						VarList::OnAddNotification(cp_, "`wWarning from `4System`w: You've been `4BANNED IP `wfrom " + Environment()->server_name + "!", "interface/atomic_button.rttex", "audio/hub_open.wav");
						Peer_Disconnect(cp_, 0);
					}
				}
			}
			return;
		}

		// fallback: anggap cmd sebagai nama player
		std::string name = cmd;
		std::ifstream ifs("database/players/" + name + "_.json");
		if (!ifs.is_open()) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
			return;
		}

		try {
			json j;
			ifs >> j;

			std::string rid_or_ip = "";

			if (j.contains("rid") && j["rid"].is_string() && !j["rid"].get<std::string>().empty()) {
				rid_or_ip = j["rid"].get<std::string>();
			}
			else if (j.contains("ip") && j["ip"].is_string() && !j["ip"].get<std::string>().empty()) {
				rid_or_ip = j["ip"].get<std::string>();
			}

			if (rid_or_ip.empty()) {
				VarList::OnConsoleMessage(peer, "`4Failed: player data has no RID/IP.");
				return;
			}
			std::string player_name = j.contains("name") && j["name"].is_string() ? j["name"].get<std::string>() : name;

			auto it = std::find(Environment()->Rid_Bans.begin(), Environment()->Rid_Bans.end(), rid_or_ip);
			if (it != Environment()->Rid_Bans.end()) {
				Environment()->Rid_Bans.erase(
					std::remove(Environment()->Rid_Bans.begin(), Environment()->Rid_Bans.end(), rid_or_ip),
					Environment()->Rid_Bans.end()
				);

				VarList::OnConsoleMessage(peer, "`o>> Unbanned RID/IP " + player_name);
				ServerPool::Logs::Add(pInfo(peer)->tankIDName + " has un-banned rid/ip " + player_name + ".", "BAN RID");
			}
			else {
				Environment()->Rid_Bans.push_back(rid_or_ip);

				VarList::OnConsoleMessage(peer, "`o>> Banned RID/IP " + player_name);
				ServerPool::Logs::Add(pInfo(peer)->tankIDName + " has banned rid/ip " + player_name + ".", "BAN RID");
				VarList::OnConsoleMessage(peer, "`#**`` `$The Gods `ohave used `5ban`` on " + player_name + "`o from " + Environment()->server_name + " `#**`` (`4/rules`` to see the rules!)", true);

				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;

					if (pInfo(cp_)->tankIDName == player_name || pInfo(cp_)->rid == rid_or_ip || pInfo(cp_)->ip == rid_or_ip) {
						VarList::OnAddNotification(cp_, "`wWarning from `4System`w: You've been `4BANNED RID/IP `wfrom " + Environment()->server_name + "!", "interface/atomic_button.rttex", "audio/hub_open.wav");
						Peer_Disconnect(cp_, 0);
					}
				}
			}
		}
		catch (...) {
			VarList::OnConsoleMessage(peer, "`4Failed to read player data.");
		}
	}
	static void BANIP(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /banip <player name>");
			return;
		}
		std::string name = cmd;
		std::ifstream ifs("database/players/" + name + "_.json");
		if (ifs.is_open()) {
			json j;
			ifs >> j;
			if (find(Environment()->Rid_Bans.begin(), Environment()->Rid_Bans.end(), j["ip"].get<string>()) != Environment()->Rid_Bans.end()) {
				Environment()->Rid_Bans.erase(remove(Environment()->Rid_Bans.begin(), Environment()->Rid_Bans.end(), j["ip"].get<string>()), Environment()->Rid_Bans.end());
				VarList::OnConsoleMessage(peer, "`o>> Unbanned IP " + j["name"].get<string>());
				ServerPool::Logs::Add(pInfo(peer)->tankIDName + " has un-banned ip " + j["name"].get<string>() + ".", "BAN IP");
			}
			else {
				Environment()->Rid_Bans.push_back(j["ip"].get<string>());
				VarList::OnConsoleMessage(peer, "`o>> Banned IP " + j["name"].get<string>());
				ServerPool::Logs::Add(pInfo(peer)->tankIDName + " has banned ip " + j["name"].get<string>() + ".", "BAN IP");
				VarList::OnConsoleMessage(peer, "`#**`` `$The Gods `ohave used `5ban`` on " + j["name"].get<string>() + "`o from " + Environment()->server_name + " `#**`` (`4/rules`` to see the rules!)", true);
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (pInfo(cp_)->tankIDName == j["name"].get<string>()) {
						VarList::OnAddNotification(cp_, "`wWarning from `4System`w: You've been `4BANNED IP `wfrom " + Environment()->server_name + "!", "interface/atomic_button.rttex", "audio/hub_open.wav");
						Peer_Disconnect(cp_, 0);
					}
				}
			}
		}
		else VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void CHECK_PLAYMODS(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /checkplaymods <player name>");
			return;
		}
		bool found = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				found = true;
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label|big|`wPlaymods of " + pInfo(cp_)->tankIDName + "|\nadd_spacer|small|" + form_mods(pInfo(cp_), 0) + "\nadd_spacer|small|\nend_dialog||Close||");
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void FREEZEALL(ENetPeer* peer) {
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false or pInfo(cp_)->world != pInfo(peer)->world) continue;
			if (not Playmods::HasById(pInfo(cp_), 2) && not Playmods::HasById(pInfo(cp_), 3) && not Playmods::HasById(pInfo(cp_), 49)) {
				Playmods::Add(cp_, 2);
				VisualHandle::State(pInfo(cp_));
				Clothing_V2::Update(cp_, true);
			}
		}
	}
	static void GHOST(ENetPeer* peer) {
		if (!peer || peer->data == NULL) return;
		if (pInfo(peer)->world.find("GROWMINES") != std::string::npos) return;
		bool admin_plus =
			Role::GetLevel(peer) > 0 ||
			Role::Owner(peer) ||
			Role::Clist(pInfo(peer)->tankIDName) ||
			Role::Staff(peer) ||
			Role::Developer(peer) ||
			Role::Administrator(peer) ||
			Role::Moderator(peer) ||
			Role::Vip(peer) ||
			Role::Donatur(peer) ||
			Role::Streamers(peer) ||
			Role::God(peer) ||
			Role::Unlimited(peer) ||
			Role::Coder(peer) ||
			pInfo(peer)->Role.has_config_access ||
			pInfo(peer)->GhostCommandAccess;
		if (!admin_plus) {
			if (pInfo(peer)->world.empty()) {
				VarList::OnConsoleMessage(peer, "`4You can only use /ghost in your own world.");
				return;
			}
			World w = get_world(pInfo(peer)->world);
			if (to_lower(w.owner_name) != to_lower(pInfo(peer)->tankIDName)) {
				VarList::OnConsoleMessage(peer, "`4Only the world owner can use /ghost here.");
				return;
			}
		}
		CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
		pInfo(peer)->ghost = !pInfo(peer)->ghost;
		std::string status = pInfo(peer)->ghost ?
			"`oQuantum tunneling activated! (`$Ghost Mode ON``)" :
			"`oMolecular cohesion restored! (`$Ghost Mode OFF``)";

		VarList::OnConsoleMessage(peer, status);
		VisualHandle::State(pInfo(peer));
		Clothing_V2::Update(peer);
	}
	static void GHOSTWORLD(ENetPeer* peer) {
		auto info = pInfo(peer);
		if (info->world != info->tankIDName) return;
		if (info->world.find("GROWMINES") != std::string::npos || info->world == "GROWCH")
			if (!Role::Clist(info->tankIDName)) return;
		CAction::Positioned(peer, info->netID, "audio/dialog_confirm.wav", 0);
		info->ghost = (info->ghost ? false : true);
		if (info->ghost == false) VarList::OnConsoleMessage(peer, "oYour body stops shimmering and returns to normal. ($Ghost in the Shell omod removed)`");
		else VarList::OnConsoleMessage(peer, "oYour atoms are suddenly aware of quantum tunneling. ($Ghost in the Shell omod added)`");
		VisualHandle::State(info);
		Clothing_V2::Update(peer);
	}
	static void SETMUSIC(ENetPeer* peer, const std::string& param) {
		if (!peer || peer->data == NULL) return;
		// Validasi parameter.
		std::string path = param;
		// Trim whitespace.
		while (!path.empty() && (path.front() == ' ' || path.front() == '\t')) path.erase(path.begin());
		while (!path.empty() && (path.back() == ' ' || path.back() == '\t' || path.back() == '\r' || path.back() == '\n')) path.pop_back();

		// Cari world yang sedang ditempati player.
		auto it = std::find_if(worlds.begin(), worlds.end(), [&](const World& w) { return w.name == pInfo(peer)->world; });
		if (it == worlds.end()) {
			VarList::OnConsoleMessage(peer, "`4World not found.``");
			return;
		}
		World* world_ = &(*it);

		DialogHandle::LoadCustomSongs();

		if (path.empty()) {
			std::string msg = "`2[World Music List - Spotify Custom Songs]``\n";
			if (!DialogHandle::g_custom_songs.empty()) {
				for (size_t i = 0; i < DialogHandle::g_custom_songs.size(); ++i) {
					msg += "`w" + std::to_string(i + 1) + ". `o" + DialogHandle::g_custom_songs[i].name + "\n";
				}
			} else {
				msg += "`4(Tidak ada lagu di database/spotify_custom.json)``\n";
			}
			msg += "`7Gunakan: `/setmusic <nomor>` (contoh: `/setmusic 1`) untuk memasang musik di world, atau `/setmusic 0` / `/setmusic off` untuk menghapus musik.";
			VarList::OnConsoleMessage(peer, msg);
			return;
		}

		if (path == "0" || to_lower(path) == "stop" || to_lower(path) == "off" || to_lower(path) == "none") {
			// Hapus music world.
			world_->worldMusic = "";
			// Simpan world ke database.
			save_world(world_->name, false);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || !cp_->data) continue;
				if (pInfo(cp_)->world != world_->name) continue;
				CAction::Log(cp_, "action|play_music\nfile|\ndelayMS|0");
			}
			// Logging.
			ServerPool::Logs::Add("[SETMUSIC] Player: " + pInfo(peer)->tankIDName + " | World: " + world_->name + " | Music removed", "SetMusic");
			VarList::OnConsoleMessage(peer, "`2World music removed successfully.``");
			return;
		}

		std::string music_file = "";
		std::string music_title = "";

		if (path.find_first_not_of("0123456789") == std::string::npos) {
			if (DialogHandle::g_custom_songs.empty()) {
				VarList::OnConsoleMessage(peer, "`4[ERROR]`` `oTidak ada lagu di database/spotify_custom.json!");
				return;
			}
			int num = std::atoi(path.c_str());
			if (num < 1 || num > (int)DialogHandle::g_custom_songs.size()) {
				VarList::OnConsoleMessage(peer, "`4[ERROR]`` `oNomor lagu tidak valid! Pilih nomor antara 1 - " + std::to_string(DialogHandle::g_custom_songs.size()));
				return;
			}
			int idx = num - 1;
			auto& song = DialogHandle::g_custom_songs[idx];
			music_file = song.path;
			music_title = "#" + std::to_string(num) + " - " + song.name;
		} else {
			// Tolak karakter berbahaya / path traversal.
			if (path.find("..") != std::string::npos ||
				path.find("\\") != std::string::npos ||
				path.find("//") != std::string::npos ||
				path.find('\0') != std::string::npos) {
				VarList::OnConsoleMessage(peer, "`4Invalid path: path traversal is not allowed.``");
				return;
			}
			// Hanya izinkan ekstensi: .ogg, .mp3, .wav
			std::string lower_path = path;
			std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::tolower);
			bool valid_ext = false;
			if (lower_path.size() >= 4) {
				std::string ext = lower_path.substr(lower_path.size() - 4);
				if (ext == ".ogg" || ext == ".mp3" || ext == ".wav") valid_ext = true;
			}
			if (!valid_ext) {
				VarList::OnConsoleMessage(peer, "`4[ERROR]`` `oGunakan nomor lagu (1 - " + std::to_string(DialogHandle::g_custom_songs.size()) + ") atau `/setmusic 0` untuk menghapus musik.");
				return;
			}
			music_file = path;
			music_title = path;
		}

		// Set music world.
		world_->worldMusic = music_file;
		// Simpan world ke database.
		save_world(world_->name, false);
		// Putar music ke semua player yang ada di world.
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || !cp_->data) continue;
			if (pInfo(cp_)->world != world_->name) continue;
			CAction::Log(cp_, "action|play_music\nfile|" + music_file + "\ndelayMS|0");
		}
		// Logging.
		ServerPool::Logs::Add("[SETMUSIC] Player: " + pInfo(peer)->tankIDName + " | World: " + world_->name + " | Music: " + music_file, "SetMusic");
		VarList::OnConsoleMessage(peer, "`2World music set to: `w" + music_title + "``");
	}
	static void BANALL(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->tankIDName == pInfo(cp_)->tankIDName or pInfo(peer)->world != pInfo(cp_)->world) continue;
				if (pInfo(peer)->Role.Role_Level <= 4) {
					VarList::OnConsoleMessage(cp_, get_player_nick(peer) + " `4world bans`` " + get_player_nick(cp_) + " from `0" + pInfo(cp_)->world + "``!");
					VarList::OnConsoleMessage(peer, get_player_nick(peer) + " `4world bans`` " + get_player_nick(cp_) + " from `0" + pInfo(cp_)->world + "``!");
					world_->bannedPlayers.push_back(make_pair(to_lower(pInfo(cp_)->tankIDName), (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()));
					VarList::OnPlaySound(cp_, "audio/repair.wav");
					Exit_World(cp_);
				}
			}
		}
	}
	static void ONE_HIT(ENetPeer* peer) {
		if (pInfo(peer)->world.find("GROWMINES") != std::string::npos) return;
		pInfo(peer)->hit1 = pInfo(peer)->hit1 ? false : true;
		VarList::OnConsoleMessage(peer, "1HIT " + a + (pInfo(peer)->hit1 == false ? "disabled" : "enabled") + a + ".");
	}
	static void PULLALL(ENetPeer* peer) {
		int pullx = pInfo(peer)->x, pully = pInfo(peer)->y;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or cp_ == peer) continue;
			if (pInfo(peer)->world == pInfo(cp_)->world) {
				if (not pInfo(cp_)->invis) {
					pInfo(cp_)->x = pullx, pInfo(cp_)->y = pully;
					VarList::OnSetPos(cp_, pullx, pully);
					VarList::OnTextOverlay(cp_, "You were pulled by " + pInfo(peer)->tankIDName);
				}
			}
		}
	}
	static void RPULL(ENetPeer* peer) {
		std::vector<ENetPeer*> targets;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or cp_ == peer) continue;
			if (pInfo(peer)->world == pInfo(cp_)->world and not pInfo(cp_)->invis) targets.push_back(cp_);
		}
		if (targets.empty()) {
			VarList::OnConsoleMessage(peer, "`oNo one else is in your world to pull.");
			return;
		}
		ENetPeer* target = targets[rand() % targets.size()];
		pInfo(target)->x = pInfo(peer)->x;
		pInfo(target)->y = pInfo(peer)->y;
		VarList::OnSetPos(target, pInfo(peer)->x, pInfo(peer)->y);
		VarList::OnTextOverlay(target, "You were randomly pulled by " + pInfo(peer)->tankIDName);
		VarList::OnConsoleMessage(peer, "`oRandomly pulled `w" + pInfo(target)->tankIDName + "`o to your location.");
	}
	static void DISABLE_SUMMON(ENetPeer* peer) {
		pInfo(peer)->d_s = (pInfo(peer)->d_s == 0 ? 1 : 0);
		VarList::OnConsoleMessage(peer, "`o>> You " + a + (pInfo(peer)->d_s ? "DISABLE" : "ACTIVE") + " summon from another players.``");
	}
	static void LOGS(ENetPeer* peer) {
		pInfo(peer)->search_page = 20;
		std::string buttons = "";
		for (int i = 0; i < Environment()->Logs.size(); i++) buttons += "\nadd_custom_button|" + Environment()->Logs[i].first + "|textLabel:`w" + Environment()->Logs[i].first + ";middle_colour:75947213;border_colour:75947213;|\nadd_custom_break|\nadd_spacer|small|";
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`w" + Environment()->server_name + " Logs System``|left|3732|\nadd_spacer|small|" + buttons + "|" + (Role::Owner(peer) ? "\nadd_spacer|small|\nadd_textbox|Empty All Logs:|left|\nadd_inner_image_label_button|Empty Logs|  `4Empty Logs``|game/tiles_page1.rttex|1.6|16|1|32|" : "") + "|\nend_dialog|logs|Cancel||");
	}
	static void REPORTS(ENetPeer* peer) {
		std::string list = "";
		for (int w_ = 0; w_ < Environment()->World_Report.size(); w_++) list += "\nadd_button|warp_to_" + Environment()->World_Report[w_].first + "|`w#" + to_string(w_ + 1) + " " + Environment()->World_Report[w_].first + "|0|0|\nadd_smalltext|`oREPORT: " + Environment()->World_Report[w_].second + "|left|";
		if (list.empty()) list = "\nadd_textbox|`oNo reports have been received yet.|left|";
		VarList::OnDialogRequest(peer, SetColor(peer) + "\nadd_label_with_icon|big|`wWorld Reports|left||image:game/tiles_page14.rttex;frame:30,23;frameSize:32;|\nadd_spacer|small|" + list + "\nadd_spacer|small|\nend_dialog|top|Close||\nadd_quick_exit|");
	}
	static void PLAYER_REPORT(ENetPeer* peer) {
		std::string list = "";
		for (int w_ = 0; w_ < Environment()->Player_Report.size(); w_++) list += "\nadd_button|info_" + Environment()->Player_Report[w_].first + "|`w#" + to_string(w_ + 1) + " " + Environment()->Player_Report[w_].first + "|0|0|\nadd_smalltext|`oREPORT: " + Environment()->Player_Report[w_].second + "|left|";
		if (list.empty()) list = "\nadd_textbox|`oNo reports have been received yet.|left|";
		VarList::OnDialogRequest(peer, SetColor(peer) + "\nadd_label_with_icon|big|`wPlayer Reports|left||image:game/tiles_page14.rttex;frame:30,23;frameSize:32;|\nadd_spacer|small|" + list + "\nadd_spacer|small|\nend_dialog|punish_view|Close||\nadd_quick_exit|");
	}
	static void UN_CURSEALL(ENetPeer* peer) {
		int uncurse_ = 0;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (Playmods::HasById(pInfo(cp_), 139, 1)) {
				uncurse_++;
			}
		}
		VarList::OnConsoleMessage(peer, "Uncursed people: " + to_string(uncurse_));
	}
	static void COLOR(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /color <number>");
			return;
		}
		int skin = std::atoi(cmd.c_str());
		pInfo(peer)->skin = skin;
		Clothing_V2::Update(peer);
	}
	static void CHECKID(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /checkid <item name>");
			return;
		}
		std::string find_target = cmd, id_list = "";
		if (find_target.size() < 3) {
			VarList::OnConsoleMessage(peer, "Item name must be more than 2 letters!");
			return;
		}
		for (int i_ = 0; i_ < items.size(); i_++) {
			uint32_t item_id = items[i_].id;
			if (item_id == 2950 || item_id == 14576 || items[item_id].blockType == SEED || items[item_id].name.find("null_item") != string::npos || items[item_id].name.find("null") != string::npos || items[item_id].name.find("Guild Flag") != string::npos || items[item_id].name.find("Guild Entrance") != string::npos || items[item_id].name.find("Guild Banner") != string::npos || items[item_id].name.find("Guild Key") != string::npos || items[item_id].name.find("World Key") != string::npos || item_id == 5640 || item_id == 5814 || item_id == 1486 || item_id == 6802 || item_id == 5070 || item_id == 5072 || item_id == 5074 || item_id == 5076 || item_id == 18 || item_id == 32 || item_id == 6336 || item_id == 9384 || item_id == 9158) continue;
			if (to_lower(fixchar2(items[i_].ori_name)).find(find_target) != string::npos) {
				id_list += "\nadd_label_with_icon|small|`o- " + items[item_id].ori_name + " ID : [" + to_string(item_id) + "]|left|" + to_string(item_id) + "|\n";
			}
		}
		if (id_list.empty()) {
			VarList::OnConsoleMessage(peer, " `4Oops: `oThere is no item starting with `w" + find_target + "`o.");
			return;
		}
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wFound item : " + find_target + "``|left|6016|\nadd_spacer|small|\ntext_scaling_string|Subscribtions++++++++|" + id_list + "|\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nend_dialog||Close||\nadd_quick_exit|\n");
	}
	static void CHANGE_NICK(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			pInfo(peer)->d_name = "";
			VarList::OnConsoleMessage(peer, "Your name changed back to " + pInfo(peer)->tankIDName);
			VisualHandle::Nick(peer, NULL);
			return;
		}
		if (cmd.size() <= 2) {
			VarList::OnConsoleMessage(peer, "Usage: /nick <`$nickname``> - This will change your Display Name.");
			return;
		}
		pInfo(peer)->d_name = cmd;
		VarList::OnConsoleMessage(peer, "Your name changed to " + cmd);
		ServerPool::Logs::Add(pInfo(peer)->tankIDName + " Change Name To -> " + cmd, "Nick");
		if (pInfo(peer)->is_legend) VisualHandle::Nick(peer, NULL);
		else VisualHandle::Nick(peer, NULL);
	}
	static void CHANGE_TITLE(ENetPeer* peer, std::string cmd) {
		if (!Role::Owner(peer) && !Role::Clist(pInfo(peer)->tankIDName)) {
			VarList::OnConsoleMessage(peer, "`4System: Only Owners can use this command.``");
			return;
		}
		std::string dialog = "set_default_color|`o\n"
			"set_bg_color|0,52,102,178|\n"
			"set_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wCustom Title Settings``|left|1366|\n"
			"add_spacer|small|\n"
			"add_textbox|`oSet or update custom title tag. Admin/Mods can set titles for other players.``|left|\n"
			"add_spacer|small|\n"
			"add_text_input|title_text|Custom Title:|" + pInfo(peer)->custom_title + "|30|\n"
			"add_text_input|target_name|Target Player Name (blank for self):||24|\n"
			"add_checkbox|show_title|Show Custom Title Tag|" + std::string(pInfo(peer)->show_custom_title ? "1" : "0") + "|\n"
			"add_spacer|small|\n"
			"end_dialog|custom_title_dialog|Cancel|Apply|\n";
		VarList::OnDialogRequest(peer, dialog);
	}
	static void FAKEBAN(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /fakeban <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				VarList::OnAddNotification(cp_, "`wWarning from `4System`w: You've been `4BANNED from `wGrowtopia for 730 Day", "interface/atomic_button.rttex", "audio/hub_open.wav");
				break;
			}
		}
	}
	static void FAKEWARN(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /fakewarn <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				VarList::OnAddNotification(cp_, "`wWarning from Admin``: you have warned by an `4Admin", "interface/atomic_button.rttex", "audio/hub_open.wav");
				break;
			}
		}
	}
	static void FLAG(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /flag <country>");
			return;
		}
		std::string flag = cmd;
		pInfo(peer)->country = flag;
		gamepacket_t p(0, pInfo(peer)->netID);
		p.Insert("OnCountryState");
		p.Insert(flag);
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (pInfo(cp_)->world == pInfo(peer)->world) p.CreatePacket(cp_);
		}
	}
	static void GUILDCLASH(ENetPeer* peer) {
		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wGuild Clash Event``|left|15856|\n"
			"add_spacer|small|\n"
			"add_textbox|`oWelcome to the `9Guild Clash Event``! Compete with other guilds to prove your supremacy and win exclusive rewards!|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`5[EVENT STATUS]``|left|\n"
			"add_smalltext|`oCurrent Season: `2Season 1 (Active)``|\n"
			"add_smalltext|`oStatus: `2CLASH IS ONGOING!``|\n"
			"add_spacer|small|\n";

		if (pInfo(peer)->guild_id != 0) {
			Guild* g = nullptr;
			for (auto& guild : guilds) {
				if (guild.guild_id == pInfo(peer)->guild_id) {
					g = &guild;
					break;
				}
			}
			if (g) {
				dialog += "add_textbox|`5[YOUR GUILD STATS]``|left|\n";
				dialog += "add_smalltext|`oGuild Name: `2" + g->guild_name + "`` (Level " + to_string(g->guild_level) + ")|\n";
				dialog += "add_smalltext|`oGuild XP: `$" + Set_Count(g->guild_xp) + "``|\n";
				dialog += "add_smalltext|`oTotal Members: `w" + to_string(g->guild_members.size()) + "``|\n";
				dialog += "add_smalltext|`oGuild World: `5" + (g->guild_world.empty() ? "None" : g->guild_world) + "``|\n";
				dialog += "add_spacer|small|\n";
			}
		} else {
			dialog += "add_textbox|`4[NO GUILD]``|left|\n";
			dialog += "add_smalltext|`oYou are currently not in any Guild. Join or create a Guild to participate in Guild Clash!|\n";
			dialog += "add_spacer|small|\n";
		}

		dialog += "add_textbox|`5[CLASH OPTIONS]``|left|\n"
			"add_button|guild_clash_leaderboard|`5Guild Leaderboard``|noflags|0|0|\n"
			"add_button|event_clash_rewards_tree|`3Clash Rewards Exchange``|noflags|0|0|\n"
			"add_button|guild_clash_info|`9Clash Rules & Info``|noflags|0|0|\n";

		if (pInfo(peer)->guild_id != 0) {
			dialog += "add_button|warp_guild_world|`2Warp to Guild World``|noflags|0|0|\n";
		} else {
			dialog += "add_button|requestcreateguildpage|`2Create a Guild``|noflags|0|0|\n";
		}

		dialog += "add_spacer|small|\n"
			"add_quick_exit|\n"
			"end_dialog|guild_clash_menu_dlg||Close|";

		VarList::OnDialogRequest(peer, dialog, 500);
	}
	static void GUILDCLASH_LB(ENetPeer* peer) {
		std::vector<Guild> sorted_guilds = guilds;
		std::sort(sorted_guilds.begin(), sorted_guilds.end(), [](const Guild& a, const Guild& b) {
			if (a.guild_level != b.guild_level) return a.guild_level > b.guild_level;
			return a.guild_xp > b.guild_xp;
		});

		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wGuild Clash Leaderboard``|left|15856|\n"
			"add_spacer|small|\n"
			"add_textbox|`oTop Guilds competing in the Guild Clash Season:``|left|\n"
			"add_spacer|small|\n";

		if (sorted_guilds.empty()) {
			dialog += "add_textbox|`4No guilds recorded yet.``|left|\n";
		} else {
			int rank = 1;
			for (const auto& g : sorted_guilds) {
				if (rank > 10) break;
				std::string rank_color = (rank == 1 ? "`$" : rank == 2 ? "`9" : rank == 3 ? "`3" : "`w");
				dialog += "add_textbox|" + rank_color + "#" + to_string(rank) + " `2" + g.guild_name + "`` `o(Lvl " + to_string(g.guild_level) + ") - `$" + Set_Count(g.guild_xp) + " XP `o[" + to_string(g.guild_members.size()) + " Members]|left|\n";
				rank++;
			}
		}

		dialog += "add_spacer|small|\n"
			"add_button|guild_clash_menu|`wBack to Menu``|noflags|0|0|\n"
			"add_quick_exit|\n"
			"end_dialog|guild_clash_lb_dlg||Close|";

		VarList::OnDialogRequest(peer, dialog, 500);
	}
	static void GUILDCLASH_INFO(ENetPeer* peer) {
		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wGuild Clash Rules & Info``|left|15856|\n"
			"add_spacer|small|\n"
			"add_textbox|`5HOW TO EARN CLASH POINTS:``|left|\n"
			"add_smalltext|`o1. Level up your Guild by earning Guild XP through active gameplay.|\n"
			"add_smalltext|`o2. Participate in World Locks and Clash events with your guild members.|\n"
			"add_smalltext|`o3. Complete daily Guild goals and contribute to your Guild Vault.|\n"
			"add_spacer|small|\n"
			"add_textbox|`5SEASON REWARDS:``|left|\n"
			"add_smalltext|`o- `1Top 1 Guild:`` Exclusive Guild Trophy, 100x Clash Tokens, Special Role Badge.|\n"
			"add_smalltext|`o- `1Top 2-3 Guilds:`` Silver Trophy, 50x Clash Tokens.|\n"
			"add_smalltext|`o- `1Top 4-10 Guilds:`` Bronze Trophy, 25x Clash Tokens.|\n"
			"add_spacer|small|\n"
			"add_button|guild_clash_menu|`wBack to Menu``|noflags|0|0|\n"
			"add_quick_exit|\n"
			"end_dialog|guild_clash_info_dlg||Close|";

		VarList::OnDialogRequest(peer, dialog, 500);
	}
	static void GUILDCLASH_WARP(ENetPeer* peer) {
		if (pInfo(peer)->guild_id != 0) {
			for (const auto& g : guilds) {
				if (g.guild_id == pInfo(peer)->guild_id) {
					if (!g.guild_world.empty()) {
						VarList::OnConsoleMessage(peer, "`oWarping to your Guild World: `2" + g.guild_world + "``...");
						std::string gworld = g.guild_world;
						Enter_World(peer, gworld);
						return;
					} else {
						VarList::OnConsoleMessage(peer, "`4Your Guild does not have a designated Guild World set.``");
						return;
					}
				}
			}
		}
		VarList::OnConsoleMessage(peer, "`4You are not currently in a Guild.``");
	}
	static void SCANNING(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /scan <itemid>");
			return;
		}
		std::string console[8];
		int id = std::atoi(cmd.c_str()), shelfs = 0, sbox_count = 0, drop_count = 0, d_count = 0, p_count = 0, v_count = 0, db_count = 0, ds_count = 0, b_count = 0;
		if (id < 0 or id > items.size()) return;
		for (int i = 0; i < worlds.size(); i++) {
			for (int c = 0; c < worlds[i].sbox1.size(); c++) {
				if (worlds[i].sbox1[c].id == id && worlds[i].sbox1[c].count > 0) {
					sbox_count++;
					console[0] += "`5#" + to_string(sbox_count) + ": `oin [`9" + worlds[i].name + "``] >> (`$S-BOX: `2" + to_string(worlds[i].sbox1[c].count) + " " + items[id].name + "``), ";
				}
			}
			for (int j = 0; j < worlds[i].drop_new.size(); j++) {
				if (worlds[i].drop_new[j][0] == id && worlds[i].drop_new[j][1] > 0) {
					drop_count++;
					console[1] += "`5#" + to_string(drop_count) + ": `oin [`9" + worlds[i].name + "``] >> (`$DROP: `2" + to_string(worlds[i].drop_new[j][1]) + " " + items[id].name + "``), ";
				}
			}
			for (int w = 0; w < worlds[i].blocks.size(); w++) {
				for (int z = 0; z < worlds[i].blocks[w].donates.size(); z++) {
					if (worlds[i].blocks[w].donates[z].item == id && worlds[i].blocks[w].donates[z].count > 0) {
						d_count++;
						console[2] += "`5#" + to_string(d_count) + ": `oin [`9" + worlds[i].name + "``] >> (`$D-BOX: `2" + to_string(worlds[i].blocks[w].donates[z].count) + " " + items[id].name + "``), ";
					}
				}
				if (items[worlds[i].blocks[w].fg].blockType == BlockTypes::VENDING) {
					if (worlds[i].blocks[w].id == id && worlds[i].blocks[w].c_ != 0) {
						v_count++;
						console[3] += "`5#" + to_string(v_count) + ": `oin [`9" + worlds[i].name + "``] >> (`$VEND: `2" + to_string(worlds[i].blocks[w].c_) + " " + items[id].name + "``), ";
					}
					if (worlds[i].blocks[w].id == id && id == 242) {
						v_count++;
						console[3] += "`5#" + to_string(v_count) + ": `oin [`9" + worlds[i].name + "``] >> (`$VEND: `2" + to_string(worlds[i].blocks[w].pr) + " " + items[id].name + "``), ";
					}
				}
			}
		}
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|Scanning in All World|left|6016|\nadd_spacer|small|\nadd_textbox|`oScanning items:|left|\nadd_smalltext|`o" + items[id].name + "|left|\nadd_spacer|small|\nadd_label_with_icon|small|Storage Box|left|6286|\nadd_smalltext|`o" + (sbox_count != 0 ? console[0] : "Performance cleared.") + "|left|\nadd_spacer|small|\nadd_label_with_icon|small|Donation Box|left|1452|\nadd_smalltext|`o" + (d_count != 0 ? console[2] : "`oPerformance cleared.") + "|left|\nadd_spacer|small|\nadd_label_with_icon|small|Vending|left|2978|\nadd_smalltext|`o" + (v_count != 0 ? console[3] : "`oPerformance cleared.") + "|left|\nadd_spacer|small|\nadd_label_with_icon|small|Dropped|left|18|\nadd_smalltext|`o" + (drop_count != 0 ? console[1] : "`oPerformance cleared.") + "|left|\nadd_spacer|small|\nadd_smalltext|`2>> Every worlds database successfully scanned!|left|\nadd_spacer|small|\nend_dialog||Close||\nadd_quick_exit|");
	}
	static void CHECK_PING(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /checkping <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd;
		bool found = false;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				found = true;
				VarList::OnConsoleMessage(peer, "`o>> " + pInfo(cp_)->tankIDName + " Ping : " + to_string(cp_->roundTripTime) + "Ms");
				break;
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Player Not Found", 0, 1);
	}
	static void DR_DESTRUCTO(ENetPeer* peer) {
		pInfo(peer)->DrDes = true;
		pInfo(peer)->d_name = "`4Dr. Destructo";
		pInfo(peer)->country = "none";
		pInfo(peer)->face = 576, pInfo(peer)->shirt = 466, pInfo(peer)->pants = 468, pInfo(peer)->hair = 0, pInfo(peer)->necklace = 0, pInfo(peer)->back = 0, pInfo(peer)->feet = 42, pInfo(peer)->hand = 1010;
		pInfo(peer)->punched = 8;
		pInfo(peer)->ghost = true;
		VisualHandle::State(pInfo(peer));
		Clothing_V2::Update_Value(peer);
		Clothing_V2::Update(peer);
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == world_->name) {
					gamepacket_t p(0, pInfo(peer)->netID);
					p.Insert("OnCountryState");
					p.Insert(pInfo(peer)->country);
					p.CreatePacket(cp_);
					VarList::OnConsoleMessage(cp_, "`4Dr. Destructo `2Attacks!:`o Defeat `4Dr. Destructo`o!``");
					VarList::OnAddNotification(cp_, "`4Dr. Destructo `2Attacks!:`o Defeat `4Dr. Destructo`o!``", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
					CAction::Positioned(cp_, pInfo(cp_)->netID, "action|play_music\nfile|audio/fight_loop.wav", 0);
					std::string names = "`4Dr. Destructo";
					gamepacket_t p2(0, pInfo(peer)->netID);
					p2.Insert("OnNameChanged");
					p2.Insert(names);
					p2.Insert(get_player_custom_wrench(peer));
					p2.CreatePacket(cp_);
				}
				world_->special_event = true;
				world_->special_event_name = "Dr. Destructo";
				world_->last_special_event = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
				if (find(Environment()->Another_Worlds.begin(), Environment()->Another_Worlds.end(), world_->name) == Environment()->Another_Worlds.end()) Environment()->Another_Worlds.push_back(world_->name);
			}
		}
	}
	static void FIX_WORLD(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			VarList::OnConsoleMessage(peer, ">> Fixed the world.");
			ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, "`8fixed world``: `#" + pInfo(peer)->world, "");
			send_fix_world(world_);
		}
	}
	static void SIZE_WORLD(ENetPeer* peer, const std::string& cmd) {
		if (!Role::Clist(pInfo(peer)->tankIDName) && !Role::has_config_access(peer) && to_lower(pInfo(peer)->tankIDName) != "albin") {
			VarList::OnConsoleMessage(peer, "`4You don't have access to use this command.");
			return;
		}

		if (pInfo(peer)->world.empty()) {
			VarList::OnConsoleMessage(peer, "`4Error: You must be in a world to use this command.");
			return;
		}

		std::stringstream ss(cmd);
		int requested_x = 0, requested_y = 0;
		std::string extra;
		if (!(ss >> requested_x >> requested_y) || (ss >> extra)) {
			VarList::OnConsoleMessage(peer, "`oUsage: /sizeworld 255 <height>`` (`oexample: /sizeworld 255 255``)");
			return;
		}

		if (requested_x != 255) {
			VarList::OnConsoleMessage(peer, "`4This source still uses a fixed world width of `w255`` in many systems. Use `w/sizeworld 255 <height>``.");
			return;
		}

		if (requested_y < 30 || requested_y > 255) {
			VarList::OnConsoleMessage(peer, "`4Height must be between `w30 `4and `w255``.");
			return;
		}

		std::string world_name = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [world_name](const World& a) { return a.name == world_name; });
		if (it == worlds.end()) {
			VarList::OnConsoleMessage(peer, "`4Error: World not found.");
			return;
		}

		World* world_ = &(*it);
		int old_x = world_->max_x > 0 ? world_->max_x : 255;
		if (old_x <= 0 || old_x > 255) old_x = 255;
		int old_y = world_->max_y > 0 ? world_->max_y : static_cast<int>(world_->blocks.size()) / 255;
		if (old_y <= 0 && !world_->blocks.empty()) old_y = static_cast<int>(world_->blocks.size()) / 255;
		if (old_y <= 0) {
			VarList::OnConsoleMessage(peer, "`4Error: Invalid world data size.");
			return;
		}

		if (old_x == requested_x && old_y == requested_y) {
			VarList::OnConsoleMessage(peer, "`oWorld size is already `w" + std::to_string(requested_x) + "x" + std::to_string(requested_y) + "``.");
			return;
		}

		World generated{};
		create_world(&generated, requested_x, requested_y);
		std::vector<WorldBlock> resized_blocks = generated.blocks;

		const int copy_x = std::min(old_x, requested_x);
		const int copy_y = std::min(old_y, requested_y);
		for (int y = 0; y < copy_y; ++y) {
			for (int x = 0; x < copy_x; ++x) {
				const int old_index = x + (y * old_x);
				const int new_index = x + (y * requested_x);
				if (old_index < 0 || old_index >= static_cast<int>(world_->blocks.size())) continue;
				if (new_index < 0 || new_index >= static_cast<int>(resized_blocks.size())) continue;
				resized_blocks[new_index] = world_->blocks[old_index];
			}
		}

		auto in_bounds = [&](int x, int y) {
			return x >= 0 && x < requested_x && y >= 0 && y < requested_y;
			};



		world_->blocks.swap(resized_blocks);
		world_->max_x = requested_x;
		world_->max_y = requested_y;
		world_->fresh_world = true;

		world_->drop_new.erase(std::remove_if(world_->drop_new.begin(), world_->drop_new.end(), [&](const std::vector<int>& drop) {
			if (drop.size() < 5) return true;
			const int drop_x = drop[3] / 32;
			const int drop_y = drop[4] / 32;
			return !in_bounds(drop_x, drop_y);
			}), world_->drop_new.end());
		for (int i = 0; i < static_cast<int>(world_->drop_new.size()); ++i) {
			if (world_->drop_new[i].size() > 2) world_->drop_new[i][2] = i;
		}
		world_->total_drop_uid = static_cast<int>(world_->drop_new.size());

		world_->machines.erase(std::remove_if(world_->machines.begin(), world_->machines.end(), [&](const WorldMachines& data) {
			return !in_bounds(data.x, data.y);
			}), world_->machines.end());
		world_->npc.erase(std::remove_if(world_->npc.begin(), world_->npc.end(), [&](const WorldNPC& data) {
			return !in_bounds(data.x, data.y);
			}), world_->npc.end());
		world_->cctv.erase(std::remove_if(world_->cctv.begin(), world_->cctv.end(), [&](const WorldCCTV& data) {
			return !in_bounds(data.x, data.y);
			}), world_->cctv.end());
		world_->sbox1.erase(std::remove_if(world_->sbox1.begin(), world_->sbox1.end(), [&](const WorldSBOX1& data) {
			return !in_bounds(data.x, data.y);
			}), world_->sbox1.end());
		world_->bulletin.erase(std::remove_if(world_->bulletin.begin(), world_->bulletin.end(), [&](const WorldBulletin& data) {
			return !in_bounds(data.x, data.y);
			}), world_->bulletin.end());
		world_->active_jammers.clear();

		if (!in_bounds(world_->s4tb.first, world_->s4tb.second)) {
			world_->s4tb = { 0, 0 };
		}

		save_world(world_->name, false);
		ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, "`8resized world``: `#" + world_->name + " `oto `w" + std::to_string(requested_x) + "x" + std::to_string(requested_y) + "``", "");
		VarList::OnConsoleMessage(peer, "`oWorld resized to `w" + std::to_string(requested_x) + "x" + std::to_string(requested_y) + "``. Reloading players in this world...");

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			if (pInfo(cp_)->world != world_->name) continue;

			pInfo(cp_)->last_world = world_->name;
			Exit_World(cp_, true);
			Enter_World(cp_, pInfo(cp_)->last_world);
		}
	}

	// Definisi variabel lokal pengganti `top_donation` lama agar tidak kena limit


	// Helper: Penambahan angka super besar menggunakan String (BigInt)
	static std::string AddBigIntStrings(std::string num1, std::string num2) {
		std::string res = "";
		int i = num1.length() - 1, j = num2.length() - 1, carry = 0;
		while (i >= 0 || j >= 0 || carry) {
			int sum = carry;
			if (i >= 0) sum += num1[i--] - '0';
			if (j >= 0) sum += num2[j--] - '0';
			res += std::to_string(sum % 10);
			carry = sum / 10;
		}
		std::reverse(res.begin(), res.end());
		res.erase(0, res.find_first_not_of('0'));
		return res.empty() ? "0" : res;
	}

	// Helper: Komparasi angka BigInt untuk Sorting Descending
	static bool CompareBigInt(const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) {
		if (a.first.length() != b.first.length())
			return a.first.length() > b.first.length();
		return a.first > b.first;
	}

	// Helper: Format WL into a lock list (Ex: 200 APSL, 120 MSL, 5 DL)
	static std::string FormatLocks(std::string amountStr) {
		if (amountStr.empty() || amountStr == "0") return "0 WL";
		amountStr.erase(0, amountStr.find_first_not_of('0')); // Bersihkan nol di depan
		if (amountStr.empty()) return "0 WL";

		std::vector<std::string> names = { "WL", "DL", "bgl", "mgl", "mgl", "KGL", "EGL", "MSL", "APSL" };
		std::vector<std::string> parts;
		int nameIdx = 0;

		// Pisahkan string tiap 2 digit (karena tiap lock kelipatan x100 alias nambah 2 nol)
		while (!amountStr.empty() && nameIdx < names.size()) {
			std::string chunk;
			if (nameIdx == names.size() - 1) { // Jika sudah mencapai APSL, sisa angka masuk ke sini semua
				chunk = amountStr;
				amountStr = "";
			}
			else {
				if (amountStr.length() > 2) {
					chunk = amountStr.substr(amountStr.length() - 2);
					amountStr.erase(amountStr.length() - 2);
				}
				else {
					chunk = amountStr;
					amountStr = "";
				}
			}

			chunk.erase(0, chunk.find_first_not_of('0')); // Bersihkan nol dari potongan
			if (!chunk.empty()) {
				parts.insert(parts.begin(), chunk + " " + names[nameIdx]);
			}
			nameIdx++;
		}

		if (parts.empty()) return "0 WL";
		std::string result = "";
		for (size_t i = 0; i < parts.size(); ++i) {
			result += parts[i];
			if (i < parts.size() - 1) result += ", ";
		}
		return result;
	}

	static void SaveDonationData() {
		nlohmann::json js;
		js["donation_event_active"] = donation_event_active;

		nlohmann::json donators = nlohmann::json::array();
		for (const auto& entry : top_donation_str) {
			nlohmann::json d;
			d["amount"] = entry.first; // Simpan sbg string agar unlimited
			d["name"] = entry.second;
			donators.push_back(d);
		}
		js["top_donation"] = donators;

		std::filesystem::create_directories("database/json");
		std::ofstream fo("database/json/donation_server.json");
		if (fo.is_open()) { fo << js.dump(4); fo.close(); }
	}

	static void LoadDonationData() {
		std::ifstream fi("database/json/donation_server.json");
		if (!fi.is_open()) return;
		nlohmann::json js;
		try { fi >> js; }
		catch (...) { return; }
		fi.close();

		if (js.contains("donation_event_active"))
			donation_event_active = js["donation_event_active"].get<bool>();

		if (js.contains("top_donation")) {
			top_donation_str.clear();
			for (const auto& d : js["top_donation"]) {
				std::string amt_str = "0";
				if (d["amount"].is_number()) {
					// Auto fix legacy data (negative values reset to 0, positives converted to unlimited string format)
					long long val = d["amount"].get<long long int>();
					if (val > 0) amt_str = std::to_string(val);
				}
				else if (d["amount"].is_string()) {
					amt_str = d["amount"].get<std::string>();
				}
				if (amt_str != "0") {
					top_donation_str.emplace_back(amt_str, d["name"].get<std::string>());
				}
			}
		}
		Build_Donation_Leaderboard();
	}

	static void Build_Donation_Leaderboard() {
		top_donation_list.clear();
		std::vector<std::pair<std::string, std::string>> sorted = top_donation_str;
		std::sort(sorted.begin(), sorted.end(), CompareBigInt);
		if (sorted.size() > 10) sorted.resize(10);

		for (size_t i = 0; i < sorted.size(); i++) {
			std::string reward_txt = "";
			if (i == 0) reward_txt = "\nadd_smalltext|`3Reward: `wStaff + Streamers + Cheater + VIP Role|left|";
			else if (i == 1) reward_txt = "\nadd_smalltext|`3Reward: `wStreamers + Cheater + VIP Role|left|";
			else if (i == 2) reward_txt = "\nadd_smalltext|`3Reward: `wCheater + VIP Role|left|";
			top_donation_list += "\nadd_label_with_icon|small|`o#" + std::to_string(i + 1) + ". `w" + sorted[i].second + "|left|1796|" + reward_txt + "\nadd_smalltext|`3Donated: `o" + FormatLocks(sorted[i].first) + "|left|";
		}
		if (top_donation_list.empty()) top_donation_list = "\nadd_smalltext|`oNo donations yet. Be the first!|left|";
	}

	static void ShowDonationEventDialog(ENetPeer* peer) {
		Build_Donation_Leaderboard();
		std::string status = donation_event_active ? "`2[ACTIVE]``" : "`4[INACTIVE]``";
		std::string d = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`$Donation Server Event``|left|1796|\n"
			"add_spacer|small|\n"
			"add_textbox|`oEvent Status: " + status + "|left|\n"
			"add_smalltext|`oUsage: `w/donate <type> <amount>`o  "
			"Types: `wwl`o, `wdl`o, `wbgl`o, `wggl`o, `wigl`o, `wsgl`o, `wggl`o, `wkgl`o, `wegl`o, `wmsl`o, `wapsl`o|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`wTop Donators (max Top 10):|left|\n"
			"add_spacer|small|\n"
			+ top_donation_list +
			"\nadd_spacer|small|\n"
			"add_textbox|`oRewards: `wTop 1: Staff+Streamers+Cheater+VIP`` `wTop 2: Streamers+Cheater+VIP`` `wTop 3: Cheater+VIP|left|\n"
			"add_spacer|small|\n"
			"add_button|donation_refresh|`wRefresh|noflags|0|0|\n"
			"end_dialog|DonationServer|Close||\n"
			"add_quick_exit|";
		VarList::OnDialogRequest(peer, d);
	}

	static void GiveDonationReward(const std::string& name, int rank) {
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) != to_lower(name)) continue;
			if (rank == 1) {
				pInfo(cp_)->Role.Staff = true;
				pInfo(cp_)->Role.Streamers = true;
				pInfo(cp_)->Role.Cheats = true;
				pInfo(cp_)->Role.Vip = true;
				VarList::OnConsoleMessage(cp_, "`2[Donation Event] Congratulations! You ranked #1! Staff + Streamers + Cheater + VIP role has been granted!");
				VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`2[Donation Event] #1 Reward: Staff+Streamers+Cheater+VIP!", 0, 1);
			}
			else if (rank == 2) {
				pInfo(cp_)->Role.Streamers = true;
				pInfo(cp_)->Role.Cheats = true;
				pInfo(cp_)->Role.Vip = true;
				VarList::OnConsoleMessage(cp_, "`2[Donation Event] Congratulations! You ranked #2! Streamers + Cheater + VIP role has been granted!");
				VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`2[Donation Event] #2 Reward: Streamers+Cheater+VIP!", 0, 1);
			}
			else if (rank == 3) {
				pInfo(cp_)->Role.Cheats = true;
				pInfo(cp_)->Role.Vip = true;
				VarList::OnConsoleMessage(cp_, "`2[Donation Event] Congratulations! You ranked #3! Cheater + VIP role has been granted!");
				VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`2[Donation Event] #3 Reward: Cheater+VIP!", 0, 1);
			}
			return; // online — done
		}
		// Offline: save reward to JSON
		std::string path = "database/players/" + to_lower(name) + ".json";
		std::ifstream fi(path);
		if (!fi.is_open()) return;
		nlohmann::json js;
		fi >> js; fi.close();
		if (rank == 1) { js["Role.Staff"] = true; js["Role.Streamers"] = true; js["Role.Cheats"] = true; js["Role.Vip"] = true; }
		else if (rank == 2) { js["Role.Streamers"] = true; js["Role.Cheats"] = true; js["Role.Vip"] = true; }
		else if (rank == 3) { js["Role.Cheats"] = true; js["Role.Vip"] = true; }
		std::ofstream fo(path);
		fo << js.dump(2); fo.close();
	}

	static void StartDonationEvent(ENetPeer* peer) {
		if (donation_event_active) {
			VarList::OnConsoleMessage(peer, "`4Donation Event is already active!");
			return;
		}
		donation_event_active = true;
		top_donation_str.clear();
		top_donation_list.clear();

		// Clear user local counters jika ada
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			pInfo(cp_)->donated_wl = 0;
		}
		SaveDonationData();

		VarList::OnConsoleMessage(peer, "`2Donation Event has started!");
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			VarList::OnConsoleMessage(cp_, "`$[DONATION EVENT] `wDonation Server Event has started! Donate your locks via the `$Donation Server`` button in the Event Menu to win exciting prizes!");
			VarList::OnAddNotification(cp_, "`$Donation Server Event Started! `wClick the Donation Server button in the Event Menu!", "interface/atomic_button.rttex", "audio/hub_open.wav");
		}
	}

	static void StopDonationEvent(ENetPeer* peer) {
		if (!donation_event_active) {
			VarList::OnConsoleMessage(peer, "`4Donation Event is not active!");
			return;
		}
		donation_event_active = false;
		std::vector<std::pair<std::string, std::string>> sorted = top_donation_str;
		std::sort(sorted.begin(), sorted.end(), CompareBigInt);
		if (sorted.size() > 10) sorted.resize(10);

		for (int i = 0; i < (int)sorted.size() && i < 3; i++)
			GiveDonationReward(sorted[i].second, i + 1);

		SaveDonationData();
		VarList::OnConsoleMessage(peer, "`2Donation Event has ended! Rewards for Top 1-3 have been sent.");
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			VarList::OnConsoleMessage(cp_, "`4[DONATION EVENT] `wDonation Server Event has ended! Thank you to everyone who donated!");
			if (sorted.size() >= 1) VarList::OnConsoleMessage(cp_, "`$#1 Donator: `w" + sorted[0].second + " `o(" + FormatLocks(sorted[0].first) + ")");
			if (sorted.size() >= 2) VarList::OnConsoleMessage(cp_, "`5#2 Donator: `w" + sorted[1].second + " `o(" + FormatLocks(sorted[1].first) + ")");
			if (sorted.size() >= 3) VarList::OnConsoleMessage(cp_, "`3#3 Donator: `w" + sorted[2].second + " `o(" + FormatLocks(sorted[2].first) + ")");
		}
	}

	static void DonateWL(ENetPeer* peer, const std::string& param) {
		if (!donation_event_active) {
			VarList::OnConsoleMessage(peer, "`4Donation Event is not active!");
			return;
		}
		std::istringstream ss(param);
		std::string type_str, amount_str;
		ss >> type_str >> amount_str;
		if (type_str.empty() || amount_str.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: `w/donate <type> <amount>");
			VarList::OnConsoleMessage(peer, "`oType: `wwl`o, `wdl`o, `wbgl`o, `wggl`o, `wigl`o, `wsgl`o, `wggl`o, `wkgl`o, `wegl`o, `wmsl`o, `wapsl");
			return;
		}
		std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::tolower);

		int item_id = 0;
		std::string type_name, zeros_to_add = "";

		// Kalkulasi pakai nol untuk mencegah OVERFLOW limit data 100%
		if (type_str == "wl") { item_id = 242;   zeros_to_add = "";                     type_name = "World Lock"; }
		else if (type_str == "dl") { item_id = 1796;  zeros_to_add = "00";                   type_name = "Diamond Lock"; }
		else if (type_str == "bgl") { item_id = 7188;  zeros_to_add = "0000";                 type_name = "Silver Gem Lock"; }
		else if (type_str == "mgl") { item_id = 8470;  zeros_to_add = "000000";               type_name = "mgl"; }

		else {
			VarList::OnConsoleMessage(peer, "`4Invalid type! Use: `wwl`4, `wdl`4, `wbgl`4, `wggl`4, `wigl`4, `wsgl`4, `wggl`4, `wkgl`4, `wegl`4, `wmsl`4, `wapsl");
			return;
		}

		int amount = 0;
		try { amount = std::stoi(amount_str); }
		catch (...) { VarList::OnConsoleMessage(peer, "`4Invalid amount!"); return; }
		if (amount <= 0) { VarList::OnConsoleMessage(peer, "`4Amount must be greater than 0!"); return; }
		if (amount > 200) { VarList::OnConsoleMessage(peer, "`4Maximum 200 per transaction!"); return; }

		int has = Inventory::Contains(peer, item_id);
		if (has < amount) { VarList::OnConsoleMessage(peer, "`4Not enough `w" + type_name + "`4! You have `w" + std::to_string(has) + "`4."); return; }

		int minus_amount = -amount;
		if (Inventory::Modify(peer, item_id, minus_amount, false, true) != 0) {
			VarList::OnConsoleMessage(peer, "`4Failed to take `w" + type_name + "`4 from inventory!");
			return;
		}

		Inventory::Visuals(peer);

		std::string wl_score_str = std::to_string(amount) + zeros_to_add;
		std::string player_total = wl_score_str;

		auto it = std::find_if(top_donation_str.begin(), top_donation_str.end(), [&](const std::pair<std::string, std::string>& e) {
			return to_lower(e.second) == to_lower(pInfo(peer)->tankIDName);
			});

		if (it != top_donation_str.end()) {
			it->first = AddBigIntStrings(it->first, wl_score_str);
			player_total = it->first;
		}
		else {
			top_donation_str.emplace_back(wl_score_str, pInfo(peer)->tankIDName);
		}

		Build_Donation_Leaderboard();
		SaveDonationData();

		VarList::OnConsoleMessage(peer, "`2Successfully donated `w" + std::to_string(amount) + " " + type_name + "`2! Total donated: `w" + FormatLocks(player_total) + "`2.");
		VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`2[Donation] " + std::to_string(amount) + " " + type_name + "!", 0, 1);
	}

	static void BANWORLD(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });

		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (world_->worldLocked && !Role::Owner(peer)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4This world is locked by the server.", 0, 1);
				return;
			}

			if (world_->nuked_by.empty()) {
				struct tm newtime;
				time_t now = time(0);
#ifdef WIN32
				localtime_s(&newtime, &now);
#elif defined(linux)
				localtime_r(&now, &newtime);
#endif
				world_->nuked_by = pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``, " +
					std::to_string(newtime.tm_mon + 1) + "/" +
					std::to_string(newtime.tm_mday) + "/" +
					std::to_string(1900 + newtime.tm_year) + " " +
					std::to_string(newtime.tm_hour) + ":" +
					(newtime.tm_min < 10 ? "0" + std::to_string(newtime.tm_min) : std::to_string(newtime.tm_min)) +
					":" + std::to_string(newtime.tm_sec);

				VarList::OnTextOverlay(peer, "`4The following world has been nuked: `o" + pInfo(peer)->world + "`!");

				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;

					if (pInfo(cp_)->world == world_->name) {
						VarList::OnConsoleMessage(cp_, "`4" + world_->name + "` was nuked! `oPlay nice, everybody!`");
						VarList::OnAddNotification(cp_, "wWarning from `4System``: This world has been nuked.",
							"interface/atomic_button.rttex", "audio/hub_open.wav");
						Exit_World(cp_);
					}
					CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/bigboom.wav", 0);
				}

				for (int i = 0; i < world_->blocks.size(); ++i) {
					if (world_->blocks[i].fg == 202 || world_->blocks[i].fg == 204 || world_->blocks[i].fg == 206 ||
						world_->blocks[i].fg == 4994 || world_->blocks[i].fg == 10000 || world_->blocks[i].fg == 5814)
						continue;

					if (items[world_->blocks[i].fg].blockType == BlockTypes::LOCK) {
						world_->last_lock = world_->blocks[i].fg;
						world_->blocks[i].fg = 2950;
						update_tile(peer, i % 100, i / 100, 2950, false, true);
					}
				}

				ServerPool::Logs::Add(pInfo(peer)->name_color + pInfo(peer)->tankIDName + " (`/banworld`) NUKED WORLD: #" + world_->name, "NUKED WORLD");
				ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, "NUKED WORLD: #" + world_->name, "");

			}
			else {
				for (int i = 0; i < world_->blocks.size(); ++i) {
					if (world_->blocks[i].fg == 202 || world_->blocks[i].fg == 204 || world_->blocks[i].fg == 206 ||
						world_->blocks[i].fg == 4994 || world_->blocks[i].fg == 10000 || world_->blocks[i].fg == 5814)
						continue;

					if (items[world_->blocks[i].fg].blockType == BlockTypes::LOCK && world_->blocks[i].fg == 2950) {
						world_->blocks[i].fg = world_->last_lock;
						update_tile(peer, i % 100, i / 100, world_->last_lock, false, true);
					}
				}

				world_->last_lock = 0;
				world_->nuked_by = "";
				VarList::OnConsoleMessage(peer, "`wThis world is once again available to everyone!");
				VarList::OnTextOverlay(peer, "`2You unnuked the world `o" + pInfo(peer)->world + "`..");
				CAction::Positioned(peer, pInfo(peer)->netID, "audio/secret.wav", 0);

				ServerPool::Logs::Add(pInfo(peer)->name_color + pInfo(peer)->tankIDName + " (``/banworld`) UNNUKED WORLD: #" + world_->name, "NUKED WORLD");
				ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, "UNNUKED WORLD: #" + world_->name, "");
			}
		}
	}
	static void HIDE(ENetPeer* peer) {
		pInfo(peer)->m_h = (pInfo(peer)->m_h == 0 ? 1 : 0);
		VarList::OnConsoleMessage(peer, "`o>> You " + a + (pInfo(peer)->m_h ? "hide" : "un-hide") + " yourself from everyone (You `" + (pInfo(peer)->m_h ? "4won't" : "2will") + "`` be visible in /mods, /msg etc.)``");
	}
	static void RADIO2(ENetPeer* peer) {
		pInfo(peer)->radio2 = (pInfo(peer)->radio2 == 0 ? 1 : 0);
		VarList::OnConsoleMessage(peer, "Radio2 " + a + (pInfo(peer)->radio2 ? "disabled, `4you now won't" : "Radio enabled, `2you can now") + a + " see`` public broadcasts.");
	}
	static void TOGGLEMODS(ENetPeer* peer) {
		if (pInfo(peer)->tmod == 1) pInfo(peer)->tmod = 0;
		else pInfo(peer)->tmod = 1;
		VarList::OnConsoleMessage(peer, (pInfo(peer)->tmod ? "`o>> You removed your mod tag.``" : "`o>> You added your mod tag back.``"));
		VisualHandle::Nick(peer, NULL);
	}
	static void SKIN(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSkin Color``|left|5468|\nadd_spacer|small|\nadd_text_input|color|Color - R,G,B:|" + pInfo(peer)->skin_c + "|11|\nadd_spacer|small|\nadd_button|restore_default|Restore to Default|noflags|0|0|\nend_dialog|skin_color|Close|Update|\nadd_quick_exit|");
	}
	static void INVIS(ENetPeer* peer) {
		vector<int> random_{ 32, 64, -32, -64, 0, 0 };
		PlayerMoving data_{};
		pInfo(peer)->t_x = pInfo(peer)->x;
		pInfo(peer)->t_y = pInfo(peer)->y;
		data_.packetType = 17, data_.netID = 105, data_.YSpeed = 105;
		BYTE* raw = packPlayerMoving(&data_);
		CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);

		if (pInfo(peer)->invis) {
			// [1] BECOMING VISIBLE
			pInfo(peer)->invis = false;
			VarList::OnConsoleMessage(peer, "`oYou are once again visible to mortals.");
			gamepacket_t p(750, pInfo(peer)->netID);
			p.Insert("OnInvis");
			p.Insert(0);

			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;

				if (pInfo(cp_)->world == pInfo(peer)->world) {
					// Make player visible
					VarList::OnSetPos(peer, pInfo(peer)->x, pInfo(peer)->y);

					// ENTERED message in talk bubble (visible to all)
					VarList::OnTalkBubble(cp_, pInfo(peer)->netID,
						"`5<`0" + pInfo(peer)->tankIDName + "`` entered, `w" +
						to_string(ServerPool::PlayerCountWorld(pInfo(peer)->world)) +
						" `5others here>``",
						0, 0); // Last 0 = show to all

					// Player count message
					VarList::OnConsoleMessage(cp_,
						"`5<`0" + pInfo(peer)->tankIDName + "`` entered, `w" +
						to_string(ServerPool::PlayerCountWorld(pInfo(peer)->world)) +
						" `5others here>``");

					p.CreatePacket(cp_);
				}
			}
			delete[] raw;
		}
		else {
			// [2] BECOMING INVISIBLE
			pInfo(peer)->invis = true;
			VarList::OnConsoleMessage(peer, "`oYou are now ninja, invisible at all.");
			gamepacket_t p(750, pInfo(peer)->netID);
			p.Insert("OnInvis");
			p.Insert(1);

			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;

				if (pInfo(cp_)->world == pInfo(peer)->world) {
					// Movement packets
					for (int i = 0; i < 6; i++) {
						raw = packPlayerMoving(&data_);
						send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
					}

					// Play vanish sound with delay
					CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/magic.wav", 0);

					// Enhanced particle effects
					for (int i = 0; i < 14; i++) {
						if (rand() % 100 <= 75) VarList::OnParticleEffect(cp_, pInfo(peer)->x - 15 * (rand() % 6), pInfo(peer)->y - 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
						if (rand() % 100 <= 75) VarList::OnParticleEffect(cp_, pInfo(peer)->x + 15 * (rand() % 6), pInfo(peer)->y - 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
						if (rand() % 100 <= 75) VarList::OnParticleEffect(cp_, pInfo(peer)->x + 15 * (rand() % 6), pInfo(peer)->y + 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
						if (rand() % 100 <= 75) VarList::OnParticleEffect(cp_, pInfo(peer)->x - 15 * (rand() % 6), pInfo(peer)->y + 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
						if (rand() % 100 <= 25) VarList::OnParticleEffect(cp_, pInfo(peer)->x - 15 * (rand() % 6), pInfo(peer)->y - 15 * (rand() % 6), rand() % 16, 3, i * 300);
					}
					// LEFT message in talk bubble (visible to all)
					VarList::OnTalkBubble(cp_, pInfo(peer)->netID,
						"`5<`0" + pInfo(peer)->tankIDName + "`` left, `w" +
						to_string(ServerPool::PlayerCountWorld(pInfo(peer)->world)) +
						" `5others here>``",
						0, 0); // Last 0 = show to all

					// Player count message
					VarList::OnConsoleMessage(cp_,
						"`5<`0" + pInfo(peer)->tankIDName + "`` left, `w" +
						to_string(ServerPool::PlayerCountWorld(pInfo(peer)->world)) +
						" `5others here>``");

					p.CreatePacket(cp_);
				}
			}
			delete[] raw;
		}

		// Update clothing
		Clothing_V2::Update_Value(peer);
		Clothing_V2::Update(peer);
	}
	static void CUSTOM_DROP(ENetPeer* peer, std::string cmd) {
		static std::vector<std::pair<int, int>> currentDrops;

		if (cmd.empty()) {
			// Execute drops if configured
			if (currentDrops.empty()) {
				VarList::OnConsoleMessage(peer,
					"`oFirst set drops with: /customdrop id1 count1, id2 count2,...\n"
					"`oExample: /customdrop 8 10, 6 5\n"
					"`oThen use /customdrop to execute");
				return;
			}

			// Perform drops
			std::string worldName = pInfo(peer)->world;
			auto it = std::find_if(worlds.begin(), worlds.end(),
				[&worldName](const World& w) { return w.name == worldName; });

			if (it == worlds.end()) return;

			World* world = &(*it);
			std::string dropMsg = "`2Dropped: ";

			for (const auto& item : currentDrops) {
				int remaining = item.second;
				dropMsg += std::to_string(item.first) + " (" + std::to_string(item.second) + "), ";

				while (remaining > 0) {
					int dropCount = (remaining > 200) ? 200 : remaining;
					WorldDrop drop;
					drop.x = (pInfo(peer)->state == 16 ? pInfo(peer)->x - ((rand() % 12) + 18)
						: pInfo(peer)->x + ((rand() % 12) + 22));
					drop.y = pInfo(peer)->y + rand() % 16;
					drop.id = item.first;
					drop.count = dropCount;
					VisualHandle::Drop(world, drop);
					remaining -= dropCount;
				}
			}
			dropMsg = dropMsg.substr(0, dropMsg.size() - 2);
			VarList::OnConsoleMessage(peer, dropMsg);
			return;
		}

		// Parse new drop configuration
		currentDrops.clear();
		std::istringstream ss(cmd);
		std::string token;

		while (std::getline(ss, token, ',')) {
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);

			std::istringstream pairStream(token);
			int itemId, quantity;

			if (pairStream >> itemId >> quantity) {
				// Validate item
				if (itemId < 1 || itemId > items.size()) continue;
				if (quantity < 1 || quantity > 200) continue;

				const auto& item = items[itemId];
				if (item.name.find("null_item") != std::string::npos ||
					item.name.find("null") != std::string::npos ||
					item.name.find("Guild Entrance") != std::string::npos ||
					item.name.find("Guild Key") != std::string::npos ||
					item.name.find("World Key") != std::string::npos ||
					itemId == 5640 || itemId == 9158 || itemId == 5814 || itemId == 5816) continue;

				currentDrops.emplace_back(itemId, quantity);
			}
		}

		// Show configuration
		std::string configMsg = "`2Configured drops: ";
		for (const auto& item : currentDrops) {
			configMsg += std::to_string(item.first) + " (" + std::to_string(item.second) + "), ";
		}
		configMsg = configMsg.substr(0, configMsg.size() - 2);
		VarList::OnConsoleMessage(peer, configMsg);
	}
	static void MAGIC(ENetPeer* peer) {
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (pInfo(cp_)->world == pInfo(peer)->world) {
				CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/magic.wav", 0);
				for (int i = 0; i < 14; i++) {
					if (rand() % 100 <= 75) VarList::OnParticleEffect(cp_, pInfo(peer)->x - 15 * (rand() % 6), pInfo(peer)->y - 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
					if (rand() % 100 <= 75) VarList::OnParticleEffect(cp_, pInfo(peer)->x + 15 * (rand() % 6), pInfo(peer)->y - 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
					if (rand() % 100 <= 75) VarList::OnParticleEffect(cp_, pInfo(peer)->x + 15 * (rand() % 6), pInfo(peer)->y + 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
					if (rand() % 100 <= 75) VarList::OnParticleEffect(cp_, pInfo(peer)->x - 15 * (rand() % 6), pInfo(peer)->y + 15 * (rand() % 6), rand() % 6 + 1, 2, i * 300);
					if (rand() % 100 <= 25) VarList::OnParticleEffect(cp_, pInfo(peer)->x - 15 * (rand() % 6), pInfo(peer)->y - 15 * (rand() % 6), rand() % 16, 3, i * 300);
					if (rand() % 100 <= 25) VarList::OnParticleEffect(cp_, pInfo(peer)->x + 15 * (rand() % 6), pInfo(peer)->y - 15 * (rand() % 6), rand() % 16, 3, i * 300);
					if (rand() % 100 <= 25) VarList::OnParticleEffect(cp_, pInfo(peer)->x + 15 * (rand() % 6), pInfo(peer)->y + 15 * (rand() % 6), rand() % 16, 3, i * 300);
					if (rand() % 100 <= 25) VarList::OnParticleEffect(cp_, pInfo(peer)->x - 15 * (rand() % 6), pInfo(peer)->y + 15 * (rand() % 6), rand() % 16, 3, i * 300);
				}
			}
		}
	}
	static void NOHANDS(ENetPeer* peer) {
		pInfo(peer)->no_hands = (pInfo(peer)->no_hands ? false : true);
		Clothing_V2::Update_Value(peer);
		Clothing_V2::Update(peer);
	}
	static void NOBODY(ENetPeer* peer) {
		pInfo(peer)->no_body = (pInfo(peer)->no_body ? false : true);
		Clothing_V2::Update_Value(peer);
		Clothing_V2::Update(peer);
	}
	static void UNEQUIP(ENetPeer* peer) {
		if (pInfo(peer)->hair != 0) Clothing::UnEquip(peer, pInfo(peer)->hair);
		if (pInfo(peer)->shirt != 0) Clothing::UnEquip(peer, pInfo(peer)->shirt);
		if (pInfo(peer)->pants != 0) Clothing::UnEquip(peer, pInfo(peer)->pants);
		if (pInfo(peer)->feet != 0) Clothing::UnEquip(peer, pInfo(peer)->feet);
		if (pInfo(peer)->face != 0) Clothing::UnEquip(peer, pInfo(peer)->face);
		if (pInfo(peer)->hand != 0) Clothing::UnEquip(peer, pInfo(peer)->hand);
		if (pInfo(peer)->back != 0) Clothing::UnEquip(peer, pInfo(peer)->back);
		if (pInfo(peer)->mask != 0) Clothing::UnEquip(peer, pInfo(peer)->mask);
		if (pInfo(peer)->necklace != 0) Clothing::UnEquip(peer, pInfo(peer)->necklace);
		if (pInfo(peer)->ances != 0) Clothing::UnEquip(peer, pInfo(peer)->ances);
	}
	static void DROPALL(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wDrop All items``|left|1432|\nadd_textbox|`0Are you sure want to drop all your items?|left|\nend_dialog|dropall|Nevermind|Yes,iam aure|\n");
	}
	static void NO_CLIP(ENetPeer* peer) {
		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
		if (it != worlds.end()) {
			World* world_ = &(*it);
			if (to_lower(world_->owner_name) != to_lower(pInfo(peer)->tankIDName)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "can only be used in your owned worlds!", 0, 0);
				return;
			}
			CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
			pInfo(peer)->no_clip = (pInfo(peer)->no_clip ? false : true);
			pInfo(peer)->ghost = (pInfo(peer)->ghost ? false : true);
			if (pInfo(peer)->ghost == false) VarList::OnConsoleMessage(peer, "`oYour body stops shimmering and returns to normal. (`$Ghost in the Shell `omod removed)``");
			else VarList::OnConsoleMessage(peer, "`oYour atoms are suddenly aware of quantum tunneling. (`$Ghost in the Shell `omod added)``");
			VisualHandle::State(pInfo(peer));
			Clothing_V2::Update(peer);
		}
	}
	static void SUMMON(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /summon <player name>");
			return;
		}
		if (Playmods::HasById(pInfo(peer), 139)) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, you can't do that while cursed.", 0, 0);
			return;
		}
		bool found_ = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->invis or pInfo(cp_)->hider or pInfo(cp_)->seeker) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				if (Playmods::HasById(pInfo(cp_), 139)) {
					found_ = true;
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, this person is in HELL right now.", 0, 0);
				}
				else if (not Role::Administrator(peer) and Role::Administrator(cp_)) {
					found_ = true;
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, you cannot summon Players with higher role than you.", 0, 0);
				}
				else if (pInfo(cp_)->d_s) {
					found_ = true;
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players summon status is `4DISABLE`o.", 0, 0);
				}
				else if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->tankIDName)) {
					found_ = true;
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, You can't summon yourself!", 0, 0);
				}
				else {
					pInfo(cp_)->update = true;
					found_ = true;
					VarList::OnTextOverlay(peer, "Summoning...");
					VarList::OnTextOverlay(cp_, "You were summoned by a mod");
					Enter_World(cp_, pInfo(peer)->world, pInfo(peer)->x / 32, pInfo(peer)->y / 32);
				}
				break;
			}
		}
		if (found_ == false) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, player was not found.", 0, 0);
		}
	}
	static void FREEZE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /freeze <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false or pInfo(cp_)->world != pInfo(peer)->world) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				if (Playmods::HasById(pInfo(cp_), 2) or Playmods::HasById(pInfo(cp_), 3) or Playmods::HasById(pInfo(cp_), 49)) VarList::OnConsoleMessage(peer, "`o>> Player is frozen already.``");
				else {
					VarList::OnConsoleMessage(peer, "`o>> You got `3frozen`` by player: `2" + pInfo(peer)->tankIDName + ".``");
					Playmods::Add(cp_, 2);
					VisualHandle::State(pInfo(cp_));
					Clothing_V2::Update(cp_, true);
				}
				break;
			}
		}
	}
	static void PUNCH_ID(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /p <id>");
			return;
		}
		int punch = std::atoi(cmd.c_str());
		if (punch >= 0 && punch < 300) {
			pInfo(peer)->punched = punch;
			VarList::OnConsoleMessage(peer, "`2Set punch effect to ID``: " + to_string(pInfo(peer)->punched) + " (set to `20`` to reset).");
		}
		else VarList::OnConsoleMessage(peer, "`2This punch effect does not exist only (0-300).");
		Clothing_V2::Update_Value(peer);
		Clothing_V2::Update(peer, true);
	}
	// /lockworld — Kunci world agar tidak bisa di-break, place, nuke, clearworld, banworld
	// oleh siapapun kecuali Owner Server.
	static void LOCKWORLD(ENetPeer* peer) {
		if (!Role::Owner(peer) && !Role::Developer(peer) && !Role::Administrator(peer)) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Only Owner/Admin can use /lockworld.", 0, 1);
			return;
		}
		std::string name_ = pInfo(peer)->world;
		if (name_.empty()) return;
		auto it = std::find_if(worlds.begin(), worlds.end(), [&](const World& a) { return a.name == name_; });
		if (it == worlds.end()) return;
		World* world_ = &(*it);
		world_->worldLocked = !world_->worldLocked;
		save_world(world_->name, false);
		if (world_->worldLocked) {
			VarList::OnConsoleMessage(peer, "`2World `" + world_->name + "`2 is now `4LOCKED`2. No one can break, place, nuke, clear, or ban this world.");
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == world_->name)
					VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`4This world is locked by the server.", 0, 1);
			}
		}
		else {
			VarList::OnConsoleMessage(peer, "`2World `" + world_->name + "`2 is now `aUNLOCKED`2.");
		}
	}
	static void NUKE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /nuke <time> <reason>");
			return;
		}
		std::istringstream ss(cmd);
		std::string Reason;
		int Time;
		try {
			if (ss >> Time >> Reason) {
				std::string name_ = pInfo(peer)->world;
				auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
				if (it != worlds.end()) {
					World* world_ = &(*it);
					if (world_->worldLocked && !Role::Owner(peer)) {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4This world is locked by the server.", 0, 1);
						return;
					}
					if (world_->owner_name == pInfo(peer)->tankIDName) {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You can't ban your own world!", 0, 0);
						return;
					}
					if (world_->nuked_by.empty()) {
						if (Reason == "") {
							VarList::OnTalkBubble(peer, pInfo(peer)->netID, "The reason can't be empty!", 0, 0);
							return;
						}
						if (Time == 0) {
							VarList::OnTalkBubble(peer, pInfo(peer)->netID, "The time can't be zero!", 0, 0);
							return;
						}
						bool has_admin = false;
						std::string cworld_ = pInfo(peer)->world, BannedBy_ = (pInfo(peer)->d_name.empty() ? pInfo(peer)->name_color : "`0") + "" + (pInfo(peer)->d_name.empty() ? pInfo(peer)->tankIDName : pInfo(peer)->d_name) + "``", ban_who = "", admin_who = "";
						struct tm newtime;
						time_t now = time(0);
#ifdef _WIN32
						localtime_s(&newtime, &now);
#elif defined(__linux__)
						localtime_r(&now, &newtime);
#endif
						world_->nuked_by = pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``, " + to_string(newtime.tm_mon + 1) + "/" + to_string(newtime.tm_mday) + "/" + to_string(1900 + newtime.tm_year) + " " + to_string(newtime.tm_hour) + ":" + (newtime.tm_min < 10 ? "0" + to_string(newtime.tm_min) + "" : "" + to_string(newtime.tm_min)) + ":" + to_string(newtime.tm_sec) + "";
						for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
							if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
							VarList::OnConsoleMessage(cp_, "`o>> `4" + world_->name + " `4was nuked from orbit`o. It's the only way to be sure. Play nice, everybody!");
							CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/bigboom.wav", 0);
							if (pInfo(cp_)->world == cworld_) {
								if (pInfo(cp_)->world == cworld_ and pInfo(cp_)->tankIDName != pInfo(peer)->tankIDName and to_lower(pInfo(cp_)->tankIDName) != to_lower(world_->owner_name)) {
									Exit_World(cp_);
								}
								if (to_lower(pInfo(cp_)->tankIDName) == to_lower(world_->owner_name) or pInfo(cp_)->tankIDName == world_->owner_name) {
									ban_who = (Role::Moderator(cp_) && pInfo(cp_)->d_name.empty() ? pInfo(cp_)->name_color : "`0") + "" + (pInfo(cp_)->d_name.empty() ? pInfo(cp_)->tankIDName : pInfo(cp_)->d_name) + "``";
									Punishment::Banned(cp_, Time, Reason, BannedBy_, 76);
								}
								else {
									ban_who = world_->owner_name;
									string path_ = "database/players/" + world_->owner_name + "_.json";
									if (_access_s(path_.c_str(), 0) == 0) {
										json r_;
										std::ifstream f_(path_, ifstream::binary);
										if (f_.fail()) continue;
										f_ >> r_;
										f_.close();
										{
											json f_ = r_["b_t"].get<int>();
											r_["b_s"] = (Time * 1000);
											r_["b_r"] = Reason;
											r_["b_b"] = BannedBy_;
											r_["b_t"] = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
										}
										{
											std::ofstream f_(path_, ifstream::binary);
											f_ << r_;
											f_.close();
										}
									}
								}
								if (find(world_->admins.begin(), world_->admins.end(), pInfo(cp_)->tankIDName) == world_->admins.end() and pInfo(peer)->tankIDName != pInfo(cp_)->tankIDName) {
									Punishment::Banned(cp_, Time, Reason, BannedBy_, 76);
									has_admin = true, admin_who = (Role::Moderator(cp_) && pInfo(cp_)->d_name.empty() ? pInfo(cp_)->name_color : "`0") + "" + (pInfo(cp_)->d_name.empty() ? pInfo(cp_)->tankIDName : pInfo(cp_)->d_name) + "``";
								}
							}
						}
						for (int i_ = 0; i_ < world_->blocks.size(); i_++) {
							if (world_->blocks[i_].fg == 202 || world_->blocks[i_].fg == 204 || world_->blocks[i_].fg == 206 || world_->blocks[i_].fg == 4994 || world_->blocks[i_].fg == 10000 || world_->blocks[i_].fg == 5814) continue;
							if (items[world_->blocks[i_].fg].blockType == BlockTypes::LOCK) {
								world_->last_lock = world_->blocks[i_].fg;
								world_->blocks[i_].fg = 2950, update_tile(peer, i_ % 100, i_ / 100, 2950, false, true);
							}
						}
						string times = "`4Time`` : `#" + ((Time / 86400 > 0) ? to_string(Time / 86400) + " days" : (Time / 3600 > 0) ? to_string(Time / 3600) + " hours" : (Time / 60 > 0) ? to_string(Time / 60) + " minutes" : to_string(Time) + " seconds");
						ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, " > NUKED WORLD: `#" + cworld_, "");
						ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, " > REASON (" + Reason + "): " + ban_who + " " + times, "");
						ServerPool::Logs::Add(pInfo(peer)->name_color + pInfo(peer)->tankIDName + " (/nuke) NUKED WORLD: `#" + pInfo(peer)->world + " REASON (" + Reason + ") TIME (" + times + ")", "NUKED WORLD");
						if (has_admin) ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, " > BANNED (" + Reason + "): " + ban_who + " " + times, "");
						return;
					}
					else {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "This world already nuked! You can unnuke first and ban this world!", 0, 0);
						return;
					}
				}
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /nuke <time> <reason>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /nuke <time> <reason>");
		}
	}
	static void NOTE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /note <player name> <text>");
			return;
		}
		std::istringstream ss(cmd);
		std::string playerName, text;
		try {
			if (ss >> playerName >> text) {
				if (text.size() < 3) {
					VarList::OnConsoleMessage(peer, "`oToo shorts notes.");
					return;
				}
				bool has_found = false;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (to_lower(pInfo(cp_)->tankIDName) == to_lower(playerName)) {
						has_found = true;
						pInfo(peer)->Account_Notes.push_back("`o" + currentDateTime() + ": >> " + text + "");
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Succesfully added note to " + playerName + "", 0, 0);
					}
				}
				if (not has_found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /note <player name> <text>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /note <player name> <text>");
		}
	}
	static void NOTES(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /notes <player name>");
			return;
		}
		bool foundacc = true;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				foundacc = false;
				string notes = "";
				if (not pInfo(cp_)->Account_Notes.empty()) {
					for (int i = 0; i < pInfo(cp_)->Account_Notes.size(); i++) notes += "\nadd_textbox|`o" + pInfo(cp_)->Account_Notes[i] + "|left|\n";
				}
				else notes += "\nadd_textbox|`oThere's no account notes for this user yet.|left|";
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label|big|`w" + pInfo(cp_)->tankIDName + " Notes|left|\nadd_spacer|small|\nadd_textbox|`oAccount notes:|left|" + notes + "|\nadd_spacer|small|\nend_dialog||Continue||");
				break;
			}
		}
		/*offline*/
		if (foundacc) {
			bool found = false;
			try {
				string name = pInfo(peer)->last_wrenched;
				std::ifstream ifs("database/players/" + name + "_.json");
				if (ifs.is_open()) {
					json j;
					ifs >> j;
					found = true;
					string notes = "";
					for (string a : j["Account_Notes"].get<vector<string>>()) notes += "\nadd_textbox|`o" + a + "|left|\n";
					if (notes == "") notes = "\nadd_textbox|`oThere's no account notes for this user yet.|left|";
					VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label|big|`w" + j["name"].get<string>() + " Notes|left|\nadd_spacer|small|\nadd_textbox|`oAccount notes:|left|" + notes + "|\nadd_spacer|small|\nend_dialog||Continue||");
				}
			}
			catch (exception) {
				return;
			}
			if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
		}
	}
	static void MUTE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, ">> Usage: /mute <name> <seconds> <reason>");
			return;
		}
		std::istringstream ss(cmd);
		std::string playerName, reason;
		int seconds_;
		try {
			if (ss >> playerName >> seconds_ >> reason) {
				int remove_playmod_id = 11;
				if (seconds_ <= 0) {
					VarList::OnConsoleMessage(peer, "`o>> The time is too low!``");
					return;
				}
				if (reason.size() < 2) {
					VarList::OnConsoleMessage(peer, "`o>> Too shorts Reason!``");
					return;
				}
				bool found = false;
				pInfo(peer)->last_wrenched = playerName;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
						found = true;
						Punishment::Muted_Cursed(cp_, seconds_, reason, pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``", remove_playmod_id);
						break;
					}
				}
				if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, ">> Usage: /mute <name> <seconds> <reason>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, ">> Usage: /mute <name> <seconds> <reason>");
		}
	}
	static void CURSE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, ">> Usage: /curse <name> <seconds> <reason>");
			return;
		}
		std::istringstream ss(cmd);
		std::string playerName, reason;
		int seconds_;
		try {
			if (ss >> playerName >> seconds_ >> reason) {
				int remove_playmod_id = 139;
				if (seconds_ <= 0) {
					VarList::OnConsoleMessage(peer, "`o>> The time is too low!``");
					return;
				}
				if (reason.size() < 2) {
					VarList::OnConsoleMessage(peer, "`o>> Too shorts Reason!``");
					return;
				}
				bool found = false;
				pInfo(peer)->last_wrenched = playerName;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
						found = true;
						Punishment::Muted_Cursed(cp_, seconds_, reason, pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``", remove_playmod_id);
						break;
					}
				}
				if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, ">> Usage: /curse <name> <seconds> <reason>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, ">> Usage: /curse <name> <seconds> <reason>");
		}
	}
	static void BANKOK(ENetPeer* peer, const std::string& cmd) {
		std::istringstream ss(cmd);
		std::string growid, reason;
		int seconds_;

		if (!(ss >> growid >> seconds_)) {
			VarList::OnConsoleMessage(peer, ">> Usage: /banp <growid> <seconds> <reason>");
			return;
		}

		std::getline(ss >> std::ws, reason);
		if (reason.empty()) reason = "No reason";

		std::string path = "database/players/" + to_lower(growid) + "_.json";
		std::ifstream ifs(path);
		if (!ifs.is_open()) {
			VarList::OnConsoleMessage(peer, ">> Player not found.");
			return;
		}

		json j;
		ifs >> j;
		ifs.close();

		long long expire = duration_cast<seconds>(system_clock::now().time_since_epoch()).count() + seconds_;
		j["banned"] = true;
		j["ban_expire"] = expire;
		j["ban_reason"] = reason;

		std::ofstream ofs(path);
		ofs << j;
		ofs.close();

		VarList::OnConsoleMessage(peer, ">> Banned `" + growid + "` for " + std::to_string(seconds_) + " seconds.\nReason: " + reason);

		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
			if (to_lower(pInfo(cp)->tankIDName) == to_lower(growid)) {
				VarList::OnAddNotification(cp, "`4[Server] You have been banned!\nReason: " + reason, "interface/atomic_button.rttex", "audio/hub_open.wav");
				Peer_Disconnect(cp, 0);
				break;
			}
		}
	}
	static void UNBAN(ENetPeer* peer, const std::string& cmd) {
		std::string growid = to_lower(cmd);
		if (growid.empty()) {
			VarList::OnConsoleMessage(peer, ">> Usage: /unban <growid>");
			return;
		}

		std::string path = "database/players/" + growid + "_.json";
		std::ifstream ifs(path);
		if (!ifs.is_open()) {
			VarList::OnConsoleMessage(peer, ">> Player not found.");
			return;
		}

		json j;
		ifs >> j;
		ifs.close();

		j["banned"] = false;
		j["ban_expire"] = 0;
		j["ban_reason"] = "";

		std::ofstream ofs(path);
		ofs << j;
		ofs.close();

		VarList::OnConsoleMessage(peer, ">> Unbanned `" + growid + "`.");
	}
	static void DeveloperSB(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dsb <text>");
			return;
		}
		if (Playmods::HasById(pInfo(peer), 122)) {
			int time_ = 0;
			for (PlayMods peer_playmod : pInfo(peer)->playmods) {
				if (peer_playmod.id == 122) {
					time_ = peer_playmod.time - time(nullptr);
					break;
				}
			}
			VarList::OnConsoleMessage(peer, ">> (" + Time::Playmod(time_) + " before you can broadcast again)");
			return;
		}
		else {
			CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
			Playmods::Add(peer, 122, 90);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[SB]_ `8[DEV] `5** from (`$" + (get_player_nick(peer)) + "`5) in [`4HIDDEN!`5] ** : `$" + cmd + "``");
			}
		}
	}
	static void SpecialSB(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /ssb <text>");
			return;
		}
		if (Playmods::HasById(pInfo(peer), 122)) {
			int time_ = 0;
			for (PlayMods peer_playmod : pInfo(peer)->playmods) {
				if (peer_playmod.id == 122) {
					time_ = peer_playmod.time - time(nullptr);
					break;
				}
			}
			VarList::OnConsoleMessage(peer, ">> (" + Time::Playmod(time_) + " before you can broadcast again)");
			return;
		}
		else {
			CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
			Playmods::Add(peer, 122, 90);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[SB]_ `o[`1S`2P`3E`4C`5I`6A`7L`o-SB] `5** from (`$" + (get_player_nick(peer)) + "`5) in [`4HIDDEN!`5] ** : `$" + cmd + "``");
			}
		}
	}
	static void CURSE_ALL(ENetPeer* peer, std::string cmd) {
		int seconds_ = 120; // Fixed 2 minutes duration
		std::string reason = "Mass curse"; // Fixed reason
		int remove_playmod_id = 139;
		std::string hellWorld = "HELL"; // Target world name
		std::string notificationTitle = "`4Ryoiki Tenkai"; // Notification title
		std::string notificationIcon = "interface/sukuna_button.rttex"; // Notification icon
		std::string notificationSound = "audio/sukuna_tenkai.wav"; // New sound effect

		bool found = false;

		// First, teleport the command executor (admin) to HELL (without curse)
		if (pInfo(peer)->world != hellWorld) {
			pInfo(peer)->update = true;
			VarList::OnConsoleMessage(peer, "`4Entering the Domain of Hell...");
			Enter_World(peer, hellWorld);
			VarList::OnAddNotification(peer, "`4Curse Initiated!", notificationIcon, notificationSound);
		}

		// Then, curse all other players and teleport them
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
			if (cp_ != peer) { // Skip the executor (already handled above)
				found = true;
				// Curse the player
				Punishment::Muted_Cursed(cp_, seconds_, reason, pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``", remove_playmod_id);

				// Send notification
				VarList::OnAddNotification(cp_, notificationTitle, notificationIcon, notificationSound);

				// Teleport to HELL world
				if (pInfo(cp_)->world != hellWorld) {
					pInfo(cp_)->update = true;
					VarList::OnConsoleMessage(cp_, "You've been cursed and sent to 4HELL`!");
					Enter_World(cp_, hellWorld);
				}
			}
		}

		if (found == false) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "No players to curse!", 0, 0);
		}
		else {
			// Send confirmation to executor
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Cursed all players for 2 minutes and sent to HELL!", 0, 0);
			VarList::OnAddNotification(peer, "Curse ALL Complete!", notificationIcon, notificationSound);
		}
	}
	static void UNMUTE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /unmute <player name>");
			return;
		}
		bool found = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				found = true;
				if (Playmods::HasById(pInfo(cp_), 11, 1)) {
					VarList::OnConsoleMessage(peer, "`o>> <`5" + pInfo(cp_)->tankIDName + "``> was Un" + to_lower(info_about_playmods[11 - 1][3]) + "d!``");
				}
				else VarList::OnConsoleMessage(peer, "`o>> The player didn't had such playmod!``");
				break;
			}
		}
		if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void UNCURSE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /uncurse <player name>");
			return;
		}
		bool found = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				found = true;
				if (Playmods::HasById(pInfo(cp_), 139, 1)) {
					VarList::OnConsoleMessage(peer, "`o>> <`5" + pInfo(cp_)->tankIDName + "``> was Un" + to_lower(info_about_playmods[139 - 1][3]) + "d!``");
				}
				else VarList::OnConsoleMessage(peer, "`o>> The player didn't had such playmod!``");
				break;
			}
		}
		if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void COPY_SET(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /copyset <player name>");
			return;
		}
		bool found = false;
		pInfo(peer)->last_wrenched = cmd;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->growid == false) continue;
			if (to_lower(pInfo(peer)->last_wrenched) == to_lower(pInfo(cp_)->tankIDName)) {
				found = true;
				if (Inventory::Contains(peer, pInfo(cp_)->hair) >= 1) pInfo(peer)->hair = pInfo(cp_)->hair;
				if (Inventory::Contains(peer, pInfo(cp_)->shirt) >= 1) pInfo(peer)->shirt = pInfo(cp_)->shirt;
				if (Inventory::Contains(peer, pInfo(cp_)->pants) >= 1) pInfo(peer)->pants = pInfo(cp_)->pants;
				if (Inventory::Contains(peer, pInfo(cp_)->feet) >= 1) pInfo(peer)->feet = pInfo(cp_)->feet;
				if (Inventory::Contains(peer, pInfo(cp_)->face) >= 1) pInfo(peer)->face = pInfo(cp_)->face;
				if (Inventory::Contains(peer, pInfo(cp_)->hand) >= 1) pInfo(peer)->hand = pInfo(cp_)->hand;
				if (Inventory::Contains(peer, pInfo(cp_)->back) >= 1) pInfo(peer)->back = pInfo(cp_)->back;
				if (Inventory::Contains(peer, pInfo(cp_)->mask) >= 1) pInfo(peer)->mask = pInfo(cp_)->mask;
				if (Inventory::Contains(peer, pInfo(cp_)->necklace) >= 1) pInfo(peer)->necklace = pInfo(cp_)->necklace;
				if (Inventory::Contains(peer, pInfo(cp_)->ances) >= 1) pInfo(peer)->ances = pInfo(cp_)->ances;
				Clothing_V2::Update(peer);
				break;
			}
		}
		if (found == false) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void MOD_SB(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /msb <text>");
			return;
		}
		if (Playmods::HasById(pInfo(peer), 122)) {
			int time_ = 0;
			for (PlayMods peer_playmod : pInfo(peer)->playmods) {
				if (peer_playmod.id == 122) {
					time_ = peer_playmod.time - time(nullptr);
					break;
				}
			}
			VarList::OnConsoleMessage(peer, ">> (" + Time::Playmod(time_) + " before you can broadcast again)");
			return;
		}
		else {
			CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
			Playmods::Add(peer, 122, 90);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[SB]_ `#[MODS-SB] `5** from (`$" + (get_player_nick(peer)) + "`5) in [`4HIDDEN!`5] ** : `$" + cmd + "``");
			}
		}
	}
	static void MOD_CHAT(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /m <text>");
			return;
		}
		// 30 second cooldown
		long long now_mc = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
		if (pInfo(peer)->chat_cmd_time + 30000 > now_mc) {
			long long rem = (pInfo(peer)->chat_cmd_time + 30000 - now_mc) / 1000 + 1;
			VarList::OnConsoleMessage(peer, "`4>> Cooldown! Wait " + to_string(rem) + " seconds before using /m again.");
			return;
		}
		pInfo(peer)->chat_cmd_time = now_mc;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or not Role::Moderator(cp_) or pInfo(cp_)->radio or pInfo(cp_)->radio2) continue;
			VarList::OnConsoleMessage(cp_, "CT:[FC]_>> `o>> [`#MOD-CHAT`o] from (``" + pInfo(peer)->name_color + pInfo(peer)->tankIDName + "```o) in [```$" + pInfo(peer)->world + "```o] > ```$`3" + cmd + "`c");
		}
	}
	static void WARN(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /warn <player name> <text>");
			return;
		}
		std::istringstream ss(cmd);
		std::string playerName, warn_message;
		try {
			if (ss >> playerName >> warn_message) {
				pInfo(peer)->last_wrenched = playerName;
				if (to_lower(pInfo(peer)->last_wrenched) == to_lower(pInfo(peer)->tankIDName)) {
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You can't warn yourself!", 0, 1);
					return;
				}
				if (warn_message == "") {
					VarList::OnTalkBubble(peer, pInfo(peer)->netID, "The warning can't be empty!", 0, 1);
					return;
				}
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
						if (not Role::Administrator(peer) and Role::Administrator(cp_)) {
							VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Hmm, you cannot warn Players with higher role than you.", 0, 0);
						}
						pInfo(cp_)->Warning++; pInfo(cp_)->Warning_Message.push_back(warn_message);
						VarList::OnConsoleMessage(cp_, "Warning from `4System``: " + warn_message);
						VarList::OnAddNotification(cp_, "Warning from `4System``: " + warn_message, "interface/atomic_button.rttex", "audio/hub_open.wav");
						ServerPool::ModLogs::Add(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, "  > `^Warn : " + pInfo(cp_)->tankIDName + "\n`4Warning Message : " + warn_message, "");
						if (pInfo(cp_)->Warning >= 3) {
							int Time = 604800;
							Punishment::Banned(cp_, 6.307e+7, "You has've been reach 3 warnings!", pInfo(cp_)->name_color + pInfo(cp_)->tankIDName + "``", 76);
							ServerPool::ModLogs::Add(cp_, pInfo(cp_)->name_color + pInfo(cp_)->tankIDName, " `^BANNED (has've been reach 3 warnings!)", "");
						}
						return;
					}
				}
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "The warning send (will work when player's online!)", 0, 0);
			}
		}
		catch (const std::invalid_argument&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /warn <player name> <text>");
		}
		catch (const std::out_of_range&) {
			VarList::OnConsoleMessage(peer, "`oUsage: /warn <player name> <text>");
		}
	}
	static void EFFECT_ID(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /eff <id>");
			return;
		}
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (pInfo(cp_)->world == pInfo(peer)->world) {
				CAction::Effect(cp_, std::atoi(cmd.c_str()), pInfo(peer)->x + 16, pInfo(peer)->y + 16);
			}
		}
	}
	static void EFFECTV2_ID(ENetPeer* peer, std::string cmd) {
		if ((cmd.find_first_not_of("0123456789") != string::npos) or cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /eff2 <id>");
			return;
		}
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (pInfo(cp_)->world == pInfo(peer)->world) {
				CAction::Effect_V2(cp_, std::atoi(cmd.c_str()), pInfo(peer)->x + 16, pInfo(peer)->y + 16);
			}
		}
	}
	static void PANEL_BAN(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /summon <player name>");
			return;
		}
		pInfo(peer)->last_wrenched = cmd; bool found = false;
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
			if (to_lower(pInfo(cp_)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
				VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|Ban Panel|left|732|\nadd_textbox|`6ON: `#" + pInfo(cp_)->tankIDName + " `w(" + pInfo(cp_)->requestedName + ") - #" + to_string(pInfo(cp_)->id) + " IP: " + pInfo(cp_)->ip + "|left|\nadd_smalltext|`oKeep in mind! Applying a ban more or equal to 730 days equals as a suspension (permanent ban) for user slowlyrise.<CR>`oChoose the ban amount and write the reason.|\nadd_text_input|Time_Banned|`wTime:||15|\nadd_text_input|Reason_Banned|`4Reason:||30|\nend_dialog|ban_panel|Cancel|Send|");
				found = true;
				break;
			}
		}
		if (not found) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Players not found!", 0, 0);
	}
	static void ULTRASPRAY(ENetPeer* peer) {
		const int GEM_COST = 200000;

		std::string dialog =
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wULTRA SPRAY ``|left|758|\n"
			"add_spacer|small|\n"
			"add_textbox|`5This will reset `2ALL`5 trees, providers, and seeds in your world|left|\n"
			"add_smalltext|`7World: `2" + pInfo(peer)->world + "|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`5Cost: `2200.000" + " Gems`5 (You have: `2" + to_string(pInfo(peer)->gems) + "`5)|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`4WARNING: `oThis action cannot be undone!|left|\n"
			"add_smalltext|`7All growing progress will be reset to zero|left|\n"
			"add_spacer|small|\n"
			"add_button|cancel|`4Cancel``|noflags|0|0|\n"
			"add_button|confirm|`2CONFIRM RESET``|noflags|0|0|\n"
			"end_dialog|ultramen_spray|||\n"  // Kept unchanged as requested
			"add_quick_exit|";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void CollectPlayer(ENetPeer* peer, const std::string& targetName) {
		ENetPeer* targetPeer = nullptr;
		for (int i = 0; i < server->peerCount; i++) {
			ENetPeer* p = &server->peers[i];
			if (p->state != ENET_PEER_STATE_CONNECTED || !p->data) continue;

			if (to_lower(pInfo(p)->tankIDName) == to_lower(targetName)) {
				targetPeer = p;
				break;
			}
		}

		if (!targetPeer) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Player not found!", 0, 0);
			return;
		}

		if (!Role::Developer(peer)) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Clist access required!", 0, 0);
			return;
		}

		float targetX = pInfo(targetPeer)->x;
		float targetY = pInfo(targetPeer)->y;
		string targetWorld = pInfo(targetPeer)->world;

		World* world = nullptr;
		for (auto& w : worlds) {
			if (w.name == targetWorld) {
				world = &w;
				break;
			}
		}

		if (!world) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "World not found!", 0, 0);
			return;
		}

		int collected = 0;
		const int MAX_PER_STACK = 200;

		for (size_t i = 0; i < world->drop_new.size(); ) {
			auto& drop = world->drop_new[i];
			int itemID = drop[0];

			if (itemID != 0 && itemID != 112 && items[itemID].untradeable == 0 && itemID != 4490 && drop[3] > 0 && drop[4] > 0) {
				for (int j = 0; j < 3; j++) {
					VarList::OnParticleEffect(targetPeer, drop[3], drop[4], 137, 3, j * 100);
				}
				CAction::Positioned(targetPeer, pInfo(targetPeer)->netID, "audio/snap.wav", 0);

				int collectAmount = min(drop[1], MAX_PER_STACK);
				Inventory::Modify(targetPeer, itemID, collectAmount);
				collected += collectAmount;
				drop[1] -= collectAmount;

				if (drop[1] <= 0) {
					world->drop_new.erase(world->drop_new.begin() + i);
					continue;
				}
			}
			i++;
		}

		if (collected > 0) {
			gamepacket_t invUpdate;
			invUpdate.Insert("OnRefreshInventory");
			invUpdate.Insert(1); // Flash effect
			invUpdate.CreatePacket(targetPeer);

			exit_(targetPeer, true);
			join_world(targetPeer, targetWorld, targetX / 32, targetY / 32);

			VarList::OnParticleEffect(targetPeer, targetX, targetY, 6, 2, 0);

		}
		else {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "No items to collect!", 0, 0);
		}
	}
	static void GIVDEX(ENetPeer* peer, std::string param) {
		if (param.empty()) {
			VarList::OnConsoleMessage(peer,
				"`oUsage: /givdex <id1> <count1>, <id2> <count2>, ... (max 10 items)\n"
				"`oExample: /givdex 7188 2, 242 1, 1796 5");
			return;
		}

		std::vector<std::pair<int, int>> itemsToGive;
		std::istringstream iss(param);
		std::string token;
		int itemCount = 0;

		// Parse items (max 10)
		while (std::getline(iss, token, ',') && itemCount < 10) {
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);

			std::istringstream itemStream(token);
			int id, count;

			if (!(itemStream >> id >> count)) {
				VarList::OnConsoleMessage(peer, "`4Invalid format for item: `o" + token);
				return;
			}

			if (id <= 0 || count <= 0 || count > 200) {
				VarList::OnConsoleMessage(peer,
					"`4Invalid `ovalues (ID: " + std::to_string(id) +
					", Count: " + std::to_string(count) + ")");
				return;
			}

			itemsToGive.emplace_back(id, count);
			itemCount++;
		}

		if (itemsToGive.empty()) {
			VarList::OnConsoleMessage(peer, "`4No valid items to give");
			return;
		}

		// Give items to all players
		int totalRecipients = 0;
		std::string ownerName = pInfo(peer)->tankIDName;

		// Build detailed console report for sender
		std::string consoleReport = "`2Giveaway `!Distributed to `2" + std::to_string(server->peerCount) + " `!players:\n";
		for (const auto& item : itemsToGive) {
			consoleReport += "`o- " + items[item.first].name + " `2(x" + std::to_string(item.second) + ")\n";
		}

		// Process each player
		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;

			// Give all items (using local copies to avoid const issues)
			for (const auto& item : itemsToGive) {
				int itemId = item.first;
				int itemCount = item.second;
				Inventory::Modify(cp, itemId, itemCount);
			}
			totalRecipients++;

			// Simple notification
			VarList::OnAddNotification(
				cp,
				"`2You received items from `0" + ownerName,
				"interface/atomic_button.rttex",
				"audio/hub_open.wav"
			);
		}

		VarList::OnConsoleMessage(peer, consoleReport);
	}
	static void GIVD(ENetPeer* peer, std::string param) {
		std::istringstream iss(param);
		int id, count;

		// Error handling
		if (!(iss >> id >> count)) {
			VarList::OnConsoleMessage(peer, "`4Usage: /givd <item_id> <amount>");
			return;
		}
		if (id <= 0 || count <= 0) {
			VarList::OnConsoleMessage(peer, "`4Invalid item ID or amount");
			return;
		}
		bool is_lock = items[id].blockType == BlockTypes::LOCK;
		if (is_lock) {
			VarList::OnConsoleMessage(peer, "`4Oops: `8You can't giveaway Lock");
			return;
		}
		int given = 0;
		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;

			Inventory::Modify(cp, id, count);
			given++;

			// ? Correct notification format (matches ASB function)
			VarList::OnAddNotification(
				cp,  // Target player
				"You received: " + items[id].name,  // Message
				"interface/atomic_button.rttex",   // Texture
				"audio/hub_open.wav"               // Sound
			);
		}

		// Console feedback for sender
		VarList::OnConsoleMessage(peer,
			"[GIVEAWAY] Distributed " + items[id].name +
			" to " + std::to_string(given) + " players"
		);
	}
	static void GIVEAWAYROLE(ENetPeer* peer, std::string param) {
		if (!Role::Clist(pInfo(peer)->tankIDName)) return;

		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wGIVEAWAY ROLE|left|15042|\n"
			"add_spacer|small|\n"
			"add_textbox|`0Select the role you want to give away random!|left|\n"
			"add_spacer|small|\n"
			"add_checkbox|give_vip|`1Giveaway VIP|0|\n"
			"add_checkbox|give_mod|`2Giveaway Moderator|0|\n"
			"add_checkbox|give_admin|`3Giveaway Administrator|0|\n"
			"add_checkbox|give_dev|`4Giveaway Developer|0|\n"
			"add_checkbox|give_god|`5Giveaway God|0|\n"
			"add_checkbox|give_unli|`9Giveaway Unlimited|0|\n"
			"add_checkbox|give_boost|`6Giveaway BOOST|0|\n"
			"add_checkbox|give_sboost|`8Giveaway SUPER BOOST|0|\n"
			"add_spacer|small|\n"
			"add_button|apply_giveaway|Apply Giveaway|\n"
			"end_dialog|giveaway_role_dialog|Cancel||\n"
			"add_quick_exit|";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void timestop(ENetPeer* peer) {
		std::string current_world = pInfo(peer)->world;

		auto it = std::find_if(worlds.begin(), worlds.end(), [current_world](const World& w) {
			return w.name == current_world;
			});
		if (it == worlds.end()) return;

		World* world_ = &(*it);
		uint8_t old_weather = world_->weather;
		world_->weather = 43; // Freeze weather effect

		std::vector<ENetPeer*> affected_peers;

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == nullptr) continue;
			if (pInfo(cp_)->world != current_world) continue;

			// ? Jangan freeze owner
			if (cp_ == peer) continue;

			// ? Freeze player lain
			Playmods::Add(cp_, 2);
			VisualHandle::State(pInfo(cp_));
			Clothing_V2::Update(cp_, true);
			VarList::OnSetCurrentWeather(cp_, 43);
			VarList::OnAddNotification(cp_, "`4Time has been stopped for 5 seconds!`w", "interface/atomic_button.rttex", "audio/hub_open.wav");

			affected_peers.push_back(cp_);
		}

		// ? Info untuk owner (khusus, tidak perlu kasih freeze logic ekstra)
		VarList::OnConsoleMessage(peer, "`2Time Stop activated for 5 seconds.");

		// ? Delay untuk menghapus efek freeze
		std::thread([affected_peers, old_weather, current_world]() {
			std::this_thread::sleep_for(std::chrono::seconds(5));

			auto it = std::find_if(worlds.begin(), worlds.end(), [current_world](const World& w) {
				return w.name == current_world;
				});
			if (it == worlds.end()) return;

			World* world_ = &(*it);
			world_->weather = old_weather;

			for (ENetPeer* cp_ : affected_peers) {
				if (!cp_ || cp_->data == nullptr) continue;

				Playmods::Remove(pInfo(cp_), 2);
				VisualHandle::State(pInfo(cp_));
				Clothing_V2::Update(cp_, true);
				VarList::OnSetCurrentWeather(cp_, old_weather);
				VarList::OnAddNotification(cp_, "`2Time resumes.`w", "interface/atomic_button.rttex", "audio/hub_open.wav");
			}
			}).detach();
	}
	static void GachaHarian(ENetPeer* peer) {
		time_t t = time(0);
		struct tm* timeInfo = localtime(&t);

		// Check if current time is within a specific range for Indonesia (WIB - UTC +7)
		if (timeInfo->tm_hour >= 0 && timeInfo->tm_hour < 24) { // can specify a time window if needed

			// Random reward between 2 and 50 gems and 2 and 50 EXP
			int gems = rand() % 49 + 2;

			// Apply rewards to the player
			pInfo(peer)->gems += gems;

			// Send dialog message to player
			std::string rewardMessage = "You received " + std::to_string(gems) + " Gems";
		}
	}
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
		size_t totalSize = size * nmemb;
		output->append((char*)contents, totalSize);
		return totalSize;
	}

	static std::string translateText(const std::string& text, const std::string& sourceLang, const std::string& targetLang) {
		CURL* curl = curl_easy_init();
		std::string response;

		if (curl) {
			std::string postFields = "q=" + text + "&source=" + sourceLang + "&target=" + targetLang;
			curl_easy_setopt(curl, CURLOPT_URL, "https://libretranslate.com/translate");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

			struct curl_slist* headers = NULL;
			headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}

		return response;
	}
	static void TRANSLATE(ENetPeer* peer, const std::string& param, const std::string& fromLang, const std::string& toLang) {
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "`4[Error] Masukkan teks yang ingin diterjemahkan.");
			return;
		}

		std::thread([peer, param, fromLang, toLang]() {
			std::string result = askTranslate(param, fromLang, toLang);
			if (result.length() > 2000)
				result = result.substr(0, 2000) + "...";

			VarList::OnConsoleMessage(peer, "`#Hasil Translasi");
			VarList::OnConsoleMessage(peer, "`oDari (" + fromLang + ") ? (" + toLang + ")");
			VarList::OnConsoleMessage(peer, "`wInput: " + param);
			VarList::OnConsoleMessage(peer, "`5Output: " + result);
			}).detach();
	}
	static void OFFLINEALBIN(ENetPeer* peer, const std::string& param, int level) {
		namespace fs = std::filesystem;
		std::vector<std::string> offlineList;

		for (const auto& entry : fs::directory_iterator("database/players/")) {
			if (!entry.is_regular_file()) continue;
			std::string filename = entry.path().filename().string();
			if (filename.find(".json") == std::string::npos) continue;
			std::string growid = filename.substr(0, filename.find(".json"));

			bool isOnline = false;
			for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
				if (p->state != ENET_PEER_STATE_CONNECTED || !p->data) continue;
				if (to_lower(pInfo(p)->tankIDName) == to_lower(growid)) {
					isOnline = true;
					break;
				}
			}

			if (!isOnline) offlineList.push_back(growid);
		}

		VarList::OnConsoleMessage(peer, "`oGrowID Offline (" + std::to_string(offlineList.size()) + "):");
		int count = 0;
		for (const auto& name : offlineList) {
			VarList::OnConsoleMessage(peer, "- `w" + name);
			if (++count >= 100) {
				VarList::OnConsoleMessage(peer, "`o...dan lainnya tidak ditampilkan.");
				break;
			}
		}
	}
	static void TRASH_ITEMS(ENetPeer* peer) {
		std::string trash_item = "";
		for (int i_ = 0; i_ < pInfo(peer)->inv.size(); i_++) {
			if (items[pInfo(peer)->inv[i_].first].untradeable == 0 && (items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::FISH || items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::LOCK
				|| items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::CRYSTAL || items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::RANDOM_BLOCK
				|| items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::OVEN || items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::BACKGROUND
				|| items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::DOOR || items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::SEED
				|| items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::WEATHER || items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::CONSUMABLE
				|| items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::CLOTHING || items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::BULLETIN_BOARD
				|| items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::ACHIEVEMENT_BLOCK || items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::SIGN
				|| items[pInfo(peer)->inv[i_].first].blockType == BlockTypes::FOREGROUND)) {
				trash_item += "\nadd_checkicon|" + to_string(pInfo(peer)->inv[i_].first) + "|" + " `2" + items[pInfo(peer)->inv[i_].first].ori_name + "|staticPurpleFrame|" + to_string(pInfo(peer)->inv[i_].first) + "||\n";
			}
		}
		if (trash_item.empty()) {
			trash_item = "\nadd_textbox|`oYou don't have any Item for Trash!|left|";
		}
		VarList::OnDialogRequest(peer, +"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wTrasher|left|9922|\nadd_spacer|small|\nadd_textbox|`wItems available for Trash:|left|\nadd_spacer|small|\nadd_textbox|`5Easily Trash your items by selecting from the list.|left|left|\ntext_scaling_string|Subscribtions++++++++|\nadd_spacer|small|" + trash_item + "\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nend_dialog|trash_item_inv|`4Close|`9Trash it!|");
	}
	static void DROP_ITEMS(ENetPeer* peer) {
		// Pastikan peer valid
		if (!peer || !pInfo(peer)) return;

		// Buat dialog untuk memilih item
		std::string drop_item = "";
		int item_count = 0;

		for (const auto& item : pInfo(peer)->inv) {
			int item_id = item.first;
			int quantity = item.second;

			// Filter item yang bisa di-drop
			if (quantity > 0 && item_id > 0 && item_id < items.size() &&
				items[item_id].untradeable == 0 &&
				(items[item_id].blockType == BlockTypes::FISH ||
					items[item_id].blockType == BlockTypes::LOCK ||
					items[item_id].blockType == BlockTypes::CRYSTAL ||
					items[item_id].blockType == BlockTypes::CLOTHING ||
					items[item_id].blockType == BlockTypes::FOREGROUND ||
					items[item_id].blockType == BlockTypes::BACKGROUND)) {

				drop_item += "\nadd_checkicon|" + std::to_string(item_id) + "|" +
					" `2" + items[item_id].ori_name + "|staticPurpleFrame|" +
					std::to_string(item_id) + "||\n";
				item_count++;
			}
		}

		if (item_count == 0) {
			drop_item = "\nadd_textbox|`oYou don't have any droppable items!|left|";
		}

		VarList::OnDialogRequest(peer,
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
			"add_label_with_icon|big|`wItem Dropper|left|9922|\n"
			"add_spacer|small|\n"
			"add_textbox|`wSelect items to drop:|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`5Choose items to drop on the ground|left|\n"
			"text_scaling_string|Subscribtions++++++++|\n"
			"add_spacer|small|" + drop_item +
			"\nadd_button_with_icon||END_LIST|noflags|0||\n"
			"add_spacer|small|\n"
			"end_dialog|drop_item_inv|`4Close|`9Drop Selected|");
	}
	static void EROLE(ENetPeer* peer, const std::string& param) {
		if (!peer || !pInfo(peer)) return;

		std::string dialog =
			"add_label_with_icon|big|`wRole Shop|left|8470|\n"
			"add_textbox|`oSelect a role to buy below.|left|\n"
			"add_spacer|small|\n"
			"add_button_with_icon|buy_cheater|`4Buy Cheater Role (20 mgl)|staticBlueFrame|9386|\n"
			"add_button_with_icon|buy_boost|`9Buy Boost Role (20 mgl)|staticBlueFrame|9928|\n"
			"add_button_with_icon|buy_vip|`2Buy VIP Role (10 mgl)|staticBlueFrame|7188|\n"
			"add_button_with_icon|buy_superboost|`3Buy SUPER BOOST (30 mgl)|staticBlueFrame|6022|\n"
			"end_dialog|buy_role_dialog_confirm||";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void SET_CSN(ENetPeer* peer, std::string cmd) {
		if (!Role::Clist(pInfo(peer)->tankIDName))
			return;

		auto args = explode(" ", cmd);
		if (args.size() < 2) {
			VarList::OnConsoleMessage(peer, "`wUsage: `/setcsn <growid> <0-36>` or `/setcsn <growid> reset`");
			return;
		}

		std::string growid = args[0];
		std::string action = to_lower(args[1]);

		for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
			if (p->state != ENET_PEER_STATE_CONNECTED || !p->data) continue;
			if (to_lower(pInfo(p)->tankIDName) == to_lower(growid)) {

				if (action == "reset") {
					pInfo(p)->csn = -1;
					VarList::OnConsoleMessage(peer, "`2Successfully reset spin number of `5" + pInfo(p)->tankIDName + "`2 to normal.");
					return;
				}

				if (is_number(action)) {
					int value = std::stoi(action);
					if (value < 0 || value > 36) {
						VarList::OnConsoleMessage(peer, "`4Invalid number. Must be between 0 and 36.");
						return;
					}
					pInfo(p)->csn = value;
					VarList::OnConsoleMessage(peer, "`2Set spin number of `5" + pInfo(p)->tankIDName + "`2 to `w" + std::to_string(value) + "`2.");
					return;
				}

				VarList::OnConsoleMessage(peer, "`4Invalid argument. Use `wreset` `4or a number between `w0�36`4.");
				return;
			}
		}

		VarList::OnConsoleMessage(peer, "`4Player `5" + growid + "`4 not found online.");
	}
	static void GET_ITEM_BY_NAME(ENetPeer* peer, std::string param) {
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "`4Usage: /get <item_name>");
			return;
		}

		std::string keyword = to_lower(param);
		int found = 0;

		for (int i = 0; i < items.size(); i++) {
			if (items[i].name.empty()) continue;

			// Hindari seed
			if (items[i].blockType == BlockTypes::SEED) continue;
			std::string lowerName = to_lower(items[i].name);
			if (lowerName.find(keyword) != std::string::npos) {
				// Restrict LOCK and GACHA items to Keizer and Albin only
				bool is_lock = items[i].blockType == BlockTypes::LOCK;
				static const std::vector<int> gacha_ids_ = { 10716, 3402, 14084, 5136, 14596, 7960, 9350 };
				bool is_gacha = std::find(gacha_ids_.begin(), gacha_ids_.end(), i) != gacha_ids_.end();
				if ((is_lock || is_gacha) && !IsKeizerOrAlbin(peer)) continue;
				// (C) items: hanya config_access atau Clist yang bisa /get
				bool is_c_item_ = items[i].name.size() >= 4 && items[i].name[0] == '(' && items[i].name[1] == 'C' && items[i].name[2] == ')' && items[i].name[3] == ' ';
				if (is_c_item_ && !Role::has_config_access(peer) && !Role::Clist(pInfo(peer)->tankIDName)) continue;
				int amount = 200;
				Inventory::Modify(peer, i, amount);
				found++;
			}
		}

		if (found > 0) {
			VarList::OnConsoleMessage(peer, "`2You received " + std::to_string(found) + " item(s) matching: `w" + param);
		}
		else {
			VarList::OnConsoleMessage(peer, "`4No items found matching: `w" + param);
		}
	}
	static void SKIP_LQUEST(ENetPeer* peer, const std::string& param) {
		if (!peer || !peer->data) return;

		Player* player = pInfo(peer);
		if (!player) return;

		int step = player->lwiz_step;
		if (step < 1 || step > 20) {
			VarList::OnConsoleMessage(peer, "`4You don't have an active Legendary Wizard quest.");
			return;
		}

		player->legendary_quest[step - 1][0] = player->legendary_quest[step - 1][1];
		add_lwiz_points(peer, player->legendary_quest[step - 1][1]);
		player->lwiz_notification = 1;

		VarList::OnConsoleMessage(peer, "`2Skipped step `w" + std::to_string(step) + "`2 of Legendary Wizard quest.");
	}
	static void saveCommandRolesToFile() {
		nlohmann::json data;
		for (const auto& [cmd, pair] : commandMap)
			data[cmd] = pair.second;

		std::ofstream out("database/json/command_roles.json");
		if (out.is_open()) {
			out << data.dump(4);
			out.close();
		}
	}
	static void add_cmd(ENetPeer* peer, const std::string& param, int level) {
		if (level < 9) {
			VarList::OnConsoleMessage(peer, "`4You have no permission.");
			return;
		}

		auto parts = explode(" ", param);
		if (parts.size() != 2) {
			VarList::OnConsoleMessage(peer, "`4Usage:`w /addcommand <cmd> <level>");
			return;
		}

		std::string cmd = "/" + parts[0];
		int new_level = std::stoi(parts[1]);

		if (new_level < 0 || new_level > 9) {
			VarList::OnConsoleMessage(peer, "`4Invalid level. Must be 0�9.");
			return;
		}

		if (defaultCommandHandlers.count(cmd)) {
			commandMap[cmd] = { defaultCommandHandlers[cmd], new_level };
			VarList::OnConsoleMessage(peer, "`2Command `w" + cmd + "`2 enabled at level `w" + std::to_string(new_level));
			saveCommandRolesToFile();
		}
		else {
			VarList::OnConsoleMessage(peer, "`4Command handler not found for `w" + cmd);
		}
	}
	static void remove_cmd(ENetPeer* peer, const std::string& param, int level) {
		auto parts = explode(" ", param);
		if (parts.size() != 2) {
			VarList::OnConsoleMessage(peer, "`4Usage:`w /removecommand <cmd> <your_level>");
			return;
		}

		std::string cmd = "/" + parts[0];
		int required_level = std::stoi(parts[1]);

		if (level < required_level) {
			VarList::OnConsoleMessage(peer, "`4You need level `w" + std::to_string(required_level) + "`4 or higher.");
			return;
		}

		if (commandMap.count(cmd)) {
			commandMap.erase(cmd);
			VarList::OnConsoleMessage(peer, "`2Command `w" + cmd + "`2 removed.");
			saveCommandRolesToFile();
		}
		else {
			VarList::OnConsoleMessage(peer, "`4Command `w" + cmd + "`4 not found.");
		}
	}
	static void P_WARP(ENetPeer* peer, const std::string& param) {
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /pwarp <growid> <world>");
			return;
		}

		auto parts = explode(" ", param);
		if (parts.size() < 2) {
			VarList::OnConsoleMessage(peer, "`oUsage: /pwarp <growid> <world>");
			return;
		}

		std::string targetName = to_lower(parts[0]);
		std::string world = parts[1];

		for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
			if (p && p->data && to_lower(pInfo(p)->tankIDName) == targetName) {
				if (pInfo(p)->world == world) {
					VarList::OnConsoleMessage(peer, "`4Player is already in that world.");
					return;
				}

				pInfo(p)->update = true;
				VarList::OnConsoleMessage(peer, "`2Teleported " + pInfo(p)->tankIDName + " to `5" + world + "``.");
				Enter_World(p, world);
				VarList::OnConsoleMessage(p, "`2You have been teleported to `5" + world + "`` by an admin.");
				return;
			}
		}
	}
	static void EXTRANUKE(ENetPeer* peer, const std::string& param) {
		if (!Role::Developer(peer)) {
			VarList::OnConsoleMessage(peer, "`4No permission.");
			return;
		}

		auto worldsList = explode(" ", param);
		if (worldsList.empty() || worldsList.size() > 100) {
			VarList::OnConsoleMessage(peer, "`oUsage: /extranuke <world1> <world2> ...> (Max 100 worlds)");
			return;
		}

		int nuked = 0;
		for (const auto& name : worldsList) {
			std::string worldName = to_upper(name);
			auto it = std::find_if(worlds.begin(), worlds.end(), [worldName](const World& w) {
				return to_upper(w.name) == worldName;
				});

			if (it == worlds.end()) {
				VarList::OnConsoleMessage(peer, "`4World not found: `w" + name);
				continue;
			}

			World* world = &(*it);
			if (!world->nuked_by.empty()) {
				VarList::OnConsoleMessage(peer, "`w" + name + " `4is already nuked.");
				continue;
			}

			world->nuked_by = "`$EXTRANUKE";
			for (int i = 0; i < world->blocks.size(); i++) {
				if (world->blocks[i].fg == 202 || world->blocks[i].fg == 204 || world->blocks[i].fg == 206 || world->blocks[i].fg == 4994 || world->blocks[i].fg == 10000 || world->blocks[i].fg == 5814) continue;
				if (items[world->blocks[i].fg].blockType == BlockTypes::LOCK) {
					world->last_lock = world->blocks[i].fg;
					world->blocks[i].fg = 2950;
				}
			}

			for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
				if (!p->data) continue;
				if (pInfo(p)->world == world->name) {
					Exit_World(p);
					VarList::OnConsoleMessage(p, "`4This world has been nuked by developer.");
				}
			}

			ServerPool::Logs::Add(pInfo(peer)->tankIDName + " used /extranuke on `" + worldName, "NUKED WORLD");
			nuked++;
		}

		VarList::OnConsoleMessage(peer, "`2Nuked " + std::to_string(nuked) + " worlds successfully.");
	}
	static std::string trim(const std::string& s) {
		size_t start = s.find_first_not_of(" \t\r\n");
		size_t end = s.find_last_not_of(" \t\r\n");
		if (start == std::string::npos || end == std::string::npos)
			return "";
		return s.substr(start, end - start + 1);
	}
	static bool is_world_exist(const std::string& name) {
		for (auto& w : worlds) {
			if (to_lower(w.name) == to_lower(name)) return true;
		}
		return false;
	}

	static void remove_world(const std::string& name) {
		worlds.erase(std::remove_if(worlds.begin(), worlds.end(), [&](const World& w) {
			return to_lower(w.name) == to_lower(name);
			}), worlds.end());
	}
	static std::string to_lower(const std::string& str) {
		std::string out = str;
		std::transform(out.begin(), out.end(), out.begin(), ::tolower);
		return out;
	}
	static void UNBLACKLIST(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /unblacklist <Nick>");
			return;
		}

		std::string name = cmd;
		std::ifstream ifs("database/players/" + name + "_.json");
		if (!ifs.is_open()) {
			VarList::OnConsoleMessage(peer, "`4Player not found.");
			return;
		}

		json j;
		ifs >> j;
		std::string pname = j["name"].get<std::string>();
		std::string prid = j["rid"].get<std::string>();

		bool removed = false;

		auto& bl = Environment()->BlackList;
		auto it = std::remove(bl.begin(), bl.end(), pname);
		if (it != bl.end()) {
			bl.erase(it, bl.end());
			removed = true;
		}
		it = std::remove(bl.begin(), bl.end(), prid);
		if (it != bl.end()) {
			bl.erase(it, bl.end());
			removed = true;
		}

		if (removed) {
			VarList::OnConsoleMessage(peer, "`2" + pname + " berhasil dihapus dari blacklist.");
			ServerPool::Logs::Add("player: " + pInfo(peer)->tankIDName + " un-blacklisted (" + prid + ") - " + pname, "/unblacklist");
		}
		else {
			VarList::OnConsoleMessage(peer, "`4" + pname + " tidak ada di daftar blacklist.");
		}
	}
	static void sellallfish_cmd(ENetPeer* peer) {
		if (!peer || !pInfo(peer)) return;

		int total_wl = 0;
		for (const auto& [id, price] : exchangeSetPrice) {
			if (items[id].blockType != BlockTypes::FISH) continue;
			int count = Inventory::Contains(peer, id);
			if (count <= 0 || price <= 0) continue;

			int minus = -count;
			Inventory::Modify(peer, id, minus);
			total_wl += price * count;
		}

		if (total_wl <= 0) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "No fish to sell.", 0, 1);
			return;
		}

		int wl = total_wl % 100;
		int total = total_wl / 100;

		int dl = total % 100;
		total /= 100;

		int bgl = total % 100;
		total /= 100;

		int ggl = total % 100;
		int mgl = total / 100;

		auto give_item = [&](int item_id, int amount) {
			if (amount <= 0) return;

			if (!Inventory::Check_Max(peer, item_id, amount)) {
				Inventory::Modify(peer, item_id, amount);
			}
			else {
				WorldDrop drop{};
				drop.id = item_id;
				drop.count = amount;
				drop.x = pInfo(peer)->x + rand() % 2;
				drop.y = pInfo(peer)->y + rand() % 2;

				std::string name = pInfo(peer)->world;
				auto it = std::find_if(worlds.begin(), worlds.end(), [&](const World& w) {
					return w.name == name;
					});

				if (it != worlds.end()) {
					VisualHandle::Drop(&(*it), drop);
				}
			}
			};

		give_item(8470, mgl);  // mgl
		give_item(7188, bgl);  // bgl
		give_item(1796, dl);   // DL
		give_item(242, wl);    // WL

		VarList::OnConsoleMessage(peer, "`2Fish sold! Profit delivered safely.");
	}
	static void save_custom_exchanges() {
		json j;
		for (auto& [item_id, list] : customExchangeList) {
			for (auto& [need, cid, amt] : list) {
				j[std::to_string(item_id)].push_back({ need, cid, amt });
			}
		}
		std::ofstream f("database/json/custom_exchanges.json");
		if (f.is_open()) f << j.dump(4), f.close();
	}

	static void load_custom_exchanges() {
		std::ifstream f("database/json/custom_exchanges.json");
		if (!f.is_open()) return;
		json j; f >> j;
		for (auto& [key, val] : j.items()) {
			int item_id = std::stoi(key);
			for (auto& arr : val) {
				if (arr.is_array() && arr.size() == 3)
					customExchangeList[item_id].push_back({ arr[0], arr[1], arr[2] });
			}
		}
	}
	static void CUSTOM_EXCHANGE(ENetPeer* peer) {
		std::string dialog = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n";
		dialog += "add_label_with_icon|big|`w[`2Custom`w] Exchange Menu|left|112|\n";
		dialog += "add_textbox|`oChoose an item to exchange below:|left|\n";
		dialog += "add_spacer|small|\n";

		for (auto& [item_id, list] : customExchangeList) {
			if (Inventory::Contains(peer, item_id) > 0) {
				dialog += "add_button_with_icon|e_" + std::to_string(item_id) + "|" + items[item_id].name + "|staticBlueFrame|""||\n";
			}
		}

		dialog += "add_spacer|small|\n";
		dialog += "add_button|cancel_btn|Cancel|left|\n";
		dialog += "end_dialog|e||";
		VarList::OnDialogRequest(peer, dialog, 700);
	}
	static void setcustomexchange_cmd(ENetPeer* peer, const std::string& text, int) {
		std::istringstream ss(text);
		int item_id, need, currency_id, reward;
		if (!(ss >> item_id >> need >> currency_id >> reward)) {
			VarList::OnConsoleMessage(peer, "`4Usage: /setcustomexchange <item_id> <need> <currency_id> <reward>");
			return;
		}
		if (item_id <= 0 || currency_id <= 0 || need <= 0 || reward <= 0) {
			VarList::OnConsoleMessage(peer, "`4Invalid values.");
			return;
		}

		auto& list = customExchangeList[item_id];
		for (const auto& tpl : list) {
			if (std::get<0>(tpl) == need && std::get<1>(tpl) == currency_id && std::get<2>(tpl) == reward) {
				VarList::OnConsoleMessage(peer, "`6That exchange already exists.");
				return;
			}
		}

		list.emplace_back(need, currency_id, reward);
		save_custom_exchanges();
		VarList::OnConsoleMessage(peer, "`2Added: `w" + std::to_string(need) + " " + items[item_id].name + "`2 ? `w" + std::to_string(reward) + " " + items[currency_id].name);
	}
	static void rmvcustomexchange_cmd(ENetPeer* peer, const std::string& text, int) {
		std::istringstream ss(text);
		int item_id, need, currency_id, reward;
		if (!(ss >> item_id >> need >> currency_id >> reward)) {
			VarList::OnConsoleMessage(peer, "`4Usage: /rmvcustomexchange <item_id> <need> <currency_id> <reward>");
			return;
		}

		auto& list = customExchangeList[item_id];
		auto it = std::remove_if(list.begin(), list.end(), [&](const auto& tpl) {
			return std::get<0>(tpl) == need && std::get<1>(tpl) == currency_id && std::get<2>(tpl) == reward;
			});
		if (it != list.end()) {
			list.erase(it, list.end());
			save_custom_exchanges();
			VarList::OnConsoleMessage(peer, "`2Removed exchange config for `w" + items[item_id].name);
		}
		else {
			VarList::OnConsoleMessage(peer, "`4Exchange rule not found.");
		}
	}
	static void Show_SetClaimRoleDialog(ENetPeer* peer) {
		json j;
		std::ifstream f("database/json/setclaimrole.json");
		if (f.good()) f >> j;

		auto gb = [&](const std::string& sec, const std::string& key) -> std::string {
			if (j.contains(sec) && j[sec].contains(key) && j[sec][key].is_boolean() && j[sec][key].get<bool>()) return "1";
			return "0";
			};
		auto gi = [&](const std::string& key) -> std::string {
			if (j.contains("amounts") && j["amounts"].contains(key) && j["amounts"][key].is_number_integer()) return std::to_string(j["amounts"][key].get<int>());
			return "0";
			};

		std::string dlg = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dlg += "add_label_with_icon|big|Set Claimable Roles & Assets|left|15042|\n";
		dlg += "add_spacer|small|\n";
		dlg += "add_textbox|`5- Roles -``|left|\n";
		dlg += "add_checkbox|role_1|`1[`wVIP`1]|" + gb("roles", "VIP") + "|\n";
		dlg += "add_checkbox|role_2|`#@MODERATOR|" + gb("roles", "MODERATOR") + "|\n";
		dlg += "add_checkbox|role_3|`e@ADMINISTRATOR|" + gb("roles", "ADMINISTRATOR") + "|\n";
		dlg += "add_checkbox|role_4|`6@DEVELOPER|" + gb("roles", "DEVELOPER") + "|\n";
		dlg += "add_checkbox|role_5|`b@OWNER SERVER|" + gb("roles", "OWNER") + "|\n";
		dlg += "add_checkbox|role_6|CHEATER|" + gb("roles", "CHEATER") + "|\n";
		dlg += "add_checkbox|role_7|`5[BOOST]``|" + gb("roles", "BOOST") + "|\n";
		dlg += "add_checkbox|role_8|`8[SUPER BOOST]``|" + gb("roles", "SUPER_BOOST") + "|\n";
		dlg += "add_checkbox|role_9|`9@GOD|" + gb("roles", "GOD") + "|\n";
		dlg += "add_checkbox|role_10|`8@UNLIMITED|" + gb("roles", "UNLIMITED") + "|\n";
		dlg += "add_spacer|small|\n";
		dlg += "add_custom_break|\n";
		dlg += "add_textbox|`5- Assets (Amount Based) -``|left|\n";
		dlg += "text_scaling_string|DEFIBRILLATOR|\n";
		dlg += "add_checkicon|gems|`oGEMS||14590||" + gb("assets_amount", "gems") + "||\n";
		dlg += "add_checkicon|level|`oLEVEL||1488||" + gb("assets_amount", "level") + "||\n";
		dlg += "add_checkicon|coin|`oCOIN||244||" + gb("assets_amount", "coin") + "||\n";
		dlg += "add_checkicon|opc|`oOPC||10668||" + gb("assets_amount", "opc") + "||\n";
		dlg += "add_checkicon|token|`oGUILD TOKEN||" + std::to_string(guild_event_id) + "||" + gb("assets_amount", "token") + "||\n";
		dlg += "add_checkicon|seals|`oSEALS||9186||" + gb("assets_amount", "seals") + "||\n";
		dlg += "add_smalltext|`oNOTE: Amount below applies to selected amount-based assets.|left|\n";
		dlg += "add_text_input|amount|`oAmount:|" + gi("amount") + "|15|\n";
		dlg += "add_spacer|small|\n";
		dlg += "add_custom_break|\n";
		dlg += "add_textbox|`5- Assets (Unlocks) -``|left|\n";
		dlg += "add_checkicon|growpass|`oGROWPASS||11304||" + gb("assets_unlock", "growpass") + "||\n";
		dlg += "add_checkicon|roadtoglory|`oROAD TO GLORY||9436||" + gb("assets_unlock", "roadtoglory") + "||\n";
		dlg += "add_checkicon|piggybank|`oPIGGY BANK||0||" + gb("assets_unlock", "piggybank") + "||\n";
		dlg += "add_checkicon|supp_1|`oSUPPORTER||10860||" + gb("assets_unlock", "supp_1") + "||\n";
		dlg += "add_checkicon|supp_2|`oSUPER-SUPP||10862||" + gb("assets_unlock", "supp_2") + "||\n";
		dlg += "add_spacer|small|\n";
		dlg += "add_custom_break|\n";
		dlg += "add_textbox|`5- Titles -``|left|\n";
		dlg += "add_checkicon|oflegend|`oOf Legend||1794||" + gb("titles", "oflegend") + "||\n";
		dlg += "add_checkicon|doctor|`oDr.Name||7068||" + gb("titles", "doctor") + "||\n";
		dlg += "add_checkicon|grow4good|`oGrow4Good||11816||" + gb("titles", "grow4good") + "||\n";
		dlg += "add_checkicon|mentor|`oMentor||9472||" + gb("titles", "mentor") + "||\n";
		dlg += "add_checkicon|tiktok|`oTiktok Badge||9530||" + gb("titles", "tiktok") + "||\n";
		dlg += "add_checkicon|content|`oContent Badge||10866||" + gb("titles", "content") + "||\n";
		dlg += "add_checkicon|partyanimal|`oParty Animal||14186||" + gb("titles", "partyanimal") + "||\n";
		dlg += "add_checkicon|tgt|`oThanksGiving||10320||" + gb("titles", "tgt") + "||\n";
		dlg += "add_checkicon|oldtimer|`oOld Timer||4370||" + gb("titles", "oldtimer") + "||\n";
		dlg += "add_checkicon|santa|`oSanta Claus||1360||" + gb("titles", "santa") + "||\n";
		dlg += "add_checkicon|gp_bronze|`oGPass Bronze||14412||" + gb("titles", "gp_bronze") + "||\n";
		dlg += "add_checkicon|gp_silver|`oGPass Silver||14410||" + gb("titles", "gp_silver") + "||\n";
		dlg += "add_checkicon|gp_gold|`oGPass Golden||14408||" + gb("titles", "gp_gold") + "||\n";
		dlg += "add_checkicon|aw_win|`oAward Winning||15114||" + gb("titles", "aw_win") + "||\n";
		dlg += "add_spacer|small|\n";
		dlg += "add_custom_break|\n";
		dlg += "add_textbox|`5- Roles Titles -``|left|\n";
		dlg += "add_checkicon|roles_farmer|`oFarmer||7064||" + gb("roles_title", "roles_farmer") + "||\n";
		dlg += "add_checkicon|roles_builder|`oBuilder||7070||" + gb("roles_title", "roles_builder") + "||\n";
		dlg += "add_checkicon|roles_surgeon|`oSurgeon||7068||" + gb("roles_title", "roles_surgeon") + "||\n";
		dlg += "add_checkicon|roles_fisher|`oFisher||7072||" + gb("roles_title", "roles_fisher") + "||\n";
		dlg += "add_checkicon|roles_chef|`oChef||7076||" + gb("roles_title", "roles_chef") + "||\n";
		dlg += "add_checkicon|roles_startopian|`oStartopian||7074||" + gb("roles_title", "roles_startopian") + "||\n";
		dlg += "add_spacer|small|\n";
		dlg += "add_custom_button|apply_claimrole|textLabel:`wSave;middle_colour:80543231;border_colour:80543231;|\n";
		dlg += "end_dialog|set_claimrole|||\n";
		dlg += "add_quick_exit|\n";
		VarList::OnDialogRequest(peer, dlg);
	}
	static void sellfish_dialog(ENetPeer* peer) {
		if (!peer || !peer->data) return;

		if (pInfo(peer)->level < 10) {
			VarList::OnConsoleMessage(peer, "`oYou need to be at least level 10 to use this feature.");
			return;
		}

		int total_fish = 0;
		int total_value = 0;
		int min_price = 0;
		int max_price = 0;

		std::vector<std::pair<int, int>> snapshot;
		{
			std::lock_guard<std::mutex> lk(g_exchangeSetPriceMtx);
			snapshot.reserve(exchangeSetPrice.size());
			for (const auto& kv : exchangeSetPrice) snapshot.push_back(kv);
		}

		std::string dialog;
		dialog.reserve(4096);

		dialog += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|`wAquatic Treasures``|left|7002|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_textbox|`oYo, got any fish to sell?`w I'll buy 'em from you, no questions asked!|left|\n";
		dialog += "add_textbox|`oFish prices depend on the type. Each fish gives a fixed WL, weight (lbs) doesn't matter.|left|\n";
		dialog += "add_spacer|small|\n";

		const int items_sz = (int)items.size();

		for (const auto& kv : snapshot) {
			const int id = kv.first;
			const int price = kv.second;

			if (id <= 0 || id >= items_sz) continue;
			if (price <= 0) continue;
			if (items[id].blockType != BlockTypes::FISH) continue;

			int lbs = 0;
			try {
				lbs = Inventory::Contains(peer, id);
			}
			catch (...) {
				lbs = 0;
			}
			if (lbs <= 0) continue;

			const int fish_count = 1;
			const int total_for_fish = price * fish_count;

			if (min_price == 0 || price < min_price) min_price = price;
			if (max_price == 0 || price > max_price) max_price = price;

			total_fish += fish_count;
			total_value += total_for_fish;

			dialog += "add_checkbox|fish_" + std::to_string(id) + "|`w" + items[id].name +
				"`` - `2" + std::to_string(price) + " WL`0 /fish (`w" +
				std::to_string(lbs) + " lbs`0, `2" +
				std::to_string(total_for_fish) + " WL`0 total)|0|\n";
		}

		dialog += "add_spacer|small|\n";
		dialog += "add_textbox|`wYou currently have `2" + std::to_string(total_fish) + " `wfish in your backpack.|left|\n";
		dialog += "add_textbox|`wIf you sell everything above, you will get about `2" + std::to_string(total_value) + " WL`w in total.|left|\n";

		if (total_fish > 0 && min_price > 0 && max_price > 0) {
			if (min_price == max_price) {
				dialog += "add_textbox|`oAll listed fish are worth `2" + std::to_string(min_price) + " WL`o per fish.|left|\n";
			}
			else {
				dialog += "add_textbox|`oFish prices range from `2" + std::to_string(min_price) + " WL`o up to `2" + std::to_string(max_price) + " WL`o per fish.|left|\n";
			}
		}

		dialog += "add_spacer|small|\n";

		if (total_fish > 0) {
			dialog += "add_button|sell|`wSell Selected|noflags|0|0|\n";
			dialog += "add_button|sellall|`wSell All|noflags|0|0|\n";
		}
		else {
			dialog += "add_textbox|`4No fish found in your backpack.|left|\n";
		}

		dialog += "add_spacer|small|\n";

		// === AUTO SELL FISH TOGGLE ===
		bool asf_on = pInfo(peer)->autosellfish;
		dialog += "add_textbox|`w--- `6Auto Sell Fish`` ---`w ------|left|\n";
		dialog += "add_textbox|`oIf `2ON`o: every new fish that enters your backpack will be sold automatically. If inventory is full, locks go to `2Bank Central`o.|left|\n";
		if (asf_on) {
			dialog += "add_button|autosellfish_toggle|`2[ON] Auto Sell Fish (Click to turn OFF)|noflags|0|0|\n";
		}
		else {
			dialog += "add_button|autosellfish_toggle|`4[OFF] Auto Sell Fish (Click to turn ON)|noflags|0|0|\n";
		}

		dialog += "add_spacer|small|\n";
		dialog += "add_textbox|`4Note: `wNo refunds after selling.|left|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_small_font_button|backsetexchange|Back to Exchange|noflags|0|0|\n";
		dialog += "add_quick_exit|\n";
		dialog += "end_dialog|sellfish_logic|Nevermind||\n";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void sellcrystal_cmd(ENetPeer* peer) {
		if (pInfo(peer)->level < 10) {
			VarList::OnConsoleMessage(peer, "`4You need to be at least level 10 to sell crystals.");
			return;
		}

		std::string checks;
		for (const auto& slot : pInfo(peer)->inv) {
			int id = slot.first;
			int count = slot.second;
			if (count > 0 && items[id].blockType == BlockTypes::CRYSTAL) {
				checks += "add_checkicon|i" + std::to_string(id) + "||noflags|" + std::to_string(id) + "||0|\n";
			}
		}

		if (checks.empty()) {
			VarList::OnConsoleMessage(peer, "`4You don't have any crystal items to sell.");
			return;
		}

		std::string dialog;
		dialog = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|`9Hii!!! Sell Your Crystal Here friend!!|left|3720|\n";
		dialog += "add_smalltext|`9NOTE: `0If you sell an item and it is not locked enough in your backpack, it will automatically drop where you are standing|\n";
		dialog += "add_spacer|small|\n";
		dialog += "max_checks|1|\n";
		dialog += "add_smalltext|`oChoose item to sell/buy:|left|\n";
		dialog += checks;
		dialog += "add_custom_break|\n";
		dialog += "add_smalltext|`oAmount:|left|\n";
		dialog += "add_text_input|count_input|Enter amount||10|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_button|sell_action|`2Sell|noflags|0|0|\n";
		dialog += "add_quick_exit|\n";
		dialog += "add_button|backsetexchange|Back|0|0|\n";
		dialog += "end_dialog|sellcrystal_logic|||\n";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void EditCustomExchange(ENetPeer* peer) {
		json j;
		std::ifstream in("database/json/exchange_items.json");
		if (in.is_open()) {
			try { in >> j; }
			catch (...) { j = json::object(); }
			in.close();
		}

		if (!j.is_object()) j = json::object();
		if (!j.contains("exchanges") || !j["exchanges"].is_array()) j["exchanges"] = json::array();

		std::ofstream out0("database/json/exchange_items.json");
		if (out0.is_open()) {
			out0 << std::setw(4) << j;
			out0.close();
		}

		auto exchanges = j["exchanges"];

		std::string dialog = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|`wEdit Exchange Items``|left|32|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_button|add_exchange|`2+ Add Exchange|0|0|\n";
		dialog += "add_button|add_exchange10|`2+10 Exchanges|0|0|\n";
		dialog += "add_spacer|small|\n";

		int total = (int)exchanges.size();
		if (total < 0) total = 0;

		for (int i = 0; i < total; i++) {
			int from_id = exchanges[i].is_object() ? exchanges[i].value("from_id", 0) : 0;
			int from_count = exchanges[i].is_object() ? exchanges[i].value("from_count", 1) : 1;
			int to_id = exchanges[i].is_object() ? exchanges[i].value("to_id", 0) : 0;
			int to_count = exchanges[i].is_object() ? exchanges[i].value("to_count", 1) : 1;

			dialog += "add_label_with_icon|small|`cExchange " + std::to_string(i + 1) + ":|left|32|\n";
			dialog += "add_text_input|from_id" + std::to_string(i) + "|From Item ID:|" + std::to_string(from_id) + "|6|\n";
			dialog += "add_text_input|from_count" + std::to_string(i) + "|From Count:|" + std::to_string(from_count) + "|4|\n";
			dialog += "add_text_input|to_id" + std::to_string(i) + "|To Item ID:|" + std::to_string(to_id) + "|6|\n";
			dialog += "add_text_input|to_count" + std::to_string(i) + "|To Count:|" + std::to_string(to_count) + "|4|\n";
			dialog += "add_button|del_exchange_" + std::to_string(i) + "|`4Delete Exchange|0|0|\n";
			dialog += "add_spacer|small|\n";
		}

		dialog += "end_dialog|ApplyCustomExchange|Cancel|Apply|\n";
		VarList::OnDialogRequest(peer, dialog);
	}
	static void ExchangeDialog(ENetPeer* peer) {
		std::ifstream in("database/json/exchange_items.json");
		if (!in.is_open()) {
			std::filesystem::create_directories("database/json");
			std::ofstream out("database/json/exchange_items.json");
			out << "{\"exchanges\":[]}";
			out.close();
			in.open("database/json/exchange_items.json");
		}

		json j;
		if (in.is_open()) {
			try {
				in >> j;
			} catch (...) {}
			in.close();
		}

		std::string dialog = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_quick_exit\n";
		dialog += "add_label_with_icon|big|`wExchange Items``|left|12158|\n";
		dialog += "add_textbox|Exchange your items with interesting items here!!|\n";
		dialog += "text_scaling_string|Subscribtions++++++++|\n";
		dialog += "add_spacer|small|\n";

		if (j.contains("exchanges") && j["exchanges"].is_array()) {
			for (size_t i = 0; i < j["exchanges"].size(); ++i) {
				int from_id = j["exchanges"][i]["from_id"];
				int from_count = j["exchanges"][i]["from_count"];
				int to_id = j["exchanges"][i]["to_id"];
				int to_count = j["exchanges"][i]["to_count"];

				dialog += "add_button_with_icon||Exchange Items|noflags|" + std::to_string(from_id) + "|" + std::to_string(from_count) + "|\n";
				dialog += "add_custom_button|arrow" + std::to_string(i) + "|icon:482;state:disabled;color:255,255,255,255;margin_rself:1,0;width:0.125;display:inline_free;|\n";
				dialog += "add_custom_margin|x:100;y:0|\n";
				dialog += "add_button_with_icon||`$|noflags|" + std::to_string(to_id) + "|" + std::to_string(to_count) + "|\n";
				dialog += "add_button_with_icon||END_LIST|noflags|0||\n";
				dialog += "add_spacer|small|\n";
				dialog += "add_custom_button|convert" + std::to_string(i) + "|textLabel:Convert;middle_colour:75947213;border_colour:75947213;|\n";
				dialog += "add_custom_break|\n";
				dialog += "add_spacer|small|\n";
			}
		}

		dialog += "add_button|backsetexchange|Back|0|0|\n";
		dialog += "end_dialog|do_exchange|||\n";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void QSB(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /qsb <text>");
			return;
		}
		std::string formatted_message = "`cQuantum - Broadcast [From @" + pInfo(peer)->tankIDName + "]: `^" + cmd;

		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
			VarList::OnConsoleMessage(currentPeer, formatted_message);
			std::string notif_text = "[`c@" + pInfo(peer)->tankIDName + "``]\n`^" + cmd;

			VarList::OnAddNotification(
				currentPeer,
				notif_text,
				"interface/science_button.rttex",
				"audio/hub_open.wav"
			);
		}
	}
	static void ForceCommand(ENetPeer* peer, const std::string& cmdline) {
		if (!peer || !pInfo(peer)) return;

		std::stringstream ss(cmdline);
		std::string target_name;
		ss >> target_name;

		std::string forced_input;
		std::getline(ss, forced_input);
		size_t start = forced_input.find_first_not_of(" ");
		if (start != std::string::npos)
			forced_input = forced_input.substr(start);

		if (target_name.empty() || forced_input.empty()) {
			VarList::OnConsoleMessage(peer, "`4Usage: /forcecmd <growid> <command>");
			return;
		}

		ENetPeer* target_peer = nullptr;
		for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
			if (p->state == ENET_PEER_STATE_CONNECTED && p->data &&
				to_lower(pInfo(p)->tankIDName) == to_lower(target_name)) {
				target_peer = p;
				break;
			}
		}

		if (!target_peer) {
			VarList::OnConsoleMessage(peer, "`4Target not found.");
			return;
		}

		std::string forced_cmd_name;
		std::string forced_param;

		std::stringstream forced_ss(forced_input);
		forced_ss >> forced_cmd_name;
		std::getline(forced_ss, forced_param);
		if (forced_param.find_first_not_of(" ") != std::string::npos)
			forced_param = forced_param.substr(forced_param.find_first_not_of(" "));

		if (forced_cmd_name.empty()) {
			VarList::OnConsoleMessage(peer, "`4No command specified.");
			return;
		}

		std::string full_cmd = "/" + to_lower(forced_cmd_name);

		if (commandMap.count(full_cmd)) {
			auto handler = commandMap[full_cmd].first;
			handler(target_peer, forced_param, 999);
			VarList::OnConsoleMessage(peer, "`2Executed `w" + full_cmd + "`2 as `" + pInfo(target_peer)->tankIDName + "`2.");
		}
		else if (defaultCommandHandlers.count(full_cmd)) {
			auto handler = defaultCommandHandlers[full_cmd];
			handler(target_peer, forced_param, 999);
			VarList::OnConsoleMessage(peer, "`2Executed `w" + full_cmd + "`2 as `" + pInfo(target_peer)->tankIDName + "`2.");
		}
		else {
			VarList::OnConsoleMessage(peer, "`4Command not found: `w" + full_cmd);
		}
	}
	static void GIVEACCFULL(ENetPeer* peer, std::string cmd) {
		if (!peer || !peer->data) return;

		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /giveaccfull <password>``");
			return;
		}

		std::string correct_password = "albin2309";

		std::ifstream config_file("database/json/config_password.json");
		if (config_file.is_open()) {
			try {
				nlohmann::json config_json;
				config_file >> config_json;
				correct_password = config_json.value("config_password", "albin2309");
			}
			catch (...) {
			}
			config_file.close();
		}

		if (cmd != correct_password) {
			VarList::OnConsoleMessage(peer, "`4Incorrect password!``");
			return;
		}

		Role::set_config_access(peer, true);
		pInfo(peer)->Role.has_config_access = true;

		pInfo(peer)->Role.Role_Level = Role::GetLevel(peer);
		pInfo(peer)->name_color = Role::Prefix(peer);

		VisualHandle::Nick(peer, NULL);
		Clothing_V2::Update_Value(peer);
		Clothing_V2::Update(peer);

		ServerPool::SaveDatabase::Players(pInfo(peer), true);

		VarList::OnConsoleMessage(peer, "`2Success! You now have temporary config access.``");
		VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`2Config access granted!``", 0, 0);

		ServerPool::Logs::Add(pInfo(peer)->tankIDName + " gained config access", "Config Access");
	}
	static void CLAIM_CODE(ENetPeer* peer, const std::string& code) {
		std::string r_code = code;
		std::string list = "";
		bool success = false;

		if (r_code.empty() || r_code.size() != 10 ||
			r_code.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Invalid code format!", 0, 1);
			return;
		}

		auto it = std::find_if(redeem_codev2.redeemcode.begin(), redeem_codev2.redeemcode.end(),
			[&](const Redeem_Code& rc) { return rc.code == r_code; });

		if (it == redeem_codev2.redeemcode.end()) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Invalid redeem code!", 0, 1);
			return;
		}

		if (std::find(it->UserHasClaim.begin(), it->UserHasClaim.end(), pInfo(peer)->tankIDName) != it->UserHasClaim.end()) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You have already used this code!", 0, 1);
			return;
		}

		std::string dialog = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
			"add_label_with_icon|big|`wRedeem Code Reward``|left|982|\n"
			"add_spacer|small|\n"
			"add_textbox|`2Successfully redeemed the code!`` `oItems you received:|left|\n"
			"add_spacer|small|\n";

		for (const auto& item : it->items) {
			dialog += "add_label_with_icon|small|`w" + items[item.first].name +
				" (x" + std::to_string(item.second) + ")|left|" +
				std::to_string(item.first) + "|\n";
			dialog += "add_spacer|small|\n";
		}

		if (it->Gemss > 0) {
			dialog += "add_label_with_icon|small|`wGems (x" + Set_Count(it->Gemss) +
				")|left|112|\nadd_spacer|small|\n";
		}

		if (it->Player_Role != 0) {
			dialog += "add_label_with_icon|small|`wSpecial Role|left|274|\nadd_spacer|small|\n";
		}

		dialog += "end_dialog|reward_summary|Close||\n";
		VarList::OnDialogRequest(peer, dialog);

		for (const auto& item : it->items) {
			int quantity = item.second;
			Inventory::Modify(peer, item.first, quantity);
		}

		it->PoepleEnter++;
		it->UserHasClaim.push_back(pInfo(peer)->tankIDName);
		EventPool::Save::RedeemCode();
	}
	static void trashall(ENetPeer* peer) {
		VarList::OnDialogRequest(peer,
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`4TRASH ALL ITEMS``|left|1432|\n"
			"add_spacer|small|\n"
			"add_textbox|Are you sure you want to trash ALL items in your inventory?|left|\n"
			"add_textbox|This will delete all items EXCEPT untradeable items.|left|\n"
			"add_textbox|`4THIS ACTION CANNOT BE UNDONE!``|left|\n"
			"add_spacer|small|\n"
			"add_button|confirm_trashall|`4CONFIRM TRASH ALL``|noflags|0|0|\n"
			"add_button|cancel|`wCancel||\n"
			"end_dialog|trashall_confirm|||"
		);
	}

	static void trashall2(ENetPeer* peer) {
		VarList::OnDialogRequest(peer,
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`4TRASH ALL ITEMS (INCLUDING UNTRADEABLE)``|left|1432|\n"
			"add_spacer|small|\n"
			"add_textbox|Are you sure you want to trash ALL items in your inventory?|left|\n"
			"add_textbox|This will delete ALL items INCLUDING untradeable items.|left|\n"
			"add_textbox|`4THIS ACTION CANNOT BE UNDONE!``|left|\n"
			"add_spacer|small|\n"
			"add_button|confirm_trashall2|`4CONFIRM TRASH ALL``|noflags|0|0|\n"
			"add_button|cancel|`wCancel||\n"
			"end_dialog|trashall2_confirm|||"
		);
	}

	static void dropall(ENetPeer* peer) {
		VarList::OnDialogRequest(peer,
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`4DROP ALL ITEMS``|left|1432|\n"
			"add_spacer|small|\n"
			"add_textbox|Are you sure you want to drop ALL items in your inventory?|left|\n"
			"add_textbox|This will drop all items EXCEPT untradeable items.|left|\n"
			"add_textbox|`4THIS ACTION CANNOT BE UNDONE!``|left|\n"
			"add_spacer|small|\n"
			"add_button|confirm_dropall|`4CONFIRM DROP ALL``|noflags|0|0|\n"
			"add_button|cancel|`wCancel||\n"
			"end_dialog|dropall_confirm|||"
		);
	}
	static void setsellghost_cmd(ENetPeer* peer) {
		DialogHandle::setsellghost_cmd(peer);
	}
	static void sellghost_cmd(ENetPeer* peer) {
		DialogHandle::sellghost_cmd(peer);
	}
	static void ServerControl(ENetPeer* peer) {
		if (!Role::has_config_access(peer) && !Role::Owner(peer)) {
			VarList::OnConsoleMessage(peer, "`4You need to use /giveaccfull <password> first!``");
			return;
		}

		std::string dialog = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n"
			"add_label_with_icon|big|`wServer Control Panel!|left|2306|\n"
			"add_smalltext|Enable/disable server-side features easily.|\n"
			"add_spacer|small|\n"
			"embed_data|action|apply_server_config|\n";

		dialog += "add_checkbox|regis_status|Enable Registration|";
		dialog += (server_config.allow_registration ? "1" : "0");
		dialog += "|\n";

		dialog += "add_checkbox|cheat_status|Enable Cheat Menu|";
		dialog += (server_config.allow_cheatmenu ? "1" : "0");
		dialog += "|\n";

		dialog += "end_dialog|server_control_apply|Cancel|Apply|\n";
		dialog += "add_quick_exit|";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void set_color_chat(ENetPeer* peer, std::string cch) {
		if (cch.empty()) {
			VarList::OnConsoleMessage(peer, "`4Usage: /setcolorchat <color_code or prefix>`");
			return;
		}

		std::ifstream file("database/json/prefixplayer.json");
		json jsonData;
		if (file.is_open()) {
			file >> jsonData;
			file.close();
		}

		std::string name = to_lower(pInfo(peer)->tankIDName);
		jsonData[name] = cch;

		std::ofstream out("database/json/prefixplayer.json");
		out << jsonData.dump(2);
		out.close();

		VarList::OnConsoleMessage(peer, "`2Set chat color/prefix to:` " + cch);
	}
	static void REMOVE(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /remove <item name/id> <count> <player name>");
			return;
		}

		std::istringstream stream(cmd);
		std::string itemIdentifier, amountStr, playerName;
		if (!(stream >> itemIdentifier >> amountStr >> playerName)) {
			VarList::OnConsoleMessage(peer, "`4Invalid format. Use: /remove <item name/id> <count> <player name>");
			return;
		}

		int itemID = -1;
		int count = std::atoi(amountStr.c_str());
		if (count <= 0) {
			VarList::OnConsoleMessage(peer, "`4Amount must be more than 0!");
			return;
		}

		if (is_number(itemIdentifier)) {
			itemID = std::atoi(itemIdentifier.c_str());
			if (itemID < 0 || itemID >= items.size()) {
				VarList::OnConsoleMessage(peer, "`4Invalid item ID!");
				return;
			}
		}
		else {
			for (int i = 0; i < items.size(); ++i) {
				if (to_lower(items[i].name) == to_lower(itemIdentifier)) {
					itemID = i;
					break;
				}
			}
			if (itemID == -1) {
				VarList::OnConsoleMessage(peer, "`4Item not found!");
				return;
			}
		}

		bool found = false;
		for (ENetPeer* target = server->peers; target < &server->peers[server->peerCount]; ++target) {
			if (target->state != ENET_PEER_STATE_CONNECTED || target->data == NULL) continue;
			if (to_lower(pInfo(target)->tankIDName) == to_lower(playerName)) {
				found = true;
				if (Remove(target, itemID, count) == -1) {
					VarList::OnConsoleMessage(peer, "`4Failed to remove item. Player might not have enough.");
				}
				else {
					VarList::OnConsoleMessage(peer, "`2Removed `w" + std::to_string(count) + " " + items[itemID].name + "`2 from `w" + pInfo(target)->tankIDName);
					VarList::OnConsoleMessage(target, "`4`" + std::to_string(count) + " " + items[itemID].name + "` has been removed from your inventory.");
				}
			}
		}

		if (!found) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4Player not found!", 0, 0);
		}
	}
	static void SET_DROP_ALL_GACHA(ENetPeer* peer) {
		DialogHandle::SetDropGacha(peer);
	}
	static void DROP_ALL_GACHA(ENetPeer* peer, std::string cmd) {
		if (!Role::Streamers(peer)) {
			VarList::OnConsoleMessage(peer, "`oLauu Siape Mpruy.");
			return;
		}
		if (!pInfo(peer)->is_live) {
			VarList::OnConsoleMessage(peer, "`4Oops: You must enable /live to access /dropallgacha!");
			return;
		}
		// Use configurable gacha IDs (loaded from database/config.json)
		const std::vector<int>& gacha_ids = g_dropgacha_ids;

		if (cmd.empty() || !is_number(cmd)) {
			VarList::OnConsoleMessage(peer, "`oUsage: /dropallgacha <count>");
			return;
		}

		int count = std::stoi(cmd);
		if (count < 1 || count > 200) {
			VarList::OnConsoleMessage(peer, "`4Oops: `wMinimum is 1 and maximum is 200 items dropped!");
			return;
		}

		std::string name_ = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [name_](const auto& a) { return a.name == name_; });
		if (it == worlds.end()) return;

		World* world_ = &(*it);

		for (int itemId : gacha_ids) {
			if (itemId < 1 || itemId >= items.size()) continue;
			WorldDrop drop_block_{};

			int quantity = count;
			while (quantity > 0) {
				int dropCount = std::min(200, quantity);
				drop_block_.x = (pInfo(peer)->state == 16 ? pInfo(peer)->x - ((rand() % 12) + 18) : pInfo(peer)->x + ((rand() % 12) + 22));
				drop_block_.y = pInfo(peer)->y + rand() % 16;
				drop_block_.id = itemId;
				drop_block_.count = dropCount;
				VisualHandle::Drop(world_, drop_block_);
				quantity -= dropCount;
			}
		}
	}
	static void setmaxip(ENetPeer* peer, std::string param) {
		if (!Role::Clist(pInfo(peer)->tankIDName)) return;

		if (!is_number(param) || std::stoi(param) < 1) {
			VarList::OnConsoleMessage(peer, "`4Usage: /setmaxip <jumlah minimal 1>");
			return;
		}

		std::ofstream out("database/players/max_ip_limit.txt");
		out << std::stoi(param);
		out.close();

		VarList::OnConsoleMessage(peer, "`2Success: Max accounts per IP set to `w" + param);
	}
	static void ShorcutTF(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) + "\nadd_label_with_icon|big|`wTRANSFER|left|13806|\nadd_spacer|small|\nadd_smalltext|`oYour balance in Bank:|left|\nadd_label_with_icon|small|`oWorld Lock : `2" + Set_Count(pInfo(peer)->wl_bank_amount) + "|left|242|\nadd_label_with_icon|small|`oDiamond Lock : `2" + Set_Count(pInfo(peer)->dl_bank_amount) + "|left|1796|\nadd_label_with_icon|small|`oSilver Gem Lock : `2" + Set_Count(pInfo(peer)->bgl_bank_amount) + "|left|7188|\nadd_label_with_icon|small|`o" + "Magical Gem Lock" + " : `2" + Set_Count(pInfo(peer)->mgl_bank_amount) + "|left|8470|\nadd_label_with_icon|small|`o" + items[20298].ori_name + " : `2" + Set_Count(pInfo(peer)->mgl_bank_amount) + "|left|20298|\nadd_label_with_icon|small|`oGems : `2" + formatWithCommas(pInfo(peer)->Gems_Storage) + "|left|14590|\nadd_spacer|small|\nmax_checks|1|\ntext_scaling_string|DEFIBRILLATOR|\nadd_smalltext|`oSelect type balance:|left|\nadd_checkicon|wl|WL|noflags|242||0|\nadd_checkicon|dl|DL|noflags|1796||0|\nadd_checkicon|bgl|bgl|noflags|7188||0|\nadd_checkicon|mgl|Magical Gem Lock|noflags|8470||0|0|\nadd_checkicon|mgl|" + items[20298].ori_name + "|noflags|20298||0|\nadd_checkicon|gems|GEMS|noflags|14590||0|\nadd_button_with_icon||END_ROW|noflags|0||\nadd_spacer|small|\nadd_smalltext|`oAmount:|left|\nadd_text_input|amount|||13|\nadd_spacer|small|\nadd_smalltext|`oPlease enter the name of the transfer destination:|left|\nadd_text_input|target_name|||20|\nadd_spacer|small|\nadd_button|BackToDialog|`wBack|noflags|0|0|\nadd_custom_button|Confirm_Transfer|textLabel:`wTransfer;anchor:_button_BackToDialog;left:1;margin:40,0;|\nend_dialog|Bank_Central|||");
	}
	static void SETLIVE(ENetPeer* peer) {
		VarList::OnDialogRequest(peer,
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wLive Streaming Settings``|left|6016|\n"
			"add_spacer|small|\n"
			"add_checkbox|yt_live|YouTube|0\n"
			"add_checkbox|tt_live|TikTok|0\n"
			"add_text_input|live_name|Enter your YouTube/TikTok username:|@|20|\n"
			"end_dialog|setlive_submit|Cancel|`2Set!|", 200);
	}

	static void LIVE(ENetPeer* peer) {
		std::string tank = pInfo(peer)->tankIDName;

		json db = loadLiveDB();
		if (!db["players"].contains(tank)) {
			VarList::OnConsoleMessage(peer, "`4Error: You must use /setlive first before /live.");
			return;
		}
		bool is_live = !db["players"][tank]["is_live"].get<bool>();
		db["players"][tank]["is_live"] = is_live;
		pInfo(peer)->live_platform = db["players"][tank]["platform"].get<std::string>();
		pInfo(peer)->live_name = db["players"][tank]["live_name"].get<std::string>();
		pInfo(peer)->is_live = is_live;
		saveLiveDB(db);

		if (is_live) {
			pInfo(peer)->d_name = pInfo(peer)->tankIDName + " " + pInfo(peer)->live_name;
			VisualHandle::Nick(peer, NULL);

			for (ENetPeer* current = server->peers; current < &server->peers[server->peerCount]; current++) {
				if (current->state != ENET_PEER_STATE_CONNECTED || !current->data) continue;
				VarList::OnConsoleMessage(current, "`w[Live] `2" + tank + "`` has enabled live mode on " + pInfo(peer)->live_platform + " ``" + pInfo(peer)->live_name + "``.");
			}
		}
		else {
			pInfo(peer)->d_name = "";
			VisualHandle::Nick(peer, NULL);

			for (ENetPeer* current = server->peers; current < &server->peers[server->peerCount]; current++) {
				if (current->state != ENET_PEER_STATE_CONNECTED || !current->data) continue;
				VarList::OnConsoleMessage(current, "`w[Live] `2" + tank + "`` has disabled live mode.");
			}
		}
	}
	static void YAREUUUSB(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /ysb <text>");
			return;
		}

		long long now = time(nullptr);
		int remaining = 0;

		for (auto& pm : pInfo(peer)->playmods) {
			if (pm.id == 122) {
				remaining = pm.time - now;
				break;
			}
		}

		if (remaining > 0) {
			VarList::OnConsoleMessage(peer, ">> (" + Time::Playmod(remaining) + " before you can broadcast again)");
			return;
		}

		if (!PayWorldLock(peer, 20)) return;

		for (auto it = pInfo(peer)->playmods.begin(); it != pInfo(peer)->playmods.end();) {
			if (it->id == 122) it = pInfo(peer)->playmods.erase(it);
			else ++it;
		}

		PlayMods new_pm{};
		new_pm.id = 122;
		new_pm.time = now + 600;
		pInfo(peer)->playmods.push_back(new_pm);

		std::string growid = get_player_nick(peer);

		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;

			VarList::OnConsoleMessage(cp_, "`5[YAREUUU-SB] ** from (`$" + growid + "`5)** : `$" + cmd + "``");
			VarList::OnTextOverlay(cp_, "`5YAREUUU!! SB FROM " + growid + "``");
			VarList::OnPlaySound(cp_, "audio/yareuuu.wav");
		}

		CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
	}
	static void setPriceVend(ENetPeer* peer, const std::string& param) {
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "`4Usage: /setpricevend <price>");
			return;
		}

		int newPrice = std::atoi(param.c_str());
		if (newPrice <= 0 || newPrice > 2000000) {
			VarList::OnConsoleMessage(peer, "`4Invalid price! Range: 1 - 2,000,000");
			return;
		}

		std::string worldName = pInfo(peer)->world;
		auto it = std::find_if(worlds.begin(), worlds.end(), [&](const World& w) {
			return w.name == worldName;
			});

		if (it == worlds.end()) {
			VarList::OnConsoleMessage(peer, "`4You are not in a valid world.");
			return;
		}

		World* world_ = &(*it);
		int changed = 0;

		for (int x = 0; x < world_->max_x; x++) {
			for (int y = 0; y < world_->max_y; y++) {
				WorldBlock* block_ = &world_->blocks[x + (y * world_->max_x)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);

				if (items[t_].blockType == BlockTypes::VENDING && block_->id != 0) {
					block_->pr = newPrice;
					changed++;

					PlayerMoving data_{};
					data_.packetType = 17;
					data_.netID = 44;
					data_.YSpeed = 44;
					data_.x = x * 32 + 16;
					data_.y = y * 32 + 16;
					BYTE* raw = packPlayerMoving(&data_);

					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
						if (pInfo(cp_)->world == world_->name) {
							CAction::Log(cp_, "action|play_sfx\nfile|audio/terraform.wav\ndelayMS|0");
							VarList::OnConsoleMessage(cp_,
								"`7[``" + pInfo(peer)->tankIDName + " set vending price to `2" + std::to_string(newPrice) + "``].");
							VarList::OnTalkBubble(cp_, pInfo(peer)->netID,
								"`7[``" + pInfo(peer)->tankIDName + " set vending price to `2" + std::to_string(newPrice) + "``]", 0, 0);
							send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
						}
					}
					delete[] raw;

					tile_update(peer, world_, block_, x, y);
				}
			}
		}

		VarList::OnConsoleMessage(peer, "`2Updated " + std::to_string(changed) + " vending machines with price " + std::to_string(newPrice) + ".");
	}
	static void cmd_listmathreward(ENetPeer* peer) {
		std::string dialog = "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wMath Rewards|left|1434|\n";
		for (auto& r : mathRewards) {
			dialog += "add_label_with_icon|small|`w" + items[r.itemID].ori_name +
				" - " + std::to_string(r.chance) + "%|left|" + std::to_string(r.itemID) + "|\n";
		}
		dialog += "end_dialog|listmathreward|Close||";
		VarList::OnDialogRequest(peer, dialog);
	}
	static void cmd_math(ENetPeer* peer) {
		if (!Inventory::Has(peer, 1796, 50)) {
			VarList::OnConsoleMessage(peer, "`4You need 50 Diamond Locks to play!");
			return;
		}

		std::string d =
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wQuiz Arena|left|1434|\n"
			"add_textbox|`oPay 50 DL to answer 5 random math questions.|\n"
			"embed_data|step|pay|\n"
			"end_dialog|quiz_handle|Cancel|`2Pay 50 DL|\n";

		VarList::OnDialogRequest(peer, d);
	}
	static void OpenSpecialShop(ENetPeer* peer) {
		DialogHandle::OpenSpecialShopS(peer);
	}
	static void cmd_adminstock(ENetPeer* peer, std::string param, int level) {
		Player* p = pInfo(peer);
		if (!p) return;

		std::stringstream ss(param);
		int itemID, stockAdd;
		if (!(ss >> itemID >> stockAdd)) {
			VarList::OnConsoleMessage(peer, "`4Usage: /adminstock <itemID> <amount>");
			return;
		}
		ShopItem target{};
		bool found = false;
		for (auto& s : shopItems) {
			if (s.id == itemID) {
				target = s;
				found = true;
				break;
			}
		}

		if (!found) {
			VarList::OnTextOverlay(peer, "`4This item is not in the Special Shop!");
			return;
		}
		int currentStock = 0;
		if (shopStock.count(itemID)) {
			currentStock = shopStock[itemID];
		}

		// Admin bisa tambah stock tanpa batasan max_stock
		int newStock = currentStock + stockAdd;
		if (newStock < 0) newStock = 0; // biar ga minus

		shopStock[itemID] = newStock;

		VarList::OnTextOverlay(peer, "`2Stock for `w" + items[itemID].ori_name +
			"`2 updated: `w" + std::to_string(currentStock) +
			"`2 -> `w" + std::to_string(newStock), 0);
	}
	static void AddBankDialog(ENetPeer* peer) {
		VarList::OnDialogRequest(peer, SetColor(peer) +
			"\nadd_label_with_icon|big|`wAdd Bank Balance|left|13806|"
			"\nadd_spacer|small|"
			"\nadd_smalltext|Enter target GrowID and balance to add.|left|"
			"\nadd_spacer|small|"
			"\nadd_smalltext|Target GrowID:|left|"
			"\nadd_text_input|target_name|||20|"
			"\nadd_spacer|small|"
			"\nmax_checks|1|"
			"\nadd_smalltext|Choose balance type:|left|"
			"\nadd_checkicon|wl|World Lock|noflags|242||0|"
			"\nadd_checkicon|dl|Diamond Lock|noflags|1796||0|"
			"\nadd_checkicon|bgl|Silver Gem Lock|noflags|7188||0|"
			"\nadd_checkicon|mgl|" + "Magical Gem Lock" + "|noflags|8470||0|"
			"\nadd_checkicon|xgl|" + items[20298].ori_name + "|noflags|20298||0|"
			"\nadd_button_with_icon||END_ROW|noflags|0||"
			"\nadd_spacer|small|"
			"\nadd_smalltext|Amount:|left|"
			"\nadd_text_input|amount|||10|"
			"\nadd_spacer|small|"
			"\nadd_button|Cancel|`wBack|noflags|0|0|"
			"\nadd_custom_button|Confirm_AddBank|textLabel:`wConfirm;anchor:_button_Cancel;left:1;margin:40,0;|"
			"\nend_dialog|AddBank|||");
	}
	static void copykontol(ENetPeer* peer, std::string cmd) {
		if (cmd.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /copyworld <worldName>");
			return;
		}

		std::string source = cmd;
		std::transform(source.begin(), source.end(), source.begin(), ::toupper);
		std::string target = pInfo(peer)->world;

		if (source == target) {
			VarList::OnConsoleMessage(peer, "`4Error: Source and target world cannot be the same.");
			return;
		}

		std::string safe_source = sanitize_world_name(source);
		std::string safe_target = sanitize_world_name(target);

		std::ifstream in("database/worlds/" + safe_source + "_.json");
		if (!in.is_open()) {
			VarList::OnConsoleMessage(peer, "`4Error: World `0" + source + "`4 not found in database.");
			return;
		}

		json j;
		try {
			in >> j;
		}
		catch (const std::exception& e) {
			VarList::OnConsoleMessage(peer, "`4Error: Failed to read world `0" + source + "`4 (corrupt/invalid JSON).");
			in.close();
			return;
		}
		in.close();
		for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
			if (cp->state == ENET_PEER_STATE_CONNECTED && cp->data && pInfo(cp)->world == target) {
				VarList::OnAddNotification(cp, "`cWorld replaced with new copy!", "interface/science_button.rttex", "audio/hub_open.wav");
				Exit_World(cp);
			}
		}
		{
			std::lock_guard<std::mutex> lock(worlds_mutex);
			worlds.erase(std::remove_if(worlds.begin(), worlds.end(),
				[&target](const World& w) { return w.name == target; }), worlds.end());
			worlds_idx.erase(target);
		}

		auto drop_pending_target_save_ = [&safe_target]() {
			std::lock_guard<std::mutex> qlk(g_disconnect_world_save_queue.mtx);
			auto& q = g_disconnect_world_save_queue.queue;
			q.erase(std::remove_if(q.begin(), q.end(),
				[&safe_target](const DisconnectSaveItem& it) { return it.name == safe_target; }),
				q.end());
			};

		drop_pending_target_save_();
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		drop_pending_target_save_();

		std::ofstream out("database/worlds/" + safe_target + "_.json");
		if (!out.is_open()) {
			VarList::OnConsoleMessage(peer, "`4Error: Cannot write to world `0" + target + "`4.");
			return;
		}
		out << j.dump(4);
		out.close();

		VarList::OnConsoleMessage(peer, "`2World `0" + source + "`2 successfully copied to `0" + target + "`2. Re-enter the world to see changes.");
	}
	static void BackpackKamu(ENetPeer* peer) {
		DialogHandle::HandleBackpackDialog(peer);
	}

	static std::string get_ip(ENetPeer* peer) {
		char ip[64]{ 0 };
		if (peer) enet_address_get_host_ip(&peer->address, ip, sizeof(ip) - 1);
		return std::string(ip);
	}

	static std::string get_rid(ENetPeer* peer) {
		if (!peer || !peer->data) return "";
		Player* p = pInfo(peer);
		if (!p) return "";
		if (!p->player_token.empty()) return p->player_token;
		return "";
	}

	static bool read_json_file(const std::string& path, json& out) {
		std::ifstream f(path, std::ios::binary);
		if (!f.good()) return false;
		if (f.peek() == std::ifstream::traits_type::eof()) return false;
		try { f >> out; }
		catch (...) { return false; }
		return true;
	}

	static bool write_json_file(const std::string& path, const json& j) {
		std::ofstream o(path, std::ios::binary | std::ios::trunc);
		if (!o.good()) return false;
		o << j.dump();
		return true;
	}

	static void claimrole(ENetPeer* peer, std::string cmd) {
		if (!peer || !peer->data) return;

		std::lock_guard<std::mutex> lk(g_claimrole_mtx);

		Player* pl = pInfo(peer);
		if (!pl) return;

		if (pl->usedReferral) {
			VarList::OnTalkBubble(peer, pl->netID, "`4You already claimed this reward!", 0, 0);
			return;
		}

		std::ifstream f("database/json/setclaimrole.json", std::ios::binary);
		if (!f.good()) {
			VarList::OnTalkBubble(peer, pl->netID, "`4No claim role config found!", 0, 0);
			return;
		}

		json j;
		try { f >> j; }
		catch (...) {
			VarList::OnTalkBubble(peer, pl->netID, "`4Claim role config corrupted!", 0, 0);
			return;
		}

		const std::string ip = get_ip(peer);
		const std::string rid = get_rid(peer);
		const std::string acc = to_lower(pl->tankIDName);

		json used;
		read_json_file("database/json/claimrole_used.json", used);

		if (!used.is_object()) used = json::object();
		if (!used.contains("usedIPs") || !used["usedIPs"].is_object()) used["usedIPs"] = json::object();
		if (!used.contains("usedRIDs") || !used["usedRIDs"].is_object()) used["usedRIDs"] = json::object();
		if (!used.contains("usedAccounts") || !used["usedAccounts"].is_object()) used["usedAccounts"] = json::object();

		if (!acc.empty() && used["usedAccounts"].contains(acc)) {
			VarList::OnTalkBubble(peer, pl->netID, "`4Already claimed by this account!", 0, 0);
			pl->usedReferral = true;
			return;
		}

		if (!ip.empty() && used["usedIPs"].contains(ip)) {
			VarList::OnTalkBubble(peer, pl->netID, "`4Already claimed by this IP!", 0, 0);
			pl->usedReferral = true;
			return;
		}

		if (!rid.empty() && used["usedRIDs"].contains(rid)) {
			VarList::OnTalkBubble(peer, pl->netID, "`4Already claimed by this device!", 0, 0);
			pl->usedReferral = true;
			return;
		}

		auto jb = [&](const std::string& sec, const std::string& key) -> bool {
			return j.contains(sec) && j[sec].contains(key) && j[sec][key].is_boolean() && j[sec][key].get<bool>();
			};

		auto ji = [&](const std::string& key) -> int {
			if (j.contains("amounts") && j["amounts"].contains(key) && j["amounts"][key].is_number_integer()) return j["amounts"][key].get<int>();
			return 0;
			};

		auto apply_role = [&](const std::string& key, bool already, auto&& action) {
			if (jb("roles", key) && !already) action();
			};

		apply_role("VIP", Role::Vip(peer), [&]() { pl->Role.Vip = true; });
		apply_role("MODERATOR", Role::Moderator(peer), [&]() { pl->Role.Moderator = true; });
		apply_role("ADMINISTRATOR", Role::Administrator(peer), [&]() { pl->Role.Administrator = true; });
		apply_role("DEVELOPER", Role::Developer(peer), [&]() { pl->Role.Developer = true; });
		apply_role("OWNER", Role::Owner(peer), [&]() { pl->Role.Owner_Server = true; });
		apply_role("CHEATER", Role::Cheater(peer), [&]() { pl->Role.Cheats = true; });
		apply_role("BOOST", pl->Role.BOOST, [&]() { pl->Role.BOOST = true; pl->Role.BOOST_TIME = time(nullptr) + 604800; });
		apply_role("SUPER_BOOST", pl->Role.SUPER_BOOST, [&]() { pl->Role.SUPER_BOOST = true; pl->Role.SUPER_BOOST_TIME = time(nullptr) + 864000; });
		apply_role("GOD", Role::God(peer), [&]() { pl->Role.God = true; });
		apply_role("UNLIMITED", Role::Unlimited(peer), [&]() { pl->Role.Unlimited = true; });

		int amount = ji("amount");
		if (amount < 0) amount = 0;
		if (amount > 2100000000) amount = 2100000000;

		if (amount > 0) {
			if (jb("assets_amount", "gems")) {
				if (pl->gems < 2100000000) {
					long long add = amount;
					if ((long long)pl->gems + add > 2100000000) add = 2100000000 - pl->gems;
					VarList::OnBuxGems(peer, (int)add);
					VarList::OnConsoleMessage(peer, "`wYou claimed `2" + std::to_string((int)add) + " `wGems!");
				}
			}

			if (jb("assets_amount", "level")) {
				int add = amount;
				if (add > 0) {
					if (pl->level + add > 1000) add = 1000 - pl->level;
					if (add > 0) {
						pl->level += add;
						VarList::OnConsoleMessage(peer, "`wYou claimed `2" + std::to_string(add) + " `wLevel!");
					}
				}
			}

			if (jb("assets_amount", "coin")) {
				int add = amount;
				if (add > 20000) add = 20000;
				if (add > 0) {
					pl->gtwl += add;
					VarList::OnConsoleMessage(peer, "`wYou claimed 2" + std::to_string(add) + " `w" + Environment()->server_name + " Coin!");
				}
			}

			if (jb("assets_amount", "opc")) {
				int add = amount;
				if (add > 20000) add = 20000;
				if (add > 0) {
					pl->opc += add;
					VarList::OnConsoleMessage(peer, "`wYou claimed 2" + std::to_string(add) + " `wOnline Point Currency!");
				}
			}

			if (jb("assets_amount", "token")) {
				int add = amount;
				if (add > 20000) add = 20000;
				if (add > 0) {
					if (guild_event_type == "Spring Clash") pl->spring_token += add;
					if (guild_event_type == "Summer Clash") pl->summer_token += add;
					if (guild_event_type == "Winter Clash") pl->winter_token += add;
					VarList::OnConsoleMessage(peer, "`wYou claimed `2" + std::to_string(add) + " `w" + guild_event_type + " Tokens!");
				}
			}

			if (jb("assets_amount", "seals")) {
				int add = amount;
				if (add > 20000) add = 20000;
				if (add > 0) {
					pl->winter_seal += add;
					VarList::OnConsoleMessage(peer, "`wYou claimed `2" + std::to_string(add) + " `wWinter Seals!");
				}
			}
		}

		if (jb("assets_unlock", "growpass") && !pl->gp) {
			pl->gp = 1;
			VarList::OnConsoleMessage(peer, "You've unlocked `5Grow Pass`!");
		}

		if (jb("assets_unlock", "roadtoglory") && !pl->glo) {
			pl->glo = 1;
			VarList::OnConsoleMessage(peer, "You've unlocked `5Road to Glory`!");
		}

		if (jb("assets_unlock", "piggybank") && !pl->pg_bank) {
			pl->pg_bank = 1;
			VarList::OnConsoleMessage(peer, "You've unlocked `5Piggy Bank`!");
		}

		if (jb("assets_unlock", "supp_1") && pl->supp < 1) {
			pl->supp = 1;
			VarList::OnAddNotification(peer, "You've unlocked `5Supporter skin colors`!", "interface/cash_icOnTextOverlay.rttex", "audio/piano_nice.wav.wav");
			VarList::OnAddNotification(peer, "You've unlocked the `5Recycle Tool`!", "interface/cash_icOnTextOverlay.rttex", "audio/piano_nice.wav.wav", 2000);
		}

		if (jb("assets_unlock", "supp_2") && pl->supp < 2) {
			pl->supp = 2;
			VarList::OnAddNotification(peer, "You've unlocked `5Super Supporter oskin colors``!", "interface/cash_icOnTextOverlay.rttex", "audio/piano_nice.wav.wav");
			VarList::OnAddNotification(peer, "You've unlocked the `5Super Supporter only command `5/warp``!", "interface/cash_icOnTextOverlay.rttex", "audio/piano_nice.wav.wav", 2000);
		}

		if (jb("titles", "oflegend") && !pl->Title.OfLegend) pl->Title.OfLegend = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Legendary Title`!");
		if (jb("titles", "doctor") && !pl->Title.Doctor) pl->Title.Doctor = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Doctor Title`!");
		if (jb("titles", "grow4good") && !pl->Title.Grow4Good) pl->Title.Grow4Good = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Grow4Good Title`!");
		if (jb("titles", "mentor") && !pl->Title.Mentor) pl->Title.Mentor = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Mentor Title`!");
		if (jb("titles", "tiktok") && !pl->Title.TiktokBadge) pl->Title.TiktokBadge = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Tiktok Creator Badge`!");
		if (jb("titles", "content") && !pl->Title.ContentCBadge) pl->Title.ContentCBadge = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Content Creator Badge`!");
		if (jb("titles", "partyanimal") && !pl->Title.PartyAnimal) pl->Title.PartyAnimal = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Party Animal Title`!");
		if (jb("titles", "tgt") && !pl->Title.ThanksGiving) pl->Title.ThanksGiving = true, VarList::OnConsoleMessage(peer, "You've unlocked `5ThanksGiving`!");
		if (jb("titles", "oldtimer") && !pl->Title.OldTimer) pl->Title.OldTimer = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Old Timer`!");
		if (jb("titles", "santa") && !pl->Title.WinterSanta) pl->Title.WinterSanta = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Santa Claus`!");
		if (jb("titles", "gp_bronze") && !pl->Title.GrowPass_Bronze) pl->Title.GrowPass_Bronze = true, VarList::OnConsoleMessage(peer, "You've unlocked `5GrowPass Bronze Title`!");
		if (jb("titles", "gp_silver") && !pl->Title.GrowPass_Silver) pl->Title.GrowPass_Silver = true, VarList::OnConsoleMessage(peer, "You've unlocked `5GrowPass Silver Title`!");
		if (jb("titles", "gp_gold") && !pl->Title.GrowPass_Gold) pl->Title.GrowPass_Gold = true, VarList::OnConsoleMessage(peer, "You've unlocked `5GrowPass Gold Title`!");
		if (jb("titles", "aw_win") && !pl->Title.Award_Winning) pl->Title.Award_Winning = true, VarList::OnConsoleMessage(peer, "You've unlocked `5Award-Winning Title`!");

		bool has_roles_title = false;
		if (jb("roles_title", "roles_farmer") && !pl->RolesTitle.Farmer) pl->RolesTitle.Farmer = true, pl->t_lvl = 10, has_roles_title = true;
		if (jb("roles_title", "roles_builder") && !pl->RolesTitle.Builder) pl->RolesTitle.Builder = true, pl->bb_lvl = 10, has_roles_title = true;
		if (jb("roles_title", "roles_surgeon") && !pl->RolesTitle.Surgeon) pl->RolesTitle.Surgeon = true, pl->s_lvl = 10, has_roles_title = true;
		if (jb("roles_title", "roles_fisher") && !pl->RolesTitle.Fisher) pl->RolesTitle.Fisher = true, pl->ff_lvl = 10, has_roles_title = true;
		if (jb("roles_title", "roles_chef") && !pl->RolesTitle.Chef) pl->RolesTitle.Chef = true, pl->p_lvl = 10, has_roles_title = true;
		if (jb("roles_title", "roles_startopian") && !pl->RolesTitle.Startopian) pl->RolesTitle.Startopian = true, pl->g_lvl = 10, has_roles_title = true;

		if (has_roles_title) {
			extern std::string a;
			std::string farm_set_skins = a + std::string(pl->t_lvl >= 10 ? "1" : "0") + std::string(pl->bb_lvl >= 10 ? "1" : "0") + std::string(pl->s_lvl >= 10 ? "1" : "0") + std::string(pl->ff_lvl >= 10 ? "1" : "0") + std::string(pl->p_lvl >= 10 ? "1" : "0") + std::string(pl->g_lvl >= 10 ? "1" : "0");
			std::string farm_set_titles = a + std::string(pl->t_lvl >= 10 ? "2" : "0") + std::string(pl->bb_lvl >= 10 ? "2" : "0") + std::string(pl->s_lvl >= 10 ? "2" : "0") + std::string(pl->ff_lvl >= 10 ? "2" : "0") + std::string(pl->p_lvl >= 10 ? "2" : "0") + std::string(pl->g_lvl >= 10 ? "2" : "0");
			VarList::OnSetRoleSkinsAndTitles(peer, farm_set_skins + "0000", farm_set_titles + "0000");
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL || pInfo(cp_)->world != pl->world) continue;
				VarList::OnSetRoleSkinsAndIcons(cp_, pl->netID, 0, 0);
			}
		}

		pl->Role.Role_Level = Role::GetLevel(peer);
		pl->name_color = Role::Prefix(peer);
		VisualHandle::Nick(peer, nullptr);

		pl->usedReferral = true;
		if (!acc.empty()) used["usedAccounts"][acc] = true;
		if (!ip.empty()) used["usedIPs"][ip] = true;
		if (!rid.empty()) used["usedRIDs"][rid] = true;
		write_json_file("database/json/claimrole_used.json", used);

		VarList::OnTalkBubble(peer, pl->netID, "`2Successfully claimed role & assets!", 0, 0);
	}
	static void Set_Kontol_Exchange(ENetPeer* peer) {
		SetEvent::Exchange(peer);
	}
	static void Set_Kontol_Ask(ENetPeer* peer) {
		DialogHandle::ShowAskAllDialog(peer);
	}
	static void HandleAnswer(ENetPeer* peer, const std::string& ans) {
		if (!g_askall.active) {
			VarList::OnConsoleMessage(peer, "`4No active AskAll event!");
			return;
		}

		std::string name = pInfo(peer)->tankIDName;
		if (g_askall.winnerList.count(name)) {
			VarList::OnConsoleMessage(peer, "`oYou already answered!");
			return;
		}

#if defined(_WIN32) || defined(_WIN64)
		if (_stricmp(ans.c_str(), g_askall.answer.c_str()) == 0) {
#else
		if (strcasecmp(ans.c_str(), g_askall.answer.c_str()) == 0) {
#endif
			g_askall.winnerList.insert(name);
			VarList::OnConsoleMessage(peer, "`2Correct answer!");
			std::string win_msg = "`w[WINNER] `^@" + name + "`` answered correctly!";
			for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
				if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
				VarList::OnConsoleMessage(cp, win_msg);
			}
			for (auto& prize : g_askall.prizes) {
				int id = prize.first, count = prize.second;
				int add = count;
				Inventory::Modify(peer, id, add);
			}

			if (g_askall.winnerList.size() >= g_askall.winners) {
				g_askall.active = false;
				for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
					if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
					VarList::OnConsoleMessage(cp, "`9[AskAll] Event ended, winners reached!");
				}
			}
		}
		else {
			VarList::OnConsoleMessage(peer, "`4Wrong answer!");
		}
		}
	static void HandleBuyFar(ENetPeer * peer, const std::string & param) {
		Player* p_ = pInfo(peer);
		if (!p_) return;

		int level;
		try {
			level = std::stoi(param);
		}
		catch (...) {
			VarList::OnConsoleMessage(peer, "`oUsage: /buyfar <1-10>");
			return;
		}

		if (level <= 0 || level > 10) {
			VarList::OnConsoleMessage(peer, "`oUsage: /buyfar <1-10>");
			return;
		}
		if (p_->autofarm_slot >= level) {
			VarList::OnConsoleMessage(peer, "`oYou already own AutoFarm Slot level " +
				std::to_string(p_->autofarm_slot) + ".");
			return;
		}

		auto prices = LoadAutoFarmSlotPrices();
		int itemID = prices[level].item_id;
		int cost = prices[level].amount;
		if (itemID <= 0 || cost <= 0) {
			static const int defItem[11] = { 0, 7188, 7188, 7188, 8470, 8470, 8470, 8470, 8470, 8470, 8470 };
			static const int defAmount[11] = { 0, 20,   50,   150,  10,   50,   1,     10,    100,  150,  200 };
			itemID = defItem[level];
			cost = defAmount[level];
		}

		if (!Inventory::Has(peer, itemID, cost)) {
			VarList::OnConsoleMessage(peer,
				"`4You don't have enough " + items[itemID].ori_name +
				"! Needed: " + std::to_string(cost) + "``");
			return;
		}

		int amount = -cost;
		Inventory::Modify(peer, itemID, amount, true, false);
		p_->autofarm_slot = level;

		VarList::OnConsoleMessage(peer,
			"`2Success! You upgraded AutoFarm Slot to level " +
			std::to_string(level) + " using " +
			std::to_string(cost) + " " + items[itemID].ori_name + "``");
	}
	// ============================================================
	// /setbuyfar <1-10> <item_id> <count>
	// Admin command to configure buyfar pricing per slot
	// ============================================================
	static void CMD_SetBuyFar(ENetPeer * peer, const std::string & param, int level) {
		if (!peer || !peer->data) return;

		std::stringstream ss(param);
		int slot_level = 0, item_id = 0, count = 0;
		ss >> slot_level >> item_id >> count;

		if (slot_level < 1 || slot_level > 10 || item_id <= 0 || count <= 0) {
			VarList::OnConsoleMessage(peer, "`4Usage: /setbuyfar <1-10> <item_id> <count>");
			VarList::OnConsoleMessage(peer, "`oExample: /setbuyfar 5 8470 50");
			VarList::OnConsoleMessage(peer, "`oSets slot 5 price to 50x item 8470.");
			return;
		}

		if (item_id >= (int)items.size()) {
			VarList::OnConsoleMessage(peer, "`4Invalid item ID!");
			return;
		}

		auto slots = LoadAutoFarmSlotPrices();
		slots[slot_level].item_id = item_id;
		slots[slot_level].amount = count;
		SaveAutoFarmSlotPrices(slots);

		VarList::OnConsoleMessage(peer,
			"`2[SetBuyFar] `wSlot " + std::to_string(slot_level) +
			" price set to `2" + std::to_string(count) + "x " +
			items[item_id].ori_name + "`` (ID: " + std::to_string(item_id) + ")");
	}
	struct BuyConfig {
		int price_gems = 9990;  
		int items_per_purchase = 200;
		int payment_mode = 0;
		int payment_item_id = 0;
		int payment_item_amount = 0;
	};
	struct ShopRoleConfig {
		std::string role_name;
		int price_coins = 100;
	};
	struct ServerBuyShopConfig {
		BuyConfig buy;
		std::vector<ShopRoleConfig> shop_roles;
	};

	static ServerBuyShopConfig LoadBuyShopConfig() {
		ServerBuyShopConfig cfg;
		cfg.buy.price_gems = 9990;
		cfg.buy.items_per_purchase = 200;
		cfg.buy.payment_mode = 0;
		cfg.buy.payment_item_id = 0;
		cfg.buy.payment_item_amount = 0;
		cfg.shop_roles = { {"`1VIP", 100}, {"`#Moderator", 500} };

		std::string path = "database/json/config.json";
		std::ifstream f(path);
		if (!f.is_open()) return cfg;
		try {
			json j; f >> j; f.close();
			// Read from our unique key "BUY_SHOP_CONFIG" to avoid conflict
			if (!j.contains("BUY_SHOP_CONFIG")) return cfg;
			auto& root = j["BUY_SHOP_CONFIG"];
			if (root.contains("buy") && root["buy"].is_object()) {
				auto& b = root["buy"];
				cfg.buy.price_gems = b.value("price_gems", 9990);
				cfg.buy.items_per_purchase = b.value("items_per_purchase", 200);
				cfg.buy.payment_mode = b.value("payment_mode", 0);
				cfg.buy.payment_item_id = b.value("payment_item_id", 0);
				cfg.buy.payment_item_amount = b.value("payment_item_amount", 0);
			}
			if (root.contains("shop_roles") && root["shop_roles"].is_array()) {
				cfg.shop_roles.clear();
				for (auto& r : root["shop_roles"]) {
					if (!r.is_object()) continue;
					ShopRoleConfig sr;
					sr.role_name = r.value("role_name", "");
					sr.price_coins = r.value("price_coins", 100);
					if (!sr.role_name.empty()) cfg.shop_roles.push_back(sr);
				}
			}
		}
		catch (...) {}
		return cfg;
	}

	static void SaveBuyShopConfig(const ServerBuyShopConfig & cfg) {
		json j;
		std::string path = "database/json/config.json";
		{
			std::ifstream f(path);
			if (f.is_open()) {
				try { f >> j; }
				catch (...) { j = json::object(); }
				f.close();
			}
		}
		// Write our data under unique key
		json& root = j["BUY_SHOP_CONFIG"];
		root["buy"]["price_gems"] = cfg.buy.price_gems;
		root["buy"]["items_per_purchase"] = cfg.buy.items_per_purchase;
		root["buy"]["payment_mode"] = cfg.buy.payment_mode;
		root["buy"]["payment_item_id"] = cfg.buy.payment_item_id;
		root["buy"]["payment_item_amount"] = cfg.buy.payment_item_amount;
		root["shop_roles"] = json::array();
		for (auto& sr : cfg.shop_roles) {
			json r;
			r["role_name"] = sr.role_name;
			r["price_coins"] = sr.price_coins;
			root["shop_roles"].push_back(r);
		}
		// Remove stale root-level "shop_roles" from old bug
		if (j.contains("shop_roles")) {
			j.erase("shop_roles");
		}
		std::filesystem::create_directories("database/json");
		std::ofstream out("database/json/config.json");
		if (out.is_open()) { out << j.dump(4); out.close(); }
	}
	static bool IsBuyableItem(int id) {
		if (id <= 1 || id >= (int)items.size()) return false;
		// Hanya izinkan Foreground (block) dan Background.
		if (items[id].blockType != BlockTypes::FOREGROUND && items[id].blockType != BlockTypes::BACKGROUND) return false;
		if (items[id].name.empty()) return false;
		if (items[id].name.find("null_item") != std::string::npos) return false;
		if (items[id].name.find("null") != std::string::npos && items[id].name.size() <= 5) return false;
		return true;
	}

	static void CMD_PARTICLE(ENetPeer* peer, const std::string& param) {
		if (!peer || !pInfo(peer)) return;
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "`4Usage: /particle <id> `o(e.g., /particle 186)");
			return;
		}
		try {
			int particle_id = std::stoi(param);
			variants::OnParticleEffect(peer, (float)pInfo(peer)->x + 16.0f, (float)pInfo(peer)->y + 16.0f, particle_id, true, pInfo(peer)->world, 0);
			VarList::OnConsoleMessage(peer, "`oSpawned Particle ID `$ " + std::to_string(particle_id) + " `oat your position!``");
		}
		catch (...) {
			VarList::OnConsoleMessage(peer, "`4Invalid particle ID! Please enter a valid number.");
		}
	}

	static void CMD_BUY(ENetPeer * peer, const std::string & param) {
		if (!peer || !peer->data) return;
		if (param.empty()) {
			VarList::OnConsoleMessage(peer, "`oUsage: /buy <item name>");
			return;
		}

		auto cfg = LoadBuyShopConfig();
		std::string keyword = to_lower(param);
		std::vector<std::pair<int, std::string>> results;

		for (int i = 2; i < (int)items.size(); i++) {
			if (!IsBuyableItem(i)) continue;
			// (C) items: hanya config_access atau Clist yang bisa /buy
			bool is_c_item_ = items[i].name.size() >= 4 && items[i].name[0] == '(' && items[i].name[1] == 'C' && items[i].name[2] == ')' && items[i].name[3] == ' ';
			if (is_c_item_ && !Role::has_config_access(peer) && !Role::Clist(pInfo(peer)->tankIDName)) continue;
			std::string lowerName = to_lower(items[i].name);
			if (lowerName.find(keyword) != std::string::npos) {
				results.push_back({ i, items[i].name });
			}
		}

		if (results.empty()) {
			VarList::OnConsoleMessage(peer, "`4No items found matching '`w" + param + "`4'.");
			return;
		}

		// Pagination — sama seperti /find
		const int items_per_page = 100;
		int page_number = 1;
		int start_index = (page_number - 1) * items_per_page, end_index = start_index + items_per_page;
		int total_items_found = results.size();
		int total_pages = (total_items_found / items_per_page) + (total_items_found % items_per_page != 0 ? 1 : 0);

		// Build item list — sama seperti /find (checkicon format)
		std::string item_list_ = "";
		for (int i = start_index; i < end_index && i < (int)results.size(); ++i) {
			int fid = results[i].first;
			bool already_checked = pInfo(peer)->find_selected_items.count(fid) > 0;
			item_list_ += "\nadd_checkicon|checkicon_" + std::to_string(fid) + "|" + items[fid].name + "|noflags|" + std::to_string(fid) + "||" + (already_checked ? "1" : "0") + "|";
		}

		// Payment info
		std::string payment_info;
		if (cfg.buy.payment_mode == 0) {
			payment_info = "`2" + std::to_string(cfg.buy.price_gems) + " Gems`` per `w" + std::to_string(cfg.buy.items_per_purchase) + "`` items";
		}
		else {
			std::string pname = (cfg.buy.payment_item_id > 0 && cfg.buy.payment_item_id < (int)items.size()) ? items[cfg.buy.payment_item_id].name : "Unknown";
			payment_info = "`2" + std::to_string(cfg.buy.payment_item_amount) + "x " + pname + "`` per `w" + std::to_string(cfg.buy.items_per_purchase) + "`` items";
		}

		// Reset selection saat search baru
		pInfo(peer)->find_selected_items.clear();

		// Build dialog — sama seperti /find
		VarList::OnDialogRequest(peer,
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wBuy Items``|left|6016|\n"
			"add_textbox|`oPayment: " + payment_info + "|left|\n"
			"add_textbox|Items Searched: `2" + to_lower(param) + "|left|\n"
			"add_textbox|Overall Results: `2" + to_string(total_items_found) + " Items|left|\n"
			"embed_data|search|" + to_lower(param) + "\n"
			"embed_data|page|" + to_string(page_number) + "\n"
			"embed_data|total_page|" + to_string(total_pages) + "\n"
			"embed_data|is_buy|1\n"
			"add_spacer|small|\n"
			"add_textbox|List Items:|left|\n"
			"add_smalltext|`5(Items Results per Page: " + to_string(items_per_page) + ")|left|\n"
			"text_scaling_string|FEatures Role o o o000000000 0|\n"
			"max_checks|" + to_string(items_per_page) + "|" + item_list_ + "|\n"
			"add_button_with_icon||END_LIST|noflags|0||\n"
			"add_spacer|small|\n"
			"add_label|small|`wPage " + to_string(page_number) + " of " + to_string(total_pages) + "|left|\n"
			"add_spacer|small|" +
			(page_number != 1 ?
				"\nadd_custom_button|previous_page" + to_string(page_number - 1) +
				"|image:interface/large/gui_left_arrow_btn.rttex;image_size:64,64;frame:0,0;|\n"
				"add_custom_margin|x:20;y:0|\n"
				"add_custom_label|PREV|target:previous_page" + to_string(page_number - 1) +
				";top:1.20;left:0.50;size:small|" : "") +
			(page_number >= total_pages ? "" :
				"\nadd_custom_button|next_page" + to_string(page_number + 1) +
				"|image:interface/large/gui_right_arrow_btn.rttex;image_size:64,64;frame:0,0;|\n"
				"add_custom_margin|x:20;y:0|\n"
				"add_custom_label|NEXT|target:next_page" + to_string(page_number + 1) +
				";top:1.20;left:0.50;size:small|") +
			"\nadd_custom_break|\n"
			"end_dialog|buy_item_dialog|Cancel|`2Buy Selected|\n"
			"add_quick_exit|\n"
		);
	}
	static void SHOP_NEW(ENetPeer * peer) {
		if (!peer || !peer->data) return;
		auto cfg = LoadBuyShopConfig();
		auto hasShopRole = [&](const std::string& role_name) -> bool {
			std::string lower = to_lower(role_name);
			if (lower == "`1vip" || lower == "vip") return Role::Vip(peer);
			if (lower == "`#moderator" || lower == "moderator") return Role::Moderator(peer);
			if (lower == "administrator") return Role::Administrator(peer);
			if (lower == "donatur") return Role::Donatur(peer);
			if (lower == "god") return Role::God(peer);
			if (lower == "unlimited") return Role::Unlimited(peer);
			if (lower == "cheater") return Role::Cheater(peer);
			if (lower == "boost") return pInfo(peer)->Role.BOOST;
			if (lower == "super_boost") return pInfo(peer)->Role.SUPER_BOOST;
			return false;
			};

		std::string roles_buttons = "";
		for (int i = 0; i < (int)cfg.shop_roles.size(); i++) {
			auto& sr = cfg.shop_roles[i];
			if (hasShopRole(sr.role_name)) {
				roles_buttons += "\nadd_button_with_icon||" + sr.role_name + "|staticYellowFrame|6292||";
			}
			else {
				roles_buttons += "\nadd_button_with_icon|shop_buy_role_" + std::to_string(i) + "|" + sr.role_name + "|staticYellowFrame|244|" + std::to_string(sr.price_coins) + "|";
			}
		}

		VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|\nadd_spacer|small|\nadd_textbox|`$" + Environment()->server_name + " Coin balance:|left|\ntext_scaling_string|Subscribtions+++|\nadd_button_with_icon||`o" + Set_Count(pInfo(peer)->gtwl) + " \xC4\x81|staticYellowFrame,no_padding_x,|244|\nreset_placement_x|\nadd_button_with_icon||END_LIST|noflags|0||\nadd_textbox|`oChoose what you want to purchase:|left|\nadd_smalltext|`oNote: You can purchase roles, coin, gems, level, titles and others in Our Official Website for real Growtopia Currency or " + Environment()->server_name + " Coin.|left|\nadd_url_button|comment|`5Visit " + Environment()->server_name + " Website``|noflags|" + Environment()->website_url + "|Do you want to open the " + Environment()->server_name + " Website?|0|0|\nadd_spacer|small|\ntext_scaling_string|AAAAAAAAAAAAA|\nadd_button_with_icon|deposit|`2DEPOSIT|noflags|1452||" + (pInfo(peer)->pg_bank ? "\nadd_button_with_icon||`5PIGGY BANK|staticYellowFrame|||" : "\nadd_button_with_icon|Buy_PiggyBank|`5PIGGY BANK|staticYellowFrame||400|") + (pInfo(peer)->Role.BOOST ? "\nadd_button_with_icon||`5[BOOST]|staticYellowFrame|6292||" : "\nadd_button_with_icon|Buy_BoostRole|`5[BOOST]|staticYellowFrame|244|400|") + (pInfo(peer)->Role.SUPER_BOOST ? "\nadd_button_with_icon||`8[SUPER-BOOST]|staticYellowFrame|6292||" : "\nadd_button_with_icon|Buy_SupBoostRole|`8[SUPER-BOOST]|staticYellowFrame|244|800|") + roles_buttons + "\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nadd_quick_exit|\nend_dialog|Shop_Assets|Nevermind.||\n", 500);
	}
	static void ConfigShopRolesDialog(ENetPeer * peer) {
		if (!peer || !peer->data) return;
		auto cfg = LoadBuyShopConfig();

		std::string dialog = "";
		dialog += "set_default_color|`o\n";
		dialog += "set_bg_color|0,52,102,178|\n";
		dialog += "set_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|`wManage Shop Roles``|left|8470|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_textbox|`oRoles currently available in /shop (paid with Coins):|left|\n";
		dialog += "add_textbox|`oCheck a role to REMOVE it from shop.|left|\n";
		dialog += "add_spacer|small|\n";

		// Show current roles with checkboxes to remove
		for (int i = 0; i < (int)cfg.shop_roles.size(); i++) {
			auto& sr = cfg.shop_roles[i];
			dialog += "add_checkbox|remove_shop_role_" + std::to_string(i) + "|`4[Remove] `w" + sr.role_name + " (`2" + std::to_string(sr.price_coins) + " Coins``)|0|\n";
		}

		dialog += "add_spacer|small|\n";
		dialog += "add_label_with_icon|small|`wAdd New Role``|left|8470|\n";
		dialog += "add_smalltext|`oAvailable roles (from getlevel):|left|\n";
		dialog += "add_smalltext|`oExcludes: owner, clist, coder, staff, streamer, has_config|left|\n";
		dialog += "add_spacer|small|\n";
		static const std::vector<std::pair<std::string, std::string>> available_roles = {
			{"`1VIP", "`1VIP"},
			{"`#Moderator", "`#Moderator"},
			{"Administrator", "`eAdministrator"},
			{"Donatur", "`cDonatur"},
			{"God", "`9God"},
			{"Unlimited", "`8Unlimited"},
			{"Cheater", "`4Cheater"},
			{"BOOST", "`5BOOST"},
			{"SUPER_BOOST", "`8SUPER BOOST"},
		};

		for (int i = 0; i < (int)available_roles.size(); i++) {
			auto& [role_key, role_display] = available_roles[i];
			// Check if already in shop
			bool in_shop = false;
			for (auto& sr : cfg.shop_roles) {
				if (to_lower(sr.role_name) == to_lower(role_key)) { in_shop = true; break; }
			}
			if (!in_shop) {
				dialog += "add_checkbox|add_shop_role_" + std::to_string(i) + "|`2[Add] " + role_display + "|0|\n";
			}
		}

		dialog += "add_spacer|small|\n";
		dialog += "add_smalltext|`oCoin Price for new role:|left|\n";
		dialog += "add_text_input|new_role_price||100|10|\n";
		dialog += "add_spacer|small|\n";

		dialog += "add_custom_button|Nevermind|textLabel:`wCancel;middle_colour:2415764;border_colour:2415764;|\n";
		dialog += "add_custom_button|Apply Changes|textLabel:`wApply;anchor:_button_Nevermind;left:1;margin:60,0;middle_colour:434431310;border_colour:434438350;|\n";
		dialog += "end_dialog|cfg_shop_roles_apply|||\n";
		dialog += "add_quick_exit|\n";

		VarList::OnDialogRequest(peer, dialog);
	}

	static void BLASTDESIGNER(ENetPeer * peer) {
		std::vector<std::pair<int, std::string>> blasts = LoadBlastDesigner();

		// Merge blast bawaan server yang belum di-override di JSON
		// Blast procedural (9164 dll) worldName-nya kosong = masih pakai generate hardcode
		static const std::vector<std::pair<int, std::string>> DEFAULT_BLASTS_LIST = {
			{9164,  ""},  // Deadly Blast (procedural)
			{1402,  ""},  // Thermonuclear Blast (procedural)
			{1060,  ""},  // Bountiful Blast (procedural)
			{8738,  ""},  // Sandy Blast (procedural)
			{8556,  ""},  // Hospital Blast (procedural)
			{6420,  ""},  // (procedural)
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
		for (auto& def : DEFAULT_BLASTS_LIST) {
			bool found = false;
			for (auto& j : blasts) {
				if (j.first == def.first) { found = true; break; }
			}
			if (!found) blasts.push_back(def);
		}

		std::string dialog = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|`wBlast Designer|left|32|\n";
		dialog += "add_smalltext|`oThis menu allows you to configure custom blast worlds.|left|\n";
		dialog += "add_smalltext|`9Note: `4You cannot change the name of a subordinate world from a blastdesigner unless the contents of that world blast.|left|\n";
		dialog += "add_spacer|small|\n";

		for (auto& p : blasts) {
			int blastId = p.first;
			std::string worldName = p.second;
			if (blastId <= 0 || blastId >= (int)items.size()) continue;

			// Tandai apakah pakai custom (dari JSON override) atau default
			std::string customWorld;
			bool isCustom = GetCustomBlastWorld(blastId, customWorld);
			std::string label = "`w" + items[blastId].name + (isCustom ? " `2[Custom]``" : " `9[Default]``");

			dialog += "add_button_with_icon|blast_" + std::to_string(blastId)
				+ "|" + label + "|staticBlueFrame| " + std::to_string(blastId) + " || \n";
		}

		dialog += "add_custom_break|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_button|add_blast|Add Blast|noflags|0|0|\n";
		dialog += "add_quick_exit|\n";
		dialog += "end_dialog|blastdesigner|Close||\n";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void ExchangeAllIn_One(ENetPeer * peer) {
		std::string dialog;
		dialog = SetColor(peer);
		dialog += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|Exchange Growtopia Private Server|left|9436|\n";
		dialog += "add_smalltext|`9Tips: `0press one of the buttons to open the exchange you want!|left|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_button|bazare_event|`^Exchage New|noflags|0|0|\n";
		dialog += "add_button|exchange_event|`8Exchange Event|noflags|0|0|\n";
		dialog += "add_button|sell_fish|`2Sell Fish|noflags|0|0|\n";
		dialog += "add_button|sell_ghost|`5Sell Ghost|noflags|0|0|\n";
		dialog += "add_button|sell_crystal|`9Sell Crystal|noflags|0|0|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_quick_exit|\n";
		dialog += "end_dialog|exchange_mainall|Nevermind.||\n";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void SetBuySlotDialog(ENetPeer * peer) {
		auto prices = LoadAutoFarmSlotPrices();

		std::string dialog;
		dialog = SetColor(peer);
		dialog += "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n";
		dialog += "add_label_with_icon|big|`wSet AutoFarm Slot Prices|left|10400|\n";
		dialog += "add_smalltext|`7Select a slot to edit its price and currency (lock).|left|\n";
		dialog += "add_spacer|small|\n";
		dialog += "add_textbox|`$Slots:|left|\n";

		for (int lvl = 1; lvl <= 10; ++lvl) {
			const auto& s = prices[lvl];
			std::string itemName = (s.item_id > 0 && s.item_id < (int)items.size() && !items[s.item_id].name.empty())
				? items[s.item_id].name
				: "Not set";

			dialog += "add_button|edit_slot_" + std::to_string(lvl) + "|`wSlot " + std::to_string(lvl) +
				" `7(" + itemName + " x" + std::to_string(s.amount) + ")|noflags|0|0|\n";
		}

		dialog += "add_spacer|small|\n";
		dialog += "add_quick_exit|\n";
		dialog += "end_dialog|setbuyslot_main|Close||\n";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void ON_ACTION(ENetPeer * peer, std::string cmd) {
		transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
		if (cmd == "/cry" || cmd == "/sad") {
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != pInfo(peer)->world) continue;
				VarList::OnTalkBubble(cp_, pInfo(peer)->netID, ":'(", 0, 0);
			}
		}
		else {
			if (cmd == "/fa" || cmd == "/foldarms" || cmd == "/fold") cmd = "/fold";
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
				if (pInfo(cp_)->world == pInfo(peer)->world) {
					bool muted_ = false;
					for (int c_ = 0; c_ < pInfo(cp_)->friends.size(); c_++) {
						if (to_lower(pInfo(cp_)->friends[c_].name) == to_lower(pInfo(peer)->tankIDName)) {
							if (pInfo(cp_)->friends[c_].mute) {
								muted_ = true;
								break;
							}
						}
					}
					if (not muted_) VarList::OnAction(cp_, pInfo(peer)->netID, cmd, 0);
				}
			}
		}
	}
	static void SaveCmdLevelOverrides() {
		std::filesystem::create_directories("database/json");
		// Read existing file to preserve "roles" key
		nlohmann::json root = nlohmann::json::object();
		{
			std::ifstream fin(CMD_LEVEL_OVERRIDE_FILE);
			if (fin.is_open()) { try { fin >> root; } catch (...) { root = nlohmann::json::object(); } }
		}
		nlohmann::json cmds = nlohmann::json::object();
		for (auto& [cmd, lvl] : commandLevelOverrides) cmds[cmd] = lvl;
		root["commands"] = cmds;
		std::ofstream f(CMD_LEVEL_OVERRIDE_FILE);
		if (f.is_open()) { f << root.dump(4); f.close(); }
	}
	static void LoadCmdLevelOverrides() {
		commandLevelOverrides.clear();
		std::ifstream f(CMD_LEVEL_OVERRIDE_FILE);
		if (!f.is_open()) return;
		nlohmann::json root;
		try { f >> root; }
		catch (...) { return; }
		// Support both old flat format and new {"commands":{...}} format
		const nlohmann::json* j = nullptr;
		if (root.is_object() && root.contains("commands") && root["commands"].is_object())
			j = &root["commands"];
		else if (root.is_object())
			j = &root;
		if (!j) return;
		for (auto& [k, v] : j->items())
			if (v.is_number_integer()) commandLevelOverrides[k] = v.get<int>();
		// Apply overrides to commandMap so level checks are immediately correct
		for (auto& [cmd, lvl] : commandLevelOverrides) {
			if (cmd.size() >= 2 && cmd.substr(0, 2) == "__") continue;
			auto it = commandMap.find(cmd);
			if (it != commandMap.end()) it->second.second = lvl;
		}
	}
	static void SaveBuiltinStringOverrides() {
		nlohmann::json j = nlohmann::json::object();
		for (auto& [k, v] : g_builtinStringOverrides) j[k] = v;
		std::filesystem::create_directories("database/json");
		std::ofstream f(BUILTIN_STRING_OVERRIDE_FILE);
		if (f.is_open()) { f << j.dump(4); f.close(); }
	}
	static void LoadBuiltinStringOverrides() {
		g_builtinStringOverrides.clear();
		std::ifstream f(BUILTIN_STRING_OVERRIDE_FILE);
		if (!f.is_open()) return;
		nlohmann::json j;
		try { f >> j; }
		catch (...) { return; }
		for (auto& [k, v] : j.items())
			if (v.is_string()) g_builtinStringOverrides[k] = v.get<std::string>();
	}
	static void SaveCustomRoles() {
		std::filesystem::create_directories("database/json");
		// Read existing file to preserve "commands" key
		nlohmann::json root = nlohmann::json::object();
		{
			std::ifstream fin(CUSTOM_ROLES_FILE);
			if (fin.is_open()) { try { fin >> root; } catch (...) { root = nlohmann::json::object(); } }
		}
		nlohmann::json arr = nlohmann::json::array();
		for (auto& r : g_customRoles) {
			nlohmann::json e;
			e["name"] = r.name; e["level"] = r.level; e["prefix"] = r.prefix;
			e["commands"] = r.commands; e["can_access_world"] = r.can_access_world; e["can_punish"] = r.can_punish;
			arr.push_back(e);
		}
		root["roles"] = arr;
		std::ofstream f(CUSTOM_ROLES_FILE);
		if (f.is_open()) { f << root.dump(4); f.close(); }
	}
	static void LoadCustomRoles() {
		std::ifstream f(CUSTOM_ROLES_FILE);
		if (!f.is_open()) return;
		nlohmann::json root;
		try { f >> root; }
		catch (...) { return; }
		// Support both old array format and new {"roles":[...]} format
		const nlohmann::json* arr = nullptr;
		if (root.is_object() && root.contains("roles") && root["roles"].is_array())
			arr = &root["roles"];
		else if (root.is_array())
			arr = &root;
		if (!arr) return;
		std::lock_guard<std::mutex> lk(g_customRoles_mtx);
		g_customRoles.clear();
		for (auto& e : *arr) {
			CustomRoleEntry r;
			r.name = e.value("name", ""); r.level = e.value("level", 0); r.prefix = e.value("prefix", "");
			r.can_access_world = e.value("can_access_world", false); r.can_punish = e.value("can_punish", false);
			if (e.contains("commands")) for (auto& c : e["commands"]) r.commands.push_back(c.get<std::string>());
			if (!r.name.empty()) g_customRoles.push_back(r);
		}
	}
	static void InitEditRole() {
		LoadCmdLevelOverrides(); LoadBuiltinStringOverrides(); LoadCustomRoles();
	}
	struct BuiltinRole { const char* name; int level; };
	static constexpr BuiltinRole BUILTIN_ROLES[] = {
		{"Player",0},{"VIP",2},{"Moderator",3},{"Administrator",4},
		{"Developer",5},{"Donatur",6},{"God",7},{"Unlimited",8},
		{"Streamers",9},{"Staff",10},{"Owner",11},{"Coder",12},{"Clist",13},{"Config",999}
	};
	static constexpr int BUILTIN_ROLE_COUNT = 14;
	static std::vector<std::string> parse_cmd_list(const std::string & s) {
		std::vector<std::string> out; std::istringstream ss(s); std::string tok;
		while (std::getline(ss, tok, ',')) {
			tok.erase(0, tok.find_first_not_of(" \t"));
			auto e = tok.find_last_not_of(" \t");
			if (e != std::string::npos) tok = tok.substr(0, e + 1);
			if (!tok.empty()) out.push_back(tok);
		}
		return out;
	}
	static void Show_EditRoleMain(ENetPeer * peer) {
		std::string wr = "`4None", pr = "`4None";
		for (int i = 0;i < BUILTIN_ROLE_COUNT;i++) {
			std::string aw = "__builtin_access_world_" + std::string(BUILTIN_ROLES[i].name);
			std::string pn = "__builtin_punish_" + std::string(BUILTIN_ROLES[i].name);
			std::string nk = "__builtin_name_" + std::string(BUILTIN_ROLES[i].name);
			std::string dn = g_builtinStringOverrides.count(nk) ? g_builtinStringOverrides.at(nk) : std::string(BUILTIN_ROLES[i].name);
			if (commandLevelOverrides.count(aw) && commandLevelOverrides.at(aw) == 1) wr = dn + "``";
			if (commandLevelOverrides.count(pn) && commandLevelOverrides.at(pn) == 1) pr = dn + "``";
		}
		{
			std::lock_guard<std::mutex> lk(g_customRoles_mtx);
			for (auto& r : g_customRoles) { if (r.can_access_world) wr = r.prefix + r.name + "``"; if (r.can_punish) pr = r.prefix + r.name + "``"; }
		}
		VarList::OnDialogRequest(peer,
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wRole Manager|left|6016|\nadd_spacer|small|\n"
			"add_textbox|`oManage builtin roles, custom roles, and command access levels.|left|\nadd_spacer|small|\n"
			"add_button|editrole_builtin|`wEdit Builtin Roles|noflags|0|0|\n"
			"add_button|editrole_custom_list|`wEdit Custom Roles|noflags|0|0|\n"
			"add_button|editrole_cmd|`wEdit Command Levels|noflags|0|0|\n"
			"add_button|editrole_add|`2Add Custom Role|noflags|0|0|\nadd_spacer|small|\n"
			"add_button|editrole_access_world|`2Can Access World: " + wr + "`o (change)|noflags|0|0|\n"
			"add_button|editrole_punish|`4Can Punish: " + pr + "`o (change)|noflags|0|0|\n"
			"end_dialog|editrole_main|Close||\nadd_quick_exit|");
	}
	static void Show_EditBuiltinList(ENetPeer * peer) {
		std::string d = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wEdit Builtin Roles|left|6016|\nadd_spacer|small|\nadd_textbox|`oSelect a builtin role to edit:|left|\nadd_spacer|small|";
		for (int i = 0;i < BUILTIN_ROLE_COUNT;i++) {
			std::string nk = "__builtin_name_" + std::string(BUILTIN_ROLES[i].name);
			std::string pk = "__builtin_prefix_" + std::string(BUILTIN_ROLES[i].name);
			std::string lk = "__builtin_level_" + std::string(BUILTIN_ROLES[i].name);
			std::string dn = g_builtinStringOverrides.count(nk) ? g_builtinStringOverrides.at(nk) : std::string(BUILTIN_ROLES[i].name);
			std::string dp = g_builtinStringOverrides.count(pk) ? g_builtinStringOverrides.at(pk) : "";
			int dl = commandLevelOverrides.count(lk) ? commandLevelOverrides.at(lk) : BUILTIN_ROLES[i].level;
			d += "\nadd_button|edit_builtin_" + std::to_string(i) + "|" + dp + dn + "`` `o(Lv." + std::to_string(dl) + ")|noflags|0|0|";
		}
		d += "\nadd_button|editrole_main_back|`wBack|noflags|0|0|\nend_dialog|editrole_builtin_list|Cancel||\nadd_quick_exit|";
		VarList::OnDialogRequest(peer, d);
	}
	static void Show_EditBuiltinForm(ENetPeer * peer, int idx) {
		if (idx < 0 || idx >= BUILTIN_ROLE_COUNT) return;
		std::string base = BUILTIN_ROLES[idx].name;
		std::string nk = "__builtin_name_" + base, pk = "__builtin_prefix_" + base, lk = "__builtin_level_" + base, ck = "__builtin_cmds_" + base;
		std::string cn = g_builtinStringOverrides.count(nk) ? g_builtinStringOverrides.at(nk) : base;
		std::string cp = g_builtinStringOverrides.count(pk) ? g_builtinStringOverrides.at(pk) : "";
		std::string cc = g_builtinStringOverrides.count(ck) ? g_builtinStringOverrides.at(ck) : "";
		int cl = commandLevelOverrides.count(lk) ? commandLevelOverrides.at(lk) : BUILTIN_ROLES[idx].level;
		VarList::OnDialogRequest(peer,
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wEdit Builtin: " + base + "|left|6016|\nadd_spacer|small|\n"
			"add_smalltext|`oDefault name: `w" + base + "`o  Default level: `w" + std::to_string(BUILTIN_ROLES[idx].level) + "|left|\nadd_spacer|small|\n"
			"add_text_input|builtin_name|Display Name:|" + cn + "|24|\nadd_spacer|tiny|\n"
			"add_text_input|builtin_prefix|Color Prefix:|" + cp + "|16|\nadd_spacer|tiny|\n"
			"add_text_input|builtin_level|Role Level (0-999):|" + std::to_string(cl) + "|4|\nadd_spacer|small|\n"
			"add_textbox|`oExtra allowed commands `w(no /, comma-separated)`o:|left|\n"
			"add_text_box_input|builtin_cmds||" + cc + "|128|4|\nadd_spacer|small|\n"
			"embed_data|builtin_idx|" + std::to_string(idx) + "\nembed_data|builtin_old_level|" + std::to_string(cl) + "\n"
			"add_custom_button|editrole_edit_cancel|textLabel:`wCancel;middle_colour:2415764;border_colour:2415764;|\n"
			"add_custom_button|editrole_builtin_save_" + std::to_string(idx) + "|textLabel:`5Save;anchor:_button_editrole_edit_cancel;left:1;margin:60,0;middle_colour:434447390;border_colour:434454430;|\n"
			"end_dialog|editrole_edit_builtin|||\nadd_quick_exit|");
	}
	static void Show_EditCustomList(ENetPeer * peer) {
		std::string d = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wCustom Roles|left|6016|\nadd_spacer|small|\nadd_textbox|`oSelect a role to edit or delete:|left|\nadd_spacer|small|";
		{
			std::lock_guard<std::mutex> lk(g_customRoles_mtx);
			for (size_t i = 0;i < g_customRoles.size();i++) { auto& r = g_customRoles[i]; d += "\nadd_button|edit_role_" + std::to_string(i) + "|" + r.prefix + r.name + "`` `o(Lv." + std::to_string(r.level) + ")|noflags|0|0|"; }
			if (g_customRoles.empty()) d += "\nadd_textbox|`4No custom roles found.|left|";
		}
		d += "\nadd_button|editrole_main_back|`wBack|noflags|0|0|\nend_dialog|editrole_custom_list|Cancel||\nadd_quick_exit|";
		VarList::OnDialogRequest(peer, d);
	}
	static void Show_AddRoleDialog(ENetPeer * peer) {
		VarList::OnDialogRequest(peer,
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`2Add Custom Role|left|6016|\nadd_spacer|small|\n"
			"add_text_input|role_name|Role Name:||24|\nadd_spacer|tiny|\n"
			"add_text_input|role_prefix|Color Prefix (e.g. `4):|`o|16|\nadd_spacer|tiny|\n"
			"add_text_input|role_level|Role Level (1-999):||4|\nadd_spacer|small|\n"
			"add_textbox|`oAllowed commands `w(no /, comma-separated)`o:|left|\n"
			"add_text_box_input|role_commands|||128|4|\nadd_spacer|small|\n"
			"add_custom_button|editrole_add_cancel|textLabel:`wCancel;middle_colour:2415764;border_colour:2415764;|\n"
			"add_custom_button|editrole_add_submit|textLabel:`2Add Role;anchor:_button_editrole_add_cancel;left:1;margin:60,0;middle_colour:434476846;border_colour:434483886;|\n"
			"end_dialog|editrole_add_dialog|||\nadd_quick_exit|");
	}
	static void Show_EditRoleForm(ENetPeer * peer, int idx) {
		std::lock_guard<std::mutex> lk(g_customRoles_mtx);
		if (idx < 0 || idx >= (int)g_customRoles.size()) return;
		auto& r = g_customRoles[idx];
		std::string cs; for (size_t i = 0;i < r.commands.size();i++) { if (i) cs += ","; cs += r.commands[i]; }
		VarList::OnDialogRequest(peer,
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`5Edit Role: " + r.name + "|left|6016|\nadd_spacer|small|\n"
			"add_text_input|role_name|Role Name:|" + r.name + "|24|\nadd_spacer|tiny|\n"
			"add_text_input|role_prefix|Color Prefix:|" + r.prefix + "|16|\nadd_spacer|tiny|\n"
			"add_text_input|role_level|Role Level (1-999):|" + std::to_string(r.level) + "|4|\nadd_spacer|small|\n"
			"add_textbox|`oAllowed commands `w(no /, comma-separated)`o:|left|\n"
			"add_text_box_input|role_commands||" + cs + "|128|4|\nadd_spacer|small|\n"
			"embed_data|role_idx|" + std::to_string(idx) + "\n"
			"add_custom_button|editrole_edit_cancel|textLabel:`wCancel;middle_colour:2415764;border_colour:2415764;|\n"
			"add_custom_button|editrole_edit_submit_" + std::to_string(idx) + "|textLabel:`5Save;anchor:_button_editrole_edit_cancel;left:1;margin:60,0;middle_colour:434447390;border_colour:434454430;|\n"
			"end_dialog|editrole_edit_form|||\nadd_quick_exit|");
	}
	static void Show_ConfirmDelete(ENetPeer * peer, int idx) {
		std::lock_guard<std::mutex> lk(g_customRoles_mtx);
		if (idx < 0 || idx >= (int)g_customRoles.size()) return;
		auto& r = g_customRoles[idx];
		VarList::OnDialogRequest(peer,
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`4Delete Role|left|6016|\nadd_spacer|small|\n"
			"add_textbox|`oAre you sure you want to delete this role?|left|\n"
			"add_smalltext|`wName: `o" + r.name + "|left|\nadd_smalltext|`wLevel: `o" + std::to_string(r.level) + "|left|\nadd_spacer|small|\n"
			"add_custom_button|editrole_del_cancel|textLabel:`wCancel;middle_colour:2415764;border_colour:2415764;|\n"
			"add_custom_button|confirm_del_" + std::to_string(idx) + "|textLabel:`4Delete;anchor:_button_editrole_del_cancel;left:1;margin:60,0;middle_colour:4290822144;border_colour:4290822144;|\n"
			"end_dialog|editrole_confirm_del|||\nadd_quick_exit|");
	}
	static void Show_AccessWorldList(ENetPeer * peer) {
		std::string d = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`2Can Access World|left|6016|\nadd_spacer|small|\nadd_textbox|`oSelect which role can break locks and access any locked world:|left|\nadd_spacer|small|\nadd_button|acworld_none|`4None (disable)|noflags|0|0|";
		for (int i = 0;i < BUILTIN_ROLE_COUNT;i++) {
			std::string nk = "__builtin_name_" + std::string(BUILTIN_ROLES[i].name);
			std::string aw = "__builtin_access_world_" + std::string(BUILTIN_ROLES[i].name);
			std::string dn = g_builtinStringOverrides.count(nk) ? g_builtinStringOverrides.at(nk) : std::string(BUILTIN_ROLES[i].name);
			std::string mk = (commandLevelOverrides.count(aw) && commandLevelOverrides.at(aw) == 1) ? " `2[ACTIVE]" : "";
			d += "\nadd_button|acworld_builtin_" + std::to_string(i) + "|" + dn + "``" + mk + "|noflags|0|0|";
		}
		{
			std::lock_guard<std::mutex> lk(g_customRoles_mtx);
			for (size_t i = 0;i < g_customRoles.size();i++) { auto& r = g_customRoles[i]; std::string mk = r.can_access_world ? " `2[ACTIVE]" : ""; d += "\nadd_button|acworld_custom_" + std::to_string(i) + "|" + r.prefix + r.name + "``" + mk + "|noflags|0|0|"; }
		}
		d += "\nadd_button|editrole_main_back|`wBack|noflags|0|0|\nend_dialog|editrole_access_world_list|Cancel||\nadd_quick_exit|";
		VarList::OnDialogRequest(peer, d);
	}
	static void Show_PunishList(ENetPeer * peer) {
		std::string d = SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`4Can Punish|left|6016|\nadd_spacer|small|\nadd_textbox|`oSelect which role can use kick/ban/mute commands:|left|\nadd_spacer|small|\nadd_button|punish_none|`4None (disable)|noflags|0|0|";
		for (int i = 0;i < BUILTIN_ROLE_COUNT;i++) {
			std::string nk = "__builtin_name_" + std::string(BUILTIN_ROLES[i].name);
			std::string pn = "__builtin_punish_" + std::string(BUILTIN_ROLES[i].name);
			std::string dn = g_builtinStringOverrides.count(nk) ? g_builtinStringOverrides.at(nk) : std::string(BUILTIN_ROLES[i].name);
			std::string mk = (commandLevelOverrides.count(pn) && commandLevelOverrides.at(pn) == 1) ? " `2[ACTIVE]" : "";
			d += "\nadd_button|punish_builtin_" + std::to_string(i) + "|" + dn + "``" + mk + "|noflags|0|0|";
		}
		{
			std::lock_guard<std::mutex> lk(g_customRoles_mtx);
			for (size_t i = 0;i < g_customRoles.size();i++) { auto& r = g_customRoles[i]; std::string mk = r.can_punish ? " `2[ACTIVE]" : ""; d += "\nadd_button|punish_custom_" + std::to_string(i) + "|" + r.prefix + r.name + "``" + mk + "|noflags|0|0|"; }
		}
		d += "\nadd_button|editrole_main_back|`wBack|noflags|0|0|\nend_dialog|editrole_punish_list|Cancel||\nadd_quick_exit|";
		VarList::OnDialogRequest(peer, d);
	}
	static void Show_EditCommandList(ENetPeer * peer, int page = 0, const std::string & search = "") {
		std::vector<std::pair<std::string, int>> cmds;

		for (auto& [k, v] : commandMap) {
			int eff = v.second;

			auto it = commandLevelOverrides.find(k);
			if (it != commandLevelOverrides.end())
				eff = it->second;

			if (!search.empty()) {
				std::string kl = k, sl = search;

				std::transform(kl.begin(), kl.end(), kl.begin(), ::tolower);
				std::transform(sl.begin(), sl.end(), sl.begin(), ::tolower);

				if (kl.find(sl) == std::string::npos)
					continue;
			}

			cmds.push_back({ k, eff });
		}

		std::sort(cmds.begin(), cmds.end());

		int total = (int)cmds.size();
		int pp = 12;

		// hitung pages tanpa std::max
		int pages = (total + pp - 1) / pp;
		if (pages < 1) pages = 1;

		// clamp page tanpa std::min/max
		if (page < 0) page = 0;
		if (page > pages - 1) page = pages - 1;

		int s = page * pp;
		int e = s + pp;
		if (e > total) e = total;

		std::string d =
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wEdit Command Levels|left|6016|\n"
			"add_spacer|small|\n"
			"add_text_input|cmd_search|Search:||32|\n"
			"add_button|cmd_search_go|`wSearch|noflags|0|0|\n"
			"add_spacer|small|\n"
			"add_textbox|`oPage " + std::to_string(page + 1) + "/" + std::to_string(pages) +
			"  (" + std::to_string(total) + " commands)|left|";

		for (int i = s; i < e; i++) {
			d += "\nadd_button|cmdlvl_" + std::to_string(page) + "_" + std::to_string(i) + "|" +
				cmds[i].first + "  `o(Lv." + std::to_string(cmds[i].second) + ")|noflags|0|0|";
		}

		d += "\nadd_spacer|small|";

		if (page > 0) {
			d += "\nadd_button|cmdpage_" + std::to_string(page - 1) + "|`w< Previous|noflags|0|0|";
		}

		if (page < pages - 1) {
			d += "\nadd_button|cmdpage_" + std::to_string(page + 1) + "|`wNext >|noflags|0|0|";
		}

		d += "\nadd_button|editrole_main_back|`wBack|noflags|0|0|"
			"\nembed_data|current_search|" + search +
			"\nend_dialog|editrole_cmd_list|Cancel||"
			"\nadd_quick_exit|";

		VarList::OnDialogRequest(peer, d);
	}
	static void Show_SetCmdLevelForm(ENetPeer * peer, const std::string & cmd, int cur, int page, const std::string & search) {
		VarList::OnDialogRequest(peer,
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSet Command Level|left|6016|\nadd_spacer|small|\n"
			"add_textbox|`wCommand: `o" + cmd + "|left|\nadd_textbox|`wCurrent level: `o" + std::to_string(cur) + "|left|\nadd_spacer|small|\n"
			"add_text_input|new_level|New Level (0-999):|" + std::to_string(cur) + "|4|\nadd_spacer|small|\n"
			"embed_data|cmd_name|" + cmd + "\nembed_data|page_num|" + std::to_string(page) + "\nembed_data|search_str|" + search + "\n"
			"add_custom_button|cmdlvl_cancel|textLabel:`wCancel;middle_colour:2415764;border_colour:2415764;|\n"
			"add_custom_button|cmdlvl_save|textLabel:`5Save;anchor:_button_cmdlvl_cancel;left:1;margin:60,0;middle_colour:434447390;border_colour:434454430;|\n"
			"end_dialog|editrole_set_cmd_level|||\nadd_quick_exit|");
	}
	static void Show_CustomRolePicker(ENetPeer * peer) {
		std::string d = SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wSelect Custom Role|left|6016|\n"
			"add_spacer|small|\n"
			"add_textbox|`oSelect a role to assign, or pick Clear to remove:|left|\n"
			"add_spacer|small|\n";
		{
			std::lock_guard<std::mutex> lk(g_customRoles_mtx);
			for (size_t i = 0; i < g_customRoles.size(); i++) {
				auto& r = g_customRoles[i];
				d += "add_button|grole_pick_" + std::to_string(i) + "|" + r.prefix + r.name + "`` `o(Lv." + std::to_string(r.level) + ")|noflags|0|0|\n";
			}
			if (g_customRoles.empty()) d += "add_textbox|`4No custom roles found. Use /editrole to create one.|left|\n";
		}
		d += "add_spacer|small|\n"
			"add_button|grole_pick_clear|`4Clear Custom Role|noflags|0|0|\n"
			"add_button|grole_pick_cancel|`wCancel|noflags|0|0|\n"
			"end_dialog|grole_picker|||\nadd_quick_exit|";
		VarList::OnDialogRequest(peer, d);
	}
	static void EDITROLE(ENetPeer * peer) {
		if (!Role::Clist(pInfo(peer)->tankIDName)) { VarList::OnConsoleMessage(peer, "`4You don't have permission to use /editrole."); return; }
		Show_EditRoleMain(peer);
	}
	static void SETSHOP(ENetPeer * peer) {
		if (!peer || !peer->data) return;
		if (!Role::Clist(pInfo(peer)->tankIDName)) {
			VarList::OnConsoleMessage(peer, "`4You don't have permission to use /setshop.");
			return;
		}
		std::string dialog =
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wShop Pack Creator (/setshop)|left|6016|\n"
			"add_spacer|small|\n"
			"add_textbox|`oCreate or edit shop pack JSON configuration easily:|left|\n"
			"add_spacer|small|\n"
			"add_text_input|shop_name|Pack File Name (JSON)|katzy_auto_0|32|\n"
			"add_smalltext|`7Filename without extension or leading hyphen (e.g. katzy_auto_0 creates -katzy_auto_0.json)|left|\n"
			"add_spacer|small|\n"
			"add_text_input|pack_title|Display Title|Dark Magic Katana|32|\n"
			"add_smalltext|`7Title shown to players in shop dialog|left|\n"
			"add_spacer|small|\n"
			"add_text_input|gems_price|Price (Gems)|500000|10|\n"
			"add_spacer|small|\n"
			"add_text_input|item_id|Item ID|14630|6|\n"
			"add_smalltext|`7ID of the item given when purchased|left|\n"
			"add_spacer|small|\n"
			"add_text_input|item_count|Item Count|1|6|\n"
			"add_smalltext|`7Quantity of item awarded|left|\n"
			"add_spacer|small|\n"
			"add_custom_button|setshop_cancel|textLabel:`wCancel;middle_colour:2415764;border_colour:2415764;|\n"
			"add_custom_button|setshop_save|textLabel:`2Save Configuration;anchor:_button_setshop_cancel;left:1;margin:60,0;middle_colour:434476846;border_colour:434483886;|\n"
			"end_dialog|setshop_dialog|||\nadd_quick_exit|";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void GACHA_DIALOG(ENetPeer * peer) {
		if (!peer || !peer->data) return;
		Player* pl = pInfo(peer);
		if (!pl) return;

		long long wallet = (long long)pl->gems;
		long long bank = pl->Gems_Storage;
		long long total = wallet + bank;
		long long cost = GachaDarkstone::COST;

		std::string wallet_str = Set_Count((int)wallet);
		std::string bank_str = Set_Count((int)bank);
		std::string total_str = Set_Count((int)total);

		// Build item pool preview icons
		std::string item_icons = "";
		std::vector<int> shown;
		for (int id : GachaDarkstone::pool) {
			if (std::find(shown.begin(), shown.end(), id) == shown.end()) {
				shown.push_back(id);
				item_icons += "\nadd_button_with_icon||" + items[id].ori_name +
					"|staticPurpleFrame|" + std::to_string(id) + "||\n";
			}
		}

		// Affordability hint
		std::string afford_line;
		if (wallet >= cost) {
			afford_line = "\nadd_smalltext|`2You have enough Gems in wallet to spin!|left|";
		}
		else if (total >= cost) {
			long long from_bank = cost - wallet;
			afford_line = "\nadd_smalltext|`6Wallet insufficient. `o" +
				Set_Count((int)from_bank) + " Gems will be taken from your Bank Central.|left|";
		}
		else {
			afford_line = "\nadd_smalltext|`4You don't have enough Gems (wallet + bank) to spin!|left|";
		}

		std::string dialog =
			SetColor(peer) +
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`5Dark Stone Gacha``|left|7960|\n"
			"add_spacer|small|\n"
			"add_textbox|`oTest your luck with the `5Dark Stone Gacha``!|left|\n"
			"add_spacer|small|\n"
			"add_label_with_icon|small|`wCost per spin|left|14590|\n"
			"add_smalltext|`2100,000 Gems`` per pull.|left|\n"
			"add_spacer|small|\n"
			"add_label_with_icon|small|`wYour Balance|left|14590|\n"
			"add_smalltext|`oWallet : `2" + wallet_str + " Gems``|left|\n"
			"add_smalltext|`oBank   : `2" + bank_str + " Gems``|left|\n"
			"add_smalltext|`oTotal  : `2" + total_str + " Gems``|left|\n" +
			afford_line +
			"\nadd_spacer|small|\n"
			"add_label_with_icon|small|`wPossible Items|left|7960|\n"
			"add_smalltext|`oThe following items can be obtained from this gacha:|left|\n" +
			item_icons +
			"\nadd_button_with_icon||END_LIST|noflags|0||\n"
			"add_spacer|small|\n"
			"add_smalltext|`oItem goes to Backpack if slots available, otherwise dropped in world.|left|\n"
			"add_spacer|small|\n"
			"add_button|gacha_cancel|`wCancel|noflags|0|0|\n"
			"add_custom_button|gacha_spin_x1|textLabel:`2Spin x1 (100k Gems);anchor:_button_gacha_cancel;left:1;margin:60,0;middle_colour:434438350;border_colour:434445390;|\n"
			"add_custom_button|gacha_spin_x10|textLabel:`6Spin x10 (1M Gems);anchor:_button_gacha_spin_x1;left:1;margin:5,0;middle_colour:454348650;border_colour:454349690;|\n"
			"end_dialog|gacha_darkstone|||";

		VarList::OnDialogRequest(peer, dialog);
	}
	static void BANNERS_DIALOG(ENetPeer * peer) {
		if (!peer || !peer->data) return;
		DialogHandle::Gacha(peer, "Event");
	}

	// ── Daily Reward System ─────────────────────────────────────────────
};  // end class Commands
