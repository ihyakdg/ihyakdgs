#pragma once
#ifdef _WIN32
#elif defined(__linux__)
#define BYTE unsigned char
#define __int64 int64_t
#define __int32 int32_t
#define __int16 int16_t
#define __int8 int8_t
#endif
#include <regex>
#include <ranges>
#include <cstdio>
#include <vector>
#include <memory>
#include <cstdlib>
#include <variant>
#include <iostream>
#include <type_traits>
#include <string_view>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <string>
#include <curl/curl.h>

using json = nlohmann::json;
using namespace std;
#define Memory_Copy memcpy
#define Memory_Set memset
#define Memory_Move memmove
#define Exit_World exit_
#define Enter_World join_world
#define Player_Respawn SendRespawn
#define Peer_Disconnect enet_peer_disconnect_later
#define Peer_Reset enet_peer_reset
#define Only_Access block_access
#define TILEFLAG_WATER 0x04000000
#define TILEFLAG_OPEN 0x00400000
#define TILEFLAG_PUBLIC 0x00800000
#define MAX_GEMS 2100000000

ENetHost* server;
using FunctionType = void(*)(ENetPeer*, std::string);
std::unordered_map<std::string, FunctionType> actionMap, dialogMap;
unordered_map<ENetPeer*, string> playerTankIDNames;
unordered_map<ENetPeer*, string> playerTankIDPasses;

using CmdFunctionType = std::function<void(ENetPeer*, const std::string&, int)>;
std::unordered_map<std::string, std::pair<CmdFunctionType, int>> commandMap;
vector<vector<int>> kainos = {};
int server_port = 0;
bool secret_standopowah = false;

class ServerEnvironment {
public:
	BYTE* ItemsDat;
	std::vector<thread> Threads;
	bool isbeta = true;
	int Access_World_Level = 0;
	long long Worlds_Refresh = 0, Last_Update = 0, Last_Update_Mines = 0, Last_Checkip = 0, Last_SystemInfo = 0, Last_Growganoth = 0, Last_Time2_ = 0, Last_Time = 0, Last_Fire_Time = 0, Last_Bot = 0, Last_Firehouse = 0, Last_Update_Royalpass = 0, Last_Update_Rich = 0, Last_Math = 0, Last_Autofarm = 0;
	int AutoFarm_Delay = 0, Dq_Day = 0, Dq_Item1 = 0, Dq_Item2 = 0, Dq_Count1 = 0, Dq_Count2 = 0, ItemsDat_Hash = 0, ItemsDat_Size = 0, ItemsDat_Version = 0, ItemsDat_Count = 0, Stock_Mag = 0, Stock_GRay = 0, Stock_ERay = 0, Server_Port = 0, Enter_Count = 0, Restart_Time = 10, janeway_payout = 0;
	long long int World_Time = 0, UpTime = 0;
	std::vector<std::tuple<int, int, int>> Npc_MarketPlace;
	std::vector<std::vector<int>> price_item = {};
	long long last_carnni_time = 0;  // FIX LAG #1: was int → overflow! ms_time ~1.7T > INT_MAX
	long long last_carnig_time = 0;  // FIX LAG #1: was int → carnival scan ran EVERY tick (always ≤ 10000)
	long long Last_Carnival = 0;  // FIX LAG #2: throttle carnival section ke 1000ms
	std::unordered_map<int, int> shop_data;
	std::vector<std::pair<int, std::string>> top_ratings, top_points, top_yesterday, top_overall, Ancient_Riddle;
	// Active_Worlds: world_name → player_count, O(1) lookup (was vector<pair> → O(n) find per peer per tick)
	std::unordered_map<std::string, int> Active_Worlds;
	std::vector<std::pair<std::string, long long int>> Home_Timed;
	int Active_Worlds_Count = 0;
	std::string Server_Password = "", apikey_ai = "", server_name = "", GT_Version = "", server_version = "", discord_url = "https://discord.gg/7qWfQMEQ", osm_link = "", osm_path = "", proto = "", website_url = "", server_ip = "", zombie_list = "", surgery_list = "", wolf_list = "", whatsapp_url = "", BOT_TOKEN = "", Active_World_List, List_World_Menu, Sb_World, Sdb_World, Deposit_World;
	std::vector<uint16_t> change_id_beach{ 3358,3359,3360,3361,3362,3363,3364,3365,3366,3367,3368,3369,3370,3371,3372,3373,3374,3375,3376,3377,3378 };
	std::vector<std::vector<string>> world_rate_types{ {},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{} };
	std::vector<std::pair<std::string, std::string>> WOTD, WOTW, Logs, World_Report, Player_Report;
	std::vector<std::string> swear_words = {}, splicing, combining, combusting, crystals, Another_Worlds, World_Menu, Wotd_Worlds, PlayerMap, Rid_Bans, BlackList, Verifed_Csn_World, Waiting_Verif_Csn, Previous_Active_Worlds;
	std::vector<std::vector<std::vector<int>>> legendary_quests = { {{0, 2000, 442}, {0, 100000}, {0, 5000}, {0, 600, 1422}, {0, 50000}, {0, 50}, {0, 3, 1462}, {0, 10000}, {0,1000, 784}, {0,100000}, {0,100000}, {0,100}, {0,1000}, {0,3, 1458}, {0,100000}, {0,1, 1614}, {0,3, 1680}, {0,10, 1672}, {0,3, 1280}, {0,1, 1794}}, {{0, 2000, 4}, {0, 100000}, {0, 5000}, {0, 600, 598}, {0, 50000}, {0, 50}, {0, 10, 900}, {0, 10000}, {0,1000, 1114}, {0,100000}, {0,100000}, {0,100}, {0,1000}, {0,3, 1576}, {0,100000}, {0,1, 394}, {0,3, 1378}, {0,1000, 1128}, {0,3, 1206}, {0,1, 1794}}, {{0, 2000, 914}, {0, 100000}, {0, 5000}, {0, 600, 254}, {0, 50000}, {0, 50}, {0, 3, 1274}, {0, 10000}, {0,1000, 324}, {0,100000}, {0,100000}, {0,100}, {0,1000}, {0,3, 1250}, {0,100000}, {0,1, 1602}, {0,5, 1354}, {0,5, 1396}, {0,2, 8284}, {0,1, 1794}}, {{0, 1000, 728}, {0, 100000}, {0, 5000}, {0, 600, 156}, {0, 50000}, {0, 50}, {0, 3, 1550}, {0, 10000}, {0,800, 678}, {0,100000}, {0,100000}, {0,100}, {0,1000}, {0,20, 1672}, {0,100000}, {0,3, 1460}, {0,100, 818}, {0,1, 1674}, {0,50, 1574}, {0,1, 1794}}, {{0, 1000, 684}, {0, 100000}, {0, 5000}, {0, 600, 604}, {0, 50000}, {0, 50}, {0, 3, 2386}, {0, 10000}, {0,800, 690}, {0,100000}, {0,100000}, {0,100}, {0,100}, {0,20, 810}, {0,100000}, {0,1, 2002}, {0,10, 2456}, {0,20, 2572}, {0,20, 2266}, {0,1, 1794}}, {{0, 2000, 1050}, {0, 100000}, {0, 10000}, {0, 1000, 6080}, {0, 100000}, {0, 50}, {0, 10, 4880}, {0, 20000}, {0,5, 5202}, {0,140000}, {0,200000}, {0,100}, {0,20}, {0,3, 5276}, {0,200000}, {0,5, 6028}, {0,3,  5650}, {0,1, 6094}, {0,3, 5078}, {0,1, 1794}}, {{0, 2000, 5042}, {0, 100000}, {0, 10000}, {0, 1000, 1736}, {0, 100000}, {0, 50}, {0, 10, 2982}, {0, 20000}, {0,800, 1936}, {0,140000}, {0,100000}, {0,100}, {0,10}, {0,10, 1824}, {0,200000}, {0,1, 4302}, {0,5, 7762}, {0,1, 5754}, {0,10, 7728}, {0,1, 1794}}, {{0, 2000, 8206}, {0, 100000}, {0, 10000}, {0, 50, 10160}, {0, 100000}, {0, 50}, {0, 50, 11152}, {0, 20000}, {0,10, 10130}, {0,100000}, {0,200000}, {0,100}, {0,10}, {0,1, 10634}, {0,200000}, {0,2, 7416}, {0,3, 11050}, {0,1, 10724}, {0,1, 10576}, {0,1, 1794}}, {{0, 2000, 9390}, {0, 100000}, {0, 10000}, {0, 150, 10214}, {0, 100000}, {0, 50}, {0, 15, 8952}, {0, 20000}, {0,10, 10332}, {0,100000}, {0,200000}, {0,100}, {0,1000}, {0,3, 2952}, {0,200000}, {0,10, 11076}, {0,5,  9376}, {0,1, 11008}, {0,1, 10806}, {0,1,1794}}, {{0, 50, 2386}, {0, 400000}, {0, 500000}, {0, 5, 1614}, {0, 750000}, {0, 200}, {0, 3, 9488}, {0, 1000000}, {0,2000, 2590}, {0,10000000}, {0,750000}, {0,1000}, {0,69420}, {0,20, 1458}, {0,100000}, {0,30, 7188}, {0,100, 1068}, {0,2, 9548}, {0,250, 7960}, {0,1, 1794}} };
	std::vector<int> growganoth_platform, random_mooncake, dstone, random_shop_item, phoenix_items_all, all_phoenix_items{ 1674, 2854, 3696, 4820, 6312, 8588, 9730, 13572 }, the_special{ 7960, 7960, 9350,9814, 7954, 7484, 5948, 9380, 11038, 3432 }, leonidas{ 7960,7960, 7960, 7960, 7954, 7484, 5948, 9380, 11038, 3432, 7954, 7484, 5948, 9380, 11038, 3432, 7954, 7484, 5948, 9380, 11038, 3432, 7954, 7484, 5948, 9380, 11038, 3432, 7954, 7484, 5948, 9380, 11038, 3432, 7954, 7484, 5948, 9380, 11038, 3432 };
	std::vector <std::string> random_{ "Failure is the opportunity to begin again more intelligently.","You are enough just as you are","Every journey has its final day. Don't rush.","Osmanthus wine tastes the same as I remember... But where are those who share the memory?", " I hope today too shall be prosperous.", "The market is closed and the port has settled. Go get some rest.", " You and I have a contract, so feel free to discuss anything at all within the scope of said contract.", " It seems my strength is returning. Not a lot, but enough.", "One man's stone is another man's gem...", " Ugh... I'm feeling a little... off...", "Even bedrock can be turned to dust...", "Inactivity serves no purpose whatsoever.", " No, I have nothing to share with you at this time. But you... You use elemental energy without the aid of a Vision. How is this possible? Please enlighten me.", "State your query. To the point.", "Foolish question. There are only two kinds of things — those that must be done and those that must not.", "Consuming this would be physically detrimental. Remove it from my presence.", "Is today the day of your creation? Return to the city with me. I will read out a speech for you in front of all my attendants. 'Embarrassing'? ...Understood." }, random_quotes{ "Any type Rocks might drop DNA'","Acid blocks drop more Rubbles", "Fossil Rocks may appear in active world", "Mystery Item pack in shop has all the newest items", "Breaking Grass has a chance to drop Butterly", "Breaking Cow has a chance to drop Cow Pet", "Leonidas Scythe is extremely rare to obtain", "Coin drop item exists", "Obtain Voucher's by depositing" }, random_color{ "`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4","`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4", "`#", "`5", "`e", "`2" , "`9", "`8", "`4" };
	std::vector<int> random_janeway_payout{ 10394, 4298 , 3936, 4134, 928, 866, 1632, 872, 2242,2244,2246 };
	std::vector<std::pair<int, int>> opc_item, items_washing, buy_item_list, find_item_list, zombieitem, surgeryitem, wolfitem, play_items = { {4374, 300}, {7670, 500}, {9250, 600}, {11536, 700} , {12992, 800} }, janeway_items = { {2, 1}, {4, 2},  {10,1},{14, 1}, {22, 2}, {26, 3}, {28, 3}, {30, 4}, {52, 5}, {54, 5} }, janeway_item;
	std::vector<uint16_t> bountiful_seeds = { 8623 , 8671 , 8695 , 8647 }, small_seeds = { 231,487,483,2019,427,1757,2009,281,781,329,459,1041,421,2787,2789,1043,2797,181,613,65,4983,283,553,425,4635,987,627,2017,989,61,4639,973,3523,331,265,859,433,253,339,1105,439,4567,333,677,5673,1427,5603,183,10533,5731,7223,10833,6035,6813,1113,1047,653,2811,4703,1429,847,665,485,1613,833,2025,1775,3931,429,3831,423,437,1773,4767,1299,361,667,2815,3473,301,4637,5683,2709,415,955,461,861,9389,5467,2791,2815,1803,419,2027,685,555,741,10531,5729,7221,10831,6033,6811,621,6401,1107,359,3471,855,309,1685,1449,1309,441,1111,221,1431,3065,9307,1257,1501,1109,4641,3587,1103,417,743,431,3823,2949,1131,3825,727,5379,729,187,787,5459,4785,3261,3521,2293,2309,2321,2333,5461,3721,327,247,63,3479,445,4783,299,863,8935,1451,261,1871,1499,455,9391,683,1159,551,1745,411,3005,557,687,4799,559,4643,3899,1605,9309,567,565,1155,3827,1327,1767,993,1157,563,4721,115,1423,10515,1329,6545,287,669,777,5463,1655,1599,561,4585,3413,4745,1771,2011,1161,1729,3757,3753,321,3265,3263,623,775,6397,129,4667,759,3767,1529,6543,4707,3835,1435,651,8929,569,527,3287,1595,6063,2795,1003,1731,4943,4963,597,4669,1531,1523,625,4297,599,657,571,873,3755,2969,1725,4603,1447,3267,5375,1683 }, small_seed_pack{ 3567, 13, 57, 17, 381, 1139, 21, 101, 379, 117, 27, 23, 581, 195, 105, 191, 377, 1307, 697, 881, 167, 185, 1323, 165, 3783, 369, 25, 171, 53, 103, 179, 29, 249, 177, 1325, 337, 189, 169, 55, 223, 175, 885, 119, 3573, 199, 549, 681, 655, 107, 163, 31, 671, 889, 887, 193, 127, 1433, 173, 7631, 413, 3571, 3569, 2809, 9393, 225, 111, 109, 201, 59, 607, 285, 2029, 2013, 2015, 1611, 995, 997, 963, 1481, 219, 197, 2023, 2021, 9331, 1305, 823, 1039, 231, 487, 483, 2019, 427, 1757, 2009, 281, 781, 329, 459, 1041, 421, 2787, 2789, 1043, 2797, 181, 613, 65, 4983, 283, 553, 425, 4635, 987, 627, 2017, 989, 61, 4639, 973, 3523, 331, 265, 859, 433, 253, 339, 1105, 439, 4567, 333, 677, 5673, 1427, 5603, 183, 10533, 5731, 7223, 10833, 6035, 6813, 1113, 1047, 653, 2811, 4703, 1429, 847, 665 };
	bool Auto_Saving = false, Enable_Joystick = false, Maintenance = false, Restart_Status = false, Restart_Status_Seconds = false, Anti_GL = false, Anti_Proxy = false, Fake_Online = false, GracefullShutdown = false;
	int Fake_Online_Count = 0;
	std::vector<std::string> lock_gacha_allowed_growids; // GrowIDs that can use /find /getitem /get on LOCK items — diisi dari config.json key "LOCK_GACHA_ALLOWED_GROWIDS"
};
ServerEnvironment* Environment();
inline ServerEnvironment TEnvironment;
ServerEnvironment* Environment() {
	return &TEnvironment;
}
class ServerEvent {
public:
	std::string dailyc_name = "", top_dailyc_list = "", top_anniversary_list = "";
	long long int Anniversary_Time = 0, x_exchange_time = 0, x_gems_time = 0, x_xp_time = 0, daily_current_time = 0, daily_wait_time = 0;
	std::vector<std::pair<long long int, std::string>> top_dailyc, top_anniversary;
	int X_Exchange = 1, X_Xp = 1, X_Gems = 1, Math_Num1 = 0, Math_Num2 = 0, Math_Result = 0, Math_Prize = 0;
	bool Exchange_Event = false, Gems_Event = false, Xp_Event = false, Anniversary = false, Harvest_Festival = false, Carnival = false, Halloween = false, Valentine = false, Summerfest = false, DailyMaths = false, Comet_Dust = false, Zombie_Apocalypse = false, DailyChallenge = false, Wait_Next = false, Can_Event = false, Locke_Salesman = false;
};
ServerEvent* Event();
ServerEvent TEvent;
ServerEvent* Event() {
	return &TEvent;
}
inline bool hasRepetitiveCharacters(std::string_view str) {
	if (str.empty()) return false;
	for (size_t i = 0; i < str.length() - 1; ++i) {
		if (str[i] == str[i + 1]) {
			return true;
		}
	}
	return false;
}
inline bool followsSequentialPattern(std::string_view str) {
	static constexpr std::array<std::string_view, 3> sequentialPatterns = { "123456789", "987654321", "24682468" };
	for (const auto& pattern : sequentialPatterns) {
		if (str.find(pattern) != std::string_view::npos) {
			return true;
		}
	}
	return false;
}
std::vector<std::pair<int, std::string>> top_guild_winners, top_player_points, top_basher_winners;
std::vector<std::pair<long long int, std::string>> top_wls, top_guild, top_basher, top_punch, last_basher_list, last_guild_last, top_richest, top_online, top_royalpass, top_mines;
long long int total_wls_recycled = 0, wls_event_time = 0, gacha_event_time = 0, cycle_s = 0, server_event_spawn = 0, next_event = 0, wait_event = 0, current_event = 0;
std::string top_wls_list = "", a = "", best_guild = "", best_quote = "", top_mines_list = "", top_on_list = "", royalpass_list = "", top_rich_list = "", top_basher_last = "", top_basher_last_shop = "", top_guild_last = "", top_guild_last_shop = "", guild_event_type = "", top_basher_list_shop = "", top_guild_list_shop = "", top_list = "", top_yesterday_list = "", top_overall_list = "", top_punch_list = "", top_basher_list = "", top_guild_list = "", top_old_guild_winners = "", top_old_winners = "";

// ── Donation Server Event ─────────────────────────────────────────────────
bool donation_event_active = false;
std::vector<std::pair<long long int, std::string>> top_donation;
std::string top_donation_list = "";
// Build_Donation_Leaderboard() didefinisikan di Commands.h setelah Set_Count tersedia
// ─────────────────────────────────────────────────────────────────────────
int  day_item = 1796, day_count = 1, personal_t1 = 0, personal_t2 = 0, personal_t3 = 0, personal_t4 = 0, personal_t5 = 0, personal_t6 = 0, personal_t7 = 0, personal_t8 = 0, personal_t9 = 0, personal_t10 = 0, personal_t11 = 0, personal_t12 = 0, personal_t13 = 0, personal_t14 = 0, personal_t15 = 0, personal_t16 = 0, personal_t17 = 0, personal_t18 = 0, personal_t19 = 0, personal_t20 = 0, guild_t1 = 0, guild_t2 = 0, guild_t3 = 0, guild_t4 = 0, guild_t5 = 0, guild_t6 = 0, guild_t7 = 0, guild_t8 = 0, guild_t9 = 0, guild_t10 = 0, guild_t11 = 0, guild_t12 = 0, guild_t13 = 0, guild_t14 = 0, guild_t15 = 0, guild_t16 = 0, guild_t17 = 0, guild_t18 = 0, guild_t19 = 0, guild_t20 = 0, guild_t21 = 0, guild_t22 = 0, guild_t23 = 0, guild_t24 = 0, guild_t25 = 0, guild_event_id = 0, total_pineapple_eaten = 0, event_item = 0, special_event_item = 0, old_event_item = 0, theitemtoday = 0, punched = 0, rolequest_day = 0, today_month = -1, today_day = -1;

enum MINING { DIAMOND_ORE = 20006, DIAMOND = 20056, PLATINUM_ORE = 20036, PLATINUM = 20060, GOLDEN_ORE = 20004, GOLDEN = 20054, SILVER_ORE = 20002, SILVER = 20044, TITANIUM_ORE = 20008, TITANIUM = 20062, VANADIUM_ORE = 20038, VANADIUM = 20064, COPPER_ORE = 20034, COPPER = 20042, TIN_ORE = 20032, TIN = 20066, CELESTINE_ORE = 20000, CELESTINE = 20052, NAVAL_BOMB = 6994, MINE_CRYSTAL = 14532, GOLDEN_MINE_CHEST = 14528, LEGENDARY_MINE_CHEST = 14530 };

std::mutex mtx;
inline std::mutex g_world_loop_mutex;
std::condition_variable cv;
std::atomic<bool> running(true);
inline std::atomic<bool> g_server_running{ true };

struct DisconnectSaveItem { std::string name; std::string json; };
inline struct {
	std::deque<DisconnectSaveItem> queue;
	std::mutex mtx;
} g_disconnect_save_queue;

inline struct {
	std::deque<DisconnectSaveItem> queue;
	std::mutex mtx;
} g_disconnect_world_save_queue;

inline std::unordered_map<std::string, long long> g_world_last_activity;
inline std::mutex g_world_last_activity_mtx;

inline long long now_ms_() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline void mark_world_active(const std::string& safe_name) {
	std::lock_guard<std::mutex> lk(g_world_last_activity_mtx);
	g_world_last_activity[safe_name] = now_ms_();
}

// Returns true if the world (by safe/sanitized name) had activity within cooldown_ms.
inline bool world_recently_active(const std::string& safe_name, long long cooldown_ms = 20000) {
	std::lock_guard<std::mutex> lk(g_world_last_activity_mtx);
	auto it = g_world_last_activity.find(safe_name);
	if (it == g_world_last_activity.end()) return false;
	return (now_ms_() - it->second) < cooldown_ms;
}
inline std::atomic<long long> g_enet_heartbeat{ 0 };
// FIX CPU: timer untuk Looping::Worlds() yang kini dipanggil di ENet thread
inline long long g_looping_worlds_next = 0;
struct {
	bool Active = false, Growch_Happy = false;
	int Heart_Size = 0, GiftedGrowch = 0, Progress_Gifted = 0, Calendar_Sold = 0, Calendar_Stock = 400, Bonanza_Sold = 0, Bonanza_Stock = 300;
	long long int Growch_Happy_Time = 0, Goblin_Spawn = 0;
	std::vector<std::pair<uint32_t, int>> Growch_Prize{ {4292, 100000}, {4286, 250000}, {4288, 50000}, {10500, 1000000} };
	std::vector<int> Bingo_Id_Task{ 10458, 3204, 5348, 8278, 8280, 12942, 10488, 5350, 10508, 5444, 11192, 10478, 10476, 2564, 10444, 8722, 10480, 11538, 12934, 10464, 5354, 5446, 5382, 5476, 8948, 12944, 10466, 10468, 13014, 5398, 5362, 5386, 5400, 5478, 14134, 11530, 10472, 7436, 5396, 7448, 10474, 10484, 7396, 11526, 5394, 12950, 5358, 5360, 12954, 11528, 5352, 12948, 5474, 11522, 10470, 10460 };
	std::vector<std::pair<int, int>> Bingo_Prize{ {5402, 5},{5402, 5},{5402, 5},{1360, 100},{1360, 100},{5404, 10}, {3206, 20},{3206, 20},{3206, 20}, {5402, 5},{5402, 5},{5402, 5},{5402, 5},{5402, 5},{5402, 5},{5402, 5},{5402, 5},{5402, 5},{5402, 5}, {5404, 10},{5404, 10},{5404, 10},{5404, 10},{5404, 10}, {3210, 1}, {9202, 1}, {5440, 1}, {10424, 1},{5404, 10},{5404, 10},{5404, 10}, {11478, 1}, {14086, 1}, {12958, 1}, {12958, 1}, {14142, 1}, {14076, 1}, {12892, 1}, {11480, 1}, {14094, 1}, {5404, 10},{5404, 10},{5404, 10}, {15082, 1}, { 5404, 10 },{5404, 10},{5404, 10},{5404, 10},{14092, 1} };
	std::vector<std::vector<double>> Calendar_2022{ {11244,263, 263,7088, 7088, 7094, 7094, 11520, 11520, 7864, 7864, 8976, 8976, 12918, 12918 , 12918, 5428, 5428, 5428, 5430, 5430, 5430, 5432, 5432, 5432, 5434, 5434, 5434, 10486, 10486, 10486, 10444, 10444, 10444, 11202, 11202, 11202, 7498, 7498, 7498, 7498, 10488, 10488,10488,10488 , 8280, 8280, 8280, 8280, 8278, 8278, 8278, 8278  }, {10054, 9202, 1384, 11192,5446, 11514, 8480, 8554, 7496, 7090, 9202, 1384, 11192,5446, 11514, 8480, 8554, 7496, 7090, 12918, 5428, 5430, 5432, 5434, 8546, 12918, 5428, 5430, 5432, 5434, 8546, 12918, 5428, 5430, 5432, 5434, 8546, 10488, 2564.5, 1360.5, 10488, 2564.5, 1360.5, 10488, 2564.5, 1360.5, 10488, 2564.5, 1360.5 }, {11158, 7490, 8556, 10258, 11520, 3676, 7086, 7082, 10036, 10042, 7490, 8556, 10258, 11520, 3676, 7086, 7082, 10036, 10042, 11202, 12918, 5446, 11202, 12918, 5446, 11202, 12918, 5446, 10444,5428, 5430, 5432, 5434, 10444,5428, 5430, 5432, 5434, 10444,5428, 5430, 5432, 5434, 10444,5428, 5430, 5432, 5434 }, {12302, 9202, 6414, 7080, 7422, 7758, 6384, 10032, 9202, 6414, 7080, 7422, 7758, 6384, 10032,11514, 11192, 1368.10, 12918,11514, 11192, 1368.10, 12918,11514, 11192, 1368.10, 12918, 5382.2, 5428, 5430, 5432, 5434, 1360.5, 5382.2, 5428, 5430, 5432, 5434, 1360.5, 5382.2, 5428, 5430, 5432, 5434, 1360.5, 5382.2, 5428, 5430, 5432, 5434, 1360.5 }, {11080, 11082, 7092, 10922, 10440, 263, 11078, 11500, 10440, 263, 11078, 11500, 9184, 7096, 7760, 10038, 7420, 10040, 7418, 9184, 7096, 7760, 10038, 7420, 10040, 7418, 9184, 7096, 7760, 10038, 7420, 10040, 7418, 8960, 9692, 8948, 8722, 8960, 9692, 8948, 8722, 8960, 9692, 8948, 8722, 8960, 9692, 8948, 8722 }, {8358, 11084, 8372, 10878, 9182, 11468, 10878, 9182, 11468, 5456, 7084, 7424, 7396, 10442, 5456, 7084, 7424, 7396, 10442, 5456, 7084, 7424, 7396, 10442,812, 802, 1758, 1758, 7738, 5018, 8468, 8556, 7752, 8474,812, 802, 1758, 1758, 7738, 5018, 8468, 8556, 7752, 8474,812, 802, 1758, 1758, 7738, 5018, 8468, 8556, 7752, 8474,812, 802, 1758, 1758, 7738, 5018, 8468, 8556, 7752, 8474 } };
	std::vector<std::vector<double>> Calendar_2023{ {9000, 11244, 5428,5428, 5430,5430, 5432, 5432, 5434, 5434,11520,11520, 7094, 7094,7764,7764, 7088,7088, 1396,1396, 263,263,14108, 14108,14108, 12918,12918,12918, 7864,7864,7864, 8976,8976,8976, 10486, 10486,10486,10444,10444,10444, 11202,11202,11202,7498,7498,7498,7498, 10488, 10488,10488,10488,8280,8280,8280,8280, 8278,8278,8278,8278},{1354, 10054,9202, 7776, 7090, 7496, 8554, 8480, 11514, 8546,9202, 7776, 7090, 7496, 8554, 8480, 11514, 8546,5446, 14108, 1384, 12918, 11192, 9004, 5446, 14108, 1384, 12918, 11192, 9004,5446, 14108, 1384, 12918, 11192, 9004,5434, 5432, 5430, 5428, 10488, 2564.5, 1360.5, 5434, 5432, 5430, 5428, 10488, 2564.5, 1360.5, 5434, 5432, 5430, 5428, 10488, 2564.5, 1360.5, 5434, 5432, 5430, 5428, 10488, 2564.5, 1360.5},{13688, 11158,7490, 10042, 7768, 10036, 7082, 7086, 9002,7490, 10042, 7768, 10036, 7082, 7086, 9002,10258, 5434, 5432, 5430, 5428, 8556, 12918, 11520, 3676,10258, 5434, 5432, 5430, 5428, 8556, 12918, 11520, 3676,10258, 5434, 5432, 5430, 5428, 8556, 12918, 11520, 3676,14108, 11202, 5446, 10444,14108, 11202, 5446, 10444,14108, 11202, 5446, 10444,14108, 11202, 5446, 10444},{13722, 12302,9202, 6414, 7772, 7080, 7422, 7758, 9202, 6414, 7772, 7080, 7422, 7758,1368.10, 1360.5, 5434, 11192, 6384, 12918, 10032, 11514,1368.10, 1360.5, 5434, 11192, 6384, 12918, 10032, 11514,1368.10, 1360.5, 5434, 11192, 6384, 12918, 10032, 11514,14108, 5382.2, 5432, 5430, 5428,14108, 5382.2, 5432, 5430, 5428,14108, 5382.2, 5432, 5430, 5428,14108, 5382.2, 5432, 5430, 5428},{878, 10922, 8996, 12242,7092, 10440, 11080, 11082, 263, 7092, 10440, 11080, 11082, 263,7418, 8960, 7760, 11078, 10040, 7420, 9184, 10038, 11500,7418, 8960, 7760, 11078, 10040, 7420, 9184, 10038, 11500,7418, 8960, 7760, 11078, 10040, 7420, 9184, 10038, 11500,7096, 9692, 8948, 8722,7096, 9692, 8948, 8722,7096, 9692, 8948, 8722,7096, 9692, 8948, 8722}, {8358, 8944, 10108, 8372, 11084, 9182, 11468, 10878, 7084, 11084, 9182, 11468, 10878, 7084,7424, 9370, 1758, 802, 812, 5456, 5018, 7396, 10442,7424, 9370, 1758, 802, 812, 5456, 5018, 7396, 10442,7424, 9370, 1758, 802, 812, 5456, 5018, 7396, 10442,7738, 8468, 8556, 7752, 8474,7738, 8468, 8556, 7752, 8474,7738, 8468, 8556, 7752, 8474,7738, 8468, 8556, 7752, 8474} };
	std::vector<std::vector<double>> Calendar_2024{ {9000,976,11244, 974,974,5430,5430,11520,11520,7094,7094,7764,7764,7088,7088,1396,1396,263,263, 7864,7864,7864,8976,8976,8976,10486,10486,10486,10444,10444,10444,11202,11202,11202,11518,11518,11518, 7498,7498,7498,7498,10488,10488,10488,10488,8280,8280,8280,8280,8278,8278,8278,8278},{1354,10054, 8546,8546,2036,2036,11514,11514,8480,8480,8554,8554,7496,7496,7090,7090,7776,7776,9202,9202,974,974, 9004,9004,9004,5446,5446,5446,11192,11192,11192,12918,12918,12918,1384,1384,1384, 5432,5432,5432,5432,10488,10488,10488,10488,2564.5,2564.5,2564.5,2564.5,1360.5,1360.5,1360.5,1360.5},{13688,11158, 2036,2036,9002,9002,7086,7086,7082,7082,10036,10036,7768,7768,10042,10042,7490,7490,974,974, 3676,3676,3676,11520,11520,11520,10258,10258,10258,8556,8556,8556,5428,5428,5428, 14108,14108,14108,14108,11202,11202,11202,11202,5446,5446,5446,5446,10444,10444,10444,10444},{13722,976,12302, 7758,7758,7422,7422,7080,7080,7772,7772,6414,6414,9202,9202,974,974, 11514,11514,11514,10032,10032,10032,1368.10,1368.10,1368.10,6384,6384,6384,11192,11192,11192,5434,5434,5434,1360.10,1360.10,1360.10,15006,15006,15006, 5382.2,5382.2,5382.2,5382.2},{878,10922,8996,15018,4538,12242, 10440,10440,263.2,263.2,11082,11082,11108,11108,7092,7092, 11500,11500,11500,8960,8960,8960,7760,7760,7760,11078,11078,11078,10040,10040,10040,7420,7420,7420,9184,9184,9184,7418,7418,7418,10038,10038,10038, 7096,7096,7096,7096,9692,9692,9692,9692,8948,8948,8948,8948,8722,8722,8722,8722},{8358,8944,10108,13266,11818,15020,8372, 9182,9182,7084,7084,10878,10878,11468,11468,11084,11084, 10442,10442,10442,9370,9370,9370,1758,1758,1758,802,802,802,812,812,812,5456,5456,5456,5018,5018,5018,7424,7424,7424,7396,7396,7396, 7738,7738,7738,7738,8468,8468,8468,8468,8556,8556,8556,8556,7752,7752,7752,7752,8474,8474,8474,8474} };
} WinterFest;
struct Edit_ItemV2 {
	std::string Name = "", Desc = "";
	int16_t rarity = 0;
	std::vector<std::pair<int, int>> Extra_Drops;
	int ID = 0, Break_Hits = 0, Far_Punch = 0, Punch_Place = 0, Punch_Hit = 0, Punch_Id = 0, Gems = 0, Xp = 0, Bonus = 0, Item_Price = 0, Change_Drop_Seeds = 0;
	bool property_farmable = false, property_unobtainable = false, property_gacha = false, property_untradeable = false, property_blacklist = false, property_blocked = false;
};
struct GlobalTrade {
	std::string seller;    //nick seller
	std::string buyer;     //nick buyer
	std::string item;      // Item yang ditrade
	int quantity;          // jumlah
	time_t timestamp;
};

std::vector<GlobalTrade> globalTradeHistory;
void AddTradeToGlobalHistory(const std::string& seller, const std::string& buyer, const std::string& item, int quantity) {
	GlobalTrade newTrade;
	newTrade.seller = seller;
	newTrade.buyer = buyer;
	newTrade.item = item;
	newTrade.quantity = quantity;
	newTrade.timestamp = std::time(nullptr); // Waktu sekarang

	globalTradeHistory.push_back(newTrade);
}
struct GiveawayDatas {
	int item_id;
	int amount;
	int giveaway_hour;
	bool active;
};
GiveawayDatas giveaways = { 0, 0, 0, false };
vector<Edit_ItemV2> EditItem;
struct {
	std::string theCCH, last_world, packet_type, packet_logs;
} ServerLogsPkt;
struct SetItem {
	int itemid = 0, fars = 0, places = 0, hit = 0, gems = 0, exp = 0;
};
std::vector<SetItem>setItem;
struct Foodie {
	double perfect, open;
};
std::map<int, Foodie> ingredients{
	{8394, {116.0, 11.0}}, {4450, {93.0, 3.5}}, {978, {60.1, 4.2}}, {8398, {1.2, 4}}, {676, {87.0, 3.2}}, {956, {60.0, 4.1}}, {874, {60.0, 4.1}}, {4602, {62, 5.2}}, {6990, {89, 5.9}}, {712, {9, 3.3}}, {4764, {90, 5}},
};
struct Point {
	int x;
	int y;
};
std::string generate_random_string(int length) {
	static const std::string alphanum = "0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string s;
	for (int i = 0; i < length; ++i) {
		s += alphanum[rand() % (alphanum.size() - 1)];
	}
	return s;
}
std::vector<Point> points;
std::vector<std::string> systemInfo = {
	"`4Global System Message`o: Our developers are dedicated to `5improving `othe server, if you have any bug `5reports `osuggestion please head to `2discord.gg/gtps ``or use '/reportbug'!",
	"`4Global System Message`o: Join `5discord.gg/gtps `ofor more information regarding the server!",
	"`4Global System Message`o: Thanks you for your support by playing this server! If you want to help us more, together, consider sharing this `5GrowTopia Private Server `oto your family & friends!",
	"`4Global System Message`o: Do /tips to see Profit Tips on this Server or you can visit world \"TIPS\"!",
	"`4Global System Message`o: Always remember to save your gems in the bank provided to avoid negatives and if a negative occurs you can fix it by typing /fixmygems",
	"`9Quote of the Day`o: Believe in yourself, take on your challenges, dig deep within yourself to conquer fears. Never let anyone bring you down. You got this.",
	"`9Quote of the Day`o: Believe in yourself. Have faith in your abilities. Without a humble but reasonable confidence in your own powers, you cannot be successful or happy. ",
	"`9Quote of the Day`o: Go confidently in the direction of your dreams. Live the life you have imagined.",
	"`9Quote of the Day`o: It's not about how much time you have, it's about how you use it.",
	"`9Quote of the Day`o: Success does not happen overnight. Keep your eye on the prize and don't look back.",
	"`9Quote of the Day`o: Success is a journey, not a destination."
};
std::vector<std::vector<std::string>> info_about_playmods{
	// playmod id, consumableId_time, playmod name, playmod on used, playmod on removed, display id, state, skin, how work, eff, say text after using, gravity
	{"1", "388", "300", "Stinky", "You really really smell.", "The air clears.", "372", "14", "spray.wav", "", "", "", "", ""},
	{"2", "1368", "2", "Frozen", "Your body has turned to ice.You can't move!", "You've thawed out.", "1368", "11", "freeze.wav", "4284769380", "", "", "", ""},
	{"3", "274", "10","Frozen", "Freeze!", "You've thawed out.", "274", "11", "freeze.wav", "4284769380", "", "", "", ""},
	{"4", "874", "180", "Egged!", "You have egg on your face.", "You washed your face!", "874", "", "", "16777215", "throw", "42", "", ""},
	{"5", "3404", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"6", "3406","0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"7", "4422","0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"8", "386", "0","", "", "", "", "", "", "", "cutewords", "", "", ""},
	{"9", "10660", "1800", "Lucky in Love: Increased chance of Golden Heart Crystal", "Increased chance of Golden Heart Crystal!", "The honeymoon phase is over!", "10660", "", "", "", "", "", ":D`#YUMMY!``", ""},
	{"10", "2206", "1800", "Irradiated", "You are aglow with radiation!", "You have recovered.", "2206", "19", "", "", "", "", "", ""},
	{"11", "408", "300", "Duct Tape", "Duct tape has covered your mouth!", "Duct tape removed. OUCH!", "408", "13", "already_used.wav", "", "", "", "", ""},
	{"12", "384", "3600", "Valentine", "You are somebody's valentine!", "Yuck!", "384", "", "choir.wav", "2526478335", "", "", "", ""},
	{"13", "2480", "90", "Megaphone!", "Broadcasting to ALL!", "You can broadcast once again.", "2480", "", "", "", "", "", "", ""},
	{"14", "528", "1800", "Lucky", "You're luckier than before!", "Your luck has worn off.", "528", "15", "", "", "", "", "", ""},
	{"15", "1510", "10", "1512", "", "", "", "", "", "", "pet", "", "Legend says that you need 10 Blarney Pebbles!", ""},
	{"16", "4672", "1800", "Envious", "It ain't easy being you.", "Healthy color restored.", "4672", "", "eat.wav", "1627349247", "", "", "", ""},
	{"17", "196", "3600", "Feelin' Blue", "A `!blueberry`` slides down your throat!", "The effects of the `!blueberry`` have worn off.", "196", "", "spray.wav", "4278190335", "drop", "", "", ""},
	{"18", "338", "2", "Floating!", "Whoooooooaaaaaaaa...", "Gravity - it's the law.", "338", "", "balloon.wav", "", "drop", "", "", "-30"},
	{"19", "962", "180", "Saucy!", "You are a saucy person.", "You got cleaned up.", "962", "", "", "65535", "throw", "45", "", ""},
	{"20", "950", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"21", "968", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"22", "1058", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"23", "1096", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"24", "868", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"25", "782", "3600", "Antidote!", "You are now immune to zombie bites! Temporarily...", "Your immunity is gone.", "782", "", "", "", "drop", "25", "", ""},
	{"26", "4668", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"27", "128", "1800", "Golden Halo!", "You have been good.", "You're falling out of favor.", "128", "7", "", "-2104114177", "drop", "25", "", ""},
	{"28", "764", "60", "Infected!", "You've been infected by the g-Virus. Punch others to infect them, too! Braiiiins...", "You've been cured.", "764", "16", "", "", "drop", "", "", ""},
	{"29", "5178", "1800", "Lucky", "You're luckier than before!", "Your luck has worn off.", "5178", "15", "", "", "", "", "", ""},
	{"30", "1058", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"31", "1094", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"32", "1096", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"33", "1098", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"34", "2002", "86400","Doctor Replusion", "Doctors won't come near you!", "You no longer repel doctors.", "2002", "", "spray.wav", "", "", "", "", ""},
	{"35", "7058", "300", "Food: Purified Scythe", "Hand Scythe breaks 5% less!", "You no longer feel pure!", "7058", "15", "", "", "", "", "", ""},
	{"36", "1056", "1800", "Lucky", "You're luckier than before!", "Your luck has worn off.", "1056", "15", "", "", "", "", "", ""},
	{"37", "4378", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"38", "614", "300", "Rotten Egg", "You really really smell.", "The air clears.", "614", "14", "spray.wav", "", "", "", "", ""},
	{"39", "1374", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"40", "406", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"41", "966", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"42", "4766", "1800", "Blushing Red", "`4Cherry`` red lips!", "The effects of the `!Cherry`` have worn off.", "4766", "", "spray.wav", "842203135", "", "", "", ""},
	{"43", "958", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"44", "950", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"45", "1580", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"46", "7052", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"47", "1634", "5", "Caffeinated", "You are full of caffeine!", "Ugh. Caffeine crash.", "1634", "14", "spray.wav", "", "", "", "", ""},
	{"48", "4666", "0", "/cry", "", "", "", "", "", "", "consume", "", "", ""},
	{"49", "6314", "2", "Frozen", "Your body has turned to ice. You can't move!", "You've thawed out.", "6314", "11", "freeze.wav", "4284769380", "", "", "", ""},
	{"50", "3064", "0", "", "", "", "3064", "", "", "", "throw", "34", "", ""},
	{"51", "8520", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"52", "10628", "0", "/dab", "", "", "", "", "", "", "consume", "", "", ""},
	{"53", "1474", "1800", "Food: Extra XP", "25% chance of double XP for all actions.", "Your stomach's rumbling.", "1474", "", "spray.wav", "", "drop", "", "", ""},
	{"54", "3546", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"55", "3600", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"56", "3836", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"57", "3240", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"58", "4410", "0", ":P`#Bleh.``", "", "", "", "", "", "", "consume", "", "", ""},
	{"59", "4752", "0", ":D`#YUM!``", "", "", "", "", "", "", "cutewords", "", "", ""},
	{"60", "2734", "0", ":D`#YUM!``", "", "", "", "", "", "", "cutewords", "", "", ""},
	{"61", "3622", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"62", "10988", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"63", "4764", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"64", "964", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"65", "3428", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"66", "3816", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"67", "126", "1800", "Devil Horns", "You little devil...", "Off the naughty list!", "126", "6", "spray.wav", "", "drop", "", "", ""},
	{"68", "1964", "1800", "Devil Horns", "You little devil...", "Off the naughty list!", "1964", "6", "spray.wav", "", "drop", "", "", ""},
	{"69", "960", "5", "ON FIRE!!!", "You are burning up!", "The fire's out.", "960", "17", "spray.wav", "842203135", "", "", "", ""},
	{"70", "712", "5", "ON FIRE!!!", "You are burning up!", "The fire's out.", "712", "17", "spray.wav", "842203135", "", "", "", ""},
	{"71", "1988", "1800", "Haunted!", "You are haunted!", "Not anymore.", "372", "18", "spray.wav", "", "", "", "", ""},
	{"72", "1772", "2", "Floating!", "Whoooooooaaaaaaaa...", "Gravity - it's the law.", "1772", "", "balloon.wav", "", "drop", "", "", "-30"},
	{"73", "5262", "5", "Neon Gum!", "Dazzle your friends as super funky neon lines course across your skin.", "Party's over.", "5262", "28", "spray.wav", "", "drop", "", "", ""},
	{"74", "676", "86400","Doctor Replusion", "Doctors won't come near you!", "You no longer repel doctors.", "676", "", "spray.wav", "", "", "", "", ""},
	{"75", "276", "0", "", "", "", "", "", "", "", "cutewords", "", "", ""},
	{"76", "732", "60", "Banned", "Reality flickers as you begin to wake up.", "You are no longer banned. Now be good!", "732", "12", "", "", "", "", "", "" },
	{"77", "618", "0", "", "", "", "", "", "", "", "cutewords", "", "", ""},
	{"78", "616", "0", "", "", "", "", "", "", "", "cutewords", "", "", ""},
	{"79", "614", "0", ":P", "", "", "", "", "", "", "consume", "", "", ""},
	{"80", "750", "1800", "Lucky", "You're luckier than before!", "Your luck has worn off.", "750", "15", "", "", "", "", "", ""},
	{"81", "752", "0", "", "", "", "", "", "", "", "cutewords", "25", "", ""},
	{"82", "1208", "0", ":D`#YUM!``", "", "", "", "", "", "", "consume", "", "", ""},
	{"83", "5114", "120", "Calm Nerves", "Steady hands of a surgeon.", "Butterfingers again.", "5114", "14", "spray.wav", "", "", "", "", ""},
	{"84", "6912", "1800", "Spicey Skills", "Reduce your skill fails by half in both Surgery and Startopia missions.", "Your stomach's too spicey.", "6912", "", "spray.wav", "", "", "", "", ""},
	{"85", "3536", "0", "Mmm, sugar!", "", "", "", "", "", "", "consume", "", "", ""},
	{"86", "1260", "3600", "Malpractice", "You are not allowed to perform surgery for a while!", "You can surg again.", "1260", "", "", "", "", "", "", ""},
	{"87", "1270", "3600", "Recovering", "You are recovering from surgery.", "You healed.", "1270", "", "", "", "", "", "", ""},
	{"88", "8500", "0", "", "", "", "", "", "", "", "cutewords", "", "", ""},
	{"89", "4602", "300", "Stinky", "You really really smell.", "The air clears.", "4602", "14", "spray.wav", "", "", "", "", ""},
	{"90", "8544", "43200", "Lupus malady", "You've been infected with lupus!", "You no longer have lupus!", "8544", "", "spray.wav", "", "", "", "", ""},
	{"91", "8540", "43200", "Moldy Guts malady", "You've been infected with moldy guts!", "You no longer have moldy guts!", "8540", "", "spray.wav", "", "", "", "", ""},
	{"92", "8546", "43200", "Ecto-Bones malady", "You've been infected with Ecto-Bones!", "Your bones are no longer ectoplasmic!", "8546", "", "spray.wav", "", "", "", "", ""},
	{"93", "8538", "43200", "Chaos Infection malady", "You've been infected with CHAOS!", "You no longer have CHAOS!", "8538", "", "spray.wav", "", "", "", "", ""},
	{"94", "8548", "43200", "Fatty Liver malady", "You've been infected with fatty liver!", "You no longer have fatty liver!", "8548", "", "spray.wav", "", "", "", "", ""},
	{"95", "8542", "43200", "Brainworms malady", "You've been infected with brainworms!", "You no longer have brainworms!", "8542", "", "spray.wav", "", "", "", "", ""},
	{"96", "6908", "1800", "Food: Buff Duration", "Increase the duration of food buffs by 30%", "Your stomach can't take anymore buffs!", "6908", "", "spray.wav", "", "drop", "", "", ""},
	{"97", "5452", "1800", "Food: Extra XP", "10% chance of triple XP for all actions.", "Your body craves sugar!", "5452", "", "spray.wav", "", "drop", "", "", "" },
	{"98", "2734", "1200", "Ultimate Super Pineapple Magic", "Ultimate Super Pineapple", "The ULTIMATE Super Pineapple Magic has come to an end!", "2734", "25", "", "", "", "", "", "" },
	{"99", "8384", "10", "Gross Bean", "You really really smell.", "The air clears.", "8384", "14", "spray.wav", "", "", "", "", ""},
	{"100", "4256", "500", "Popcorn", "Popcorn!", "", "4256", "", "", "", "consume", "206", "", "" },
	{ "101", "5406", "300", "Red Winterfest Crown!", "You're in the Winterfest spirit.", "Your paper crown dissolved in your forehead sweat!", "5406", "", "spray.wav", "", "", "", "", "" },
	{ "102", "5408", "300", "Green Winterfest Crown!", "You're in the Winterfest spirit.", "Your paper crown dissolved in your forehead sweat!", "5408", "", "spray.wav", "", "", "", "", "" },
	{ "103", "5410", "300", "Silver Winterfest Crown!", "You're in the Winterfest spirit.", "Your paper crown dissolved in your forehead sweat!", "5410", "", "spray.wav", "", "", "", "", "" },
	{ "104", "5412", "300", "Gold Winterfest Crown!", "You're in the Winterfest spirit.", "Your paper crown dissolved in your forehead sweat!", "5412", "", "spray.wav", "", "", "", "", "" },
	{ "105", "4232", "0", "`#:D YUM!``", "", "", "4232", "", "", "", "consume", "204", "", "" },
	{ "106", "4324", "1800", "Extra XP", "Your brain has been enhanced!", "You're getting dumber by the second.", "4324", "", "spray.wav", "", "drop", "", "", "" },
	{ "107", "4596", "1800", "Food: Surgery XP", "Gain 30% more XP from Surgery!", "Your stomach's rumbling.", "4596", "", "spray.wav", "", "drop", "", "", "" },
	{ "108", "11068", "0", "`#:D Ahhh! That is REFRESHING!``", "", "", "11068", "", "", "", "consume", "204", "", "" },
	{ "109", "6050", "1800", "Food: Pet Gems 200", "Get 200 Gems for beating a Pet Trainer.", "Your stomach's rumbling.", "6050", "", "spray.wav", "", "drop", "", "", "" },
	{ "110", "6048", "1800", "Food: Pet Gems 150", "Get 150 Gems for beating a Pet Trainer.", "Your stomach's rumbling.", "6048", "", "spray.wav", "", "drop", "", "", "" },
	{ "111", "6046", "1800", "Food: Pet Gems 100", "Get 100 Gems for beating a Pet Trainer.", "Your stomach's rumbling.", "6046", "", "spray.wav", "", "drop", "", "", "" },
	{ "112", "6910", "1800", "Food: Delectable", "25% chance of double Growtokens from Daily Quests.", "Your stomach's devasted!", "6910", "", "spray.wav", "", "drop", "", "", "" },
	{ "113", "4604", "1800", "Food: Breaking Gems", "10% chance of a gem when you break a block.", "Your stomach's rumbling.", "4604", "", "spray.wav", "", "drop", "", "", "" },
	{ "114", "3728", "6", "Slimed!", "You're covered in ectoplasm!", "You got cleaned up.", "3728", "", "spray.wav", "", "drop", "", "", "" },
	{ "115", "1602", "900", "Minty", "Ya'll are feelin' minty, sugah.", "Healthy color restored.", "1602", "", "eat.wav", "1627349247", "", "", "", "" },
	{ "116", "12152", "600", "Janeway's Coffee: Speedy", "On my mark... Go Fast!", "Speed normalized!", "12152", "14", "spray.wav", "", "", "", "", "" },
	{ "117", "4982", "180", "Orange", "You are a orange person.", "You got cleaned up.", "4982", "", "spray.wav", "9240575", "", "", "", "" },
	{ "118", "4594", "1800", "Food: Tree Growth food buff", "Trees you plant grow 5% faster!", "Your stomach's rumbling.", "4594", "", "spray.wav", "", "drop", "", "", "" },
	{ "119", "1662", "5", "Spikeproof", "You are briefly immune to spikes and lava", "You feel vulnerable again.", "1662", "10", "", "", "", "", "", "" },
	{ "120", "733", "120", "BAN Cooldown!", "BAN cooldown!", "Your ban cooldown is gone.", "732", "", "", "", "", "", "", "" },
	{ "121", "2480", "1800", "VIP-SB!", "Broadcasting to ALL!", "You can broadcast once again.", "2480", "", "", "", "", "", "", "" },
	{ "122", "2480", "90", "OWNER-SB!", "Broadcasting to ALL!", "You can broadcast once again.", "2480", "", "", "", "", "", "", "" },
	{ "123", "368", "6", "Muddy", "You've been splattered with mud!", "Clean again!", "368", "", "spray.wav", "1348237567", "", "", "", "" },
	{ "124", "10490", "1800", "Performance Enhanced!", "Increased Points with Gloves!", "Get back to the competition!", "10490", "", "spray.wav", "", "", "", "", "" },
	{ "125", "9852", "2678400", "Moderator Role", "You have received Moderator role!", "Moderator role is over...", "9852", "", "spray.wav", "", "drop", "", "", "" },
	{ "126", "9854", "2678400", "VIP Role", "You have received VIP role!", "VIP role is over...", "9854", "", "spray.wav", "", "drop", "", "", "" },
	{ "127", "9018", "1200", "Dark Ticket", "Welcome to TOMB OF GROWGANOTH!", "Your journey is over...", "9018", "", "boo_evil_laugh.wav", "", "drop", "", "", "" },
	{ "128", "2992", "1200", "Wolf Whistle", "Welcome to Wolf World!", "Your journey is over...", "2992", "", "boo_evil_laugh.wav", "", "drop", "", "", "" },
	{ "129", "9266", "86400", "1-Day Subscription Token", "Received 1 Day Subscribtion", "Your journey is over...", "9266", "", "spray.wav", "", "drop", "", "", "Subscribtion" },
	{ "130", "6856", "259200", "3-Day Subscription Token", "Received 3 Day Subscribtion", "Your journey is over...", "6856", "", "spray.wav", "", "drop", "", "", "Subscribtion" },
	{ "131", "6858", "1209600", "14-Day Subscription Token", "Received 14 Day Subscribtion", "Your journey is over...", "6858", "", "spray.wav", "", "drop", "", "", "Subscribtion" },
	{ "132", "6860", "2592000", "30-Day Subscription Token", "Received 30 Day Subscribtion", "Your journey is over...", "6860", "", "spray.wav", "", "drop", "", "", "Subscribtion" },
	{ "133", "8188", "31536000", "1-Year Subscription Token", "Received 1 Year Subscribtion", "Your journey is over...", "8188", "", "spray.wav", "", "drop", "", "", "Subscribtion" },
	{ "134", "11400", "1800", "Energy Drink: Double XP & gems", "Double gems & XP!", "Your stomach's rumbling.", "11400", "", "eat.wav", "", "drop", "", "", "" },
	{ "135", "4830", "3", "Balloon Immunity", "Balloons can't hit you, but you can't throw them either.", "You can now throw balloons and get hit by them too.", "4830", "15", "eat.wav", "", "drop", "", "", "" },
	{ "136", "4844", "600", "Dripping Wet!", "You've been hit, you're too wet to throw balloons.", "All dry, go nuts.", "4844", "27", "", "", "throw", "0", "", "" },
	{ "137", "4842", "600", "Balloon-Proof", "Balloon repellent applied.", "Your stomach's rumbling.", "4842", "", "spray.wav", "", "drop", "", "", "" },
	{ "138", "4882", "600", "Towel Rack Checkpoint", "Towel Rack Checkpoint cooldown.", "You can hit the checkpoint again!", "4882", "", "spray.wav", "", "drop", "", "", "" },
	{ "139", "278", "60", "Curse", "You've been cursed to (the world) `4hell``!", "You are free to go!", "278", "12", "", "", "", "", "", "" },
	{ "140", "4984", "0", "`#:D YUM!``", "", "", "4984", "", "", "", "consume", "204", "", "" },
	{ "141", "9904", "600", "Stinky", "You really really smell.", "The air clears.", "9904", "14", "spray.wav", "", "", "", "", ""},
	{ "142", "2480", "60", "Legend SB!", "Broadcasting to ALL!", "You can broadcast once again.", "2480", "", "", "", "", "", "", "" },
	{ "143", "9726", "604800", "Cheater Role G", "You have received cheater role!", "Cheater role is over...", "9726", "", "spray.wav", "", "drop", "", "", "" },
	{ "144", "12600", "250", "Ultra World Spray", "Calmdown with the world spray..", "You can now use ultra world spray again.", "12600", "", "", "", "", "", "", "" },
	{ "145", "6914", "1800", "Sparkling Gems", "Get extra gems from a variety of actions!", "Your stomach has too many bubbles!", "6914", "", "spray.wav", "", "drop", "", "", "" },
	{ "146", "5940", "1800", "Guild Potion: Fishing", "The Guild Leader released the magic!", "The effects of the Guild Potion have ended!", "5940", "", "spray.wav", "", "drop", "", "", "" },
	{ "147", "5942", "1800", "Guild Potion: Blocks", "The Guild Leader released the magic! 1% chance of Extra Blocks!", "The effects of the Guild Potion have ended!", "5942", "", "spray.wav", "", "drop", "", "", "" },
	{ "148", "5934", "1800", "Guild Potion: Geiger", "The Guild Leader released the magic!", "The effects of the Guild Potion have ended!", "5934", "", "spray.wav", "", "drop", "", "", "" },
	{ "149", "5938", "1800", "Guild Potion: Gems", "The Guild Leader released the magic! 1% bonus chance of Extra Gems!", "The effects of the Guild Potion have ended!", "5938", "", "spray.wav", "", "drop", "", "", "" },
	{ "150", "2446", "10800", "Extra XP & Extra Wealth & Calm Nerves", "J A Night Luck for everything applied! J", "You lost your night luck... J", "2446", "", "", "", "", "", "", "" },
	{ "151", "5984", "1800", "High Jump", "Your feet spring to life", "Your feet shrink back to normal", "5984", "", "spray.wav", "", "drop", "", "", "" },
	{ "152", "3742", "60", "Spray Tan", "You are orange.", "The spray tan wore off.", "3742", "", "spray.wav", "613939967", "", "", "", "" },
	{ "153", "10400", "86400", "Cheater Role G", "You have received cheater role!", "Cheater role is over...", "10400", "", "spray.wav", "", "drop", "", "", "" },
	{ "124", "1280_2592000", "Recently Name-Changed", "You changed your name.``", "You can now change your name again!", "1280", "", "", "", "", "", "", "" },
	{ "154", "7056", "1800", "Food: Extra XP", "25% chance of double XP for catching ghosts.", "Your stomach's rumbling.", "7056", "", "spray.wav", "", "drop", "", "", "" },
	{ "155", "2480", "60", "MODS-SB!", "Broadcasting to ALL!", "You can broadcast once again.", "2480", "", "", "", "", "", "", "" },
	{ "156", "5936", "1800", "Guild Potion: Grow", "The Guild Leader released the magic! 1% bonus chance of Extra Grow!", "The effects of the Guild Potion have ended!", "5936", "", "spray.wav", "", "drop", "", "", "" },
	{ "157", "1280_604800", "Recently Pet Name-Changed", "You changed your name.", "You can now change your name again!", "1280", "", "spray.wav", "", "drop", "", "", "" },
	{ "158", "14526", "7200", "Miner's License", "You are miner's.", "Miner's License is over...", "14526", "", "spray.wav", "", "drop", "", "", "" },
	{ "159", "1432", "7200", "Report-Bug Cooldown", "You are a good person.", "Report-Bug Cooldown is over...", "1432", "", "spray.wav", "", "drop", "", "", "" },
	{ "160", "13994", "120", "Elder Sigil", "Growganoth pulls you closer!", "Elder Sigil Cooldown is over...", "13994", "", "spray.wav", "", "drop", "", "", "" },
	{ "161", "9216", "1500", "Compelitor: Finale Ticket", "You're in the running!", "You run the Finale!", "9216", "", "spray.wav", "", "drop", "", "", "" },
	{ "162", "9216", "3600", "Winner: Finale Ticket", "You're in the running!", "You run the Finale!", "9216", "", "spray.wav", "", "drop", "", "", "" },
	{ "163", "1", "5","Star Platinum", "The World", "What was that?", "1482", "11", "freeze.wav", "4284769380", "", "", "", "" }
};
std::vector<std::vector<std::vector<int>>> crystal_receptai = {
	{{2242 /*red*/, 2}, {2244 /*green*/, 0}, {2246 /*blue*/, 0}, {2248 /*white*/, 3}, {2250 /*black*/, 0}, {2254, 1}},
	{{2242 /*red*/, 1}, {2244 /*green*/, 1}, {2246 /*blue*/, 0}, {2248 /*white*/, 3}, {2250 /*black*/, 0}, {2262, 1}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 3}, {2246 /*blue*/, 0}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {2942, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 0}, {2246 /*blue*/, 2}, {2248 /*white*/, 3}, {2250 /*black*/, 0}, {2258, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 1}, {2246 /*blue*/, 1}, {2248 /*white*/, 3}, {2250 /*black*/, 0}, {2264, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 3}, {2246 /*blue*/, 2}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {2944, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 2}, {2246 /*blue*/, 0}, {2248 /*white*/, 3}, {2250 /*black*/, 0}, {2256, 1}},
	{{2242 /*red*/, 1}, {2244 /*green*/, 0}, {2246 /*blue*/, 1}, {2248 /*white*/, 3}, {2250 /*black*/, 0}, {2260, 1}},
	{{2242 /*red*/, 3}, {2244 /*green*/, 0}, {2246 /*blue*/, 2}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {2940, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 0}, {2246 /*blue*/, 0}, {2248 /*white*/, 0}, {2250 /*black*/, 5}, {2212, 1}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 0}, {2246 /*blue*/, 2}, {2248 /*white*/, 1}, {2250 /*black*/, 0}, {2972, 1}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 1}, {2246 /*blue*/, 2}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {2268, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 2}, {2246 /*blue*/, 0}, {2248 /*white*/, 1}, {2250 /*black*/, 2}, {2266, 1}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 0}, {2246 /*blue*/, 0}, {2248 /*white*/, 0}, {2250 /*black*/, 3}, {2754, 1}},
	{{2242 /*red*/, 1}, {2244 /*green*/, 0}, {2246 /*blue*/, 0}, {2248 /*white*/, 3}, {2250 /*black*/, 1}, {2756, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 3}, {2246 /*blue*/, 0}, {2248 /*white*/, 2}, {2250 /*black*/, 0}, {2706, 1}},
	{{2242 /*red*/, 1}, {2244 /*green*/, 1}, {2246 /*blue*/, 1}, {2248 /*white*/, 0}, {2250 /*black*/, 2}, {3180, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 0}, {2246 /*blue*/, 0}, {2248 /*white*/, 2}, {2250 /*black*/, 3}, {4124, 1}},
	{{2242 /*red*/, 3}, {2244 /*green*/, 0}, {2246 /*blue*/, 0}, {2248 /*white*/, 2}, {2250 /*black*/, 0}, {5268, 1}},
	{{2242 /*red*/, 3}, {2244 /*green*/, 1}, {2246 /*blue*/, 0}, {2248 /*white*/, 1}, {2250 /*black*/, 0}, {5266, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 0}, {2246 /*blue*/, 0}, {2248 /*white*/, 1}, {2250 /*black*/, 4}, {6848, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 0}, {2246 /*blue*/, 3}, {2248 /*white*/, 2}, {2250 /*black*/, 0}, {6820, 1}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 2}, {2246 /*blue*/, 3}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {2272, 3}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 0}, {2246 /*blue*/, 3}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {2276, 5}},
	{{2242 /*red*/, 0}, {2244 /*green*/, 2}, {2246 /*blue*/, 1}, {2248 /*white*/, 2}, {2250 /*black*/, 0}, {2252, 1}},
	{{2242 /*red*/, 3}, {2244 /*green*/, 0}, {2246 /*blue*/, 1}, {2248 /*white*/, 1}, {2250 /*black*/, 0}, {2274, 1}},
	{{2242 /*red*/, 1}, {2244 /*green*/, 2}, {2246 /*blue*/, 2}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {2270, 10}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 1}, {2246 /*blue*/, 1}, {2248 /*white*/, 1}, {2250 /*black*/, 0}, {2590, 5}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 1}, {2246 /*blue*/, 1}, {2248 /*white*/, 1}, {2250 /*black*/, 0}, {2812, 1}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 2}, {2246 /*blue*/, 0}, {2248 /*white*/, 1}, {2250 /*black*/, 0}, {3424, 1}},
	{{2242 /*red*/, 2}, {2244 /*green*/, 2}, {2246 /*blue*/, 1}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {3770, 3}}, {{2242 /*red*/, 3}, {2244 /*green*/, 2}, {2246 /*blue*/, 0}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {3526, 20}}, {{2242 /*red*/, 2}, {2244 /*green*/, 2}, {2246 /*blue*/, 0}, {2248 /*white*/, 0}, {2250 /*black*/, 1}, {3582, 1}}, {{2242 /*red*/, 2}, {2244 /*green*/, 0}, {2246 /*blue*/, 2}, {2248 /*white*/, 0}, {2250 /*black*/, 1}, {4762, 1}}, {{2242 /*red*/, 2}, {2244 /*green*/, 1}, {2246 /*blue*/, 0}, {2248 /*white*/, 2}, {2250 /*black*/, 0}, {5204, 1}}, {{2242 /*red*/, 1}, {2244 /*green*/, 0}, {2246 /*blue*/, 2}, {2248 /*white*/, 2}, {2250 /*black*/, 0}, {5106, 1}}, {{2242 /*red*/, 1}, {2244 /*green*/, 1}, {2246 /*blue*/, 1}, {2248 /*white*/, 2}, {2250 /*black*/, 0}, {5104, 1}}, {{2242 /*red*/, 1}, {2244 /*green*/, 2}, {2246 /*blue*/, 1}, {2248 /*white*/, 1}, {2250 /*black*/, 0}, {5272, 10}}, {{2242 /*red*/, 1}, {2244 /*green*/, 3}, {2246 /*blue*/, 1}, {2248 /*white*/, 0}, {2250 /*black*/, 1}, {5274, 1}}, {{2242 /*red*/, 1}, {2244 /*green*/, 1}, {2246 /*blue*/, 1}, {2248 /*white*/, 3}, {2250 /*black*/, 0}, {5270, 1}}, {{2242 /*red*/, 1}, {2244 /*green*/, 3}, {2246 /*blue*/, 1}, {2248 /*white*/, 0}, {2250 /*black*/, 0}, {5280, 10}}, {{2242 /*red*/, 3}, {2244 /*green*/, 0}, {2246 /*blue*/, 0}, {2248 /*white*/, 1}, {2250 /*black*/, 1}, {392, 1}}, {{2242 /*red*/, 4}, {2244 /*green*/, 0}, {2246 /*blue*/, 4}, {2248 /*white*/, 5}, {2250 /*black*/, 5}, {12304, 1}}, {{2242 /*red*/, 10}, {2244 /*green*/, 0}, {2246 /*blue*/, 10}, {2248 /*white*/, 10}, {2250 /*black*/, 10}, {9886, 1}}
};
std::map<std::string, std::pair<int, int>> Gp_Prize{
	/*Page 1 - Atas*/{"p2p_150", {14990, 1}}, {"p2p_300", {15016, 1}}, {"p2p_450", {10858, 1}}, {"p2p_600", {1486, 2}}, {"p2p_750", {2480, 1}}, {"p2p_900", {15014, 1}},
	/*Page 1 - Bawah*/{"f2p_150", {5706, 20}}, {"f2p_300", {10842, 1}}, {"f2p_600", {10838, 1}}, {"f2p_900", {1486, 2}},
	/*Page 2 - Atas*/ {"p2p_1100", {8196, 4}}, {"p2p_1300", {3684, 1}}, {"p2p_1500", {10858, 2}}, {"p2p_1700", {3562, 1}}, {"p2p_1900", {2480, 1}}, {"p2p_2100", {14992, 1}},
	/*Page 2 - Bawah*/{"f2p_1300", {10834, 1}}, {"f2p_1700", {10836, 1}}, {"f2p_2100", {10858, 1}},
	/*Page 3 - Atas*/ {"p2p_2350", {1488, 1}}, {"p2p_2600", {1486, 2}}, {"p2p_2850", {10858, 2}}, {"p2p_3100", {8196, 4}}, {"p2p_3350", {2480, 1}}, {"p2p_3600", {15012, 1}},
	/*Page 3 - Bawah*/{"f2p_2600", {2992, 1}}, {"f2p_3100", {1486, 5}}, {"f2p_3600", {10858, 2}},
	/*Page 4 - Atas*/ {"p2p_3900", {942, 1}}, {"p2p_4200", {1486, 5}}, {"p2p_4500", {10858, 2}}, {"p2p_4800", {2992, 1}}, {"p2p_5100", {2480, 1}}, {"p2p_5400", {13574, 1}},
	/*Page 4 - Bawah*/{"f2p_4200", {2478, 5}}, {"f2p_4800", {10842, 1}}, {"f2p_5400", {14990, 1}}
};
std::map<int, int> Pet_Food{ {196, 5}, {712, 25}, {868, 1}, {874, 1}, {950, 50}, {3428, 2}, {3836, 50}, {4586, 25}, {4602, 2}, {4766, 25}, {962, 2}, {968, 50} };
struct FoodR {
	std::vector<int> recipe{};
	std::vector<int> spices{};
	int spicepoint = 0, reward = 0, percentage = 0;
};
std::map<int, FoodR> foodrecipes{
	{0, {{8394, 4450, 978, 8398}, {}, 0, 1200, 60} },
	{1, {{4764, 4764}, {}, 1, 11002, 90} },
	{2, {{6990, 4602, 712, 8398}, {4568, 4568, 4568}, 1, 13704, 100} },
};
int get_cook_offset(int food) {
	int expected = 0;
	int items = food;
	while (items >= 256) {
		items -= 256;
		expected += 1;
	}
	switch (food) {
	case 4586:
	case 4588:
		return 17;
	case 4764:
		return 18;
	case 3428:
		return 13;
	case 868:
	case 822:
	case 962:
		return 3;
	case 540:
		return 2;
	case 3468:
		return 13;
	case 4578:
		return 17;
	case 4610:
	case 4766:
	case 6314:
	case 7054:
	case 4670:
	case 712:
	case 4572:
	case 7014:
	case 3472:
	case 4602:
	case 8392:
	case 8394:
	case 8396:
	case 4568:
	case 8398:
	case 614:
	case 4666:
	case 5416:
	case 4984:
	case 7000:
	case 7004:
	case 8252:
	case 4410:
	case 8384:
	case 6958:
	case 8064:
	case 4672:
	case 676:
	case 4450:
	case 978:
	case 5376:
	case 4560:
	case 664:
	case 4564:
	case 6974:
	case 196:
	case 874:
	case 4562:
	case 6990:
	case 7462:
	case 1198:
	case 3836:
	case 4624:
	case 956:
	case 3622:
	case 4580:
	case 4674:
	case 4570:
		return expected;
	}
	return 0;
}
int get_cook_id(int food) {
	int expected = food;
	while (expected >= 256) {
		expected -= 256;
	}
	switch (food) {
	case 3468:
		return 140;
	case 4578:
		return 226;
	case 4586:
		return 234;
	case 4588:
		return 236;
	case 540:
		return 28;
	case 3428:
	case 868:
		return 100;
	case 822:
		return 54;
	case 962:
		return 194;
	case 4764:
		return 156;
	case 4610:
	case 4766:
	case 6314:
	case 7054:
	case 4670:
	case 712:
	case 4572:
	case 7014:
	case 3472:
	case 4602:
	case 8392:
	case 8396:
	case 4568:
	case 8398:
	case 614:
	case 4666:
	case 5416:
	case 4984:
	case 7000:
	case 7004:
	case 8252:
	case 4410:
	case 8384:
	case 6958:
	case 8064:
	case 4672:
	case 676:
	case 4450:
	case 978:
	case 5376:
	case 4560:
	case 8394:
	case 664:
	case 4564:
	case 6974:
	case 196:
	case 874:
	case 4562:
	case 6990:
	case 7462:
	case 1198:
	case 3836:
	case 4624:
	case 956:
	case 3622:
	case 4580:
	case 4674:
	case 4570:
		return expected;
		//100 Milk, 54 Water, 194 Tomato
	}
	return 0;
}
std::vector<std::vector<std::vector<int>>> receptai;
struct dotted : numpunct<char> {
	char do_thousands_sep()   const { return ','; }
	std::string do_grouping() const { return "\3"; }
};
inline std::string Set_Count(int gems) {
	static const std::locale dotted_loc(std::locale::classic(), new dotted);
	stringstream ss;
	ss.imbue(dotted_loc);
	ss << gems;
	return ss.str();
}
class CommaNumpunct : public numpunct<char> {
protected:
	virtual char do_thousands_sep() const {
		return ',';
	}
	virtual std::string do_grouping() const {
		return "\3";
	}
};
inline std::string formatWithCommas(long long int value) {
	static const std::locale comma_loc(std::locale::classic(), new CommaNumpunct());
	stringstream ss;
	ss.imbue(comma_loc);
	ss << value;
	return ss.str();
}
inline std::string getStrUpper(const std::string& txt) {
	std::string ret;
	ret.reserve(txt.length());
	for (unsigned char c : txt) ret += static_cast<char>(toupper(c));
	return ret;
}
inline std::string fixchar2(const std::string& name) {
	std::string ret;
	ret.reserve(name.length());
	for (size_t i = 0; i < name.length(); i++) {
		if (name[i] == '`') {
			i++;
		} else {
			ret += name[i];
		}
	}
	return ret;
}
inline std::string fixchar4(const std::string& name) {
	std::string ret;
	ret.reserve(name.length());
	for (size_t i = 0; i < name.length(); i++) {
		if (name[i] == '`') i++;
		else ret += name[i];
	}
	return ret;
}
inline uint32_t name_to_number(const std::string& str) {
	std::uint32_t hash = 0x811c9dc5;
	std::uint32_t prime = 0x1000193;
	for (int i = 0; i < str.size(); ++i) {
		std::uint8_t value = str[i];
		hash = hash ^ value;
		hash *= prime;
	}
	hash /= 100;
	return hash;
}
inline std::string fixchar3(const std::string& str) {
	std::string ret;
	ret.reserve(str.length());
	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		if (c == '`') {
			i++;
			continue;
		}
		if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
		if (c >= 'a' && c <= 'z') {
			ret += c;
		}
	}
	return ret;
}
inline int randombetween(int min, int max) {
	if (min >= max) return min;
	static thread_local std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}
inline std::string to_lower(const std::string& s) {
	std::string result = s;
	for (char& c : result) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
	return result;
}
inline bool iequals(std::string_view a, std::string_view b) {
	if (a.length() != b.length()) return false;
	return std::equal(a.begin(), a.end(), b.begin(), b.end(),
		[](char c1, char c2) {
			return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2));
		});
}
inline std::string replace_str2(std::string str, const std::string& from, const std::string& to) {
	if (from.empty()) return str;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}
string cleanup_(string strText) {
	string temp = "";
	for (int i = 0; i < strText.size(); ++i) {
		if ((strText[i] >= 'a' && strText[i] <= 'z') || (strText[i] >= 'A' && strText[i] <= 'Z')) {
			temp = temp + strText[i];
		}
		else {
			temp = temp + " ";
		}
	}
	return temp;
}
typedef struct _CL_Vec2i {
	int x;
	int y;
	_CL_Vec2i() {
		x = 0;
		y = 0;
	}
	_CL_Vec2i(int f) {
		x = f;
		y = f;
	}
	_CL_Vec2i(int x, int y) {
		this->x = x;
		this->y = y;
	}
	_CL_Vec2i(uint32_t x, uint32_t y) {
		this->x = x;
		this->y = y;
	}
	bool operator==(const _CL_Vec2i& other) const { return x == other.x && y == other.y; }
	bool operator!=(const _CL_Vec2i& other) const { return x != other.x || y != other.y; }
	_CL_Vec2i operator+(const _CL_Vec2i& other) const { return _CL_Vec2i(x + other.x, y + other.y); }
	_CL_Vec2i operator-(const _CL_Vec2i& other) const { return _CL_Vec2i(x - other.x, y - other.y); }
	_CL_Vec2i operator*(const int& other) const { return _CL_Vec2i(x * other, y * other); }
} CL_Vec2i;
typedef struct _CL_Vec2f {
	float x;
	float y;
	_CL_Vec2f() {
		x = 0.0f;
		y = 0.0f;
	}
	_CL_Vec2f(float f) {
		x = f;
		y = f;
	}
	_CL_Vec2f(float x, float y) {
		this->x = x;
		this->y = y;
	}
	bool operator==(const _CL_Vec2f& other) const { return x == other.x && y == other.y; }
	bool operator!=(const _CL_Vec2f& other) const { return x != other.x || y != other.y; }
	_CL_Vec2f operator+(const _CL_Vec2f& other) const { return _CL_Vec2f(x + other.x, y + other.y); }
	_CL_Vec2f operator-(const _CL_Vec2f& other) const { return _CL_Vec2f(x - other.x, y - other.y); }
	_CL_Vec2f operator*(const float& other) const { return _CL_Vec2f(x * other, y * other); }
} CL_Vec2f;
typedef struct _CL_Vec3f {
	float x;
	float y;
	float z;
	_CL_Vec3f() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	_CL_Vec3f(float f) {
		x = f;
		y = f;
		z = f;
	}
	_CL_Vec3f(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	_CL_Vec3f(uint16_t x, uint16_t y, uint16_t z) {
		this->x = static_cast<float>(x);
		this->y = static_cast<float>(y);
		this->z = static_cast<float>(z);
	}
	bool operator==(const _CL_Vec3f& other) const {
		return x == other.x && y == other.y && z == other.z;
	}
	bool operator!=(const _CL_Vec3f& other) const {
		return x != other.x || y != other.y || z != other.z;
	}
	_CL_Vec3f operator+(const _CL_Vec3f& other) const {
		return _CL_Vec3f(x + other.x, y + other.y, z + other.z);
	}
	_CL_Vec3f operator-(const _CL_Vec3f& other) const {
		return _CL_Vec3f(x - other.x, y - other.y, z - other.z);
	}
	_CL_Vec3f operator*(const float& other) const {
		return _CL_Vec3f(x * other, y * other, z * other);
	}
} CL_Vec3f;
typedef struct _CL_Vec3i {
	int x;
	int y;
	int z;
	_CL_Vec3i() {
		x = 0;
		y = 0;
		z = 0;
	}
	_CL_Vec3i(int f) {
		x = f;
		y = f;
		z = f;
	}
	_CL_Vec3i(int x, int y, int z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	bool operator==(const _CL_Vec3i& other) const {
		return x == other.x && y == other.y && z == other.z;
	}
	bool operator!=(const _CL_Vec3i& other) const {
		return x != other.x || y != other.y || z != other.z;
	}
	_CL_Vec3i operator+(const _CL_Vec3i& other) const {
		return _CL_Vec3i(x + other.x, y + other.y, z + other.z);
	}
	_CL_Vec3i operator-(const _CL_Vec3i& other) const {
		return _CL_Vec3i(x - other.x, y - other.y, z - other.z);
	}
	_CL_Vec3i operator*(const int& other) const {
		return _CL_Vec3i(x * other, y * other, z * other);
	}
} CL_Vec3i;
typedef struct _CL_Rectf {
	float x;
	float y;
	float width;
	float height;
	_CL_Rectf() {
		x = 0.0f;
		y = 0.0f;
		width = 0.0f;
		height = 0.0f;
	}
	_CL_Rectf(float f) {
		x = f;
		y = f;
		width = f;
		height = f;
	}
	_CL_Rectf(float x, float y, float width, float height) {
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}
	bool operator==(const _CL_Rectf& other) const {
		return x == other.x && y == other.y && width == other.width && height == other.height;
	}
	bool operator!=(const _CL_Rectf& other) const {
		return x != other.x || y != other.y || width != other.width || height != other.height;
	}
	_CL_Rectf operator+(const _CL_Rectf& other) const {
		return _CL_Rectf(x + other.x, y + other.y, width + other.width, height + other.height);
	}
	_CL_Rectf operator-(const _CL_Rectf& other) const {
		return _CL_Rectf(x - other.x, y - other.y, width - other.width, height - other.height);
	}
	_CL_Rectf operator*(const float& other) const {
		return _CL_Rectf(x * other, y * other, width * other, height * other);
	}
} CL_Rectf;
typedef struct _CL_Recti {
	int x;
	int y;
	int width;
	int height;
	_CL_Recti() {
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}
	_CL_Recti(int f) {
		x = f;
		y = f;
		width = f;
		height = f;
	}
	_CL_Recti(int x, int y, int width, int height) {
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}
	bool operator==(const _CL_Recti& other) const {
		return x == other.x && y == other.y && width == other.width && height == other.height;
	}
	bool operator!=(const _CL_Recti& other) const {
		return x != other.x || y != other.y || width != other.width || height != other.height;
	}
	_CL_Recti operator+(const _CL_Recti& other) const {
		return _CL_Recti(x + other.x, y + other.y, width + other.width, height + other.height);
	}
	_CL_Recti operator-(const _CL_Recti& other) const {
		return _CL_Recti(x - other.x, y - other.y, width - other.width, height - other.height);
	}
	_CL_Recti operator*(const int& other) const {
		return _CL_Recti(x * other, y * other, width * other, height * other);
	}
} CL_Recti;
class TextScanner {
public:
	TextScanner() : m_data() {}
	explicit TextScanner(const std::string& string) {
		this->parse(string);
	}
	explicit TextScanner(const std::vector<std::pair<std::string, std::string>>& data) {
		for (const auto& it : data)
			this->add(it.first, it.second);
	}
	~TextScanner() = default;
	void parse(const std::string& string) {
		m_data = this->string_tokenize(string, "\n");
		for (auto& data : m_data) {
			// FIX: erase \r instead of replacing with \0 (null byte).
			// Replacing with \0 corrupts values like "dialog_return\0" which
			// causes actionMap.find("dialog_return") to fail on \r\n packets.
			data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
		}
	}
	static std::vector<std::string> string_tokenize(const std::string& string, const std::string& delimiter = "|") {
		std::vector<std::string> tokens{};
		for (const auto& word : std::views::split(string, delimiter)) tokens.emplace_back(word.begin(), word.end());
		return tokens;
	}
	std::string get(const std::string& key, int index = 1, const std::string& token = "|", int key_index = 0) {
		if (m_data.empty()) return "";
		for (auto& data : m_data) {
			if (data.empty()) continue;
			std::vector<std::string> tokenize = this->string_tokenize(data, token);
			if (tokenize[key_index] == key) {
				if (index < 0 || index >= tokenize.size()) return "";
				return tokenize[key_index + index];
			}
		}
		return "";
	}
	template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
	T get(const std::string& key, int index = 1, const std::string& token = "|") {
		return std::stoi(this->get(key, index, token));
	}
	template<typename T, typename std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
	T get(const std::string& key, int index = 1, const std::string& token = "|") {
		if (std::is_same_v<T, double>) return std::stod(this->get(key, index, token));
		else if (std::is_same_v<T, long double>) return std::stold(this->get(key, index, token));
		return std::stof(this->get(key, index, token));
	}
	bool try_get(const std::string& key, std::string& value) noexcept {
		if (!this->contain(key)) return false;
		value = this->get(key);
		return true;
	}
	template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
	bool try_get(const std::string& key, T& value) noexcept {
		if (!this->contain(key)) return false;
		value = std::stoi(this->get(key));
		return true;
	}
	TextScanner* add(const std::string& key, const std::string& value, const std::string& token = "|") {
		m_data.push_back(key + token + value);
		return this;
	}
	template<typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
	TextScanner * add(const std::string & key, const T & value, const std::string & token = "|") {
		this->add(key, std::to_string(value), token);
		return this;
	}
	TextScanner* add(const std::string& key, const CL_Vec2i& value, const std::string& token = "|") {
		std::string data{
			std::to_string(value.x) + '|' +
			std::to_string(value.y)
		};
		this->add(key, data, token);
		return this;
	}
	TextScanner* add(const std::string& key, const CL_Recti& value, const std::string& token = "|") {
		std::string data{
			std::to_string(value.x) + '|' +
			std::to_string(value.y) + '|' +
			std::to_string(value.width) + '|' +
			std::to_string(value.height)
		};
		this->add(key, data, token);
		return this;
	}
	void set(const std::string& key, const std::string& value, const std::string& token = "|") {
		if (m_data.empty()) return;
		for (auto& data : m_data) {
			std::vector<std::string> tokenize = this->string_tokenize(data, token);
			if (tokenize[0] == key) {
				data = std::string{ tokenize[0] };
				data += token;
				data += value;
				break;
			}
		}
	}
	template<typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
	void set(const std::string & key, const T & value, const std::string & token = "|") {
		this->set(key, std::to_string(value), token);
	}
	bool contain(const std::string& key) {
		return this->get(key) != "" ? true : false;
	}
	std::vector<std::string> get_all_array() {
		std::vector<std::string> ret{};
		for (int i = 0; i < m_data.size(); i++) ret.push_back(std::format("[{}]: {}", i, m_data[i]));
		return ret;
	}
	std::string get_all_raw() const {
		std::string string{};
		for (int i = 0; i < m_data.size(); i++) {
			string += m_data.at(i);
			if (i + 1 >= m_data.size()) continue;
			if (!m_data.at(i + 1).empty()) string += '\n';
		}
		return string;
	}
	std::vector<std::string> get_data() const { return m_data; }
	bool empty() { return m_data.empty(); }
	std::size_t size() { return m_data.size(); }
private:
	std::vector<std::string> m_data;
};
inline constexpr uint32_t quick_hash(const std::string_view& data) {
	uint32_t hash = 5381;
	for (const auto& c : data) hash = ((hash << 5) + hash) + c;
	return hash;
}
constexpr uint32_t operator "" _qh(const char* str, std::size_t len) {
	return quick_hash(std::string_view{ str, len });
}
bool item_playmod(int item, string name_, bool not_hand = false) {
	name_ = replace_str2(cleanup_(to_lower(name_)), " ", "_");
	vector<string> player_playmods{};
	player_playmods.push_back(items[item].playmod);
	for (int i_ = 0; i_ < player_playmods.size(); i_++) {
		if (player_playmods[i_].empty()) continue;
		string playmod = replace_str2(cleanup_(to_lower(player_playmods[i_])), " ", "_");
		if (playmod.find(name_) != string::npos) return true;
	}
	return false;
}
vector<string> get_properties(unsigned char btype, unsigned char bcateg) {
	vector<string> list;
	if ((btype & 0x40) > 0) {
		list.push_back("NoShadow");
	} if ((btype & 0x20) > 0) {
		list.push_back("NoSelf");
	} if ((btype & 2) > 0) {
		list.push_back("Wrenchable");
	} if ((btype & 8) > 0) {
		list.push_back("Dropless");
	} if ((btype & 0x10) > 0) {
		list.push_back("NoSeed");
	} if ((btype & 4) > 0) {
		list.push_back("Permanent");
	} if ((btype & 1) > 0) {
		list.push_back("MultiFacing");
	} if ((btype & 0x80) > 0) {
		list.push_back("WorldLock");
	} if ((bcateg & 0x80) > 0) {
		list.push_back("Untradable");
	} if ((bcateg & 0x20) > 0) {
		list.push_back("Foreground");
	} if ((bcateg & 0x10) > 0) {
		list.push_back("Public");
	} if ((bcateg & 1) > 0) {
		list.push_back("Beta");
	} if ((bcateg & 4) > 0) {
		list.push_back("Mod");
	} if ((bcateg & 8) > 0) {
		list.push_back("RandomGrow");
	} if ((bcateg & 0x40) > 0) {
		list.push_back("Holiday");
	} if ((bcateg & 2) > 0) {
		list.push_back("AutoPickup");
	}
	return list;
}
void replaceAll(string& str, const string& from, const string& to) {
	if (from.empty())return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}
int item_average2(vector<int> const& v) {
	return (1.0 * accumulate(v.begin(), v.end(), 0LL) / v.size());
}
string getItemCategory(const char actionType, string name_) {
	switch (actionType) {
	case 0:
		return "Fist";
	case 1:
		return "Wrench";
	case 2:
		return "Door";
	case 3:
		return "Lock";
	case 4:
		return "Gems";
	case 5:
		return "Treasure";
	case 6:
		return "Deadly_Block";
	case 7:
		return "Trampoline_Block";
	case 8:
		return "Consummable";
	case 9:
		return "Gateway";
	case 10:
		return "Sign";
	case 11:
		return "SFX_Foreground";
	case 12:
		return "Toggleable_Foreground";
	case 13:
		return "Main_Door";
	case 14:
		return "Platform";
	case 15:
		return "Bedrock";
	case 16:
		return "Pain_Block";
	case 17:
		return "Foreground_Block";
	case 18:
		return "Background_Block";
	case 19:
		return "Seed";
	case 20:
		return "Clothing";
	case 21:
		return "Animated_Foreground_Block";
	case 22:
		return "SFX_Background";
	case 23:
		return "Toggleable_Background";
	case 24:
		return "Bouncy";
	case 25:
		return "Spike";
	case 26:
		return "Portal";
	case 27:
		return "Checkpoint";
	case 28:
		return "Sheet_Music";
	case 29:
		return "Slippery_Block";
	case 31:
		return "Switch_Block";
	case 32:
		return "Chest";
	case 33:
		return "Mailbox";
	case 34:
		return "Bulletin_Board";
	case 35:
		return "Event_Mystery_Block";
	case 36:
		return "Random_Block";
	case 37:
		return "Component";
	case 38:
		return "Provider";
	case 39:
		return "Chemical_Combiner";
	case 40:
		return "Achievement_Block";
	case 41:
		return "Weather_Machine";
	case 42:
		return "Scoreboard";
	case 43:
		return "Sungate";
	case 44:
		return "Profile";
	case 45:
		return "Toggleable_Deadly_Block";
	case 46:
		return "Heart_Monitor";
	case 47:
		return "Donation_Box";
	case 48:
		return "unknown";
	case 49:
		return "Mannequin";
	case 50:
		return "Security_Camera";
	case 51:
		return "Magic_Egg";
	case 52:
		return "Game_Block";
	case 53:
		return "Game_Generator";
	case 54:
		return "Xenonite";
	case 55:
		return "Phone_Booth";
	case 56:
		return "Crystal";
	case 57:
		return "Crime_Villain";
	case 58:
		return "Clothing_Compactor";
	case 59:
		return "Spotlight";
	case 60:
		return "Pushing_Block";
	case 61:
		return "Display";
	case 62:
		return "Vending";
	case 63:
		return "Fish";
	case 64:
		return "Fish_Tank_Port";
	case 65:
		return "Solar_Collector";
	case 66:
		return "Forge";
	case 67:
		return "Giving_Tree";
	case 68:
		return "Giving_Tree_Stump";
	case 69:
		return "Steam_Block";
	case 70:
		return "Steam_Pain_Block";
	case 71:
		return "Steam_Music_Block";
	case 72:
		return "Silkworm";
	case 73:
		return "Sewing_Machine";
	case 74:
		return "Country_Flag";
	case 75:
		return "Lobster_Trap";
	case 76:
		return "Painting_Easel";
	case 77:
		return "Battle_Pet_Cage";
	case 78:
		return "Pet_Trainer";
	case 79:
		return "Steam_Engine";
	case 80:
		return "Lock_Bot";
	case 81:
		return "Weather_Machine_S1";
	case 82:
		return "Spirit_Storage";
	case 83:
		return "Display_Shelf";
	case 84:
		return "VIP";
	case 85:
		return "Chal_Timer";
	case 86:
		return "Chal_Flag";
	case 87:
		return "Fish_Mount";
	case 88:
		return "Portrait";
	case 89:
		return "Weather_Machine_S2";
	case 90:
		return "Fossil";
	case 91:
		return "Fossil_Prep_Station";
	case 92:
		return "DNA_Processor";
	case 93:
		return "Blaster";
	case 94:
		return "Valhowla_Treasure";
	case 95:
		return "Chemsynth";
	case 96:
		return "Chemsynth_Tank";
	case 97: case -126:
		return "Untrade_A_Box";
	case 98:
		return "Oven";
	case 99:
		return "Audio";
	case 100:
		return "Geiger_Charger";
	case 101:
		return "Adventure_Reset";
	case 102:
		return "Tomb_Robber";
	case 103:
		return "Faction";
	case 104:
		return "Red_Faction";
	case 105:
		return "Green_Faction";
	case 106:
		return "Blue_Faction";
	case 107:
		return "Artifact";
	case 108:
		return "LemonJelly";
	case 109:
		return "FishTrainingTank";
	case 110:
		return "FishingBlock";
	case 111:
		return "ItemSucker";
	case 112:
		return "Planter";
	case 113:
		return "Robot";
	case 114:
		return "Command";
	case 115:
		return "Ticket";
	case 116:
		return "Stats_Block";
	case 117:
		return "Field_Node";
	case 118:
		return "Ouija_Board";
	case 119:
		return "Architect_Machine";
	case 120:
		return "Starship";
	case 121:
		return "Spike";
	case 122:
		return "Gravity_Well";
	case 123:
		return "Autobreak_Blocks";
	case 124:
		return "Autobreak_Trees";
	case 125:
		return "Autobreak";
	case 126:
		return "TimedBlock";
	case 127:
		return "TimedPlatform";
	case 128:
		return "Mud_Puddle";
	case 129:
		return "Root_Cutting";
	case 130:
		return "Safe_Vault";
	case -115:
		return "Kranken";
	default:
		return "";
	}
}
double stoi_double(const string& val) {
	double value;
	stringstream ss(val);
	ss >> value;
	return value;
}
string join(const vector<string> v, const char delimeter) {
	string str;
	for (auto p = v.begin(); p != v.end(); ++p) {
		str += *p;
		if (p != (v.end() - 1))
			str += delimeter;
	}
	return str;
}
bool contains_special_characters(const std::string& str) {
	const std::string special_characters = "`!@#$%^&*()_+=-\|/?.,><;:[]{}";
	return str.find_first_of(special_characters) != std::string::npos;
}
std::string toLower(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
	return result;
}
bool isCorrupt(const std::pair<int, int>& p) {
	return p.first < 0 || p.second < 0;
}
inline vector<string> explode(const string& delimiter, const string& str) {
	vector<string> arr;
	if (delimiter.empty()) return arr;

	if (str.find(delimiter) == string::npos && (delimiter != "\n" && delimiter != ",")) {
		arr.resize(30, "");
		return arr;
	}

	arr.reserve(8);
	size_t start = 0;
	size_t end = str.find(delimiter);
	while (end != string::npos) {
		arr.push_back(str.substr(start, end - start));
		start = end + delimiter.length();
		end = str.find(delimiter, start);
	}
	arr.push_back(str.substr(start));
	return arr;
}
BlockTypes get_blocktype(const string& c_, const string& name_) {
	if (c_ == "Foreground_Block") {
		return BlockTypes::FOREGROUND;
	}
	else if (c_ == "Background_Block" || c_ == "Back" || c_ == "Sheet_Music") {
		return BlockTypes::BACKGROUND;
	}
	else if (c_ == "Mannequin") {
		return BlockTypes::MANNEQUIN;
	}
	else if (c_ == "Spirit_Storage") {
		return BlockTypes::Spirit_Storage;
	}
	else if (c_ == "Geiger_Charger") {
		return BlockTypes::GEIGER_CHARGER;
	}
	else if (c_ == "Autobreak_Blocks" || c_ == "Autobreak_Trees" || c_ == "Autobreak") {
		return BlockTypes::AUTO_BLOCK;
	}
	else if (c_ == "Seed") {
		return BlockTypes::SEED;
	}
	else if (c_ == "Display_Shelf") {
		return BlockTypes::Display_Shelf;
	}
	else if (c_ == "Country_Flag") {
		return BlockTypes::COUNTRY_FLAG;
	}
	else if (c_ == "Painting_Easel") {
		return BlockTypes::Painting_Easel;
	}
	else if (c_ == "Fish_Mount") {
		return BlockTypes::Fish_Mount;
	}
	else if (c_ == "Portrait") {
		return BlockTypes::PORTRAIT;
	}
	else if (c_ == "Heart_Monitor") {
		return BlockTypes::Heart_Monitor;
	}
	else if (c_ == "Consummable") {
		return BlockTypes::CONSUMABLE;
	}
	else if (c_ == "Kranken") {
		return BlockTypes::KRANKEN;
	}
	else if (c_ == "Pain_Block") {
		return BlockTypes::PAIN_BLOCK;
	}
	else if (c_ == "Main_Door") {
		return BlockTypes::MAIN_DOOR;
	}
	else if (c_ == "Chal_Timer") {
		return BlockTypes::TIMER;
	}
	else if (c_ == "Bedrock") {
		return BlockTypes::BEDROCK;
	}
	else if (c_ == "ItemSucker") {
		return BlockTypes::SUCKER;
	}
	else if (c_ == "Door") {
		return BlockTypes::DOOR;
	}
	else if (c_ == "Fist") {
		return BlockTypes::FIST;
	}
	else if (c_ == "Sign") {
		return BlockTypes::SIGN;
	}
	else if (c_ == "Wrench") {
		return BlockTypes::WRENCH;
	}
	else if (c_ == "Checkpoint") {
		return BlockTypes::CHECKPOINT;
	}
	else if (c_ == "Lock") {
		return BlockTypes::LOCK;
	}
	else if (c_ == "Lock_Bot") {
		return BlockTypes::LOCK_BOT;
	}
	else if (c_ == "Pet_Trainer") {
		return BlockTypes::PET_TRAINER;
	}
	else if (c_ == "Achievement_Block") {
		return BlockTypes::ACHIEVEMENT_BLOCK;
	}
	else if (c_ == "Gateway") {
		return BlockTypes::GATEWAY;
	}
	else if (c_ == "Clothing") {
		return BlockTypes::CLOTHING;
	}
	else if (c_ == "Security_Camera") {
		return BlockTypes::CCTV;
	}
	else if (c_ == "Platform") {
		return BlockTypes::PLATFORM;
	}
	else if (c_ == "SFX_Foreground") {
		return BlockTypes::SFX_FOREGROUND;
	}
	else if (c_ == "Gems") {
		return BlockTypes::GEMS;
	}
	else if (c_ == "Toggleable_Foreground") {
		return BlockTypes::TOGGLE_FOREGROUND;
	}
	else if (c_ == "Chemical_Combiner") {
		return BlockTypes::CHEMICAL_COMBINER;
	}
	else if (c_ == "Treasure") {
		return BlockTypes::TREASURE;
	}
	else if (c_ == "Deadly_Block") {
		return BlockTypes::DEADLY;
	}
	else if (c_ == "Trampoline_Block") {
		return BlockTypes::TRAMPOLINE;
	}
	else if (c_ == "Animated_Foreground_Block") {
		return BlockTypes::ANIM_FOREGROUND;
	}
	else if (c_ == "Portal") {
		return BlockTypes::PORTAL;
	}
	else if (c_ == "Random_Block") {
		return BlockTypes::RANDOM_BLOCK;
	}
	else if (c_ == "Bouncy") {
		return BlockTypes::BOUNCY;
	}
	else if (c_ == "Chest") {
		return BlockTypes::CHEST;
	}
	else if (c_ == "Switch_Block") {
		return BlockTypes::SWITCH_BLOCK;
	}
	else if (c_ == "Magic_Egg") {
		return BlockTypes::MAGIC_EGG;
	}
	else if (c_ == "Crystal") {
		return BlockTypes::CRYSTAL;
	}
	else if (c_ == "Mailbox") {
		return BlockTypes::MAILBOX;
	}
	else if (c_ == "Bulletin_Board") {
		return BlockTypes::BULLETIN_BOARD;
	}
	else if (c_ == "Faction") {
		return BlockTypes::FACTION;
	}
	else if (c_ == "Component") {
		return BlockTypes::COMPONENT;
	}
	else if (c_ == "Weather_Machine") {
		return BlockTypes::WEATHER;
	}
	else if (c_ == "ItemSucker") {
		return BlockTypes::SUCKER;
	}
	else if (c_ == "Fish_Tank_Port") {
		return BlockTypes::FISH;
	}
	else if (c_ == "Fish") {
		return BlockTypes::FISH_TANK;
	}
	else if (c_ == "Battle_Pet_Cage") {
		return BlockTypes::BATTLE_CAGE;
	}
	else if (c_ == "Steam_Block") {
		return BlockTypes::STEAM;
	}
	else if (c_ == "ground_Block") {
		return BlockTypes::GROUND_BLOCK;
	}
	else if (c_ == "Display") {
		return BlockTypes::DISPLAY;
	}
	else if (c_ == "Untrade_A_Box" || c_ == "Safe_Vault") {
		return BlockTypes::STORAGE;
	}
	else if (c_ == "Vending") {
		return BlockTypes::VENDING;
	}
	else if (c_ == "Blaster") {
		return BlockTypes::TRICKSTER;
	}
	else if (c_ == "Giving_Tree") {
		return BlockTypes::GIVING_TREE;
	}
	else if (c_ == "VIP") {
		return BlockTypes::VIP_ENTRANCE;
	}
	else if (c_ == "Donation_Box") {
		return BlockTypes::DONATION;
	}
	else if (c_ == "Phone_Booth") {
		return BlockTypes::PHONE;
	}
	else if (c_ == "Sewing_Machine") {
		return BlockTypes::SEWINGMACHINE;
	}
	else if (c_ == "Crime_Villain") {
		return BlockTypes::CRIME_VILLAIN;
	}
	else if (c_ == "Provider") {
		return BlockTypes::PROVIDER;
	}
	else if (c_ == "Adventure_Reset") {
		return BlockTypes::ADVENTURE;
	}
	else if (c_ == "Game_Block") {
		return BlockTypes::GAME_BLOCK;
	}
	else if (c_ == "Game_Generator") {
		return BlockTypes::GAME_GENERATOR;
	}
	else if (c_ == "Oven") {
		return BlockTypes::OVEN;
	}
	else {
		return BlockTypes::UNKNOWN;
	}
}
inline int GetMessageTypeFromPacket(ENetPacket* packet) {
	int result;
	if (packet->dataLength > 3u) {
		result = *(packet->data);
	}
	else {
		cout << "[!] Bad packet length, ignoring message" << endl;
		result = 0;
	}
	return result;
}
char* GetTextPointerFromPacket(ENetPacket* packet) {
	char zero = 0;
	memcpy(packet->data + packet->dataLength - 1, &zero, 1);
	return (char*)(packet->data + 4);
}
inline void send_(ENetPeer* peer, int num, char* data, const int len) {
	const auto packet = enet_packet_create(nullptr, len + 5, ENET_PACKET_FLAG_RELIABLE);
	memcpy(packet->data, &num, 4);
	if (data != nullptr) {
		memcpy(packet->data + 2, data, len);
	}
	char zero = 0;
	if (enet_peer_send(peer, 0, packet) != 0) {
		enet_packet_destroy(packet);
	}
}
class Enet_Initialize {
public:
	int Initialize(int port) {
		enet_initialize();
		ENetAddress address;
		enet_address_set_host(&address, "0.0.0.0");
		address.port = port;
		server = enet_host_create(&address, 1024, 2, 0, 0);
		server->usingNewPacketForServer = true; // Enable new packet handling
		server->checksum = enet_crc32;
		enet_host_compress_with_range_coder(server);
		return 0;
	}
};
Enet_Initialize* Enet_Server();
Enet_Initialize Enet_In;
Enet_Initialize* Enet_Server() {
	return &Enet_In;
}
ClothTypes get_clothtype(string t_, BlockTypes a_) {
	if (a_ == BlockTypes::CLOTHING) {
		if (t_ == "Hat") {
			return ClothTypes::HAIR;
		}
		else if (t_ == "Shirt") {
			return ClothTypes::SHIRT;
		}
		else if (t_ == "Pants") {
			return ClothTypes::PANTS;
		}
		else if (t_ == "Feet") {
			return ClothTypes::FEET;
		}
		else if (t_ == "Face") {
			return ClothTypes::FACE;
		}
		else if (t_ == "Hand") {
			return ClothTypes::HAND;
		}
		else if (t_ == "Back") {
			return ClothTypes::BACK;
		}
		else if (t_ == "Hair") {
			return ClothTypes::HAIR;
		}
		else if (t_ == "Chest") {
			return ClothTypes::NECKLACE;
		}
		else if (t_ == "Ances") {
			return ClothTypes::ANCES;
		}
		else if (t_ == "Mask") {
			return ClothTypes::MASK;
		}
		else if (t_ == "Necklace") {
			return ClothTypes::NECKLACE;
		}
		else {
			return ClothTypes::NONE;
		}
	}
	else {
		return ClothTypes::NONE;
	}
}
inline uint32_t get_hash(unsigned char* str, const int len) {
	unsigned char* n = static_cast<unsigned char*>(str);
	uint32_t acc = 0x55555555;
	if (len == 0) {
		while (*n)
			acc = (acc >> 27) + (acc << 5) + *n++;
	}
	else {
		for (int i = 0; i < len; i++)
			acc = (acc >> 27) + (acc << 5) + *n++;
	}
	return acc;
}
int get_punch_id(const int id_) {
	switch (id_) {
	case 138:
	case 2976:
	case 8354:
		return 1;
	case 14564:
	case 14566:
	case 14568:
	case 14570:
	case 14572:
	case 14574:
	case 14576:
	case 14220:
	case 14222:
	case 14598:
	case 19988:
	case 19990:
	case 19958:
	case 19960:
	case 19962:
		return 237;
	case 5924:
		return 30;
	case 366:
	case 1464:
		return 2;
	case 472: return 3;
	case 594:
	case 10130:
	case 5424:
	case 5456:
	case 4136:
	case 10052:
		return 4;
	case 768: return 5;
	case 900:
	case 7760:
	case 9272:
	case 5002:
	case 7758:
	case 4664:
	case 8046:
	case 9062:
	case 1252:
	case 1254:
	case 9086:
	case 3680:
	case 5176:
	case 7750:
	case 1228:
	case 3430:
		return 6;
	case 910:
	case 4332:
	case 1250:
	case 12656:
		return 7;
	case 930:
	case 1010:
	case 6382:
	case 9698:
	case 6368:
		return 8;
	case 1016:
	case 6058:
		return 9;
	case 1204:
	case 9534:
	case 10928:
		return 10;
	case 1378: return 11;
	case 1440:
	case 4508:
	case 2224:
	case 4512:
	case 4510:
	case 4515:
	case 10996:
	case 9654:
	case 4514:
	case 11764:
	case 7748:
		return 12;
	case 1484:
	case 5160:
	case 9802:
	case 9508:
		return 13;
	case 1512:
	case 1648:
		return 14;
	case 1542: return 15;
	case 1576: return 16;
	case 1676:
	case 7504:
		return 17;
	case 1748:
	case 8006:
	case 8008:
	case 8010:
	case 8012:
		return 19;
	case 1710:
	case 4644:
	case 1714:
	case 1712:
	case 6044:
	case 1570:
		return 18;
	case 3578: return 19;
	case 1780: return 20;
	case 1782:
	case 5156:
	case 9776:
	case 9810:
	case 10120:
		return 21;
	case 1804:
	case 5194:
	case 9784:
		return 22;
	case 1868:
	case 7754:
	case 1998:
		return 23;
	case 1874: return 24;
	case 1946:
	case 2800:
		return 25;
	case 1952:
	case 2854:
	case 9280:
	case 1974:
		return 26;
	case 1956: return 27;
	case 1960: return 28;
	case 2908:
	case 6312:
	case 9496:
	case 8554:
	case 3162:
	case 9536:
	case 8584:
	case 4956:
	case 3466:
	case 4166:
	case 2952:
	case 9520:
	case 9522:
	case 8440:
	case 3932:
	case 3934:
	case 7434:
	case 8732:
	case 3108:
	case 9766:
	case 12368:
	case 10780:
	case 3160:
	case 12102:
	case 4688:
	case 8604:
	case 3070:
	case 10402:
	case 7500:
	case 3206:
	case 3588:
	case 2636:
	case 8452:
	case 11066:
	case 9076:
	case 7890:
	case 8736:
	case 10936:
	case 10938:
		return 29;
	case 1980:
	case 7106:
	case 8042:
		return 30;
	case 2066:
	case 4150:
	case 11082:
	case 11080:
	case 9714:
	case 11078:
	case 3678:
	case 10686:
		return 31;
	case 2212:
	case 5174:
	case 5004:
	case 5006:
	case 5008:
	case 8912:
		return 32;
	case 2218: return 33;
	case 2220: return 34;
	case 2266: return 35;
	case 2386: return 36;
	case 2388: return 37;
	case 2450:
		return 38;
	case 2476:
	case 4208:
	case 12308:
	case 10336:
	case 9804:
		return 39;
	case 4294:
	case 1948:
		return 40;
	case 2512:
	case 9732:
	case 6338:
	case 6670:
	case 3736:
	case 10406:
	case 10232:
	case 10994:
	case 7146:
		return 41;
	case 2572:
	case 11072:
		return 42;
	case 2592:
	case 9396:
	case 2596:
	case 10930:
	case 9548:
	case 9812:
	case 9800:
	case 5158:
		return 43;
	case 2720: return 44;
	case 2752: return 45;
	case 2754:
	case 9830:
	case 9898:
		return 46;
	case 2756:
		return 47;
	case 2802: return 49;
	case 2866: return 50;
	case 2876: return 51;
	case 2878:
	case 2880:
		return 52;
	case 2906:
	case 4170:
	case 2888:
	case 4278:
	case 4126:
		return 53;
	case 2886: return 54;
	case 2890: return 55;
	case 2910: return 56;
	case 3066:
	case 10288:
		return 57;
	case 3124:
	case 5088:
		return 58;
	case 3168:
	case 3166:
		return 59;
	case 3214:
	case 9194:
	case 4506:
		return 60;
	case 7408:
	case 3238:
		return 61;
	case 3274: return 62;
	case 3300: return 64;
	case 3418: return 65;
	case 3476: return 66;
	case 3596: return 67;
	case 3686: return 68;
	case 3716:
	case 6086:
		return 69;
	case 4110:
	case 2986:
	case 4252:
		return 70;
	case 4290: return 71;
	case 4474: return 72;
	case 4464:
	case 9500:
		return 73;
	case 4660: return 74;
	case 4746:
	case 4750:
	case 4748:
		return 75;
	case 4778:
	case 6026: case 7784:
		return 76;
	case 4996:
		return 77;
	case 4840: return 78;
	case 5206: return 79;
	case 5480:
	case 9770:
	case 9778:
	case 9772:
	case 9906:
	case 9908:
	case 9918:
	case 10290:
	case 10362:
		return 80;
	case 6110: return 81;
	case 6308: return 82;
	case 6310: return 83;
	case 6298: return 84;
	case 6756: return 85;
	case 7044: return 86;
	case 6892: return 87;
	case 6966: return 88;
	case 7088:
	case 11020:
		return 89;
	case 7098:
	case 9032:
		return 90;
	case 10384:
		return 153;
	case 14562:
		return 46;
	case 7192: return 91;
	case 7136:
	case 11788:
		return 92;
	case 7142: return 93;
	case 7216: return 94;
	case 7196:
		return 95;
	case 7392:
	case 9604:
		return 96;
	case 6754: return 97;
	case 7384: return 98;
	case 7414: return 99;
	case 7402:
	case 7396:
		return 100;
	case 7424: return 101;
	case 7470:
	case 9738:
		return 102;
	case 7488: return 103;
	case 7586:
	case 7650: return 105;
	case 6804:
	case 6358:
	case 7646:
		return 106;
	case 7568:
	case 7570:
	case 7572:
	case 7574:
		return 107;
	case 7668: return 108;
	case 7660:
	case 9060:
		return 109;
	case 7584:
		return 110;
	case 7736:
	case 9116:
	case 9118:
	case 7826:
	case 7828:
	case 11440:
	case 11442:
	case 11312:
	case 7830:
	case 7832:
	case 10670:
	case 9120:
	case 9122:
	case 10680:
	case 10626:
	case 10578:
	case 10334:
	case 11380:
	case 11326:
	case 7912:
	case 11298:
	case 10498:
	case 7940:
	case 12342:
	case 8492:
	case 9340:
	case 11358:
		return 111;
	case 7836:
	case 7838:
	case 7840:
	case 7842:
		return 112;
	case 7950: return 113;
	case 8002: return 114;
	case 8022: return 116;
	case 8036: return 118;
	case 9348:
	case 8372:
	case 8810:
		return 119;
	case 8038:
	case 11990:
	case 8360:
	case 8510:
	case 8374:
		return 120;
	case 8358: return 121;
	case 8364: return 122;
	case 8438: return 123;
	case 10066:
	case 8494:
	case 11310:
	case 9360:
		return 126;
	case 8814: return 127;
	case 8816:
	case 8818:
	case 8820:
	case 8822:
		return 128;
	case 8910: return 129;
	case 8942: return 130;
	case 8944:
	case 5276:
	case 10940:
		return 131;
	case 8432:
	case 8434:
	case 8436:
	case 8950:
		return 132;
	case 8946:
	case 9576:
	case 9636:
		return 133;
	case 8960: return 134;
	case 9006: return 135;
	case 9058:
	case 13710:
		return 136;
	case 9082:
	case 9304:
	case 9506:
		return 137;
	case 9066:
		return 138;
	case 9136: return 139;
	case 9138:
		return 140;
	case 9172:
	case 9176:
		return 141;
	case 9190: return 142;
	case 9254: return 143;
	case 9256: return 144;
	case 9236: return 145;
	case 9342: return 146;
	case 9542: return 147;
	case 9378: return 148;
	case 9376: return 149;
	case 9410: return 150;
	case 9462: return 151;
	case 9606:
	case 9758:
		return 152;
	case 9716:
	case 5192:
	case 9764:
	case 9916:
		return 153;
	case 10048:
	case 9912:
		return 167;
	case 10064:
	case 10604:
		return 168;
	case 10046: return 169;
	case 10050: return 170;
	case 10128: return 171;
	case 10210:
	case 9544:
		return 172;
	case 10250: return 173;
	case 10246: return 175;
	case 10278: return 176;
	case 10292:
	case 7406:
	case 9450:
		return 177;
	case 10330: return 178;
	case 10392:
		return 179;
	case 10388:
	case 9524:
	case 9598:
		return 180;
	case 11620: return 181;
	case 10426: return 183;
	case 10442: return 184;
	case 10506: return 185;
	case 10494: return 186;
	case 10618: return 187;
	case 10652: return 188;
	case 10676: return 191;
	case 10674: return 192;
	case 10694: return 193;
	case 10714: return 194;
	case 10724: return 195;
	case 10722: return 196;
	case 10754: return 197;
	case 10800: return 198;
	case 10888: return 199;
	case 10886:
		return 200;
	case 10894: return 201;
	case 10890: return 202;
	case 9880:
	case 9782:
	case 9947:
	case 10922:
	case 9550:
	case 9974:
		return 203;
	case 10914: return 204;
	case 10990: return 205;
	case 10998: return 206;
	case 10952: return 207;
	case 11000: return 208;
	case 11006: return 209;
	case 11046: return 210;
	case 11052: return 211;
	case 10960: return 212;
	case 10956:
	case 9774:
	case 9896:
	case 10944:
		return 213;
	case 10958: return 214;
	case 10954: return 215;
	case 11076: return 216;
	case 11084:
	case 10020:
		return 217;
	case 11118:
	case 9546:
	case 9574:
	case 9874:
	case 9914:
		return 218;
	case 11120: return 219;
	case 11116: return 220;
	case 11158: return 221;
	case 11162: return 222;
	case 11142: return 223;
	case 11232: return 224;
	case 11140: return 225;
	case 11248:
	case 9596:
		return 226;
	case 11240: return 227;
	case 11250: return 228;
	case 11284: return 229;
	case 11292: return 231;
	case 11308: return 232;
	case 11314: return 233;
	case 11316: return 234;
	case 11324: return 235;
	case 11354: return 236;
	case 11760:
	case 11464:
	case 11438:
	case 12846:
	case 12230:
	case 11716:
	case 11718:
	case 11674:
	case 11630:
	case 11786:
	case 11872:
	case 11762:
	case 11994:
	case 12172:
	case 12184:
	case 11460:
	case 12014:
	case 12016:
	case 12018:
	case 12020:
	case 12022:
	case 12024:
	case 12246:
	case 12248:
	case 12176:
	case 12242:
	case 11622:
	case 12350:
	case 12300:
	case 12374:
	case 12356:
	case 12286:
	case 12628:
	case 12420:
	case 12384:
	case 12410:
	case 12412:
	case 12404:
	case 12402:
	case 12416:
	case 12658:
	case 11542:
	case 12860:
	case 12870:
	case 12862:
	case 12850:
	case 12886:
	case 12990:
	case 12992:
	case 12880:
	case 13060:
	case 13136:
	case 13114:
	case 13118:
	case 13190:
	case 13326:
	case 13330:
	case 13332:
	case 13366:
	case 13188:
	case 13410:
	case 13486:
	case 13488:
	case 13490:
	case 13492:
	case 13494:
	case 13484:
	case 13578:
	case 13552:
	case 13554:
	case 13572:
	case 13606:
		return 237;
	case 11384: return 239;
	case 11458: return 240;
	case 11814:
	case 12232:
	case 12302:
	case 12872:
	case 12874:
	case 12958:
	case 13324:
	case 13424:
		return 241;
	case 11548:
	case 11552:
		return 242;
	case 11704:
	case 11706:
		return 243;
	case 12180:
	case 12346:
	case 12344:
	case 13058:
	case 13498:
	case 13322:
		return 244;
	case 11506:
	case 11508:
	case 11562:
	case 11768:
	case 11882:
	case 11720:
	case 11884:
	case 13116:
	case 11536:
		return 245;
	case 12432:
	case 12434:
	case 12842:
	case 12640:
	case 13268: return 246;
	case 14302: return 237;
	case 11818:
	case 11876:
	case 12000:
	case 12240:
	case 12642:
	case 12644:
	case 13022:
	case 13024:
	case 13396:
	case 13398:
	case 12564:
		return 248;
	case 14538:
	case 14540:
		return 242;
	}
	return 0;
}
inline std::string currentDateTime() {
	auto now = time(nullptr);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d.%m.%Y %R", &tstruct);
	return buf;
}
string to_upper(string s) {
	for (char& c : s)
		c = toupper(c);
	return s;
}
inline std::string replace_str(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty()) return str;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}
class GetItemManager {
public:
	static int Serialize() {
		string n_ = "database/items.dat";
		ifstream file(n_, ios::binary | ios::ate);
		__int64 size = file.tellg();
		if (size == -1) {
			std::cout << "[ERROR]: Serialize failed: database/items.dat not found or size is -1" << std::endl;
			return -1;
		}
		Environment()->ItemsDat_Size = (int)size;
		char* data = new char[size];
		file.seekg(0, ios::beg);
		file.read((char*)(data), size);
		if (Environment()->ItemsDat != nullptr) {
			delete[] Environment()->ItemsDat;
			Environment()->ItemsDat = nullptr;
		}
		Environment()->ItemsDat = new BYTE[60 + size];
		int MessageType = 0x4, PacketType = 0x10, NetID = -1, CharState = 0x8;
		memset(Environment()->ItemsDat, 0, 60);
		memcpy(Environment()->ItemsDat, &MessageType, 4);
		memcpy(Environment()->ItemsDat + 4, &PacketType, 4);
		memcpy(Environment()->ItemsDat + 8, &NetID, 4);
		memcpy(Environment()->ItemsDat + 16, &CharState, 4);
		memcpy(Environment()->ItemsDat + 56, &size, 4);
		file.seekg(0, std::ios::beg);
		file.read((char*)(Environment()->ItemsDat + 60), size);
		Environment()->ItemsDat_Hash = get_hash((unsigned char*)data, (const int)size);
		int memPos = 0, itemCount;
		int16_t itemsdatVersion = 0;
		memcpy(&itemsdatVersion, data + memPos, 2);
		Environment()->ItemsDat_Version = itemsdatVersion;
		memPos += 2;
		memcpy(&itemCount, data + memPos, 4);
		Environment()->ItemsDat_Count = itemCount;
		const string key_ = "PBG892FXX982ABC*";
		memPos += 4;

		for (int i = 0; i < itemCount; i++) {
			ItemDB def;
			memcpy(&def.id, data + memPos, 4);

			memPos += 4;
			def.editableType = data[memPos];
			if (def.editableType == 3 || def.editableType == 31 || def.editableType == 1 || def.editableType == 21 || def.editableType == 23) {
				def.multiple_sides = 1;
			}
			memPos += 1;
			def.itemCategory = data[memPos];
			memPos += 1;
			def.actionType = data[memPos];
			if (def.actionType == 12 or def.actionType == 119 or def.actionType == 45 or def.actionType == 32 or def.actionType == 31 or def.actionType == 122 or def.actionType == 36 or def.actionType == 18) {
				if (def.actionType == 31 || def.actionType == 36 || def.actionType == 12 || def.actionType == 18 || def.id == 11692) {
					def.can_be_changed_to_public = 1;
				}
				def.toggleable = 1;
			}
			memPos += 1;
			def.hitSoundType = data[memPos];
			memPos += 1;
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.name += data[memPos] ^ (key_[(j + def.id) % key_.length()]);
				memPos++;
			}
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.texture += data[memPos];
				memPos++;
			}
			memcpy(&def.textureHash, data + memPos, 4);
			memPos += 4;
			def.itemKind = memPos[data];
			memPos += 1;
			memcpy(&def.val1, data + memPos, 4);
			memPos += 4;
			def.textureX = data[memPos];
			memPos += 1;
			def.textureY = data[memPos];
			memPos += 1;
			def.spreadType = data[memPos];
			memPos += 1;
			def.isStripeyWallpaper = data[memPos];
			memPos += 1;
			def.collisionType = data[memPos];
			if (def.collisionType == 1) {
				def.solid = 1;
			}
			memPos += 1;
			def.breakHits = data[memPos];
			if (def.breakHits <= 0 or def.breakHits > 42 * 6) {
				def.breakHits = 42 * 6;
			}
			memPos += 1;
			memcpy(&def.dropChance, data + memPos, 4);
			memPos += 4;
			def.clothingType = data[memPos];
			memPos += 1;
			memcpy(&def.rarity, data + memPos, 2);
			memPos += 2;
			def.maxAmount = data[memPos];
			memPos += 1;
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.extraFile += data[memPos];
				memPos++;
			}
			if (def.extraFile.find("mp3") != string::npos) def.musical_block = true;
			memcpy(&def.extraFileHash, data + memPos, 4);
			memPos += 4;
			memcpy(&def.base_weather, data + memPos, 4);
			memPos += 4;
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.petName += data[memPos];
				memPos++;
			}
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.petPrefix += data[memPos];
				memPos++;
			}
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.petSuffix += data[memPos];
				memPos++;
			}
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.petAbility += data[memPos];
				memPos++;
			}
			def.seedBase = data[memPos];
			memPos += 1;
			def.seedOverlay = data[memPos];
			memPos += 1;
			def.treeBase = data[memPos];
			memPos += 1;
			def.treeLeaves = data[memPos];
			memPos += 1;
			memcpy(&def.seedColor, data + memPos, 4);
			memPos += 4;
			memcpy(&def.seedOverlayColor, data + memPos, 4);
			memPos += 4;
			memPos += 4;
			memcpy(&def.growTime, data + memPos, 4);
			memPos += 4;
			memcpy(&def.val2, data + memPos, 2);
			memPos += 2;
			memcpy(&def.isRayman, data + memPos, 2);
			memPos += 2;
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.extraOptions += data[memPos];
				memPos++;
			}
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.texture2 += data[memPos];
				memPos++;
			}
			strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				def.extraOptions2 += data[memPos];
				memPos++;
			}
			memPos += 80;
			if (itemsdatVersion >= 11) {
				strLen = *(int16_t*)&data[memPos];
				memPos += 2;
				for (int j = 0; j < strLen; j++) {
					def.punchOptions += data[memPos];
					memPos++;
				}
			}
			if (itemsdatVersion >= 12) memPos += 13;
			if (itemsdatVersion >= 13) memPos += 4;
			if (itemsdatVersion >= 14) memPos += 4;
			if (itemsdatVersion >= 15) {
				memPos += 25;
				int16_t strLen = *(int16_t*)&data[memPos];
				memPos += 2 + strLen;
			}
			if (itemsdatVersion >= 16) {
				int16_t jLen_ = *(int16_t*)&data[memPos];
				memPos += 2 + jLen_;
			}
			if (itemsdatVersion >= 17) memPos += 4;
			if (itemsdatVersion >= 18) memPos += 4;
			if (itemsdatVersion >= 19) memPos += 9;
			if (itemsdatVersion >= 21) memPos += 2;
			if (itemsdatVersion >= 22) {
				def.description = ""; // real per-item description follows; don't append onto the default fallback text
				int16_t strLen = *(int16_t*)&data[memPos];
				memPos += 2;
				for (int j = 0; j < strLen; j++) {
					def.description += data[memPos];
					memPos++;
				}
			}
			if (itemsdatVersion >= 23) {
				memcpy(&def.r_1, data + memPos, 2);
				memPos += 2;
				memcpy(&def.r_2, data + memPos, 2);
				memPos += 2;
			}
			if (itemsdatVersion >= 24) memPos += 1;
			if (itemsdatVersion >= 25) {
				int16_t strLen = *(int16_t*)&data[memPos];
				memPos += 2 + strLen;
			}
			if (itemsdatVersion >= 26) memPos += 5;

			if (i != def.id) {
				def.id = i;
			}
			string category = getItemCategory(def.actionType, def.name);
			if (def.actionType == 127 || def.actionType == 126 || def.actionType == 118 || def.actionType == 117 || def.actionType == 116 || def.actionType == 115 || def.actionType == 113 || def.actionType == 109 || def.actionType == 106 || def.actionType == 105 || def.actionType == 104 || def.actionType == 102 || def.actionType == 99 || def.actionType == 96 || def.actionType == 86 || def.actionType == 79 || def.actionType == 75 || def.actionType == 72 || def.actionType == 71 || def.actionType == 68 || def.actionType == 66 || def.actionType == 65 || def.actionType == 53 || def.actionType == 52 || def.actionType == 50 || def.actionType == 43 || def.actionType == 91 || def.id == 5818 || def.id == 5820) def.blocked_place = true;
			if (def.actionType == 77) def.battle_cage = 1;
			if (def.id == 3832 || def.id == 6016 || def.id == 1436 || def.id == 14548 || def.id == 8246 || def.id == 10258 || def.id == 2646 || def.id == 3202) def.blocked_place = false;
			if (def.blocked_place) def.simple_load = true;
			if (def.id == 4632 or def.id == 4630) { def.audio_rack = true; def.simple_load = false; def.blocked_place = false; }
			string clothingType_ = "";
			if (def.actionType == 107) {
				category = "Clothing";
				clothingType_ = "Ances";
			}
			if (def.actionType == 20) {
				switch (def.clothingType) {
				case 0: clothingType_ = "Hair";
					break;
				case 1: clothingType_ = "Shirt";
					break;
				case 2: clothingType_ = "Pants";
					break;
				case 3: clothingType_ = "Feet";
					break;
				case 4: clothingType_ = "Face";
					break;
				case 5: clothingType_ = "Hand";
					break;
				case 6: clothingType_ = "Back";
					break;
				case 7: clothingType_ = "Mask";
					break;
				case 8: clothingType_ = "Necklace";
					break;
				case 9: clothingType_ = "Ances";
					break;
				default: break;
				}
			}
			if ((def.id % 2) == 1) category = "Seed";
			vector<string> prop_ = get_properties(def.actionType, def.itemCategory);
			string thisss = (prop_.empty() ? "0" : join(prop_, ','));
			vector<string> properties = explode(",", thisss);
			def.properties = Property_Zero;
			for (string& prop : properties) {
				if (prop == "NoSeed") {
					def.properties += Property_NoSeed;
				} if (prop == "Dropless") {
					def.properties += Property_Dropless;
				} if (prop == "Beta") {
					def.properties += Property_Beta;
				} if (prop == "Mod") {
					def.properties += Property_Mod;
				} if (prop == "Untradable") {
					def.properties += Property_Untradable;
					def.untradeable = 1;
				} if (prop == "Wrenchable") {
					def.properties += Property_Wrenchable;
				} if (prop == "MultiFacing") {
					def.properties += Property_MultiFacing;
				} if (prop == "Permanent") {
					def.properties += Property_Permanent;
				} if (prop == "AutoPickup") {
					def.properties += Property_AutoPickup;
				} if (prop == "WorldLock") {
					def.properties += Property_WorldLock;
				} if (prop == "NoSelf") {
					def.properties += Property_NoSelf;
				} if (prop == "RandomGrow") {
					def.properties += Property_RandomGrow;
				} if (prop == "Public" && def.id != 6852) {
					def.properties += Property_Public;
				} if (prop == "Foreground") {
					def.properties += Property_Foreground;
				} if (prop == "OnlyCombiner") {
					def.properties += Property_Chemical;
				}
			}
			if (def.id == 9570) def.properties += Property_Mod;
			if (def.id == 9828) def.properties = Property_Zero;
			if (def.id == 5926 || def.id == 12958 || def.id == 9862 || def.id == 9866 || def.id == 9868 || def.id == 9920 || def.id == 7188 || def.id == 10018 || def.id == 10120 || def.id == 10122 || def.id == 10124 || def.id == 10118) {
				def.untradeable = 0;
				def.properties &= ~Property_Untradable;
			}
			if (def.id == 14596 || def.id == 2950 || def.id == 9772 || def.id == 5930 || def.id == 9770 || def.id == 9778 || def.id == 14516 || def.id == 14518 || def.id == 14520 || def.id == 14522 || def.id == 14524 || def.id == 5136 || def.id == 14084 || def.id == 7960 || def.id == 9726 || def.id == 10934 || def.id == 14562 || def.id == 10384 || def.id == 9846 || def.id == 9774 || def.id == 8430 || def.id == 9852 || def.id == 9854 || def.id == 9882 || def.id == 9896 || def.id == 10372 || def.id == 10400) {
				def.untradeable = 1;
				def.properties += Property_Untradable;
			}
			def.blockType = get_blocktype(category, def.name);
			if (def.id == 11138) {
				def.blockType = VIP_ENTRANCE;
			}
			else if (def.id == 6954 || def.id == 5638 || def.id == 5930 || def.id == 9850 || def.id == 10266 || def.id == 21220) {
				def.blockType = SUCKER;
				if (def.id == 21220 && (def.name.empty() || def.name.find("null") != std::string::npos)) {
					def.name = "Unlimited Magplant";
					def.ori_name = "Unlimited Magplant";
				}
			}
			else if (def.id == 10258 || def.id == 1436 || def.id == 8246) {
				def.blockType = CCTV;
			}
			else if (def.actionType == 81 or def.id == 5958 or def.id == 12054) {
				def.blockType = WEATHER;
			}
			if (def.blockType == GAME_BLOCK or def.blockType == GAME_GENERATOR) { def.simple_load = false; def.blocked_place = false; }
			def.clothType = get_clothtype(clothingType_, def.blockType);
			def.effect = get_punch_id(def.id);
			def.ori_name = fixchar2(def.name);
			items.push_back(def);
			if (items[def.id].properties & Property_RandomGrow) {
				if (def.name.find("Tangram Block") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Tangram Block Seed" : "Tangram Block");
				else if (def.name.find("Card Block") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Card Block Seed" : "Card Block");
				else if (def.name.find("Growsaber") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Growsaber Seed" : "Growsaber");
				else if (def.name.find("Surgical") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Surgical Tool Seed" : "Surgical Tool");
				else if (def.name.find("Super Shirt") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Super Shirt Seed" : "Super Shirt");
				else if (def.name.find("Muscle Shirt") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Muscle Shirt Seed" : "Muscle Shirt");
				else if (def.name.find("Heroic Tights") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Heroic Tights Seed" : "Heroic Tights");
				else if (def.name.find("Super Tights") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Super Tights Seed" : "Super Tights");
				else if (def.name.find("Heroic Boots") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Heroic Boots Seed" : "Heroic Boots");
				else if (def.name.find("Superhero Mask") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Superhero Mask Seed" : "Superhero Mask");
				else if (def.name.find("Crimefighter Mask") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Crimefighter Mask Seed" : "Crimefighter Mask");
				else if (def.name.find("Super Cape") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Super Cape Seed" : "Super Cape");
				else if (def.name.find("Superheroine Top") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Superheroine Top Seed" : "Superheroine Top");
				else if (def.name.find("Super Eye Mask") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Super Eye Seed" : "Super Eye");
				else if (def.name.find("Super Logo") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Super Logo Seed" : "Super Logo");
				else if (def.name.find("Superpower") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Superpower Seed" : "Superpower");
				else if (def.name.find("Villain Portrait") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Villain Portrait Seed" : "Villain Portrait");
				else if (def.name.find("Super Logo") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Super Logo Seed" : "Super Logo");
				else if (def.name.find("Fashionable Dress") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Fashionable Dress Seed" : "Fashionable Dress");
				else if (def.name.find("Stylin' Dress") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Stylin' Dress Seed" : "Stylin' Dress");
				else if (def.name.find("High Heels") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "High Heels Seed" : "High Heels");
				else if (def.name.find("Fashion Purse") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Fashion Purse Seed" : "Fashion Purse");
				else if (def.name.find("Number Block") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Number Block Seed" : "Number Block");
				else if (def.name.find("Number") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Number Seed" : "Number");
				else if (def.name.find("Paint Bucket") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Paint Bucket Seed" : "Paint Bucket");
				else if (def.name.find("Crayon") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Crayon Block Seed" : "Crayon Block");
				else if (def.name.find("Command") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Command Tool Seed" : "Command Tool");
				else if (def.name.find("Growmoji") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Growmoji Seed" : "Growmoji");
				else if (def.name.find("Guild Flag") != string::npos)
					items[def.id].name = (def.name.find("Seed") != string::npos ? "Guild Flag Seed" : "Guild Flag");
				else {
					int a_ = 0;
					int last_id = 0;
					while (items[def.id - a_].properties & Property_RandomGrow) {
						if (items[def.id - a_].blockType != BlockTypes::SEED) last_id = def.id - a_;
						a_++;
					}
					items[def.id].name = (def.name.find("Seed") != string::npos ? items[last_id + 1].name : items[last_id].name);
				}
			}
			if (i == 9266) {
				items[i].max_gems3 = 500;
				items[i].clash_token = 10;
			}
			if (i == 6856) {
				items[i].max_gems3 = 100;
				items[i].clash_token = 10;
			}
			if (i == 6858) {
				items[i].max_gems3 = 200;
				items[i].clash_token = 10;
			}
			if (i == 6860) {
				items[i].max_gems3 = 4000;
				items[i].clash_token = 70;
			}
			if (i == 8188 || i == 6862 || i == 8188) {
				items[i].max_gems3 = 9600;
				items[i].clash_token = 70;
			}
			if (i == 6142 || i == 1240 || items[i].actionType == -115 || i == 9080) items[i].multiple_sides = 1;
			if (i == 834 || i == 611 || i == 914 || i == 916 || i == 918 || i == 920 || i == 874)items[i].max_gems3 = 1;
			if (i == 202)items[i].max_gems3 = 5;
			if (i == 838 || i == 840 || i == 844 || i == 390 || i == 540 || i == 922 || i == 388 || i == 386)items[i].max_gems3 = 10;
			if (i == 204) items[i].max_gems3 = 20;
			if (i == 8534)items[i].max_gems3 = 25;
			if (i == 720 || i == 206 || i == 528 || i == 8500) items[i].max_gems3 = 50;
			if (i == 366 || i == 932 || i == 542 || i == 882)items[i].max_gems3 = 100;
			if (i == 710)items[i].max_gems3 = 200;
			if (i == 930 || i == 768 || i == 672 || i == 830 || i == 1008 || i == 8542)items[i].max_gems3 = 250;
			if (i == 242)items[i].max_gems3 = 300;
			if (i == 1240)items[i].max_gems3 = 400;
			if (i == 362 || i == 692)items[i].max_gems3 = 500;
			if (i == 898) items[i].max_gems3 = 750;
			if (i == 946 || i == 934 || i == 984 || i == 276 || i == 274 || i == 408 || i == 11432 || i == 4490)items[i].max_gems3 = 1000;
			if (i == 10394) items[i].max_gems3 = 1000;
			if (i == 574 || i == 592 || i == 766 || i == 760 || i == 1012) items[i].max_gems3 = 2500;
			if (i == 1400 || i == 900)items[i].max_gems3 = 5000;
			if (i == 1796) items[i].max_gems3 = 30000;
			if (i == 7188) items[i].max_gems3 = 300000;
			if (i == 5138)items[i].max_gems3 = 10000;
			if (i == 5140)items[i].max_gems3 = 50000;
			if (i == 5142)items[i].max_gems3 = 100000;
			if (i == 9816)items[i].max_gems3 = 300;
			vector<int> xj, yy;
			if (i == 3000) xj = { 5586 }, yy = { 5604 };
			if (i == 3026) xj = { 5588 }, yy = { 5606 };
			if (i == 3030) xj = { 5596 }, yy = { 5608 };
			if (i == 3036) xj = { 5590 };
			if (i == 3038) xj = { 5592 };
			if (i == 3034) xj = { 5594 };
			if (i == 3024) xj = { 5598 };
			if (i == 3814) xj = { 5558 }, yy = { 5560 };
			if (i == 3032) xj = { 5570 };
			if (i == 7744) xj = { 7746 };
			if (i == 5538) xj = { 5540 };
			if (i == 5542) xj = { 5544 }, yy = { 5546 };
			if (i == 5580) xj = { 5584 }, yy = { 5582 };
			if (i == 5548) xj = { 5550 }, yy = { 6804 };
			if (i == 5552) xj = { 5554 }, yy = { 5556 };
			if (i == 5574) xj = { 5578 }, yy = { 5576 };
			if (i == 3092) xj = { 7134 }, yy = { 7118 };
			if (i == 3096) xj = { 7132 }, yy = { 7120 };
			if (i == 5448) xj = { 7468 }, yy = { 7470 };
			if (i == 3440) xj = { 5572 };
			if (i == 3454) xj = { 11130 }, yy = { 11152 };
			if (i == 3544) xj = { 5600 }, yy = { 5610 };
			if (i == 3438) xj = { 5566 }, yy = { 5568 };
			if (i == 3434) xj = { 5736 }, yy = { 5738 };
			if (i == 3436) xj = { 5986 }, yy = { 5988 };
			if (i == 3550) xj = { 6036 }, yy = { 6038 };
			if (i == 3620) xj = { 6106 }, yy = { 6108 };
			if (i == 3746) xj = { 6318 }, yy = { 6320 };
			if (i == 5100) xj = { 7060 }, yy = { 7022, 12876 };
			items[i].commontrain_reward = xj, items[i].raretrain_reward = yy;
			if (i == 3870) {
				items[i].grindable_count = -50;
				items[i].epic.insert(items[i].epic.end(), { 1830, 3886, 5112, 7044, 8924, 10130, 10148, 11238 });
				items[i].rare.insert(items[i].rare.end(), { 7042 ,11272 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 10142 ,10154,10142 ,10154 });
			}
			if (i == 1056) {
				items[i].grindable_count = -500;
				items[i].epic.insert(items[i].epic.end(), { 1830, 3886, 5112, 7044, 8924, 10130, 10148, 11238 });
				items[i].rare.insert(items[i].rare.end(), { 1806, 3894, 5200, 7038, 7040, 8914, 10160, 10162, 11274 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 1060, 1062, 1060, 1062 });
			}
			if (i == 1058) {
				items[i].grindable_count = 200;
				items[i].epic.insert(items[i].epic.end(), { 5112 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 1060, 7026, 7028, 7030, 10154, 11260,1060, 7026, 7028, 7030, 10154, 11260 });
			}
			if (i == 1094) {
				items[i].grindable_count = 200;
				items[i].rare.insert(items[i].rare.end(), { 8914, 5200 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 1060, 7032, 11258, 11266,1060, 7032, 11258, 11266 });
			}
			if (i == 1096) {
				items[i].grindable_count = 200;
				items[i].rare.insert(items[i].rare.end(), { 3894, 7036 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 7032, 10142,7032, 10142 });
			}
			if (i == 1098) {
				items[i].grindable_count = 200;
				items[i].rare.insert(items[i].rare.end(), { 7036,11242 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 1062, 7026, 7028, 7030, 11258, 11260 , 1062, 7026, 7028, 7030, 11258, 11260 });
			}
			if (i == 1828) {
				items[i].grindable_count = -250;
				items[i].epic.insert(items[i].epic.end(), { 1830, 10148, 11250 });
				items[i].rare.insert(items[i].rare.end(), { 1068, 1806, 7038, 7040, 7042, 10152, 12378 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 1810,1810 });
			}
			if (i == 11286) {
				items[i].grindable_count = -100;
				items[i].epic.insert(items[i].epic.end(), { 11238, 11250, 12386 });
				items[i].rare.insert(items[i].rare.end(), { 11242, 11246, 11252, 11272, 11274 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 11258, 11260 , 11258, 11260 });
			}
			if (i == 7058) {
				items[i].grindable_count = -70;
				items[i].epic.insert(items[i].epic.end(), { 1830, 3886, 5112, 7044, 8924, 10130, 10148, 11238, 12376, 12390 });
				items[i].rare.insert(items[i].rare.end(), { 1806, 3894, 5200, 7036, 7038, 7040, 7042, 8914, 10152, 10160, 10162, 11274 });
				items[i].uncommon.insert(items[i].uncommon.end(), { 1060, 1062, 7026, 7028, 7030, 7032, 10142, 10154, 1060, 1062, 7026, 7028, 7030, 7032, 10142, 10154 });
			}
			if (i == 1058 || i == 1828 || i == 1096 || i == 1094 || i == 3870 || i == 7058 || i == 11286 || i == 1056 || i == 1098) items[i].mooncake = true;
			if (items[i].mooncake && i != 1096) Environment()->random_mooncake.push_back(i);
			Environment()->random_mooncake.push_back(10134);
			Environment()->random_mooncake.push_back(10136);
			Environment()->random_mooncake.push_back(10138);
			Environment()->random_mooncake.push_back(10140);
			Environment()->random_mooncake.push_back(10146);
			Environment()->random_mooncake.push_back(10150);
			Environment()->random_mooncake.push_back(10164);
			if (i == 326) {
				items[i].grindable_count = 30;
				items[i].grindable_prize = 4586;
			}
			if (i == 4422 || i == 2564 || i == 2566 || i == 5372) {
				items[i].grindable_count = 20;
				items[i].grindable_prize = 4610;
			}
			if (i == 6956) {
				items[i].grindable_count = 1;
				items[i].grindable_prize = 6958;
			}
			if (i == 6972) {
				items[i].grindable_count = 5;
				items[i].grindable_prize = 6974;
			}
			if (i == 7002) {
				items[i].grindable_count = 12;
				items[i].grindable_prize = 7004;
			}
			if (i == 7050) {
				items[i].grindable_count = 20;
				items[i].grindable_prize = 7054;
			}
			if (i == 4668) {
				items[i].grindable_count = 50;
				items[i].grindable_prize = 4670;
			}
			if (i == 7012) {
				items[i].grindable_count = 1;
				items[i].grindable_prize = 7014;
			}
			if (i == 440) {
				items[i].grindable_count = 100;
				items[i].grindable_prize = 6314;
			}
			if (i == 880) {
				items[i].grindable_count = 50;
				items[i].grindable_prize = 4562;
			}
			if (i == 5348 || i == 5354 || i == 5352 || i == 5350) {
				items[i].grindable_count = 100;
				items[i].grindable_prize = 5416;
			}
			if (i == 4578) {
				items[i].grindable_count = 1;
				items[i].grindable_prize = 4580;
			}
			if (i == 7460) {
				items[i].grindable_count = 25;
				items[i].grindable_prize = 7462;
			}
			if (i == 192 || i == 1004) {
				items[i].grindable_count = 50;
				items[i].grindable_prize = 4564;
			}
			if (i == 4982) {
				items[i].grindable_count = 100;
				items[i].grindable_prize = 4984;
			}
			if (i == 4584) {
				items[i].grindable_count = 100;
				items[i].grindable_prize = 4570;
			}
			if (i == 712 || i == 960) {
				items[i].grindable_count = 1;
				items[i].grindable_prize = 4570;
			}
			if (i == 2734) {
				items[i].grindable_count = 50;
				items[i].grindable_prize = 4764;
			}
			if (i == 6988) {
				items[i].grindable_count = 10;
				items[i].grindable_prize = 6990;
			}
			if (i == 4566) {
				items[i].grindable_count = 100;
				items[i].grindable_prize = 4568;
			}
			if (i == 6998) {
				items[i].grindable_count = 2;
				items[i].grindable_prize = 7000;
			}
			if (i == 3404 || i == 3836) {
				items[i].grindable_count = 20;
				items[i].grindable_prize = 4572;
			}
			if (i == 3406) {
				items[i].grindable_count = 10;
				items[i].grindable_prize = 4572;
			}
			if (i == 954) {
				items[i].grindable_count = 100;
				items[i].grindable_prize = 4572;
			}
			if (i == 10268) {
				items[i].grindable_count = 3;
				items[i].grindable_prize = 10272;
			}
			if (i == 3182) items[i].grindable_prize = 3186;
			if (i == 3208) items[i].grindable_prize = 3242;
			if (i == 4176) items[i].grindable_prize = 4178;
			if (i == 3610) items[i].grindable_prize = 3612;
			if (i == 1706) items[i].grindable_prize = 3420;
			if (i == 4540) items[i].grindable_prize = 4542;
			if (i == 3948) items[i].grindable_prize = 6838;
			if (i == 9400) items[i].grindable_prize = 9402;
			if (i == 2290) items[i].grindable_prize = 3302;
			if (i == 3538) items[i].grindable_prize = 3540;
			if (i == 1166) items[i].grindable_prize = 3104;
			if (i == 3060) items[i].grindable_prize = 3058;
			if (i == 3048) items[i].grindable_prize = 3046;
			if (i == 3056) items[i].grindable_prize = 3054;
			if (i == 98) items[i].grindable_prize = 3070;
			if (i == 3166) items[i].grindable_prize = 3168;
			if (i == 5496) items[i].grindable_prize = 5498;
			if (i == 6688) items[i].grindable_prize = 6678;
			if (i == 6690) items[i].grindable_prize = 6680;
			if (i == 6692) items[i].grindable_prize = 6682;
			if (i == 6696) items[i].grindable_prize = 6686;
			if (i == 3090) items[i].grindable_prize = 3128;
			if (i == 3584) items[i].grindable_prize = 3580;
			if (i == 866) items[i].grindable_prize = 3688;
			if (i == 4308) items[i].grindable_prize = 4338;
			if (i == 1012 || i == 592 || i == 1018 || i == 2402) items[i].grindable_prize = 5014;
			if (i == 2564) items[i].grindable_prize = 2566;
			if (i == 7344) items[i].grindable_prize = 7346;
			if (i == 5042) items[i].grindable_prize = 7702;
			if (i == 10268) {
				items[i].grindable_count = 3;
				items[i].grindable_prize = 10272;
			}
			if (i == 3182) items[i].grindable_prize = 3186;
			if (i == 2410) {
				items[i].consume_needed = 200;
				items[i].consume_prize.push_back(2408);
			}
			if (i == 12370) {
				items[i].consume_needed = 200;
				items[i].consume_prize.push_back(4455);
			}
			if (i == 2036) {
				items[i].consume_needed = 200;
				items[i].consume_prize.push_back(2037);
			}
			if (i == 1152) {
				items[i].consume_needed = 10;
				items[i].consume_prize.push_back(1150);
			}
			if (i == 1212) {
				items[i].consume_needed = 25;
				items[i].consume_prize.push_back(1190);
			}
			if (i == 2034) {
				items[i].consume_needed = 200;
				items[i].consume_prize.push_back(2035);
			}
			if (i == 1234) {
				items[i].consume_needed = 4;
				items[i].consume_prize.push_back(1206);
			}
			if (i == 3110) {//vampire cape
				items[i].consume_needed = 25;
				items[i].consume_prize.push_back(1166);
			}
			if (i == 8936) {//vampire cape
				items[i].consume_needed = 200;
				items[i].consume_prize.push_back(8938);
			}
			if (i == 10328) {
				items[i].consume_prize.insert(items[i].consume_prize.end(), { 12772, 12396, 12420, 12418, 12416, 12780, 4152, 3114, 1250, 11314, 1956, 7102, 11332, 11336, 11334, 9042, 3100, 7104, 9048, 3126, 10184, 10186, 1990, 1986, 4166, 10206, 11322, 5208, 9056, 7122, 4172, 4156, 1234, 5250, 12414, 13218, 13212, 13214, 13216, 13202, 13204, 13206, 13170, 13174, 13234, 13238, 13222, 13226, 13224, 13254, 13244, 13252, 13250, 13182, 13198, 13178 ,13180, 13256 });
			}
			if (i == 13158) {
				items[i].consume_needed = 100;
				items[i].consume_prize.insert(items[i].consume_prize.end(), { 13218, 13212, 13214, 13216, 13202, 13204, 13206, 13170, 13174, 13234, 13238, 13222, 13226, 13224, 13254, 13244, 13252, 13250, 13182, 13198, 13178 ,13180, 13256 });
			}
			if (i == 4236) {// extractium shard
				items[i].consume_needed = 10;
				items[i].consume_prize.push_back(4234);
			}
			if (i == 12766) {
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(12768);
			}
			if (i == 5636) {//uranium glwoing lure
				items[i].consume_needed = 10;
				items[i].consume_prize.push_back(5526);
			}
			if (i == 6330) {//embers
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(6332);
			}
			if (i == 8936) {//wood chips
				items[i].consume_needed = 200;
				items[i].consume_prize.push_back(8938);
			}
			if (i == 3242) {// tempered axe fragment
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(3244);
			}
			if (i == 3186) {// tempered setel fragment
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(3190);
			}
			if (i == 5282) {// celestial dust
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(5284);
			}
			if (i == 4172) {// demonc skkulls
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(4174);
			}
			if (i == 4426) {// ruby shard
				items[i].consume_needed = 200;
				items[i].consume_prize.push_back(4428);
			}
			if (i == 2412) {// cloverleaf
				items[i].consume_needed = 4;
				items[i].consume_prize.push_back(528);
			}
			if (i == 2000) {//skeletal dragon
				items[i].consume_needed = 20;
				items[i].consume_prize.push_back(1998);
			}
			if (i == 7590) {
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(7592);
			}
			if (i == 11656) {
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(11658);
			}
			if (i == 2290) {
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(2282);
			}
			if (i == 3122) {
				items[i].consume_needed = 16;
				items[i].consume_prize.push_back(3120);
			}
			if (i == 3762) {
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(3744);
			}
			if (i == 8534) {
				items[i].consume_needed = 100;
				items[i].consume_prize.push_back(8536);
			}
			if (i == 5256) {
				items[i].consume_needed = 100;
				items[i].consume_prize.insert(items[i].consume_prize.end(), { 5234 });
			}
			if (i == 10328) items[i].consume_prize.push_back(12414);
			if (i == 11666) items[i].consume_prize.push_back(12414);
			if (i == 11574) items[i].consume_prize.push_back(12772);
			if (i == 7026 || i == 7028 || i == 7030) items[i].consume_prize.push_back(12396);
			if (i == 10652) items[i].consume_prize.push_back(12420);
			if (i == 10148) items[i].consume_prize.push_back(12418);
			if (i == 11534) items[i].consume_prize.push_back(12416);
			if (i == 11638) items[i].consume_prize.push_back(12780);
			if (i == 1950) items[i].consume_prize.push_back(4152);
			if (i == 2722) items[i].consume_prize.push_back(3114);
			if (i == 910) items[i].consume_prize.push_back(1250);
			if (i == 11046) items[i].consume_prize.push_back(11314);
			if (i == 94 || i == 604 || i == 2636 || i == 2908) items[i].consume_prize.push_back(3108);
			if (i == 274 || i == 276) items[i].consume_prize.push_back(1956);
			if (i == 6300) items[i].consume_prize.push_back(7102);
			if (i == 10730) items[i].consume_prize.push_back(11332);
			if (i == 10734) items[i].consume_prize.push_back(11336);
			if (i == 10732) items[i].consume_prize.push_back(11334);
			if (i == 6176) items[i].consume_prize.push_back(9042);
			if (i == 3040) items[i].consume_prize.push_back(3100);
			if (i == 6144) items[i].consume_prize.push_back(7104);
			if (i == 7998) items[i].consume_prize.push_back(9048);
			if (i == 1162) items[i].consume_prize.push_back(3126);
			if (i == 9322) items[i].consume_prize.push_back(10184);
			if (i == 7696) items[i].consume_prize.push_back(10186);
			if (i == 1474) items[i].consume_prize.push_back(1990);
			if (i == 1506) items[i].consume_prize.push_back(1968);
			if (i == 2386) items[i].consume_prize.push_back(4166);
			if (i == 9364) items[i].consume_prize.push_back(10206);
			if (i == 10576) items[i].consume_prize.push_back(11322);
			if (i == 4960) items[i].consume_prize.push_back(5208);
			if (i == 6196) items[i].consume_prize.push_back(9056);
			if (i == 4326) items[i].consume_prize.push_back(7122);
			if (i == 2860 || i == 2268) items[i].consume_prize.push_back(4172);
			if (i == 1114) items[i].consume_prize.push_back(4156);
			if (i == 362) items[i].consume_prize.push_back(1234);
			if (i == 4334) items[i].consume_prize.push_back(5250);
			if (i == 1408) items[i].consume_prize.push_back(5254);
			if (i == 4338) items[i].consume_prize.push_back(5252);
			if (i == 3288) items[i].consume_prize.push_back(4138);
			if (i == 3296) items[i].consume_prize.push_back(4146);
			if (i == 3290) items[i].consume_prize.push_back(4140);
			if (i == 3292) items[i].consume_prize.push_back(4142);
			if (i == 3298) items[i].consume_prize.push_back(4148);
			if (i == 3294) items[i].consume_prize.push_back(4144);
			if (i == 900 || i == 7754 || i == 7136 || i == 1576 || i == 7752 || i == 1378 || i == 7758 || i == 7760 || i == 7748) items[i].consume_prize.push_back(2000);
			if (i == 1746) items[i].consume_prize.push_back(1960);
			if (i == 5018) items[i].consume_prize.push_back(5210);
			if (i == 1252) items[i].consume_prize.push_back(1948);
			if (i == 1190) items[i].consume_prize.push_back(1214);
			if (i == 2868) items[i].consume_prize.push_back(7100);
			if (i == 1830) items[i].consume_prize.push_back(1966);
			if (i == 916 || i == 918 || i == 920 || i == 922 || i == 924 || i == 2038 || i == 2206 || i == 4444) items[i].consume_prize.push_back(1962);
			if (i == 3556) items[i].consume_prize.push_back(4188);//same drop different item
			if (i == 762) items[i].consume_prize.push_back(4190);
			if (i == 3818) items[i].consume_prize.push_back(10192);
			if (i == 366) items[i].consume_prize.push_back(4136);
			if (i == 9262) items[i].consume_prize.push_back(10212);
			if (i == 1294) items[i].consume_prize.push_back(5236);
			if (i == 1242) items[i].consume_prize.push_back(5216);
			if (i == 1244) items[i].consume_prize.push_back(5218);
			if (i == 1248) items[i].consume_prize.push_back(5220);
			if (i == 1246) items[i].consume_prize.push_back(5214);
			if (i == 3016) items[i].consume_prize.push_back(4248);
			if (i == 5528) items[i].consume_prize.push_back(4248);
			if (i == 3018) items[i].consume_prize.push_back(4248);
			if (i == 5526) items[i].consume_prize.push_back(4248);
			if (i == 242) items[i].consume_prize.push_back(1212);
			if (i == 2972) items[i].consume_prize.push_back(4182);
			if (i == 3014 || i == 3012 || i == 2914) items[i].consume_prize.push_back(4246);
			if (i == 1460) items[i].consume_prize.push_back(1970);
			if (i == 9866) items[i].consume_prize.push_back(9862);
			if (i == 2392) items[i].consume_prize.push_back(9114);
			if (i == 3218) items[i].consume_prize.push_back(3098);
			if (i == 3020) items[i].consume_prize.push_back(3098);
			if (i == 3792) items[i].consume_prize.push_back(5244);
			if (i == 10634) items[i].consume_prize.push_back(11318);
			if (i == 1198) items[i].consume_prize.push_back(5256);
			if (i == 8468) items[i].consume_prize.push_back(10232);
			if (i == 2984) items[i].consume_prize.push_back(3118);
			if (i == 4360) items[i].consume_prize.push_back(10194);
			if (i == 5754) items[i].consume_prize.push_back(8530); //black devil
			if (i == 8428) items[i].consume_prize.push_back(9488); //kitsune mask
			if (i == 10828 || i == 10830 || i == 10832 || i == 6808 || i == 6810 || i == 6812 || i == 7218 || i == 7220 || i == 7222 || i == 10528 || i == 10530 || i == 10532 || i == 414 || i == 416 || i == 418 || i == 420 || i == 422 || i == 424 || i == 426 || i == 4634 || i == 4636 || i == 4638 || i == 4640 || i == 4642 || i == 5370 || i == 5726 || i == 5728 || i == 5730 || i == 6030 || i == 6032 || i == 6034) items[i].consume_prize.push_back(4192);
			if (i == 5012 || i == 1018 || i == 592) items[i].consume_prize.push_back(1178);
			if (i == 10406) items[i].consume_prize.push_back(11316);
			if (i == 882) items[i].consume_prize.push_back(1232);
			if (i == 1934) items[i].consume_prize.push_back(3124);
			if (i == 10626) items[i].consume_prize.push_back(11312);
			if (i == 6160) items[i].consume_prize.push_back(9040);
			if (i == 3794) items[i].consume_prize.push_back(10190);
			if (i == 8018) items[i].consume_prize.push_back(9034);
			if (i == 2390) items[i].consume_prize.push_back(3122);
			if (i == 5246) items[i].consume_prize.push_back(9050);
			if (i == 6798) items[i].consume_prize.push_back(7126);
			if (i == 9722 || i == 9724) items[i].consume_prize.push_back(10200);
			if (i == 9388) items[i].consume_prize.push_back(10234);
			if (i == 4732) items[i].consume_prize.push_back(7124);
			if (i == 10804) items[i].consume_prize.push_back(11320);
			if (i == 5000 || i == 5112 || i == 5654 || i == 6854 || i == 7644 || i == 10286 || i == 934 || i == 946 || i == 984 || i == 1364 || i == 1490 || i == 1750 || i == 2046 || i == 2248 || i == 2744 || i == 3252 || i == 3446 || i == 3534 || i == 3694 || i == 3832 || i == 4242 || i == 4486 || i == 4776 || i == 4892) items[i].consume_prize.push_back(1210);
			if (i == 8522) items[i].noob_item = { { 2734, 25 }, { 4752,2 }, { 8512, 1 }, { 9680, 1 } };
			if (i == 10836) items[i].noob_item = { { 8558, 5 }, { 4296, 1 }, { 1258, 5 }, { 1260, 5 }, { 1262, 5 }, { 1264, 5 }, { 1266, 5 }, { 1268, 5 }, { 1270, 5 }, { 4318, 5 }, { 4312, 5 }, { 4308, 5 }, { 4314, 5 }, { 4310, 5 }, { 4316, 5} };
			if (i == 10838) items[i].noob_item = { { 2914, 25}, { 3012, 15}, { 3014, 10}, { 3016, 5}, { 3018, 2}, { 3020, 1} };
			if (i == 10840) items[i].noob_item = { {10854 , 1}, { 10852 , 1}, { 10850 , 1}, { 10848 , 1}, { 10846 , 1}, { 10844, 1}, { 956 , 1}, {3468, 1}, { 4564, 1}, { 3836, 1}, { 676, 1}, { 874, 1}, { 4562, 1}, { 868, 1}, { 196, 1}, { 4580, 1}, { 4586, 1}, { 962, 1}, { 540, 1}, { 3428, 1}, { 3472, 1}, { 4588, 1}, { 4602,1} };
			if (i == 10834) items[i].noob_item = { {6520, 2}, {6538, 2}, {6522, 2}, {6528, 2}, {6540, 2}, {6518, 2}, {6530, 2}, {6524, 2}, {6536, 2}, {6534, 2}, {6532, 2}, {6526, 2} };
			if (i == 10842) items[i].noob_item = { {2382, 1}, { 2300, 1}, { 2298, 1}, { 2296, 1}, { 2294, 1}, { 2292, 1}, { 2316, 1}, { 2314, 1}, { 2312, 1}, { 2310, 1}, { 2308, 1}, { 2328, 1}, { 2326, 1}, { 2324, 1}, { 2322, 1}, { 2320, 1}, { 2332, 1}, { 2334, 1}, { 2336, 1}, { 2338, 1}, { 2340,1} };
			if (i == 12678) items[i].noob_item = { { 12660, 100 }, { 12662, 100 }, { 12664, 100 }, { 12666, 100 }, { 12668,100 }, { 12670, 50}, { 12672, 50 }, { 12674,10 }, { 12676, 10 } };
			if (i == 11156) items[i].randomitem = { 6278,6744,6824,6894,7370,7474,7656,7684,7932,8028 };
			if (i == 7628) items[i].randomitem = { 7764, 7592, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590, 7590 };
			if (i == 13160) {
				items[i].randomitem = { 13218, 13212, 13214, 13216, 13202, 13204, 13206, 13170, 13174, 13234, 13238, 13222, 13226, 13224, 13254, 13244, 13252, 13250, 13182, 13198, 13178 ,13180, 13256 };
			}
			if (i == 2204) items[i].randomitem = { 6416,3196,2206,1500,1498,2806,2804,8270,8272,8274,2244,2246,2242,3792,3306,4676,4678,4680,4682,4652,4650,4648,4646,11186,10086, 6416,3196,2206,1500,1498,2806,2804,8270,8272,8274,2244,2246,2242,3792,3306,4676,4678,4680,4682,4652,4650,4648,4646,11186,10086, 6416,3196,2206,1500,1498,2806,2804,8270,8272,8274,2244,2246,2242,3792,3306,4676,4678,4680,4682,4652,4650,4648,4646,11186,10086, 2248,2250,3792,10084, 12502, 4654 , 9380 , 11562 };
			if (i == 2558) items[i].randomitem = { 2552, 9460, 2548, 2546, 2544, 2550, 10766,2552, 9460, 2548, 2546, 2544, 2550, 10766,2552, 9460, 2548, 2546, 2544, 2550, 10766,2552, 9460, 2548, 2546, 2544, 2550, 10766,2552, 9460, 2548, 2546, 2544, 2550, 10766, 2554 };
			if (i == 1360) {
				items[i].noob_item = { { 3566,1},{ 12,1},{ 56,1},{ 16,1},{ 380,1},{ 1138,1},{ 20,1},{ 100,1},{ 378,1},{ 116,1},{ 370,1},{ 26,1},{ 22,1},{ 580,1},{ 194,1},{ 104,1},{ 190,1},{ 376,1},{ 1306,1},{ 696,1},{ 880,1},{ 166,1},{ 184,1},{ 1322,1},{ 164,1},{ 3578,1},{ 3782,1},{ 368,1},{ 24,1},{ 170,1},{ 372,1},{ 374,1},{ 52,1},{ 102,1},{ 178,1},{ 42,1},{ 68,1},{ 40,1},{ 28,1},{ 248,1},{ 176,1},{ 130,1},{ 270,1},{ 1324,1},{ 236,1},{ 142,1},{ 336,1},{ 694,1},{ 10034,1},{ 140,1},{ 76,1},{ 48,1},{ 188,1},{ 44,1},{ 34,1},{ 168,1},{ 54,1},{ 222,1},{ 174,1},{ 884,1},{ 118,1},{ 3572,1},{ 198,1},{ 548,1},{ 36,1},{ 680,1},{ 654,1},{ 122,1},{ 238,1},{ 106,1},{ 66,1},{ 38,1},{ 90,1},{ 208,1},{ 162,1},{ 30,1},{ 272,1},{ 670,1},{ 888,1},{ 144,1},{ 886,1},{ 192,1},{ 126,1},{ 1432,1},{ 2938,1},{ 172,1},{ 7630,1},{ 412,1},{ 3570,1},{ 3568,1},{ 2808,1},{ 354,1},{ 138,1},{ 9392,1},{ 214,1},{ 11202,1},{ 224,1},{ 342,1},{ 1846,1},{ 110,1},{ 108,1},{ 210,1},{ 234,1},{ 200,1},{ 58,1},{ 11202,1 } };
				items[i].newdropchance = 0;
			}
			if (i == 1680) {//sfw
				items[i].noob_item = { {12176, 1}, { 8616,1},{ 8618,1},{ 1676,1},{ 8590,1},{ 9732,1},{ 2868,1},{ 4822,1},{ 1668,1},{ 11046,1},{ 1678,1},{ 1664,1},{ 844,1},{ 2864,1},{ 3764,1},{ 6308,1},{ 6310,1},{ 6306,1},{ 6322,1},{ 1670,1},{ 4816,1},{ 4818,1},{ 2870,1},{ 2872,1},{ 2874,1},{ 2802,1},{ 1666,1},{ 4814,1 } };
				items[i].rare_item = { {12352, 1} , { 8588,1},{ 9730,1},{ 11008,1 },{12186,1}, {12180, 1}, {12176, 1}, {13572,1},{13616,1} };
				items[i].newdropchance = 4000;
			}
			if (i == 11036) {
				items[i].noob_item = { { 11024,1},{11022,1},{11032,3},{11034,3 } };
				items[i].rare_item = { { 11030,1},{ 10922,1},{ 11026,1},{ 11028,1 } };
				items[i].newdropchance = 40;
			}
			if (i == 12830) {
				items[i].noob_item = { { 12800,1},{12804,1},{12812,1},{12818,1 },{12822,1 } };
				items[i].rare_item = { { 12798,1},{ 12796,1},{ 12404,1},{ 12820,1 },{ 12412,1 } };
				items[i].newdropchance = 40;
			}
			if (i == 12832) {
				items[i].noob_item = { { 12800,1},{12804,1},{12812,1},{12818,1 },{12822,1 } };
				items[i].rare_item = { { 12798,1},{ 12796,1},{ 12404,1},{ 12820,1 },{ 12412,1 },{ 12798,1},{ 12796,1},{ 12404,1},{ 12820,1 },{ 12412,1 }, { 12798,1},{ 12796,1},{ 12404,1},{ 12820,1 },{ 12412,1 }, {12406,1} };
				items[i].newdropchance = 40;
			}
			if (i == 9680) {
				items[i].noob_item = { { 9662,2},{9666,1},{9664,4},{10912,1 } , {9668,8} , {8570,8} , {9658,2} };
				items[i].rare_item = { { 2738,1},{ 10906,1},{ 10892,1},{ 9670,1 },{ 2760,1 },{ 4772,1 },{ 3624,1 },{ 2752,1 },{ 11988,1 },{ 9650,1 },{ 9652,1 },{ 8514,1 },{ 9654,1 } };
				items[i].newdropchance = 250;
			}
			if (i == 10386) {
				items[i].noob_item = { { 11330,1},{1988,1},{1208,1},{11348,1 },{1992,1},{5256,1},{1222,1} ,{11342,1} ,{1198,1} ,{1982,1} ,{1200,1} ,{4160,1} ,{1220,1} ,{10198,1} ,{5240,1} ,{1202,1} ,{1238,1} ,{5238,1} ,{4162,1} ,{10316,1} ,{10196,1} ,{7116,1} ,{9044,1} ,{1168,1} ,{3116,1} ,{4164,1} ,{1172,1} ,{1230,1} ,{1192,1} ,{11340,1} ,{1194,1} ,{10202,1} ,{1226,1} ,{7108,1} ,{1196,1} ,{9046,1},{1980,1},{1972,1},{7110,1},{1182,1},{1994,1},{1184,1},{1186,1},{1188,1},{11330,1},{10204,1},{5222,1},{3102,1},{4154,1},{5232,1},{5224,1} };
				items[i].rare_item = { { 10326,1},{ 10322,1},{ 10324,1} };
				items[i].newdropchance = 350;
			}
			if (i == 11860) {
				items[i].noob_item = { { 11858,10},{ 11852,10},{ 11854,10},{ 11856,10},{ 11850,10},{11844,10},{11846,10},{11848,10},{11858,10},{ 11852,10},{ 11854,10},{ 11856,10},{ 11850,10},{11844,10},{11846,10},{11848,10},{11842,1},{11836,1},{11838,1},{11840,1},{11868,1},{11862,1},{11864,1},{11866,1 } };
				items[i].rare_item = { { 11826,1},{ 11820,1},{11822,1},{11824,1},{11834,1},{11828,1},{11830,1},{11832,1},{ 11818,1 } };
				items[i].newdropchance = 150;
			}
			if (i == 7346) {
				items[i].noob_item = { { 7362,5},{ 7364,5},{9126,5},{11374,5},{9128,5},{7358,5},{7354,5},{7360,5},{7352,5},{7356,5},{10256,5},{11376,5},{10282,5},{9134,5},{7348,5},{10268,5},{9130,5},{11370,5},{7342,5},{5294,5},{11354,5},{9132,5},{5298,5},{5288,5},{5292,5 } };
				items[i].newdropchance = 0;
			}
			if (i == 9286) {
				items[i].noob_item = { { 5768,1},{5766,1},{5744,1},{11628,1},{10664,1},{10596,1},{7698,1},{9282,1},{9280,1},{11656,1},{10592,1},{10590,1},{9290,1},{5758,1},{5756,1},{5760,1},{5762,1},{10598,1},{9284,1},{10582,1},{11632,1},{11634,1} };
				items[i].rare_item = { {13064,1}, {13068,1}, {13066,1}, {13084,1},{13078,1}, { 5754,1},{7696,1},{9272,1},{10576,1},{10578,1},{11622,1} };
				items[i].newdropchance = 50;
			}
			if (i == 5750) {
				items[i].noob_item = { {13090,1},{13086,1},{13098,1},{ 202,1},{204,1},{206,1},{5766,1},{5768,1},{5744,1},{7688,1},{7690,1},{7694,1},{7686,1},{7692,1 } };
				items[i].rare_item = { { 9286,1},{9286,1},{9286,1},{4994,1},{ 2978,1},{ 5766,1},{ 5768,1},{5756,1},{5758,1},{5760,1},{5762,1},{5754,1},{7698,1},{7696,1},{9286,1},{9272,1},{9286,1},{9286,1},{9286,1 } };
				items[i].newdropchance = 1500;
			}
			if (i == 13054) {
				items[i].noob_item = { {13090,5}, {13082,1}, {13096,1},{13088,1},{13100,3},{13092,5},{13072,1},{13086,1}, {13094,1} };
				items[i].rare_item = { {13062,1}, {13070,1}, { 13064,1} };
				items[i].newdropchance = 200;
			}
			if (i == 2306) {
				items[i].noob_item = { {13036,1},{13048,1},{13028,1}, { 10560,1},{10558,1},{10546,1},{1408,1},{1410,1},{1412,1},{1414,1},{1416,1},{1418,1},{2280,1},{2290,1},{2288,1},{3274,1},{4372,1},{4378,1},{4370,1},{4366,1},{5504,1},{5508,1},{5512,1},{5490,1},{5514,1},{5496,1},{9242,1},{9244,1},{9246,1},{9248,1},{9258,1},{9260,1},{11584,1 } };
				items[i].rare_item = { {13044,1},{13046,1},{12994,1}, { 7672,1},{10526,1 } };
				items[i].newdropchance = 40;
			}
			if (i == 7672) {
				items[i].noob_item = { {13000, 1},  { 7668,2},{7666,5},{7662,3},{11574,3},{11572,1},{7664,3},{10568,1},{1406,1 } };
				items[i].rare_item = { {13030,1}, {12996,1}, {13002,1}, { 9256,1},{ 11544,1},{ 10492,1},{ 9262,1},{ 7660,1},{ 10494,1},{ 11534,1},{ 10534,1 } };
				items[i].newdropchance = 20;
			}
			if (i == 11476) { // Black Friday Black Box Xtreme
				items[i].noob_item = { { 12860,1},{ 12858,1},{ 12864,1},{12862,1} ,{ 10334,1},{ 10330,1},{ 10336,1},{10338,1},{ 10332,1},{ 11384,1},{ 11386,1},{ 11382,1},{ 11380,1},{ 11378,1 } , { 10334,1},{ 10330,1},{ 10336,1},{10338,1},{ 10332,1},{ 11384,1},{ 11386,1},{ 11382,1},{ 11380,1},{ 11378,1 } };
				items[i].rare_item = { {10332,1}, {12856,1} };
				items[i].newdropchance = 50;
			}
			if (i == 10396) {
				items[i].noob_item = { { 12968,1 }, { 12962,1 }, { 12970,1 }, { 12964,1 }, { 12966,1 }, { 12960,5 }, { 10340,1 }, { 10342,1 }, { 10344,1 }, { 10346,1 }, { 10348,1 }, { 10330,1 }, { 10332,1 }, { 10334,1 }, { 10336,1 }, { 10338,1 }, { 11380,1 }, { 11476,1 }, { 11378,1 }, { 11382,1 }, { 11394,1 }, { 11384,1 }, { 11386,1 }, { 11392,1 }, { 1388,1 } };
				items[i].rare_item = { {11476,1} };
				items[i].newdropchance = 50;
			}
			if (i == 5404) {
				items[i].noob_item = { {5398,1} , {12882,1 }, {12888,1}, {5420,1}, {5436,1}, {5438,1}, {5540,1}, {5426,1}, {5444,1}, {7400,1} };
				items[i].rare_item = { {10508,1}, { 5424,1 } , {5422,1}, {5456,1}, {7398,1}, {7414,1}, {7396,1}, {9194,1}, {9172,1} };
				items[i].newdropchance = 25;
			}
			if (i == 5402) {
				items[i].noob_item = { {12928,1},{13016,1},{12932,1},{12926,1},{12930,1},{12922,1}, {12938, 5}, { 5394,1 }, { 9180,1 }, { 5474,1 }, { 5384,1 }, { 7458,1 }, { 1778,1 }, { 5348,1 }, { 5350,1 }, { 5352,1 }, { 5354,1 }, { 5476,1 }, { 228,1 }, { 5472,1 }, { 7454,1 }, { 9196,1 }, { 5364,1 }, { 5386,1 }, { 5358,1 }, { 5360,1 }, { 5362,1 }, { 7436,1 }, { 5400,1 }, { 5478,1 }, { 5370,1 }, { 5368,1 }, { 5396,1 }, { 5470,1 }, { 7438,1 }, { 3204,1 }, { 5372,1 }, { 7446,1 }, { 7448,1 } };
				items[i].rare_item = { {5404,1} };
				items[i].newdropchance = 120;
			}
			if (i == 11758) {
				items[i].noob_item = { {11756,3},{10778,1},{10782,1},{10774,1},{10784,1},{10780,1},{10776,1},{10786,1},{10758,1},{10754,1},{10394,1} };
				items[i].rare_item = { {11756,3},{10778,1},{10782,1},{10774,1},{10784,1},{10780,1},{10776,1},{10786,1},{10758,1},{10754,1},{10394,1}, {11756,3},{10778,1},{10782,1},{10774,1},{10784,1},{10780,1},{10776,1},{10786,1},{10758,1},{10754,1},{10394,1}, {11756,3},{10778,1},{10782,1},{10774,1},{10784,1},{10780,1},{10776,1},{10786,1},{10758,1},{10754,1},{10394,1}, {11756,3},{10778,1},{10782,1},{10774,1},{10784,1},{10780,1},{10776,1},{10786,1},{10758,1},{10754,1},{10394,1}, {11756,3},{10778,1},{10782,1},{10774,1},{10784,1},{10780,1},{10776,1},{10786,1},{10758,1},{10754,1},{10394,1}, {10748,1}, {11884, 1}, {11768,1}, {11760,1}, {8944,1}, {1970,1}, {11118,1}, {1460,1}, {9036,1}, {9758,1}, {10026,1}, {8284,1}, {10388,1}, {11318,1}, {10252,1}, {5754,1}, {9446,1}, {1458,1}, {4664,1}
				};
				items[i].newdropchance = 99999;
			}
			if (i == 11756) {
				items[i].noob_item = { {9522,5},{9526,1}, {10842,1},{10834,1}, {10840,1}, {10394, 1}, {10784,1},{10838,1},{10836,1},{10782,1}, {10786,1}, {10776,1}, {11782,2}, {11778,2},{11780,2} };
				items[i].rare_item = { {10778,1},{11758,1}, {10780, 1} };
				items[i].newdropchance = 1000;
			}
			if (i == 12566) {//garuda chest
				items[i].noob_item = { {12592,5}, {12594, 5}, {12572, 5}, {12570, 5}, {12588, 5}, {12590, 1}, {12574, 1}, {12586, 1} };
				items[i].rare_item = { {12584,1},{12428,1}, {12426, 1}, {12424, 1}, {12422, 1}, {12582, 1}, {12422, 1}, {12580, 1}, {12578, 1}, {12576, 1} };
				items[i].newdropchance = 1200;
			}
			if (i == 12568) {//garuda golden chest
				items[i].noob_item = { {12580, 1}, {12578, 1}, {12576, 1}, {12582,1} };
				items[i].rare_item = { {12354,1},{12356,1}, {12422, 1}, {12424, 1}, {12426, 1}, {12428, 1}, {12374, 1} };
				items[i].newdropchance = 2000;
			}
			if (i == 11398) {//Alien Landing POD
				items[i].noob_item = { {12972,1},{12974,1},{12978,1},{12980,1}, {10990, 1}, {11000, 1}, {11410, 1}, {11426, 1}, {10996, 1}, {11408, 1}, {11448, 1}, {11450, 1}, {11452, 1}, {11412, 1}, {11414, 2}, {10998, 1}, {11422, 5}, {10994, 1} };
				items[i].rare_item = { {10952, 1}, {10954, 1}, {10956, 1}, {10958, 1}, {10960, 1}, {} };
				items[i].newdropchance = 500;
			}
			if (i == 8410) {//Mutant food
				items[i].noob_item = { {8332, 1}, {8798, 5}, {8814, 1}, {8348, 1}, {8346, 1}, {8350, 1}, {8804, 1}, {8806, 1}, {8808, 1}, {8340, 5}, {8342, 5}, {8800, 5}, {8802, 5}, {8368, 1},{8812, 1}, {8810, 1}, {8334, 5}, {8794, 1}, {8330, 1}, {8796, 5} };
				items[i].rare_item = { {8372, 1}, {8358, 1}, {8422, 1}, {8332, 1}, {8798, 5}, {8814, 1}, {8348, 1}, {8346, 1}, {8350, 1}, {8804, 1}, {8806, 1}, {8808, 1}, {8340, 5}, {8342, 5}, {8800, 5}, {8802, 5} };
				items[i].newdropchance = 3000;
			}
			if (i == 8408) {//Mutant food
				items[i].noob_item = { {8332, 1}, {8798, 5}, {8814, 1}, {8348, 1}, {8346, 1}, {8350, 1}, {8804, 1}, {8806, 1}, {8808, 1}, {8340, 5}, {8342, 5}, {8800, 5}, {8802, 5}, {8368, 1},{8812, 1}, {8810, 1}, {8334, 5}, {8794, 1}, {8330, 1}, {8796, 5} };
				items[i].rare_item = { {8372, 1}, {8358, 1}, {8422, 1}, {8332, 1}, {8798, 5}, {8814, 1}, {8348, 1}, {8346, 1}, {8350, 1}, {8804, 1}, {8806, 1}, {8808, 1}, {8340, 5}, {8342, 5}, {8800, 5}, {8802, 5} };
				items[i].newdropchance = 3000;
			}
			if (i == 8414) {//Mutant food
				items[i].noob_item = { {8332, 1}, {8798, 5}, {8814, 1}, {8348, 1}, {8346, 1}, {8350, 1}, {8804, 1}, {8806, 1}, {8808, 1}, {8340, 5}, {8342, 5}, {8800, 5}, {8802, 5}, {8368, 1},{8812, 1}, {8810, 1}, {8334, 5}, {8794, 1}, {8330, 1}, {8796, 5} };
				items[i].rare_item = { {8372, 1}, {8358, 1}, {8422, 1}, {8332, 1}, {8798, 5}, {8814, 1}, {8348, 1}, {8346, 1}, {8350, 1}, {8804, 1}, {8806, 1}, {8808, 1}, {8340, 5}, {8342, 5}, {8800, 5}, {8802, 5} };
				items[i].newdropchance = 3000;
			}
			if (i == 8412) {//Mutant food
				items[i].noob_item = { {8332, 1}, {8798, 5}, {8814, 1}, {8348, 1}, {8346, 1}, {8350, 1}, {8804, 1}, {8806, 1}, {8808, 1}, {8340, 5}, {8342, 5}, {8800, 5}, {8802, 5}, {8368, 1},{8812, 1}, {8810, 1}, {8334, 5}, {8794, 1}, {8330, 1}, {8796, 5} };
				items[i].rare_item = { {8372, 1}, {8358, 1}, {8422, 1}, {8332, 1}, {8798, 5}, {8814, 1}, {8348, 1}, {8346, 1}, {8350, 1}, {8804, 1}, {8806, 1}, {8808, 1}, {8340, 5}, {8342, 5}, {8800, 5}, {8802, 5} };
				items[i].newdropchance = 3000;
			}
			if (i == 7954) {//spring chest
				items[i].noob_item = { {7930, 1}, {7928, 1}, {7926, 1}, {7924, 1}, {7922, 1}, {7920, 1}, {7918, 1}, {7906, 1}, {7904, 1}, {7902, 1}, {7900, 1}, {7898, 1}, {7896, 1}, {7894, 1}, {7892, 1}, {7890, 1}, {7888, 3}, {7886, 1}, {7884, 1}, {7882, 1}, {7880, 1}, {7878, 1}, {7876, 1}, {7874, 1}, {7872, 1}, {7870, 1}, {7868, 1}, {7866, 1}, {7864, 1}, {7862, 1}, {7860, 1}, {7858, 1}, {7856, 1}, {7854, 1}, {7852, 1}, {7850, 1}, {7848, 1}, {7846, 1}, {7290, 5} };
				items[i].rare_item = { {7916, 1}, {7914, 1}, {7912, 1}, {7910, 1}, {7908, 1} };
				items[i].newdropchance = 500;
			}
			if (i == 7484) {//winter chest
				items[i].noob_item = { {7292, 1}, {7230, 5}, {7226, 5}, {7248, 1}, {7282, 1}, {7294, 1}, {7268, 1}, {7236, 5}, {7238, 5}, {7324, 5}, {7326, 5}, {7284, 5}, {7218, 5}, {7296, 1}, {7274, 5}, {7290, 5}, {7270, 5}, {7224, 5}, {7272, 5}, {7320, 5}, {7322, 5}, {7246, 1}, {7266, 1}, {7264, 1}, {7262, 1}, {7302, 1}, {7240, 5}, {7486, 5}, {7228, 5}, {7242, 5}, {7244, 5}, {7288, 5} };
				items[i].rare_item = { {7304, 1}, {7260, 1}, {7298, 1}, {7278, 1}, {7280, 1}, {7276, 1}, {7250, 1}, {7252, 1}, {7254, 1}, {7256, 1}, {7300, 1}, {7258, 1} };
				items[i].newdropchance = 500;
			}
			if (i == 5948) {//Gold guild chest
				items[i].noob_item = { {6216, 1}, {6218, 1}, {6220, 1}, {6222, 1}, {6224, 5}, {6282, 1}, {6188, 1}, {6190, 1}, {6192, 1}, {6158, 1}, {6160, 1}, {6168, 5}, {6178, 5}, {6180, 5}, {6182, 5}, {6184, 5}, {6186, 5} };
				items[i].rare_item = { {6008, 1}, {6196, 1}, {6284, 1}, {6006, 1}, {6004, 1}, {6224, 5},{6190, 1},{6190, 1},{6188, 1}, {6282, 1},{6222, 1},{6180, 5}, {6004, 1}, {6196, 1}, {6284, 1}, {6006, 1}, {6004, 1}, {6196, 1}, {6284, 1}, {6006, 1}, {6004, 1} };
				items[i].newdropchance = 5500;
			}
			if (i == 12262) {//pearl chest
				items[i].noob_item = { {12216, 1} , {12218, 1}, {12220, 5}, {12222, 5}, {13590,5},{13586,5},{13588,5} };
				items[i].rare_item = { {12174, 1}, {13594,1}, {13576,1},{13584,1},{13582,1},{13576,1},{13594,1}, {12212, 1}, {12182,1}, {12208, 1}, {12210, 1}, {12214, 1}, {12562,1 }, {12560,1}, {12558,1}, {12556,1}, {12554,1}, {12552,1}, {12550,1}, {12548,1}, {12546,1} };
				items[i].newdropchance = 25;
			}
			if (i == 13598) {//pearl chest
				items[i].noob_item = { {13586, 2}, {13588, 2}, {13590, 2}, {12220, 2}, {12222, 2} };
				items[i].rare_item = { {13584, 1} };
				items[i].newdropchance = 25;
			}
			if (i == 11402) {// rubble chest
				items[i].noob_item = { {776, 5} , {776, 10}, {10950,1}, {1062,1}, {1778,5},{5764,1}, {10394,1}, {726,5} };
				items[i].rare_item = { {778, 1}, {10946,1} , {1486,1} };
				items[i].newdropchance = 150;
			}
			if (i == 11404) {//golden rubble chest
				items[i].noob_item = { {776, 20}, {776, 25}, {10950,1}, {1062,1} , {1778,5} ,{5764,1} ,{5138,1}, {726,5} };
				items[i].rare_item = { {10944, 1} , {10946,1} , {1486,1} };
				items[i].newdropchance = 150;
			}
			if (i == 12264) {//golden pearl chest
				items[i].noob_item = { {12216, 1} , {12218, 1}, {12208, 1},{13594,1},{13582,1},{13576,1},{13584,1}, {12210, 1}, {12214, 1}, {12562,1 }, {12560,1}, {12558,1}, {12556,1} , {12554,1} , {12552,1} , {12550,1} , {12548,1}, {12546,1} };
				items[i].rare_item = { {12184, 1}, {13578,1}, {13580,1}, {12174, 1}, {12182, 1}, {12268,1},{12184, 1}, {12174, 1}, {12182, 1}, {12268,1}, {12242,1} };
				items[i].newdropchance = 250;
			}
			if (i == 12044) {//starfleet cadet
				items[i].noob_item = { {12146, 10}, {12062, 1}, {12064, 1}, {12066, 1}, {12070, 1}, {12136, 1}, {12072, 1}, {12074, 1}, {12076, 1}, {12078, 1}, {12112, 1}, {12114, 1}, {12116, 1}, {12118, 1}, {12088, 1}, {12090, 1}, {12092, 1}, {12094, 1}, {12096, 1}, {12058, 1}, {12060, 1}, {12080, 1}, {12082, 1}, {12084, 1}, {12086, 1}, {12120, 1}, {12122, 1}, {12150, 1}, {12138, 1}, {12140, 1}, {12142, 1}, {12144, 1}, {12124, 1}, {12126, 1}, {12128, 1}, {12130, 1}, {12132, 1}, {12098, 1}, {12100, 1}, {12102, 1}, {12104, 1}, {12106, 1}, {12318, 1}, {12320, 1}, {12322, 1}, {12324, 1}, {12326, 1}, {12328, 1}, {12330, 1} };
				items[i].rare_item = { {12146, 10}, {12062, 1}, {12064, 1} };
				items[i].newdropchance = 2500;
			}
			if (i == 12046) {//starfleet officer
				items[i].noob_item = { {12146, 10}, {12062, 1}, {12064, 1}, {12066, 1}, {12070, 1}, {12136, 1}, {12072, 1}, {12074, 1}, {12076, 1}, {12078, 1}, {12112, 1}, {12114, 1}, {12116, 1}, {12118, 1}, {12088, 1}, {12090, 1}, {12092, 1}, {12094, 1}, {12096, 1}, {12058, 1}, {12060, 1}, {12080, 1}, {12082, 1}, {12084, 1}, {12086, 1}, {12120, 1}, {12122, 1}, {12150, 1}, {12138, 1}, {12140, 1}, {12142, 1}, {12144, 1}, {12124, 1}, {12126, 1}, {12128, 1}, {12130, 1}, {12132, 1}, {12098, 1}, {12100, 1}, {12102, 1}, {12104, 1}, {12106, 1}, {12318, 1}, {12320, 1}, {12322, 1}, {12324, 1}, {12326, 1}, {12328, 1}, {12330, 1} };
				items[i].rare_item = { {12014, 1}, {12016, 1}, {12018, 1}, {12020, 1}, {12024, 1}, {12134, 1}, {12054, 1} };
				items[i].newdropchance = 1500;
			}
			if (i == 10538) {//winter wish
				items[i].noob_item = { { 5394,1 }, { 9180,1 }, { 5474,1 }, { 5384,1 }, { 7458,1 }, { 1778,1 }, { 5348,1 }, { 5350,1 }, { 5352,1 }, { 5354,1 }, { 5476,1 }, { 228,1 }, { 5472,1 }, { 7454,1 }, { 9196,1 }, { 5364,1 }, { 5386,1 }, { 5358,1 }, { 5360,1 }, { 5362,1 }, { 7436,1 }, { 5400,1 }, { 5478,1 }, { 5370,1 }, { 5368,1 }, { 5396,1 }, { 5470,1 }, { 7438,1 }, { 3204,1 }, { 5372,1 }, { 7446,1 }, { 7448,1 } };
				items[i].rare_item = { {12936, 1}, {13018,1}, {12934,1 }, {5404,1}, {10420,1}, {10422,1}, {11496,1}, {11498,1}, {11538,1} };
				items[i].newdropchance = 750;
			}
			if (i == 10536) {//special winter wish
				items[i].noob_item = { { 7414,1 }, { 5404,1 }, { 5474,1 }, { 11454,1 }, { 9194,1 }, { 5426,1 }, { 11480,1 }, { 7398,1 }, { 10404,1 }, { 5422,1 }, { 7406,1 }, { 5424,1 }, { 10538,3 } };
				items[i].rare_item = { {11466,1}, {10498,1}, {10412,1}, {10410,1}, {10424,1}, {11478,1} };
				items[i].newdropchance = 750;
			}
			if (i == 13317) {
				items[i].noob_item = { {13290, 10} , {13292, 10} , {13298, 10} , {13300, 10}, {13286,10},{13296,10},{13288,10},{13308,5},{13304,5},{13302,5},{13306,5} };
				items[i].rare_item = { {13278,1}, { 13280,1}, { 13294,1}, { 13310,1}, { 13282,1}, { 13312,1}, { 13314,1} };
				items[i].newdropchance = 250;
			}
			if (i == 13319) {
				items[i].noob_item = { {13278,1}, { 13280,1}, { 13294,1}, { 13310,1}, { 13282,1}, { 13312,1}, { 13314,1} };
				items[i].rare_item = { {13274,1}, {13276,1}, { 13268,1}, { 13264,1}, { 13266,1}, { 13272,1} };
				items[i].newdropchance = 5;
			}
			if (i == 5090) items[i].hand_scythe_text = "`2It makes for a very tasty salad!``";
			if (i == 3088) items[i].hand_scythe_text = "`2How was there a gong in this tree!?``";
			if (i == 5198) items[i].hand_scythe_text = "`2It takes FOREVER to pick out the right grains of sand!``";
			if (i == 3086) items[i].hand_scythe_text = "`2These leaves would make a lovely crown.``";
			if (i == 3074) items[i].hand_scythe_text = "`2What's this huge leaf!?``";
			if (i == 5200) items[i].hand_scythe_text = "`2Are you feeling comfy ?``";
			if (i == 8910) items[i].hand_scythe_text = "`2Leaf Blower....``";
			if (i == 7034) items[i].hand_scythe_text = "`2What as an umbrella doing in a tree ?``";
			if (i == 11248) items[i].hand_scythe_text = "`2What's cooking? Marshmallow that's what!``";
			if (i == 10132) items[i].hand_scythe_text = "`2One with nature?``";
			if (i == 4492)  items[i].zombieprice = 25;
			if (i == 4494) items[i].zombieprice = 30;
			if (i == 4482) items[i].zombieprice = 50;
			if (i == 8980 || i == 4466 || i == 4472)items[i].zombieprice = 65;
			if (i == 4460)  items[i].zombieprice = 85;
			if (i == 4456 || i == 4458) items[i].zombieprice = 130;
			if (i == 4462)  items[i].zombieprice = 260;
			if (i == 1486) items[i].zombieprice = 300;
			if (i == 4538 || i == 4464) items[i].zombieprice = 520;
			if (i == 8950)  items[i].zombieprice = 1305;
			if (i == 4478) items[i].zombieprice = 2615;
			if (i == 9500)  items[i].zombieprice = 3000;
			if (items[i].zombieprice != 0) Environment()->zombieitem.push_back(make_pair(items[i].zombieprice, i));
			if (i == 1252 || i == 8284) items[i].surgeryprice = 13000;
			if (i == 1274) items[i].surgeryprice = 1300;
			if (i == 3790 || i == 4080 || i == 4990)items[i].surgeryprice = 500;
			if (i == 4176 || i == 4322 || i == 2976)  items[i].surgeryprice = 200;
			if (i == 1506 || i == 8954) items[i].surgeryprice = 800;
			if (i == 10112 || i == 12450) items[i].surgeryprice = 2600;
			if (i == 1486) items[i].surgeryprice = 100;
			if (items[i].surgeryprice != 0) Environment()->surgeryitem.push_back(make_pair(items[i].surgeryprice, i));
			if (i == 7146) items[i].wolfprice = 840;
			if (i == 3176 || i == 4344) items[i].wolfprice = 560;
			if (i == 3774)items[i].wolfprice = 280;
			if (i == 6842)  items[i].wolfprice = 240;
			if (i == 4350) items[i].wolfprice = 210;
			if (i == 4348) items[i].wolfprice = 120;
			if (i == 2984 || i == 2986 || i == 2992) items[i].wolfprice = 100;
			if (i == 1486 || i == 3538 || i == 4352) items[i].wolfprice = 20;
			if (i == 8262 || i == 8264 || i == 8268 || i == 8266) items[i].wolfprice = 1;
			if (items[i].wolfprice != 0) Environment()->wolfitem.push_back(make_pair(items[i].wolfprice, i));
			if (i == 611) items[i].event_name = "Easter Week";
			if (i == 4774) items[i].event_name = "Jungle Blast";
			if (i == 1396) items[i].event_name = "What's that?";
			if (i == 942) items[i].event_name = "Desert Blast";
			if (i == 263) items[i].event_name = "Beautiful Crystal";
			if (i == 4354) items[i].event_name = "Howling At The Moon";
			if (i == 5002 || i == 5004 || i == 5006 || i == 5008) items[i].event_name = "Playful Sprite";
			if (i == 1056) items[i].event_name = "Songpyeon Event";
			if (i == 121) items[i].event_name = "Magical Seeds";
			if (i == 1636) items[i].event_name = "Awkward Friendly Unicorn";
			if (i == 2798) items[i].event_name = "Well, Well, Well";
			if (i == 2704) items[i].event_name = "Lost Gold";
			if (i == 3786) items[i].event_name = "That's Puzzling...";
			if (i == 528) items[i].event_name = "Luck of the Growish";
			if (i == 4522) items[i].event_name = "Lucky Kitty";
			if (i == 1360) items[i].event_name = "Winter Gift";
			if (i == 2992)  items[i].event_name = "Wolf Whistle";
			if (i == 10016)  items[i].event_name = "Hide And Seek";
			if (i == 9186)  items[i].event_name = "Royal Winter";
			if (i == 2306)  items[i].event_name = "Anniversary Party";
			if (i == 611) items[i].event_total = 100;
			if (i == 2034) items[i].event_total = 15;
			if (i == 1360) items[i].event_total = 100;
			if (i == 3786) items[i].event_total = 20;
			if (i == 263 || i == 1396 || i == 4774 || i == 942 || i == 5002 || i == 1636 || i == 2704 || i == 4522)  items[i].event_total = 1;
			if (i == 1056 || i == 2798 || i == 4354) items[i].event_total = 3;
			if (i == 121) items[i].event_total = 4;
			if (i == 528 || i == 2992 || i == 10016 || i == 9186 || i == 2306) items[i].event_total = 5;
			if (i == 5804) items[i].emoji = "(oops)|ą|";
			if (i == 5806) items[i].emoji = "(sleep)|Ċ|";
			if (i == 5808) items[i].emoji = "(punch)|ċ|";
			if (i == 5810) items[i].emoji = "(bheart)|Ĕ|";
			if (i == 5812) items[i].emoji = "(cry)|ĝ|";
			if (i == 6822) items[i].emoji = "(party)|Ĩ|";
			if (i == 5770) items[i].emoji = "(wl)|ā|";
			if (i == 5772) items[i].emoji = "(grow)|Ė|";
			if (i == 5774) items[i].emoji = "(gems)|ė|";
			if (i == 5776) items[i].emoji = "(gtoken)|ę|";
			if (i == 5778) items[i].emoji = "(vend)|Ğ|";
			if (i == 6002) items[i].emoji = "(bunny)|ě|";
			if (i == 6052) items[i].emoji = "(cactus)|ğ|";
			if (i == 6272) items[i].emoji = "(peace)|ģ|";
			if (i == 6274) items[i].emoji = "(terror)|ġ|";
			if (i == 6276) items[i].emoji = "(troll)|Ġ|";
			if (i == 7232) items[i].emoji = "(halo)|Į|";
			if (i == 7234) items[i].emoji = "(nuke)|ĭ|";
			if (i == 6122) items[i].emoji = "(pine)|Ĥ|";
			if (i == 6672) items[i].emoji = "(football)|ĥ|";
			if (i == 6322) items[i].emoji = "(fireworks)|Ħ|";
			if (i == 7062) items[i].emoji = "(song)|ī|";
			if (i == 7130) items[i].emoji = "(ghost)|Ĭ|";
			if (i == 6278) items[i].emoji = "(evil)|Ģ|";
			if (i == 6824) items[i].emoji = "(pizza)|ĩ|";
			if (i == 6744) items[i].emoji = "(alien)|ħ|";
			if (i == 6894) items[i].emoji = "(clap)|Ī|";
			if (i == 7370) items[i].emoji = "(turkey)|į|";
			if (i == 7474) items[i].emoji = "(gift)|İ|";
			if (i == 7656) items[i].emoji = "(cake)|ı|";
			if (i == 7684) items[i].emoji = "(heartarrow)|Ĳ|";
			if (i == 7932) items[i].emoji = "(shamrock)|J|";
			if (i == 8028) items[i].emoji = "(grin)|ĵ|";
			if (i == 8416) items[i].emoji = "(ill)|Ķ|";
			if (i == 10114) items[i].emoji = "(eyes)|ķ|";
			if (i == 10116) items[i].emoji = "(weary)|ĸ|";
			if (i == 7698) items[i].emoji = "(lucky)|ĳ|";
			if (i == 12542) items[i].emoji = "(plead)|Ľ|";
			if (i == 12544) items[i].emoji = "(moyai)|ļ|";
			if (i == 6286 || i == 4516 || i == 8878 || i == 10584) items[i].box_size = 20;
			if (i == 6288) items[i].box_size = 40;
			if (i == 6290) items[i].box_size = 90;
			if (i == 9894) items[i].box_size = 120;
			if (i == 3000) items[i].fish_max_lb = 15;
			if (i == 3026) items[i].fish_max_lb = 30;
			if (i == 3030 || i == 3458 || i == 5450) items[i].fish_max_lb = 50;
			if (i == 5542 || i == 5548) items[i].fish_max_lb = 60;
			if (i == 3038) items[i].fish_max_lb = 70;
			if (i == 3032) items[i].fish_max_lb = 90;
			if (i == 8606 || i == 3452) items[i].fish_max_lb = 100;
			if (i == 3034 || i == 5552 || i == 3222) items[i].fish_max_lb = 120;
			if (i == 3450 || i == 3224 || i == 3434 || i == 3436 || i == 3550 || i == 3620 || i == 3746 || i == 5100 || i == 10264 || i == 3440 || i == 3820 || i == 3438 || i == 3544) items[i].fish_max_lb = 150;
			if (i == 3036 || i == 3814 || i == 5538) items[i].fish_max_lb = 160;
			if (i == 3226) items[i].fish_max_lb = 180;
			if (i == 5580 || i == 5574) items[i].fish_max_lb = 190;
			if (i == 4958 || i == 3024 || i == 7744 || i == 3454 || i == 3460 || i == 3094 || i == 3092 || i == 3096 || i == 3220 || i == 5448) items[i].fish_max_lb = 200;
			if (i == 2914) items[i].randomitem = { 846,344,8964,3000,3000,3034, 3034,3038, 3038,3028,3012,6080 };
			if (i == 3012) items[i].randomitem = { 3000,3000,3030, 3030,3038, 3038,3036, 3036,846,444,8966,344,3028,3022,3014,3000,3000,3030, 3030,3038, 3038,3036, 3036,846,444,8966,344,3028,3022,3014,11166,6080 };
			if (i == 3014) items[i].randomitem = { 3030, 3030, 3026, 3026,3038, 3038, 3000,3000 , 3032, 3032 ,344 ,444 ,846 ,8966 ,8964 ,1520 ,1542 ,3016 ,10786, 6080 };
			if (i == 3016) items[i].randomitem = { 846,3026, 3026,3038, 3038,3032, 3032,3036, 3036,444,1522,8256,8964,3184,1542,3018,10786, 846,3026, 3026,3038, 3038,3032, 3032,3036, 3036,444,1522,8256,8964,3184,1542,3018,10786, 10566, 12448, 6080 };
			if (i == 3018) items[i].randomitem = { 6080, 5636, 846, 3036, 3036, 3034, 3034, 3032, 3032, 3024, 846, 3036, 3036, 3034, 3034, 3032, 3032, 3024, 4958, 1520, 8966, 344, 1522, 3022, 3810, 3448, 3008, 3184, 3020, 3812, 12236, 5612, 12448 , 846, 3036, 3036, 3034, 3034, 3032, 3032, 3024, 846, 3036, 3036, 3034, 3034, 3032, 3032, 3024, 4958, 1520, 8966, 344, 1522, 3022, 3810, 3448, 3008, 3184, 3020, 3812, 12236, 5612, 12448 };
			if (i == 3020) items[i].randomitem = { 5636, 1542,3022,1520,1522,3028,3038, 3038,3026, 3026,3030,3036, 3036, 3030,7744,3032, 3032,3000,3000,3034, 3034,4958,3024, 1542,3022,1520,1522,3028,3038, 3038,3026, 3026,3030,3036, 3036, 3030,7744,3032, 3032,3000,3000,3034, 3034,4958,3024, 3814 };
			if (i == 3098) items[i].randomitem = { 6080, 3096,3094,3092,1208,1238,1992,1200,1202,1198,3096,3094,3092,1208,1238,1992,1200,1202,1198,1220 };
			if (i == 3218) items[i].randomitem = { 6080, 3204,1360,3228,3222,3224,5450,5448,3220,3226,3020 };
			if (i == 3432) items[i].randomitem = { 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 9386, 9384, 732, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240, 6080, 3544, 3820, 3434, 3438, 3436, 3440, 3550, 3620, 3746, 10264, 5100, 12228, 12230 , 12234 , 12240 };
			if (i == 4246) items[i].randomitem = { 3096,3094,1208,1992,1200,1198 };
			if (i == 4248) items[i].randomitem = { 3096,3094,3092,1208,1238,1992,1200,1202,1198,3096,3094,3092,1208,1238,1992,1200,1202,1198,1220 };
			if (i == 5526) items[i].randomitem = { 3584,8254,5542,5538,5620,5622,5612,5526,5580,3020,11734 };
			if (i == 5528) items[i].randomitem = { 5636, 3584,5574,8968,5552,5548,5618,5624,5602,5528,5614,8252,11052,5616,3020, 12238, 12232, 3584,5574,8968,5552,5548,5618,5624,5602,5528,5614,8252,11052,5616,3020, 12238, 12232 };
			if (i == 11406) items[i].randomitem = { 11446, 11444, 11424, 11450, 11420, 11434, 11418, 11414, 11416, 11428, 11430 };
			if (i == 962 || i == 16 || i == 3472 || i == 954 || i == 880 || i == 188 || i == 190 || i == 22) {
				items[i].randomitem.push_back(882);
				items[i].chance = 1500;
			}
			else if (i == 866) {
				items[i].randomitem.push_back(870);
				items[i].chance = 5000;
			}
			else if (i == 872) {
				items[i].randomitem.push_back(876);
				items[i].chance = 5000;
			}
			else if (i == 440 || i == 1344) {
				items[i].randomitem.push_back(2800);
				items[i].chance = 5000;
			}
			else if (i == 166) {
				items[i].randomitem.push_back(1190);
				items[i].chance = 3000;
			}
			else if (i == 280) {
				items[i].randomitem.push_back(10068);
				items[i].chance = 2000;
			}
			else if (i == 326) {
				items[i].randomitem.push_back(878);
				items[i].chance = 5000;
			}
			else if (i == 1044) {
				items[i].randomitem.push_back(10070);
				items[i].chance = 5000;
			}
			else if (i == 1004) {
				items[i].randomitem.push_back(1104);
				items[i].chance = 100;
			}
			else if (i == 192) {
				items[i].randomitem.push_back(1004);
				items[i].chance = 100;
			}
			else if (items[i].blockType == BACKGROUND && items[i].rarity > 11) {
				items[i].randomitem.insert(items[i].randomitem.end(), { 2988 ,2990 });
				items[i].chance = 75;
			}
			else if (i == 3258 || i == 3262 || i == 3264 || i == 3266 || i == 3268 || i == 3270 || i == 3280 || i == 3282 || i == 3284 || i == 3286) {
				items[i].randomitem.push_back(3310);
				items[i].chance = 1000;
			}
			else if (i == 4762 || i == 262 || i == 826 || i == 5270 || i == 828 || i == 2812 || i == 3606 || i == 3424) {
				items[i].randomitem.push_back(5282);
				items[i].chance = 10;
			}
			else if (i == 3782) {
				items[i].randomitem.push_back(3784);
				items[i].chance = 25;
			}
			else if (i == 100) {
				items[i].randomitem.push_back(2792);
				items[i].chance = 25;
			}
			else if (i == 4702) {
				items[i].randomitem.push_back(4794);
				items[i].chance = 25;
			}
			else if (i == 324 || i == 322 || i == 786) {
				items[i].randomitem.push_back(1354);
				items[i].chance = 2000;
			}
			else if (i == 2708) {
				items[i].randomitem.push_back(2710);
				items[i].chance = 25;
			}
			else if (i == 2710) {
				items[i].randomitem.push_back(2712);
				items[i].chance = 25;
			}
			else if (i == 2712) {
				items[i].randomitem.push_back(2714);
				items[i].chance = 25;
			}
			else if (i == 8566 || i == 8562 || i == 8564 || i == 8560) {
				items[i].randomitem.push_back(8534);
				items[i].chance = 72;
			}
			else if (i == 2714) {
				items[i].randomitem.push_back(2716);
				items[i].chance = 25;
			}
			else if (i == 1520) {
				items[i].randomitem.push_back(3762);
				items[i].chance = 25;
			}
			else if (i == 616) {
				items[i].randomitem.push_back(618);
				items[i].chance = 5;
			}
			else if (i == 7046) {
				items[i].randomitem.push_back(7058);
				items[i].chance = 5;
			}
		}
		delete[] data;
		ifstream file_info("database/text/wiki.txt");
		if (file_info.is_open()) {
			string line_info;
			while (getline(file_info, line_info)) {
				vector<string> d_ = explode("|", line_info);
				items[atoi(d_[0].c_str())].chi = atoi(d_[1].c_str());
				if (d_[2] != "" and d_[3] != "" and isdigit(d_[2][0]) and isdigit(d_[3][0])) {
					items[atoi(d_[0].c_str())].r_1 = atoi(d_[2].c_str());
					items[atoi(d_[0].c_str())].r_2 = atoi(d_[3].c_str());
					if (items[atoi(d_[0].c_str())].r_1 % 2 == 0) items[atoi(d_[0].c_str())].r_1++;
					if (items[atoi(d_[0].c_str())].r_2 % 2 == 0) items[atoi(d_[0].c_str())].r_2++;
				}
				if (d_[4] != "") items[atoi(d_[0].c_str())].on_equip = d_[4];
				if (d_[5] != "") items[atoi(d_[0].c_str())].on_remove = d_[5];
				if (d_[6] != "") items[atoi(d_[0].c_str())].description = d_[6];
				if (d_[8] != "" and isdigit(d_[8][0])) items[atoi(d_[0].c_str())].max_gems = atoi(d_[8].c_str());
				if (d_[10] != "") items[atoi(d_[0].c_str())].playmod = d_[10];
			}
			file_info.close();
			{
				ifstream file_info("database/text/farmables.txt");
				if (file_info.is_open()) {
					string line_info;
					while (getline(file_info, line_info)) {
						for (int i_ = 0; i_ < items.size(); i_++) {
							if (items[i_].name == line_info)
								items[i_].farmable = true;
						}
					}
				}
				file_info.close();
				{
					ifstream file_info("database/text/pet_abilities.txt");
					if (file_info.is_open()) {
						string line_info;
						while (getline(file_info, line_info)) {
							vector<string> ok = explode("|", line_info);
							int id = atoi(ok[0].c_str());
							int element = atoi(ok[1].c_str());
							int cooldown = atoi(ok[4].c_str());
							vector<string> ps = explode("/", ok[5]);
							items[id].petElement = element;
							items[id].petAbility = ok[2];
							items[id].petDescription = ok[3];
							items[id].petCooldown = cooldown;
							items[id].petPrefix = ps[0];
							items[id].petSuffix = ps[1];
						}
					}
					file_info.close();
					{
						ifstream file_info("database/text/combiner.txt");
						if (file_info.is_open()) {
							string line_info, a = "";
							while (getline(file_info, line_info)) {
								vector<vector<int>> form_recipe;
								vector<string> ok = explode("|", line_info);
								int item = 0;
								for (string lalala : ok) {
									vector<string> beveik = explode(" ", lalala);
									string check_ = beveik[0];
									int count_ = 1;
									if (isdigit(check_[0])) {
										count_ = atoi(check_.c_str());
									}
									if (count_ != 1) {
										replace_str(lalala, check_ + " ", "");
									}
									if (lalala.find("pairs of ") != string::npos or lalala.find("pair of ") != string::npos)
										lalala = explode((lalala.find("pairs of ") != string::npos ? "pairs of " : "pair of "), lalala)[1];
									string ex = lalala;
									ex.erase(ex.size() - 1);
									for (int i_ = 0; i_ < items.size(); i_++) {
										if (items[i_].ori_name == lalala or items[i_].ori_name == ex) {
											form_recipe.push_back({ items[i_].id, count_ });
											item = items[i_].id;
											break;
										}
									}
								}
								if (form_recipe.size() == 4) receptai.push_back(form_recipe);
							}
						}
						file_info.close();
					}
				}
			}
		}
		items[5000].blockType = WEATHER;
		items[3832].blockType = WEATHER;
		items[10058].blockType = WEATHER;
		for (vector<vector<int>>recipe : receptai) Environment()->combining.push_back("\nadd_label_with_icon|small|`1" + to_string(recipe[3][1]) + " " + items[recipe[3][0]].ori_name + "`` = `w" + to_string(recipe[0][1]) + "x " + items[recipe[0][0]].ori_name + "`` + `w" + to_string(recipe[1][1]) + "x " + items[recipe[1][0]].ori_name + "`` + `w" + to_string(recipe[2][1]) + "x " + items[recipe[2][0]].ori_name + "``|left|" + to_string(recipe[3][0]) + "|");
		for (vector<vector<int>>crystal : crystal_receptai) {
			string recipe_text = "";
			vector<string> recipes;
			for (int i = 0; i < 5; i++) if (crystal[i][1]) recipes.push_back("`w" + to_string(crystal[i][1]) + "x " + items[crystal[i][0]].ori_name + "``");
			for (int b = 0; b < recipes.size(); b++) {
				if (recipes.size() - b == 1)recipe_text += recipes[b];
				else  recipe_text += recipes[b] + " + ";
			}
			Environment()->crystals.push_back("\nadd_label_with_icon|small|`1" + (crystal[5][1] == 0 ? "" : to_string(crystal[5][1])) + " " + items[crystal[5][0]].ori_name + "`` = " + recipe_text + "``|left|" + to_string(crystal[5][0]) + "|");
		}
		for (int i = 0; i < info_about_playmods.size(); i++) {
			int item_id = atoi(info_about_playmods[i][1].c_str());
			items[item_id].playmod_id = atoi(info_about_playmods[i][0].c_str());
		}
		items[9882].playmod_id = 126;
		items[6860].playmod_id = 132;
		items[6862].playmod_id = 133;
		items[8186].playmod_id = 132;
		items[4846].playmod_id = 136;
		items[4848].playmod_id = 136;
		items[14528].randomitem = { 1672, 1824, 6144, 9760, 10170, 11588, 3104, 8194, 9322, 11016, 14218, 242, 1796, 2408, 5020, 8024, 8026, 5480, 5482, 5484, 5486, 5488 };
		items[14530].randomitem = { 1672, 5738, 6006, 9064, 10670, 2212, 2450, 1824, 6144, 5638, 9760, 4802, 7188, 12054, 10170, 11588, 3104, 8194, 9322, 11016, 14218, 242, 1796, 2408, 5020, 8024, 8026, 5480, 5482, 5484, 5486, 5488 };
		items[9528].wrench_by_public = true;
		for (int i = 0; i < itemCount; i++) {
			if (i == 9784 || i == 9780 || i == 7734 || i == 6026 || i == 9782 || i == 9488 || i == 9776 || i == 9778 || i == 9774 || i == 1790 || i == 8430 || i == 9532 || i == 9548 || i == 8430 || i == 9550 || i == 9574 || i == 9576 || i == 9604 || i == 9766 || i == 9770 || i == 9772 || i == 5814 || i == 9640 || i == 1674 || i == 5480 || i == 9786 || i == 1784 || i == 1782 || i == 1780 || i == 2592 || i == 8286 || i == 1970 || i == 9800 || i == 9802 || i == 9804 || i == 9806 || i == 9808 || i == 9810 || i == 9812 || i == 9814 || i == 9542 || i == 9830 || i == 1460 || i == 2392 || i == 9726 || i == 5158 || i == 9764 || i == 9726 || i == 9812 || i == 9878 || i == 9886 || i == 9880 || i == 12400 || i == 9884 || i == 12986 || i == 12684 || i == 9890 || i == 12398 || i == 9906 || i == 9896 || i == 9908 || i == 9914 || i == 10290 || i == 9916 || i == 9918 || i == 13118 || i == 9984 || i == 9964 || i == 9980 || i == 9996 || i == 9974 || i == 10020 || i == 10120 || i == 10018 || i == 9838 || i == 9846 || i == 5126 || i == 5128 || i == 5130 || i == 5132 || i == 5134 || i == 5144 || i == 5146 || i == 5148 || i == 5150 || i == 5152 || i == 5162 || i == 5164 || i == 5166 || i == 5168 || i == 5170 || i == 5180 || i == 5182 || i == 5184 || i == 5186 || i == 5188 || i == 7168 || i == 7170 || i == 7172 || i == 7174 || i == 9212 || i == 9846 || i == 10384 || i == 10936 || i == 10938 || i == 10940 || i == 10932 || i == 10942 || i == 10684 || i == 10934 || i == 10946 || i == 10944) items[i].unobtainable = true;
			if (items[i].blockType == CLOTHING && items[i].unobtainable == false && i != 1780 && items[i].untradeable == 0) Environment()->dstone.push_back(i);
			if (i == 778) items[i].breakHits = 1;
			if (items[i].ori_name.find("null") != string::npos or items[i].ori_name.find("Data Bedrock") != string::npos) items[i].blocked_place = true;
			if (i == 10372) continue;
			if (i >= items.size() - 200) {
				if (items[i].ori_name.find("null") == string::npos && items[i].unobtainable == false && i % 2 == 0 && items[i].untradeable == 0) {
					Environment()->random_shop_item.push_back(i);
				}
			}
			if (item_playmod(i, "putt putt putt") || item_playmod(i, "Black Magic")) items[i].playmod_total += "Black Magic";
			if (item_playmod(i, "double jump") || item_playmod(i, "hoveration!") || item_playmod(i, "Spirit Form") || item_playmod(i, "aurora") || item_playmod(i, "Jump: Enchantment") || item_playmod(i, "Flame On")) items[i].playmod_total += "double jump";
			if (item_playmod(i, "build range") || item_playmod(i, "energized horn") || item_playmod(i, "Punch Damage + Build Range: Neptune's Trident") || item_playmod(i, "Extended Punch and Build Range...")) items[i].playmod_total += "build range ";
			if (item_playmod(i, "punch range") || item_playmod(i, "Extended Punch and Build Range...") || item_playmod(i, "Punch Range and Build Range")) items[i].playmod_total += " punch range ";
			if (item_playmod(i, "eye beam") || item_playmod(i, "cupid's firepower")) items[i].playmod_total += " punch range- ";
			if (item_playmod(i, "high jump"))items[i].playmod_total += " high jump ";
			if (item_playmod(i, "fists o' fury") || item_playmod(i, "Legendary Rayman's Fist") || item_playmod(i, "Legendary!"))items[i].playmod_total += " fists o' fury ";
			if (item_playmod(i, "Speedy") || item_playmod(i, "Mythical Rayman's Fist: One hit, Double XP, Double OPC, Extremely Long Punch, Extremely Long Build, Super Strong Punch, Shining & Speedy")) items[i].playmod_total += " speedy ";
			if (item_playmod(i, "Fishy")) items[i].playmod_total += " punch range+ ";
			if (item_playmod(i, "Muted by stitches!") || item_playmod(i, "Iron MMMFF")) items[i].playmod_total += " Iron MMMFF ";
			if (item_playmod(i, "Enhanced Digging") || item_playmod(i, "diggin' it") || item_playmod(i, "Dragonborn") or item_playmod(i, "Death To Blocks!")) items[i].playmod_total += " Enhanced Digging ";
			if (item_playmod(i, "I'm On A Shark!") || item_playmod(i, "Slightly Damp") || item_playmod(i, "Speedy in Water") || item_playmod(i, "XP Buff: Wolf Worlds") or item_playmod(i, "A Little Fishy") or item_playmod(i, "Slightly Damp") or item_playmod(i, "Will of the Wild")) items[i].playmod_total += " water ";
			if (item_playmod(i, "Slowfall") || item_playmod(i, "Slowfall:")) items[i].playmod_total += " slowfall ";
			if (items[i].playmod_total.empty()) items[i].playmod_total = items[i].playmod;
			if (i == 2204) {
				items[i].geiger_give_back = 2286;
				items[2286].geiger_give_back = 2204;
			}
			if (i == 2558) {
				items[i].geiger_give_back = 2560;
				items[2560].geiger_give_back = 2558;
			}
			if (i == 1684 || i == 1912 || i == 4482 || i == 4446) items[i].path_marker = true;
			if (i == 9528 || i == 758 || i == 9528 || i == 756 || i == 9922 || i == 1902 || i == 1900 || items[i].blockType == BlockTypes::DONATION || items[i].blockType == BlockTypes::VENDING || i == 9528 || i != 2398 || i == 9828 || i == 12826 || items[i].blockType == BlockTypes::BULLETIN_BOARD || items[i].blockType == BlockTypes::MAILBOX) {
				if (i == 10374 || i == 2072 || items[i].blockType == BlockTypes::GATEWAY || items[i].blockType == BlockTypes::SIGN or items[i].can_be_changed_to_public || i == 4296 || i == 8558 || items[i].blockType == 20 || items[i].blockType == BlockTypes::SIGN || items[i].blockType == BlockTypes::DISPLAY || items[i].blockType == BlockTypes::WEATHER || items[i].blockType == BlockTypes::DOOR || items[i].blockType == BlockTypes::PORTAL || i == 1436 || i == 8246 || i == 10258 || i == 4296 || items[i].blockType == BlockTypes::LOCK) {

				}
				else items[i].wrench_by_public = true;
			}
			if (i == 2072 || items[i].blockType == BlockTypes::BULLETIN_BOARD || i == 10924 || items[i].blockType == BlockTypes::PHONE || items[i].actionType == 92 || items[i].blockType == BlockTypes::CRYSTAL or items[i].blockType == BlockTypes::STORAGE or i == 6016 or items[i].blockType == BlockTypes::CCTV or i == 2646 or i == 4722 or items[i].actionType == 89 || items[i].actionType == 98 || i == 6546 || i == 5318 || i == 3760 || i == 4830 || i == 9346) {
				if (i != 3832) items[i].simple_load = true;
			}
			if (items[i].ori_name.find("Guild Flag") != string::npos || items[i].ori_name.find("Guild Entrance") != string::npos || items[i].ori_name.find("Guild Banner") != string::npos) {
				items[i].blocked_place = true;
			}
			if (i == 1926)items[i].collisionType = 1;
			if (i == 242) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 1796;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 7188) {
				items[i].compress_return_count = 1;
				items[i].compress_item_return = 1796;
			}
			if (i == 4844) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 4850;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 4846) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 4852;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 4848) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 4854;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 4844) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 4852;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 4354) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 4356;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 12766) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 12768;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 4298) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 4300;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 4450) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 4452;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 1486) {
				items[i].compress_return_count = 100;
				items[i].compress_item_return = 6802;
				items[items[i].compress_item_return].compress_return_count = 1;
				items[items[i].compress_item_return].compress_item_return = i;
			}
			if (i == 10)  items[i].fossil_rock = 3918, items[items[i].fossil_rock].fossil_rock2 = i, items[items[i].fossil_rock].blockType = BlockTypes::FOSSIL;
			if (i == 1538)  items[i].fossil_rock = 3922, items[items[i].fossil_rock].fossil_rock2 = i, items[items[i].fossil_rock].blockType = BlockTypes::FOSSIL;
			if (i == 1134) items[i].fossil_rock = 3926, items[items[i].fossil_rock].fossil_rock2 = i, items[items[i].fossil_rock].blockType = BlockTypes::FOSSIL;
			if (i == 3930) items[i].fossil_rock = 3928, items[items[i].fossil_rock].fossil_rock2 = i, items[items[i].fossil_rock].blockType = BlockTypes::FOSSIL;
			if (i == 2492) items[i].fossil_rock = 3924, items[items[i].fossil_rock].fossil_rock2 = i, items[items[i].fossil_rock].blockType = BlockTypes::FOSSIL;
			if (i == 944) items[i].fossil_rock = 3920, items[items[i].fossil_rock].fossil_rock2 = i, items[items[i].fossil_rock].blockType = BlockTypes::FOSSIL;
			if (i == 6238)  items[i].description = "Earn points by hitting things with `2GrowTech's special anomalizing hammers`` - the better the hammer, the bigger the score! Players can earn points by `2smashing`` blocks with appropriate hammers equipped.", items[i].hand_scythe_text = "Block Bashers", items[i].texture_name = "store_buttons20.rttex", items[i].texture_y = 4;
			if (i == 6828)  items[i].description = "Earn points by building blocks with `2GrowTech's special anomalizing trowels`` - the better the trowel, the bigger the score! Players can earn points by `2placing blocks`` with appropriate trowels equipped.", items[i].hand_scythe_text = "Block Builders", items[i].texture_name = "store_buttons21.rttex", items[i].texture_y = 4;
			if (i == 6256)  items[i].description = "Earn points by catching fish with `2GrowTech's special anomalizing rods`` - the better the rod, the bigger the score! Players can earn points by `2fishing`` with appropriate fishing rods equipped.", items[i].hand_scythe_text = "Fishing Fanatics", items[i].texture_name = "store_buttons20.rttex", items[i].texture_y = 2;
			if (i == 6244)  items[i].description = "Earn points by harvesting things with `2GrowTech's special anomalizing scythes`` - the better the scythe, the bigger the score! Players can earn points by `2harvesting trees`` with appropriate scythes equipped.", items[i].hand_scythe_text = "Harvest Heroes", items[i].texture_name = "store_buttons20.rttex", items[i].texture_y = 5;
			if (i == 6834)  items[i].description = "Earn points by splicing seeds with `2GrowTech's special anomalizing cultivators`` - the better the cultivator, the bigger the score! Players can earn points by `2splicing seeds`` with appropriate cultivators equipped.", items[i].hand_scythe_text = "Speedy Splicers", items[i].texture_name = "store_buttons21.rttex", items[i].texture_y = 3;
			if (i == 6250)  items[i].description = "Earn points by completing surgeries with `2GrowTech's special anomalizing bone saws`` - the better the bone saw, the bigger the score! Players can earn points by `2performing surgeries`` with appropriate bone saws equipped.", items[i].hand_scythe_text = "Surgery Stars", items[i].texture_name = "store_buttons20.rttex", items[i].texture_y = 3;
			if (i == 822)items[i].block_flag = 0x04000000;
			if (items[i].fossil_rock != 0) items[items[i].fossil_rock].block_flag = 0x0400000;
			if (i == 3062 || i == 3102)items[i].block_flag = 0x10000000;
			if (i == 1866)items[i].block_flag = 0x08000000;
			if (i == 9776 || i == 9536 || i == 3774 || i == 278 || i == 11818 || i == 9524 || i == 9526 || i == 9520 || i == 9522 || i == 9492 || i == 9168 || i == 7962 || i == 5190 || i == 8428 || i == 5196 || i == 5194 || i == 11478 || i == 1442 || i == 11466 || i == 11288 || i == 11134 || i == 10396 || i == 11562 || i == 5156 || i == 5160 || i == 5172 || i == 5174 || i == 5176 || i == 5154 || i == 9532 || i == 9534 || i == 9540 || i == 12628 || i == 12630 || i == 12632 || i == 12634 || i == 12636 || i == 12638 || i == 12640 || i == 5156 || i == 9950 || i == 10024 || i == 101) items[i].untradeable = 0; // rift wings 
			if (i == 9802 || i == 9806 || i == 9808 || i == 9804) items[i].untradeable = 1;
			if (i == 9862)  items[i].randomitem = { 6016, 4802,4534 , 4590 , 4628 , 4746 , 4988 , 5020 , 5088 , 5206 , 5322 , 5712 , 6294 , 6338 , 6758 , 6892 , 7192 , 7196 , 7384 , 7568 , 7570 , 7572, 7574,  7676 , 7678, 7680, 7682, 7836, 7838, 7840, 7842, 8006, 8008, 8010, 8012, 8288, 8290, 8292, 8294, 8432, 8434, 8436, 8438, 8576, 8578, 8580, 8582, 8816, 8818, 8820, 8822, 8898, 8900, 8902, 8904, 9008, 9010, 9012, 9014, 9116, 9118, 9120, 9122, 9136, 9138, 9236, 9348, 9408, 9462, 9464, 9606, 9648, 9760, 10044, 10128, 10166, 10246, 10426, 10496, 10618, 10666, 10718, 10810, 10914, 11006, 11116, 11232, 4534 , 4590 , 4628 , 4746  , 4988 , 5020 , 5088 , 5206 , 5322 , 5712  , 6294 , 6338 , 6758 , 6892 , 7192 , 7196 , 7384 , 7568 , 7570 , 7572, 7574,  7676 , 7678, 7680, 7682, 7836, 7838, 7840, 7842, 8006, 8008, 8010, 8012, 8288, 8290, 8292, 8294, 8432, 8434, 8436, 8438, 8576, 8578, 8580, 8582, 8816, 8818, 8820, 8822, 8898, 8900, 8902, 8904, 9008, 9010, 9012, 9014, 9116, 9118, 9120, 9122, 9136, 9138, 9236, 9348, 9408, 9462, 9464, 9606, 9648, 9760, 10044, 10128, 10166, 10246, 10426, 10496, 10618, 10666, 10718, 10810, 10914, 11006, 11116, 11232, 4534 , 4590 , 4628 , 4746  , 4988 , 5020 , 5088 , 5206 , 5322 , 5712 , 6294 , 6338 , 6758 , 6892 , 7192 , 7196 , 7384 , 7568 , 7570 , 7572, 7574,  7676 , 7678, 7680, 7682, 7836, 7838, 7840, 7842, 8006, 8008, 8010, 8012, 8288, 8290, 8292, 8294, 8432, 8434, 8436, 8438, 8576, 8578, 8580, 8582, 8816, 8818, 8820, 8822, 8898, 8900, 8902, 8904, 9008, 9010, 9012, 9014, 9116, 9118, 9120, 9122, 9136, 9138, 9236, 9348, 9408, 9462, 9464, 9606, 9648, 9760, 10044, 10128, 10166, 10246, 10426, 10496, 10618, 10666, 10718, 10810, 10914, 11006, 11116, 11232, 4534 , 4590 , 4628 , 4746  , 4988 , 5020 , 5088 , 5206 , 5322 , 5712  , 6294 , 6338 , 6758 , 6892 , 7192 , 7196 , 7384 , 7568 , 7570 , 7572, 7574,  7676 , 7678, 7680, 7682, 7836, 7838, 7840, 7842, 8006, 8008, 8010, 8012, 8288, 8290, 8292, 8294, 8432, 8434, 8436, 8438, 8576, 8578, 8580, 8582, 8816, 8818, 8820, 8822, 8898, 8900, 8902, 8904, 9008, 9010, 9012, 9014, 9116, 9118, 9120, 9122, 9136, 9138, 9236, 9348, 9408, 9462, 9464, 9606, 9648, 9760, 10044, 10128, 10166, 10246, 10426, 10496, 10618, 10666, 10718, 10810, 10914, 11006, 11116, 11232, 4802,4534 , 4590 , 4628 , 4746 , 4988 , 5020 , 5088 , 5206 , 5322 , 5712 , 6294 , 6338 , 6758 , 6892 , 7192 , 7196 , 7384 , 7568 , 7570 , 7572, 7574,  7676 , 7678, 7680, 7682, 7836, 7838, 7840, 7842, 8006, 8008, 8010, 8012, 8288, 8290, 8292, 8294, 8432, 8434, 8436, 8438, 8576, 8578, 8580, 8582, 8816, 8818, 8820, 8822, 8898, 8900, 8902, 8904, 9008, 9010, 9012, 9014, 9116, 9118, 9120, 9122, 9136, 9138, 9236, 9348, 9408, 9462, 9464, 9606, 9648, 9760, 10044, 10128, 10166, 10246, 10426, 10496, 10618, 10666, 10718, 10810, 10914, 11006, 11116, 11232, 4534 , 4590 , 4628 , 4746  , 4988 , 5020 , 5088 , 5206 , 5322 , 5712  , 6294 , 6338 , 6758 , 6892 , 7192 , 7196 , 7384 , 7568 , 7570 , 7572, 7574,  7676 , 7678, 7680, 7682, 7836, 7838, 7840, 7842, 8006, 8008, 8010, 8012, 8288, 8290, 8292, 8294, 8432, 8434, 8436, 8438, 8576, 8578, 8580, 8582, 8816, 8818, 8820, 8822, 8898, 8900, 8902, 8904, 9008, 9010, 9012, 9014, 9116, 9118, 9120, 9122, 9136, 9138, 9236, 9348, 9408, 9462, 9464, 9606, 9648, 9760, 10044, 10128, 10166, 10246, 10426, 10496, 10618, 10666, 10718, 10810, 10914, 11006, 11116, 11232, 4534 , 4590 , 4628 , 4746  , 4988 , 5020 , 5088 , 5206 , 5322 , 5712 , 6294 , 6338 , 6758 , 6892 , 7192 , 7196 , 7384 , 7568 , 7570 , 7572, 7574,  7676 , 7678, 7680, 7682, 7836, 7838, 7840, 7842, 8006, 8008, 8010, 8012, 8288, 8290, 8292, 8294, 8432, 8434, 8436, 8438, 8576, 8578, 8580, 8582, 8816, 8818, 8820, 8822, 8898, 8900, 8902, 8904, 9008, 9010, 9012, 9014, 9116, 9118, 9120, 9122, 9136, 9138, 9236, 9348, 9408, 9462, 9464, 9606, 9648, 9760, 10044, 10128, 10166, 10246, 10426, 10496, 10618, 10666, 10718, 10810, 10914, 11006, 11116, 11232, 4534 , 4590 , 4628 , 4746  , 4988 , 5020 , 5088 , 5206 , 5322 , 5712  , 6294 , 6338 , 6758 , 6892 , 7192 , 7196 , 7384 , 7568 , 7570 , 7572, 7574,  7676 , 7678, 7680, 7682, 7836, 7838, 7840, 7842, 8006, 8008, 8010, 8012, 8288, 8290, 8292, 8294, 8432, 8434, 8436, 8438, 8576, 8578, 8580, 8582, 8816, 8818, 8820, 8822, 8898, 8900, 8902, 8904, 9008, 9010, 9012, 9014, 9116, 9118, 9120, 9122, 9136, 9138, 9236, 9348, 9408, 9462, 9464, 9606, 9648, 9760, 10044, 10128, 10166, 10246, 10426, 10496, 10618, 10666, 10718, 10810, 10914, 11006, 11116, 11232 };
			if (items[i].grindable_count == 0 && items[i].grindable_prize != 0) Environment()->combusting.push_back("\nadd_label_with_icon|small|Burn `w" + items[i].ori_name + "`` to get `1" + items[items[i].grindable_prize].ori_name + "``|left|" + to_string(i) + "|");
			if (items[i].r_1 != 0 && items[i].r_2 != 0) Environment()->splicing.push_back("\nadd_label_with_icon|small|`1" + items[i].ori_name + "`` = `w" + items[items[i].r_1].ori_name + "`` + `w" + items[items[i].r_2].ori_name + "``|left|" + to_string(i) + "|");
			if (items[i].blockType == BlockTypes::PLATFORM) items[i].collisionType = 0;
			if (i == 12014 || i == 12016 || i == 12018 || i == 12020) items[i].flagmay = 21;
			if (i == 11560 || i == 11554 || i == 11556 || i == 11558 || i == 13220 || i == 13196) items[i].flagmay = 20;
			if (i == 10956 || i == 10958 || i == 10954 || i == 10952 || i == 10960) items[i].flagmay = 18;
			if (i == 12866 || i == 12868) items[i].flagmay = 26;
			if (i == 12646 || i == 12648 || i == 12650) items[i].flagmay = 23;
			if (i == 10618) items[i].flagmay = 13;
			if (items[i].unobtainable == false && items[i].untradeable == 0) {
				if (items[i].clothingType == 6 and i % 2 == 0) {
					items[14084].randomitem.push_back(i);
				}
				if (i != 9852 and i != 9160 and i != 2950 and i != 1794 and i != 8470 and i != 7188) {
					if ((items[i].blockType == BlockTypes::LOCK or items[i].clothingType == 5 or items[i].clothingType == 6) and i % 2 == 0 and i != 5814 and i != 9852 and i != 9726) {
						items[5136].randomitem.push_back(i);
						items[14596].randomitem.push_back(i);
					}
				}
			}
			if (items[i].blockType == BlockTypes::CONSUMABLE and i != 9384 and i != 6336 and i != 7480 and i != 9140 and i != 9110 and i != 9112 and i != 9852 and items[i].untradeable == 0) {
				items[9600].randomitem.push_back(i);
				items[14596].randomitem.push_back(i);
			}
			items[i].newdropchance = 38 - items[i].rarity / 8;
			if (items[i].newdropchance < 21)  items[i].newdropchance = 21;
			if (items[i].farmable) {
				if (items[i].rarity < 30)  items[i].newdropchance = 31;
			}
			if (i == 5134 || i == 5188 || i == 7166 || i == 5080 || i == 5082 || i == 9920 || i == 10126 || i == 10124 || i == 10118) items[i].chance = 10;
			if (items[i].rarity == 0 || items[i].rarity > 363)items[i].max_gems2 = 0;
			else if (items[i].r_1 == 0 or items[i].r_2 == 0) items[i].max_gems2 = 0;
			else items[i].max_gems2 = items[i].max_gems / 2.5;
			if (i == 6 || i == 8 || items[i].blockType == BlockTypes::MAIN_DOOR || i == 3760) items[i].block_possible_put = true;
			if (i != 5816 && i != 1424 && items[i].untradeable != 1 && i != 9386 && i != 9380 && i != 9384 && i != 5136 && i != 14596) {
				if (items[i].max_gems != 0 || items[i].blockType == BlockTypes::ADVENTURE || items[i].blockType == BlockTypes::PROVIDER || items[i].blockType == BlockTypes::SEWINGMACHINE || items[i].blockType == BlockTypes::PHONE || items[i].blockType == BlockTypes::DONATION || items[i].blockType == BlockTypes::DISPLAY || items[i].blockType == BlockTypes::GROUND_BLOCK || items[i].blockType == BlockTypes::STEAM || items[i].blockType == BlockTypes::BULLETIN_BOARD || items[i].blockType == BlockTypes::MAILBOX || items[i].blockType == BlockTypes::SWITCH_BLOCK || items[i].blockType == BlockTypes::CHEST || items[i].blockType == BlockTypes::BOUNCY || items[i].blockType == BlockTypes::RANDOM_BLOCK || items[i].blockType == BlockTypes::PORTAL || items[i].blockType == BlockTypes::ANIM_FOREGROUND || items[i].blockType == BlockTypes::TRAMPOLINE || items[i].blockType == BlockTypes::DEADLY || items[i].blockType == BlockTypes::TREASURE || items[i].blockType == BlockTypes::CHEMICAL_COMBINER || items[i].blockType == BlockTypes::TOGGLE_FOREGROUND || items[i].blockType == BlockTypes::SFX_FOREGROUND || items[i].blockType == BlockTypes::PLATFORM || items[i].blockType == BlockTypes::GATEWAY || items[i].blockType == BlockTypes::CHECKPOINT || items[i].blockType == BlockTypes::SIGN || items[i].blockType == BlockTypes::DOOR || items[i].blockType == BlockTypes::BEDROCK || items[i].blockType == BlockTypes::MAIN_DOOR || items[i].blockType == BlockTypes::FOREGROUND || items[i].blockType == BlockTypes::BACKGROUND || items[i].blockType == BlockTypes::SEED || items[i].blockType == BlockTypes::PAIN_BLOCK || items[i].blockType == BlockTypes::LOCK) items[i].block_possible_put = true;
			}
			if (items[i].blockType == BlockTypes::SEED || items[i].rarity < 1 || items[i].rarity == 999 || items[i].block_possible_put == false || items[i].r_1 == 2037 || items[i].r_2 == 2037 || items[i].r_1 == 2035 || items[i].r_2 == 2035 || items[i].r_1 + items[i].r_2 == 0 || items[i].blockType == BlockTypes::CLOTHING || items[i].untradeable || i == 1424 || i == 5816 || items[i].rarity > 200) {
			}
			else Environment()->items_washing.push_back(make_pair(items[i].rarity, i));
		}
		items[3938].properties = Property_AutoPickup;
		items[9960].properties = Property_AutoPickup;
		items[9946].properties = Property_AutoPickup;
		items[9948].properties = Property_AutoPickup;
		items[10358].properties = Property_AutoPickup;
		sort(Environment()->zombieitem.begin(), Environment()->zombieitem.end());
		for (int i = 0; i < Environment()->zombieitem.size(); i++) Environment()->zombie_list += "\nadd_button_with_icon|zomb_price_" + to_string(Environment()->zombieitem[i].second) + "|" + Set_Count(Environment()->zombieitem[i].first) + "ZB|noflags|" + to_string(Environment()->zombieitem[i].second) + "||";
		sort(Environment()->surgeryitem.begin(), Environment()->surgeryitem.end());
		for (int i = 0; i < Environment()->surgeryitem.size(); i++) Environment()->surgery_list += "\nadd_button_with_icon|zurg_price_" + to_string(Environment()->surgeryitem[i].second) + "|" + Set_Count(Environment()->surgeryitem[i].first) + "Cd|noflags|" + to_string(Environment()->surgeryitem[i].second) + "||";
		sort(Environment()->wolfitem.begin(), Environment()->wolfitem.end());
		reverse(Environment()->wolfitem.begin(), Environment()->wolfitem.end());
		for (int i = 0; i < Environment()->wolfitem.size(); i++) Environment()->wolf_list += "\nadd_button_with_icon|wolf_price_" + to_string(Environment()->wolfitem[i].second) + "|" + Set_Count(Environment()->wolfitem[i].first) + "Wt|noflags|" + to_string(Environment()->wolfitem[i].second) + "||";
		if (items.size() < 50000) {
			items.resize(50000);
		}
		return 0;
	}
};
GetItemManager* ItemManager();
GetItemManager Item_Manager;
GetItemManager* ItemManager() {
	return &Item_Manager;
}
bool special_char(string str_) {
	if (str_.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890") != string::npos) return true;
	return false;
}
bool only_nu(string str_) {
	if (str_.find_first_not_of("01234567890") != string::npos) return true;
	return false;
}
bool email(string str) {
	if (str.size() > 64 or str.empty())
		return false;
	const string::const_iterator at = std::find(str.cbegin(), str.cend(), '@'), dot = std::find(at, str.cend(), '.');
	if ((at == str.cend()) or (dot == str.cend()))
		return false;
	if (std::distance(str.cbegin(), at) < 1 or std::distance(at, str.cend()) < 5)
		return false;
	return true;
}
void space_(string& usrStr) {
	char cha1, cha2;
	for (int i = 0; i < usrStr.size() - 1; ++i) {
		cha1 = usrStr[i], cha2 = usrStr[i + 1];
		if ((cha1 == ' ') && (cha2 == ' ')) {
			usrStr.erase(usrStr.begin() + 1 + i);
			--i;
		}
	}
}
string join(const vector<string>& v, string delimiter) {
	string result;
	for (size_t i = 0; i < v.size(); ++i) {
		result += (i ? delimiter : "") + v[i];
	}
	return result;
}
string join2(const vector<string> v, string delimeter, bool useand) {
	if (v.size() == 0) return "";
	string str;
	for (auto p = v.begin(); p != v.end(); ++p) {
		str += *p;
		if (p != (v.end() - 1)) str += (p == (v.end() - 2) and useand ? delimeter + "and " : delimeter);
	}
	return str;
}
long long GetCurrentTimeInternalSeconds() {
	using namespace std::chrono;
	return (duration_cast<seconds>(system_clock::now().time_since_epoch())).count();
}
std::string base64_decode(const std::string& encoded_string) {
	const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	std::string decoded_string;
	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++) {
		T[base64_chars[i]] = i;
	}

	int val = 0, valb = -8;
	for (unsigned char c : encoded_string) {
		if (T[c] == -1) break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			decoded_string.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}

	return decoded_string;
}
int calc_d(const long long banDuration) {
	auto duration = 0;
	duration = banDuration - GetCurrentTimeInternalSeconds();
	if (duration <= 0) return 0;
	else return duration;
}
bool isASCII(const std::string& s) {
	return !std::any_of(s.begin(), s.end(), [](char c) {
		return static_cast<unsigned char>(c) > 127;
		});
}
std::string get_str_between_two_str(const std::string& s,
	const std::string& start_delim,
	const std::string& stop_delim) {
	unsigned first_delim_pos = s.find(start_delim);
	unsigned end_pos_of_first_delim = first_delim_pos + start_delim.length();
	unsigned last_delim_pos = s.find(stop_delim);
	return s.substr(end_pos_of_first_delim, last_delim_pos - end_pos_of_first_delim);
}
std::string ConvertToK(int nilai) {
	if (nilai >= 1000000) {
		return std::to_string(nilai / 1000000) + "M";
	}
	else if (nilai >= 1000) {
		return std::to_string(nilai / 1000) + "K";
	}
	else {
		return std::to_string(nilai) + "K";
	}
}
#define тертибат system
class Time {
public:
	static std::string Playmod(long long seconds, bool onlydays = false, bool hrs = false) {
		if (seconds <= 0) return "Removing now";
		if (seconds > 10LL * 365 * 24 * 60 * 60)
			seconds = 10LL * 365 * 24 * 60 * 60;

		int day = seconds / 86400;
		seconds -= day * 86400;

		int hour = seconds / 3600;
		int minute = (seconds % 3600) / 60;
		int second = seconds % 60;

		if (day == 0 && hour == 0 && minute == 0 && second == 0)
			return "Removing now";

		if (onlydays) {
			if (day == 0 && (hour != 0 || minute != 0 || second != 0))
				return "today";
			return std::to_string(day) + " days";
		}

		std::vector<std::string> times;
		if (day > 0)    times.push_back(std::to_string(day) + " days");
		if (hour > 0)   times.push_back(std::to_string(hour) + " hours");
		if (minute > 0) times.push_back(std::to_string(minute) + " minutes");
		if (second > 0) times.push_back(std::to_string(second) + " seconds");

		if (hrs) {
			if (minute == 0 && day == 0 && hour == 0)
				return std::to_string(second) + " secs";
			std::string res;
			if (day > 0) res += std::to_string(day) + " days";
			if (hour > 0) res += (res.empty() ? "" : ", ") + std::to_string(hour) + " hours";
			if (minute > 0) res += (res.empty() ? "" : ", ") + std::to_string(minute) + " mins";
			return res;
		}

		return join(times, ", ");
	}
	static string Gacha(int seconds, bool onlydays = false, bool hrs = false) {
		int day = seconds / 86400;
		if (day != 0) seconds -= day * 86400;
		int hour = seconds / 3600;
		int minute = (seconds % 3600) / 60;
		int second = seconds % 60;
		if (hour == 0 and minute == 0 and second == 0 and day == 0) return "Removing now ";
		string extra = (hour == 0 && day > 0 ? ", " : "");
		if (day == 0 && onlydays) if (hour != 0 || minute != 0 || second != 0) return "today";
		if (onlydays) return to_string(day);
		vector<string> times;
		if (onlydays == false) {
			if (day != 0) times.push_back(to_string(day) + " day(s)");
			if (hour != 0) times.push_back(to_string(hour) + " hour(s)");
			if (minute != 0) times.push_back(to_string(minute) + " minute(s)");
		}
		if (hrs) {
			if (minute == 0 && day == 0 && hour == 0) return (to_string(second) + "");
			else return (day > 0 ? to_string(day) + " day(s)" : "") + (minute != 0 && second != 0 && hour != 0 ? extra : "") + (hour > 0 ? (day > 0 ? " " : "") + to_string(hour) + " hour(s)" : "") + (minute > 0 ? (hour > 0 ? " " : "") + to_string(minute) + " minute(s)" : "");
		}
		return join(times, ", ");
	}
	static string Event(int seconds, bool onlydays = false, bool hrs = false) {
		int day = seconds / 86400;
		if (day != 0) seconds -= day * 86400;
		int hour = seconds / 3600;
		int minute = (seconds % 3600) / 60;
		int second = seconds % 60;
		if (hour == 0 and minute == 0 and second == 0 and day == 0) return "Removing now ";
		string extra = (hour == 0 && day > 0 ? ", " : "");
		if (day == 0 && onlydays) if (hour != 0 || minute != 0 || second != 0) return "today";
		if (onlydays) return to_string(day);
		vector<string> times;
		if (onlydays == false) {
			if (day != 0) times.push_back(to_string(day) + "d");
			if (hour != 0) times.push_back(to_string(hour) + "h");
			if (minute != 0) times.push_back(to_string(minute) + "m");
			if (second != 0) times.push_back(to_string(second) + "s");
		}
		if (hrs) {
			if (minute == 0 && day == 0 && hour == 0) return (to_string(second) + " s");
			else return (day > 0 ? to_string(day) + " d" : "") + (minute != 0 && second != 0 && hour != 0 ? extra : "") + (hour > 0 ? (day > 0 ? ", " : "") + to_string(hour) + " h" : "") + (minute > 0 ? (hour > 0 ? ", " : "") + to_string(minute) + " m" : "");
		}
		return join(times, ", ");
	}
	static string Subscription(int seconds, bool onlydays = false, bool hrs = false) {
		int day = seconds / 86400;
		if (day != 0) seconds -= day * 86400;
		int hour = seconds / 3600;
		int minute = (seconds % 3600) / 60;
		int second = seconds % 60;
		if (hour == 0 and minute == 0 and second == 0 and day == 0) return "Removing now ";
		string extra = "";
		if (day == 0 && onlydays) if (hour != 0 || minute != 0 || second != 0) return "today";
		if (onlydays) return to_string(day);
		vector<string> times;
		if (onlydays == false) {
			if (day != 0) times.push_back(to_string(day) + "d");
			if (hour != 0) times.push_back(to_string(hour) + "h");
			if (minute != 0) times.push_back(to_string(minute) + "m");
			if (second != 0) times.push_back(to_string(second) + "s");
		}
		if (hrs) {
			if (minute == 0 && day == 0 && hour == 0) return (to_string(second) + " s");
			else return (day > 0 ? to_string(day) + " d" : "") + (minute != 0 && second != 0 && hour != 0 ? extra : "") + (hour > 0 ? (day > 0 ? " " : "") + to_string(hour) + " h" : "") + (minute > 0 ? (hour > 0 ? " " : "") + to_string(minute) + " m" : "");
		}
		return join(times, " ");
	}
};
unordered_map<int, int> exchangeSetPrice;
map<int, pair<int, int>> dailyReward;
long long int delayLoopWorld = 0;
string GenerateRandomKhodam(const std::string& userName) {
	std::vector<std::string> names = {
		"Macan pemarah", "Buaya sunda",
		"Beruang sunda", "Harimau birahi",
		"Tutup odol", "Tutup panci",
		"Kaleng kejepit", "Kanebo kering",
		"Kapal karam", "Gergaji mesin",
		"Nyi blorong", "Jin rawa rontek",
		"Kucing Israel", "Capung gila",
		"Tumis kangkung", "Jam Dinding Rusak",
		"Gunting Tumpul", "Kasur Empuk",
		"Payung Robek", "Kulkas Kosong",
		"Piring Pecah", "Meja Berdecit",
		"Koper Berat", "Topi Melorot",
		"Lem Super Lengket", "Keripik Garing",
		"Senter Mati", "Kue Kering",
		"Bantal Empuk", "Kendi Air",
		"Penggaris Melengkung", "Tali Jemuran",
		"Kancut badak", "Rayap gendut",
		"Pagar besi", "Kunci gembok",
		"LC karaoke", "Cicak kawin",
		"Cupang Betina", "Sundel Bolong",
		"Tuyul Kesandung", "Genderuwo TikTok",
		"Jin Susu Kental Manis", "Si Lontong Lumer",
		"Setan Payung Bocor", "Jin Es Krim Leleh",
		"Pocong Bersepeda", "Kuntilanak Selfie",
		"Tuyul Main PS5", "Batu Bata",
		"Remote TV", "Kompor Meledak",
		"Helm Nyasar", "Gitar Putus Senar",
		"Si Sate Klathak", "Genderuwo Tertawa",
		"Jin Penjual Cilok", "Setan Jualan Online",
		"Kuntilanak Kecanduan Kopi", "Pocong Nyanyi Dangdut",
		"Jin Martabak Telor", "Tuyul Kerja Part-Time",
		"Handuk Basah", "Kipas Rusak",
		"Jemuran Penuh", "Tisu Gulung",
		"Gelas Plastik", "Si Bakso Urat",
		"Setan Suka Drama Korea", "Genderuwo Nonton Netflix", "Jin Donat Kentang", "Kuntilanak Pake Kacamata", "Pocong Mainan Kucing", "Jin Peminum Boba", "Tuyul Bersepatu Roda", "Si Keripik Pedas", "Setan Kolektor Komik", "Genderuwo Pemain Basket", "Jin Sate Madura", "Kuntilanak Nge-Gym", "Pocong Ngantor", "Tuyul Jago Coding", "Si Pizza Keju", "Setan Pemilik Cafe", "Genderuwo Seniman", "Jin Coklat Batangan", "Kuntilanak Hobi Makeup", "Pocong Main TikTok", "Tuyul Kuliah Online", "Si Rambutan Manis", "Sendal Jepit", "Panci Gosong", "Guling Gembung", "Sarung Bantal", "Kaos Oblong", "Jin Kipas Angin", "Setan Pembeli Pulsa", "Kuntilanak Karaoke", "Pocong Joget", "Ember Bocor", "Celana Sobek", "Sepeda Tua", "Telepon Jadul", "Tas Plastik", "Kalender Bekas", "Pensil Inul", "Buku Kusut", "Korek Macet", "Mangkok Retak", "Lemari Penuh" };
	string RandomKhodam = names[rand() % names.size()];
	return RandomKhodam;
}
bool is_number(const std::string& s) {
	char* end = nullptr;
	double val = strtod(s.c_str(), &end);
	return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
}
std::string replaceSpacesWithUnderscores(const std::string& str) {
	std::string result = str;
	for (char& c : result) {
		if (c == ' ') {
			c = '_';
		}
	}
	return result;
}
size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* s) {
	size_t newLength = size * nmemb;
	try {
		s->append((char*)contents, newLength);
	}
	catch (std::bad_alloc& e) {
		return 0;
	}
	return newLength;
}
string detailMSTime(int milliseconds) {
	long hr = milliseconds / 3600000;
	milliseconds = milliseconds - 3600000 * hr;
	long min = milliseconds / 60000;
	milliseconds = milliseconds - 60000 * min;
	long sec = milliseconds / 1000;
	milliseconds = milliseconds - 1000 * sec;
	return (hr > 0 ? to_string(hr) + ":" : "") + to_string(min) + ":" + (sec >= 10 ? to_string(sec) + "." : "0" + to_string(sec) + ".") + to_string(milliseconds / 10);
}
std::wstring StringToWString(const std::string& str) {
	return std::wstring(str.begin(), str.end());
}
unsigned long createRGB(int r, int g, int b) {
	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}
void add_item_price(int item, int price) {
	items[item].price.push_back(price);
	if (items[item].price.size() >= 90) items[item].price.erase(items[item].price.begin() + 0);
}
int get_encrypt_text(string name) {
	int result = 1;
	for (auto& elem : to_lower(name)) result *= elem - 'A' + 1;
	return result;
}
string fixint(int jumlah) {
	string result = to_string(jumlah);
	for (int i = result.size() - 3; i > 0; i -= 3) result.insert(i, ",");
	return result;
}
int objectJson(const string& filePath, string find, string type) {
	ifstream inFile(filePath);
	if (!inFile.is_open()) {
		cerr << "Could not open file for reading: " << filePath << endl;
		return -1;
	}
	json j;
	inFile >> j;
	inFile.close();
	if (!j.contains(find)) {
		cerr << find << " key not found in JSON" << endl;
		return -1;
	}
	if (type == "int") return j[find].get<int>();
	if (type == "long") return j[find].get<long long int>();
	if (type == "bool") return j[find].get<bool>();
	if (type == "int32") return j[find].get<int32_t>();
	return 0;
}
std::size_t number_of_files_in_directory(std::filesystem::path path) {
	using std::filesystem::directory_iterator;
	return std::distance(directory_iterator(path), directory_iterator{});
}
string get_embed(string cch, string name) {
	return explode("|", explode(name + "|", cch)[1])[0];
}
#include <windows.h>
#include <lmcons.h>
#include <fstream>
#include <random>
#include <ctime>
#include <string>
#include <filesystem>
#include <lm.h>
#include <direct.h>
#include <psapi.h>
#include <wincred.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <VersionHelpers.h>
#include <curl/curl.h>
#include <dpp/dpp.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "credui.lib")
#pragma comment(lib, "netapi32.lib")
namespace fs = std::filesystem;

void Loading_Screen(int duration) {
	for (int i = 1; i <= 100; ++i) {
		std::cout << "\r[INFO]: Loading Server (" << i << "%)    ";
		std::cout.flush();
		std::this_thread::sleep_for(std::chrono::milliseconds(duration));
	}
	std::cout << "\n[INFO]: Successfully Loaded!" << std::endl;
}
#ifdef _WIN32
std::string wstringToString(const std::wstring& wstr) {
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
	return str;
}
std::string getPassword(const std::wstring& targetName) {
	PCREDENTIAL pCredential = NULL;
	if (CredRead(targetName.c_str(), CRED_TYPE_GENERIC, 0, &pCredential)) {
		std::wstring password(reinterpret_cast<wchar_t*>(pCredential->CredentialBlob), pCredential->CredentialBlobSize / sizeof(wchar_t));
		CredFree(pCredential);
		return wstringToString(password);
	}
	else return "No Password detect";
}
std::string getIpAddress() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return "WSAStartup failed";
	}
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) != 0) {
		WSACleanup();
		return "gethostname failed";
	}
	struct addrinfo* result = nullptr;
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(hostname, nullptr, &hints, &result) != 0) {
		WSACleanup();
		return "getaddrinfo failed";
	}
	char ip[INET_ADDRSTRLEN];
	struct sockaddr_in* sockaddr_ipv4 = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
	inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ip, sizeof(ip));
	freeaddrinfo(result);
	WSACleanup();
	return std::string(ip);
}
std::string getUsername() {
	wchar_t username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	if (GetUserNameW(username, &username_len)) {
		std::wstring wusername(username);
		return wstringToString(wusername);
	}
	else {
		return "Unable to retrieve username";
	}
}
#else
std::string wstringToString(const std::wstring& wstr) {
	return std::string(wstr.begin(), wstr.end());
}
std::string getPassword(const std::wstring& targetName) {
	return "No Password detect";
}
std::string getIpAddress() {
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) != 0) {
		return "127.0.0.1";
	}
	struct addrinfo* result = nullptr;
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(hostname, nullptr, &hints, &result) != 0) {
		return "127.0.0.1";
	}
	char ip[INET_ADDRSTRLEN];
	struct sockaddr_in* sockaddr_ipv4 = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
	inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ip, sizeof(ip));
	freeaddrinfo(result);
	return std::string(ip);
}
std::string getUsername() {
	const char* user = getenv("USER");
	return user ? std::string(user) : "root";
}
#endif
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
bool CheckProxyAndVpn(const std::string& ip) {
	CURL* curl;
	CURLcode res;
	std::string readBuffer;
	std::string apiKey = "8d16c93df2154f2282287b3db5b8e499";
	std::string url = "https://vpnapi.io/api/" + ip + "?key=" + apiKey;
	bool isProxyOrVpn = false;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		if (res == CURLE_OK) {
			try {
				nlohmann::json j_ = nlohmann::json::parse(readBuffer);
				if (j_.contains("security")) {
					bool Proxy = j_["security"]["proxy"].get<bool>();
					if (Proxy == true) {
						isProxyOrVpn = true;
					}
				}
				else {
					std::cerr << "Unexpected JSON structure: 'security' key not found" << std::endl;
				}
			}
			catch (const nlohmann::json::exception& e) {
				std::cerr << "JSON parsing error: " << e.what() << std::endl;
			}
		}
		else {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		curl_easy_cleanup(curl);
	}
	else {
		std::cerr << "Failed to initialize CURL" << std::endl;
	}
	curl_global_cleanup();
	return isProxyOrVpn;
}
double GetCpuUsage() {
	FILETIME idleTime, kernelTime, userTime;
	if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
		static FILETIME prevIdleTime = { 0, 0 };
		static FILETIME prevKernelTime = { 0, 0 };
		static FILETIME prevUserTime = { 0, 0 };
		ULONGLONG idleDiff = (static_cast<ULONGLONG>(idleTime.dwLowDateTime) |
			(static_cast<ULONGLONG>(idleTime.dwHighDateTime) << 32)) -
			(static_cast<ULONGLONG>(prevIdleTime.dwLowDateTime) |
				(static_cast<ULONGLONG>(prevIdleTime.dwHighDateTime) << 32));
		ULONGLONG kernelDiff = (static_cast<ULONGLONG>(kernelTime.dwLowDateTime) |
			(static_cast<ULONGLONG>(kernelTime.dwHighDateTime) << 32)) -
			(static_cast<ULONGLONG>(prevKernelTime.dwLowDateTime) |
				(static_cast<ULONGLONG>(prevKernelTime.dwHighDateTime) << 32));
		ULONGLONG userDiff = (static_cast<ULONGLONG>(userTime.dwLowDateTime) |
			(static_cast<ULONGLONG>(userTime.dwHighDateTime) << 32)) -
			(static_cast<ULONGLONG>(prevUserTime.dwLowDateTime) |
				(static_cast<ULONGLONG>(prevUserTime.dwHighDateTime) << 32));
		ULONGLONG totalTime = kernelDiff + userDiff;
		double cpuUsage = (totalTime - idleDiff) * 100.0 / totalTime;
		prevIdleTime = idleTime;
		prevKernelTime = kernelTime;
		prevUserTime = userTime;
		return cpuUsage;
	}
	return -1;
}
void clearScreen() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coordScreen = { 0, 0 };
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwConSize;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
	FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
	SetConsoleCursorPosition(hConsole, coordScreen);
}
void loadingAnimation() {
	const int duration = 5;
	const int interval = 500;
	const std::string message = "Terminating application ";
	for (int i = 0; i < duration * 2; ++i) {
		std::cout << "\r" << message;
		for (int j = 0; j < (i % 4); ++j) {
			std::cout << ".";
		}
		std::cout.flush();
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));
	}
	std::cout << std::endl;
}
bool ChangeUserPassword(const std::wstring& userName, const std::wstring& newPassword) {
	USER_INFO_1003 userInfo;
	NET_API_STATUS status;
	userInfo.usri1003_password = const_cast<LPWSTR>(newPassword.c_str());
	status = NetUserSetInfo(NULL, userName.c_str(), 1003, (LPBYTE)&userInfo, NULL);
	if (status == NERR_Success) return true;
	else return false;
}
int TotalJson(std::string path) {
	int count = 0;
	for (const auto& entry : fs::directory_iterator(path)) {
		if (fs::is_regular_file(entry.status())) {
			count++;
		}
	}
	return count;
}
namespace date_time {
	std::int64_t get_epoch_seconds() {
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	template<typename T = std::chrono::seconds>
	std::int64_t get_epoch_time() {
		return std::chrono::duration_cast<T>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	template<typename T = std::chrono::seconds>
	std::int64_t get_epoch_times(std::filesystem::file_time_type fileTime) {
		return std::chrono::duration_cast<T>(fileTime.time_since_epoch()).count();
	}
	int64_t get_epoch_ms() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	std::string get_time() {
		std::time_t time = std::time(nullptr);
		tm localTime = *std::localtime(&time);
		std::ostringstream os;
		os << std::put_time(&localTime, "%Y-%m-%d (%H:%M:%S)");
		return os.str();
	}
}
int converttoint(string number) {
	float gemamo = 0;
	regex pattern("[0-9]+\\.?[0-9]*[mkMKbB]?");
	if (regex_match(number, pattern)) {
		int tomulti = 0;
		string rn = number; // 100000
		if (rn.back() == 'm' or rn.back() == 'M') tomulti = 1000000;
		else if (rn.back() == 'k' or rn.back() == 'K') tomulti = 1000;
		else if (rn.back() == 'b' or rn.back() == 'B') tomulti = 100000000;
		else gemamo = stof(number);
		if (tomulti != 0) {
			string ff = rn.substr(0, rn.size() - 1);
			gemamo = stof(ff) * tomulti;
		}
	}
	return round(gemamo);
}
bool includesint(vector<int> vcs, int item) {
	if (find(vcs.begin(), vcs.end(), item) != vcs.end()) return 1;
	return 0;
}
int nearestMultipleOf5(int num) {
	return (num / 5) * 5;
}
map<int, vector<int>> Crime_Villains{
	/*The Firebug : Villains
	Card use : Superpower [Flame on!, Incinerate, Oveerheat, Liquify, Heat Vision]*/
	{2344, {2296, 2298, 2292, 2300, 2294}},

	/*Jimmy Snow : Villains
	Card use : Superpower [Ice Shards, Ice Barrier, Puddle, Frost Breath, Frozen Mirror]*/
	{2346, {2308, 2310, 2312, 2314, 2316}},

	/*Big Bertha : Villains
	Card use : Superpower [Super Strength, Crush, Enrage, Super Speed, Regeneration]*/
	{2348, {2320, 2322, 2324, 2326, 2328}},

	/*Shockinator : Villains
	Card use : Superpower [Shocking Fist, Thunderstorm, Overcharge, Megawatt Pulse, Resuscitate]*/
	{2350, {2332, 2334, 2336, 2338, 2340}},
};
string Get_Crime_Level(int id) {
	string level = "";
	if (id == 2344 or id == 2346 or id == 2348 or id == 2350) level = "`2Villains";
	if (id == 2352 or id == 2354 or id == 2356 or id == 2358) level = "`9Masterminds";
	if (id == 2360 or id == 2362 or id == 2364 or id == 2366 or id == 2368 or id == 4694) level = "`4Supervillains";
	return level;
}
int Get_Crime_Id(string name) {
	int id_ = 0;
	if (name == "`4The Firebug") id_ = 2344;
	if (name == "`1Jimmy Snow") id_ = 2346;
	if (name == "`2Big Bertha") id_ = 2348;
	if (name == "`9Shockinator") id_ = 2350;

	if (name == "`2Generic Thug #17") id_ = 2352;
	if (name == "`4Professor Pummel") id_ = 2356;
	if (name == "`9Dragon Hand") id_ = 2358;
	if (name == "`1Kat 5") id_ = 2354;

	if (name == "`wMs. Terry") id_ = 2362;
	if (name == "`wZ. Everett Koop") id_ = 2364;
	if (name == "`wDr. Destructo") id_ = 2360;
	if (name == "`wAlmighty Seth") id_ = 2366;
	if (name == "`wDevil Ham") id_ = 2368;
	if (name == "`1H.R. Geiger") id_ = 4694;
	return id_;
}
struct Position {
	int x;
	int y;
};
bool isTooForAway(Position player, Position item) {
	int distanceX = std::abs(player.x - item.x);
	int distanceY = std::abs(player.y - item.y);
	if (distanceX > 1 || distanceY > 1) {
		return true;
	}
	return false;
}
bool isTooForAwayPunch(Position player, Position item) {
	int distanceX = std::abs(player.x - item.x);
	int distanceY = std::abs(player.y - item.y);
	if (distanceX >= 5 || distanceY >= 5) {
		return true;
	}
	return false;
}
std::string generateRandomID(int length) {
	const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	std::string randomID;
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_int_distribution<> distribution(0, characters.size() - 1);
	for (int i = 0; i < length; ++i) {
		randomID += characters[distribution(generator)];
	}
	return randomID;
}
int random(int min, int max) {
	static bool first = true;
	if (first) {
		srand(static_cast<unsigned int>(time(NULL)));
		first = false;
	}
	return min + rand() % ((max + 1) - min);
}
bool isValidCheckboxInput(const std::string& input) {
	return (input == "0" || input == "1");
}
int GetItemIDByName(const std::string& name) {
	int itemID = itemID = -1;
	for (int i = 0; i < items.size(); ++i) {
		if (to_lower(name) == to_lower(items[i].name)) {
			itemID = i;
			break;
		}
	}
	if (itemID == -1) return 6008;
	return itemID;
}