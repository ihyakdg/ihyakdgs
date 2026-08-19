/*GrowTopia Private Server v1.1 by LyChie#8807 / Aditya Dyansyh*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <queue>
#include <algorithm>
#include <thread>
#include <chrono>
#include <csignal>
#include <functional>
#include <fstream>
#include <csignal>
#include <unordered_set>
#include "enet_new/include/enet/enet.h"
#include "include/read_item_packet_data.h"
#include "include/nlohmann/json.hpp"
#include "include/proton/rtparam.hpp"
#include "include/HTTPRequest.hpp"
#include "NopySource/Handle/ItemDefination.h"
#include "NopySource/Handle/BaseServer.h"
#include "NopySource/Handle/PlayerInfo.h"
#include "NopySource/Handle/PacketHandler.h"
#include "NopySource/Handle/GuildInfo.h"
#include "NopySource/Handle/WorldInfo.h"
#include "NopySource/Handle/CrashHandler.cpp"
#include "NopySource/Handle/DialogReturn.h"
#include "NopySource/Handle/LoginHandler.h"
#include "NopySource/Handle/HTTP.h"
#include "NopySource/Action/action.h"
#include "NopySource/Dialog/dialog_return.h"
#include "NopySource/Server/server_pool.h"
#include "NopySource/Utils/Utils.hpp"
#include "FirewallHelper.h"
#include "NopySource/Commands/Commands.h"
#include "NopySource/Handle/DailyReward.h"
#include <windows.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#ifdef _WINDOWS_
#include <windows.h>
#include <chrono>
#define LOG_CALLSTACK(reason) log_callstack(__FILE__, __LINE__, reason)

#include <atomic>
#include <mutex>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstring>
// g_ghostThreadRun removed: ghost logic now runs inside Looping::worldEvent() (no separate thread)
// g_worldsMutex & g_enetMutex removed — unused, risk of deadlock if used inconsistently
// g_world_loop_mutex defined as inline in BaseServer.h

#include <iostream>

static void PrintBanner() {
	std::cout <<
		R"(███╗   ██╗ ██████╗ ██████╗ ██╗   ██╗
████╗  ██║██╔═══██╗██╔══██╗╚██╗ ██╔╝
██╔██╗ ██║██║   ██║██████╔╝ ╚████╔╝ 
██║╚██╗██║██║   ██║██╔═══╝   ╚██╔╝  
██║ ╚████║╚██████╔╝██║        ██║   
╚═╝  ╚═══╝ ╚═════╝ ╚═╝        ╚═╝   
)" << std::endl;
}


std::unique_ptr<HTTPServer> g_server;
BOOL WINAPI ConsoleHandler(DWORD dwType) {
	switch (dwType) {
	case CTRL_LOGOFF_EVENT: case CTRL_SHUTDOWN_EVENT: case CTRL_CLOSE_EVENT: {
		ServerPool::Trigger();
		return TRUE;
	}
	default:
		break;
	}
	return FALSE;
}
#else
#define Sleep(x) sleep(x)
#include <signal.h>
void ConsoleHandler(int sig) {
	switch (sig) {
	case SIGINT: case SIGTERM: { ServerPool::Trigger(); break; }
	default: break;
	}
}
#endif

// ──────────────────────────────────────────────────────────────────────────
// Thread::LoopingThread  (ported from src (1).zip – no deadlock design)
// ──────────────────────────────────────────────────────────────────────────
class Thread {
public:
	class LoopingThread {
	public:
		struct Delays {
			long long MiningWorld  = 0;
			long long ShutdownPipe = 0;
		};
		static inline Delays delays{};
		static inline HANDLE hPipe = NULL;

		static bool InitPipe(const std::string& serverName) {
			std::string pipeName = "\\\\.\\pipe\\shutdown_" + serverName;
			hPipe = CreateNamedPipeA(
				pipeName.c_str(),
				PIPE_ACCESS_INBOUND,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
				1, 1024, 1024, 0, NULL
			);
			if (hPipe == INVALID_HANDLE_VALUE) {
				Logger::Info("ERROR", "Failed to create named pipe");
				return false;
			}
			// ConnectNamedPipe dengan PIPE_NOWAIT akan return false jika belum ada
			// client yang connect — ini normal untuk non-blocking pipe, bukan error.
			BOOL connected = ConnectNamedPipe(hPipe, NULL);
			if (!connected) {
				DWORD err = GetLastError();
				// ERROR_PIPE_CONNECTED = client sudah connect sebelum ConnectNamedPipe
				// ERROR_NO_DATA = pipe masih menunggu client (normal untuk NOWAIT)
				// ERROR_PIPE_LISTENING = pipe dalam mode listening (normal untuk NOWAIT)
				if (err != ERROR_PIPE_CONNECTED && err != ERROR_NO_DATA && err != ERROR_PIPE_LISTENING) {
					Logger::Info("ERROR", "ConnectNamedPipe failed, code: " + std::to_string(err));
				}
			}
			return true;
		}

		static void ShutdownPipe() {
			if (hPipe == NULL || hPipe == INVALID_HANDLE_VALUE) return;
			DWORD bytesAvailable = 0;
			if (PeekNamedPipe(hPipe, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0) {
				char buf[256]; DWORD bytesRead = 0;
				if (ReadFile(hPipe, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
					buf[bytesRead] = '\0';
					if (strcmp(buf, "shutdown") == 0) {
						Environment()->GracefullShutdown = true;
						Logger::Info("INFO", "Received shutdown request via named pipe");
						ServerPool::Trigger();
					}
				}
			}
		}

		static void Cleanup() {
			if (hPipe != NULL && hPipe != INVALID_HANDLE_VALUE) {
				CloseHandle(hPipe);
				hPipe = NULL;
			}
		}

		static void Trigger() {
			while (g_server_running.load()) {
				long long ms_time = date_time::get_epoch_ms();
				if (delays.MiningWorld - ms_time <= 0) {
					delays.MiningWorld = ms_time + 500;
					// MiningWorld reserve tick (extend here if needed)
				}
				if (delays.ShutdownPipe - ms_time <= 0) {
					delays.ShutdownPipe = ms_time + 1000;
					ShutdownPipe();
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			Cleanup();
		}
	};
};

class Looping {
public:
	static void Events(ENetPeer* peer, bool forced = false) {
		std::string name_ = pInfo(peer)->world;
		World* world_ = find_world(name_);
		if (world_ != nullptr) {
			long long now_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
			if (world_->special_event == false && server_event_spawn + 300000 < now_ms) {
				int event_item = 0;
				if (rand() % 200 < 1 && world_->last_special_event + 900000 < now_ms or forced) {
					std::vector<int> list = { 263,942,942,1396,4774,4522, 4774, 5002, 121, 1636, 2798, 2704, 3786, 4354 , 4354, 2992, 2992, 10016, 10016, 1396 };
					world_->special_event_item = (WinterFest.Active ? 9186 : list[rand() % list.size()]);
					event_item = world_->special_event_item;
					world_->special_event = true;
					for (ENetPeer* cp__event : world_->joinedPlayers) {
						if (cp__event->state != ENET_PEER_STATE_CONNECTED or cp__event->data == NULL) continue;
						VarList::OnConsoleMessage(cp__event, "`2" + items[world_->special_event_item].event_name + ":`` " + (items[world_->special_event_item].event_total == 1 ? "`oYou have`` `030`` `oseconds to find and grab the`` `#" + items[world_->special_event_item].name + "```o.``" : "`#" + to_string(items[world_->special_event_item].event_total) + " " + items[world_->special_event_item].name + "`` `ospawn in your world, you have`` `030`` `oseconds to collect them.``") + "");
						VarList::OnAddNotification(cp__event, "`2" + items[world_->special_event_item].event_name + ":`` " + (items[world_->special_event_item].event_total == 1 ? "`oYou have`` `030`` `oseconds to find and grab the`` `#" + items[world_->special_event_item].name + "```o.``" : "`#" + to_string(items[world_->special_event_item].event_total) + " " + items[world_->special_event_item].name + "`` `ospawn in your world, you have`` `030`` `oseconds to collect them.``") + "", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
					}
					world_->last_special_event = now_ms;
				}
					else if (Event()->Zombie_Apocalypse && rand() % 3 < 1 && world_->last_special_event + 180000 < now_ms) {
					for (ENetPeer* cp_ : world_->joinedPlayers) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						VarList::OnSetCurrentWeather(cp_, 31);
						VarList::OnConsoleMessage(cp_, "`2Zombie Apocalypse:`` `oIt's the Return of the Growing Dead!``");
						VarList::OnAddNotification(cp_, "`2Zombie Apocalypse:`` `oIt's the Return of the Growing Dead!``", "interface/large/special_event.rttex", "audio/owooooo.wav");
						if (not Playmods::HasById(pInfo(cp_), 28)) {
							if (rand() % 2 < 1) {
								Playmods::Add(cp_, 28);
								Clothing_V2::Update(cp_, true);
								Clothing_V2::Update_Value(cp_);
							}
						}
					}
					world_->last_special_event = now_ms;
				}
				else if (Event()->Comet_Dust && rand() % 3 < 1 && world_->last_comet_spawn + 180000 < now_ms) {
					event_item = 2034;
					world_->last_comet_spawn = now_ms;
				}
					else if (WinterFest.Active and WinterFest.Goblin_Spawn + 500000 < now_ms) {
					int sentto = 0;
					for (ENetPeer* cp_ : world_->joinedPlayers) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or not pInfo(cp_)->Silver_Donatur or pInfo(cp_)->Already_Tried_Gob) continue;
						if (sentto > 5) continue;
						sentto++;
						VarList::OnDialogRequest(cp_, "set_default_color|`o\nset_bg_color|43,34,74,200|\nset_border_color|112,86,191,255|\n\nadd_label_with_icon|big|`9Stuff-4-Toys Volunteer Drive|left|4264|\nadd_spacer|small|\nadd_textbox|`9Stuff-4-Toys`` is looking for volunteers! Are you willing to hand out some toys right now, dressed in our `2Present Goblin Disguise``?|left|\nadd_smalltext|- It will only take about 30 seconds at your time!|left|\nadd_smalltext|- You won't be able to pick up items while in disguise.|left|\nadd_smalltext|- You will be teleported to the white door, and your Checkpoint will be reset.|left|\nadd_smalltext|- You'll be bringin joy to " + Environment()->server_name + "!|left|\nadd_smalltext|- `4You only have 30 seconds to respond this message!|left|\nend_dialog|Goblin_Disguise|I hate joy|I am happy to help!|");
					}
					WinterFest.Goblin_Spawn = now_ms;
				}
				if (event_item != 0) {
					if (find(Environment()->Another_Worlds.begin(), Environment()->Another_Worlds.end(), world_->name) == Environment()->Another_Worlds.end()) Environment()->Another_Worlds.emplace_back(world_->name);
					server_event_spawn = now_ms;
					WorldDrop drop_block_{};
					drop_block_.count = 1;
					for (int i_ = 0; i_ < items[event_item].event_total; i_++) {
						drop_block_.id = event_item == 2034 ? (rand() % 100 < 25 ? 2036 : 2034) : world_->special_event_item, drop_block_.x = rand() % 99 * 32, drop_block_.y = rand() % 54 * 32;
						world_->world_event_items.emplace_back(drop_block_.id);
						VisualHandle::Drop(world_, drop_block_, true);
					}
				}
			}
		}
	}
	static void Worlds() {
		if (Environment()->Auto_Saving == false) {
			long long ms_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
			if (Environment()->Last_Time2_ - ms_time <= 0 && Environment()->Restart_Status) {
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					VarList::OnConsoleMessage(cp_, "`4Global System Message``: Restarting server for update in `4" + to_string(Environment()->Restart_Time) + "`` minutes");
					CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/ogg/suspended.ogg", 700);
				}
				Environment()->Restart_Time -= 1;
				if (Environment()->Restart_Time == 0) {
					Environment()->Last_Time2_ = ms_time + 10000, Environment()->Restart_Status_Seconds = true, Environment()->Restart_Status = false;
					Environment()->Restart_Time = 50;
				}
				else Environment()->Last_Time2_ = ms_time + 60000;
			}
			if (Environment()->Restart_Status_Seconds && Environment()->Last_Time2_ - ms_time <= 0) {
				bool save_ = false, send_now = false;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					VarList::OnConsoleMessage(cp_, "`4Global System Message``: Restarting server for update in `4" + (Environment()->Restart_Time > 0 ? to_string(Environment()->Restart_Time) : "ZERO") + "`` seconds" + (Environment()->Restart_Time > 0 ? "" : "! Should be back up in a minute or so. BYE!") + "");
					send_now = true;
				}
				if (Environment()->Restart_Time > 0) save_ = false;
				else save_ = true;
				Environment()->Last_Time2_ = ms_time + 10000;
				if (save_ && send_now) {
					Environment()->Restart_Status_Seconds = false;
					Environment()->Maintenance = true;
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						Peer_Disconnect(cp_, 0);
					}
				}
				else Environment()->Restart_Time -= 10;
			}
			if (Environment()->Last_Time - ms_time <= 0) {
				if (Environment()->Last_Checkip - ms_time <= 0) {
					Environment()->Last_Checkip = ms_time + 900000;
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world.empty()) continue;
						char ClientConnection[16];
						enet_address_get_host_ip(&cp_->address, ClientConnection, 16);
						std::string str(ClientConnection);
						if (pInfo(cp_)->ip != str) pInfo(cp_)->ip = str, pInfo(cp_)->fa_ip = str;
					}
				}
				bool gems_ended = (Event()->x_gems_time - time(nullptr) <= 0 && Event()->Gems_Event);
				bool xp_ended = (Event()->x_xp_time - time(nullptr) <= 0 && Event()->Xp_Event);
				bool exchange_ended = (Event()->x_exchange_time - time(nullptr) <= 0 && Event()->Exchange_Event);
				if (gems_ended)     Event()->Gems_Event = false;
				if (xp_ended)       Event()->Xp_Event = false;
				if (exchange_ended) Event()->Exchange_Event = false;
				Environment()->Active_Worlds.clear();
				Environment()->Active_Worlds.reserve(32); // FIX LAG: avoid rehash on insert
				static int reset_bucket = 0;
				reset_bucket = (reset_bucket + 1) % 5;
				int peer_idx = 0;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (!special_char(pInfo(cp_)->world) and pInfo(cp_)->world != "" and pInfo(cp_)->world != "EXIT" and pInfo(cp_)->world.find("GROWMINES_") == string::npos) {
						Environment()->Active_Worlds[pInfo(cp_)->world]++;
					}
					if (pInfo(cp_)->world != "" && pInfo(cp_)->in_enter_game && (peer_idx % 5) == reset_bucket)
						PlayerCharacter::Reset(cp_);
					peer_idx++;
					if (gems_ended) {
						VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[SB]_ `o**`` `4Global System Message:`` `$Gems event has ended``");
						VarList::OnAddNotification(cp_, "`9Gems Event has ended`w!", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
					}
					if (xp_ended) {
						VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[SB]_ `o**`` `4Global System Message:`` `$Xp event has ended``");
						VarList::OnAddNotification(cp_, "`9Xp Event has ended`w!", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
					}
					if (exchange_ended) {
						VarList::OnConsoleMessage(cp_, "CP:_PL:0_OID:_CT:[SB]_ `o**`` `4Global System Message:`` `$Exchange event has ended``");
						VarList::OnAddNotification(cp_, "`9Xp Event has ended`w!", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
					}
				}
				if (Honors_Update.last_honors_reset - ms_time <= 0) {
					std::vector<int> added;
					Environment()->janeway_item.clear();
					Environment()->janeway_payout = Environment()->random_janeway_payout[rand() % Environment()->random_janeway_payout.size()];
					for (int i = 0; i < 5; i++) {
						int random_item = rand() % Environment()->janeway_items.size();
						if (find(added.begin(), added.end(), Environment()->janeway_items[random_item].first) == added.end()) {
							added.push_back(Environment()->janeway_items[random_item].first);
							Environment()->janeway_item.push_back(std::make_pair(Environment()->janeway_items[random_item].first, Environment()->janeway_items[random_item].second));
							Environment()->janeway_item.push_back(std::make_pair(Environment()->janeway_items[random_item].first + 1, Environment()->janeway_items[random_item].second * 4));
						}
					}
					time_t currentTime;
					time(&currentTime);
					const auto localTime = localtime(&currentTime);
					const auto Hour = localTime->tm_hour; const auto Min = localTime->tm_min; const auto Sec = localTime->tm_sec; const auto Year = localTime->tm_year + 1900; const auto Day = localTime->tm_mday; const auto Month = localTime->tm_mon + 1;
					if (Hour >= 6 and Hour < 15) DaylightDragon.param1 = 0, DaylightDragon.param2 = 0, DaylightDragon.param3 = 1, DaylightDragon.param4 = 5, DaylightDragon.param5 = 0, DaylightDragon.param6 = 2;
					if (Hour >= 15 and Hour < 18) DaylightDragon.param1 = 1, DaylightDragon.param2 = 0, DaylightDragon.param3 = 1, DaylightDragon.param4 = 5, DaylightDragon.param5 = 0, DaylightDragon.param6 = 0;
					if (Hour >= 18 and Hour <= 0 or Hour > 0 and Hour < 6) DaylightDragon.param1 = 2, DaylightDragon.param2 = 0, DaylightDragon.param3 = 1, DaylightDragon.param4 = 5, DaylightDragon.param5 = 0, DaylightDragon.param6 = 1;
					ServerPool::Honors::Reset();
					Honors_Update.last_honors_reset = ms_time + 7200000;
				}
				struct tm newtime;
				time_t now = time(0);
#ifdef _WIN32
				localtime_s(&newtime, &now);
#elif defined(__linux__)
				localtime_r(&now, &newtime);
#endif
				if (today_day != newtime.tm_mday) {
					today_day = newtime.tm_mday;
					std::vector<int8_t> random_xy{ -1, 0 };
					Environment()->Dq_Item1 = Environment()->small_seeds[rand() % Environment()->small_seeds.size()] + random_xy[rand() % random_xy.size()];
					Environment()->Dq_Item2 = Environment()->small_seed_pack[rand() % Environment()->small_seed_pack.size()] + random_xy[rand() % random_xy.size()];
					Environment()->Dq_Count1 = rand() % 200 + 1;
					Environment()->Dq_Count2 = rand() % 100 + 1;
					Environment()->Dq_Day = today_day;
					int Random = rand() % 43 + 1;
					Environment()->Ancient_Riddle.clear();
					static const std::pair<int, const char*> riddles[] = {
						{982, "I see you wish to acquire greatness. I'd never block your efforts."},
						{598, "This is a part of a fantasy, but I wouldn't want to be around it when it comes to a close."},
						{320, "1812 is over, sure, but there's a way to bring it back. I can go on, but you should know how by now."},
						{756, "Bring me a bandit. I don't even need all of him."},
						{824, "I could never tell you what I want, because I'm not a fan of spoilers, and I'd prefer to keep things cool."},
						{456, "My paradise is lost, but perhaps you can bring me halfway there."},
						{480, "If you're a record breaker, stay away from this."},
						{784, "I'd like a huckleberry. Know where I can find one?"},
						{658, "If you shoot this, would it be redundant? At the very least, I'll know you're bored."},
						{1430, "Everyone asks the gods for these. Just once, I'd like to get one."},
						{114, "You can silence this, but it will never die."},
						{596, "A store full of wonders, none of which are for sale, is exactly the mark I'd like to spot."},
						{286, "I'd like to remove some underlings from my employ. Give me a place to put them."},
						{1048, "Spark my memories of the open range."},
						{992, "I'd like a bright idea, but remember that I'm a city girl at heart."},
						{1046, "I'm undecided, but I think the cowboy life calls to me on this one."},
						{220, "Block out a little time to tune this the right way and that's what you'll have."},
						{64, "If you ever had to use the bathroom in a dungeon, theres a chance you will put this together."},
						{666, "Don't make an assumption, presume, or suppose - just take this for what it is and igneore the distractions."},
						{684, "There aren't any drinks to be found here, but if they were, they'd be good for your blood."},
						{872, "A creature of roads and rubber. It is dangerous to go alone, but even moreso, where beasts such as this are concerned."},
						{866, "Time to get low. Perhaps this choice will cheese you off, but I suggest you move along."},
						{1044, "It's worth more than a nickel, has nothing to do with chickens, and up to eight of them are correct, which I find baffling."},
						{786, "Sorry, I can't quite racal what I wanted for this one. Call it a sign of the times."},
						{1420, "Don't blink."},
						{970, "Ag! You woke me up. I dreamt of when I was a smaller, simpler goddess. Perhaps you could remind me of that time?"},
						{260, "Find me a shimmering thing of wonder - I don't care what, so long as it's ausome!"},
						{186, "Bessie's a good name for a cow. Put it together with the sea France, and you have the first step in getting me what I want..."},
						{780, "2623, you'll know what to do. But there are alternates."},
						{298, "Actually, I can't think of anything. Therefore, the opposite must do."},
						{926, "Steel yourself, for I grow weary (or is it hungry?) and must set this discussion aside."},
						{688, "These are spooky. I know everybody has one, but getting it to me is the key."},
						{1002, "Hercules never defeated one of these, though, flames are still involved."},
						{1530, "Whenever I sea these, I remember to keep quiet."},
						{194, "Get me my favorite topping, and remember - I'm a fun girl."},
						{334, "What's your favorite color? I can't decide between pink and yellow, so you'll have to bring me both."},
						{1896, "I have trouble remembering things, so get me something that never forgets."},
						{436, "Get me something to paint-and don't bangle it up."},
						{454, "I want something strange, deadly, and otherworldly - but nothing from Mercury, please."},
						{988, "Take one down from its perch and bring it here - just make sure you don't sluice it!"},
						{1312, "What do Vikings, Lumberjacks and Shrubbery have in common?"},
						{382, "I hate to break it to you, but it's time to give me some space."},
						{922, "Most of the things I want are a mystery, but this one is especially so."},
						{664, "I had a plan to get something tasty, but it's full of holes.."}
					};
					if (Random >= 1 && Random <= 44) {
						Environment()->Ancient_Riddle.emplace_back(riddles[Random - 1].first, riddles[Random - 1].second);
					}
					json j;
					j["Dq_Item1"] = Environment()->Dq_Item1;
					j["Dq_Item2"] = Environment()->Dq_Item2;
					j["Dq_Count1"] = Environment()->Dq_Count1;
					j["Dq_Count2"] = Environment()->Dq_Count2;
					j["Dq_Day"] = Environment()->Dq_Day;
					for (const auto& riddle : Environment()->Ancient_Riddle) {
						j["Ancient_Riddle"].push_back({ riddle.first, riddle.second });
					}
					std::string dump_str = j.dump(4);
					std::thread([dump_str]() {
						std::ofstream o("database/json/daily_reset.json");
						if (o.is_open()) { o << dump_str << std::endl; }
						}).detach();

					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						pInfo(cp_)->dd = 0, pInfo(cp_)->dq_day = newtime.tm_mday;
						CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/cash_register.wav", 0);
						VarList::OnConsoleMessage(cp_, "`2*** The Daily Quest Update``: requires `$" + to_string(Environment()->Dq_Count1) + " " + items[Environment()->Dq_Item1].name + "`` and `$" + to_string(Environment()->Dq_Count2) + " " + items[Environment()->Dq_Item2].name + "``. Go tell the `5Crazy Jim``!");
					}
				}
				if (Event()->daily_current_time - time(nullptr) <= 0 and Event()->DailyChallenge == true) {
					EventPool::DailyChallenge::Wait();
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						VarList::OnConsoleMessage(cp_, "`2Daily Challenge: `oTime is Up! Figuring out the winners....");
						if (Event()->top_dailyc.size() != 0) VarList::OnConsoleMessage(cp_, "`9Today's winners:", 2000);
						std::vector<std::pair<long long int, std::string>> top_tiers = Event()->top_dailyc;
						sort(top_tiers.begin(), top_tiers.end());
						reverse(top_tiers.begin(), top_tiers.end());
						top_tiers.resize((top_tiers.size() >= 10 ? 10 : top_tiers.size()));
						for (std::uint8_t i = 0; i < top_tiers.size(); i++) {
							if (i < 5) VarList::OnConsoleMessage(cp_, "`w#" + to_string(i + 1) + ". `1" + top_tiers[i].second + " ``with " + Set_Count(top_tiers[i].first) + " Points", 2000);
						}
						if (pInfo(cp_)->world != "") Daily_Challenge::DailyChallengeRequest(cp_);
						VarList::OnConsoleMessage(cp_, "`9Join us in " + Time::Playmod(Event()->daily_wait_time - time(nullptr)) + " for another shot at the prize!", 2500);
					}
				}
				if (Event()->daily_wait_time - time(nullptr) <= 0 and not Event()->DailyChallenge) {
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						if (pInfo(cp_)->world != "") Daily_Challenge::DailyChallengeRequest(cp_);
					}
					EventPool::DailyChallenge::Start();
				}
				if (current_event - time(nullptr) <= 0 and not Event()->Can_Event) EventPool::Guilds::Reset();
				if (wait_event - time(nullptr) <= 0 and Event()->Can_Event and not Event()->Wait_Next) EventPool::Guilds::Next();
				if (next_event - time(nullptr) <= 0 and Event()->Can_Event and Event()->Wait_Next) EventPool::Guilds::Start();
				if (Environment()->Worlds_Refresh - ms_time <= 0) {
					Environment()->Worlds_Refresh = ms_time + 15000;

					// ─── WORLD MENU: warna-warni (rainbow HSL) per world ───
					// Tiap world dapat warna acak unik tiap refresh (15 detik),
					// dan world yang baru saja aktif tetap nampil sejenak walau
					// playernya sudah kosong (sticky list, max 40 entries).
					static std::mt19937 world_color_rng(
						static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
					std::uniform_int_distribution<int> dist(3, 4);
					std::uniform_real_distribution<float> h(0.0f, 360.0f);

					// Step 1: sort world aktif by player count (desc).
					// Environment()->Active_Worlds sudah di-rebuild di awal tick ini
					// (loop peer di atas), jadi tidak perlu loop ulang.
					std::vector<std::pair<std::string, int>> sorted_worlds(
						Environment()->Active_Worlds.begin(), Environment()->Active_Worlds.end());
					std::sort(sorted_worlds.begin(), sorted_worlds.end(), [](auto& a, auto& b) {
						return a.second > b.second;
						});

					// Step 2: build merged list dari world aktif
					std::vector<std::string> merged;
					merged.reserve(sorted_worlds.size());
					for (auto& [wname, wcount] : sorted_worlds)
						merged.push_back(wname);

					// Step 3: isi slot sisa (sampai 40) dari list sebelumnya, biar
					// world yang baru kosong tidak langsung hilang dari menu
					for (auto& prev : Environment()->Previous_Active_Worlds) {
						if (merged.size() >= 40) break;
						if (std::find(merged.begin(), merged.end(), prev) == merged.end())
							merged.push_back(prev);
					}

					// Step 4: simpan list ini buat refresh selanjutnya
					Environment()->Previous_Active_Worlds = merged;

					// Step 5: generate Active_World_List dengan warna pelangi acak
					Environment()->Active_World_List.clear();
					for (auto& name : merged) {
						auto it = Environment()->Active_Worlds.find(name);
						int count = (it != Environment()->Active_Worlds.end()) ? it->second : 0;

						float H = h(world_color_rng), S = 0.9f, L = 0.6f;
						float C = (1.0f - std::fabs(2.0f * L - 1.0f)) * S;
						float X = C * (1.0f - std::fabs(std::fmod(H / 60.0f, 2.0f) - 1.0f));
						float m = L - C / 2.0f;
						float R, G, B;
						if (H < 60)       R = C, G = X, B = 0;
						else if (H < 120) R = X, G = C, B = 0;
						else if (H < 180) R = 0, G = C, B = X;
						else if (H < 240) R = 0, G = X, B = C;
						else if (H < 300) R = X, G = 0, B = C;
						else              R = C, G = 0, B = X;

						uint32_t color = (static_cast<uint32_t>((B + m) * 255) << 24)
							| (static_cast<uint32_t>((G + m) * 255) << 16)
							| (static_cast<uint32_t>((R + m) * 255) << 8)
							| 255;

						Environment()->Active_World_List += "\nadd_floater|" + name + "|" + to_string(count)
							+ "|0." + to_string(dist(world_color_rng)) + "|" + to_string(color);
					}

					if (Environment()->Active_World_List.empty())
						Environment()->Active_World_List = "\nadd_floater|START|0|0.5|3529161471";

					Environment()->Active_Worlds_Count = (int)merged.size();
					Environment()->Active_Worlds.clear();
				}
			}
		if (Environment()->Last_Math - ms_time <= 0 and not Event()->DailyMaths) {
			if (Environment()->Last_Math - ms_time <= 0) Environment()->Last_Math = ms_time + 300000;
			static thread_local std::mt19937 rng(std::random_device{}());
			std::uniform_int_distribution<int> dist1(1, 992), dist2(1, 892);
			int number1 = dist1(rng), number2 = dist2(rng);
			std::uniform_int_distribution<int> dist_num(0, number1 - 1);
			Event()->Math_Num1 = dist_num(rng) + number2;
			Event()->Math_Num2 = dist_num(rng) + number2 + 22;
			Event()->Math_Result = Event()->Math_Num1 + Event()->Math_Num2;
			std::uniform_int_distribution<int> dist_prize1(1, 42220), dist_prize2(1, 65300);
			Event()->Math_Prize = dist_prize1(rng) + dist_prize2(rng);
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL || pInfo(cp_)->world.empty()) continue;
					CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/startopia_tool_droid.wav", 0);
					VarList::OnConsoleMessage(cp_, "`9** " + Environment()->server_name + " Daily Math [Questions : `3'" + to_string(Event()->Math_Num1) + " + " + to_string(Event()->Math_Num2) + "'`9 = ?] Prize: `2" + to_string(Event()->Math_Prize) + "`9 (gems) ! `o(/a <answer>).");
					Event()->DailyMaths = true;
				}
			}
			if (Environment()->Last_Update - ms_time <= 0) {
				// Adaptive update interval: slow down when server is busy
				int total_peers_upd = server ? (int)server->peerCount : 0;
				int upd_interval = (total_peers_upd >= 50) ? 4000 : (total_peers_upd >= 30) ? 3000 : 2500;
				Environment()->Last_Update = ms_time + upd_interval;

				// Build quick per-world player count map to skip heavy ops in crowded worlds
				// FIX LAG: reserve() → avoid rehashing when inserting many entries
				std::unordered_map<std::string, int> world_player_count;
				world_player_count.reserve(64);
				for (ENetPeer* cpc = server->peers; cpc < &server->peers[server->peerCount]; ++cpc) {
					if (cpc->state != ENET_PEER_STATE_CONNECTED || !cpc->data || pInfo(cpc)->world.empty()) continue;
					world_player_count[pInfo(cpc)->world]++;
				}

				std::unordered_set<World*> worlds_processed_this_tick; // dedup: skip repeated block-scans for same world
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL or pInfo(cp_)->tankIDName.empty() or pInfo(cp_)->world.empty()) continue;
					std::string name_ = pInfo(cp_)->world;

					// FIX LAG (crowded world): skip expensive per-player world processing
					// for players in overly crowded worlds. Threshold 20+ players:
					// block-scan tetap jalan (world-level, dedup), tapi personal updates di-throttle.
					int wpc = world_player_count.count(name_) ? world_player_count[name_] : 0;
					bool is_crowded = (wpc >= 20);

					World* world = find_world(name_);
					if (world != nullptr) {
						if (world->name == "GROWCH" and WinterFest.Active) {
							if (WinterFest.Growch_Happy_Time - time(nullptr) <= 0 and WinterFest.GiftedGrowch != 0 and WinterFest.Growch_Happy) {
								world->weather = 12;
								WinterFest.Growch_Happy = false;
								WinterFest.Growch_Happy_Time = time(nullptr) + (300 * 60);
								for (ENetPeer* cp_2 = server->peers; cp_2 < &server->peers[server->peerCount]; ++cp_2) {
									if (cp_2->state != ENET_PEER_STATE_CONNECTED or cp_2->data == NULL) continue;
									VarList::OnConsoleMessage(cp_2, "`oThe Growch is angry again, making he happy again!");
									if (pInfo(cp_2)->world == "GROWCH") {
										VarList::OnSetCurrentWeather(cp_2, world->weather);
									}
								}
							}
						}
						world->fresh_world = true;

						if (!world || world->blocks.empty()) continue;

						// Per-world dedup: block-scan logic runs ONCE per world per tick
						if (worlds_processed_this_tick.insert(world).second) {
							// --- Epoch Clock (fg==5958) via cached index ---
							if (world->cached_epoch_idx == -2) {
								auto pc = std::find_if(world->blocks.begin(), world->blocks.end(), [](const WorldBlock& a) { return a.fg == 5958; });
								world->cached_epoch_idx = (pc != world->blocks.end()) ? (int)std::distance(world->blocks.begin(), pc) : -1;
							}
							if (world->cached_epoch_idx >= 0 && world->cached_epoch_idx < (int)world->blocks.size()) {
								WorldBlock& block = world->blocks[world->cached_epoch_idx];
								if ((block.flags & 0x00400000) != 0) {
									const long long current_time = time(nullptr);
							if (block.epoch_cycle - current_time <= 1) {
									if (world->weather == 38) world->weather = block.epoch_state.first[1] ? 39 : (block.epoch_state.first[2] ? 40 : 38);
									else if (world->weather == 39) world->weather = block.epoch_state.first[2] ? 40 : (block.epoch_state.first[0] ? 38 : 39);
									else if (world->weather == 40) world->weather = block.epoch_state.first[0] ? 38 : (block.epoch_state.first[1] ? 39 : 40);
									block.epoch_cycle = current_time + (block.epoch_state.second * 60);
									for (ENetPeer* cp_2 : world->joinedPlayers) {
										if (cp_2->state != ENET_PEER_STATE_CONNECTED || cp_2->data == NULL) continue;
										VarList::OnSetCurrentWeather(cp_2, (Event()->Comet_Dust ? 16 : world->weather == 0 ? 80 : world->weather));
									}
								}
								}
							}
							// --- Infinity Machine (fg==10058) via cached index ---
							if (world->cached_infinity_idx == -2) {
								auto pi = std::find_if(world->blocks.begin(), world->blocks.end(), [](const WorldBlock& a) { return a.fg == 10058; });
								world->cached_infinity_idx = (pi != world->blocks.end()) ? (int)std::distance(world->blocks.begin(), pi) : -1;
							}
							if (world->cached_infinity_idx >= 0 && world->cached_infinity_idx < (int)world->blocks.size()) {
								if (world->blocks[world->cached_infinity_idx].flags & 0x00400000 && world->infinity.size() != 0) {
									long long current_time = time(nullptr);
									if (world->blocks[world->cached_infinity_idx].epoch_cycle - current_time <= 1) {
										if (world->cycle_ke >= (int)world->infinity.size() || world->cycle_ke < 0) world->cycle_ke = 0;
									world->weather = items[world->infinity[world->cycle_ke]].base_weather;
									world->cycle_ke++;
									world->blocks[world->cached_infinity_idx].epoch_cycle = current_time + (world->blocks[world->cached_infinity_idx].infinity_cycle * 60);
									for (ENetPeer* cp_2 : world->joinedPlayers) {
										if (cp_2->state != ENET_PEER_STATE_CONNECTED || cp_2->data == NULL) continue;
										VarList::OnSetCurrentWeather(cp_2, (Event()->Comet_Dust ? 16 : world->weather == 0 ? 80 : world->weather));
									}
									}
								}
							}
							// --- Growch Tree (fg==3202) via cached index ---
							if (world->cached_growch_tree_idx == -2) {
								auto pg = std::find_if(world->blocks.begin(), world->blocks.end(), [](const WorldBlock& a) { return a.fg == 3202; });
								world->cached_growch_tree_idx = (pg != world->blocks.end()) ? (int)std::distance(world->blocks.begin(), pg) : -1;
							}
							if (world->cached_growch_tree_idx >= 0 && world->cached_growch_tree_idx < (int)world->blocks.size()) {
								int x_ = world->cached_growch_tree_idx % 100, y_ = world->cached_growch_tree_idx / 100;
								if (world->name == "GROWCH" && WinterFest.Active && world->blocks[world->cached_growch_tree_idx].fg == 3202 && world->blocks[world->cached_growch_tree_idx].planted - time(nullptr) <= 0) {
									world->blocks[world->cached_growch_tree_idx].fg = 3200;
									world->cached_growch_tree_idx = -2;
									for (ENetPeer* cp_2 = server->peers; cp_2 < &server->peers[server->peerCount]; ++cp_2) {
										if (cp_2->state != ENET_PEER_STATE_CONNECTED || cp_2->data == NULL) continue;
										if (world->name == "GROWCH") { update_tile(cp_2, x_, y_, 3200, true); tile_update(cp_2, world, &world->blocks[x_ + (y_ * 100)], x_, y_); }
									}
								}
							}
						} // end worlds_processed_this_tick
					}
				}
			}
			if (Environment()->Last_Growganoth - ms_time <= 0 and Event()->Halloween) {
				std::string name_ = "GROWGANOTH";
				World* world_ = find_world(name_);
				if (world_ != nullptr) {
					Environment()->Last_Growganoth = ms_time + 20000;
					for (int i_ = 0; i_ < Environment()->growganoth_platform.size(); i_++) {
						int pos = Environment()->growganoth_platform[i_];
						if (pos < 0 || pos >= static_cast<int>(world_->blocks.size()))
							continue;
						int x_ = pos % 100;
						int y_ = pos / 100;
						world_->blocks[pos].fg = 0;
						PlayerMoving data{ 0, 0, 3, 0, 0, x_, y_, 0, (float)x_, (float)y_, 0, 0 };
						BYTE* p_ = packPlayerMoving(&data);
						for (ENetPeer* cp_ : world_->joinedPlayers) {
							if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
							send_raw(cp_, 4, p_, 56, ENET_PACKET_FLAG_RELIABLE);
						}
						delete[] p_;
					}
					Environment()->growganoth_platform.clear();
					for (int i_ = 0; i_ < 50; i_++) {
						int x_ = rand() % 89 + 1;
						int y_ = rand() % 52 + 1;
						int idx = x_ + (y_ * 100);
						if (idx >= 0 && idx < static_cast<int>(world_->blocks.size())) {
							if (world_->blocks[idx].fg == 0) {
								Environment()->growganoth_platform.push_back(idx);
								world_->blocks[idx].fg = (i_ < 40 ? 1222 : 1224);

								PlayerMoving data{ 0, 0, 3, 0, world_->blocks[idx].fg, x_, y_, 0, (float)x_, (float)y_, 0, 0 };
								BYTE* p_ = packPlayerMoving(&data);
								for (ENetPeer* cp_ : world_->joinedPlayers) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
									send_raw(cp_, 4, p_, 56, ENET_PACKET_FLAG_RELIABLE);
								}
								delete[] p_;
							}
						}
						int specialIdx = 25 + (30 * 100);
						if (specialIdx >= 0 && specialIdx < static_cast<int>(world_->blocks.size())) {
							if (world_->blocks[specialIdx].fg == 0) {
								world_->blocks[specialIdx].fg = (i_ > 40 ? 1226 : 1228);
								Environment()->growganoth_platform.push_back(specialIdx);

								PlayerMoving data{ 0, 0, 3, 0, world_->blocks[specialIdx].fg, 25, 30, 0, 25.0f, 30.0f, 0, 0 };
								BYTE* p_ = packPlayerMoving(&data);
								for (ENetPeer* cp_ : world_->joinedPlayers) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
									send_raw(cp_, 4, p_, 56, ENET_PACKET_FLAG_RELIABLE);
								}
								delete[] p_;
							}
						}
						if (world_->blocks[25 + (30 * 100)].fg == 0) {
							world_->blocks[25 + (30 * 100)].fg = (i_ > 40 ? 1226 : 1228);
							Environment()->growganoth_platform.push_back(25 + (30 * 100));
							PlayerMoving data{ 0, 0, 3, 0, world_->blocks[25 + (30 * 100)].fg, 25, 30, 0, (float)25, (float)30, 0, 0 };
							BYTE* p_ = packPlayerMoving(&data);
							for (ENetPeer* cp_ : world_->joinedPlayers) {
								if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
								send_raw(cp_, 4, p_, 56, ENET_PACKET_FLAG_RELIABLE);
							}
							delete[] p_;
						}
					}
				}
			}
			// FIX LAG #2: Throttle carnival section ke 1000ms (was: every 200ms tick = 5x/s!)
		// FIX LAG #3: Early-exit jika tidak ada player di CARNIVAL → skip O(n_peers) scan
		// FIX LAG #1: last_carnig_time sekarang long long (lihat BaseServer.h), tidak overflow lagi
			if (Environment()->Last_Carnival - ms_time <= 0) {
				Environment()->Last_Carnival = ms_time + 1000;

				vector<string> gamesbusy = {};
				bool cancum = false;
				const string name_ = "CARNIVAL";

				// Early-exit: check CARNIVAL world has players via joinedPlayers
				bool has_carnival_player = false;
				{
					World* carnival_check = find_world(name_);
					if (carnival_check != nullptr && !carnival_check->joinedPlayers.empty()) {
						has_carnival_player = true;
						for (ENetPeer* currentPeer : carnival_check->joinedPlayers) {
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
							if (pInfo(currentPeer)->carnival_playing != "")
								gamesbusy.push_back(pInfo(currentPeer)->carnival_playing);
						}
					}
				}

				World* world_ = (has_carnival_player ? find_world(name_) : nullptr);
				if (world_ != nullptr) {
					if (world_->name == name_) {
						int ySize = (int)world_->blocks.size() / 100, xSize = (int)world_->blocks.size() / ySize;
						int x = 0, y = 0;

						vector<int> visuals;
						vector<int> sltochange;
						vector<int> rltochange;
						vector<int> rllist;
						vector<int> sllist;
						vector<int> grpllist, sgrpllist, sevillist, seviladd, splatformadd, sgrtochange;
						vector<int> grtochange;
						vector<int> platformadd;
						vector<int> eviladd;
						vector<int> evillist;
						string Savedtext = "Creep, Right = ";
						string Creep = "", Eye = "", Reye = "", Rcreep = "";

						if (find(gamesbusy.begin(), gamesbusy.end(), "Growganoth_Right") != gamesbusy.end() && Environment()->last_carnig_time - ms_time <= 10000) {
							int donen = 0;
							for (int i_ = 0; i_ < world_->blocks.size(); i_++) {
								if (world_->blocks[i_].fg == 1222) {
									world_->blocks[i_].fg = 1224;
								}
								if (world_->blocks[i_].fg == 1226) {
									if ((world_->blocks[i_].flags & 0x00400000) > 0) world_->blocks[i_].flags ^= 0x00400000;
									world_->blocks[i_].fg = 9162;
									visuals.push_back(i_);
								}
								if (world_->blocks[i_].fg == 1224) {
									Creep += to_string(i_) + ", ";
									if (i_ % xSize > 50) sgrpllist.push_back(i_);
									visuals.push_back(i_);
								}
								if (world_->blocks[i_].fg == 9162) {
									Eye += to_string(i_) + ", ";
									if (i_ % xSize > 50) sevillist.push_back(i_);
								}
							}

							int scrp = 0;
							int secrp = 0;
							while (splatformadd.size() < 13) {
								scrp = sgrpllist[rand() % sgrpllist.size()];
								if (find(splatformadd.begin(), splatformadd.end(), scrp) == splatformadd.end()) splatformadd.push_back(scrp);
							}
							while (seviladd.size() < 9) {
								secrp = sevillist[rand() % sevillist.size()];
								if (find(seviladd.begin(), seviladd.end(), secrp) == seviladd.end()) seviladd.push_back(secrp);
							}
						}

						if (find(gamesbusy.begin(), gamesbusy.end(), "Growganoth_Left") != gamesbusy.end() && Environment()->last_carnig_time - ms_time <= 10000) {
							vector<int> listeye{ 2301, 2601, 2605, 2695, 2801, 2994, 3004, 3197, 3202, 3401, 3402, 3505, 3594, 3595, 3596, 3597, 3598, 3604, 3705, 3897, 3901, 4003, 4198, 4301, 4302, 4303, 4304, 4305, 4394, 4502, 4704, 4705, 4801, 4905, 5096, 5198 };
							vector<int> listcreep{ 2304, 2401, 2405, 2495, 2502, 2597, 2694, 2702, 2704, 2805, 2895, 2902, 2996, 2998, 3001, 3103, 3105, 3195, 3201, 3294, 3304, 3398, 3495, 3496, 3497, 3602, 3694, 3701, 3704, 3797, 3805, 4004, 4095, 4098, 4101, 4105, 4194, 4196, 4202, 4203, 4204, 4205, 4296, 4398, 4495, 4501, 4504, 4595, 4597, 4601, 4703, 4794, 4797, 4802, 4898, 4995, 5001, 5003, 5004, 5194, 5202, 5205, 5297 };

							for (int i = 0; i < listcreep.size(); i++) {
								int i_ = listcreep[i];
								if (world_->blocks[i_].fg == 1222) {
									world_->blocks[i_].fg = 1224;
								}
								if (world_->blocks[i_].fg == 1224) {
									Rcreep += to_string(i_) + ", ";
									grpllist.push_back(i_);
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
										if (pInfo(currentPeer)->world == name_) {
											x = i_ % xSize;
											y = i_ / xSize;
											WorldBlock* block_ = &world_->blocks.at(i_);
											PlayerMoving data_{};

											data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
											BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
											BYTE* blc = raw + 56;
											form_visual(blc, *block_, *world_, currentPeer, false);
											send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
											delete[] raw;
										}
									}
								}
							}

							for (int i = 0; i < listeye.size(); i++) {
								int i_ = listeye[i];
								if (world_->blocks[i_].fg == 1226) {
									world_->blocks[i_].fg = 9162;
									for (ENetPeer* currentPeer : world_->joinedPlayers) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
										x = i_ % xSize;
										y = i_ / xSize;
										WorldBlock* block_ = &world_->blocks.at(i_);
										PlayerMoving data_{};

										data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
										BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
										BYTE* blc = raw + 56;
										form_visual(blc, *block_, *world_, currentPeer, false);
										send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
										delete[] raw;
									}
								}
								if (world_->blocks[i_].fg == 9162) {
									evillist.push_back(i_);
								}
							}

							int crp = 0;
							int ecrp = 0;
							while (platformadd.size() < 13) {
								crp = grpllist[rand() % grpllist.size()];
								if (find(platformadd.begin(), platformadd.end(), crp) == platformadd.end()) platformadd.push_back(crp);
							}
							while (eviladd.size() < 9) {
								ecrp = evillist[rand() % evillist.size()];
								if (find(eviladd.begin(), eviladd.end(), ecrp) == eviladd.end()) eviladd.push_back(ecrp);
							}

							world_->blocks[2203].flags = world_->blocks[2203].flags & 0x00400000 ? world_->blocks[2203].flags : world_->blocks[2203].flags | 0x00400000;
							for (ENetPeer* currentPeer : world_->joinedPlayers) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
								x = 2203 % xSize;
								y = 2203 / xSize;
								WorldBlock* block_ = &world_->blocks.at(2203);
								PlayerMoving data_{};

								data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
								BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
								BYTE* blc = raw + 56;
								form_visual(blc, *block_, *world_, currentPeer, false);
								send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
								delete[] raw;
							}
							for (auto& [owner, bot] : bots) {
								if (!bot.active) continue;
								if (time(nullptr) - bot.last_spam_time >= 3) {
									World* bot_world = find_world(bot.world_name);
									if (bot_world != nullptr) {
										for (ENetPeer* cp : bot_world->joinedPlayers) {
											if (cp->state != ENET_PEER_STATE_CONNECTED || cp->data == NULL) continue;
											VarList::OnTalkBubble(cp, bot.netID, bot.text, 2, false);
										}
									}
									bot.last_spam_time = time(nullptr);
								}
							}
							for (int i = 0; i < platformadd.size(); i++) {
								int i_ = platformadd[i];
								if (find(platformadd.begin(), platformadd.end(), i_) != platformadd.end()) {
									world_->blocks[i_].fg = 1222;
									for (ENetPeer* currentPeer : world_->joinedPlayers) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
										x = i_ % xSize;
										y = i_ / xSize;
										WorldBlock* block_ = &world_->blocks.at(i_);
										PlayerMoving data_{};

										data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
										BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
										BYTE* blc = raw + 56;
										form_visual(blc, *block_, *world_, currentPeer, false);
										send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
										delete[] raw;
									}
								}
							}

							for (int i = 0; i < eviladd.size(); i++) {
								int i_ = eviladd[i];
								if (find(eviladd.begin(), eviladd.end(), i_) != eviladd.end()) {
									world_->blocks[i_].flags = world_->blocks[i_].flags & 0x00400000 ? world_->blocks[i_].flags : world_->blocks[i_].flags | 0x00400000;
									world_->blocks[i_].fg = 1226;
									for (ENetPeer* currentPeer : world_->joinedPlayers) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
										x = i_ % xSize;
										y = i_ / xSize;
										WorldBlock* block_ = &world_->blocks.at(i_);
										PlayerMoving data_{};

										data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
										BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
										BYTE* blc = raw + 56;
										form_visual(blc, *block_, *world_, currentPeer, false);
										send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
										delete[] raw;
									}
								}
							}
						}

						if (find(gamesbusy.begin(), gamesbusy.end(), "Shooter_Left") != gamesbusy.end()) {
							sllist = { 5243, 5241, 5239, 5237, 5042, 5040, 5038, 4839, 4841, 4640 };
							int slreward = sllist[rand() % sllist.size()];
							int slr2 = sllist[rand() % sllist.size()], slr3 = sllist[rand() % sllist.size()];
							while (slr2 == slreward) slr2 = sllist[rand() % sllist.size()];
							while (slr3 == slreward || slr3 == slr2) slr3 = sllist[rand() % sllist.size()];
							sltochange = { slreward, slr2, slr3 };
						}

						if (find(gamesbusy.begin(), gamesbusy.end(), "Shooter_Right") != gamesbusy.end()) {
							rllist = { 5275, 5273, 5271, 5269, 5074, 5072, 5070, 4871, 4873, 4672 };
							int rlreward = rllist[rand() % rllist.size()];
							int rlr2 = rllist[rand() % rllist.size()], rlr3 = rllist[rand() % rllist.size()];
							while (rlr2 == rlreward) rlr2 = rllist[rand() % rllist.size()];
							while (rlr3 == rlreward || rlr3 == rlr2) rlr3 = rllist[rand() % rllist.size()];
							rltochange = { rlreward, rlr2, rlr3 };
						}

						int nx = 0, ny = 0;
						bool finna = false;
						if (find(gamesbusy.begin(), gamesbusy.end(), "Growganoth_Right") != gamesbusy.end() || find(gamesbusy.begin(), gamesbusy.end(), "Growganoth_Left") != gamesbusy.end()) finna = true;

						if (find(gamesbusy.begin(), gamesbusy.end(), "Growganoth_Left") == gamesbusy.end()) {
							for (int i_ = 0; i_ < world_->blocks.size(); i_++) {
								bool visual = false;

								if (find(gamesbusy.begin(), gamesbusy.end(), "Growganoth_Right") != gamesbusy.end() && Environment()->last_carnig_time - ms_time <= 10000) {
									finna = true;
									if (i_ == 2296) world_->blocks[i_].flags = world_->blocks[i_].flags & 0x00400000 ? world_->blocks[i_].flags : world_->blocks[i_].flags | 0x00400000, visual = true;

									if (find(splatformadd.begin(), splatformadd.end(), i_) != splatformadd.end()) {
										world_->blocks[i_].fg = 1222;
										visual = true;
									}
									if (find(seviladd.begin(), seviladd.end(), i_) != seviladd.end()) {
										world_->blocks[i_].flags = world_->blocks[i_].flags & 0x00400000 ? world_->blocks[i_].flags : world_->blocks[i_].flags | 0x00400000;
										world_->blocks[i_].fg = 1226;
										visual = true;
									}
								}

								if (find(gamesbusy.begin(), gamesbusy.end(), "Shooter_Left") != gamesbusy.end()) {
									if (world_->blocks[i_].fg == 1908 && find(sltochange.begin(), sltochange.end(), i_) == sltochange.end() && find(sllist.begin(), sllist.end(), i_) != sllist.end()) {
										if ((world_->blocks[i_].flags & 0x00400000) > 0) world_->blocks[i_].flags ^= 0x00400000;
										visual = true;
									}
								}

								if (find(gamesbusy.begin(), gamesbusy.end(), "Shooter_Right") != gamesbusy.end()) {
									if (world_->blocks[i_].fg == 1908 && find(rltochange.begin(), rltochange.end(), i_) == rltochange.end() && find(rllist.begin(), rllist.end(), i_) != rllist.end()) {
										if ((world_->blocks[i_].flags & 0x00400000) > 0) world_->blocks[i_].flags ^= 0x00400000;
										visual = true;
									}
								}

								if (find(gamesbusy.begin(), gamesbusy.end(), "Shooter_Left") != gamesbusy.end()) {
									if (find(sltochange.begin(), sltochange.end(), i_) != sltochange.end()) {
										x = i_ % xSize;
										y = i_ / xSize;
										WorldBlock* block_ = &world_->blocks.at(i_);
										if (block_->fg == 1908 && (world_->blocks[(int)x + ((int)y * 100)].flags & 0x00400000) == 0) {
											block_->flags |= 0x00400000;
											visual = true;
										}
									}
								}

								if (find(gamesbusy.begin(), gamesbusy.end(), "Shooter_Right") != gamesbusy.end()) {
									if (find(rltochange.begin(), rltochange.end(), i_) != rltochange.end()) {
										x = i_ % xSize;
										y = i_ / xSize;
										WorldBlock* block_ = &world_->blocks.at(i_);
										if (block_->fg == 1908 && (world_->blocks[(int)x + ((int)y * 100)].flags & 0x00400000) == 0) {
											block_->flags |= 0x00400000;
											visual = true;
										}
									}
								}

								if (visual || find(visuals.begin(), visuals.end(), i_) != visuals.end()) {
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
										if (pInfo(currentPeer)->world == name_) {
											x = i_ % xSize;
											y = i_ / xSize;
											WorldBlock* block_ = &world_->blocks.at(i_);
											PlayerMoving data_{};

											data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
											BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
											BYTE* blc = raw + 56;
											form_visual(blc, *block_, *world_, currentPeer, false);
											send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
											delete[] raw;
										}
									}
								}
							}
						}

						if (finna) Environment()->last_carnig_time = ms_time + 10000; // long long, no overflow
					}
				}
			} // ── end if (Last_Carnival) ──────────────────────────────────
			if (Environment()->Last_Update_Rich - ms_time <= 0) {
				Environment()->Last_Update_Rich = ms_time + 60000;
				top_richest.clear();
				try {
					for (const auto& entry : std::filesystem::directory_iterator("database/players/")) {
						if (!entry.is_regular_file() || entry.path().extension() != ".json") continue;
						try {
							std::ifstream ifs(entry.path());
							if (!ifs.is_open()) continue;
							json j;
							ifs >> j;
							ifs.close();
							std::string pName = j.value("name", "");
							if (pName.empty()) continue;
							if (j.contains("b_s") && j["b_s"].get<long long int>() != 0) continue;
							long long world_lock = 0;
							if (j.contains("inventory")) {
								auto inv = j["inventory"].get<std::vector<std::pair<int, int>>>();
								for (auto& pr : inv) {
									if (pr.first == 242) world_lock += pr.second;
									else if (pr.first == 1796) world_lock += (long long)pr.second * 100;
									else if (pr.first == 7188) world_lock += (long long)pr.second * 10000;
									else if (pr.first == 8470) world_lock += (long long)pr.second * 1000000;
								}
							}
							world_lock += (long long)j.value("wl_bank_amount", 0);
							world_lock += (long long)j.value("dl_bank_amount", 0) * 100;
							world_lock += (long long)j.value("bgl_bank_amount", 0) * 10000;
							world_lock += (long long)j.value("ggl_bank_amount", 0) * 1000000;
							long long total_gems = (long long)j.value("gems", 0) / 2000;
							top_richest.push_back({ total_gems + world_lock, pName });
						}
						catch (...) { continue; }
					}
				}
				catch (...) {}
				std::sort(top_richest.begin(), top_richest.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->tankIDName.empty()) continue;
					string find = pInfo(cp_)->tankIDName;
					if (pInfo(cp_)->growpass_points != 0) {
						auto it2 = std::find_if(top_royalpass.begin(), top_royalpass.end(), [&](const auto& p) { return p.second == find; });
						if (it2 != top_royalpass.end()) *it2 = { pInfo(cp_)->growpass_points, find };
						else top_royalpass.push_back({ pInfo(cp_)->growpass_points, find });
					}
					if (pInfo(cp_)->Mines_Score != 0) {
						auto it3 = std::find_if(top_mines.begin(), top_mines.end(), [&](const auto& p) { return p.second == find; });
						if (it3 != top_mines.end()) *it3 = { pInfo(cp_)->Mines_Score, find };
						else top_mines.push_back({ pInfo(cp_)->Mines_Score, find });
					}
					if (pInfo(cp_)->punch_count != 0) {
						auto it4 = std::find_if(top_punch.begin(), top_punch.end(), [&](const auto& p) { return p.second == find; });
						if (it4 != top_punch.end()) *it4 = { pInfo(cp_)->punch_count, find };
						else top_punch.push_back({ pInfo(cp_)->punch_count, find });
					}
				}
				std::sort(top_royalpass.begin(), top_royalpass.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
				std::sort(top_mines.begin(), top_mines.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
				std::sort(top_punch.begin(), top_punch.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
			}
			if (Environment()->Last_SystemInfo - ms_time <= 0) {
				Environment()->Last_SystemInfo = ms_time + 30000000;
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL || pInfo(cp_)->world.empty()) continue;
					VarList::OnConsoleMessage(cp_, systemInfo[rand() % systemInfo.size()]);
					VarList::OnPlaySound(cp_, "audio/beep.wav");
				}
			}
			if (Environment()->Last_Firehouse - ms_time <= 0) {
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL or pInfo(cp_)->tankIDName.empty()) continue;
					std::string name_ = pInfo(cp_)->world;
					World* world = find_world(name_);
					if (world != nullptr) {
						world->fresh_world = true;
						std::vector<WorldBlock>::iterator p3 = find_if(world->blocks.begin(), world->blocks.end(), [&](const WorldBlock& a) { return a.fg == 3072; });
						if (p3 != world->blocks.end()) {
							std::vector<WorldBlock>::iterator p2 = find_if(world->blocks.begin(), world->blocks.end(), [&](const WorldBlock& a) { return a.flags & 0x10000000; });
							if (p2 != world->blocks.end()) {
								int x_ = int(p2 - world->blocks.begin()) % 100, y_ = int(p2 - world->blocks.begin()) / 100;
								int x_x = int(p3 - world->blocks.begin()) % 100, y_y = int(p3 - world->blocks.begin()) / 100;
								PlayerMoving data_{};
								data_.packetType = 36, data_.netID = 34, data_.characterState = 0x8, data_.x = x_x, data_.y = y_y;
								int32_t to_netid = 0x10000000;
								BYTE* raw = packPlayerMoving(&data_);
								raw[3] = 5;
								Memory_Copy(raw + 8, &to_netid, 4);
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(cp_)->world == world->name) {
										send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									}
								}
								delete[] raw;
								apply_tile_visual(world, &world->blocks[x_ + (y_ * 100)], x_, y_, 0x10000000, true);
							}
						}
						if (Environment()->Last_Fire_Time - ms_time <= 0) {
							if (world->total_fires < 150) {
								std::vector<WorldBlock>::iterator p2 = find_if(world->blocks.begin(), world->blocks.end(), [&](const WorldBlock& a) { return a.flags & 0x10000000 && (world->fire_try > 10 ? a.applied_fire == true : a.applied_fire == false); });
								if (p2 != world->blocks.end()) {
									int x_ = int(p2 - world->blocks.begin()) % 100, y_ = int(p2 - world->blocks.begin()) / 100;
									std::vector<int> random_xy{ 1, 0, -1, 0 };
									int randomx = 0, randomy = 0;
									if (rand() % 2 < 1) randomx = x_ + random_xy[rand() % random_xy.size()], randomy = y_;
									else randomx = x_, randomy = y_ + random_xy[rand() % random_xy.size()];
									if (randomx > 0 && randomx < world->max_x && randomy > 0 && randomy < world->max_y) {
										bool has_fire = world->blocks[randomx + (randomy * 100)].flags & 0x10000000, has_water = world->blocks[randomx + (randomy * 100)].flags & 0x04000000;
										if (world->blocks[randomx + (randomy * 100)].fg != 0 && has_fire == false && has_water == false && items[world->blocks[randomx + (randomy * 100)].fg].blockType != BlockTypes::MAIN_DOOR && items[world->blocks[randomx + (randomy * 100)].fg].blockType != BlockTypes::BEDROCK && world->blocks[randomx + (randomy * 100)].fg != 9570) apply_tile_visual(world, &world->blocks[randomx + (randomy * 100)], randomx, randomy, 0x10000000);
										else {
											world->blocks[x_ + (y_ * 100)].fire_try++;
											if (world->blocks[x_ + (y_ * 100)].fire_try >= 8) world->blocks[x_ + (y_ * 100)].applied_fire = true, world->blocks[x_ + (y_ * 100)].fire_try = 0;
										}
									}
								}
								else {
									world->fire_try++;
									if (world->fire_try > 10) world->fire_try = 0;
								}
							}
						}
					}
				}
			}
			if (Environment()->Last_Time - ms_time <= 0) {
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL || pInfo(cp_)->world.empty()) continue;
					if (pInfo(cp_)->pet_netID == 0 && pInfo(cp_)->pet_type != -1 && !pInfo(cp_)->world.empty() && pInfo(cp_)->show_pets) {
						Pet_Ai::Create(cp_);
					}
					else if (pInfo(cp_)->pet_netID != 0 && pInfo(cp_)->pet_type != -1 && !pInfo(cp_)->world.empty()) {
						if (!pInfo(cp_)->pet_ClothesUpdated && pInfo(cp_)->show_pets) {
							Pet_Ai::Update(cp_, pInfo(cp_)->pet_netID, pInfo(cp_)->pet_level, pInfo(cp_)->master_pet, pInfo(cp_)->active_bluename);
							// FIX: mark as updated so it does not broadcast every 6 seconds repeatedly
							pInfo(cp_)->pet_ClothesUpdated = true;
						}
					}
				}
				for (int a = 0; a < Environment()->Another_Worlds.size(); a++) {
					std::string name = Environment()->Another_Worlds[a];
					World* world = find_world(name);
					if (world != nullptr) {
						world->fresh_world = true;
						if (world->machines.size() == 0 && world->npc.size() == 0 && world->special_event == false) {
							Environment()->Another_Worlds.erase(Environment()->Another_Worlds.begin() + a);
							a--;
							if (ServerPool::PlayerCountWorld(world->name) == 0) {
								save_world(world->name, true);
							}
							continue;
						}
						if (world->special_event) {
							if (world->last_special_event + 30000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
								for (ENetPeer* currentPeer_event = server->peers; currentPeer_event < &server->peers[server->peerCount]; ++currentPeer_event) {
									if (currentPeer_event->state != ENET_PEER_STATE_CONNECTED or currentPeer_event->data == NULL or pInfo(currentPeer_event)->world != world->name) continue;
									if (world->special_event_name == "Dr. Destructo") {
										if (pInfo(currentPeer_event)->DrDes) {
											VarList::OnConsoleMessage(currentPeer_event, "Destructo has won! (`4Dr. Destructo`` mod removed)");
											pInfo(currentPeer_event)->d_name = "";
											pInfo(currentPeer_event)->DrDes = false, pInfo(currentPeer_event)->ghost = false;
											pInfo(currentPeer_event)->Health_DrDes = 100, pInfo(currentPeer_event)->face = 0, pInfo(currentPeer_event)->shirt = 0, pInfo(currentPeer_event)->pants = 0, pInfo(currentPeer_event)->hair = 0, pInfo(currentPeer_event)->necklace = 0, pInfo(currentPeer_event)->back = 0, pInfo(currentPeer_event)->feet = 0, pInfo(currentPeer_event)->hand = 0, pInfo(currentPeer_event)->punched = 0;
											VisualHandle::State(pInfo(currentPeer_event));
											VisualHandle::Nick(currentPeer_event, NULL);
											Clothing_V2::Update_Value(currentPeer_event);
											Clothing_V2::Update(currentPeer_event);
										}
										VarList::OnConsoleMessage(currentPeer_event, "`4Dr. Destructo `oescaped!");
										VarList::OnAddNotification(currentPeer_event, "`4Dr. Destructo `oescaped!", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
									}
									else {
										VarList::OnConsoleMessage(currentPeer_event, "`2" + items[world->special_event_item].event_name + ":`` " + (items[world->special_event_item].event_total == 1 ? "`oTime's up! Nobody found it!``" : "`oTime's up! " + to_string(world->special_event_item_taken) + " of " + to_string(items[world->special_event_item].event_total) + " items found.``") + "");
										VarList::OnAddNotification(currentPeer_event, "`2" + items[world->special_event_item].event_name + ":`` " + (items[world->special_event_item].event_total == 1 ? "`oTime's up! Nobody found it!``" : "`oTime's up! " + to_string(world->special_event_item_taken) + " of " + to_string(items[world->special_event_item].event_total) + " items found.``") + "", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
									}
									PlayerMoving data_{};
									for (int i_ = 0; i_ < world->drop_new.size(); i_++) {
										if (find(world->world_event_items.begin(), world->world_event_items.end(), world->drop_new[i_][0]) != world->world_event_items.end()) {
											BYTE* raw1_ = PackBlockUpdate(14, 0, 0, 0, 0, 0, 0, 0, world->drop_new[i_][2], 0, 0, 0, 0, 0);
											for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
												if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != world->name) continue;
												send_raw(cp_, 4, raw1_, 56, ENET_PACKET_FLAG_RELIABLE);
											}
											delete[] raw1_;
											world->drop_new.erase(world->drop_new.begin() + i_);
											i_--;
										}
									}
								}
								world->world_event_items.clear();
								world->last_special_event = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count(), world->special_event_item = 0, world->special_event_item_taken = 0, world->special_event = false, world->special_event_name = "";
							}
							continue;
						}
						for (int i_ = 0; i_ < world->machines.size(); i_++) {
							WorldMachines* machine = &world->machines[i_];
							if (world->blocks[machine->x + (machine->y * 100)].pr <= 0 or not world->blocks[machine->x + (machine->y * 100)].enabled or machine->target_item == 0) {
								if (items[world->blocks[machine->x + (machine->y * 100)].fg].blockType == BlockTypes::AUTO_BLOCK) {
									world->machines.erase(world->machines.begin() + i_);
									i_--;
								}
								continue;
							}
							WorldBlock* itemas = &world->blocks[machine->x + (machine->y * 100)];
							int ySize = world->blocks.size() / 100, xSize = world->blocks.size() / ySize;
							if (itemas->pr > 0) {
								if (machine->last_ - ms_time > 0) continue;
								if (itemas->fg == 6952 or (itemas->fg == 6954 && itemas->build_only == false)) {
									int itemas_ = (itemas->fg == 6954 ? machine->target_item - 1 : machine->target_item);
									static std::unordered_map<std::string, int> s_machine_block_cache;
									std::string cache_key = world->name + ":" + std::to_string(itemas_);
									int cached_idx = -1;
									{
										auto cit = s_machine_block_cache.find(cache_key);
										if (cit != s_machine_block_cache.end()) {
											int idx = cit->second;
											if (idx >= 0 && idx < (int)world->blocks.size() &&
												(world->blocks[idx].fg == itemas_ || world->blocks[idx].bg == itemas_))
												cached_idx = idx;
											else
												s_machine_block_cache.erase(cit);
										}
									}
									std::vector<WorldBlock>::iterator p;
									if (cached_idx >= 0) {
										p = world->blocks.begin() + cached_idx;
									}
									else {
										p = find_if(world->blocks.begin(), world->blocks.end(), [&](const WorldBlock& a) { return a.fg == itemas_ or a.bg == itemas_; });
										if (p != world->blocks.end())
											s_machine_block_cache[cache_key] = (int)(p - world->blocks.begin());
									}
									if (p != world->blocks.end()) {
										WorldBlock* block_ = &world->blocks[p - world->blocks.begin()];
										int size = p - world->blocks.begin(), x_ = size % xSize, y_ = size / xSize;
										if (items[itemas_].blockType == BlockTypes::BACKGROUND and block_->fg != 0) continue;
										BYTE* raw1_ = PackBlockUpdate(17, 0x8, x_ * 32 + 16, y_ * 32 + 16, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0);
										BYTE* raw2_ = PackBlockUpdate(36, 0x8, x_ * 32 + 16, y_ * 32 + 16, 0, 0, 0, 110, 0, 0, 0, 0, 0, 0);
										for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
											if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != world->name) continue;
											send_raw(cp_, 4, raw1_, 56, ENET_PACKET_FLAG_RELIABLE);
											send_raw(cp_, 4, raw2_, 56, ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw1_;
										delete[] raw2_;
										itemas->pr--;
										if (itemas->pr <= 0) {
											PlayerMoving data_{};
											data_.packetType = 5, data_.punchX = machine->x, data_.punchY = machine->y, data_.characterState = 0x8;
											BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world, itemas));
											BYTE* blc = raw + 56;
											form_visual(blc, *itemas, *world, NULL, false);
											for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
												if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
												if (pInfo(cp_)->world == world->name) {
													send_raw(cp_, 4, raw, 112 + alloc_(world, itemas), ENET_PACKET_FLAG_RELIABLE);
												}
											}
											delete[] raw;
										}
										if (block_->hp == -1) {
											int breakhits = items[itemas_].breakHits;
											block_->hp = (breakhits == 1 ? breakhits * 3 : breakhits > 3 ? 3 : breakhits);
											block_->lp = ms_time;
										}
										block_->hp -= 1;
										if (block_->hp == 0) {
											if (items[itemas_].max_gems != 0) {
												int maxgems = items[itemas_].max_gems;
												if (itemas_ == 120) maxgems = 50;
												int c_ = rand() % (maxgems + 1);
												if (c_ != 0) {
													bool no_seed = false, no_gems = false, no_block = false;
													if (itemas_ == 2242 or itemas_ == 2244 or itemas_ == 2246 or itemas_ == 2248 or itemas_ == 2250 or itemas_ == 542) no_seed = true, no_block = true;
													else {
														for (int i_ = 0; i_ < world->drop_new.size(); i_++) {
															if (abs(world->drop_new[i_][4] - y_ * 32) <= 16 and abs(world->drop_new[i_][3] - x_ * 32) <= 16) {
																if (world->drop_new[i_][0] == 112 and items[itemas_].rarity < 8) {
																	no_gems = true;
																}
																else {
																	no_seed = true, no_block = true;
																}
															}
														}
													}
													int chanced = 0;
													if (rand() % 100 < 8) {
														WorldDrop drop_block_{};
														drop_block_.id = itemas_, drop_block_.count = 1, drop_block_.x = (x_ * 32) + rand() % 17, drop_block_.y = (y_ * 32) + rand() % 17;
														if (not use_mag(world, drop_block_, x_, y_) and not no_block) {
															VisualHandle::Drop(world, drop_block_);
														}
													}
													else if (rand() % 100 < (items[itemas_].newdropchance + chanced)) {
														WorldDrop drop_seed_{};
														drop_seed_.id = itemas_ + 1, drop_seed_.count = 1, drop_seed_.x = (x_ * 32) + rand() % 17, drop_seed_.y = (y_ * 32) + rand() % 17;
														if (not use_mag(world, drop_seed_, x_, y_) and not no_seed) {
															VisualHandle::Drop(world, drop_seed_);
														}
													}
													else if (not no_gems) {
														drop_rare_item(world, NULL, itemas_, x_, y_, false);
														gems_(NULL, world, c_, x_ * 32, y_ * 32, itemas_);
													}
												}
											}
											reset_(block_, x_, y_, world);
											PlayerMoving data_{};
											data_.packetType = 5, data_.punchX = x_, data_.punchY = y_, data_.characterState = 0x8;
											BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world, block_));
											BYTE* blc = raw + 56;
											form_visual(blc, *block_, *world, NULL, false);
											for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
												if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
												if (pInfo(cp_)->world == world->name) {
													send_raw(cp_, 4, raw, 112 + alloc_(world, block_), ENET_PACKET_FLAG_RELIABLE);
												}
											}
											delete[] raw;
										}
										else {
											BYTE* raw1_ = PackBlockUpdate(0x8, 0x0, x_, y_, 0, 0, 0, -1, 6, x_, y_, 0, 0, 0);
											for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
												if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != world->name) continue;
												send_raw(cp_, 4, raw1_, 56, ENET_PACKET_FLAG_RELIABLE);
											}
											delete[] raw1_;
										}
									}
								}
								else if (itemas->fg == 6950 or (itemas->fg == 6954 && itemas->build_only)) {
									// FIX LAG: cache target seed block → O(1) instead of O(5400) per tick
									static std::unordered_map<std::string, int> s_planter_block_cache;
									std::string pcache_key = world->name + ":seed:" + std::to_string(machine->target_item);
									int pcached_idx = -1;
									{
										auto pcit = s_planter_block_cache.find(pcache_key);
										if (pcit != s_planter_block_cache.end()) {
											int idx = pcit->second;
											if (idx >= 0 && idx < (int)world->blocks.size() && world->blocks[idx].fg == machine->target_item)
												pcached_idx = idx;
											else
												s_planter_block_cache.erase(pcit);
										}
									}
									std::vector<WorldBlock>::iterator p;
									if (pcached_idx >= 0) {
										p = world->blocks.begin() + pcached_idx;
									}
									else {
										p = find_if(world->blocks.begin(), world->blocks.end(), [&](const WorldBlock& a) { return a.fg == machine->target_item; });
										if (p != world->blocks.end())
											s_planter_block_cache[pcache_key] = (int)(p - world->blocks.begin());
									}
									if (p != world->blocks.end()) {
										int a_ = p - world->blocks.begin();
										long long times_ = time(nullptr);
										std::uint32_t laikas = std::uint32_t((times_ - world->blocks[a_].planted <= items[world->blocks[a_].fg].growTime ? times_ - world->blocks[a_].planted : items[world->blocks[a_].fg].growTime));
										if (items[world->blocks[a_].fg].blockType == BlockTypes::SEED and laikas == items[world->blocks[a_].fg].growTime) {
											int x_ = a_ % xSize, y_ = a_ / xSize;
											WorldBlock* block_ = &world->blocks[x_ + (y_ * 100)];
											int drop_count = items[block_->fg - 1].rarity == 1 ? (items[block_->fg - 1].farmable ? (rand() % 6) + 5 : (rand() % block_->fruit) + 1) : items[block_->fg - 1].farmable ? (rand() % 6) + 4 : (rand() % block_->fruit) + 1;
											if (harvest_seed(world, block_, x_, y_, drop_count, -1)) {

											}
											else if (world->weather == 8 and rand() % 300 < 2) {
												WorldDrop drop_block_{};
												drop_block_.id = 3722, drop_block_.count = 1, drop_block_.x = x_ * 32 + rand() % 17, drop_block_.y = y_ * 32 + rand() % 17;
												VisualHandle::Drop(world, drop_block_);
												BYTE* raw1_ = PackBlockUpdate(0x11, 0, drop_block_.x, drop_block_.y, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0);
												for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
													if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != world->name) continue;
													send_raw(cp_, 4, raw1_, 56, ENET_PACKET_FLAG_RELIABLE);
												}
												delete[] raw1_;
											}
											if (drop_count != 0) drop_rare_item(world, NULL, machine->target_item - 1, x_, y_, true);
											BYTE* raw1_ = PackBlockUpdate(17, 0x8, x_ * 32 + 16, y_ * 32 + 16, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0);
											BYTE* raw2_ = PackBlockUpdate(36, 0x8, x_ * 32 + 16, y_ * 32 + 16, 0, 0, 0, 109, 0, 0, 0, 0, 0, 0);
											for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
												if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != world->name) continue;
												send_raw(cp_, 4, raw1_, 56, ENET_PACKET_FLAG_RELIABLE);
												send_raw(cp_, 4, raw2_, 56, ENET_PACKET_FLAG_RELIABLE);
											}
											delete[] raw1_;
											delete[] raw2_;
											itemas->pr--;
											if (itemas->pr <= 0) {
												PlayerMoving data_{};
												data_.packetType = 5, data_.punchX = machine->x, data_.punchY = machine->y, data_.characterState = 0x8;
												BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world, itemas));
												BYTE* blc = raw + 56;
												form_visual(blc, *itemas, *world, NULL, false);
												for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
													if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
													if (pInfo(cp_)->world == world->name) {
														send_raw(cp_, 4, raw, 112 + alloc_(world, itemas), ENET_PACKET_FLAG_RELIABLE);
													}
												}
												delete[] raw;
											}
										}
									}
								}
							}
						}
						long long time_ = time(nullptr);
						for (int i_ = 0; i_ < world->npc.size(); i_++) {
							WorldNPC* npc = &world->npc[i_];
							if (not npc->enabled) continue;
							if (npc->last_ - time_ > 0) continue;
							// FIX LAG #4: O(1) unordered_map.find() — was O(n) linear scan
							int active = 0;
							{
								auto npc_it = active_npc.find(world->name);
								if (npc_it != active_npc.end()) {
									for (const auto& n : npc_it->second) { if (n.uid != -1) active++; if (active > 10) break; }
								}
							}
							if (active > 10) continue;
							npc->last_ = time_ + npc->rate_of_fire;
							WorldBlock* itemas = &world->blocks[npc->x + (npc->y * 100)];
							if (not itemas->enabled) continue;
							switch (itemas->fg) {
							case 8020: case 4344: {
								PlayerMoving data_{};
								data_.packetType = 34;
								data_.x = static_cast<float>(npc->x) * 32 + 16;
								data_.y = static_cast<float>(npc->y) * 32 + (itemas->fg == 8020 ? 6 : 16); //nuo y
								data_.XSpeed = static_cast<float>(npc->x) * 32 + 16;
								data_.YSpeed = static_cast<float>(npc->y) * 32 + (itemas->fg == 8020 ? 6 : 16); // iki y
								data_.punchY = npc->projectile_speed;
								BYTE* raw = packPlayerMoving(&data_);
								uint16_t uid = (active_npc.find(world->name) != active_npc.end() ? active_npc[world->name].size() : 0);
								raw[1] = (itemas->fg == 8020 ? 15 : 8);
								raw[2] = uid;
								raw[3] = 2;
								Memory_Copy(raw + 40, &npc->kryptis, 4);
								npc->uid = uid;
								npc->started_moving = ms_time;
								if (active_npc.find(world->name) != active_npc.end()) {
									active_npc[world->name].emplace_back(*npc);
								}
								else {
									std::vector<WorldNPC> list_;
									list_.emplace_back(*npc);
									active_npc.insert({ world->name, list_ });
								}
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(cp_)->world == world->name and pInfo(cp_)->x != -1) {
										send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									}
								}
								delete[]raw;
								break;
							}
							default: {
								bool cant_del = false;
								std::unordered_map<std::string, std::vector<WorldNPC>>::iterator it;
								// FIX LAG #4b: O(1) active_npc lookup
								{
									auto npc_it2 = active_npc.find(world->name);
									if (npc_it2 != active_npc.end()) {
										for (auto& n2 : npc_it2->second) { if (npc->uid == n2.uid) { cant_del = true; break; } }
									}
								}
								if (not cant_del) {
									world->npc.erase(world->npc.begin() + i_);
								}
								break;
							}
							}
						}
						std::unordered_map<std::string, std::vector<WorldNPC>>::iterator it;
						// FIX LAG #4c: O(1) active_npc lookup for NPC movement
						{
							auto it = active_npc.find(world->name);
							if (it != active_npc.end()) {
								for (int i_ = 0; i_ < it->second.size(); i_++) {
									WorldNPC* npc_ = &it->second[i_];
									if (npc_->uid == -1) continue;
									WorldBlock* itemas = &world->blocks[npc_->x + (npc_->y * 100)];
									double per_sekunde_praeina_bloku = (double)npc_->projectile_speed / 32;
									double praejo_laiko = (double)(ms_time - npc_->started_moving) / 1000;
									double praejo_distancija = (double)per_sekunde_praeina_bloku * (double)praejo_laiko;
									double current_x = ((int)npc_->kryptis == 180 ? (((double)npc_->x - (double)praejo_distancija) * 32) + 16 : (((double)npc_->x + (double)praejo_distancija) * 32) + 16);
									double current_y = (double)npc_->y * 32;
									if (current_x / 32 < 0 or current_x / 32 >= 100 or current_y / 32 < 0 or current_y / 32 >= 60)
									{
										PlayerMoving data_{};
										data_.packetType = 34;
										data_.x = (current_x);
										data_.y = (current_y + (npc_->id == 8020 ? 6 : 16));
										data_.XSpeed = (current_x);
										data_.YSpeed = (current_y + (npc_->id == 8020 ? 6 : 16));
										data_.punchY = npc_->projectile_speed;
										BYTE* raw = packPlayerMoving(&data_);
										raw[1] = (itemas->fg == 8020 ? 15 : 8);
										raw[2] = npc_->uid;
										raw[3] = 7;
										for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
											if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
											if (pInfo(cp_)->world == world->name and pInfo(cp_)->x != -1) {
												send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											}
										}
										delete[]raw;
										npc_->uid = -1;
										continue;
									}
									try {
										WorldBlock* block_ = &world->blocks[current_x / 32 + (current_y / 32 * 100)];
										if (items[block_->fg].collisionType == 1 or (current_x / 32) > 100 or (current_x / 32) < 0) {
											PlayerMoving data_{};
											data_.packetType = 34;
											data_.x = (current_x);
											data_.y = (current_y + (npc_->id == 8020 ? 6 : 16));
											data_.XSpeed = (current_x);
											data_.YSpeed = (current_y + (npc_->id == 8020 ? 6 : 16));
											data_.punchY = npc_->projectile_speed;
											BYTE* raw = packPlayerMoving(&data_);
											raw[1] = (itemas->fg == 8020 ? 15 : 8);
											raw[2] = npc_->uid;
											raw[3] = 7;
											for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
												if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
												if (pInfo(cp_)->world == world->name and pInfo(cp_)->x != -1) {
													send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
												}
											}
											delete[]raw;
											npc_->uid = -1;
										}
									}
									catch (out_of_range) {
										continue;
									}
								}
								break;
							}
						}
					}
				}
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL or pInfo(cp_)->tankIDName.empty() or pInfo(cp_)->world.empty()) continue;
					auto mining_cooldownleft = calc_d(pInfo(cp_)->MiningTime);
					if (mining_cooldownleft < 1 and pInfo(cp_)->In_World_Mining) {
						exit_(cp_, false, true); // FIX DEADLOCK: Worlds() dipanggil under g_world_loop_mutex
						VarList::OnEndMission(cp_);
					}
					if (pInfo(cp_)->face == 4260 and pInfo(cp_)->Present_Goblin - time(nullptr) < 0) {
						pInfo(cp_)->face = 0;
						Clothing_V2::Update_Value(cp_);
						Clothing_V2::Update(cp_);
					}
					if (pInfo(cp_)->world == "WINTERCLASHPARKOUR" and pInfo(cp_)->In_World_Clash) {
						pInfo(cp_)->In_World_Clash = false;
						VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`6I SHALL FACE THE FINALE PARKOUR!!!", 0, 1, 500);
						gamepacket_t packet(500, pInfo(cp_)->netID);
						packet.Insert("OnCountdownStart"), packet.Insert(1500), packet.Insert(-1), packet.CreatePacket(cp_);
					}
					if (pInfo(cp_)->Role.BOOST) {
						if (pInfo(cp_)->Role.BOOST_TIME - time(nullptr) < 0) {
							pInfo(cp_)->Role.BOOST = false;
							pInfo(cp_)->Role.BOOST_TIME = 0;
							VisualHandle::Nick(cp_, NULL);
							VarList::OnConsoleMessage(cp_, "`oYour `5[BOOST]`` Role has run out, type /shop to purchase roles");
						}
					}
					if (pInfo(cp_)->Role.SUPER_BOOST) {
						if (pInfo(cp_)->Role.SUPER_BOOST_TIME - time(nullptr) < 0) {
							pInfo(cp_)->Role.SUPER_BOOST = false;
							pInfo(cp_)->Role.SUPER_BOOST_TIME = 0;
							VisualHandle::Nick(cp_, NULL);
							VarList::OnConsoleMessage(cp_, "`oYour `8[SUPER-BOOST]`` Role has run out, type /shop to purchase roles");
						}
					}

					if (pInfo(cp_)->adventure_begins && pInfo(cp_)->timerActive) {
						if (!player::algorithm::adventureTimers(cp_)) {
							pInfo(cp_)->adventure_begins = false;
							pInfo(cp_)->timerActive = false;
							if (pInfo(cp_)->lives >= 1) {
								pInfo(cp_)->lives = 0;
								VisualHandle::Nick(cp_, NULL);
							}
							VarList::OnCountdownEnd(cp_, pInfo(cp_)->netID);
							Player_Respawn(cp_, true, 0, 1);
							VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, ":'( I failed my adventure!", 1, true);
							pInfo(cp_)->timers = 0;
						}
					}
					if (pInfo(cp_)->hand == 3578 || pInfo(cp_)->face == 3576) {
						if (pInfo(cp_)->hand_torch + 60000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							int got = 0;
							if (pInfo(cp_)->hand == 3578) {
								if (pInfo(cp_)->hand_torch != 0) {
									Inventory::Modify(cp_, 3578, got);
									if (got - 1 >= 1) {
										VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`4My torch went out, but I have " + to_string(got - 1) + " more!``", 0, 0);
									}
									Inventory::Modify(cp_, 3578, got = -1);
								}
							}
							else if (pInfo(cp_)->face == 3576) {
								Inventory::Modify(cp_, 3306, got = -1);
							}
							pInfo(cp_)->hand_torch = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						}
					}
					else if (pInfo(cp_)->hand == 2204 or pInfo(cp_)->hand == 2558 and pInfo(cp_)->x != -1 and pInfo(cp_)->y != -1) {
						if (pInfo(cp_)->random_geiger_time < 100) {
							pInfo(cp_)->random_geiger_time++;
						}
						else {
							pInfo(cp_)->random_geiger_time = 0;
							pInfo(cp_)->geiger_x = (rand() % 100) * 32;
							pInfo(cp_)->geiger_y = (rand() % 54) * 32;
						}
						int hands_ = pInfo(cp_)->hand;
						if (not Playmods::HasById(pInfo(cp_), 10)) {
							if (pInfo(cp_)->geiger_x == -1 and pInfo(cp_)->geiger_y == -1) {
								pInfo(cp_)->geiger_x = (rand() % 100) * 32;
								pInfo(cp_)->geiger_y = (rand() % 54) * 32;
							}
							int a_ = pInfo(cp_)->geiger_x + ((pInfo(cp_)->geiger_y * 100) / 32), b_ = pInfo(cp_)->x + ((pInfo(cp_)->y * 100) / 32), diff = abs(a_ - b_) / 32;
							if (diff < 30) {
								int t_ = 1500;
								if (diff >= 6) t_ = 1350;
								else if (diff < 15) t_ = 1000;
								else if (diff <= 1) t_ = 2500;
								if (pInfo(cp_)->geiger_time + t_ < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
									pInfo(cp_)->geiger_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
									PlayerMoving data_{};
									data_.packetType = 17, data_.characterState = 0x8, data_.x = pInfo(cp_)->x + 10, data_.y = pInfo(cp_)->y + 16, data_.XSpeed = (diff >= 30 ? 0 : (diff >= 15 ? 1 : 2)), data_.YSpeed = 114;
									BYTE* raw = packPlayerMoving(&data_);
									send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									delete[] raw;
									if (diff <= 1) {
										pInfo(cp_)->geiger_x = -1, pInfo(cp_)->geiger_y = -1;
										int give_back_geiger = items[pInfo(cp_)->hand].geiger_give_back;
										{
											int c_ = -1;
											Inventory::Modify(cp_, pInfo(cp_)->hand, c_);
											int c_2 = 1;
											if (Inventory::Modify(cp_, give_back_geiger, c_2) != 0) {
												std::string name_ = pInfo(cp_)->world;
												World* world_ = find_world(name_);
												if (world_ != nullptr) {
													world_->fresh_world = true;
													WorldDrop drop_block_{};
													drop_block_.id = give_back_geiger, drop_block_.count = 1, drop_block_.x = pInfo(cp_)->x + rand() % 17, drop_block_.y = pInfo(cp_)->y + rand() % 17;
													VisualHandle::Drop(world_, drop_block_);
												}
											}
											int seconds = 1800;
											if (Playmods::HasById(pInfo(cp_), 148) and rand() % 50 < 2) {/*Guild Potion: Geiger*/
												seconds = 500;
											}
											pInfo(cp_)->hand = give_back_geiger;
											Clothing_V2::Update(cp_);
											Playmods::Add(cp_, 10, seconds);
											CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/dialog_confirm.wav", 0);
										}
										if (Event()->dailyc_name == "Geiger") Daily_Challenge::Add_Points(cp_, rand() % 50);
										if (pInfo(cp_)->starglitter_geiger < pInfo(cp_)->starglitter_geiger2 && pInfo(cp_)->starglitter_geiger != -1) DailyEvent_Task::MStarglitter(cp_, "geiger", 1);
										if (pInfo(cp_)->lwiz_step == 13) {
											if (pInfo(cp_)->lwiz_quest == 5 || pInfo(cp_)->lwiz_quest == 6 || pInfo(cp_)->lwiz_quest == 7 || pInfo(cp_)->lwiz_quest == 8) {
												add_lwiz_points(cp_, 1);
											}
										}
										if (pInfo(cp_)->role_quest and pInfo(cp_)->quest_number == 1) {
											if (pInfo(cp_)->role_quest_type == "Geiger" and pInfo(cp_)->need_required < pInfo(cp_)->quest_required) {
												pInfo(cp_)->need_required += 1;
												if (pInfo(cp_)->need_required >= pInfo(cp_)->quest_required) {
													pInfo(cp_)->need_required = pInfo(cp_)->quest_required;
													VarList::OnConsoleMessage(cp_, "`9Geiger Quest! `ocomplete! Go claim your points!");
													VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`9Geiger Quest! ``complete! Go claim your points!", 0, 0);
												}
											}
										}
										if (pInfo(cp_)->BiweeklyQ_2 < 20) {
											pInfo(cp_)->BiweeklyQ_2 += 1;
											if (pInfo(cp_)->BiweeklyQ_2 >= 20) {
												pInfo(cp_)->BiweeklyQ_2 = 20;
												VarList::OnConsoleMessage(cp_, "`9Biweekly Goal ''`2Find 20 radioactive items``'' is complete! Go call Crazy Jim!");
												VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`9Biweekly Goal ''`2Find 20 radioactive items``'' is complete! Go call Crazy Jim!", 0, 0);
											}
										}
										int give_times = 1;
										if (pInfo(cp_)->gp) {
											if (complete_gpass_task(cp_, "Geiger")) give_times++;
										}
										for (int i = 0; i < give_times; i++) {
											int item_ = items[hands_].randomitem[rand() % items[hands_].randomitem.size()], c_ = 1;
											if (item_ == 1486) if (pInfo(cp_)->lwiz_step == 6) add_lwiz_points(cp_, 1);
											if (item_ == 1486 && pInfo(cp_)->C_QuestActive && pInfo(cp_)->C_QuestKind == 11 && pInfo(cp_)->C_QuestProgress < pInfo(cp_)->C_ProgressNeeded) {
												pInfo(cp_)->C_QuestProgress += 1;
												if (pInfo(cp_)->C_QuestProgress >= pInfo(cp_)->C_ProgressNeeded) {
													pInfo(cp_)->C_QuestProgress = pInfo(cp_)->C_ProgressNeeded;
													VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`9Ring Quest task complete! Go tell the Ringmaster!", 0, 0);
												}
											}
											if (Inventory::Modify(cp_, item_, c_) != 0) {
												std::string name_ = pInfo(cp_)->world;
												World* world_ = find_world(name_);
												if (world_ != nullptr) {
													world_->fresh_world = true;
													WorldDrop drop_block_{};
													drop_block_.id = item_, drop_block_.count = 1, drop_block_.x = pInfo(cp_)->x + rand() % 17, drop_block_.y = pInfo(cp_)->y + rand() % 17;
													VisualHandle::Drop(world_, drop_block_);
												}
											}
											VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "I found `21 " + items[item_].name + "``!" + (hands_ == 2558 ? " But now I lost it in my basket!" : "") + "", 0, 0);
											PlayerMoving data_{};
											data_.packetType = 19, data_.plantingTree = 0, data_.netID = 0;
											data_.punchX = item_;
											data_.x = pInfo(cp_)->x + 10, data_.y = pInfo(cp_)->y + 16;
											int32_t to_netid = pInfo(cp_)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 5;
											Memory_Copy(raw + 8, &to_netid, 4);
											for (ENetPeer* cp_2 = server->peers; cp_2 < &server->peers[server->peerCount]; ++cp_2) {
												if (cp_2->state != ENET_PEER_STATE_CONNECTED or cp_2->data == NULL) continue;
												if (pInfo(cp_2)->world == pInfo(cp_)->world) {
													send_raw(cp_2, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
													VarList::OnConsoleMessage(cp_2, get_player_nick(cp_) + " found `21 " + items[item_].name + "``!");
												}
											}
											delete[]raw;
										}
										CAction::Effect(cp_, 48, (float)pInfo(cp_)->x + 10, (float)pInfo(cp_)->y + 16);
									}
								}
							}
						}
					}
					if (pInfo(cp_)->text_time - ms_time <= 0) {
						if (pInfo(cp_)->show_pets and pInfo(cp_)->RandomSentences) {
							std::string pet_world_ = pInfo(cp_)->world;
							ENetPeer* pet_owner_ = cp_;
							best_quote = Environment()->random_[rand() % Environment()->random_.size()];
							for (ENetPeer* inner_cp = server->peers; inner_cp < &server->peers[server->peerCount]; ++inner_cp) {
								if (inner_cp->state != ENET_PEER_STATE_CONNECTED or inner_cp->data == NULL || pInfo(inner_cp)->world.empty()) continue;
								if (pInfo(inner_cp)->world == pet_world_) {
									VarList::OnConsoleMessage(inner_cp, "CP:_PL:0_OID:_CT:[W]_ `6<`w" + pInfo(pet_owner_)->pet_name + "`6> " + best_quote + "");
									VarList::OnTalkBubble(inner_cp, pInfo(pet_owner_)->pet_netID, best_quote, 2, 0);
								}
							}
						}
					}
					if (pInfo(cp_)->text_time - ms_time <= 0) pInfo(cp_)->text_time = ms_time + 500000;
					if (pInfo(cp_)->save_time + 300000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
						if (pInfo(cp_)->save_time != 0) {
							int opco_earned = 50;
							pInfo(cp_)->opc += 50;
							if (pInfo(cp_)->show_pets and pInfo(cp_)->pet_level >= 40) {
								pInfo(cp_)->opc += 5;
								opco_earned += 5;
								VarList::OnTalkBubble(cp_, pInfo(cp_)->pet_netID, "Received `25`` Opc");
							}
							if (pInfo(cp_)->hand == 10384) {
								pInfo(cp_)->opc += 2;
								opco_earned += 2;
							}
							if (pInfo(cp_)->gp || pInfo(cp_)->hand == 10384 || pInfo(cp_)->hair == 9542 || pInfo(cp_)->hair == 9984 || pInfo(cp_)->hair == 9920 || pInfo(cp_)->necklace == 9964 || pInfo(cp_)->necklace == 10176 || pInfo(cp_)->pants == 9782 || pInfo(cp_)->hand == 9880 || pInfo(cp_)->hand == 10020 || pInfo(cp_)->hand == 9974 || pInfo(cp_)->hand == 9918 || pInfo(cp_)->hand == 10290 || pInfo(cp_)->hand == 9916 || pInfo(cp_)->hand == 9914 || pInfo(cp_)->hand == 9766 || pInfo(cp_)->hand == 9772 || pInfo(cp_)->hand == 9908) {
								pInfo(cp_)->opc++;
								opco_earned++;
							}
							VarList::OnConsoleMessage(cp_, "`9[OPC] `wYou received `2" + to_string(opco_earned) + " Online Point Currency``! Total: `2" + to_string(pInfo(cp_)->opc) + " OPC``");
							for (int i_ = 0; i_ < world_rating.size(); i_++) {
								if (world_rating[i_].name == pInfo(cp_)->world) ServerPool::Honors::Add(pInfo(cp_)->world, pInfo(cp_)->world_owner);
							}
							if (pInfo(cp_)->EarnFreeGems.Quest_2 < 60) pInfo(cp_)->EarnFreeGems.Quest_2 += 5;
							Looping::Events(cp_);
							ServerPool::Top_Player::Add(to_lower(pInfo(cp_)->tankIDName));
						}
						pInfo(cp_)->save_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
					}
					if (pInfo(cp_)->World_Timed != 0 and pInfo(cp_)->WorldTimed) {
						std::string name_ = pInfo(cp_)->world;
						World* world_ = find_world(name_);
						if (world_ != nullptr) {
							if (to_lower(world_->owner_name) != to_lower(pInfo(cp_)->tankIDName)) {
								if (pInfo(cp_)->World_Timed - time(nullptr) == 60 && pInfo(cp_)->WorldTimed) {
									VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "Your access to this world will expire in less than a minute!", 0, 0);
								}
								else if (pInfo(cp_)->World_Timed - time(nullptr) < 0 && pInfo(cp_)->WorldTimed) {
									VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "Your access to this world has expired!", 0, 0);
									exit_(cp_, false, true); // FIX DEADLOCK: Worlds() dipanggil under g_world_loop_mutex
								}
							}
						}
					}
					if (pInfo(cp_)->fishing_used != 0) {
						if (pInfo(cp_)->last_fish_catch + pInfo(cp_)->fish_seconds < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() && rand() % 100 < (pInfo(cp_)->hand == 6258 ? 100 : pInfo(cp_)->hand == 3010 ? 95 : 90)) {
							PlayerMoving data_{};
							data_.packetType = 17, data_.netID = 34, data_.YSpeed = 34, data_.x = pInfo(cp_)->f_x * 32 + 16, data_.y = pInfo(cp_)->f_y * 32 + 16;
							pInfo(cp_)->last_fish_catch = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
							BYTE* raw = packPlayerMoving(&data_);
							for (ENetPeer* cp__event = server->peers; cp__event < &server->peers[server->peerCount]; ++cp__event) {
								if (cp__event->state != ENET_PEER_STATE_CONNECTED or cp__event->data == NULL or pInfo(cp__event)->world != pInfo(cp_)->world) continue;
								send_raw(cp__event, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
								CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/splash.wav", 0);
							}
							delete[] raw;
							if (pInfo(cp_)->cheater_settings & SETTINGS::SETTINGS_16 && pInfo(cp_)->disable_cheater == 0) {
								int bait = pInfo(cp_)->fishing_used, fx = pInfo(cp_)->f_x, fy = pInfo(cp_)->f_y;
								stop_fishing(cp_, false, "");
								bool can_recast = true;
								if (pInfo(cp_)->hair == 3042 && rand() % 100 < 5) {

								}
								else {
									int rem = -1;
									if (Inventory::Modify(cp_, bait, rem) != 0) {
										can_recast = false;
									}
								}
								if (can_recast) {
									pInfo(cp_)->fishing_used = bait, pInfo(cp_)->f_x = fx, pInfo(cp_)->f_y = fy;
									pInfo(cp_)->last_fish_catch = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
									PlayerMoving re_{};
									re_.packet_3 = 0, re_.netID = pInfo(cp_)->netID, re_.x = fx, re_.y = fy;
									BYTE* re_raw = packFishMoving(&re_);
									for (ENetPeer* cp__event = server->peers; cp__event < &server->peers[server->peerCount]; ++cp__event) {
										if (cp__event->state != ENET_PEER_STATE_CONNECTED or cp__event->data == NULL or pInfo(cp__event)->world != pInfo(cp_)->world) continue;
										send_raw(cp__event, 4, re_raw, 56, ENET_PACKET_FLAG_RELIABLE);
									}
									delete[] re_raw;
								}
								else {
									VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "You ran out of bait!", 0, 0);
									VarList::OnConsoleMessage(cp_, "You ran out of bait!");
								}
							}
						}
					}
					long long time_ = time(nullptr);
					bool clothing_needs_update_ = false; // FIX LAG: batch Clothing_V2::Update
					for (int i_ = 0; i_ < pInfo(cp_)->playmods.size(); i_++) {
						if (pInfo(cp_)->playmods[i_].id == 12) {
							if (pInfo(cp_)->valentine_time + 2500 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
								pInfo(cp_)->valentine_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
								for (ENetPeer* valentine = server->peers; valentine < &server->peers[server->peerCount]; ++valentine) {
									if (valentine->state != ENET_PEER_STATE_CONNECTED or valentine->data == NULL) continue;
									if (pInfo(valentine)->world == pInfo(cp_)->world and pInfo(valentine)->tankIDName == pInfo(cp_)->playmods[i_].user) {
										if (not pInfo(valentine)->invis and not pInfo(cp_)->invis and pInfo(cp_)->x != -1 and pInfo(cp_)->y != -1 and pInfo(valentine)->x != -1 and pInfo(valentine)->y != -1) {
											gamepacket_t p;
											p.Insert("OnParticleEffect");
											p.Insert(13);
											p.Insert((float)pInfo(valentine)->x + 10, (float)pInfo(valentine)->y + 16);
											p.Insert((float)0), p.Insert((float)pInfo(cp_)->netID);
											bool double_send = false;
											for (int i_2 = 0; i_2 < pInfo(valentine)->playmods.size(); i_2++) {
												if (pInfo(valentine)->playmods[i_2].id == 12 and pInfo(valentine)->playmods[i_2].user == pInfo(cp_)->tankIDName) {
													double_send = true;
													break;
												}
											}
											gamepacket_t p2;
											p2.Insert("OnParticleEffect");
											p2.Insert(13);
											p2.Insert((float)pInfo(cp_)->x + 10, (float)pInfo(cp_)->y + 16);
											p2.Insert((float)0), p2.Insert((float)pInfo(valentine)->netID);
											for (ENetPeer* valentine_bc = server->peers; valentine_bc < &server->peers[server->peerCount]; ++valentine_bc) {
												if (valentine_bc->state != ENET_PEER_STATE_CONNECTED or valentine_bc->data == NULL) continue;
												if (pInfo(valentine_bc)->world == pInfo(cp_)->world) {
													p.CreatePacket(valentine_bc);
													if (double_send) p2.CreatePacket(valentine_bc);
												}
											}
										}
										break;
									}
								}
							}
						}
						//carnival stuff
						if (!pInfo(cp_)->carnival_playing.empty()) {
							long long elapsed = (duration_cast<seconds>(system_clock::now().time_since_epoch())).count() - pInfo(cp_)->carnival_entered;
							if (elapsed >= 60) {
								std::string game = pInfo(cp_)->carnival_playing;
								if (game == "Shooter_Left" || game == "Shooter_Right") {
									if (pInfo(cp_)->carnivalscore >= 30) {
										carnivalPrize(cp_, game == "Shooter_Left" ? 1 : 10);
									} else {
										VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`8You only scored " + to_string(pInfo(cp_)->carnivalscore) + " points!`` No prize for you!");
										VarList::OnConsoleMessage(cp_, "`8You only scored " + to_string(pInfo(cp_)->carnivalscore) + " points!`` No prize for you!");
										gamepacket_t p, j;
										p.Insert("OnRaceEnd"), p.CreatePacket(cp_);
										j.Insert("OnCooldownEnd"), j.CreatePacket(cp_);
										carnivalEnd(cp_, "end");
									}
								}
								else if (game == "Brutal_Bounce") {
									carnivalPrize(cp_, 6);
									carnivalEnd(cp_, "end");
								}
								else if (game == "Spiky_Survival") {
									carnivalPrize(cp_, 8);
									carnivalEnd(cp_, "end");
								}
								else {
									VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "`8You didn't make it in time!`` No prize for you!");
									VarList::OnConsoleMessage(cp_, "`8You didn't make it in time!`` No prize for you!");
									gamepacket_t p, j;
									p.Insert("OnRaceEnd"), p.CreatePacket(cp_);
									j.Insert("OnCooldownEnd"), j.CreatePacket(cp_);
									carnivalEnd(cp_, "end");
								}
							}
						}
						// Speedrun: check if player reached end tile
						if (pInfo(cp_)->carnival_playing == "Speedrun_5000") {
							int tx = (int)(pInfo(cp_)->x / 32), ty = (int)(pInfo(cp_)->y / 32);
							if (tx == 25 && (ty == 10 || ty == 11)) {
								gamepacket_t p, j;
								p.Insert("OnRaceEnd"), p.CreatePacket(cp_);
								j.Insert("OnCooldownEnd"), j.CreatePacket(cp_);
								carnivalPrize(cp_, 7);
							}
						}
						if (pInfo(cp_)->playmods[i_].time - time_ < 0) {
							if (pInfo(cp_)->playmods[i_].id == 162) {
								VarList::OnCountdownEnd(cp_, pInfo(cp_)->netID);
								exit_(cp_, false, true); // FIX DEADLOCK: Worlds() dipanggil under g_world_loop_mutex
							}
							if (pInfo(cp_)->playmods[i_].id == 125) {
								pInfo(cp_)->Role.Moderator = false;
								if (not pInfo(cp_)->d_name.empty()) {
									pInfo(cp_)->d_name = "";
									VisualHandle::Nick(cp_, NULL);
								}
							}
							else if (pInfo(cp_)->playmods[i_].id == 126) pInfo(cp_)->Role.Vip = false;
							else if (pInfo(cp_)->playmods[i_].id == 127 || pInfo(cp_)->playmods[i_].id == 128) exit_(cp_, false, true); // FIX DEADLOCK: Worlds() dipanggil under g_world_loop_mutex
							else if (pInfo(cp_)->playmods[i_].id == 136) pInfo(cp_)->hit_by = 0;
							else if (pInfo(cp_)->playmods[i_].id == 143) {
								pInfo(cp_)->Role.Cheats = false;
								pInfo(cp_)->cheater_settings = 0;
								autofarm_status(cp_);
							}
							CAction::Positioned(cp_, pInfo(cp_)->netID, "audio/dialog_confirm.wav", 0);
							VarList::OnConsoleMessage(cp_, info_about_playmods[pInfo(cp_)->playmods[i_].id - 1][5] + " (`$" + info_about_playmods[pInfo(cp_)->playmods[i_].id - 1][3] + "`` mod removed)");
							pInfo(cp_)->playmods.erase(pInfo(cp_)->playmods.begin() + i_);
							// FIX LAG: set dirty flag, jangan langsung Update (batch di bawah)
							clothing_needs_update_ = true;
							break;
						}
					}
					// FIX LAG: Clothing_V2::Update called ONCE after playmod loop finishes
					// Cegah N*N broadcast jika banyak playmod expire sekaligus di world padat
					if (clothing_needs_update_) {
						Clothing_V2::Update_Value(cp_);
						Clothing_V2::Update(cp_);
					}
				}
				if (Environment()->Last_Time - ms_time <= 0) Environment()->Last_Time = ms_time + 6000; // was 2500ms
				if (Environment()->Last_Firehouse - ms_time <= 0) Environment()->Last_Firehouse = ms_time + 2000000;
				if (Environment()->Last_Fire_Time - ms_time <= 0) Environment()->Last_Fire_Time = ms_time + 2000000;
			}
		}
	}
	static void autofarm() {
		// Timer check dihapus — autofarmdelay() yang mengatur interval via sleep,
		// Last_Autofarm double-if here is redundant and causes timing drift
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL || pInfo(cp_)->world.empty()) continue;
			if (pInfo(cp_)->cheater_settings & SETTINGS::SETTINGS_0 && pInfo(cp_)->disable_cheater == 0) {
				if (pInfo(cp_)->last_used_block != 0 && pInfo(cp_)->autofarm_x != -1) {
					// FIX NULL CRASH: verify world still exists before Punch
					World* af_world = find_world(pInfo(cp_)->world);
					if (af_world == nullptr || af_world->blocks.empty()) continue;
					int active_slots = pInfo(cp_)->autofarm_slot;
					int max_slots = getAutoFarmSlot(pInfo(cp_));
					if (active_slots > max_slots) active_slots = max_slots;
					for (int i = 0; i < active_slots; i++) {
						PlayerCharacter::Punch(
							cp_,
							pInfo(cp_)->last_used_block,
							pInfo(cp_)->autofarm_x + (pInfo(cp_)->backwards ? i * -1 : i),
							pInfo(cp_)->autofarm_y,
							pInfo(cp_)->x,
							pInfo(cp_)->y,
							true
						);
					}
				}
			}
		}


	}

	static void worldEvent() {
		auto sendJarDrop = [&](World* world, float x, float y) {
			int dropitem = rand() % 101, item_id;
			if (dropitem > 70) {
				gems_(NULL, world, rand() % 136 + 5, x, y - 16.0f, 3720);
				return;
			}
			else if (dropitem > 57) item_id = 3720;
			else if (dropitem > 30) item_id = 3721;
			else return;
			WorldDrop drop{};
			drop.id = item_id;
			drop.count = 1;
			drop.x = x;
			drop.y = y;
			auto name_ = world->name;
			auto it = std::find_if(worlds.begin(), worlds.end(), [=](const World& w) { return w.name == name_; });
			if (it != worlds.end()) VisualHandle::Drop(&*it, drop);
		};

		static auto worldEventLastTime = std::chrono::high_resolution_clock::now();
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = now - worldEventLastTime;
		worldEventLastTime = now;
		double deltaTime = elapsed.count();
		if (deltaTime <= 0.0) deltaTime = 0.1;
		if (deltaTime > 0.5)  deltaTime = 0.5;

		for (auto worldIndex = 0; worldIndex < (int)worlds.size(); worldIndex++) {
			if (!SafeWorldAccess(worlds[worldIndex].name)) continue;
			World* world = &worlds[worldIndex];

			{ // GHOSTS
				for (auto ghostIndex = 0; ghostIndex < (int)world->ghosts.size();) {
					WorldGhost* ghost = &world->ghosts[ghostIndex];
					bool shouldErase = false;

					if (ghost->type == 1 || ghost->type == 12) {
						if (ghost->distance > 0) {
							ghost->time += deltaTime;

							double vectorX = (ghost->new_pos[0] - ghost->last_pos[0]) / ghost->distance;
							double vectorY = (ghost->new_pos[1] - ghost->last_pos[1]) / ghost->distance;

							double displacementX = ghost->speed * ghost->time * vectorX;
							double displacementY = ghost->speed * ghost->time * vectorY;

							double currentX = ghost->last_pos[0] + displacementX;
							double currentY = ghost->last_pos[1] + displacementY;

							ghost->visual_pos[0] = currentX;
							ghost->visual_pos[1] = currentY;

							// Check hit with players in world
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == nullptr || pInfo(currentPeer)->world != world->name) continue;

								double distance = sqrt(pow(ghost->visual_pos[0] - pInfo(currentPeer)->x, 2) +
									pow(ghost->visual_pos[1] - pInfo(currentPeer)->y, 2));
								if (distance <= 32) {
									if (!Playmods::HasById(pInfo(currentPeer), 114)) {
										GUP::GameUpdatePacket particle{};
										particle.m_type = 17;
										particle.m_net_id = 195;
										particle.m_vec_x = (float)ghost->visual_pos[0];
										particle.m_vec_y = (float)ghost->visual_pos[1];
										particle.m_vec2_y = 195;
										GUP::send_packet(currentPeer, 4, &particle, sizeof(GUP::GameUpdatePacket), ENET_PACKET_FLAG_RELIABLE);

										vector<string> bubble{ "`2AIYEE! A ghost!``", "`2I've been slimed!``" };
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(currentPeer)->netID);
										p.Insert(bubble[rand() % bubble.size()]);
										p.Insert(0), p.Insert(0);
										p.CreatePacket(currentPeer);
									}
									Playmods::Add(currentPeer, 114);
									Clothing_V2::Update_Value(currentPeer);
									Clothing_V2::Update(currentPeer);
								}
							}

							// Check jar interaction
							for (auto JarIndex = 0; JarIndex < (int)world->ghosts.size(); JarIndex++) {
								WorldGhost* npc = &world->ghosts[JarIndex];
								if (npc->type == 2 && npc->time >= 2) {
									if (npc->state == 10) continue;
									double x_distance = ghost->visual_pos[0] - npc->last_pos[0];
									double y_distance = ghost->visual_pos[1] - npc->last_pos[1];

									if (std::abs(y_distance) <= 96 && std::abs(x_distance) <= 96) {
										ghost->last_pos[0] = ghost->visual_pos[0];
										ghost->last_pos[1] = ghost->visual_pos[1];
										ghost->new_pos[0] = npc->last_pos[0];
										ghost->new_pos[1] = npc->last_pos[1];
										ghost->speed = 100;
										ghost->action = 3;
										ghost->isCaught = 1;
										ghost->state = npc->id;
										ghost->distance = sqrt(pow(ghost->new_pos[0] - ghost->last_pos[0], 2) +
											pow(ghost->new_pos[1] - ghost->last_pos[1], 2));
										ghost->time = 0;
										ghost->maxTime = ghost->distance / ghost->speed;
										GHOST::updateGhost(world, *ghost);
									}
								}
							}
						}

						if (ghost->time >= ghost->maxTime) {
							if (ghost->state == 500) {
								ghost->state = 0;
							}

							if (ghost->isCaught == 1) {
								int jarIndex = -1;
								for (int j = 0; j < (int)world->ghosts.size(); j++) {
									if (world->ghosts[j].type == 2 && world->ghosts[j].id == ghost->state && world->ghosts[j].state != 10) {
										jarIndex = j;
										break;
									}
								}

								if (jarIndex != -1) {
									WorldGhost* npc = &world->ghosts[jarIndex];
									ghost->last_pos[0] = ghost->new_pos[0];
									ghost->last_pos[1] = ghost->new_pos[1];
									ghost->action = 4;
									ghost->type = npc->id;
									ghost->isCaught = 0;
									ghost->speed = 0;

									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == nullptr) continue;
										if (pInfo(currentPeer)->world != world->name) continue;
										if (pInfo(currentPeer)->netID == npc->distance) {
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(currentPeer)->netID);
											p.Insert("`3I caught a ghost!");
											p.Insert(0), p.Insert(1);
											p.CreatePacket(currentPeer);

											gamepacket_t p2;
											p2.Insert("OnConsoleMessage");
											p2.Insert("`3I caught a ghost!");
											p2.CreatePacket(currentPeer);

											if (Event()->Harvest_Festival) {
												if (rand() % 20 == 0) {
													WorldDrop drop_bundle{};
													drop_bundle.id = 3870, drop_bundle.count = 1,
													drop_bundle.uid = uint16_t(world->drop_new.size()) + 1,
													drop_bundle.x = (float)(npc->last_pos[0] + (rand() % 17 - 8)),
													drop_bundle.y = (float)(npc->last_pos[1] + (rand() % 17 - 8));
													VisualHandle::Drop(world, drop_bundle);
												}
											}

											int give = 1;
											Inventory::Modify(currentPeer, 3722, give);

											PlayerMoving data_{};
											data_.packetType = 19, data_.plantingTree = 0, data_.netID = 0;
											data_.punchX = 3722;
											data_.x = (float)npc->last_pos[0], data_.y = (float)npc->last_pos[1];
											int32_t to_netid = pInfo(currentPeer)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 5;
											memcpy(raw + 8, &to_netid, 4);
											for (ENetPeer* currentPeer2 = server->peers; currentPeer2 < &server->peers[server->peerCount]; ++currentPeer2) {
												if (currentPeer2->state != ENET_PEER_STATE_CONNECTED || currentPeer2->data == NULL) continue;
												if (pInfo(currentPeer2)->world != pInfo(currentPeer)->world) continue;
												send_raw(currentPeer2, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											}
											delete[] raw;
										}
									}

									sendJarDrop(world, (float)npc->last_pos[0], (float)npc->last_pos[1]);
									npc->state = 10;
									GHOST::updateGhost(world, *npc);
									ghost->state = 10;
									GHOST::updateGhost(world, *ghost);

									if (ghostIndex < jarIndex) {
										world->ghosts.erase(world->ghosts.begin() + jarIndex);
										world->ghosts.erase(world->ghosts.begin() + ghostIndex);
									}
									else if (ghostIndex > jarIndex) {
										world->ghosts.erase(world->ghosts.begin() + ghostIndex);
										world->ghosts.erase(world->ghosts.begin() + jarIndex);
										ghostIndex--;
									}
									else {
										world->ghosts.erase(world->ghosts.begin() + ghostIndex);
									}
									continue;
								}
								else {
									ghost->isCaught = 0;
									ghost->state = 0;
								}
							}

							double newX = retRandomValues(-180, 180);
							double newY = retRandomValues(-180, 180);
							if (std::abs(newX) < 64) newX = (newX < 0 ? -96 : 96);
							if (std::abs(newY) < 64) newY = (newY < 0 ? -96 : 96);

							ghost->last_pos[0] = ghost->visual_pos[0];
							ghost->last_pos[1] = ghost->visual_pos[1];

							double targetX = ghost->last_pos[0] + newX;
							double targetY = ghost->last_pos[1] + newY;
							double maxX = static_cast<double>(world->max_x) * 32.0;
							double maxY = static_cast<double>(world->max_y) * 32.0;

							if (targetX <= 64.0) targetX = ghost->last_pos[0] + retRandomValues(96, 200);
							else if (targetX >= maxX - 64.0) targetX = ghost->last_pos[0] - retRandomValues(96, 200);

							if (targetY <= 64.0) targetY = ghost->last_pos[1] + retRandomValues(96, 200);
							else if (targetY >= maxY - 64.0) targetY = ghost->last_pos[1] - retRandomValues(96, 200);

							ghost->new_pos[0] = std::clamp(targetX, 64.0, maxX - 64.0);
							ghost->new_pos[1] = std::clamp(targetY, 64.0, maxY - 64.0);
							ghost->speed = retRandomValues(33, 45);
							ghost->action = 3;
							ghost->state = 0;
							ghost->distance = sqrt(pow(ghost->new_pos[0] - ghost->last_pos[0], 2) +
								pow(ghost->new_pos[1] - ghost->last_pos[1], 2));
							ghost->time = 0;
							ghost->maxTime = ghost->distance > 0 ? (ghost->distance / ghost->speed) : 2.0;
							if (ghost->maxTime < 0.5) ghost->maxTime = 2.0;
							GHOST::updateGhost(world, *ghost);
						}
					}
					else if (ghost->type == 2) {
						ghost->time += deltaTime;
						if (ghost->time >= ghost->maxTime) {
							ghost->state = 10;
							shouldErase = true;
						}
					}

					if (shouldErase) {
						sendJarDrop(world, (float)ghost->last_pos[0], (float)ghost->last_pos[1]);
						ghost->state = 10;
						GHOST::updateGhost(world, *ghost);
						world->ghosts.erase(world->ghosts.begin() + ghostIndex);
					}
					else {
						ghostIndex++;
					}
				}
			}

			{ // GAME GENERATOR
				if (world->game.started) {
					world->game.timeStarted += deltaTime;

					if (world->game.timeStarted == deltaTime) {
						for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
							if (world->name == pInfo(currentPeer)->world) {
								SendRespawn(currentPeer, 1, 1, 1);
								break;
							}
						}
					}

					if (static_cast<int>(world->game.timeStarted) != static_cast<int>(world->game.timeStarted - 0.1)) {
						int timeLeft = world->game.time - static_cast<int>(world->game.timeStarted);
						vector<int> remainders = { 30, 10, 5, 4, 3, 2, 1 };
						gamepacket_t p;
						if (timeLeft % 60 == 0 && timeLeft > 0) {
							p.Insert("OnConsoleMessage");
							p.Insert(std::format("Game ends in `5{} minutes``!", timeLeft / 60));
						}
						for (auto rem : remainders) {
							if (timeLeft == rem) {
								p.Insert("OnConsoleMessage");
								p.Insert(std::format("Game ends in `5{} seconds``!", timeLeft));
							}
						}
						for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
							if (world->name == pInfo(currentPeer)->world) {
								p.CreatePacket(currentPeer);
								if (timeLeft <= 0) {
									world->game.manageBattle(currentPeer, END);
									SendRespawn(currentPeer, 1, 1, 1);
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	static void InitLoopWorld() {
		while (g_server_running.load()) {
			try {
				if (!server) {
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					continue;
				}
				if (server->peerCount <= 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					continue;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				{
					std::lock_guard<std::mutex> worldGuard(g_world_loop_mutex);
					Looping::Worlds();
					Looping::worldEvent();
				}
				PartyEvent::Tick();
				PthtAutoHarvest::tick_jobs();
			}
			catch (const std::exception& e) {
				std::cerr << "[InitLoopWorld] Exception: " << e.what() << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(300));
			}
			catch (...) {
				std::cerr << "[InitLoopWorld] Unknown exception" << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(300));
			}
		}
	}
};
void Detected() {
	while (g_server_running.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		if (!server || server->peerCount <= 0) continue;
	}
}
void RunGiveaway() {
	while (g_server_running.load()) {
		std::this_thread::sleep_for(std::chrono::minutes(1));
		if (!server || server->peerCount <= 0) continue;

		if (!giveaways.active) continue;

		time_t now = time(0);
		tm* ltm = gmtime(&now);
		int current_hour = (ltm->tm_hour + 7) % 24;

		if (current_hour == giveaways.giveaway_hour) {
			Logger::Info("SERVER", "Running giveaway for item: " + to_string(giveaways.item_id) + " Amount: " + to_string(giveaways.amount));
			std::lock_guard<std::mutex> guard(g_world_loop_mutex);
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL || pInfo(cp_)->tankIDName.empty())
					continue;

				if (Inventory::Modify(cp_, giveaways.item_id, giveaways.amount) == 0) {
					VisualHandle::Trade(cp_, giveaways.item_id, 500);
					VarList::OnTalkBubble(cp_, pInfo(cp_)->netID, "Succesfully added " + std::to_string(giveaways.amount) + "x Item ID " + std::to_string(giveaways.item_id) + " to your inventory.", 0, true);
					VarList::OnAddNotification(cp_, "[`6G`7I`8VE`2A`pW`8A`4Y] You received " + std::to_string(giveaways.amount) + "x Item ID " + std::to_string(giveaways.item_id) + "!" + "", "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
					VarList::OnConsoleMessage(cp_, "[`6G`7I`8VE`2A`pW`8A`4Y] You received " + std::to_string(giveaways.amount) + "x Item ID " + std::to_string(giveaways.item_id) + "!");
				}
				else {
					VarList::OnConsoleMessage(cp_, "[`6G`7I`8VE`2A`pW`8A`4Y] Failed to add item to your inventory.");
				}
			}

			giveaways.active = false;
		}
	}
}
// autofarmdelay: thread terpisah khusus autofarm.
// Shadow section TIDAK acquire g_shadow_mutex lagi (fix ABBA deadlock):
//   dulu: autofarm hold world_mutex → coba shadow_mutex
//         ShadowFarmTick hold shadow_mutex → akses worlds tanpa world_mutex
//   fix:  shadow section di autofarm sekarang protected by world_mutex saja (konsisten).
void autofarmdelay() {
	while (g_server_running.load()) {
		int delay = Environment()->AutoFarm_Delay;
		if (delay <= 0) delay = 1500;
		if (delay < 100) delay = 100;
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		if (!server || server->peerCount <= 0) continue;
		{
			std::lock_guard<std::mutex> guard(g_world_loop_mutex);
			Looping::autofarm();

			// Chat Spammer Tick
			long long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL || pInfo(cp_)->world.empty()) continue;
			}
		}
	}
}

class Login_Detect {
private:
	std::unordered_map<std::string, int> loginAttempts;
	chrono::steady_clock::time_point lastCheckTime;
public:
	Login_Detect() {
		lastCheckTime = chrono::steady_clock::now();
	}
	void LoginAttempt(ENetPeer* peer, std::string& ipAddress) {
		if (Environment()->Anti_Proxy) {
			if (CheckProxyAndVpn(ipAddress)) {
				Logger::Info("WARNING", "The IP address [" + ipAddress + "] is associated with a Proxy or Vpn.");
				SystemPool::FailedLogin(peer, "`4Sorry, The IP address [" + ipAddress + "] is associated with a Proxy or Vpn.");
				return;
			}
		}
		if (find(Environment()->Rid_Bans.begin(), Environment()->Rid_Bans.end(), ipAddress) != Environment()->Rid_Bans.end()) {
			SystemPool::FailedLogin(peer, "`4Sorry, this account (`5" + ipAddress + "`4) has been suspended. Visit `5" + Environment()->discord_url + "`` `4if you have any questions.", "`5Join Discord Server");
			return;
		}
		auto now = chrono::steady_clock::now();
		auto elapsedTime = chrono::duration_cast<chrono::seconds>(now - lastCheckTime).count();
		if (elapsedTime >= 60) {
			loginAttempts.clear();
			lastCheckTime = now;
		}
		loginAttempts[ipAddress]++;
		if (loginAttempts[ipAddress] > 5) {
			Logger::Info("WARNING", "Login spam attacks from ip addresses: " + ipAddress + "!");
			SystemPool::FailedLogin(peer, "`o[ANTI-SPAM] Please wait 5 seconds before logging on again.");
		}
	}
};

static Login_Detect g_LoginMonitor;

static inline double dist2(double ax, double ay, double bx, double by) {
	const double dx = ax - bx;
	const double dy = ay - by;
	return (dx * dx) + (dy * dy);
}
void gameTimerThread() {
	using namespace std::chrono;
	auto lastTime = high_resolution_clock::now();

	while (g_server_running.load()) {
		try {
			std::this_thread::sleep_for(milliseconds(100));

			auto nowTime = high_resolution_clock::now();
			lastTime = nowTime;
			constexpr double deltaTime = 0.1; // constant – do not change

			// FIX USE-AFTER-FREE: hold g_world_loop_mutex for the ENTIRE game timer tick.
			// Old code: release mutex after snapshot, then access peer->data outside lock.
			// Disconnect handler (now also inside this mutex) deletes peer->data.
			// Result: use-after-free → hang/crash. Holding the lock here is safe because
			// the lock is held only ~1ms per 100ms tick (game worlds with active games are rare).
			std::lock_guard<std::mutex> gameTimerLock(g_world_loop_mutex);

			for (size_t worldIndex = 0; worldIndex < worlds.size(); worldIndex++) {
				World* world = &worlds[worldIndex];
				if (!world->game.started) continue;

				world->game.timeStarted += deltaTime;

				// First tick after game start: force-respawn everyone to their spawn point
				if (world->game.timeStarted == deltaTime) {
					for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
						if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
						if (world->name != pInfo(cp)->world) continue;
						SendRespawn(cp, true, 1, true);
						break;
					}
				}

				// Every time the integer-second changes, check countdown + time limit
				if (static_cast<int>(world->game.timeStarted) !=
					static_cast<int>(world->game.timeStarted - deltaTime)) {

					int timeLeft = (int)world->game.time - static_cast<int>(world->game.timeStarted);

					// Build optional countdown message
					gamepacket_t p;
					bool hasMsg = false;

					if (timeLeft > 0 && timeLeft % 60 == 0) {
						p.Insert("OnConsoleMessage");
						p.Insert(std::format("Game ends in `5{} minutes``!", timeLeft / 60));
						hasMsg = true;
					}
					else {
						static const std::vector<int> remainders = { 30, 10, 5, 4, 3, 2, 1 };
						for (int rem : remainders) {
							if (timeLeft == rem) {
								p.Insert("OnConsoleMessage");
								p.Insert(std::format("Game ends in `5{} second{}``!", timeLeft, timeLeft == 1 ? "" : "s"));
								hasMsg = true;
								break;
							}
						}
					}

					// Broadcast to world and end game if time's up
					for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
						if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
						if (world->name != pInfo(cp)->world) continue;

						if (hasMsg) p.CreatePacket(cp);

						if (timeLeft <= 0) {
							world->game.manageBattle(cp, END);
							SendRespawn(cp, true, 1, true);
							break; // manageBattle(END) clears game.started, loop will skip next tick
						}
					}
				}
			}
		}
		catch (const std::bad_alloc&) {
			fputs("[gameTimerThread] bad_alloc: skipping tick\n", stderr);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		catch (...) {}
	}
}
int main(int argc, char** argv[]) {
	InstallCrashHandler();
	SetupFirewallAndPorts();
	json db = loadLiveDB();
	LoadFindItemBlacklist();
	LoadDropGachaIds();
	DialogHandle::LoadCustomSongs();
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	SetConsoleTitleA("Administrator: Nopy - Growtopia Private Server");
	srand(unsigned int(time(nullptr)));
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleOutputCP(CP_UTF8);
	DWORD mode; SMALL_RECT rect;
	rect.Left = 0, rect.Top = 0, rect.Right = 120, rect.Bottom = 40;
	SetConsoleWindowInfo(hConsole, TRUE, &rect);
	GetConsoleMode(hConsole, &mode);
	SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	/*
	const char* message = "Welcome to Growtopia Private Server 1.1 (C++), Development By LyChie.\n"
		"\n"
		"Based on GTPS3 Source.\n\n"
		"We are dedicated to providing a secure and reliable service tailored to your needs.\n"
		"Your trust and feedback inspire us to continuously improve and introduce new features.\n"
		"Should you encounter any issues or have suggestions, feel free to contact our support team.\n"
		"and look forward to serving you better.\n";
	const char* title = "Growtopia Private Server. - @LyChie";
	MessageBoxA(NULL, message, title, MB_OK | MB_ICONINFORMATION);
	*/
	//Loading_Screen(50);
	clearScreen();
	LoadBlastDesigner();
	LoadGhostPriceData();
	DialogHandle::LoadCustomSongs();
	ResetShopStock();
	PrintBanner();
	Logger::Info("INFO", "Starting up server, this might take a while...");
	Logger::Info("INFO", "Initializing server...");
#ifdef _WIN32
	BOOL ret = SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	// NOTE: signal registration untuk SIGSEGV/SIGABRT dilakukan di SetupCrashHandler() di bawah.
	// Jangan register ulang di sini karena akan ditimpa & Auto_Save tidak pernah dipanggil.
#elif defined(__linux__)
	signal(SIGINT, ConsoleHandler);
	signal(SIGTERM, ConsoleHandler);
#endif
	Environment()->Server_Port = 55000;
	Environment()->UpTime = time(nullptr);
	if (Enet_Server()->Initialize(Environment()->Server_Port) == -1) {
		Logger::Info("ERROR", "Failed to Initialize ENet, Terminating the Server...");
		return EXIT_FAILURE;
	}
	Logger::Info("INFO", "Loading items.dat...");
	ItemDataUtils::loadLinux(), ItemDataUtils::loadIOS();
	if (ItemManager()->Serialize() == -1) {
		Logger::Info("ERROR", "Failed to load items.dat, please make sure the file is on '/database'");
		return EXIT_FAILURE;
	}
	else Logger::Info("INFO", "Succesfully loaded Items.dat | Version: " + to_string(Environment()->ItemsDat_Version) + " | Hash: " + to_string(Environment()->ItemsDat_Hash) + " | Total Items: " + Set_Count(Environment()->ItemsDat_Count) + "");
	Logger::Info("INFO", "Enet Initialized succesfully");
	Logger::Info("INFO", "Loading server statistics...");
	EventPool::Load::All();
	EventPool::Load::SetExchange();
	EventPool::Load::DailyReward();
	DropEditV1_Init();
	LoadRamadhanActive();
	LoadSetRecipeJson();
	exchangeData.load();
	kitsConfig.load();
	Commands::LoadDonationData();
	SetConsum_Init();
	exn_load();
	Action::Action_Map(), Dialog_Return::Dialog(), Server_Pool::Events(), Commands::Command();
	Logger::Info("INFO", "Succesfully registered all actions, dialog return | Total Action: " + Set_Count(actionMap.size()) + " | Total Dialog Return: " + Set_Count(dialogMap.size()) + "");
	Logger::Info("INFO", "Succesfully registered all commands | Total Commands: " + Set_Count(commandMap.size()));
	Logger::Info("INFO", "All configuration has been succesfully loaded");
	
	// Performance optimization: reserve vector capacities
	Environment()->Another_Worlds.reserve(100);
	worlds.reserve(500);
	Environment()->Active_Worlds.reserve(100);
	Environment()->Active_World_List.reserve(2000);
	
	SetupCrashHandler();
	Logger::Info("INFO", "" + Environment()->server_name + " V" + Environment()->server_version + " - is now listening at " + Environment()->server_ip + ":" + to_string(Environment()->Server_Port) + " | Date: " + currentDateTime());

	// FIX DO LOGIN: validate server_ip isn't a private/internal address.
	// On DigitalOcean, forgetting to set the public IP in config.json causes OnSendToServer
	// to redirect clients to an unreachable address → stuck at "Connecting..." screen.
	{
		const std::string& sip = Environment()->server_ip;
		bool is_private = sip.empty()
			|| sip == "127.0.0.1"
			|| sip == "0.0.0.0"
			|| sip.substr(0, 3) == "10."
			|| sip.substr(0, 7) == "172.16."
			|| sip.substr(0, 8) == "192.168.";
		if (is_private) {
			Logger::Info("WARNING",
				"[SERVER_IP] '" + sip + "' looks like a private/internal IP! "
				"Clients on DigitalOcean/VPS will get stuck at login. "
				"Set your public IP in config.json -> SERVER -> IP");
		}
	}
	Environment()->Threads.emplace_back(std::thread(Detected));
	// autofarmdelay thread RESTORED — berjalan terpisah, interval sesuai AutoFarm_Delay config
	Environment()->Threads.emplace_back(std::thread(autofarmdelay));
	std::thread giveawayThread(RunGiveaway);
	giveawayThread.detach();
	Environment()->Threads.emplace_back(std::thread(ServerPool::SaveDatabase::All));
	std::thread logFlushThread(SystemPool::FlushLogBuffers);
	logFlushThread.detach();
	std::thread disconnectSaveThread([]() {
		while (g_server_running.load()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::deque<DisconnectSaveItem> snap;
			{
				std::lock_guard<std::mutex> lk(g_disconnect_save_queue.mtx);
				snap.swap(g_disconnect_save_queue.queue);
			}
			for (auto& item : snap) {
				if (item.name.empty() || item.json.empty()) continue;
				std::ofstream f("database/players/" + item.name + "_.json");
				if (f.is_open()) { f << item.json; f.close(); }
			}
		}
		});
	disconnectSaveThread.detach();
	std::thread disconnectWorldSaveThread([]() {
		while (g_server_running.load()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::deque<DisconnectSaveItem> snap;
			{
				std::lock_guard<std::mutex> lk(g_disconnect_world_save_queue.mtx);
				snap.swap(g_disconnect_world_save_queue.queue);
			}
			for (auto& item : snap) {
				if (item.name.empty() || item.json.empty()) continue;
				std::ofstream w_("database/worlds/" + item.name + "_.json");
				if (w_.is_open()) { w_ << item.json << std::endl; w_.close(); }
			}
		}
		});
	disconnectWorldSaveThread.detach();
	std::thread watchdogThread([]() {
		while (g_server_running.load()) {
			std::this_thread::sleep_for(std::chrono::seconds(10));
			long long now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
			long long last = g_enet_heartbeat.load();
			if (last > 0 && now - last > 10000) {
				Logger::Info("WATCHDOG", "ENet thread tidak ada aktivitas selama " +
					std::to_string((now - last) / 1000) + "s! Server mungkin stuck (exception loop/deadlock).");
				// Reset heartbeat supaya alert tidak spam
				g_enet_heartbeat.store(now);
			}
		}
		});
	watchdogThread.detach();
	Logger::Info("HTTP", "Built-in HTTP.h server disabled (using external GTPS HTTP http.exe).");
	Environment()->Threads.emplace_back(std::thread(SystemPool::ServerInputPlugin));
	// ghostThread removed: ghost logic integrated into Looping::worldEvent() inside InitLoopWorld()
	// Thread::LoopingThread - shutdown pipe + future mining reserve
	Thread::LoopingThread::InitPipe(Environment()->server_name);
	std::thread loopingThreadTrigger(Thread::LoopingThread::Trigger);
	loopingThreadTrigger.detach();
	thread gameTimerThreads_(gameTimerThread);
	if (gameTimerThreads_.joinable()) gameTimerThreads_.detach();
	ENetEvent Event_Pool;
	ENetEvent event;
	if (!server) {
		Logger::Info("INFO", "ENet server not initialized!Check enet_host_create.");
		return 1;
	}
	Logger::Info("INFO", "Server ready.");
	while (true) {
		if (!server) break;
		try {
			g_enet_heartbeat.store(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
			int events_processed = 0;
			while (enet_host_service(server, &Event_Pool, 5) > 0) { // FAST PACKET RESPONSIVENESS: 5ms timeout
				events_processed++;
				// FIX CPU: limit events per batch to prevent starving Looping::Worlds()
				if (events_processed >= 100) break;
				ENetPeer* peer = Event_Pool.peer;
				if (!Event_Pool.peer) continue;
				switch (Event_Pool.type) {
				case ENET_EVENT_TYPE_NONE: {
					break;
				}
				case ENET_EVENT_TYPE_CONNECT: {
					enet_peer_timeout(peer, 0, 30000, 45000);
					enet_peer_ping_interval(peer, 1000);

					char clientConnection[16];
					enet_address_get_host_ip(&peer->address, clientConnection, 16);

					peer->data = new Player;
					pInfo(peer)->last_movement_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

					send_(peer, 1, nullptr, 0);

					pInfo(peer)->ip = clientConnection;
					pInfo(peer)->id = peer->connectID;

					// :
					/*
					int count = 0;
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						if (cp_->address.host == peer->address.host) count++;
					}
					if (count > 3) {
						SystemPool::FailedLogin(peer, "4SERVER PROTECTION!o: Please disable proxy or GTBot, before connecting to 6" + Environment()->server_name + "o.``");
						break;
					}
					*/

					g_LoginMonitor.LoginAttempt(peer, pInfo(peer)->ip);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT: {
					if (!Event_Pool.peer->data) break;
					if (peer->data != NULL) {
						std::string json_data;
						std::string player_name;
						{
							std::lock_guard<std::mutex> worldLock(g_world_loop_mutex);
							if (pInfo(peer)->world != "") exit_(peer, false, true);
							if (pInfo(peer)->trading_with != -1) cancel_trade(peer, false);
							pInfo(peer)->in_enter_game = true;
							player_name = pInfo(peer)->tankIDName;
							if (!player_name.empty()) {
								Logger::Info("DISCONNECT", "Player disconnected: " + player_name);
							}
							ServerPool::SaveDatabase::Players(pInfo(peer), true, &json_data);
							delete Event_Pool.peer->data;
							Event_Pool.peer->data = NULL;
						}
						if (!json_data.empty() && !player_name.empty()) {
							std::lock_guard<std::mutex> qlk(g_disconnect_save_queue.mtx);
							g_disconnect_save_queue.queue.push_back({ player_name, std::move(json_data) });
						}
					}
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE: {
					if (peer->data == NULL) {
						if (Event_Pool.packet) enet_packet_destroy(Event_Pool.packet);
						continue;
					}
					pInfo(peer)->last_movement_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
					pInfo(peer)->is_afk_sleeping = false;
					if (peer->incomingDataTotal >= 25000) {
						VarList::OnConsoleMessage(peer, "`oWarning from `4System`o: Unusual packet detected");
						Peer_Disconnect(peer, 0);
					}
					if (Event_Pool.packet->dataLength < 0x8 || Event_Pool.packet->dataLength > 0x800) {
						enet_packet_destroy(Event_Pool.packet);
						break;
					}
					switch (GetMessageTypeFromPacket(Event_Pool.packet)) {
					case NET_MESSAGE_GENERIC_TEXT: {
						std::string cch = GetTextPointerFromPacket(Event_Pool.packet);
						SystemPool::SaveCchLogs(cch);

						if (!isASCII(cch)
							|| cch.find("action|getDRAnimations") != std::string::npos
							|| cch.find("action|refresh_player_tribute_data") != std::string::npos
							|| cch.find("action|get_seed_diary_data") != std::string::npos
							|| cch.find("action|survey") != std::string::npos
							|| cch.find("action|setFilter") != std::string::npos
							|| cch.find("action|complete_seeddiary_tutorial") != std::string::npos) {
							enet_packet_destroy(Event_Pool.packet);
							continue;
						}
						if (pInfo(peer)->packetSent.lastActionSent == 0) {
							pInfo(peer)->packetSent.lastActionSent = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() + 1000;
						}
						if (pInfo(peer)->packetSent.lastActionSent >= (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							pInfo(peer)->packetSent.totalActionSent++;
						}
						else {
							pInfo(peer)->packetSent.totalActionSent = 0, pInfo(peer)->packetSent.lastActionSent = 0;
						}
						if (pInfo(peer)->packetSent.totalActionSent >= 10) {
							pInfo(peer)->Account_Notes.push_back("`o" + currentDateTime() + ": >> Flagged for Spam (in " + pInfo(peer)->world + ") Type: (FLAGGED AS: SPAM PACKET) (Code 1)");
							VarList::OnConsoleMessage(peer, "`7Your client sending too many packets. attempt to reconnect");
							ServerPool::SaveDatabase::Players(pInfo(peer), true);
							Peer_Disconnect(peer, 0);
							{
								std::lock_guard<std::mutex> lk(g_world_loop_mutex);
								if (peer->data != NULL) {
									delete Event_Pool.peer->data;
									Event_Pool.peer->data = NULL;
								}
							}
							enet_packet_destroy(Event_Pool.packet);
							continue;
						}
						ServerLogsPkt.theCCH = currentDateTime() + " [NAME] " + pInfo(peer)->tankIDName + " [IP ADDRESS] " + pInfo(peer)->ip + " [EVENT TYPE] " + to_string(Event_Pool.type) + " [EVENT PACKET TYPE] " + to_string(GetMessageTypeFromPacket(Event_Pool.packet)) + " [TEXT POINTER] " + cch;
						//if (delayLoopWorld <= date_time::get_epoch_time<milliseconds>()) delayLoopWorld = date_time::get_epoch_time<milliseconds>() + 800, Looping::Worlds();
						if (not pInfo(peer)->Has_Login) {
							replaceAll(cch, "/p", "\n");
							SystemPool::PlayerLogin(peer, cch);
							break;
						}
						TextScanner parser(cch);
						std::string action = "";
						if (parser.try_get("action", action)) {
							if (actionMap.find(action) != actionMap.end()) {
								actionMap[action](peer, cch);
							}
							else {
								// FIX: Don't disconnect for unhandled actions — just log silently
								Logger::Info("WARNING", "Unhandle NET_MESSAGE_GENERIC_TEXT -> : [" + cch + "] Sent by " + pInfo(peer)->tankIDName);
							}
							break;
						}
						// FIX FALL-THROUGH: kalau "action|" tidak ada,
						// jangan jatuh ke NET_MESSAGE_GAME_MESSAGE (yang akan picu Peer_Reset palsu).
						break;
					}
					case NET_MESSAGE_GAME_MESSAGE: {
						if (pInfo(peer)->bypass == false) {
							pInfo(peer)->Account_Notes.push_back("`o" + currentDateTime() + ": >> Flagged for Hacking Type: (FLAGGED AS: PACKET BYPASS) (Code 2)");
							Peer_Reset(peer);
							break;
						}
						std::string cch = GetTextPointerFromPacket(Event_Pool.packet);
						ServerLogsPkt.packet_logs = currentDateTime() + " " + pInfo(peer)->tankIDName + " [" + pInfo(peer)->requestedName + "] ip [" + pInfo(peer)->ip + "] " + to_string(peer->address.host) + ": " + cch;
						if (pInfo(peer)->AccountSecured && pInfo(peer)->fa_ip != pInfo(peer)->ip or pInfo(peer)->tankIDName.empty() && explode("\n", cch)[0] == "action|join_request") {
							VarList::OnFailedToEnterWorld(peer);
							break;
						}
						TextScanner parser(cch);
						std::string action;
						if (parser.try_get("action", action)) {
							if (actionMap.find(action) != actionMap.end()) {
								actionMap[action](peer, cch);
							}
							else {
								// FIX: Don't disconnect for unhandled game message actions — just log
								Logger::Info("WARNING", "Unhandle NET_MESSAGE_GAME_MESSAGE -> [" + cch + "] Sent by " + pInfo(peer)->tankIDName);
							}
							break;
						}
						break;
					}
					case NET_MESSAGE_GAME_PACKET: {
						if (pInfo(peer)->tankIDName.empty() || pInfo(peer)->world.empty() || pInfo(peer)->world == "EXIT") break;
						BYTE* tankUpdatePacket = GetStructPointerFromTankPacket(Event_Pool.packet);
						if (tankUpdatePacket == nullptr) break;
						PlayerMoving pMov = unpackPlayerMoving(tankUpdatePacket);
						int tidown = 25, todc = 15;
						if (pInfo(peer)->lpps24 + tidown < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							pInfo(peer)->pps24 = 0;
							pInfo(peer)->lpps24 = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						}
						else {
							pInfo(peer)->pps24++;
							if (pInfo(peer)->pps24 >= 50) {
								if (pMov.packetType == 3 || pMov.packetType == 11 || pMov.packetType == 0) todc = 500;
								if (pInfo(peer)->pps24 >= todc) {
									pInfo(peer)->Account_Notes.push_back("`o" + currentDateTime() + ": >> Flagged for Spam (in " + pInfo(peer)->world + ") Type: (FLAGGED AS: SPAM PACKET) (Code 3)");
									VarList::OnConsoleMessage(peer, "`7Your client sending too many packets. attempt to reconnect");
									ServerPool::SaveDatabase::Players(pInfo(peer), true);
									Peer_Disconnect(peer, 0);
									// FIX USE-AFTER-FREE: delete inside mutex
									{
										std::lock_guard<std::mutex> lk(g_world_loop_mutex);
										if (peer->data != NULL) {
											delete Event_Pool.peer->data;
											Event_Pool.peer->data = NULL;
										}
									}
								}
								break;
							}
						}
						ServerLogsPkt.packet_type = "[PACKET TYPE]: " + to_string(pMov.packetType), ServerLogsPkt.last_world = pInfo(peer)->world;
						if (eventMap.find(pMov.packetType) != eventMap.end()) {
							eventMap[pMov.packetType](peer, &pMov);
						}
						else {
							const seconds cooldownDuration(5);
							auto now = steady_clock::now();
							if (duration_cast<seconds>(now - pInfo(peer)->lastPacketTime) < cooldownDuration) {
								Peer_Disconnect(peer, 0);
								break;
							}
							pInfo(peer)->lastPacketTime = now;
							Logger::Info("WARNING", "unhandled EVENT_TYPE_GAME_PACKET -> [" + to_string(pMov.packetType) + "] Sent by " + pInfo(peer)->tankIDName);
							SystemPool::SaveLogsUnhandle_Packet("EVENT_TYPE_GAME_PACKET -> [" + to_string(pMov.packetType) + "] Sent by " + pInfo(peer)->tankIDName);
						}
						break;
					}
					default:
						Logger::Info("WARNING", "Unhandled ENET_EVENT_TYPE_RECEIVE ->  [" + to_string(GetMessageTypeFromPacket(Event_Pool.packet)) + "] Sent by " + pInfo(peer)->tankIDName);
						break;
					}
					enet_packet_destroy(Event_Pool.packet);
					break;
				}
				default:
					if (peer->data != NULL) {
						Logger::Info("INFO", "Unhandle ENETEVENT -> [" + to_string(Event_Pool.type) + "] Sent by " + pInfo(peer)->tankIDName);
						SystemPool::SaveLogsUnhandle_Packet("ENETEVENT -> [" + to_string(Event_Pool.type) + "] Sent by " + pInfo(peer)->tankIDName);
					}
					break;
				}
				enet_host_flush(server);
			}

			{
				std::lock_guard<std::mutex> worldGuard(g_world_loop_mutex);
				PthtAutoHarvest::tick_jobs();

				long long loop_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
				if (g_looping_worlds_next - loop_now <= 0) {
					g_looping_worlds_next = loop_now + 100; // FAST WORLD TICK: 100ms interval for smooth gameplay
					// FIX DEADLOCK: main loop Worlds() must hold same mutex as disconnect handler
					// to prevent concurrent access to peer->data and world structures.
					Looping::Worlds();
					PartyEvent::Tick();

					// Check AFK sleep
					long long now_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
					static long long g_last_status_file_update = 0;
					if (now_ms - g_last_status_file_update >= 3000) {
						g_last_status_file_update = now_ms;
						try {
							DialogHandle::recalc_richest();
						} catch (...) {}
					}

					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL || pInfo(cp_)->world.empty() || pInfo(cp_)->world == "EXIT") continue;
						if (pInfo(cp_)->last_movement_time != 0 && !pInfo(cp_)->is_afk_sleeping && pInfo(cp_)->fishing_used == 0) {
							if (now_ms - pInfo(cp_)->last_movement_time >= 10000) {
								pInfo(cp_)->is_afk_sleeping = true;
								Commands::ON_ACTION(cp_, "/sleep");
							}
						}
					}
				} else {
					// FIX CPU: yield CPU when no events and no world tick needed
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}
		catch (const std::bad_alloc&) {
			fputs("[MainLoop] bad_alloc: out of memory, skipping iteration\n", stderr);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		catch (const std::exception& e) {
			Logger::Info("ERROR", std::string("[MainLoop] Exception tertangkap: ") + e.what());
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		catch (...) {
			Logger::Info("ERROR", "[MainLoop] Unknown exception tertangkap, server tetap jalan...");
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	RemoveFirewallRules();
	return EXIT_SUCCESS;
}