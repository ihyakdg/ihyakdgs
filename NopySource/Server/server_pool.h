#pragma once
#include <string>
#include "NopySource/Handle/WorldInfo.h"

using EventFunctionType = void(*)(ENetPeer*, PlayerMoving*);
std::map<int, EventFunctionType> eventMap;

class Server_Pool {
public:
	static void Events() {
		eventMap[0] = Server_Pool::PACKET_STATE;
		eventMap[3] = Server_Pool::PACKET_TILE_CHANGE_REQUEST;
		eventMap[7] = Server_Pool::PACKET_TILE_ACTIVATE_REQUEST;
		eventMap[10] = Server_Pool::PACKET_ITEM_ACTIVATE_REQUEST;
		eventMap[11] = Server_Pool::PACKET_ITEM_ACTIVATE_OBJECT_REQUEST;
		eventMap[18] = Server_Pool::PACKET_SET_ICON_STATE;
		eventMap[22] = Server_Pool::PACKET_PING_REQUEST;
		eventMap[23] = Server_Pool::PACKET_GOT_PUNCHED;
		eventMap[24] = Server_Pool::PACKET_APP_CHECK_RESPONSE;
		eventMap[25] = Server_Pool::PACKET_APP_INTEGRITY_FAIL;
		eventMap[76] = Server_Pool::PACKET_PROXY_CHECK_RESPONSE;
	}
private:
	static void PACKET_STATE(ENetPeer* peer, PlayerMoving* pMov) {
		int currentX = pInfo(peer)->x / 32, currentY = pInfo(peer)->y / 32, targetX = pMov->x / 32, targetY = pMov->y / 32;
		CL_Vec2f position = { static_cast<float>(pInfo(peer)->x), static_cast<float>(pInfo(peer)->y) };
		CL_Vec2i current_pos = { static_cast<int>((pInfo(peer)->x + 10) / 32), static_cast<int>((pInfo(peer)->y + 15) / 32) };
		CL_Vec2i future_pos = { static_cast<int>((pMov->x + 10) / 32), static_cast<int>((pMov->y + 15) / 32) };
		CL_Vec2i top_left = { static_cast<int>(pMov->x / 32), static_cast<int>(pMov->y / 32) };
		CL_Vec2i top_right = { static_cast<int>((pMov->x + 19) / 32), static_cast<int>(pMov->y / 32) };
		CL_Vec2i bottom_left = { static_cast<int>(pMov->x / 32), static_cast<int>((pMov->y + 29) / 32) };
		CL_Vec2i bottom_right = { static_cast<int>((pMov->x + 19) / 32), static_cast<int>((pMov->y + 29) / 32) };
		std::string name_ = pInfo(peer)->world;
		if (name_ != "") {
			if (!pInfo(peer)->isNpcTrading && pInfo(peer)->world == "TRADE") {
				int npcSpawnPosX = getNpcSpawnPos(NPC_EXCHANGE)[0];
				int distanceToNpcTips = abs(targetX - npcSpawnPosX / 32);
				if (distanceToNpcTips >= 1 && distanceToNpcTips <= 8) {
					NonPlayerCharacter::Talk(peer, NPC_EXCHANGE, "Hello " + pInfo(peer)->tankIDName + ", welcome to the goods exchange place! Just Wrench Me.");
					NonPlayerCharacter::State(peer, NPC_EXCHANGE, true);
					NonPlayerCharacter::Action(peer, NPC_EXCHANGE, "/wave");
					pInfo(peer)->isNpcTrading = true;
				}
			}
			if (!pInfo(peer)->isNpcTrading && pInfo(peer)->world == "MARKETPLACE") {
				int npcSpawnPosX = getNpcSpawnPos(NPC_MARKETPLACE)[0];
				int distanceToNpcTips = abs(targetX - npcSpawnPosX / 32);
				if (distanceToNpcTips >= 1 && distanceToNpcTips <= 8) {
					NonPlayerCharacter::Talk(peer, NPC_MARKETPLACE, "Hello " + pInfo(peer)->tankIDName + ", welcome to the goods MarketPlace! Just Wrench Me.");
					NonPlayerCharacter::State(peer, NPC_MARKETPLACE, (pInfo(peer)->x / 32 <= getNpcSpawnPos(NPC_MARKETPLACE)[0] / 32));
					NonPlayerCharacter::Action(peer, NPC_MARKETPLACE, "/wave");
					pInfo(peer)->isNpcTrading = true;
				}
			}
			if (pInfo(peer)->world.find("GROWMINES") != std::string::npos) {
				if (pInfo(peer)->hit1) {
					pInfo(peer)->hit1 = false;
					VarList::OnConsoleMessage(peer, "1HIT disabled.");
				}
				if (pInfo(peer)->ghost) {
					CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
					pInfo(peer)->ghost = false;
					VarList::OnConsoleMessage(peer, "`oYour body stops shimmering and returns to normal. (`$Ghost in the Shell `omod removed)``");
					VisualHandle::State(pInfo(peer));
					Clothing_V2::Update(peer);
				}
			}
			if (pInfo(peer)->world == "MINING" and not pInfo(peer)->Has_Read_2) {
				PlayerMoving data_{};
				data_.packetType = 37, data_.netID = 20206;
				BYTE* raw = packPlayerMoving(&data_);
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != pInfo(peer)->world) continue;
					send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
				}
				delete[] raw;
				pInfo(peer)->Has_Reads++;
				if (pInfo(peer)->Has_Reads >= 10) pInfo(peer)->Has_Read_2 = true;
			}
			World* world_ = PthtAutoHarvest::find_world(name_);
			if (world_ != nullptr) {
				if (world_->admin_event_active) {
					int64_t now_ms = currentTimestamp();
					if (now_ms - world_->admin_event_last_spawn_ms >= 1000) {
						world_->admin_event_last_spawn_ms = now_ms;
						static const int gacha_ids[] = { 10716, 3402, 14084, 5136, 14596, 7960, 9350 };
						int item_id = gacha_ids[rand() % (sizeof(gacha_ids) / sizeof(gacha_ids[0]))];
						if ((rand() % 1000) < 5) item_id = 15874;
						int tx = rand() % 100, ty = rand() % 60;
						for (int tries = 0; tries < 80; tries++) {
							int cx = rand() % 100, cy = rand() % 60;
							auto& b = world_->blocks[cx + (cy * 100)];
							if (b.fg == 0 && b.bg == 0) { tx = cx; ty = cy; break; }
						}
						int drop_x = tx * 32, drop_y = ty * 32;
						int net_id = 0;
						for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
							if (p->state != ENET_PEER_STATE_CONNECTED || !p->data) continue;
							if (pInfo(p)->world != world_->name) continue;
							net_id = pInfo(p)->netID;
							break;
						}
						BYTE* pkt = new BYTE[60];
						memset(pkt, 0, 60);
						pkt[0] = 4;
						*reinterpret_cast<int*>(pkt + 4) = 0x1;
						*reinterpret_cast<int*>(pkt + 8) = item_id;
						*reinterpret_cast<int*>(pkt + 12) = 1;
						*reinterpret_cast<int*>(pkt + 16) = drop_x;
						*reinterpret_cast<int*>(pkt + 20) = drop_y;
						*reinterpret_cast<int*>(pkt + 24) = net_id;
						for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
							if (p->state != ENET_PEER_STATE_CONNECTED || !p->data) continue;
							if (pInfo(p)->world != world_->name) continue;
							send_raw(p, 4, pkt, 56, ENET_PACKET_FLAG_RELIABLE);
							CAction::Effect_V2(p, 18, drop_x, drop_y);
							VarList::OnPlaySound(p, "audio/phoniex.wav");
							VarList::OnTextOverlay(p, "`6Admin Event:`` `wGacha dropped!``");
						}
						delete[] pkt;
						std::vector<int> raw{ item_id, 1, net_id, drop_x, drop_y, 0 };
						world_->drop_new.push_back(raw);
					}
					if (now_ms - world_->admin_event_last_weather_ms >= 5000) {
						world_->admin_event_last_weather_ms = now_ms;
						world_->weather = rand() % 22;
						for (ENetPeer* p = server->peers; p < &server->peers[server->peerCount]; ++p) {
							if (p->state != ENET_PEER_STATE_CONNECTED || !p->data) continue;
							if (pInfo(p)->world != world_->name) continue;
							VarList::OnSetCurrentWeather(p, world_->weather);
						}
					}
				}
				if (pInfo(peer)->x != -1 and pInfo(peer)->y != -1) {
					int x_ = (pInfo(peer)->state == 16 ? (int)pMov->x / 32 : round((double)pMov->x / 32));
					int y_ = (int)pMov->y / 32;
					if (x_ < 0 or x_ >= 100 or y_ < 0 or y_ >= 60) return;
					WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
					if (block_ != NULL) {
						if (block_->fg == 4740 && pInfo(peer)->adventure_begins) { // ADVENTURE'S END
							pInfo(peer)->adventure_begins = false;
							int time = pInfo(peer)->timerActive ? pInfo(peer)->totalTime - (pInfo(peer)->timers - date_time::get_epoch_time()) : date_time::get_epoch_time() - pInfo(peer)->timers;
							std::string textBubble = "", textConsole = "";
							if (pInfo(peer)->lives != 0) textBubble = "`7[" + pInfo(peer)->name_color + pInfo(peer)->tankIDName + " `wcompleted an adventure with " + to_string(pInfo(peer)->lives) + " lives left in " + player::algorithm::second_adventure(time) + "!`7]";
							else textBubble = "`7[" + pInfo(peer)->name_color + pInfo(peer)->tankIDName + " `wcompleted an adventure in " + player::algorithm::second_adventure(time) + "!`7]";
							if (pInfo(peer)->lives != 0) textConsole = "`7[" + pInfo(peer)->name_color + pInfo(peer)->tankIDName + " `ocompleted an adventure with " + to_string(pInfo(peer)->lives) + " lives left in " + player::algorithm::second_adventure(time) + "!`7]";
							else textConsole = "`7[" + pInfo(peer)->name_color + pInfo(peer)->tankIDName + " `ocompleted an adventure in " + player::algorithm::second_adventure(time) + "!`7]";
							if (pInfo(peer)->timerActive) {
								pInfo(peer)->timerActive = false;
								VarList::OnCountdownEnd(peer, pInfo(peer)->netID);
							}
							pInfo(peer)->timers = 0;
							if (pInfo(peer)->lives >= 1) {
								pInfo(peer)->lives = 0;
								VisualHandle::Nick(peer, NULL);
							}
							for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
								if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
								if (pInfo(cp_)->world == pInfo(peer)->world) {
									VarList::OnConsoleMessage(cp_, textConsole);
									VarList::OnTalkBubble(cp_, pInfo(peer)->netID, ":D");
									VarList::OnTalkBubble(cp_, pInfo(peer)->netID, textBubble);
								}
							}
						}
						if (block_->fg == 1792 and world_->name == "LEGENDARYMOUNTAIN") { // Legendary Orb
							int c_ = 1; bool success = false;
							if (Inventory::Contains(peer, 1794) == 0) {
								Inventory::Modify(peer, 1794, c_);
								success = true;
								if (success) {
									VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`9You have claimed a Legendary Orb!``", 0, 0);
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != pInfo(peer)->world) continue;
										CAction::Effect(cp_, 46, (float)pInfo(peer)->x + 16, (float)pInfo(peer)->y + 16);
									}
								}
							}
						}
						if (block_->fg == 3212 and world_->name == "GROWCH") { // Ice Heart
							pInfo(peer)->c_x = 0, pInfo(peer)->c_y = 0; bool success = false;
							int c_ = 1;
							if (Inventory::Contains(peer, 3210) == 0) {
								Inventory::Modify(peer, 3210, c_);
								success = true;
								if (success) {
									if (WinterFest.Active) DailyEvent_Task::WinterFest(peer, "6", 1);
									VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`wYou've found the Growch's `1Icy Heart!``", 0, 1);
								}
							}
						}
						if (world_->name == "WINTERCLASHPARKOUR" or world_->name == "SUMMERCLASHPARKOUR" or world_->name == "SPRINGCLASHPARKOUR") { // Finale Claim 1-9
							if (block_->fg == 8776 or block_->fg == 8778 or block_->fg == 8780 or block_->fg == 8782 or block_->fg == 8784 or block_->fg == 8786 or block_->fg == 8788 or block_->fg == 8790 or block_->fg == 8792) {
								if (Playmods::HasById(pInfo(peer), 163)) return;
								bool has_received = false;
								int randomIndex = 0, add = 1, id_prize = 0;
								std::vector<std::string> commonItems = {}, rareItems = {}, epicItems = {};
								if (block_->fg == 8776) { // 1
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "Extract-O-Snap","Growmoji Halo","2-Face","Scavenger Lord's Wheels","Sprite's Robe","Future Perfect Chip","Lyre" }, rareItems = { "Golden Midas Masterpants","Growmoji Nuke","Mage's Orb","Swirling Rainbow Eyes" }, epicItems = { "Tiger Spirit","Crafter's Cape","Supplier's Cape","Crown of Duplication" };
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Extract-O-Snap","Growmoji Terrified Face","Growmoji Peace Sign","Guild Steed","Surgical Tool Bag","Guild Night Stallion","Holographic Mannequin","World Architect" }, rareItems = { "Rainbow Hair","Growmoji Trollface","Smoog the Great Dragon","Amulet of Force" }, epicItems = { "Scepter of the Honor Guard","Squirrel King","Glove of the Giants","Bear Spirit" };
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Extract-O-Snap","Guild Show Horse","Guild Noble's Carriage","Junk Cannon","Surgical Tool Bag","Paintball Power Popper","Cardboard Sky Starter","Magic Magnet" }, rareItems = { "Forged Iron Skin","Country Tweed Waistcoat & Tie","Country Tweed Trousers","Condor Wings" }, epicItems = { "Eagle Spirit","Sturdy Crackfire Fighter","Ionic Pulse Cannon Tank","Flaming Boxing Gloves" };
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 98.0, rareRate = 1.5, epicRate = 0.5;
									if (dropChance <= commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (block_->fg == 8778) { // 2
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "Growmoji Halo","2-Face","Scavenger Lord's Wheels","Sprite's Robe","Future Perfect Chip","Lyre","Golden Midas Masterpants","Growmoji Nuke" }, rareItems = { "Mage's Orb","Swirling Rainbow Eyes"," Tiger Spirit","Crafter's Cape" }, epicItems = { "Supplier's Cape","Crown of Duplication","Mad Hatter" };
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Growmoji Terrified Face","Growmoji Peace Sign","Guild Steed","Surgical Tool Bag","Guild Night Stallion","Holographic Mannequin","World Architect","Growmoji Trollface" }, rareItems = { "Rainbow Hair","Smoog the Great Dragon","Amulet of Force","Scepter of the Honor Guard" }, epicItems = { "Squirrel King","Glove of the Giants","Bear Spirit","Intergalactic Warrior Hair" };
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Guild Show Horse","Guild Noble's Carriage","Junk Cannon","Surgical Tool Bag","Paintball Power Popper","Cardboard Sky Starter","Magic Magnet" }, rareItems = { "Forged Iron Skin","Country Tweed Waistcoat & Tie","Country Tweed Trousers","Condor Wings" }, epicItems = { "Eagle Spirit","Sturdy Crackfire Fighter","Ionic Pulse Cannon Tank","Flaming Boxing Gloves","Steampunk Wings" };
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 96.0, rareRate = 3.0, epicRate = 1.0;
									if (dropChance < commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (block_->fg == 8780) { // 3
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "2-Face","Scavenger Lord's Wheels","Sprite's Robe","Future Perfect Chip","Lyre","Golden Midas Masterpants","Golden Midas Masterpants","Growmoji Nuke" }, rareItems = { "Mage's Orb","Swirling Rainbow Eyes","Tiger Spirit","Crafter's Cape" }, epicItems = { "Supplier's Cape","Crown of Duplication","Mad Hatter","Extractor Dynamo" };
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Guild Steed","Surgical Tool Bag","Guild Night Stallion","Holographic Mannequin","World Architect","Growmoji Trollface","Rainbow Hair" }, rareItems = { "Smoog the Great Dragon","Amulet of Force","Scepter of the Honor Guard","Squirrel King" }, epicItems = { "Glove of the Giants","Bear Spirit","Intergalactic Warrior Hair","Extractor Dynamo" };
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Guild Noble's Carriage","Junk Cannon","Surgical Tool Bag","Paintball Power Popper","Cardboard Sky Starter","Magic Magnet" }, rareItems = { "Forged Iron Skin","Country Tweed Waistcoat & Tie","Country Tweed Trousers","Condor Wings","Eagle Spirit" }, epicItems = {"Sturdy Crackfire Fighter","Ionic Pulse Cannon Tank","Flaming Boxing Gloves","Steampunk Wings","Extractor Dynamo"};
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 93.0, rareRate = 4.5, epicRate = 2.0;
									if (dropChance < commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (block_->fg == 8782) { // 4
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "Scavenger Lord's Wheels","Sprite's Robe","Future Perfect Chip","Lyre","Golden Midas Masterpants","Growmoji Nuke" }, rareItems = { "Mage's Orb","Swirling Rainbow Eyes","Tiger Spirit","Crafter's Cape" }, epicItems = { "Supplier's Cape","Crown of Duplication","Mad Hatter","Extractor Dynamo" };
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Guild Steed","Surgical Tool Bag","Guild Night Stallion","Holographic Mannequin","World Architect","Growmoji Trollface" }, rareItems = { "Rainbow Hair","Smoog the Great Dragon","Amulet of Force","Scepter of the Honor Guard" }, epicItems = { "Squirrel King","Glove of the Giants","Bear Spirit","Intergalactic Warrior Hair","Extractor Dynamo" };
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Junk Cannon","Surgical Tool Bag","Paintball Power Popper","Cardboard Sky Starter","Magic Magnet" }, rareItems = { "Forged Iron Skin","Country Tweed Waistcoat & Tie","Country Tweed Trousers","Condor Wings","Eagle Spirit" }, epicItems = {"Sturdy Crackfire Fighter","Ionic Pulse Cannon Tank","Flaming Boxing Gloves","Steampunk Wings","Extractor Dynamo"};
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 89.5, rareRate = 7.5, epicRate = 3;
									if (dropChance <= commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (block_->fg == 8784) { // 5
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "Sprite's Robe","Future Perfect Chip","Lyre","Golden Midas Masterpants","Growmoji Nuke" }, rareItems = { "Mage's Orb","Swirling Rainbow Eyes","Tiger Spirit","Crafter's Cape" }, epicItems = { "Supplier's Cape","Crown of Duplication","Mad Hatter","Extractor Dynamo" };
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Surgical Tool Bag","Guild Night Stallion","Holographic Mannequin","World Architect","Growmoji Trollface" }, rareItems = { "Rainbow Hair","Smoog the Great Dragon","Amulet of Force","Scepter of the Honor Guard","Squirrel King" }, epicItems = {"Glove of the Giants","Bear Spirit","Intergalactic Warrior Hair","Extractor Dynamo","Aurora Wings"};
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Surgical Tool Bag","Paintball Power Popper","Cardboard Sky Starter","Magic Magnet" }, rareItems = { "Forged Iron Skin","Country Tweed Waistcoat & Tie","Country Tweed Trousers","Condor Wings","Eagle Spirit","Sturdy Crackfire Fighter" }, epicItems = { "Ionic Pulse Cannon Tank","Flaming Boxing Gloves","Steampunk Wings","Extractor Dynamo","War Hammers of Darkness" };
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 86.0, rareRate = 9.5, epicRate = 4.5;
									if (dropChance <= commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (block_->fg == 8786) { // 6
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "Sprite's Robe","Future Perfect Chip","Lyre","Golden Midas Masterpants","Growmoji Nuke","Mage's Orb" }, rareItems = { "Swirling Rainbow Eyes","Tiger Spirit","Crafter's Cape","Supplier's Cape" }, epicItems = { "Crown of Duplication","Mad Hatter","Extractor Dynamo","Robotic MK.IV Wings" };
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Guild Night Stallion","Holographic Mannequin","World Architect","Growmoji Trollface" }, rareItems = { "Rainbow Hair","Smoog the Great Dragon","Amulet of Force","Scepter of the Honor Guard","Squirrel King" }, epicItems = {"Glove of the Giants","Bear Spirit","Intergalactic Warrior Hair","Extractor Dynamo","Aurora Wings"};
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Paintball Power Popper","Cardboard Sky Starter","Magic Magnet","Forged Iron Skin" }, rareItems = { "Country Tweed Waistcoat & Tie","Country Tweed Trousers","Condor Wings","Eagle Spirit","Sturdy Crackfire Fighter" }, epicItems = {"Ionic Pulse Cannon Tank","Flaming Boxing Gloves","Steampunk Wings","Extractor Dynamo","War Hammers of Darkness"};
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 81.5, rareRate = 12.0, epicRate = 6.5;
									if (dropChance <= commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (block_->fg == 8788) { // 7
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "Sprite's Robe","Future Perfect Chip","Lyre","Golden Midas Masterpants","Growmoji Nuke","Mage's Orb"," Swirling Rainbow Eyes" }, rareItems = { "Tiger Spirit","Crafter's Cape","Supplier's Cape","Crown of Duplication","Mad Hatter" }, epicItems = { "Extractor Dynamo","Robotic MK.IV Wings","Dual Crescent Blade","Storm Breaker","The Great Chameleon: Master of Disguise" };
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Guild Night Stallion","Holographic Mannequin","World Architect","Growmoji Trollface","Rainbow Hair" }, rareItems = { "Smoog the Great Dragon","Amulet of Force","Scepter of the Honor Guard","Squirrel King","Glove of the Giants" }, epicItems = {"Bear Spirit","Intergalactic Warrior Hair","Extractor Dynamo","Aurora Wings","Dual Crescent Blade","Pet Royal Guard"};
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Paintball Power Popper","Cardboard Sky Starter","Magic Magnet","Forged Iron Skin" }, rareItems = { "Country Tweed Waistcoat & Tie","Country Tweed Trousers","Condor Wings","Eagle Spirit","Sturdy Crackfire Fighter","Ionic Pulse Cannon Tank" }, epicItems = { "Flaming Boxing Gloves","Steampunk Wings","Extractor Dynamo","War Hammers of Darkness","Dual Crescent Blade","Storm Breaker" };
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 76.5, rareRate = 15.5, epicRate = 8.0;
									if (dropChance <= commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (block_->fg == 8790) { // 8
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "Future Perfect Chip","Lyre","Golden Midas Masterpants","Growmoji Nuke","Mage's Orb","Swirling Rainbow Eyes" }, rareItems = { "Tiger Spirit","Crafter's Cape","Supplier's Cape","Crown of Duplication","Mad Hatter" }, epicItems = {"Extractor Dynamo","Robotic MK.IV Wings","Dual Crescent Blade","Storm Breaker","The Great Chameleon: Master of Disguise"};
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Rainbow Hair","Holographic Mannequin","World Architect","Growmoji Trollface" }, rareItems = { "Smoog the Great Dragon","Amulet of Force","Scepter of the Honor Guard","Squirrel King","Glove of the Giants","Bear Spirit" }, epicItems = {"Intergalactic Warrior Hair","Extractor Dynamo","Aurora Wings","Pet Royal Guard","Flaming Aura","Dual Crescent Blade"};
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Cardboard Sky Starter","Magic Magnet","Forged Iron Skin" }, rareItems = { "Country Tweed Waistcoat & Tie","Country Tweed Trousers","Condor Wings","Eagle Spirit","Sturdy Crackfire Fighter","Ionic Pulse Cannon Tank" }, epicItems = { "Flaming Boxing Gloves","Steampunk Wings","Extractor Dynamo","War Hammers of Darkness","Dual Crescent Blade","Storm Breaker","Purple Aura" };
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 72.5, rareRate = 18.0, epicRate = 10.5;
									if (dropChance <= commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (block_->fg == 8792) { // 9
									std::srand(static_cast<unsigned int>(std::time(0)));
									if (world_->name == "WINTERCLASHPARKOUR") commonItems = { "Future Perfect Chip","Lyre","Golden Midas Masterpants","Growmoji Nuke","Mage's Orb","Swirling Rainbow Eyes","Tiger Spirit","Crafter's Cape" }, rareItems = { "Supplier's Cape","Crown of Duplication","Mad Hatter","Extractor Dynamo","Robotic MK.IV Wings" }, epicItems = {"Dual Crescent Blade","Storm Breaker","The Great Chameleon: Master of Disguise","Jack's Hat","Sun Blade"};
									if (world_->name == "SUMMERCLASHPARKOUR") commonItems = { "Rainbow Hair","Holographic Mannequin","World Architect","Growmoji Trollface" }, rareItems = { "Smoog the Great Dragon","Amulet of Force","Scepter of the Honor Guard","Squirrel King","Glove of the Giants","Bear Spirit" }, epicItems = {"Intergalactic Warrior Hair","Extractor Dynamo","Aurora Wings","Flaming Aura","Dancemaster's Crown","Sun Blade","Dual Crescent Blade","Pet Royal Guard"};
									if (world_->name == "SPRINGCLASHPARKOUR") commonItems = { "Cardboard Sky Starter","Magic Magnet","Forged Iron Skin","Country Tweed Waistcoat & Tie" }, rareItems = { "Country Tweed Trousers","Condor Wings","Eagle Spirit","Sturdy Crackfire Fighter","Ionic Pulse Cannon Tank","laming Boxing Gloves" }, epicItems = { "Steampunk Wings","Extractor Dynamo","War Hammers of Darkness","Dual Crescent Blade","Storm Breaker","Purple Aura","Green Aura","Sun Blade"};
									double dropChance = static_cast<double>(rand()) / RAND_MAX * 100.0;
									const double commonRate = 64.0, rareRate = 22.0, epicRate = 14.0;
									if (dropChance <= commonRate) {
										randomIndex = std::rand() % commonItems.size(), id_prize = GetItemIDByName(commonItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else if (dropChance < commonRate + rareRate) {
										randomIndex = std::rand() % rareItems.size(), id_prize = GetItemIDByName(rareItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									else {
										randomIndex = std::rand() % epicItems.size(), id_prize = GetItemIDByName(epicItems[randomIndex]);
										VarList::OnConsoleMessage(peer, "`4You received 1 " + items[id_prize].name + ".");
										VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You received 1 " + items[id_prize].name + ".", 0, 1);
									}
									if (Inventory::Modify(peer, id_prize, add) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = id_prize, drop_block_.count = 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										VisualHandle::Drop(world_, drop_block_);
									}
									has_received = true;
								}
								if (has_received) {
									for (int i_ = 0; i_ < pInfo(peer)->playmods.size(); i_++) {
										if (pInfo(peer)->playmods[i_].id == 162) {
											CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
											VarList::OnConsoleMessage(peer, info_about_playmods[pInfo(peer)->playmods[i_].id - 1][5] + " (`$" + info_about_playmods[pInfo(peer)->playmods[i_].id - 1][3] + "`` mod removed)");
											pInfo(peer)->playmods.erase(pInfo(peer)->playmods.begin() + i_);
											Clothing_V2::Update_Value(peer);
											Clothing_V2::Update(peer);
										}
									}
									Playmods::Add(peer, 163);
									VarList::OnCountdownEnd(peer, pInfo(peer)->netID);
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
										if (pInfo(cp_)->world == pInfo(peer)->world) {
											CAction::Effect(cp_, 46, (float)pInfo(peer)->x + 16, (float)pInfo(peer)->y + 16);
										}
									}
								}
							}
						}
						if (not pInfo(peer)->ghost) {
							if (pMov->x < 0 or pMov->y < 0 or pInfo(peer)->x == 0 && pInfo(peer)->y == 0) {
								int found_door = 0;
								vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [&](const WorldBlock& a) { return a.fg == 6; });
								if (p != world_->blocks.end()) {
									found_door = p - world_->blocks.begin();
									VarList::OnSetPos(peer, found_door % 100 * 32, found_door / 100 * 32, 0);
								}
								return;
							}
							if (items[block_->fg].collisionType == 1) {
								pInfo(peer)->x = position.x, pInfo(peer)->y = position.y;
								VarList::OnSetPos(peer, static_cast<float>(pInfo(peer)->x), static_cast<float>(pInfo(peer)->y));
								return;
							}
							if (pInfo(peer)->c_x * 32 != (int)pMov->x and pInfo(peer)->c_y * 32 != (int)pMov->y) {
								bool NoClip = patchNoClip(world_, pInfo(peer)->x, pInfo(peer)->y, block_, peer);
								if (NoClip) {
									pInfo(peer)->x = position.x, pInfo(peer)->y = position.y;
									VarList::OnSetPos(peer, static_cast<float>(pInfo(peer)->x), static_cast<float>(pInfo(peer)->y));
									return;
								}
							}
						}
						if (IsObstacle(world_, pInfo(peer), top_left) && IsObstacle(world_, pInfo(peer), top_right) && IsObstacle(world_, pInfo(peer), bottom_left) && IsObstacle(world_, pInfo(peer), bottom_right)) {
							int found_door = 0;
							vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [&](const WorldBlock& a) { return a.fg == 6; });
							if (p != world_->blocks.end()) {
								found_door = p - world_->blocks.begin();
								VarList::OnSetPos(peer, found_door % 100 * 32, found_door / 100 * 32, 0);
							}
							return;
						}
						else if (IsObstacle(world_, pInfo(peer), top_left) || IsObstacle(world_, pInfo(peer), top_right) || IsObstacle(world_, pInfo(peer), bottom_left) || IsObstacle(world_, pInfo(peer), bottom_right)) {
							int found_door = 0;
							vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [&](const WorldBlock& a) { return a.fg == 6; });
							if (p != world_->blocks.end()) {
								found_door = p - world_->blocks.begin();
								VarList::OnSetPos(peer, found_door % 100 * 32, found_door / 100 * 32, 0);
							}
							return;
						}
					}
				}
			}
		}
		if (pInfo(peer)->update) {
			if (not pInfo(peer)->world.empty() && pMov->characterState == 4) {
				if (pInfo(peer)->x > 0 && pInfo(peer)->y > 0) pInfo(peer)->update = false;
				if (pInfo(peer)->in_world_tutorial and pInfo(peer)->Tutorial_Number == 1 and not pInfo(peer)->completed_tutorial) {
					Tutorial::FTUESet(peer, 1);
					Tutorial::HideMenu(peer, 1);
					Tutorial::FTUEPopup(peer, "Welcome to GrowRabid!", "Hello " + pInfo(peer)->tankIDName + "! I am Crazy Jim and I will run you through the basics before letting you create anything you want!");
					Tutorial::ShowDialog(peer, "Click here and open special instructions for you!");
					Tutorial::Arrow(peer, "FtueTaskButton");
					Tutorial::ClearArrow(peer, "arrow_ui"), Tutorial::ClearArrow(peer, "arrow_ui_extra"), Tutorial::ClearArrow(peer, "arrow_world");
					Tutorial::OnResetInv(peer);
					Tutorial::OnShowInv(peer, 0);
					Tutorial::AutoSetChat(peer);
				}
				if (pInfo(peer)->world == "MINING" and not pInfo(peer)->Has_Read_) pInfo(peer)->Has_Read_ = true;
				if (pInfo(peer)->world == "GROWCH" and not pInfo(peer)->Has_Reads_ and WinterFest.Heart_Size != 0 and WinterFest.Active) {
					pInfo(peer)->Has_Reads_ = true;
					std::string banner = "", list_item = "";
					if (WinterFest.Heart_Size == 1) banner = "wtr_lvl1_salkjd.rttex", list_item = "\nadd_label_with_icon|small|`wIcicles|left|2226|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Snowy Rocks Seed with a Death Spikes Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wTangyuan|left|1374|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Water Bucket Seed with a Wheat Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wElf Hat|left|1380|\nadd_smalltext|Dropped from combining a Transmog Crystal, a Santa Hat, and 20 Green Blocks in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSnow Crust Block|left|7444|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Snowy Rocks Seed with an Ice Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wPartridge in a Pear Tree|left|11518|\nadd_smalltext|Harvesting a Giving Tree at 45-49%.|left|";
					if (WinterFest.Heart_Size == 2) banner = "wtr_lvl2_wjcxuy.rttex", list_item = "\nadd_label_with_icon|small|`wCandy Cane Block|left|1358|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a White Block Seed with a Red Wood Wall Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wReindeer Bell|left|2222|\nadd_smalltext|Dropped from breaking Snowy Rocks.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wLumber Axe|left|3206|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Dark Brown Block Seed with a Dark Grey Block Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSnowy Rocks Platform|left|7452|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Snowy Rocks Seed with a Snow Crust Block Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wGingerbread Man Mask|left|10522|\nadd_smalltext|Dropped from combining a Space Opera Mask, 50 Cherry Jelly Blocks, and 100 Gingerbread Blocks in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wWinterFest Garland|left|14110|\nadd_smalltext|This item's recipe is not yet found or public.|left|";
					if (WinterFest.Heart_Size == 3) banner = "wtr_lvl3_ncuewb.rttex", list_item = "\nadd_label_with_icon|small|`wSnowball|left|1368|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing an Ice Seed with a Water Bucket Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wYeti Hat|left|2234|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Twisted Spikes Seed with an Ice Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wFestivus Pole|left|1370|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Checkpoint Seed with a Wooden Platform Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wAbominable Snowman Suit|left|7408|\nadd_smalltext|Dropped from combining 50 Snow Blocks, 100 Small Icicles, and 25 Snowflake Blocks in any Chemical Combiner with 7 Icy Igloos placed surrounding the combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wAbominable Snowman Mask|left|7410|\nadd_smalltext|Dropped from combining 50 Snow Blocks, 100 Small Icicles, and 20 Winter Winds in any Chemical Combiner with 7 Icy Igloos placed surrounding the combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wLast Christmas Cut|left|13020|\nadd_smalltext|Obtainable by catching a Ghost of Winterfest Past using a Ghost Jar or Ghost Trap.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wGhost of WinterFest Present Robe|left|14102|\nadd_smalltext|Dropped from combining 20 Winter Gifts, 1 Wizard's Robe, and 1 Ghost-In-A-Jar in any Chemical Combiner.|left|";
					if (WinterFest.Heart_Size == 4) banner = "wtr_lvl4_wucnea.rttex", list_item = "\nadd_label_with_icon|small|`wbuckskin Pants|left|1392|left|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Cave Entrance Seed with a Brown Block Seed.|left|\nadd_label_with_icon|small|`wGiant Candle|left|1372|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Climbing Vine Seed with a Torch Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSnow Husky|left|7430|\nadd_smalltext|Purchased from the Store for 25,000 gems.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wWinterfest Checkpoint|left|9198|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing an Ice Seed with a Checkpoint Seed.|left|";
					if (WinterFest.Heart_Size == 5) banner = "wtr_lvl5_suenfa.rttex", list_item = "\nadd_label_with_icon|small|`wBuckskin Jacket|left|1388|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Cave Entrance Seed with a Barrel Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wWinter Flu Vaccine|left|2240|\nadd_smalltext|Obtained from successfully performing a surgery.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wCandy Cane Fence|left|1362|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Wrought-Iron Fence Seed with a Sugar Cane Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wPumpkin Pie|left|7464|\nadd_smalltext|Obtained from cooking.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wEggnog|left|7466|\nadd_smalltext|Obtained from cooking.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wZamgrowni|left|10504|\nadd_smalltext|Dropped from combining a Dear John Tractor, 200 Ice, and 200 Snowy Rocks in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wDinosaur in Ice Block|left|11490|\nadd_smalltext|Dropped from breaking Ice.|left|";
					if (WinterFest.Heart_Size == 6) banner = "wtr_lvl6_kjhgrd.rttex", list_item = "\nadd_label_with_icon|small|`wAntler Hat|left|1382|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Cuzco Wall Mount Seed with a Das Red Balloon Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wBuckskin Hood|left|1386|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Cave Entrance Seed with a Rock Platform Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSnowmobile|left|2230|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Mega Rock Speaker Seed with an Orange Stuff Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wIce Sculptures|left|9200|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing an Ice Seed with a Glass Block Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wChocolate Checkpoint|left|12940|\nadd_smalltext|Harvesting a Giving Tree at 35-39%.|left|";
					if (WinterFest.Heart_Size == 7) banner = "wtr_lvl7_qucnem.rttex", list_item = "\nadd_label_with_icon|small|`wGlowing Nose|left|1384|\nadd_smalltext|Dropped from combining 12 Chemical B, 10 Chemical R, and 4 Chemical Y in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wCandy Cane Platform|left|2232|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing an Rock Platform Seed with a Sugar Cane Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSock On The Wall|left|3256|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Texas Limestone Seed with a Red Royal Wallpaper Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wCandy Cane Streetlamp|left|7450|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Sugar Cane Seed with a Streetlamp Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wDancing Snow Globe|left|11512|\nadd_smalltext|Dropped from combining 10 Snowballs, a Fishbowl, and a Hula Bobblehead in any Chemical Combiner.|left|";
					if (WinterFest.Heart_Size == 8) banner = "wtr_lvl8_mewhvs.rttex", list_item = "\nadd_label_with_icon|small|`wHoliday Cheer Bomb|left|2236|\nadd_smalltext|Dropped from combining 5 Mysterious Chemical, 3 Chemical R, and 4 Chemical B in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wParka Hood - Orange|left|5442|\nadd_smalltext|Dropped from combining a Parka Hood - Blue, 20 Orange Blocks, and a Transmog Crystal in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wIce Dragon Hand|left|1378|\nadd_smalltext|Dropped from combining a Dragon Hand, 20 Snowballs, and a Transmog Crystal in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wParka Hood - Blue|left|5444|\nadd_smalltext|Dropped from combining a Parka Hood - Orange, 20 Blue Blocks, and a Transmog Crystal in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSiberian Tiger Block|left|2228|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Tiger Block Seed with a Snowy Rocks Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wPuffy Blue Jacket|left|5446|\nadd_smalltext|Dropped from combining a Puffy Orange Jacket, 20 Blue Blocks, and a Transmog Crystal in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wCloak of Fire and Ice|left|9190|\nadd_smalltext|Obtained by consming a Deluxe Winterfest Cracker with 5 Winterfest Cracker in the player's inventory.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wMalanka Mask|left|12924|\nadd_smalltext|Obtained by successfully completing a Star Voyage in the Galactic Nexus sector.|left|";
					if (WinterFest.Heart_Size == 9) banner = "wtr_lvl9_krdvbn.rttex", list_item = "\nadd_label_with_icon|small|`wYeonnalligi|left|1376|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Blue Portal Seed with an Orange Portal Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wMagic Reindeer Bell|left|2224|\nadd_smalltext|Dropped from combining a Transmog Crystal, a Reindeer Bell, and 20 Glowing Noses in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wRiding Winter Wolf|left|3250|\nadd_smalltext|Obtained from completing Wolfworld.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wArmored WinterBot - Feet|left|9174|\nadd_smalltext|Dropped from combining 200 Ice, a Snowshoes, and 100 Shield Fragments in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wIce Shard Hair|left|11488|\nadd_smalltext|Obtained from successfully performing a surgery on someone with a Gem Cuts malady.|left|";
					if (WinterFest.Heart_Size == 10) banner = "wtr_lvl10_dleups.rttex", list_item = "\nadd_label_with_icon|small|`wHoliday Gift Box|left|1390|\nadd_smalltext|Dropped from harvesting its tree, which is made by splicing a Treasure Chest Seed with a Slot Machine Seed.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wMighty Snow Rod|left|2218|\nadd_smalltext|Purchased from the Store for 30 Growtokens.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wGingerbread Man|left|5414|\nadd_smalltext|Obtained from cooking a Gingerbread Cookie at 95% or above.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSnow Leopard Shawl|left|10438|\nadd_smalltext|Purchased from the Store for 200 Growtokens.|left|";
					if (WinterFest.Heart_Size == 11) banner = "wtr_lvl11_xualqu.rttex", list_item = "\nadd_label_with_icon|small|`wSnow Leopard Tail|left|7416|\nadd_smalltext|Obtained from completing Wolfworld.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSnow Husky Sledge|left|7432|\nadd_smalltext|Dropped from combining 4 Snow Huskies, 10 Snowshoes, and 2 Icy Axe Blades in any Chemical Combiner.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wArmored WinterBot - Fists|left|9176|\nadd_smalltext|Purchased from the Store for 300,000 gems.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wAura Antlers|left|11466|\nadd_smalltext|Consume a Special Winter Wish.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSnow Monarch's Mantle|left|12892|\nadd_smalltext|Consume a Special Winter Wish.|left|\nadd_spacer|small|\nadd_label_with_icon|small|`wSnowglobe Staff|left|14092|\nadd_smalltext|Consume a Special Winter Wish.|left|";
					VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_image_button||interface/large/" + banner + "|bannerlayout|||\nadd_spacer|small|" + list_item + "|\nadd_spacer|small|\nadd_custom_margin|x:250;y:0|\nadd_custom_button||image:interface/tutorial/taptocontinue.rttex;image_size:382,53;width:0.35;min_width:100;|", 1000);
				}
				if (pInfo(peer)->world == "TRADE") NonPlayerCharacter::Cloth(peer, NPC_EXCHANGE);
				if (pInfo(peer)->world == "MARKETPLACE") NonPlayerCharacter::Cloth(peer, NPC_MARKETPLACE);
				Clothing_V2::Update(peer);
			}
		}
		if (pMov->characterState == 268435472 || float(pMov->characterState) == 80 || float(pMov->characterState) == 6.71091e+07 || float(pMov->characterState) == 6.71113e+07 || pMov->characterState == 268435488 || pMov->characterState == 262208 || pMov->characterState == 327680 || pMov->characterState == 673712008 || pMov->characterState == 67371216 || pMov->characterState == 268435504 || pMov->characterState == 268435616 || pMov->characterState == 268435632 || pMov->characterState == 268435456 || pMov->characterState == 224 || pMov->characterState == 112 || pMov->characterState == 80 || pMov->characterState == 96 || pMov->characterState == 224 || pMov->characterState == 65584 || pMov->characterState == 65712 || pMov->characterState == 65696 || pMov->characterState == 65536 || pMov->characterState == 65552 || pMov->characterState == 65568 || pMov->characterState == 65680 || pMov->characterState == 192 || pMov->characterState == 65664 || pMov->characterState == 65600 || pMov->characterState == 67860 || pMov->characterState == 64) {
			if (pInfo(peer)->lava_time + 5000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
				pInfo(peer)->lavaeffect = 0;
				pInfo(peer)->lava_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			}
			else {
				if (pInfo(peer)->xenonite & SETTINGS::XENONITE_FORCE_HEAT_RESIST || pInfo(peer)->xenonite & SETTINGS::XENONITE_BLOCK_HEAT_RESIST || (pInfo(peer)->cheater_settings & SETTINGS::SETTINGS_8 && pInfo(peer)->disable_cheater == 0)) {
					if (pInfo(peer)->lavaeffect >= (pInfo(peer)->xenonite & SETTINGS::XENONITE_FORCE_HEAT_RESIST or (pInfo(peer)->cheater_settings & SETTINGS::SETTINGS_8 && pInfo(peer)->disable_cheater == 0) ? 7 : 3)) {
						pInfo(peer)->lavaeffect = 0;
						Player_Respawn(peer, false, 0, true);
					}
					else pInfo(peer)->lavaeffect++;
				}
				else {
					if (pInfo(peer)->lavaeffect >= (pInfo(peer)->feet == 250 || pInfo(peer)->necklace == 5426 || (pInfo(peer)->mask == 5712 && pInfo(peer)->wild == 6) ? 7 : 3)) {
						pInfo(peer)->lavaeffect = 0;
						Player_Respawn(peer, false, 0, true);
					}
					else pInfo(peer)->lavaeffect++;
				}
			}
		}
		if (pInfo(peer)->fishing_used != 0) {
			if (pInfo(peer)->f_xy != pInfo(peer)->x + pInfo(peer)->y) pInfo(peer)->move_warning++;
			if (pInfo(peer)->move_warning > 0) stop_fishing(peer, true, "Sit still if you wanna fish!");
			if (pMov->punchX > 0 && pMov->punchY > 0) {
				pInfo(peer)->punch_warning++;
				if (pInfo(peer)->punch_warning >= 2) stop_fishing(peer, false, "");
			}
		}
		if (pInfo(peer)->hand == 2286 or pInfo(peer)->hand == 2560) {
			if (rand() % 100 < 4) {
				pInfo(peer)->geiger_++;
				int give_Back = items[pInfo(peer)->hand].geiger_give_back;
				if (pInfo(peer)->geiger_ >= 100) {
					int c_ = -1, c_2 = 1;
					Inventory::Modify(peer, pInfo(peer)->hand, c_);
					Inventory::Modify(peer, give_Back, c_2);
					pInfo(peer)->hand = give_Back;
					pInfo(peer)->geiger_ = 0;
					VarList::OnConsoleMessage(peer, "You are detecting radiation... (`$" + items[pInfo(peer)->hand].ori_name + "`` mod added)");
					CAction::Positioned(peer, pInfo(peer)->netID, "audio/dialog_confirm.wav", 0);
					Clothing_V2::Update(peer);
				}
			}
		}
		if (pInfo(peer)->show_pets) {
			if (rand() % 100 < 2) {
				if (pInfo(peer)->Pets_Not2) {
					pInfo(peer)->Pets_hunger++;
					pInfo(peer)->Pets_health--;
				}
				if (pInfo(peer)->Pets_health < 20) {
					if (not pInfo(peer)->Pets_Not3) {
						VarList::OnTalkBubble(peer, pInfo(peer)->pet_netID, "help me.....my blood keeps decreasing", 2, true);
						pInfo(peer)->Pets_Not3 = true;
					}
				}
				if (not pInfo(peer)->Pets_Not2 and pInfo(peer)->Pets_hunger > 0) pInfo(peer)->Pets_hunger--;
				if (pInfo(peer)->Pets_hunger < 30 and not pInfo(peer)->Pets_Not2) {
					if (not pInfo(peer)->Pets_Not) {
						VarList::OnTalkBubble(peer, pInfo(peer)->pet_netID, "I am hungry... give me some food.", 2, true);
						pInfo(peer)->Pets_Not = true;
					}
					if (pInfo(peer)->Pets_hunger < 1 or pInfo(peer)->Pets_hunger == 0) pInfo(peer)->Pets_Not2 = true;
				}
				if (pInfo(peer)->Pets_health == 0) {
					pInfo(peer)->Pets_Dead = true;
					pInfo(peer)->Pets_Not = false;
					pInfo(peer)->Pets_Not2 = false;
					pInfo(peer)->Pets_Not3 = false;
					pInfo(peer)->Pets_Death_Times++;
					VarList::OnTalkBubble(peer, pInfo(peer)->pet_netID, "I'm Dead.................", 2, true);
					pInfo(peer)->pet_ID = 0;
					pInfo(peer)->show_pets = false;
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						if (pInfo(cp_)->world == pInfo(peer)->world) {
							VarList::OnKilled(cp_, pInfo(peer)->pet_netID, 0);
							VarList::OnRemove(cp_, pInfo(peer)->pet_netID, 0, 500);
						}
					}
				}
			}
		}
		if (pInfo(peer)->face == 4260 and pInfo(peer)->Present_Goblin != 0) {
			if (pInfo(peer)->x != (int)pMov->x) {
				if (pInfo(peer)->i240 + 6500 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
					pInfo(peer)->i240 = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
					vector<int> idx{ 4266, 4268, 4270, 4272, 4262, 4284, 4278, 4294 };
					WorldDrop item_{};
					string name_ = pInfo(peer)->world;
					World* world_ = PthtAutoHarvest::find_world(name_);
					if (world_ != nullptr) {
						world_->fresh_world = true;
						item_.id = idx[rand() % idx.size()], item_.count = 1, item_.x = (int)pMov->x + rand() % 17, item_.y = (int)pMov->y + rand() % 17;
						VisualHandle::Drop(world_, item_);
					}
				}
			}
		}
		if (pInfo(peer)->back == 240) {
			if (pInfo(peer)->gems > 0) {
				if (pInfo(peer)->x != (int)pMov->x) {
					if (pInfo(peer)->i240 + 750 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
						pInfo(peer)->i240 = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						pInfo(peer)->gems -= 1;
						WorldDrop item_{};
						std::string name_ = pInfo(peer)->world;
						World* world_ = PthtAutoHarvest::find_world(name_);
						if (world_ != nullptr) {
							world_->fresh_world = true;
							item_.id = 112, item_.count = 1, item_.x = (int)pMov->x + rand() % 17, item_.y = (int)pMov->y + rand() % 17;
							VisualHandle::Drop(world_, item_);
						}
						VarList::OnBuxGems(peer);
					}
				}
			}
		}
		if (pInfo(peer)->isStopTime) {
			// Player sedang di-freeze: kirim koreksi posisi ke diri sendiri saja
			pMov->netID = pInfo(peer)->netID;
			BYTE* raw = packPlayerMoving(pMov);
			send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
			delete[] raw;
		} else {
			// FIX ANTI-TELE / FIX EXPLOIT
			// Validasi delta posisi sebelum trust koordinat client.
			// Tanpa ini, executor bisa kirim PACKET_STATE dengan pInfo->x/y di mana saja
			// lalu spoof anti-far-place / anti-far-punch / anti-tooFar-NPC dengan trivial.
			//
			// Strategi: jika delta > MAX_DELTA_PX dalam single packet dan player tidak
			// dalam mode anchor-special (mod ghost/jetpack/fishing/dll. yang membenarkan
			// teleport server-side), snap-back ke posisi terakhir & log.
			// Owner / Admin / Developer dilewatkan karena role tersebut sering pakai
			// /warp / cmd custom yang server-driven (server akan re-broadcast posisi baru).
			static constexpr int ANTI_TELE_MAX_DELTA_PX = 600; // ~18 tiles, sangat permisif
			int prev_x = pInfo(peer)->x, prev_y = pInfo(peer)->y;
			int dx = std::abs((int)pMov->x - prev_x);
			int dy = std::abs((int)pMov->y - prev_y);
			bool tele_suspect = (dx > ANTI_TELE_MAX_DELTA_PX || dy > ANTI_TELE_MAX_DELTA_PX);
			bool first_state = (prev_x <= 0 && prev_y <= 0);
			bool world_just_entered = pInfo(peer)->in_enter_game;
			if (tele_suspect && !first_state && !world_just_entered && !Role::Moderator(peer)) {
				// Snap player back to previous server-side position; broadcast nothing,
				// hanya kirim koreksi self-positional ke peer ybs supaya client tidak
				// terlihat "tersangkut" di server.
				PlayerMoving snap{};
				snap.packetType = pMov->packetType;
				snap.netID = pInfo(peer)->netID;
				snap.characterState = pMov->characterState;
				snap.x = (float)prev_x;
				snap.y = (float)prev_y;
				BYTE* raw = packPlayerMoving(&snap);
				send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
				delete[] raw;
				pInfo(peer)->Account_Notes.push_back("`o" + currentDateTime() + ": >> Flagged anti-tele dx=" + std::to_string(dx) + " dy=" + std::to_string(dy) + " in " + pInfo(peer)->world);
				return; // jangan update pInfo->x/y — tetap pakai posisi server
			}
			PlayerCharacter::Move(peer, pMov);
		}
		pInfo(peer)->x = (int)pMov->x, pInfo(peer)->y = (int)pMov->y, pInfo(peer)->state = pMov->characterState & 0x10;
		pInfo(peer)->last_state = pMov->characterState;
		if (pInfo(peer)->show_pets and pInfo(peer)->pet_netID != 0) Pet_Ai::Move(peer, pMov);
	}
	static void PACKET_TILE_CHANGE_REQUEST(ENetPeer* peer, PlayerMoving* pMov) {
		// FIX OFF-BY-ONE: world width = 100 → index valid 0..99.
		// Sebelumnya `> 100` lolos saat punchX==100 → akses world_->blocks[100 + y*100] (OOB).
		if (pMov->punchX < 0 or pMov->punchX >= 100 or pMov->punchY < 0 or pMov->punchY >= 60) return;
		// World lock protection: hanya Owner Server yang bisa break/place di locked world.
		{
			World* w_ = find_world(pInfo(peer)->world);
			if (w_ && w_->worldLocked && !Role::Owner(peer)) {
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4This world is locked by the server.", 0, 1);
				return;
			}
		}
		if (pInfo(peer)->trading_with != -1) cancel_trade(peer, false);
		Position player = { pInfo(peer)->x / 32, pInfo(peer)->y / 32 };
		Position target = { pMov->punchX, pMov->punchY };
		if (isTooForAwayPunch(player, target) and not Role::Moderator(peer)) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`w(Too Far Away)``", 0, 1);
			return;
		}
		if (pInfo(peer)->autofarm) {
			VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You can't do that while auto-farming!", 0, true);
			return;
		}
		Player::AntiFarmSpam& antiFarm = pInfo(peer)->antiFarmSpam;
		if (pMov->plantingTree == 18) {
			pInfo(peer)->punch_count++;
			antiFarm.lastBreakTile = date_time::get_epoch_ms();
		}
		PlayerCharacter::Punch(peer, pMov->plantingTree, pMov->punchX, pMov->punchY, pMov->x, pMov->y);
	}
	static void PACKET_TILE_ACTIVATE_REQUEST(ENetPeer* peer, PlayerMoving* pMov) {
		// FIX OFF-BY-ONE: sama seperti PACKET_TILE_CHANGE_REQUEST.
		if (pMov->punchX < 0 or pMov->punchX >= 100 or pMov->punchY < 0 or pMov->punchY >= 60) return;
		std::string name_ = pInfo(peer)->world;
		World* world_ = PthtAutoHarvest::find_world(name_);
		if (world_ != nullptr) {
			if (pMov->punchX >= world_->max_x || pMov->punchY >= world_->max_y) return;
			world_->fresh_world = true;
			WorldBlock* block_ = &world_->blocks[pMov->punchX + (pMov->punchY * 100)];
			if (block_ != NULL) {
				if (items[items[block_->fg ? block_->fg : block_->bg].id].blockType == BlockTypes::CHECKPOINT) {
					if (pInfo(peer)->ghost || pInfo(peer)->invis) return;
					bool impossible = patchNoClip(world_, pInfo(peer)->x, pInfo(peer)->y, block_, peer);
					if (impossible) {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`w(Too Far Away)``", 0, 1);
						return;
					}
					if (block_->fg == 4882) { // Towel Rack Checkpoint
						if (not Playmods::HasById(pInfo(peer), 138) && Playmods::HasById(pInfo(peer), 136, 1)) {
							Playmods::Add(peer, 138);
						}
					}
					if (block_->fg == 9858) { // Growganoth Stone
						Wipe::Darkticket(peer, true);
					}
					if (block_->fg == 2994) { // Great Wolf Totem
						Wipe::Whistle(peer);
					}
					pInfo(peer)->c_x = pMov->punchX, pInfo(peer)->c_y = pMov->punchY;
					VarList::SetRespawnPos(peer, pInfo(peer)->netID, pInfo(peer)->c_x + (pInfo(peer)->c_y * 100), 0);
				}
				else if (items[block_->fg ? block_->fg : block_->bg].id == 6) { // Main Door
					Exit_World(peer);
				}
				else if (block_->fg == 4722 && !pInfo(peer)->adventure_begins) { // Adventure Begins
					pInfo(peer)->adventure_begins = true;
					if (block_->times != 0) {
						pInfo(peer)->totalTime = block_->times;
						pInfo(peer)->timers = date_time::get_epoch_time() + (block_->times - 1);
						pInfo(peer)->timerActive = true;
						gamepacket_t packet(0, pInfo(peer)->netID);
						packet.Insert("OnCountdownStart");
						packet.Insert(block_->times);
						packet.Insert(-1);
						packet.CreatePacket(peer);
					}
					else pInfo(peer)->timers = date_time::get_epoch_time();
					if (block_->lives != 0) {
						pInfo(peer)->lives = block_->lives;
						VisualHandle::Nick(peer, NULL);
					}
					VarList::OnAddNotification(peer, block_->heart_monitor, "interface/large/adventure.rttex", "audio/gong.wav");
				}
				else if (block_->fg == 428 || block_->fg == 430) { // Race Start & Ends
					if ((block_->fg == 428 && pInfo(peer)->race_flag == 0) or block_->fg == 430) {
						int time = 0;
						gamepacket_t p(0, pInfo(peer)->netID), p3(0, pInfo(peer)->netID);
						p.Insert("OnRace" + a + (block_->fg == 428 ? "Start" : "End"));
						if (block_->fg == 430) {
							time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() - pInfo(peer)->race_flag;
							p.Insert(time);
						}
						else pInfo(peer)->race_flag = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						p.CreatePacket(peer);
						CAction::Positioned(peer, pInfo(peer)->netID, "audio/" + a + (block_->fg == 428 ? "race_start" : "race_end") + ".wav", 0);
						if (block_->fg == 430) {
							pInfo(peer)->race_flag = 0;
							for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
								if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != world_->name) continue;
								VarList::OnConsoleMessage(cp_, get_player_nick(peer) + " `0finished in`` `2" + detailMSTime(time) + "```o!``");
							}
						}
					}
				}
				else if (items[block_->fg].blockType == BlockTypes::DOOR || items[block_->fg].blockType == BlockTypes::PORTAL) {
					std::string door_target = block_->door_destination, door_id = "";
					World target_world = *world_;
					bool locked = true, found_ = false;
					if (Only_Access(peer, world_, block_) || block_->open) locked = false;
					int spawn_x = 0, spawn_y = 0, gtick = 0;
					bool zombie = false, notick = false, cumtomyface = false;
					struct tm newtime;
					time_t now = time(0);
					localtime_s(&newtime, &now);
					time_t currentTime;
					time(&currentTime);
					const auto localTime = localtime(&currentTime);

					Inventory::Modify(peer, 1898, gtick);
					if (block_->fg == 1910 && gtick == 0) notick = true;
					if (notick && (Event()->Carnival || localTime->tm_mday >= 27)) {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You dont have any Golden Ticket.");
					}
					if (!Event()->Carnival && localTime->tm_mday <= 27 && block_->fg == 1910) {
						notick = true;
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Carnival is not opened yet!");
					}


					// ── Carnival game door – handle SEBELUM normal door flow ────────────────────
					if (block_->fg == 1910 && world_->name == "CARNIVAL" && !notick) {
						if (door_target.find(":") != std::string::npos) {
							std::vector<std::string> cdet = explode(":", door_target);
							if (cdet.size() >= 2) door_id = cdet[1];
						}
						std::string game = "";
						if      (door_id == "HUMANSHATEEACHOTHER")  game = "Growganoth_Left";
						else if (door_id == "HUMANSHATEEACHOTHER2") game = "Growganoth_Right";
						else if (door_id == "I7162HGYDD")           game = "Shooter_Left";
						else if (door_id == "I7162HGYDP")           game = "Shooter_Right";
						else if (door_id == "SPEEDRUN5000")         game = "Speedrun_5000";
						else if (door_id == "BRUTALBOUNCE")         game = "Brutal_Bounce";
						else if (door_id == "SPIKYSURVIVAL")        game = "Spiky_Survival";
						else if (door_id == "MIRRORMAZE")           game = "Mirror_Maze";
						else if (door_id == "CARDLEFT")             game = "Card_Left";
						else if (door_id == "CARDRIGHT")            game = "Card_Right";

						if (!game.empty()) {
							if (!Event()->Carnival) {
								VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Carnival is not opened yet!");
							} else if (!door_join_carnival(peer, world_, game)) {
								VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`4You don't have enough Golden Tickets!");
							}
							VarList::OnSetFreezeState(peer, pInfo(peer)->netID, 250, 1);
							VarList::OnSetFreezeState(peer, pInfo(peer)->netID, 250, 0);
							VarList::OnZoomCamera(peer, 250);
							return;
						}
					}
					// ─────────────────────────────────────────────────────────────────────────

					if (!locked && block_->fg != 762 && !zombie && !notick) {
						if (door_target.find(":") != std::string::npos) {
							std::vector<std::string> details = explode(":", door_target); if (details.size() >= 2) {
								door_target = details[0]; door_id = details[1];
							}
						}

						if (!door_target.empty() && door_target != world_->name) {
							if (!check_name(peer, door_target)) {
								gamepacket_t p(250, pInfo(peer)->netID);
								p.Insert("OnSetFreezeState");
								p.Insert(1);
								p.CreatePacket(peer);

								gamepacket_t p2(250);
								p2.Insert("OnConsoleMessage");
								p2.Insert(door_target);
								p2.CreatePacket(peer);

								gamepacket_t p3(250);
								p3.Insert("OnZoomCamera");
								p3.Insert((float)10000.000000);
								p3.Insert(1000);
								p3.CreatePacket(peer);

								gamepacket_t p4(250, pInfo(peer)->netID);
								p4.Insert("OnSetFreezeState");
								p4.Insert(0);
								p4.CreatePacket(peer);
								return;
							}
							target_world = get_world(door_target);
						}

						int ySize = (int)target_world.blocks.size() / 100, xSize = (int)target_world.blocks.size() / ySize;
						if (!door_id.empty()) {
							for (int i_ = 0; i_ < target_world.blocks.size(); i_++) {
								WorldBlock block_data = target_world.blocks[i_];
								if (block_data.fg == 1684 || block_data.fg == 1912 || items[block_data.fg].blockType == BlockTypes::DOOR || items[block_data.fg].blockType == BlockTypes::PORTAL) {
									if (block_data.door_id == door_id) {
										spawn_x = i_ % xSize, spawn_y = i_ / xSize;
										if (pInfo(peer)->carnival_playing == "Shooter_Left") spawn_x = 42, spawn_y = 53;
										if (pInfo(peer)->carnival_playing == "Shooter_Right") spawn_x = 72, spawn_y = 53;
										if (pInfo(peer)->carnival_playing == "Growganoth_Left") spawn_x = 3, spawn_y = 53;
										if (pInfo(peer)->carnival_playing == "Growganoth_Right") spawn_x = 96, spawn_y = 53;
										break;
									}
								}
							}
						}

						if (!locked && !door_id.empty()) {
							auto p = std::find_if(target_world.blocks.begin(), target_world.blocks.end(), [&](const WorldBlock& a) {
								return (items[a.fg].path_marker || items[a.fg].blockType == BlockTypes::DOOR || items[a.fg].blockType == BlockTypes::PORTAL) && a.door_id == door_id;
								});
							if (p != target_world.blocks.end()) {
								int i_ = p - target_world.blocks.begin();
								spawn_x = i_ % 100, spawn_y = i_ / 100;
							}
							else found_ = true;
						}
						else found_ = true;
					}
					if (block_->fg == 1682) { // Adv Gateway
						std::string story = "";
						std::string button = "";
						auto has_adventure = [&](const int& id) {
							for (const auto& item : pInfo(peer)->adventure_item) {
								if (item.id == id) return true;
							}
							return false;
							};
						for (int i = 0; i < 5; i++) {
							if (not block_->gate_options[i].first.empty()) {
								string storys = block_->gate_options[i].first;
								if (storys.substr(0, storys.length() - (storys.length() - 1)) == "+" or storys.substr(0, storys.length() - (storys.length() - 1)) == "#") {
									if (storys.substr(1, storys.length() - (storys.length() - 1)) == "I") {
										if (has_adventure(1698)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "R") {
										if (has_adventure(1694)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "T") {
										if (has_adventure(1702)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "B") {
										if (has_adventure(1700)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "K") {
										if (has_adventure(1696)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "G") {
										if (has_adventure(4738)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "P") {
										if (has_adventure(4716)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else {
										story.append("add_textbox|" + storys + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
								}
								else if (storys.substr(0, storys.length() - (storys.length() - 1)) == "-") {
									if (storys.substr(1, storys.length() - (storys.length() - 1)) == "I") {
										if (!has_adventure(1698)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "R") {
										if (!has_adventure(1694)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "T") {
										if (!has_adventure(1702)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "B") {
										if (!has_adventure(1700)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "K") {
										if (!has_adventure(1696)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "G") {
										if (!has_adventure(4738)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else if (storys.substr(1, storys.length() - (storys.length() - 1)) == "P") {
										if (!has_adventure(4716)) story.append("add_textbox|" + storys.substr(2) + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
									else {
										story.append("add_textbox|" + storys + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
									}
								}
								else {
									story.append("add_textbox|" + storys + "|left|\n" + (i > 3 ? "add_spacer|small|\n" : ""));
								}
							}
							if (not block_->gate_options[i].second.empty()) {
								string btn = block_->gate_options[i].second;
								if (btn.substr(0, btn.length() - (btn.length() - 1)) == "+" or btn.substr(0, btn.length() - (btn.length() - 1)) == "#") {
									if (btn.substr(1, btn.length() - (btn.length() - 1)) == "I") {
										if (has_adventure(1698)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "R") {
										if (has_adventure(1694)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "T") {
										if (has_adventure(1702)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "B") {
										if (has_adventure(1700)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "K") {
										if (has_adventure(1696)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "G") {
										if (has_adventure(4738)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "P") {
										if (has_adventure(4716)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else {
										button.append("add_button|button" + to_string(i) + "|" + block_->gate_options[i].second + "|noflags|0|0|\n");
									}
								}
								else if (btn.substr(0, btn.length() - (btn.length() - 1)) == "-") {
									if (btn.substr(1, btn.length() - (btn.length() - 1)) == "I") {
										if (!has_adventure(1698)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "R") {
										if (!has_adventure(1694)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "T") {
										if (!has_adventure(1702)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "B") {
										if (!has_adventure(1700)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "K") {
										if (!has_adventure(1696)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "G") {
										if (!has_adventure(4738)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else if (btn.substr(1, btn.length() - (btn.length() - 1)) == "P") {
										if (!has_adventure(4716)) button.append("add_button|button" + to_string(i) + "|" + btn.substr(2) + "|noflags|0|0|\n");
									}
									else {
										button.append("add_button|button" + to_string(i) + "|" + block_->gate_options[i].second + "|noflags|0|0|\n");
									}
								}
								else {
									button.append("add_button|button" + to_string(i) + "|" + block_->gate_options[i].second + "|noflags|0|0|\n");
								}
							}
						}
						VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wGateway to Adventure``|left|1682|\nembed_data|tilex|" + to_string(pMov->punchX) + "\nembed_data|tiley|" + to_string(pMov->punchY) + "\nadd_spacer|small|\n" + story + "|\n" + button + "|\nend_dialog|adventure_join|Cancel||");
						VarList::OnSetFreezeState(peer, pInfo(peer)->netID, 250, 1);
						VarList::OnZoomCamera(peer, 250);
						VarList::OnSetFreezeState(peer, pInfo(peer)->netID, 250, 0);
					}
					if (block_->fg == 762) { // Password Door
						pInfo(peer)->lastwrenchx = pMov->punchX, pInfo(peer)->lastwrenchy = pMov->punchY;
						if (pInfo(peer)->WarnPDoor == 25) {
							VarList::OnConsoleMessage(peer, "`4SPAM PASSWORD DOOR DETECTED!`o, have a nice day.");
							Punishment::Banned(peer, 6.307e+7, "SPAM PASSWORD DOOR DETECTED!", "AUTO-BANNED BY SYSTEM", 76, true);
						}
						using namespace std::chrono;
						if (pInfo(peer)->lastPDOOR + 500/*(Setengah Detik/half a second)*/ < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							pInfo(peer)->lastPDOOR = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						}
						else pInfo(peer)->WarnPDoor++;
						if (block_->door_id.empty()) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "No password has been set yet!", 0, 0);
						else VarList::OnDialogRequest(peer, SetColor(peer) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wPassword Door``|left|762|\nadd_textbox|The door requires a password.|left|\nadd_text_input|password|Password||24|\nend_dialog|password_reply|Cancel|OK|");
						VarList::OnSetFreezeState(peer, pInfo(peer)->netID, 250, 1);
						VarList::OnZoomCamera(peer, 250);
						VarList::OnSetFreezeState(peer, pInfo(peer)->netID, 250, 0);
					}
					bool block_a = false;
					if ((block_->fg == 10358 && Only_Access(peer, world_, block_) == false) && block_->open && block_->shelf_1 != 0) { // Entry Booth
						int my_wls = get_wls(peer, true);
						if (my_wls >= block_->shelf_1) {
							get_wls(peer, true, true, block_->shelf_1);
							block_->wl += block_->shelf_1;
							block_a = false;
							VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`2Entry!``", 0, 0);
						}
						else {
							block_a = true;
							pInfo(peer)->lastwrenchx = pMov->punchX, pInfo(peer)->lastwrenchy = pMov->punchY;
							VarList::OnTalkBubble(peer, pInfo(peer)->netID, "You don't have enough world locks!", 0, 0);
							VarList::OnSetFreezeState(peer, pInfo(peer)->netID, 250, 1);
							VarList::OnZoomCamera(peer, 250);
							VarList::OnSetFreezeState(peer, pInfo(peer)->netID, 250, 0);
						}
					}
					if (block_->fg != 1682 && block_->fg != 762 && block_a == false) {
						Enter_World(peer, target_world.name, spawn_x, spawn_y, 250, locked, true, found_);
					}
				}
				else {
					switch (block_->fg) {
					case 3270: case 3496: {
						Position2D steam_connector = track_steam(world_, block_, pMov->punchX, pMov->punchY);
						if (steam_connector.x >= 0 and steam_connector.y >= 0) {
							WorldBlock* block_s = &world_->blocks[steam_connector.x + (steam_connector.y * 100)];
							switch (block_s->fg) {
							case 3286: { // Steam Door
								block_s->flags = (block_s->flags & 0x00400000 ? block_s->flags ^ 0x00400000 : block_s->flags | 0x00400000);
								PlayerMoving data_{};
								data_.packetType = 5, data_.punchX = steam_connector.x, data_.punchY = steam_connector.y, data_.characterState = 0x8;
								BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_s));
								BYTE* blc = raw + 56;
								form_visual(blc, *block_s, *world_, peer, false);
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(cp_)->world == world_->name) {
										CAction::Positioned(cp_, pInfo(peer)->netID, "audio/hiss3.wav", 100);
										send_raw(cp_, 4, raw, 112 + alloc_(world_, block_s), ENET_PACKET_FLAG_RELIABLE);
									}
								}
								delete[] raw; // FIX MEMLEAK: blc = raw + 56, jangan delete.
								break;
							}
							case 3724: { // Spirit Storage Unit
								uint32_t scenario = 20;
								for (int i = 0; i < world_->drop_new.size(); i++) {
									Position2D dropped_at{ world_->drop_new[i][3] / 32, world_->drop_new[i][4] / 32 };
									if (dropped_at.x == steam_connector.x and dropped_at.y == steam_connector.y) {
										if (world_->drop_new[i][0] == 3722) {
											uint32_t explo_chance = world_->drop_new[i][1];
											PlayerMoving data_{};
											data_.packetType = 14, data_.netID = -2, data_.plantingTree = world_->drop_new[i][2];
											BYTE* raw = packPlayerMoving(&data_);
											int32_t item = -1;
											Memory_Copy(raw + 8, &item, 4);
											for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
												if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
												if (pInfo(cp_)->world == name_) {
													send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
												}
											}
											world_->drop_new[i][0] = 0, world_->drop_new[i][3] = -32, world_->drop_new[i][4] = -32;
											world_->drop_new.erase(world_->drop_new.begin() + i);
											delete[] raw;
											block_s->c_ += explo_chance;
											if (block_s->c_ * 5 >= 105) {
												float explosion_chance = (float)((block_s->c_ * 5) - 100) * 0.5;
												if (explosion_chance > rand() % 100) {
													block_s->fg = 3726;
													vector<int> all_p{ 13020, 12464, 3734, 3732, 3748, 3712, 3706, 3708, 3718, 11136, 3728, 10056, 3730, 3788, 3750, 3738, 6060, 3738, 6840, 3736, 7784, 9596, 9598, 12288 };
													uint32_t prize = 0;
													if (block_s->c_ * 5 <= 115) prize = 3734;
													else if (block_s->c_ * 5 <= 130) prize = 3732;
													else if (block_s->c_ * 5 <= 140) prize = 3748;
													else if (block_s->c_ * 5 <= 150) prize = 12464;
													else if (block_s->c_ * 5 <= 170) {
														vector<int> p_drops = {
															3712, 3706, 3708, 3718, 11136
														};
														prize = p_drops[rand() % p_drops.size()];
													}
													else if (block_s->c_ * 5 <= 190)  prize = 3728;
													else if (block_s->c_ * 5 <= 205)  prize = 10056;
													else if (block_s->c_ * 5 <= 220)  prize = 3730;
													else if (block_s->c_ * 5 == 225)  prize = 3788;
													else if (block_s->c_ * 5 <= 240)  prize = 3750;
													else if (block_s->c_ * 5 == 245)  prize = 3738;
													else if (block_s->c_ * 5 <= 255)  prize = 6060;
													else if (block_s->c_ * 5 <= 265 or explo_chance * 5 >= 265) {
														if (explo_chance * 5 >= 265) prize = all_p[rand() % all_p.size()];
														else prize = 3738;
													}
													else {
														vector<int> p_drops = {
															6840
														};
														if (block_s->c_ * 5 >= 270) p_drops.emplace_back(3736);
														if (block_s->c_ * 5 >= 295) p_drops.emplace_back(7784);
														if (block_s->c_ * 5 >= 369) p_drops.emplace_back(9598);
														if (block_s->c_ * 5 >= 500) p_drops.emplace_back(9596);
														if (block_s->c_ * 5 >= 850) p_drops.emplace_back(12288);
														prize = p_drops[rand() % p_drops.size()];
													}
													if (prize != 0) {
														if (prize == 3750) Epic_Quest::Add(peer, "Get a Ghost Charm from s Spirit Storage explosion", pInfo(peer)->epicq_ghostcharm, 1, 1);
														WorldDrop drop_block_{};
														drop_block_.x = steam_connector.x * 32 + rand() % 17;
														drop_block_.y = steam_connector.y * 32 + rand() % 17;
														drop_block_.id = prize, drop_block_.count = 1;
														VisualHandle::Drop(world_, drop_block_);
														{
															PlayerMoving data_{};
															data_.packetType = 0x11, data_.x = steam_connector.x * 32 + 16, data_.y = steam_connector.y * 32 + 16;
															data_.YSpeed = 97, data_.XSpeed = 3724;
															BYTE* raw = packPlayerMoving(&data_);
															PlayerMoving data_2{};
															data_2.packetType = 0x11, data_2.x = steam_connector.x * 32 + 16, data_2.y = steam_connector.y * 32 + 16;
															data_2.YSpeed = 108;
															BYTE* raw2 = packPlayerMoving(&data_2);
															for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
																if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
																if (pInfo(cp_)->world == world_->name) {
																	VarList::OnConsoleMessage(cp_, "`#[A `9Spirit Storage Unit`` exploded, bringing forth an `9" + items[prize].name + "`` from The Other Side!]``");
																	send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
																	send_raw(cp_, 4, raw2, 56, ENET_PACKET_FLAG_RELIABLE);
																}
															}
															delete[] raw;  // FIX MEMLEAK: dulu `delete[] raw, raw2;` (comma op)
															delete[] raw2; // → raw2 leak per spirit-storage explosion.
														}
														scenario = 22;
													}
													block_s->c_ = 0;
												}
												PlayerMoving data_{};
												data_.packetType = 5, data_.punchX = steam_connector.x, data_.punchY = steam_connector.y, data_.characterState = 0x8;
												BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_s));
												BYTE* blc = raw + 56;
												form_visual(blc, *block_s, *world_, peer, false);
												for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
													if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
													if (pInfo(cp_)->world == world_->name) {
														send_raw(cp_, 4, raw, 112 + alloc_(world_, block_s), ENET_PACKET_FLAG_RELIABLE);
													}
												}
												delete[] raw; // FIX MEMLEAK: blc offset, jangan delete.
											}
											break;
										}
									}
								}
								PlayerMoving data_{};
								data_.packetType = 32;
								data_.punchX = steam_connector.x;
								data_.punchY = steam_connector.y;
								BYTE* raw = packPlayerMoving(&data_);
								raw[3] = scenario;
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(peer)->world != pInfo(cp_)->world) continue;
									send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
								}
								delete[] raw;
								break;
							}
							default:
								break;
							}
						}
						PlayerMoving data_{};
						data_.packetType = 32;
						data_.punchX = pMov->punchX;
						data_.punchY = pMov->punchY;
						BYTE* raw = packPlayerMoving(&data_);
						for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
							if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
							if (pInfo(peer)->world != pInfo(cp_)->world) continue;
							send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
						}
						delete[] raw;
						break;
					}
					default:
						break;
					}
				}
			}
		}
	}
	static void PACKET_ITEM_ACTIVATE_REQUEST(ENetPeer* peer, PlayerMoving* pMov) {
		if (pInfo(peer)->trading_with != -1) cancel_trade(peer, false);
		if (pMov->plantingTree <= 0 or pMov->plantingTree >= items.size()) return;
		int invTotal = 0;
		Inventory::Modify(peer, pMov->plantingTree, invTotal);
		if (invTotal < 1 || invTotal == 0) return;
		if (pMov->plantingTree == 6336) {/*GuideBook*/
			DialogHandle::GuideBook(peer);
			return;
		}
		if (items[pMov->plantingTree].blockType != BlockTypes::CLOTHING) {
			if (Inventory::Get_Slots(pInfo(peer)) >= 1) {
				if (items[pMov->plantingTree].compress_return_count != 0) {
					int countofused = 0, getdl = 1, getwl = 100, removewl = -100, removedl = -1, countwl = 0;
					Inventory::Modify(peer, items[pMov->plantingTree].compress_item_return, countwl);
					if (items[pMov->plantingTree].compress_return_count == 100 ? countwl <= 199 : countwl <= 100) {
						Inventory::Modify(peer, pMov->plantingTree, countofused);
						if (items[pMov->plantingTree].compress_return_count == 100 ? countofused >= 100 : countofused >= 1) {
							Inventory::Modify(peer, pMov->plantingTree, items[pMov->plantingTree].compress_return_count == 100 ? removewl : removedl);
							Inventory::Modify(peer, items[pMov->plantingTree].compress_item_return, items[pMov->plantingTree].compress_return_count == 100 ? getdl : getwl);
							VarList::OnTalkBubble(peer, pInfo(peer)->netID, items[pMov->plantingTree].compress_return_count == 100 ? "You compressed 100 `2" + items[pMov->plantingTree].ori_name + "`` into a `2" + items[items[pMov->plantingTree].compress_item_return].ori_name + "``!" : "You shattered a `2" + items[pMov->plantingTree].ori_name + "`` into 100 `2" + items[items[pMov->plantingTree].compress_item_return].ori_name + "``!", 0, 1);
							VarList::OnConsoleMessage(peer, items[pMov->plantingTree].compress_return_count == 100 ? "You compressed 100 `2" + items[pMov->plantingTree].ori_name + "`` into a `2" + items[items[pMov->plantingTree].compress_item_return].ori_name + "``!" : "You shattered a `2" + items[pMov->plantingTree].ori_name + "`` into 100 `2" + items[items[pMov->plantingTree].compress_item_return].ori_name + "``!");
						}
					}
				}
			}
		}
		else {
			if (pInfo(peer)->fishing_used != 0) stop_fishing(peer, true, "Sit still if you wanna fish!");
			Clothing::Equip(peer, pMov->plantingTree);
		}
	}
	static void PACKET_ITEM_ACTIVATE_OBJECT_REQUEST(ENetPeer* peer, PlayerMoving* pMov) {
		// FIX: Untuk packet type 11 (collect), punchX/punchY BUKAN koordinat tile.
		// Hanya validate ghost/invis. punchX/punchY bisa berisi data apapun dari client.
		if (pInfo(peer)->ghost or pInfo(peer)->invis) return;
		bool displaybox = true;
		std::string name_ = pInfo(peer)->world;
		World* world_ = PthtAutoHarvest::find_world(name_);
		if (world_ != nullptr) {
			if (pMov->y / 32 >= world_->max_y || pMov->x / 32 >= world_->max_x) return;
			world_->fresh_world = true;
			for (int i_ = 0; i_ < world_->drop_new.size(); i_++) {
				if (world_->drop_new[i_][2] == pMov->plantingTree) {
					bool block = false;
					if (items[world_->drop_new[i_][0]].blockType == BlockTypes::FISH) {
						for (int a_ = 0; a_ < pInfo(peer)->inv.size(); a_++) {
							if (pInfo(peer)->inv[a_].first == world_->drop_new[i_][0]) {
								block = true;
								break;
							}
						}
					}
					float x = (world_->drop_new[i_][3]) / 32.0f, y = (world_->drop_new[i_][4]) / 32.0f;
					CL_Vec2i current_pos = { static_cast<int>((pInfo(peer)->x + 10) / 32), static_cast<int>((pInfo(peer)->y + 15) / 32) };
					CL_Vec2i future_pos = { static_cast<int>(x), static_cast<int>(y) };
					int x_diff = std::abs(current_pos.x - future_pos.x), y_diff = std::abs(current_pos.y - future_pos.y);
					if (pInfo(peer)->x > 0 && pInfo(peer)->y > 0 && (x_diff > 8 or y_diff > 8)) {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`w(Too Far Away)``", 0, 1);
						return;
					}
					if (block) return;
					WorldBlock* block_ = &world_->blocks[world_->drop_new[i_][3] / 32 + (world_->drop_new[i_][4] / 32 * 100)];
					if (world_->world_settings & SETTINGS::SETTINGS_2 && Only_Access(peer, world_, block_) == false) {
						VarList::OnTalkBubble(peer, pInfo(peer)->netID, "`1(Collect disable here)", 0, 1);
						return;
					}
					if (block_->fg == 1422 || block_->fg == 2488) displaybox = Only_Access(peer, world_, block_);
					if (displaybox) {
						if (pInfo(peer)->Tutorial_Number == 3 and world_->drop_new[i_][0] == 3) {
							if (pInfo(peer)->Need_To_Required < pInfo(peer)->Requiring) {
								pInfo(peer)->Need_To_Required += 1;
								Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								if (pInfo(peer)->Need_To_Required >= pInfo(peer)->Requiring) {
									pInfo(peer)->Need_To_Required = pInfo(peer)->Requiring;
									CAction::Effect(peer, 48, (float)pInfo(peer)->x + 10, (float)pInfo(peer)->y + 16);
									VarList::OnConsoleMessage(peer, "`oCongratulations, You have completed the task: Collect Dirt Seeds.");
									VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Congratulations, You have completed the task: `oCollect Dirt Seeds``.", 0, 0);
									pInfo(peer)->Need_To_Required = 0, pInfo(peer)->Requiring = 2, pInfo(peer)->Tutorial_Number = 4;
									Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								}
							}
						}
						if (pInfo(peer)->Tutorial_Number == 6 and world_->drop_new[i_][0] == 11) {
							if (pInfo(peer)->Need_To_Required < pInfo(peer)->Requiring) {
								pInfo(peer)->Need_To_Required += 1;
								Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								if (pInfo(peer)->Need_To_Required >= pInfo(peer)->Requiring) {
									pInfo(peer)->Need_To_Required = pInfo(peer)->Requiring;
									CAction::Effect(peer, 48, (float)pInfo(peer)->x + 10, (float)pInfo(peer)->y + 16);
									VarList::OnConsoleMessage(peer, "`oCongratulations, You have completed the task: Collect Rock Seeds.");
									VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Congratulations, You have completed the task: `oCollect Rock Seeds``.", 0, 0);
									pInfo(peer)->Need_To_Required = 0, pInfo(peer)->Requiring = 2, pInfo(peer)->Tutorial_Number = 7;
									Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								}
							}
						}
						if (pInfo(peer)->Tutorial_Number == 7 and world_->drop_new[i_][0] == 15) {
							if (pInfo(peer)->Need_To_Required < pInfo(peer)->Requiring) {
								pInfo(peer)->Need_To_Required += 1;
								Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								if (pInfo(peer)->Need_To_Required >= pInfo(peer)->Requiring) {
									pInfo(peer)->Need_To_Required = pInfo(peer)->Requiring;
									CAction::Effect(peer, 48, (float)pInfo(peer)->x + 10, (float)pInfo(peer)->y + 16);
									VarList::OnConsoleMessage(peer, "`oCongratulations, You have completed the task: Collect Cave Background Seeds.");
									VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Congratulations, You have completed the task: `oCollect Cave Background Seeds``.", 0, 0);
									pInfo(peer)->Need_To_Required = 0, pInfo(peer)->Requiring = 1, pInfo(peer)->Tutorial_Number = 8;
									Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								}
							}
						}
						if (pInfo(peer)->Tutorial_Number == 11 and world_->drop_new[i_][0] == 5) {
							if (pInfo(peer)->Need_To_Required < pInfo(peer)->Requiring) {
								pInfo(peer)->Need_To_Required += 1;
								Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								if (pInfo(peer)->Need_To_Required >= pInfo(peer)->Requiring) {
									pInfo(peer)->Need_To_Required = pInfo(peer)->Requiring;
									CAction::Effect(peer, 48, (float)pInfo(peer)->x + 10, (float)pInfo(peer)->y + 16);
									VarList::OnConsoleMessage(peer, "`oCongratulations, You have completed the task: Collect Lava Seeds.");
									VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Congratulations, You have completed the task: `oCollect Lava Seeds``.", 0, 0);
									pInfo(peer)->Need_To_Required = 0, pInfo(peer)->Requiring = 3, pInfo(peer)->Tutorial_Number = 12;
									Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								}
							}
						}
						if (pInfo(peer)->Tutorial_Number == 14 and world_->drop_new[i_][0] == 101) {
							if (pInfo(peer)->Need_To_Required < pInfo(peer)->Requiring) {
								pInfo(peer)->Need_To_Required += 1;
								Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								if (pInfo(peer)->Need_To_Required >= pInfo(peer)->Requiring) {
									pInfo(peer)->Need_To_Required = pInfo(peer)->Requiring;
									CAction::Effect(peer, 48, (float)pInfo(peer)->x + 10, (float)pInfo(peer)->y + 16);
									VarList::OnConsoleMessage(peer, "`oCongratulations, You have completed the task: Collect Wood Blocks Seeds.");
									VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Congratulations, You have completed the task: `oCollect Wood Blocks Seeds``.", 0, 0);
									pInfo(peer)->Need_To_Required = 0, pInfo(peer)->Requiring = 1, pInfo(peer)->Tutorial_Number = 15;
									Tutorial::Button(peer, pInfo(peer)->Need_To_Required, pInfo(peer)->Requiring, pInfo(peer)->Tutorial_Number);
								}
							}
						}
						int c_ = world_->drop_new[i_][1];
						if (world_->special_event && find(world_->world_event_items.begin(), world_->world_event_items.end(), world_->drop_new[i_][0]) != world_->world_event_items.end()) {
							world_->special_event_item_taken++;
							if (world_->special_event_item == 9186) VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Royal Winter Wonders: You have collected `21 Royal Winter Seal``.");
							if (items[world_->special_event_item].event_total == world_->special_event_item_taken) {
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != name_) continue;
									if (items[world_->special_event_item].event_total != 1) {
										VarList::OnConsoleMessage(cp_, "`2" + items[world_->special_event_item].event_name + ":`` `0" + pInfo(peer)->tankIDName + "`` found a " + items[world_->drop_new[i_][0]].ori_name + "! (" + to_string(world_->special_event_item_taken) + "/" + to_string(items[world_->special_event_item].event_total) + ")``");
									}
									VarList::OnConsoleMessage(cp_, "`2" + items[world_->special_event_item].event_name + ":`` `oSuccess!`` " + (items[world_->special_event_item].event_total == 1 ? "`2" + pInfo(peer)->tankIDName + "`` `ofound it!``" : "All items found!``") + "");
									VarList::OnAddNotification(cp_, "`2" + items[world_->special_event_item].event_name + ":`` `oSuccess! " + (items[world_->special_event_item].event_total == 1 ? "`2" + pInfo(peer)->tankIDName + "`` found it!``" : "All items found!``"), "interface/large/special_event.rttex", "audio/cumbia_horns.wav");
								}
								world_->world_event_items.clear();
								world_->last_special_event = 0, world_->special_event_item = 0, world_->special_event_item_taken = 0, world_->special_event = false;
							}
							else {
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != name_) continue;
									VarList::OnConsoleMessage(cp_, "`2" + items[world_->special_event_item].event_name + ":`` `0" + pInfo(peer)->tankIDName + "`` found a " + items[world_->drop_new[i_][0]].ori_name + "! (" + to_string(world_->special_event_item_taken) + "/" + to_string(items[world_->special_event_item].event_total) + ")``");
								}
							}
						}
						int free_slots = Inventory::Get_Slots(pInfo(peer));
						if (Inventory::Contains(peer, world_->drop_new[i_][0]) >= 200 or free_slots == 0 and Inventory::Contains(peer, world_->drop_new[i_][0]) == 0 and world_->drop_new[i_][0] != 112 and world_->drop_new[i_][0] != 4490 and world_->drop_new[i_][0] != 9186) return;
						if (world_->drop_new[i_][0] == 112 or world_->drop_new[i_][0] == 4490 or world_->drop_new[i_][0] == 9186) {
							PlayerMoving data_{};
							data_.packetType = 14, data_.netID = (world_->drop_new[i_][0] == 4490 or world_->drop_new[i_][0] == 9186 ? -2 : pInfo(peer)->netID), data_.plantingTree = world_->drop_new[i_][2];
							BYTE* raw = packPlayerMoving(&data_);
							if (world_->drop_new[i_][0] == 112 || world_->drop_new[i_][0] == 4490) {
								if (pInfo(peer)->gems >= MAX_GEMS) {
									VarList::OnTalkBubble(peer, pInfo(peer)->netID, "Warning! Your gems have reached the maximum limit", 0, 0);
									return;
								}
								else {
									if (world_->drop_new[i_][0] == 4490) {
										VarList::OnBuxGems(peer, (1000 * world_->drop_new[i_][1]));
										PlayerMoving data2_{};
										data2_.x = world_->drop_new[i_][3] + 16, data2_.y = world_->drop_new[i_][4] + 16, data2_.packetType = 19, data2_.punchX = world_->drop_new[i_][0], data2_.punchY = pInfo(peer)->netID;
										int32_t to_netid = pInfo(peer)->netID;
										BYTE* raw2 = packPlayerMoving(&data2_);
										raw2[3] = 5;
										Memory_Copy(raw2 + 8, &to_netid, 4);
										send_raw(peer, 4, raw2, 56, ENET_PACKET_FLAG_RELIABLE);
										delete[] raw2;
									}
									else pInfo(peer)->gems += c_;
								}
							}
							else if (world_->drop_new[i_][0] == 9186) {
								pInfo(peer)->winter_seal += c_;
								PlayerMoving data2_{};
								data2_.x = world_->drop_new[i_][3] + 16, data2_.y = world_->drop_new[i_][4] + 16, data2_.packetType = 19, data2_.punchX = world_->drop_new[i_][0], data2_.punchY = pInfo(peer)->netID;
								int32_t to_netid = pInfo(peer)->netID;
								BYTE* raw2 = packPlayerMoving(&data2_);
								raw2[3] = 5;
								Memory_Copy(raw2 + 8, &to_netid, 4);
								send_raw(peer, 4, raw2, 56, ENET_PACKET_FLAG_RELIABLE);
								delete[] raw2;
							}
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != name_) continue;
								send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
							}
							delete[]raw;
							world_->drop_new.erase(world_->drop_new.begin() + i_);
						}
						else {
							auto drop = world_->drop_new[i_];
							ServerPool::CctvLogs::Add(peer, "took", to_string(world_->drop_new[i_][1]) + " " + items[world_->drop_new[i_][0]].name);
							world_->drop_new.erase(world_->drop_new.begin() + i_);
							SaveDropItemMoreTimes(peer, drop[0], drop[1]);
							PlayerMoving data_{};
							data_.packetType = 14;
							data_.netID = pInfo(peer)->netID;
							data_.plantingTree = drop[2];
							BYTE* raw = packPlayerMoving(&data_);
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
								if (pInfo(currentPeer)->world != name_) continue;
								send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
							}
							delete[] raw;
							break;
						}
					}
				}
			}
		}
	}
	static void PACKET_SET_ICON_STATE(ENetPeer* peer, PlayerMoving* pMov) {
		PlayerCharacter::Move(peer, pMov);
	}
	static void PACKET_GOT_PUNCHED(ENetPeer* peer, PlayerMoving* pMov) {
		// ---- Game Generator hit detection ----
		{
			std::string name_ = pInfo(peer)->world;
			World* world_ = PthtAutoHarvest::find_world(name_);
			if (world_ != nullptr && world_->game.started) {
				auto gamePlayer = world_->game.GetPlayer(pInfo(peer)->netID);
				int32_t attackerNetID = pMov->effect_flags_check; // m_target_net_id from GUP+8

				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED || cp_->data == NULL) continue;
					if (pInfo(peer)->world != pInfo(cp_)->world) continue;
					if (pInfo(peer)->netID == pInfo(cp_)->netID) continue;
					if (pInfo(cp_)->netID != (uint32_t)attackerNetID) continue;

					auto attackerPlayer = world_->game.GetPlayer((uint32_t)attackerNetID);
					if (attackerPlayer == nullptr || gamePlayer == nullptr) break;
					// Friendly-fire check: same team → skip (FFA team=4 always allowed)
					if (attackerPlayer->team == gamePlayer->team && attackerPlayer->team != 4) break;

					gamePlayer->hits++;

					if (gamePlayer->hits >= world_->game.player_hits) {
						SendRespawn(peer, false, 0, 1); // kill the victim

						// Give kill score to attacker
						world_->game.ModifyPlayer(cp_, SCORE, world_->game.pv_kill);

						// Particle death effect
						GUP::GameUpdatePacket particlePacket{};
						particlePacket.m_type = 17;
						particlePacket.m_particle_emitter_id = 69;
						particlePacket.m_vec_x = (float)pInfo(peer)->x;
						particlePacket.m_vec_y = (float)pInfo(peer)->y;
						particlePacket.m_particle_variable = (float)attackerPlayer->team;
						particlePacket.m_particle_alt_id = 69;
						for (ENetPeer* cp2 = server->peers; cp2 < &server->peers[server->peerCount]; ++cp2) {
							if (cp2->state != ENET_PEER_STATE_CONNECTED || cp2->data == NULL) continue;
							if (pInfo(cp2)->world == pInfo(peer)->world)
								GUP::send_packet(cp2, 4, &particlePacket, sizeof(GUP::GameUpdatePacket), ENET_PACKET_FLAG_RELIABLE);
						}

						if (world_->game.isOptionEnabled(RESPAWN_ON_SCORE))
							SendRespawn(peer, false, 1, 0);
						if (world_->game.isOptionEnabled(RESET_ON_SCORE))
							world_->game.ReturnBlocks(peer);

						// Apply die score to victim and reset hit counter
						world_->game.ModifyPlayer(peer, SCORE, world_->game.pv_die);
						world_->game.ModifyPlayer(peer, HIT, -(int8_t)gamePlayer->hits);

						// Lives system
						if (world_->game.lives > 0) {
							world_->game.ModifyPlayer(peer, LIVE, -1);
							// gamePlayer pointer may still be valid; re-fetch to be safe
							auto gp2 = world_->game.GetPlayer(pInfo(peer)->netID);
							if (gp2 != nullptr && gp2->lives <= 0) {
								world_->game.manageBattle(peer, LEAVE, (uint8_t)gp2->netID);
							}
						}
					}
					break; // found attacker, done
				}
				return; // don't run infection logic during a game
			}
		}
		// ---- End Game Generator hit detection ----

		if (pMov->punchX < 0 or pMov->punchX > 100 or pMov->punchY < 0 or pMov->punchY >= 60) return;
		if (pInfo(peer)->last_inf + 5000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
			pInfo(peer)->last_inf = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			bool inf = false;
			for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
				if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(peer)->world != pInfo(cp_)->world or pInfo(peer)->netID == pInfo(cp_)->netID) continue;
				if (abs(pInfo(cp_)->last_infected - pMov->plantingTree) <= 3) {
					if (Playmods::HasById(pInfo(cp_), 28) && not Playmods::HasById(pInfo(peer), 28) && inf == false) {
						if (Playmods::HasById(pInfo(peer), 25)) {
							for (ENetPeer* cp_2 = server->peers; cp_2 < &server->peers[server->peerCount]; ++cp_2) {
								if (cp_2->state != ENET_PEER_STATE_CONNECTED or cp_2->data == NULL or pInfo(peer)->world != pInfo(cp_2)->world) continue;
								PlayerMoving data_{};
								data_.packetType = 19, data_.punchX = 782, data_.x = pInfo(peer)->x + 10, data_.y = pInfo(peer)->y + 16;
								int32_t to_netid = pInfo(peer)->netID;
								BYTE* raw = packPlayerMoving(&data_);
								raw[3] = 5;
								Memory_Copy(raw + 8, &to_netid, 4);
								send_raw(cp_2, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
								delete[]raw;
							}
						}
						else {
							string name_ = pInfo(peer)->world;
							World* world_ = PthtAutoHarvest::find_world(name_);
							if (world_ != nullptr) {
								bool can_cancel = true;
								if (find(world_->active_jammers.begin(), world_->active_jammers.end(), 1278) != world_->active_jammers.end()) can_cancel = false;
								if (can_cancel) {
									pInfo(cp_)->last_infected = 0;
									inf = true;
									VarList::OnConsoleMessage(peer, "You've been infected by the g-Virus. Punch others to infect them, too! Braiiiins... (`$Infected!`` mod added, `$1 mins`` left)");
									VarList::OnAddNotification(peer, "`4You were infected by " + pInfo(cp_)->tankIDName + "!", "interface/large/infected.rttex", "");
									PlayMods give_playmod{};
									give_playmod.id = 28, give_playmod.time = time(nullptr) + 60;
									pInfo(peer)->playmods.emplace_back(give_playmod);
									Clothing_V2::Update(peer);
								}
							}
							else return;
						}
					}
					else if (Playmods::HasById(pInfo(peer), 28) && not Playmods::HasById(pInfo(cp_), 28) && inf == false) {
						inf = true;
						Player_Respawn(peer, 0, true);
						for (int i_ = 0; i_ < pInfo(peer)->playmods.size(); i_++) {
							if (pInfo(peer)->playmods[i_].id == 28) {
								pInfo(peer)->playmods[i_].time = 0;
								break;
							}
						}
						string name_ = pInfo(cp_)->world;
						World* world_ = PthtAutoHarvest::find_world(name_);
						if (world_ != nullptr) {
							WorldDrop drop_block_{};
							drop_block_.id = rand() % 100 < 50 ? 4450 : 4490, drop_block_.count = pInfo(cp_)->hand == 9500 ? 2 : 1, drop_block_.uid = uint16_t(world_->drop_new.size()) + 1, drop_block_.x = pInfo(peer)->x, drop_block_.y = pInfo(peer)->y;
							VisualHandle::Drop(world_, drop_block_);
						}
					}
				}
			}
		}
	}
	static void PACKET_APP_CHECK_RESPONSE(ENetPeer* peer, PlayerMoving* pMov) {
	
	}
	static void PACKET_APP_INTEGRITY_FAIL(ENetPeer* peer, PlayerMoving* pMov) {

	}
	static void PACKET_PING_REQUEST(ENetPeer* peer, PlayerMoving* pMov) {

	}
	static void PACKET_PROXY_CHECK_RESPONSE(ENetPeer* peer, PlayerMoving* pMov) {
		size_t pSize = (sizeof(PlayerMoving) * 7);
		BYTE* data = (BYTE*)std::malloc(pSize);
		if (!data) return;
		uint16_t net_id = pInfo(peer)->netID;
		Memory_Copy(data + 1, &net_id, 2);
		uint8_t pos = (uint8_t)pMov->XSpeed + pMov->YSpeed;
		Memory_Copy(data + 3, &pos, 1);
		int flags = 0x08;
		Memory_Copy(data + 4, &flags, 4);
		uint16_t item = (uint16_t)pMov->plantingTree;
		Memory_Copy(data + 8, &item, 2);
		int punchpos = (int)pMov->punchX + pMov->punchY;
		Memory_Copy(data + 10, &punchpos, 4);
		std::free(data);
	}
};