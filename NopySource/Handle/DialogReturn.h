#pragma warning(push)
#pragma warning(disable: 26812)
#pragma warning(disable: 6054)
#pragma warning(disable: 6385)
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#pragma once
#include <filesystem>
#include <random>
#include <string>
#include "NopySource/Handle/WorldInfo.h"
#include <iostream>
#include <fstream>
#include <sstream>
namespace fs = std::filesystem;

void call_dialog(ENetPeer* p_, std::string cch) {
	if (pInfo(p_)->trading_with != -1 and cch.find("trade_") == string::npos) {
		cancel_trade(p_, false, true);
		return;
	}
	if (pInfo(p_)->world.empty() and not pInfo(p_)->tankIDName.empty()) return;
	std::vector<std::string> a_ = explode("|", replace_str(cch, "\n", "|"));
	for (int i_ = 0; i_ < a_.size(); i_++) {
		if (a_[i_] == "dispshelf") {
			if (a_.size() != 13 and a_.size() != 14) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
			if (x_ < 0 or y_ < 0) break;
			string name_ = pInfo(p_)->world;
			World* world_ = find_world(name_);
			if (world_ != nullptr) {
				if (y_ >= world_->max_y || x_ >= world_->max_x) break;
				WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				if (items[t_].blockType != BlockTypes::Display_Shelf || Only_Access(p_, world_, block_) == false && !guild_access(p_, world_->guild_id)) break;
				for (int b_ = 0; b_ < a_.size(); b_++) {
					if (a_[b_] == "replace1" || a_[b_] == "replace2" || a_[b_] == "replace3" || a_[b_] == "replace4") {
						if (a_.size() >= (b_ + 1) - 1) {
							if (not isdigit(a_[b_ + 1][0])) break;
							int item_id = std::atoi(a_[b_ + 1].c_str());
							if (item_id <= 0 || item_id >= items.size()) break;
							if (items[item_id].untradeable || item_id == 1424 || item_id == 5816 || items[item_id].blockType == BlockTypes::FISH) {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(p_)->netID);
								p.Insert("You can't display untradeable items.");
								p.Insert(0), p.Insert(0);
								p.CreatePacket(p_);
								return;
							}
							if ((a_[b_] == "replace1" ? block_->shelf_1 : a_[b_] == "replace2" ? block_->shelf_2 : a_[b_] == "replace3" ? block_->shelf_3 : block_->shelf_4) != 0) {
								int b = 1;
								if (Inventory::Modify(p_, (a_[b_] == "replace1" ? block_->shelf_1 : a_[b_] == "replace2" ? block_->shelf_2 : a_[b_] == "replace3" ? block_->shelf_3 : block_->shelf_4), b) == 0) {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("Picked up 1 " + items[(a_[b_] == "replace1" ? block_->shelf_1 : a_[b_] == "replace2" ? block_->shelf_2 : a_[b_] == "replace3" ? block_->shelf_3 : block_->shelf_4)].name + ".");
									p.CreatePacket(p_);
									if (Role::Administrator(p_)) ServerPool::Logs::Add(pInfo(p_)->tankIDName + " stole `" + items[(a_[b_] == "replace1" ? block_->shelf_1 : a_[b_] == "replace2" ? block_->shelf_2 : a_[b_] == "replace3" ? block_->shelf_3 : block_->shelf_4)].name + "` in(" + pInfo(p_)->world + ") from Display Shelf", "Display Shelf");
									(a_[b_] == "replace1" ? block_->shelf_1 : a_[b_] == "replace2" ? block_->shelf_2 : a_[b_] == "replace3" ? block_->shelf_3 : block_->shelf_4) = 0;
								}
								else {
									gamepacket_t p;
									p.Insert("OnTalkBubble");
									p.Insert(pInfo(p_)->netID);
									p.Insert("No room to take " + items[(a_[b_] == "replace1" ? block_->shelf_1 : a_[b_] == "replace2" ? block_->shelf_2 : a_[b_] == "replace3" ? block_->shelf_3 : block_->shelf_4)].name + ".");
									p.Insert(0), p.Insert(0);
									p.CreatePacket(p_);
									break;
								}
							}
							int b = -1;
							if (Inventory::Modify(p_, item_id, b) == 0) {
								(a_[b_] == "replace1" ? block_->shelf_1 : a_[b_] == "replace2" ? block_->shelf_2 : a_[b_] == "replace3" ? block_->shelf_3 : block_->shelf_4) = item_id;
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("Put " + items[(a_[b_] == "replace1" ? block_->shelf_1 : a_[b_] == "replace2" ? block_->shelf_2 : a_[b_] == "replace3" ? block_->shelf_3 : block_->shelf_4)].name + " on display.");
								p.CreatePacket(p_);
							}
						}
					}
					else if (a_[b_] == "remove") {
						if (block_->shelf_1 != 0) {
							int b = 1;
							if (Inventory::Modify(p_, block_->shelf_1, b) == 0) {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("Picked up 1 " + items[block_->shelf_1].name + ".");
								p.CreatePacket(p_);
								if (Role::Administrator(p_)) ServerPool::Logs::Add(pInfo(p_)->tankIDName + " stole `" + items[block_->shelf_1].name + "` in(" + pInfo(p_)->world + ") from Display Shelf", "Display Shelf");
								block_->shelf_1 = 0;
							}
							else {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(p_)->netID);
								p.Insert("No room to take " + items[block_->shelf_1].name + ".");
								p.Insert(0), p.Insert(0);
								p.CreatePacket(p_);
								break;
							}
						}
						if (block_->shelf_2 != 0) {
							int b = 1;
							if (Inventory::Modify(p_, block_->shelf_2, b) == 0) {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("Picked up 1 " + items[block_->shelf_2].name + ".");
								p.CreatePacket(p_);
								if (Role::Administrator(p_)) ServerPool::Logs::Add(pInfo(p_)->tankIDName + " stole `" + items[block_->shelf_2].name + "` in(" + pInfo(p_)->world + ") from Display Shelf", "Display Shelf");
								block_->shelf_2 = 0;
							}
							else {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(p_)->netID);
								p.Insert("No room to take " + items[block_->shelf_2].name + ".");
								p.Insert(0), p.Insert(0);
								p.CreatePacket(p_);
								break;
							}
						}
						if (block_->shelf_3 != 0) {
							int b = 1;
							if (Inventory::Modify(p_, block_->shelf_3, b) == 0) {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("Picked up 1 " + items[block_->shelf_3].name + ".");
								p.CreatePacket(p_);
								if (Role::Administrator(p_)) ServerPool::Logs::Add(pInfo(p_)->tankIDName + " stole `" + items[block_->shelf_3].name + "` in(" + pInfo(p_)->world + ") from Display Shelf", "Display Shelf");
								block_->shelf_3 = 0;
							}
							else {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(p_)->netID);
								p.Insert("No room to take " + items[block_->shelf_3].name + ".");
								p.Insert(0), p.Insert(0);
								p.CreatePacket(p_);
								break;
							}
						}
						if (block_->shelf_4 != 0) {
							int b = 1;
							if (Inventory::Modify(p_, block_->shelf_4, b) == 0) {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("Picked up 1 " + items[block_->shelf_4].name + ".");
								p.CreatePacket(p_);
								if (Role::Administrator(p_)) ServerPool::Logs::Add(pInfo(p_)->tankIDName + " stole `" + items[block_->shelf_4].name + "` in(" + pInfo(p_)->world + ") from Display Shelf", "Display Shelf");
								block_->shelf_4 = 0;
							}
							else {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(p_)->netID);
								p.Insert("No room to take " + items[block_->shelf_4].name + ".");
								p.Insert(0), p.Insert(0);
								p.CreatePacket(p_);
								break;
							}
						}
						break;
					}
				}
				tile_update(p_, world_, block_, x_, y_);
			}
			return;
		}
		else if (a_[i_] == "portrait") {
			try {
				if (not isdigit(a_.at(i_ + 2).at(0)) or not isdigit(a_.at(i_ + 5).at(0))) break;
				int x_ = std::atoi(a_.at(i_ + 2).c_str()), y_ = std::atoi(a_.at(i_ + 5).c_str());
				if (x_ < 0 or y_ < 0) break;
				string name_ = pInfo(p_)->world;
				World* world_ = find_world(name_);
				if (world_ != nullptr) {
					if (y_ >= world_->max_y || x_ >= world_->max_x) break;
					WorldBlock* block_ = &world_->blocks.at(x_ + (y_ * 100));
					uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
					if (items[t_].blockType != BlockTypes::PORTRAIT || Only_Access(p_, world_, block_) == false && !guild_access(p_, world_->guild_id)) break;
					for (int b_ = 0; b_ < a_.size(); b_++) {
						if (a_.size() > b_ + 1) {
							if (a_.at(b_ + 1) == "chk1") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 1;
								}
							}
							else if (a_.at(b_ + 1) == "chk2") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 2;
								}
							}
							else if (a_.at(b_ + 1) == "chk3") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 3;
								}
							}
							else if (a_.at(b_ + 1) == "chk4") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 4;
								}
							}
							else if (a_.at(b_ + 1) == "chk5") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 5;
								}
							}
							else if (a_.at(b_ + 1) == "chk6") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 6;
								}
							}
							else if (a_.at(b_ + 1) == "chk7") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 7;
								}
							}
							else if (a_.at(b_ + 1) == "chk9") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 9;
								}
							}
							else if (a_.at(b_ + 1) == "chk11") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 11;
								}
							}
							else if (a_.at(b_ + 1) == "chk12") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 12;
								}
							}
							else if (a_.at(b_ + 1) == "chk14") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 14;
								}
							}
							else if (a_.at(b_ + 1) == "chk16") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 16;
								}
							}
							else if (a_.at(b_ + 1) == "chk18") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 18;
								}
							}
							else if (a_.at(b_ + 1) == "chk22") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 22;
								}
							}
							else if (a_.at(b_ + 1) == "chk27") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 27;
								}
							}
							else if (a_.at(b_ + 1) == "chk28") {
								if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
								if (a_.size() >= (b_ + 2) - 1) {
									if (not isdigit(a_.at(b_ + 2).at(0))) break;
									if (a_.at(b_ + 2) == "1") block_->portrait.c_expression = 28;
								}
							}
						}
						if (a_.at(b_) == "erase") {
							int b = -4;
							if (Inventory::Modify(p_, 3492, b) == 0) {
								block_->portrait.c_expression = 1;
								block_->portrait.c_skin = 0;
								block_->portrait.c_face = 0;
								block_->portrait.c_head = 0;
								block_->portrait.c_hair = 0;
							}
						}
						else if (a_.at(b_) == "playerNetID") {
							if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) {
								if (a_.size() >= (b_ + 1) - 1) {
									if (not isdigit(a_.at(b_ + 1).at(0))) break;
									uint32_t netID = std::atoi(a_.at(b_ + 1).c_str());
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
										if (pInfo(cp_)->world == pInfo(p_)->world and pInfo(cp_)->netID == netID) {
											int red = 0, yellow = 0, green = 0, aqua = 0, blue = 0, purple = 0, charcoal = 0, varnish = 0;
											{
												Inventory::Modify(p_, 3478, red);
												Inventory::Modify(p_, 3480, yellow);
												Inventory::Modify(p_, 3482, green);
												Inventory::Modify(p_, 3484, aqua);
												Inventory::Modify(p_, 3486, blue);
												Inventory::Modify(p_, 3488, purple);
												Inventory::Modify(p_, 3490, charcoal);
												Inventory::Modify(p_, 3492, varnish);
												if (red >= 2 and yellow >= 2 and green >= 2 and aqua >= 2 and blue >= 2 and purple >= 2 and charcoal >= 2 and varnish >= 2) {
													red = -2, yellow = -2, green = -2, aqua = -2, blue = -2, purple = -2, charcoal = -2, varnish = -2;
													if (Inventory::Modify(p_, 3478, red) == 0 and Inventory::Modify(p_, 3480, yellow) == 0 and Inventory::Modify(p_, 3482, green) == 0
														and Inventory::Modify(p_, 3484, aqua) == 0 and Inventory::Modify(p_, 3486, blue) == 0 and Inventory::Modify(p_, 3488, purple) == 0
														and Inventory::Modify(p_, 3490, charcoal) == 0 and Inventory::Modify(p_, 3492, varnish) == 0) {
														block_->portrait.c_expression = 1;
														block_->portrait.c_skin = pInfo(cp_)->skin;
														block_->portrait.c_face = pInfo(cp_)->face;
														block_->portrait.c_head = pInfo(cp_)->hair;
														block_->portrait.c_hair = pInfo(cp_)->mask;
														block_->txt = pInfo(cp_)->name_color + pInfo(cp_)->tankIDName;
													}
												}
											}
											break;
										}
									}
								}
							}
						}
						else if (a_.at(b_) == "artname") {
							if (block_->portrait.c_skin == 0 and block_->portrait.c_face == 0 and block_->portrait.c_hair == 0 and block_->portrait.c_head == 0) continue;
							if (a_.size() >= (b_ + 1) - 1) {
								string txt = a_.at(b_ + 1);
								if (txt.size() > 60) break;
								block_->txt = txt;
							}
						}
					}
					tile_update(p_, world_, block_, x_, y_);
				}
			}
			catch (out_of_range) {
				return;
			}
			return;
		}
		else if (a_.at(i_) == "artcanvas") {
			try {
				if (not isdigit(a_.at(i_ + 2).at(0)) or not isdigit(a_.at(i_ + 5).at(0))) break;
				int x_ = std::atoi(a_.at(i_ + 2).c_str()), y_ = std::atoi(a_.at(i_ + 5).c_str());
				if (x_ < 0 or y_ < 0) break;
				string name_ = pInfo(p_)->world;
				World* world_ = find_world(name_);
				if (world_ != nullptr) {
					if (y_ >= world_->max_y || x_ >= world_->max_x) break;
					WorldBlock* block_ = &world_->blocks.at(x_ + (y_ * 100));
					uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
					if (items[t_].blockType != BlockTypes::Painting_Easel || Only_Access(p_, world_, block_) == false && !guild_access(p_, world_->guild_id)) break;
					for (int b_ = 0; b_ < a_.size(); b_++) {
						if (a_.at(b_) == "erase") {
							int b = -4;
							if (Inventory::Modify(p_, 3492, b) == 0) {
								block_->id = 0;
								block_->txt = "";
							}
						}
						else if (a_.at(b_) == "paint") {
							if (a_.size() >= (b_ + 1) - 1) {
								if (not isdigit(a_.at(b_ + 1).at(0))) break;
								int item_id = std::atoi(a_.at(b_ + 1).c_str());
								if (item_id <= 0 || item_id >= items.size()) break;
								int kiek_turi = 0;
								Inventory::Modify(p_, item_id, kiek_turi);
								if (kiek_turi == 0) break;
								int red = 0, yellow = 0, green = 0, aqua = 0, blue = 0, purple = 0, charcoal = 0, varnish = 0;
								Inventory::Modify(p_, 3478, red);
								Inventory::Modify(p_, 3480, yellow);
								Inventory::Modify(p_, 3482, green);
								Inventory::Modify(p_, 3484, aqua);
								Inventory::Modify(p_, 3486, blue);
								Inventory::Modify(p_, 3488, purple);
								Inventory::Modify(p_, 3490, charcoal);
								Inventory::Modify(p_, 3492, varnish);
								if (red >= 2 and yellow >= 2 and green >= 2 and aqua >= 2 and blue >= 2 and purple >= 2 and charcoal >= 2 and varnish >= 2) {
									red = -2, yellow = -2, green = -2, aqua = -2, blue = -2, purple = -2, charcoal = -2, varnish = -2;
									if (Inventory::Modify(p_, 3478, red) == 0 and Inventory::Modify(p_, 3480, yellow) == 0 and Inventory::Modify(p_, 3482, green) == 0
										and Inventory::Modify(p_, 3484, aqua) == 0 and Inventory::Modify(p_, 3486, blue) == 0 and Inventory::Modify(p_, 3488, purple) == 0
										and Inventory::Modify(p_, 3490, charcoal) == 0 and Inventory::Modify(p_, 3492, varnish) == 0) {
										block_->id = item_id;
										block_->txt = items[item_id].name;
									}
								}
							}
						}
						else if (a_.at(b_) == "artname") {
							if (a_.size() >= (b_ + 1) - 1) {
								string txt = a_.at(b_ + 1);
								if (txt.size() > 60) break;
								block_->txt = txt;
							}
						}
					}
					tile_update(p_, world_, block_, x_, y_);
				}
			}
			catch (out_of_range) {
				Logger::Info("ERROR", "crash try by " + pInfo(p_)->tankIDName);
				return;
			}
			return;
		}
		else if (a_[i_] == "mannequin_edit") {
			if (a_.size() == 14) {
				if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0]) or not isdigit(a_[i_ + 8][0])) break;
				int x_ = 0, y_ = 0, tile_ = 0;
				try {
					x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str()), tile_ = std::atoi(a_[i_ + 8].c_str());
				}
				catch (out_of_range) {
					return;
				}
				if (x_ < 0 or y_ < 0) break;
				if (tile_ <= 0 || tile_ >= items.size()) break;
				string name_ = pInfo(p_)->world;
				World* world_ = find_world(name_);
				if (world_ != nullptr) {
					if (y_ >= world_->max_y || x_ >= world_->max_x) break;
					if (items[tile_].blockType != CLOTHING or items[tile_].untradeable or items[tile_].clothType == ClothTypes::ANCES) return;
					WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
					uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
					if (items[t_].blockType != BlockTypes::MANNEQUIN || Only_Access(p_, world_, block_) == false && !guild_access(p_, world_->guild_id)) break;
					uint16_t current_mannequin_item = 0;
					switch (items[tile_].clothingType) {
					case 0: current_mannequin_item = block_->mannequin.c_hair;
						break;
					case 1: current_mannequin_item = block_->mannequin.c_shirt;
						break;
					case 2: current_mannequin_item = block_->mannequin.c_pants;
						break;
					case 3: current_mannequin_item = block_->mannequin.c_feet;
						break;
					case 4: current_mannequin_item = block_->mannequin.c_head;
						break;
					case 5: current_mannequin_item = block_->mannequin.c_hand;
						break;
					case 6: current_mannequin_item = block_->mannequin.c_back;
						break;
					case 7: current_mannequin_item = block_->mannequin.c_mask;
						break;
					case 8: current_mannequin_item = block_->mannequin.c_neck;
						break;
					case 9: {
						return;
					}
					} if (current_mannequin_item == tile_) {
						return;
					}
					else {
						if (current_mannequin_item != 0) {
							int b = 1;
							if (Inventory::Modify(p_, int(current_mannequin_item), b) != 0) {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(p_)->netID);
								p.Insert("`5[`2You don't have inventory space!``]``");
								p.Insert(0), p.Insert(0);
								p.CreatePacket(p_);
								return;
							}
						}
						int c_ = -1;
						if (Inventory::Modify(p_, tile_, c_) == 0) {
							switch (items[tile_].clothingType) {
							case 0: block_->mannequin.c_hair = tile_;
								break;
							case 1: block_->mannequin.c_shirt = tile_;
								break;
							case 2: block_->mannequin.c_pants = tile_;
								break;
							case 3: block_->mannequin.c_feet = tile_;
								break;
							case 4: block_->mannequin.c_head = tile_;
								break;
							case 5: block_->mannequin.c_hand = tile_;
								break;
							case 6: block_->mannequin.c_back = tile_;
								break;
							case 7: block_->mannequin.c_mask = tile_;
								break;
							case 8: block_->mannequin.c_neck = tile_;
								break;
							case 9: {
								return;
							}
							}
							gamepacket_t p(0, pInfo(p_)->netID);
							p.Insert("OnPlayPositioned");
							p.Insert("audio/change_clothes.wav");
							tile_update(p_, world_, block_, x_, y_);
						}
					}
				}
			}
			else {
				if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
				int x_ = 0, y_ = 0;
				try {
					x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
				}
				catch (out_of_range) {
					return;
				}
				string name_ = pInfo(p_)->world;
				World* world_ = find_world(name_);
				if (world_ != nullptr) {
					WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
					uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
					if (items[t_].blockType != BlockTypes::MANNEQUIN || Only_Access(p_, world_, block_) == false && !guild_access(p_, world_->guild_id)) break;
					bool clear_all = false;
					int fkthis = 0;
					for (int b_ = 0; b_ < a_.size(); b_++) {
						if (a_[b_].substr(0, 9) == "checkbox_") {
							if (a_.size() >= (b_ + 1) - 1) {
								std::vector<std::string> target_ = explode("_", a_[b_]);
								if (not isdigit(target_[1][0])) break;
								int item_remove = std::atoi(target_[1].c_str());
								int aa = 1;
								if (item_remove <= 0 || item_remove >= items.size()) break;
								if (a_[b_ + 1] == "1" or clear_all) {
									if (block_->mannequin.c_hair != item_remove and block_->mannequin.c_shirt != item_remove and block_->mannequin.c_pants != item_remove and block_->mannequin.c_feet != item_remove
										and block_->mannequin.c_head != item_remove and block_->mannequin.c_hand != item_remove and block_->mannequin.c_back != item_remove
										and block_->mannequin.c_mask != item_remove and block_->mannequin.c_neck != item_remove) return;
									if (Inventory::Modify(p_, item_remove, aa) == 0) {
										switch (items[item_remove].clothingType) {
										case 0: block_->mannequin.c_hair = 0;
											break;
										case 1: block_->mannequin.c_shirt = 0;
											break;
										case 2: block_->mannequin.c_pants = 0;
											break;
										case 3: block_->mannequin.c_feet = 0;
											break;
										case 4: block_->mannequin.c_head = 0;
											break;
										case 5: block_->mannequin.c_hand = 0;
											break;
										case 6: block_->mannequin.c_back = 0;
											break;
										case 7: block_->mannequin.c_mask = 0;
											break;
										case 8: block_->mannequin.c_neck = 0;
											break;
										}
									}
								}
							}
						}
						else if (a_[b_] == "buttonClicked") {
							if (a_.size() >= (b_ + 1) - 1) {
								if (a_[b_ + 1] == "clear") clear_all = true;
							}
						}
						else if (a_[b_] == "sign_text") {
							if (a_.size() >= (b_ + 1) - 1) {
								string txt = a_[b_ + 1];
								if (txt.size() > 128) break;
								block_->txt = txt;
							}
						}
					}
					tile_update(p_, world_, block_, x_, y_);
				}
			}
			return;
		}
		else if (a_.at(i_) == "vip_edit") {
			try {
				if (not isdigit(a_.at(i_ + 2).at(0)) or not isdigit(a_.at(i_ + 5).at(0))) break;
				int x_ = std::atoi(a_.at(i_ + 2).c_str()), y_ = std::atoi(a_.at(i_ + 5).c_str());
				string name_ = pInfo(p_)->world;
				std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
				if (p != worlds.end()) {
					World* world_ = &worlds[p - worlds.begin()];
					WorldBlock* block_ = &world_->blocks.at(x_ + (y_ * 100));
					uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
					if (items[t_].blockType != BlockTypes::VIP_ENTRANCE) break;
					if (to_lower(world_->owner_name) != to_lower(pInfo(p_)->tankIDName)) break;
					for (int b_ = 0; b_ < a_.size(); b_++) {
						if (a_.at(b_).substr(0, 9) == "checkbox_") {
							std::vector<std::string> target_ = explode("_", a_.at(b_));
							if (target_.at(1) == "public") {
								if (not block_->limit_admins and a_.at(b_ + 1) == "1") {
									block_->limit_admins = true;
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert(get_player_nick(p_) + " has set a `$VIP Entrance`` to `$PUBLIC");
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
										if (pInfo(cp_)->world == world_->name) {
											p.CreatePacket(cp_);
										}
									}
								}
								else if (block_->limit_admins and a_[b_ + 1] == "0") {
									block_->limit_admins = false;
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert(get_player_nick(p_) + " has set a `$VIP Entrance`` to `4PRIVATE``");
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
										if (pInfo(cp_)->world == world_->name) {
											p.CreatePacket(cp_);
										}
									}
								}
							}
							else if (target_.size() == 2 and a_.at(b_ + 1) == "0") {
								string user_name = target_.at(1);
								if (find(block_->admins.begin(), block_->admins.end(), to_lower(user_name)) != block_->admins.end()) {
									block_->admins.erase(remove(block_->admins.begin(), block_->admins.end(), to_lower(user_name)), block_->admins.end());
								}
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert(user_name + " was removed from a VIP List.");
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (to_lower(pInfo(cp_)->tankIDName) == to_lower(user_name)) {
										CAction::Log(cp_, "action|play_sfx\nfile|audio/dialog_cancel.wav\ndelayMS|0");
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(cp_)->netID);
										if (not Role::Moderator(p_) and not Role::Administrator(p_)) pInfo(p_)->name_color = "`2";
										p.Insert(get_player_nick(p_) + " has `4removed`` you from a VIP List in `w" + world_->name + "``.");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(cp_);
									}
									if (pInfo(cp_)->world == world_->name) {
										p.CreatePacket(cp_);
									}
								}
							}
						}
						else if (a_.at(b_) == "playerNetID") {
							if (a_.size() >= (b_ + 1) - 1) {
								if (not isdigit(a_.at(b_ + 1).at(0))) break;
								uint32_t netID = std::atoi(a_.at(b_ + 1).c_str());
								if (netID == pInfo(p_)->netID) {
									gamepacket_t p;
									p.Insert("OnTalkBubble");
									p.Insert(pInfo(p_)->netID);
									p.Insert("I'm already a VIP!");
									p.Insert(0), p.Insert(0);
									p.CreatePacket(p_);
									break;
								}
								if (block_->admins.size() >= 26 or block_->Admin_Bfg.size() >= 10) {
									gamepacket_t p;
									p.Insert("OnTalkBubble");
									p.Insert(pInfo(p_)->netID);
									p.Insert("VIP Entrance has reached access limit!");
									p.Insert(0);
									p.Insert(0);
									p.CreatePacket(p_);
									continue;
								}
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(cp_)->world == world_->name and pInfo(cp_)->netID == netID) {
										if (find(block_->admins.begin(), block_->admins.end(), to_lower(pInfo(cp_)->tankIDName)) != block_->admins.end()) {
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(p_)->netID);
											p.Insert(get_player_nick(cp_) + " is already on the VIP list.");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(p_);
											return;
										}
										block_->admins.push_back(to_lower(pInfo(cp_)->tankIDName));
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(p_)->netID);
										p.Insert("Put " + get_player_nick(cp_) + " on the VIP List.");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(p_);
										{
											gamepacket_t p;
											p.Insert("OnConsoleMessage");
											p.Insert(get_player_nick(cp_) + " was added to a VIP List.");
											for (ENetPeer* cp_2 = server->peers; cp_2 < &server->peers[server->peerCount]; ++cp_2) {
												if (cp_2->state != ENET_PEER_STATE_CONNECTED or cp_2->data == NULL) continue;
												if (pInfo(cp_2)->world == world_->name) {
													p.CreatePacket(cp_2);
												}
											}
										}
										break;
									}
								}
								break;
							}
						}
					}
					tile_update(p_, world_, block_, x_, y_);
				}
			}
			catch (out_of_range) {
				return;
			}
			catch (exception) {
				return;
			}
			return;
		}
		else if (a_[i_] == "weatherspcl") {
			if (a_.size() != 13 and a_.size() != 17 and a_.size() != 19) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
			if (x_ < 0 or y_ < 0) break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				if (y_ >= world_->max_y || x_ >= world_->max_x) break;
				WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				if (Only_Access(p_, world_, block_) == false && !guild_access(p_, world_->guild_id)) break;
				if (block_->fg != 5000 and block_->fg != 3832 and block_->fg != 3694) break;
				bool update_ = false;
				if (block_->fg == 3694 and a_.size() == 17) {
					if (not isdigit(a_[i_ + 8][0]) or not isdigit(a_[i_ + 10][0]) or not isdigit(a_[i_ + 12][0])) break;
					int r = std::atoi(a_[i_ + 8].c_str()), g = std::atoi(a_[i_ + 10].c_str()), b = std::atoi(a_[i_ + 12].c_str());
					if (r < 0 or g < 0 or b < 0 or r > 255 or g > 255 or b > 255) return;
					if (r < 40 and g < 40 and b < 40) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You can't make a heatwave that dark (one of the colors must be 40+)!", 0, 1);
						return;
					}
					block_->r = r;
					block_->g = g;
					block_->b = b;
					update_ = true;
				}
				else if (block_->fg == 3832 and a_.size() == 19) {
					if (not isdigit(a_[i_ + 8][0])) break;
					uint32_t new_id = std::atoi(a_[i_ + 8].c_str());
					if (new_id > items.size() or new_id < 0) break;
					if (new_id <= items.size()) {
						if (block_->id != new_id) {
							block_->id = new_id;
							update_ = true;
						}
					}
					int new_gravity = std::atoi(a_[i_ + 10].c_str());
					if (new_gravity > 500) new_gravity = 500;
					if (new_gravity < -500) new_gravity = -500;
					if (block_->gravity != new_gravity) {
						block_->gravity = new_gravity;
						update_ = true;
					}
					if (not isdigit(a_[i_ + 12][0])) break;
					bool new_spin = std::atoi(a_[i_ + 12].c_str());
					if (block_->spin != new_spin) block_->spin = new_spin, update_ = true;
					if (not isdigit(a_[i_ + 14][0])) break;
					bool new_invert = std::atoi(a_[i_ + 14].c_str());
					if (block_->invert != new_invert) block_->invert = new_invert, update_ = true;
				}
				else if (block_->fg == 3832 and a_.size() == 17) {
					int new_gravity = std::atoi(a_[i_ + 8].c_str());
					if (new_gravity > 500) new_gravity = 500;
					if (new_gravity < -500) new_gravity = -500;
					if (block_->gravity != new_gravity) {
						block_->gravity = new_gravity;
						update_ = true;
					}
					if (not isdigit(a_[i_ + 10][0])) break;
					bool new_spin = std::atoi(a_[i_ + 10].c_str());
					if (block_->spin != new_spin) block_->spin = new_spin, update_ = true;
					if (not isdigit(a_[i_ + 12][0])) break;
					bool new_invert = std::atoi(a_[i_ + 12].c_str());
					if (block_->invert != new_invert) block_->invert = new_invert, update_ = true;
				}
				else {
					if (not isdigit(a_[i_ + 8][0])) break;
					uint32_t new_id = std::atoi(a_[i_ + 8].c_str());
					if (new_id > items.size() or new_id < 0) break;
					if (block_->fg == 5000 and items[new_id].blockType != BlockTypes::BACKGROUND) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "That's not a background!", 0, 1);
						break;
					}
					if (block_->id != new_id) {
						block_->id = new_id;
						update_ = true;
					}
				}
				tile_update(p_, world_, block_, x_, y_);
				if (block_->enabled and update_) {
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						if (pInfo(cp_)->world == name_) {
							VarList::OnSetCurrentWeather(cp_, (Event()->Comet_Dust ? 16 : world_->weather == 0 ? 80 : world_->weather));
						}
					}
				}
			}
			break;
		}
		else if (a_[i_] == "itemaddedtosucker" or a_[i_] == "itemremovedfromsucker") {
			if (a_.size() != 13) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
			if (x_ < 0 or y_ < 0) break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				if (y_ >= world_->max_y || x_ >= world_->max_x) break;
				WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				string owner_name = to_lower(world_->owner_name), user_name = to_lower(pInfo(p_)->tankIDName);
				bool magplant_access = false;
				if (to_lower(world_->owner_name) == to_lower(pInfo(p_)->tankIDName) or Role::Administrator(p_)) magplant_access = true;
				if (world_->world_settings & SETTINGS::SETTINGS_10) {
					if (find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(p_)->tankIDName)) != world_->admins.end()) magplant_access = true;
				}
				if (block_->locked) {
					WorldBlock* check_lock = &world_->blocks[block_->lock_origin];
					if (to_lower(check_lock->owner_name) == to_lower(pInfo(p_)->tankIDName)) magplant_access = true;
				}
				if (items[block_->fg].blockType != BlockTypes::SUCKER or magplant_access == false) break;
				if (not isdigit(a_[i_ + 8][0])) break;
				if (t_ == 6948 and block_->pr >= 1500 and a_[i_] != "itemremovedfromsucker" or t_ == 6946 and block_->pr >= 1500 and a_[i_] != "itemremovedfromsucker" or t_ == 5638 and block_->pr >= 25000 and a_[i_] != "itemremovedfromsucker" or t_ == 9850 and block_->pr >= 50000 and a_[i_] != "itemremovedfromsucker" or t_ == 10266 and block_->pr >= 500000 and a_[i_] != "itemremovedfromsucker" or t_ == 5930 and block_->pr >= 1000000 and a_[i_] != "itemremovedfromsucker" or t_ == 21220 and block_->pr >= 2000000000 and a_[i_] != "itemremovedfromsucker") break;
				int add_count = std::atoi(a_[i_ + 8].c_str());
				if (a_[i_] == "itemremovedfromsucker") {
					int has_count = (block_->fg == 6954 ? block_->c_ : block_->pr), block_id = block_->id;
					if (block_->fg == 6954) {
						block_id = block_->invert ? block_->id : block_->id - 1;
					}
					if (add_count > has_count) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You are removing what you dont have.", 0, 1);
						break;
					}
					int c_ = 0;
					Inventory::Modify(p_, block_id, c_);
					add_count = (add_count + c_ > 200 ? (add_count > 200 ? 200 - c_ : (add_count > c_ ? 200 - add_count : 200 - c_)) : add_count);
					if (add_count > has_count or c_ + add_count > 200) break;
					int add_ = add_count;
					if (Inventory::Modify(p_, block_id, add_) == 0) {
						if (block_->fg == 6954)  block_->c_ -= add_count;
						else block_->pr -= add_count;
					}
				}
				else {
					int c_ = 0;
					Inventory::Modify(p_, block_->id, c_);
					if (c_ < add_count) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You dont have " + items[block_->id].name, 0, 1);
						VarList::OnConsoleMessage(p_, "You dont have " + items[block_->id].name);
						break;
					}
					int remove_count = add_count * -1;
					if (Inventory::Modify(p_, block_->id, remove_count) == 0) {
						block_->pr += add_count;
						VarList::OnConsoleMessage(p_, "Items added.");
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Items added.", 0, 1);
					}
				}
				bool found_ = false;
				for (int i_ = 0; i_ < world_->machines.size(); i_++) {
					WorldMachines* machine_ = &world_->machines[i_];
					if (machine_->x == x_ and machine_->y == y_) {
						machine_->enabled = block_->enabled;
						machine_->buildmode = block_->spin;
						machine_->target_item = block_->id;
						found_ = true;
						break;
					}
				}
				if (not found_) {
					WorldMachines new_machine;
					new_machine.enabled = block_->enabled;
					new_machine.buildmode = block_->spin;
					new_machine.x = x_, new_machine.y = y_;
					new_machine.id = block_->fg;
					new_machine.target_item = block_->id;
					world_->machines.push_back(new_machine);
					if (find(Environment()->Another_Worlds.begin(), Environment()->Another_Worlds.end(), world_->name) == Environment()->Another_Worlds.end()) {
						Environment()->Another_Worlds.push_back(world_->name);
					}
				}
				tile_update(p_, world_, block_, x_, y_);
			}
			break;
		}
		else if (a_[i_] == "itemsucker_block") {
			if (a_.size() != 13 and a_.size() != 16 and a_.size() != 14) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
			if (x_ < 0 or y_ < 0) break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				if (y_ >= world_->max_y || x_ >= world_->max_x || x_ < 0 || y_ < 0) break;
				int idx_ = x_ + (y_ * world_->max_x);
				if (idx_ < 0 || idx_ >= static_cast<int>(world_->blocks.size())) break;
				WorldBlock* block_ = &world_->blocks[idx_];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				if (Only_Access(p_, world_, block_) == false) break;
				if (items[t_].blockType != BlockTypes::SUCKER) break;
				if (a_.size() == 14 or a_[11] == "getplantationdevice") {
					if (block_->id != 0 and (block_->fg == 5638 or block_->fg == 5930 or block_->fg == 9850 or block_->fg == 10266 or block_->fg == 21220)) {
						int c_ = 0;
						Inventory::Modify(p_, 5640, c_);
						if (c_ != 0) Inventory::Modify(p_, 5640, c_ *= -1);
						c_ = 1;
						if (Inventory::Modify(p_, 5640, c_) == 0) {
							VarList::OnConsoleMessage(p_, "You received a " + items[5640].name + ".");
							VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You received a " + items[5640].name + ".", 0, 0);
							pInfo(p_)->magnetron_id = block_->id;
							pInfo(p_)->magnetron_x = x_;
							pInfo(p_)->magnetron_y = y_;
							VarList::OnPlanterActivated(p_, block_->id, x_, y_);
						}
					}
					break;
				}
				if (a_.size() == 13 && a_[i_ + 7] == "selectitem") {
					if (!isdigit(a_[i_ + 8][0])) break;
					uint32_t putting_item = std::atoi(a_[i_ + 8].c_str());
					if (putting_item >= items.size() || putting_item < 0) break;
					if (t_ == 5638 || t_ == 5930 || t_ == 6948 || t_ == 6946 || t_ == 9850 || t_ == 10266 || t_ == 21220) {
						if (block_->pr != 0) break;
						if ((t_ == 6948 && items[putting_item].blockType == BlockTypes::SEED) ||
							(t_ == 6946 && items[putting_item].blockType != BlockTypes::SEED)) {
							VarList::OnTalkBubble(p_, pInfo(p_)->netID,
								(t_ == 6948 ? "You cannot store seeds in this machine." : "You can only store seeds in this machine."), 0, 1);
							VarList::OnConsoleMessage(p_,
								(t_ == 6948 ? "You cannot store seeds in this machine." : "You can only store seeds in this machine."));
							break;
						}
						if (items[putting_item].untradeable || items[putting_item].blockType == BlockTypes::LOCK) {
							VarList::OnTalkBubble(p_, pInfo(p_)->netID, "This item is not compatible.", 0, 1);
							VarList::OnConsoleMessage(p_, "This item is not compatible.");
							break;
						}
						if (items[putting_item].blockType == BlockTypes::SUCKER || items[putting_item].id == 6016 || items[putting_item].id == 6952) {
							putting_item = 0;
						}
						block_->id = putting_item;
						block_->enabled = true;
					}
				}
				else {
					for (int b_ = 0; b_ < a_.size(); b_++) {
						if (a_[b_] == "retrieveitem") {
							if (block_->id != 0) {
								int c_ = 0;
								Inventory::Modify(p_, block_->id, c_);
								if (c_ < 200) {
									VarList::OnDialogRequest(p_, "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_spacer|small|\nembed_data|tilex|" + to_string(x_) + "\nembed_data|tiley|" + to_string(y_) + "\nadd_label_with_icon|small|`2" + items[block_->id].name + "``|left|" + to_string(block_->id) + "|\nadd_textbox|`wHow many `2" + items[block_->id].name + "`` would you like to remove?``|left||\nadd_text_input|itemtoremove|Amount:|" + (block_->pr + c_ > 200 ? (block_->pr > 200 ? to_string(200 - c_) : (block_->pr > c_ ? to_string(200 - block_->pr) : to_string(200 - c_))) : to_string(block_->pr)) + "|20|\nend_dialog|itemremovedfromsucker|Close|Retrieve|\nadd_spacer|small|");
								}
							}
							break;
						}
						else if (a_[b_] == "addstocks5000") {
							if (Role::has_config_access(p_)) {
								block_->pr += 5000;
								VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Added 5000 blocks to this machine!", 0, 1);
							}
							break;
						}
						else if (a_[b_] == "additem") {
							if (block_->id != 0) {
								int c_ = 0;
								Inventory::Modify(p_, block_->id, c_);
								if (c_ != 0) {
									VarList::OnDialogRequest(p_, "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_spacer|small|\nembed_data|tilex|" + to_string(x_) + "\nembed_data|tiley|" + to_string(y_) + "\nadd_label_with_icon|small|`2" + items[block_->id].name + "``|left|" + to_string(block_->id) + "|\nadd_smalltext|You have " + to_string(c_) + " `2" + items[block_->id].name + "`` in your backpack.|left|\nadd_textbox|`wHow many `2" + items[block_->id].name + "`` would you like to add?``|left|\nadd_text_input|itemtoadd|Amount:|" + (block_->pr + c_ > (block_->fg == 21220 ? 2000000000 : block_->fg == 5638 ? 25000 : block_->fg == 9850 ? 50000 : block_->fg == 10266 ? 500000 : block_->fg == 5930 ? 2000000000 : 1500) ? to_string((block_->fg == 21220 ? 2000000000 : block_->fg == 5638 ? 25000 : block_->fg == 9850 ? 50000 : block_->fg == 10266 ? 500000 : block_->fg == 5930 ? 2000000000 : 1500) - block_->pr) : to_string(c_)) + "|20|\nend_dialog|itemaddedtosucker|Close|Add|\nadd_spacer|small|");
								}
							}
							break;
						}
						else if (a_[b_] == "clearitem") {
							if (block_->id != 0 and block_->pr == 0) {
								if (block_->fg == 5638 || block_->fg == 5930 || block_->fg == 9850 || block_->fg == 10266 || block_->fg == 21220) {
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != pInfo(p_)->world) continue;
										if (pInfo(cp_)->magnetron_x == x_ && pInfo(cp_)->magnetron_y == y_) {
											int c_ = 0;
											Inventory::Modify(cp_, (block_->fg == 5638 ? 5640 : 9158), c_);
											if (c_ != 0) {
												int remove_c = -c_;
												Inventory::Modify(cp_, (block_->fg == 5638 ? 5640 : 9158), remove_c);
											}
											pInfo(cp_)->magnetron_id = 0;
											pInfo(cp_)->magnetron_x = 0;
											pInfo(cp_)->magnetron_y = 0;
										}
									}
								}
								block_->id = 0, block_->enabled = true;
								VarList::OnDialogRequest(p_, get_sucker(p_, world_, block_, x_, y_));
								break;
							}
							break;
						}
						else if (a_[b_].substr(0, 4) == "chk_") {
							std::vector<std::string> target_ = explode("_", a_[b_]);
							if (target_[1] == "enablesucking") {
								block_->enabled = (a_[b_ + 1] == "1" and not block_->enabled ? true : (a_[b_ + 1] == "0" and block_->enabled ? false : false));
							}
						}
					}
				}
				bool found_ = false;
				for (int i_ = 0; i_ < world_->machines.size(); i_++) {
					WorldMachines* machine_ = &world_->machines[i_];
					if (machine_->x == x_ and machine_->y == y_) {
						machine_->enabled = block_->enabled;
						machine_->buildmode = block_->spin;
						machine_->target_item = block_->id;
						found_ = true;
						break;
					}
				}
				if (not found_) {
					WorldMachines new_machine;
					new_machine.enabled = block_->enabled;
					new_machine.buildmode = block_->spin;
					new_machine.x = x_, new_machine.y = y_;
					new_machine.id = block_->fg;
					new_machine.target_item = block_->id;
					world_->machines.push_back(new_machine);
					if (find(Environment()->Another_Worlds.begin(), Environment()->Another_Worlds.end(), world_->name) == Environment()->Another_Worlds.end()) {
						Environment()->Another_Worlds.push_back(world_->name);
					}
				}
				tile_update(p_, world_, block_, x_, y_);
			}
			break;
		}
		else if (a_[i_] == "autoactionblock") {
			if (a_.size() == 4) return;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
			if (x_ < 0 or y_ < 0) break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				if (y_ >= world_->max_y || x_ >= world_->max_x || x_ < 0 || y_ < 0) break;
				int idx_ = x_ + (y_ * world_->max_x);
				if (idx_ < 0 || idx_ >= static_cast<int>(world_->blocks.size())) break;
				WorldBlock* block_ = &world_->blocks[idx_];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				string owner_name = to_lower(world_->owner_name), user_name = to_lower(pInfo(p_)->tankIDName);
				if (owner_name != user_name and not Role::Administrator(p_)) {
					if (block_->locked) {
						WorldBlock* check_lock = &world_->blocks[block_->lock_origin];
						if (check_lock->owner_name != pInfo(p_)->tankIDName) break;
					}
					else {
						break;
					}
				}
				if (items[t_].blockType == BlockTypes::AUTO_BLOCK or t_ == 6954) {
					for (int b_ = 0; b_ < a_.size(); b_++) {
						if (a_[b_] == "removegems" && block_->pr > 0) {
							VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You have removed `4" + Set_Count(block_->pr) + "`` gems from this machine!", 0, 1);
							VarList::OnBuxGems(p_, block_->pr);
							block_->pr = 0;
						}
						else if (a_[b_] == "removeitems") {
							if (block_->c_ != 0) {
								int c_ = 0, id_ = (block_->invert ? block_->id : block_->id - 1);
								Inventory::Modify(p_, id_, c_);
								if (c_ < 200) {
									VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_spacer|small|\nembed_data|tilex|" + to_string(x_) + "\nembed_data|tiley|" + to_string(y_) + "\nadd_label_with_icon|small|`2" + items[id_].name + "``|left|" + to_string(id_) + "|\nadd_textbox|`wHow many `2" + items[id_].name + "`` would you like to remove?``|left||\nadd_text_input|itemtoremove|Amount:|" + (block_->c_ + c_ > 200 ? (block_->c_ > 200 ? to_string(200 - c_) : (block_->c_ > c_ ? to_string(200 - block_->c_) : to_string(200 - c_))) : to_string(block_->c_)) + "|20|\nend_dialog|itemremovedfromsucker|Close|Retrieve|\nadd_spacer|small|");
								}
							}
						}
						else if (a_[b_] == "refillfuel" && block_->pr < 8000) {
							if (not isdigit(a_[b_ + 1][0])) return;
							int target_amount = std::atoi(a_[b_ + 1].c_str());
							if (target_amount > pInfo(p_)->gems) target_amount = pInfo(p_)->gems;
							if (target_amount == 0) break;
							if (block_->pr + target_amount > 8000) {
								target_amount = 8000 - block_->pr;
							}
							VarList::OnBuxGems(p_, target_amount * -1);
							block_->pr += target_amount;
						}
						else if (a_[b_] == "harvest_trees" && block_->c_ <= 0) {
							if (not isdigit(a_[b_ + 1][0])) return;
							int target_amount = std::atoi(a_[b_ + 1].c_str());
							block_->build_only = target_amount;
						}
						else if (a_[b_] == "collect_seeds" && block_->c_ <= 0) {
							if (not isdigit(a_[b_ + 1][0])) return;
							int target_amount = std::atoi(a_[b_ + 1].c_str());
							block_->invert = target_amount;
						}
						else if (a_[b_] == "selecttarget") {
							if (not isdigit(a_[b_ + 1][0])) return;
							uint32_t target_item = std::atoi(a_[b_ + 1].c_str());
							if (target_item <= 0 || target_item > items.size() || block_->c_ != 0) return;
							if (t_ == 6952) {
								if (items[target_item].blockType == BlockTypes::SEED) target_item--;
								if (items[target_item].untradeable || items[target_item].rarity < 0 || items[target_item].rarity == 999 || items[target_item].block_possible_put == false || target_item == 610 || target_item == 9385 || target_item == 9387 || target_item == 611 || target_item == 9380 || target_item == 9386 || target_item == 5136 || target_item == 9600 || items[target_item].untradeable || items[target_item].rarity == 999 || target_item == 9380 || target_item == 9386 || target_item == 9384 || target_item == 9902 || target_item == 10716 || target_item == 5136 || target_item == 9600 || target_item == 9381 || target_item == 9387 || target_item == 9385 || target_item == 5137 || target_item == 9601) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Sorry that item is `4NOT`` allowed in this machine!", 0, 1);
									VarList::OnConsoleMessage(p_, "Sorry that item is `4NOT`` allowed in this machine!");
									return;
								}
								if (items[target_item].blockType == BlockTypes::CLOTHING or items[target_item].properties & Property_Mod) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`4NOPE!``", 0, 1);
									VarList::OnConsoleMessage(p_, "`4NOPE!``");
									return;
								}
							}
							else {
								if (items[target_item].blockType != BlockTypes::SEED) target_item++;
								if (target_item == 611 || items[target_item].rarity == 999 || target_item == 9380 || target_item == 9386 || target_item == 9384 || target_item == 9902 || target_item == 10716 || target_item == 5136 || target_item == 9600 || target_item == 9381 || target_item == 9387 || target_item == 9385 || target_item == 5137 || target_item == 9601) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Sorry that item is `4NOT`` allowed in this machine!", 0, 1);
									VarList::OnConsoleMessage(p_, "Sorry that item is `4NOT`` allowed in this machine!");
									return;
								}
								if (items[target_item].collisionType != 1 and items[target_item].blockType != BlockTypes::SEED) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`4NOPE!``", 0, 1);
									VarList::OnConsoleMessage(p_, "`4NOPE!``");
									return;
								}
								if (t_ == 6954) {
									if (items[target_item].blockType == BlockTypes::CLOTHING or items[target_item].properties & Property_Mod) {
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`4NOPE!``", 0, 1);
										VarList::OnConsoleMessage(p_, "`4NOPE!``");
										return;
									}
								}
							}
							block_->id = target_item;
						}
					}
					bool found_ = false;
					for (int i_ = 0; i_ < world_->machines.size(); i_++) {
						WorldMachines* machine_ = &world_->machines[i_];
						if (machine_->x == x_ and machine_->y == y_) {
							machine_->enabled = block_->enabled;
							machine_->buildmode = block_->spin;
							machine_->target_item = block_->id;
							found_ = true;
							break;
						}
					}
					if (not found_) {
						WorldMachines new_machine;
						new_machine.enabled = block_->enabled;
						new_machine.buildmode = block_->spin;
						new_machine.x = x_, new_machine.y = y_;
						new_machine.id = block_->fg;
						new_machine.target_item = block_->id;
						world_->machines.push_back(new_machine);
						if (find(Environment()->Another_Worlds.begin(), Environment()->Another_Worlds.end(), world_->name) == Environment()->Another_Worlds.end()) {
							Environment()->Another_Worlds.push_back(world_->name);
						}
					}
					tile_update(p_, world_, block_, x_, y_);
					break;
				}
			}
			break;
		}
		else if (a_[i_] == "blaster") {
			if (a_.size() != 17) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0]) or not isdigit(a_[i_ + 8][0]) or not isdigit(a_[i_ + 10][0]) or not isdigit(a_[i_ + 12][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str()), often_ = std::atoi(a_[i_ + 8].c_str()), speed_ = std::atoi(a_[i_ + 10].c_str());
			if (x_ < 0 or y_ < 0) break;
			if (speed_ > 250) speed_ = 250;
			if (speed_ < 10) speed_ = 10;
			if (often_ > 20) often_ = 20;
			if (often_ < 2) often_ = 2;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				if (y_ >= world_->max_y || x_ >= world_->max_x) break;
				WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				if (Only_Access(p_, world_, block_) == false || items[t_].blockType != BlockTypes::TRICKSTER) break;
				block_->flags = (block_->flags & 0x00400000 and a_[i_ + 12] == "1" ? block_->flags ^ 0x00400000 : (a_[i_ + 12] == "0" ? block_->flags | 0x00400000 : block_->flags));
				bool found_ = false;
				block_->rate_of_fire = often_;
				block_->projectile_speed = speed_;
				for (int i_ = 0; i_ < world_->npc.size(); i_++) {
					WorldNPC* npc_ = &world_->npc[i_];
					if (npc_->x == x_ and npc_->y == y_) {
						npc_->enabled = (block_->flags & 0x00400000 ? true : false);
						npc_->rate_of_fire = often_;
						npc_->projectile_speed = speed_;
						npc_->kryptis = (block_->flags & 0x00200000 ? 180 : 0);
						found_ = true;
						break;
					}
				}
				if (not found_) {
					WorldNPC new_npc;
					new_npc.enabled = (block_->flags & 0x00400000 ? true : false);
					new_npc.x = x_, new_npc.y = y_;
					new_npc.id = block_->fg;
					new_npc.projectile_speed = speed_;
					new_npc.rate_of_fire = often_;
					new_npc.kryptis = (block_->flags & 0x00200000 ? 180 : 0);
					world_->npc.push_back(new_npc);
					if (find(Environment()->Another_Worlds.begin(), Environment()->Another_Worlds.end(), world_->name) == Environment()->Another_Worlds.end()) {
						Environment()->Another_Worlds.push_back(world_->name);
					}
				}
				block_->enabled = (block_->flags & 0x00400000 ? true : false);
				tile_update(p_, world_, block_, x_, y_);
				VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Settings updated!", 0, 1);
			}
			break;
		}
		else if (a_[i_] == "switcheroo_edit") {
			if (a_.size() < 13) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
			if (x_ < 0 or y_ < 0) break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				if (y_ >= world_->max_y || x_ >= world_->max_x) break;
				WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				if (Only_Access(p_, world_, block_) && items[t_].blockType == BlockTypes::GATEWAY or items[t_].can_be_changed_to_public) {
					block_->flags = (block_->flags & 0x00800000 and a_[i_ + 8] == "0" ? block_->flags ^ 0x00800000 : (a_[i_ + 8] == "1" ? block_->flags | 0x00800000 : block_->flags));
					if (a_.size() == 15) block_->flags = (block_->flags & 0x02000000 and a_[i_ + 10] == "0" ? block_->flags ^ 0x02000000 : (a_[i_ + 10] == "1" ? block_->flags | 0x02000000 : block_->flags));
					tile_update(p_, world_, block_, x_, y_);
				}
			}
			break;
		}
		else if (a_[i_] == "door_edit") {
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				WorldBlock* block_ = &world_->blocks[pInfo(p_)->lastwrenchx + (pInfo(p_)->lastwrenchy * 100)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				if (items[t_].blockType != BlockTypes::DOOR and items[t_].blockType != BlockTypes::PORTAL) break;
				if (Only_Access(p_, world_, block_) == false && !guild_access(p_, world_->guild_id)) break;
				if (a_.size() >= 8 && block_->fg == 10358) {
					string btn = a_[i_ + 8];
					if (btn == "withdraw_a") {
						if (block_->wl != 0) {
							int total_wl = block_->wl, c_ = 0, taken = 0;
							bool returned = false, collect = false;
							string collected_ = "";
							if (pInfo(p_)->tankIDName != world_->owner_name) {
								ServerPool::Logs::Add("player: " + pInfo(p_)->tankIDName + " lvl: " + to_string(pInfo(p_)->level) + ", took " + to_string(total_wl) + " World Locks from Entry Booth in: [" + pInfo(p_)->world + "]", "Entry Booth (Steal)");
							}
							if (total_wl >= 10000) {
								if (total_wl > 2000000) c_ = 200;
								else c_ = total_wl / 10000;
								total_wl -= c_ * 10000;
								block_->wl -= c_ * 10000;
								taken = c_;
								if (Inventory::Modify(p_, 7188, c_) == 0) {
									collected_ += to_string(taken) + " Platinum Gem Locks";
									collect = true;
								}
								else pInfo(p_)->bgl_bank_amount += c_, returned = true;
								c_ = 0;
							}
							if (total_wl >= 100) {
								if (total_wl > 20000) c_ = 200;
								else c_ = total_wl / 100;
								total_wl -= c_ * 100;
								block_->wl -= c_ * 100;
								taken = c_;
								if (Inventory::Modify(p_, 1796, c_) == 0) {
									collected_ += (collected_.empty() ? "" : " and ") + to_string(taken) + " Diamond Locks";
									collect = true;
								}
								else pInfo(p_)->dl_bank_amount += c_, returned = true;
								c_ = 0;
							}
							if (total_wl >= 1) {
								if (total_wl > 200) c_ = 200;
								else c_ = total_wl;
								total_wl -= c_;
								block_->wl -= c_;
								taken = c_;
								if (Inventory::Modify(p_, 242, c_) == 0) {
									collected_ += (collected_.empty() ? "" : " and ") + to_string(taken) + " World Locks";
									collect = true;
								}
								else pInfo(p_)->wl_bank_amount += c_, returned = true;
								c_ = 0;
							}
							if (returned) {
								VarList::OnTextOverlay(p_, "Some of the World Locks could not fit in your inventory!");
								VarList::OnTextOverlay(p_, "We have placed them in your Personal Backpack.", 1500);
							}
							{
								VarList::OnTalkBubble(p_, pInfo(p_)->netID, (collect ? "You collected " + collected_ + "" : "You don't have room in your backpack!"), 0, 1);
								VarList::OnConsoleMessage(p_, (collect ? "You collected " + collected_ + "" : "You don't have room in your backpack!"));
							}
						}
						return;
					}
				}
				if (a_.size() != 19 and a_.size() != 17) break;
				if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
				string door_name = a_[i_ + 8];
				string door_target = a_[i_ + 10];
				string door_id = a_[i_ + 12];
				if (block_->fg == 10358) {
					int entry_price = std::atoi(door_name.c_str());
					if (entry_price < 0 || entry_price >(not Role::Administrator(p_) ? 200 : 20000)) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Minimum price is 0 and maximum is 200 World Locks!");
						entry_price = block_->shelf_1;
					}
					else block_->shelf_1 = entry_price;
				}
				if (a_.size() == 19) {
					string checkbox_locked = a_[i_ + 14];
					block_->open = (checkbox_locked == "0" ? false : true);
				}
				if (door_name.size() > 100) break;
				if (door_target.size() > 24) break;
				if (door_id.size() > 11) break;
				transform(door_target.begin(), door_target.end(), door_target.begin(), ::toupper);
				transform(door_id.begin(), door_id.end(), door_id.begin(), ::toupper);
				block_->door_destination = door_target, block_->door_id = door_id;
				block_->txt = door_name;
				if (block_->fg == 10358) block_->txt = "`$[```4ENTRY PRICE```$]``: " + to_string(block_->shelf_1) + " ?";
				tile_update(p_, world_, block_, pInfo(p_)->lastwrenchx, pInfo(p_)->lastwrenchy);
			}
			break;
		}
		else if (a_[i_] == "vending") {
			if (a_.size() != 14 and a_.size() != 13 and a_.size() != 20 and a_.size() != 17 and a_.size() != 19 and a_.size() != 23) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			if (a_.size() == 13 and not isdigit(a_[i_ + 8][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
			if (x_ < 0 or y_ < 0) break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				if (y_ >= world_->max_y || x_ >= world_->max_x) break;
				WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				if (items[t_].blockType != BlockTypes::VENDING) break;
				bool access = Only_Access(p_, world_, block_, true);
				if ((std::abs(x_ * 32 - pInfo(p_)->x) > 15 || std::abs(y_ * 32 - pInfo(p_)->y) > 15) && access == false) {
					VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Get closer!", 0, 0);
					break;
				}
				if (a_.size() == 19 or a_.size() == 23) {
					if (not isdigit(a_[i_ + 11][0])) break;
					int expectprice = std::atoi(a_[i_ + 8].c_str()), expectitem = std::atoi(a_[i_ + 11].c_str()), buycount = std::atoi(a_[i_ + 14].c_str());
					if (a_.size() == 23) {
						buycount = std::atoi(a_[i_ + 11].c_str()), expectprice = std::atoi(a_[i_ + 14].c_str()), expectitem = std::atoi(a_[i_ + 17].c_str());
					}
					if (expectprice != block_->pr) break;
					if (expectitem != block_->id) break;
					if (buycount < 0) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "NEGATIVE, DOES NOT COMPUTE", 0, 0);
						break;
					}
					if (buycount == 0) break;
					int normal_price = (block_->pr > 0 ? block_->pr : (block_->pr * -1));
					if (normal_price == 0) break;
					int actual_buy_count = (block_->pr > 0 ? buycount : (normal_price < buycount ? buycount / normal_price : normal_price / buycount) * (block_->pr * -1));
					if (actual_buy_count == 0 or actual_buy_count > block_->c_) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "There aren't enough available to fulfill your order!", 0, 1);
						VarList::OnConsoleMessage(p_, "There aren't enough available to fulfill your order!");
						break;
					}
					if (block_->pr < 0) {
						if (buycount < (block_->pr * -1)) {
							VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You have to buy at least 1 World Lock's worth.", 0, 1);
							VarList::OnConsoleMessage(p_, "You have to buy at least 1 World Lock's worth.");
							break;
						}
					}
					int my_wls = get_wls(p_, true);
					int cost_ = (block_->pr > 0 ? block_->pr * buycount : (normal_price < buycount ? buycount / normal_price : normal_price / buycount));
					if (cost_ > my_wls) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You can't afford that many!", 0, 1);
						VarList::OnConsoleMessage(p_, "You can't afford that many!");
						break;
					}
					int has_alr = 0;
					Inventory::Modify(p_, block_->id, has_alr);
					if (has_alr + actual_buy_count > 200) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You don't have room in your backpack!", 0, 1);
						VarList::OnConsoleMessage(p_, "You don't have room in your backpack!");
						break;
					}
					int free_slots = Inventory::Get_Slots(pInfo(p_));
					if (free_slots == 0) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You don't have room in your backpack!", 0, 1);
						VarList::OnConsoleMessage(p_, "You don't have room in your backpack!");
						break;
					}
					if (a_.size() == 19) {
						VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wPurchase Confirmation``|left|1366|\nadd_spacer|small|\nembed_data|tilex|" + to_string(x_) + "\nembed_data|tiley|" + to_string(y_) + "\nadd_spacer|small|\nadd_textbox|`4You'll give:``|left|\nadd_spacer|small|\nadd_label_with_icon|small|(`w" + to_string(cost_) + "``) `8World Locks``|left|242|\nadd_spacer|small|\nadd_textbox|`2You'll get:``|left|\nadd_spacer|small|\nadd_label_with_icon|small|(`w" + to_string(actual_buy_count) + "``) `2" + items[block_->id].ori_name + "``|left|" + to_string(block_->id) + "|\nadd_spacer|small|\nadd_spacer|small|\nadd_textbox|Are you sure you want to make this purchase?|left|\nembed_data|verify|1\nembed_data|buycount|" + to_string(buycount) + "\nembed_data|expectprice|" + to_string(expectprice) + "\nembed_data|expectitem|" + to_string(expectitem) + "\nend_dialog|vending|Cancel|OK|");
						break;
					}
					int a_ = actual_buy_count;
					if (Inventory::Get_Slots(pInfo(p_)) > 2) {
						if (Inventory::Modify(p_, block_->id, a_) == 0) {
							get_wls(p_, true, true, cost_);
							ServerPool::Logs::Add(pInfo(p_)->tankIDName + " buy " + to_string(actual_buy_count) + "x " + items[block_->id].ori_name + " for " + to_string(cost_) + " World Locks" + " in World: [" + pInfo(p_)->world + "]", "Vending Machine Buying");
							ServerPool::CctvLogs::Add(p_, "bought", "" + to_string(actual_buy_count) + " " + items[block_->id].ori_name + " for " + to_string(cost_) + " World Locks");
							if (block_->pr > 0) add_item_price(block_->id, block_->pr);
							PlayerMoving data_{};
							data_.x = x_ * 32 + 16, data_.y = y_ * 32 + 16;
							data_.packetType = 19, data_.plantingTree = 100;
							data_.punchX = block_->id, data_.punchY = pInfo(p_)->netID;
							int32_t to_netid = pInfo(p_)->netID;
							BYTE* raw = packPlayerMoving(&data_);
							raw[3] = 5;
							Memory_Copy(raw + 8, &to_netid, 4);
							block_->c_ -= actual_buy_count, block_->wl += cost_;
							if (block_->c_ <= 0) {
								block_->id = 0, block_->pr = 0;
							}
							PlayerMoving data_vending{};
							data_vending.packetType = 5, data_vending.punchX = x_, data_vending.punchY = y_, data_vending.characterState = 0x8;
							BYTE* v_ = packPlayerMoving(&data_vending, 112 + alloc_(world_, block_));
							BYTE* blc = v_ + 56;
							form_visual(blc, *block_, *world_, p_, false);
							for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
								if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
								if (pInfo(cp_)->world == world_->name) {
									send_raw(cp_, 4, v_, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
									VarList::OnConsoleMessage(cp_, "`7[```9" + pInfo(p_)->tankIDName + " bought " + to_string(actual_buy_count) + " " + items[block_->id].ori_name + " for " + to_string(cost_) + " World Locks.```7]``");
									CAction::Positioned(cp_, pInfo(p_)->netID, "audio/cash_register.wav", 0);
									send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
								}
							}
							delete[] raw; // FIX MEMLEAK: dulu `delete[]raw, v_, blc;` (comma op) ? v_ leak.
							delete[] v_;  // blc adalah pointer offset (v_ + 56), JANGAN di-delete.
							if (block_->locked) {
								upd_lock(*block_, *world_, p_);
							}
							break;
						}
					}
					VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You don't have room in your backpack!", 0, 1);
					VarList::OnConsoleMessage(p_, "You don't have room in your backpack!");
					break;
				}
				if (access == false) break;
				bool update_ = false;
				if (cch.find("chk_peritem|1") != string::npos and cch.find("chk_perlock|1") != string::npos) {
					VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You can't have it both ways.", 0, 1);
					VarList::OnConsoleMessage(p_, "You can't have it both ways.");
					break;
				}
				if (a_.size() == 20 or a_.size() == 17 or a_.size() == 14) {
					for (int b_ = 0; b_ < a_.size(); b_++) {
						if (a_[b_].substr(0, 4) == "chk_") {
							std::vector<std::string> target_ = explode("_", a_[b_]);
							if (target_[1] == "peritem") {
								if (block_->pr < 0 and a_[b_ + 1] == "1") {
									block_->pr *= -1, update_ = true;
								}
							}
							else if (target_[1] == "perlock") {
								if (block_->pr > 0 and a_[b_ + 1] == "1") {
									if (block_->pr > 200 or block_->pr > block_->c_) {
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You don't have enough in there for anyone to buy at that price!", 0, 1);
										VarList::OnConsoleMessage(p_, "You don't have enough in there for anyone to buy at that price!");
										update_ = false;
										break;
									}
									block_->pr = block_->pr * -1;
									update_ = true;
								}
							}
						}
						else if (a_[b_] == "setprice") {
							int price_ = std::atoi(a_[b_ + 1].c_str());
							if (price_ == 0 or not isdigit(a_[b_ + 1][0]) and price_ >= 0) {
								block_->pr = 0;
								update_ = true;
							}
							else if (price_ < 0 or price_ > 2000000) {
								VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Well, that's a silly price.", 0, 1);
								break;
							}
							else {
								if (block_->pr < 0) {
									if (price_ > block_->c_) {
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You don't have enough in there for anyone to buy at that price!", 0, 1);
										VarList::OnConsoleMessage(p_, "You don't have enough in there for anyone to buy at that price!");
										update_ = false;
										break;
									}
									if (price_ > 200) {
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, "That price is so low, nobody can carry a World Lock worth of items!", 0, 1);
										VarList::OnConsoleMessage(p_, "That price is so low, nobody can carry a World Lock worth of items!");
										break;
									}
								}
								block_->pr = (block_->pr < 0 ? price_ * -1 : price_);
								update_ = true;
							}
						}
						else if (a_[b_] == "addstocks") {
							if (block_->id != 0) {
								int c_ = 0;
								Inventory::Modify(p_, block_->id, c_);
								int has_ = c_;
								c_ *= -1;
								if (Inventory::Modify(p_, block_->id, c_) == 0) {
									block_->c_ += has_;
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Added " + to_string(has_) + " items to the machine.", 0, 1);
									VarList::OnConsoleMessage(p_, "Added " + to_string(has_) + " items to the machine.");
								}
							}
							update_ = false;
							break;
						}
						else if (a_[b_] == "addstocks5000") {
							if (block_->id != 0) {
								if (Role::has_config_access(p_)) {
									block_->c_ += 5000;
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Added 5000 items to the machine.", 0, 1);
									VarList::OnConsoleMessage(p_, "Added 5000 items to the machine.");
								}
							}
							update_ = false;
							break;
						}
						else if (a_[b_] == "withdraw") {
							if (access) {
								if (block_->wl != 0) {
									int total_wl = block_->wl, c_ = 0, taken = 0;
									bool returned = false, collect = false;
									string collected_ = "";
									if (pInfo(p_)->tankIDName != world_->owner_name) ServerPool::Logs::Add("player: " + pInfo(p_)->tankIDName + " lvl: " + to_string(pInfo(p_)->level) + ", took " + to_string(total_wl) + " World Locks from Vending Machine in: [" + pInfo(p_)->world + "]", "Vending Machine (Steal)");
									if (total_wl >= 10000) {
										if (total_wl > 2000000) c_ = 200;
										else c_ = total_wl / 10000;
										total_wl -= c_ * 10000;
										block_->wl -= c_ * 10000;
										taken = c_;
										if (Inventory::Modify(p_, 7188, c_) == 0) {
											collected_ += to_string(taken) + " Platinum Gem Locks";
											collect = true;
										}
										else pInfo(p_)->bgl_bank_amount += c_, returned = true;
										c_ = 0;
									}
									if (total_wl >= 100) {
										if (total_wl > 20000) c_ = 200;
										else c_ = total_wl / 100;
										total_wl -= c_ * 100;
										block_->wl -= c_ * 100;
										taken = c_;
										if (Inventory::Modify(p_, 1796, c_) == 0) {
											collected_ += (collected_.empty() ? "" : " and ") + to_string(taken) + " Diamond Locks";
											collect = true;
										}
										else pInfo(p_)->dl_bank_amount += c_, returned = true;
										c_ = 0;
									}
									if (total_wl >= 1) {
										if (total_wl > 200) c_ = 200;
										else c_ = total_wl;
										total_wl -= c_;
										block_->wl -= c_;
										taken = c_;
										if (Inventory::Modify(p_, 242, c_) == 0) {
											collected_ += (collected_.empty() ? "" : " and ") + to_string(taken) + " World Locks";
											collect = true;
										}
										else pInfo(p_)->wl_bank_amount += c_, returned = true;
										c_ = 0;
									}
									if (returned) {
										VarList::OnTextOverlay(p_, "Some of the World Locks could not fit in your inventory!");
										VarList::OnTextOverlay(p_, "We have placed them in your Personal Backpack.", 1500);
									}
									{
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, (collect ? "You collected " + collected_ + "" : "You don't have room in your backpack!"), 0, 1);
										VarList::OnConsoleMessage(p_, (collect ? "You collected " + collected_ + "" : "You don't have room in your backpack!"));
									}
								}
								update_ = false;
							}
							break;
						}
						else if (a_[b_] == "upgradedigital") {
							if (access) {
								VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wVending Machine|left|2978|\nembed_data|tilex|" + to_string(x_) + "\nembed_data|tiley|" + to_string(y_) + "\nadd_spacer|small|\nadd_textbox|`$Are you sure you want to upgrade to a DigiVend Machine for `44.000 Gems`$?|left|\nadd_spacer|small|\nend_dialog|Upgrade_DigiVend|Close|Upgrade|");
								break;
							}
						}
						else if (a_[b_] == "pullstocks") {
							if (access) {
								if (Inventory::Get_Slots(pInfo(p_)) > 1) {
									int c_ = block_->c_;
									if (Inventory::Modify(p_, block_->id, c_) == 0) {
										VarList::OnConsoleMessage(p_, "You picked up " + to_string(block_->c_) + " " + items[block_->id].ori_name + ".");
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You picked up " + to_string(block_->c_) + " " + items[block_->id].ori_name + ".", 0, 1);
										block_->c_ = 0, block_->id = 0, block_->pr = 0;
									}
									else if (block_->c_ > 200) {
										int has_ = 0;
										Inventory::Modify(p_, block_->id, has_);
										int give_ = 200 - has_;
										block_->c_ -= give_;
										VarList::OnConsoleMessage(p_, "You picked up " + to_string(give_) + " " + items[block_->id].ori_name + ", leaving " + to_string(block_->c_) + " in the machine.");
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You picked up " + to_string(give_) + " " + items[block_->id].ori_name + ", leaving " + to_string(block_->c_) + " in the machine.", 0, 1);
										Inventory::Modify(p_, block_->id, give_);
									}
									update_ = false;
								}
								else {
									VarList::OnConsoleMessage(p_, "You don't have room in your backpack!");
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You don't have room in your backpack!", 0, 1);
								}
							}
							break;
						}
					}
				}
				else {
					if (access) {
						if (block_->id != 0) break;
						int item_id = std::atoi(a_[i_ + 8].c_str());
						int c_ = 0, am_ = 0;
						Inventory::Modify(p_, item_id, c_);
						if (c_ == 0) break;
						if (items[item_id].untradeable or item_id == 1424 or item_id == 5816 or item_id == 242 or item_id == 1796 or item_id == 7188 or item_id == 8470 or item_id == 9160 or items[item_id].blockType == BlockTypes::FISH) {
							VarList::OnConsoleMessage(p_, (items[item_id].blockType == BlockTypes::LOCK or item_id == 8470 or item_id == 1424 or item_id == 5816 ? "No no no." : "Can't put that in a " + items[block_->fg].name + "."));
							VarList::OnTalkBubble(p_, pInfo(p_)->netID, (items[item_id].blockType == BlockTypes::LOCK or item_id == 8470 or item_id == 1424 or item_id == 5816 ? "No no no." : "Can't put that in a " + items[block_->fg].name + "."), 0, 1);
							break;
						}
						am_ = c_;
						c_ *= -1;
						if (Inventory::Modify(p_, item_id, c_) == 0) {
							block_->id = item_id;
							block_->c_ = am_;
							for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
								if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
								if (pInfo(cp_)->world == world_->name) {
									VarList::OnConsoleMessage(cp_, "`7[``" + pInfo(p_)->tankIDName + " put `2" + items[item_id].ori_name + "`` in the " + items[block_->fg].name + ".`7]``");
									VarList::OnTalkBubble(cp_, pInfo(p_)->netID, "`7[``" + pInfo(p_)->tankIDName + " put `2" + items[item_id].ori_name + "`` in the " + items[block_->fg].name + ".`7]``", 0, 0);
								}
								VarList::OnDialogRequest(p_, get_vending(p_, world_, block_, x_, y_));
							}
						}
					}
				}
				if (update_) {
					PlayerMoving data_{};
					data_.packetType = 17;
					data_.netID = 44;
					data_.YSpeed = 44;
					data_.x = x_ * 32 + 16;
					data_.y = y_ * 32 + 16;
					BYTE* raw = packPlayerMoving(&data_);
					for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
						if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
						if (pInfo(cp_)->world == world_->name) {
							CAction::Log(cp_, "action|play_sfx\nfile|audio/terraform.wav\ndelayMS|0");
							VarList::OnConsoleMessage(cp_, (block_->pr == 0 ? "`7[``" + pInfo(p_)->tankIDName + " disabled the Vending Machine.```7]``" : "`7[``" + pInfo(p_)->tankIDName + " changed the price of `2" + items[block_->id].ori_name + "`` to " + (block_->pr < 0 ? "`6" + to_string(block_->pr * -1) + " per World Lock.```7]``" : "`5" + to_string(block_->pr) + " World Locks each.```7]``")));
							VarList::OnTalkBubble(cp_, pInfo(p_)->netID, (block_->pr == 0 ? "`7[``" + pInfo(p_)->tankIDName + " disabled the Vending Machine.```7]``" : "`7[``" + pInfo(p_)->tankIDName + " changed the price of `2" + items[block_->id].ori_name + "`` to " + (block_->pr < 0 ? "`6" + to_string(block_->pr * -1) + " per World Lock.```7]``" : "`5" + to_string(block_->pr) + " World Locks each.```7]``")), 0, 0);
							send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
						}
					}
					delete[] raw;
				}
				tile_update(p_, world_, block_, x_, y_);
			}
			break;
		}
		else if (a_[i_] == "displayblock") {
			if (a_.size() != 11) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			int x_ = std::atoi(a_[i_ + 2].c_str()), y_ = std::atoi(a_[i_ + 5].c_str());
			if (x_ < 0 or y_ < 0) break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				if (y_ >= world_->max_y || x_ >= world_->max_x) break;
				WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
				uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
				if (items[t_].blockType != BlockTypes::DISPLAY) break;
				string owner_name = to_lower(world_->owner_name), user_name = to_lower(pInfo(p_)->tankIDName);
				if (owner_name != user_name and not owner_name.empty() and not Role::Administrator(p_)) {
					if (block_->locked) {
						WorldBlock* check_lock = &world_->blocks[block_->lock_origin];
						if (check_lock->owner_name != pInfo(p_)->tankIDName) break;
					}
					else {
						break;
					}
				}
				if (a_.size() == 11) {
					int c_ = 1;
					if (Inventory::Modify(p_, block_->id, c_) == 0) {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You removed `5" + items[block_->id].name + "`` from the " + items[block_->fg].name + ".", 0, 0);
						block_->id = 0;
						tile_update(p_, world_, block_, x_, y_);
					}
					else VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You don't have room to pick that up!", 0, 0);
				}
			}
			break;
		}		
		else if (a_[i_] == "mute_friend" or a_[i_] == "block_friend") {
			if (a_.size() != 8) break;
			if (a_[5] == "back") {
				DialogHandle::Info_Friend(p_, pInfo(p_)->last_edit);
			}
			else if (a_[i_] == "mute_friend" and a_[5] == "ok") {
				for (int c_ = 0; c_ < pInfo(p_)->friends.size(); c_++) {
					if (to_lower(pInfo(p_)->friends[c_].name) == to_lower(pInfo(p_)->last_edit)) {
						if (pInfo(p_)->friends[c_].mute) pInfo(p_)->friends[c_].mute = false;
						else pInfo(p_)->friends[c_].mute = true;
					}
				}
				DialogHandle::Info_Friend(p_, pInfo(p_)->last_edit);
			}
			else if (a_[i_] == "block_friend" and a_[5] == "ok") {
				for (int c_ = 0; c_ < pInfo(p_)->friends.size(); c_++) {
					if (to_lower(pInfo(p_)->friends[c_].name) == to_lower(pInfo(p_)->last_edit)) {
						if (pInfo(p_)->friends[c_].block_trade) pInfo(p_)->friends[c_].block_trade = false;
						else pInfo(p_)->friends[c_].block_trade = true;
					}
				}
				DialogHandle::Info_Friend(p_, pInfo(p_)->last_edit);
			}
			break;
		}
		else if (a_[i_] == "friends_options") {
			if (a_.size() != 12) break;
			if (a_[7] == "checkbox_public") {
				if (isValidCheckboxInput(a_[8])) {
					pInfo(p_)->show_location_ = std::atoi(a_[8].c_str());
				}
			}
			if (a_[9] == "checkbox_notifications") {
				if (isValidCheckboxInput(a_[10])) {
					pInfo(p_)->show_friend_notifications_ = std::atoi(a_[10].c_str());
				}
			}
			if (a_[5] == "back") {
				std::vector<std::string> friends_;
				for (int c_ = 0; c_ < pInfo(p_)->friends.size(); c_++) {
					friends_.push_back(pInfo(p_)->friends[c_].name);
				}
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (find(friends_.begin(), friends_.end(), to_lower(pInfo(cp_)->tankIDName)) != friends_.end()) {
						DialogHandle::Friends(p_, true);
						return;
					}
				}
				DialogHandle::Friends(p_);
			}
			break;
		}
		else if (a_[i_] == "mute_all_friends" or a_[i_] == "remove_all_friends" or a_[i_] == "block_all_friends") {
			if (a_.size() != 8) break;
			if (a_[5] == "back") {
				DialogHandle::Edit_All_Friends(p_);
				break;
			}
			if (a_[5] == "close") break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				for (int c_ = 0; c_ < pInfo(p_)->friends.size(); c_++) {
					if (find(pInfo(p_)->last_friends_selection.begin(), pInfo(p_)->last_friends_selection.end(), c_) != pInfo(p_)->last_friends_selection.end()) {
						if (a_[i_] == "mute_all_friends") pInfo(p_)->friends[c_].mute = (pInfo(p_)->friends[c_].mute ? false : true);
						else if (a_[i_] == "block_all_friends") pInfo(p_)->friends[c_].block_trade = (pInfo(p_)->friends[c_].block_trade ? false : true);
						else if (a_[i_] == "remove_all_friends") remove_friend(p_, world_, pInfo(p_)->friends[c_].name, false);
					}
				}
				DialogHandle::Edit_All_Friends(p_);
			}
			break;
		}
		else if (a_[i_] == "all_friends") {
			if (a_.size() < 10) break;
			if (a_[5] == "back") {
				std::vector<std::string> friends_;
				for (int c_ = 0; c_ < pInfo(p_)->friends.size(); c_++) {
					friends_.push_back(pInfo(p_)->friends[c_].name);
				}
				for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
					if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
					if (find(friends_.begin(), friends_.end(), to_lower(pInfo(cp_)->tankIDName)) != friends_.end()) {
						DialogHandle::Friends(p_, true);
						return;
					}
				}
				DialogHandle::Friends(p_);
				break;
			}
			pInfo(p_)->last_friends_selection.clear();
			string action_ = a_[5];
			try {
				for (int b_ = 0; b_ < a_.size(); b_++) {
					if (a_.at(b_).substr(0, 3) == "cf_") {
						if (!isValidCheckboxInput(a_.at(b_ + 1))) return;
						if (a_.at(b_ + 1) == "1") pInfo(p_)->last_friends_selection.push_back(std::atoi(explode("_", a_.at(b_)).at(1).c_str()));
					}
				}
			}
			catch (out_of_range) {
				break;
			}
			if (pInfo(p_)->last_friends_selection.size() == 0) {
				VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wConfirm``|left|1366|\nadd_textbox|`oYou have not selected any friends|left|\nadd_spacer|small|\nadd_button|back|`wBack``|noflags|0|0|\nadd_button|close|`wClose``|noflags|0|0|\nend_dialog|remove_all_friends|||\nadd_quick_exit|");
			}
			else {
				if (action_ == "mute_multi_friends") VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wConfirm``|left|1366|\nadd_textbox|`oAre you sure you want to `4mute`` " + to_string(pInfo(p_)->last_friends_selection.size()) + " friends?|left|\nadd_spacer|small|\nadd_button|confirm|`wYes``|noflags|0|0|\nadd_button|back|`wBack``|noflags|0|0|\nadd_button|close|`wClose``|noflags|0|0|\nend_dialog|mute_all_friends|||\nadd_quick_exit|");
				else if (action_ == "block_multi_friends") VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wConfirm``|left|1366|\nadd_textbox|`oAre you sure you want to `4block trade`` " + to_string(pInfo(p_)->last_friends_selection.size()) + " friends?|left|\nadd_spacer|small|\nadd_button|confirm|`wYes``|noflags|0|0|\nadd_button|back|`wBack``|noflags|0|0|\nadd_button|close|`wClose``|noflags|0|0|\nend_dialog|block_all_friends|||\nadd_quick_exit|");
				else VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wConfirm``|left|1366|\nadd_textbox|`oAre you sure you want to `4remove`` " + to_string(pInfo(p_)->last_friends_selection.size()) + " friends?|left|\nadd_spacer|small|\nadd_button|confirm|`wYes``|noflags|0|0|\nadd_button|back|`wBack``|noflags|0|0|\nadd_button|close|`wClose``|noflags|0|0|\nend_dialog|remove_all_friends|||\nadd_quick_exit|");
			}
			break;
		}
		else if (a_[i_] == "guild_members_options") {
			if (a_.size() != 12) break;
			if (pInfo(p_)->guild_id == 0) break;
			string public_ = a_[8];
			if (public_ != "0" and public_ != "1") break;
			string notifications_ = a_[10];
			if (notifications_ != "0" and notifications_ != "1") break;
			uint32_t guild_id = pInfo(p_)->guild_id;
			vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
			if (p != guilds.end()) {
				Guild* guild_information = &guilds[p - guilds.begin()];
				guild_information->guild_settings &= ~SETTINGS::SETTINGS_0;
				guild_information->guild_settings |= SETTINGS::SETTINGS_0;
				for (int i_ = 0; i_ < guild_information->guild_members.size(); i_++) {
					GuildMember* member_search = &guild_information->guild_members[i_];
					if (to_lower(member_search->member_name) == to_lower(pInfo(p_)->tankIDName)) {
						if (not member_search->public_location and public_ == "1") {
							member_search->public_location = true;
						}
						else if (member_search->public_location and public_ == "0") {
							member_search->public_location = false;
						}
						{
							if (not member_search->show_notifications and notifications_ == "1") {
								member_search->show_notifications = true;
							}
							else if (member_search->show_notifications and notifications_ == "0") {
								member_search->show_notifications = false;
							}
						}
						break;
					}
				}
			}
			if (a_[5] == "back") {
				string guild_info = get_guild_info(p_);
				if (guild_info == "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wGrow Guild ``|left|5814|\nadd_textbox|Something went wrong!|left|\nend_dialog|guildalreadyjoined|Close||") set_Guilds(p_);
				VarList::OnDialogRequest(p_, guild_info);
			}
			break;
		}
		else if (a_[i_] == "lock_edit" or a_[i_] == "editguildlock") {
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				int x_ = 0, y_ = 0;
				try {
					x_ = std::atoi(a_.at(i_ + 2).c_str()), y_ = std::atoi(a_.at(i_ + 5).c_str());
				}
				catch (out_of_range) {
					return;
				}
				if (x_ < 0 or x_ >= world_->max_x or y_ < 0 or y_ >= world_->max_y) break;
				if (p != worlds.end()) {
					WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
					uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
					if (items[t_].blockType != BlockTypes::LOCK) break;
					string owner_name = to_lower(world_->owner_name), user_name = to_lower(pInfo(p_)->tankIDName);
					if (a_.size() == 11) {
						if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) {
							if (find(block_->admins.begin(), block_->admins.end(), user_name) != block_->admins.end()) {
								block_->admins.erase(remove(block_->admins.begin(), block_->admins.end(), user_name), block_->admins.end());
								CAction::Log(p_, "action|play_sfx\nfile|audio/dialog_cancel.wav\ndelayMS|0");
								PlayerMoving data_{};
								data_.packetType = 5, data_.punchX = x_, data_.punchY = y_, data_.characterState = 0x8;
								int alloc = alloc_(world_, block_);
								BYTE* raw = packPlayerMoving(&data_, 112 + alloc);
								BYTE* blc = raw + 56;
								form_visual(blc, *block_, *world_, p_, false);
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(cp_)->world == world_->name) {
										VarList::OnConsoleMessage(cp_, user_name + " removed their access from a " + items[t_].name);
										send_raw(cp_, 4, raw, 112 + alloc, ENET_PACKET_FLAG_RELIABLE);
									}
									if (pInfo(cp_)->world == world_->name and to_lower(pInfo(cp_)->tankIDName) == to_lower(user_name)) {
										VarList::OnTalkBubble(cp_, pInfo(p_)->netID, "I've removed my access!", 0, 0);
									}
								}
								// FIX MEMLEAK: dulu `delete[] raw, blc;` (comma op) hanya delete raw,
								// tapi tidak ada leak karena blc memang offset (raw + 56). Lebih jelas:
								delete[] raw; // blc = raw + 56 ? JANGAN di-delete.
								if (block_->locked) {
									upd_lock(*block_, *world_, p_);
								}
							}
						}
						else if (find(world_->admins.begin(), world_->admins.end(), user_name) != world_->admins.end()) {
							world_->admins.erase(remove(world_->admins.begin(), world_->admins.end(), user_name), world_->admins.end());
							CAction::Log(p_, "action|play_sfx\nfile|audio/dialog_cancel.wav\ndelayMS|0");
							pInfo(p_)->name_color = (Role::Developer(p_) ? "`6@" : Role::Administrator(p_) ? "`e@" : Role::Moderator(p_) ? "`#@" : Role::Vip(p_) ? "`w[`1VIP`w] " : (to_lower(pInfo(p_)->tankIDName) == to_lower(world_->owner_name)) ? "`2" : "`0");
							VisualHandle::Nick(p_, NULL);
							PlayerMoving data_{};
							data_.packetType = 5, data_.punchX = x_, data_.punchY = y_, data_.characterState = 0x8;
							int alloc = alloc_(world_, block_);
							BYTE* raw = packPlayerMoving(&data_, 112 + alloc);
							BYTE* blc = raw + 56;
							form_visual(blc, *block_, *world_, p_, false);
							for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
								if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
								if (pInfo(cp_)->world == world_->name) {
									VarList::OnConsoleMessage(cp_, user_name + " removed their access from a " + items[t_].name);
									send_raw(cp_, 4, raw, 112 + alloc, ENET_PACKET_FLAG_RELIABLE);
								}
								if (pInfo(cp_)->world == world_->name and to_lower(pInfo(cp_)->tankIDName) == to_lower(user_name)) {
									VarList::OnTalkBubble(cp_, pInfo(p_)->netID, "I've removed my access!", 0, 0);
								}
							}
							delete[] raw; // FIX MEMLEAK: blc = raw + 56, JANGAN di-delete.
							if (block_->locked) {
								upd_lock(*block_, *world_, p_);
							}
						}
						break;
					}
					if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) {
						if (to_lower(block_->owner_name) != user_name and not Role::Administrator(p_)) break;
					}
					else {
						if (owner_name != user_name and not Role::Administrator(p_)) break;
					}
					bool reapply = false;
					for (int b_ = 0; b_ < a_.size(); b_++) {
						bool upd_ = false;
						if (a_[b_] == "getKey") {
							if (to_lower(pInfo(p_)->tankIDName) != to_lower(world_->owner_name)) break;
							int id = 4516;
							std::vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [id](const WorldBlock& a) { return a.fg == id; });
							if (p != world_->blocks.end()) {
								VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`4Remove Untrade-a-Box before Get Key!``", 0, 0);
								return;
							}
							if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) break;
							int key_ = 1424;
							if (block_->fg == 5814) {
								key_ = 5816;
								VarList::OnDialogRequest(p_, SetColor(p_) + "\nadd_label_with_icon|big|`wGet " + items[key_].name + " ``|left|" + to_string(key_) + "|\nadd_spacer|small|\nadd_textbox|The key to this lock can be used to transfer leadership of the guild to another guild Member.|left|\nadd_textbox|Use it like a `5World Lock Key``, but you can only give it to another Member of your guild!|left|\nadd_spacer|small|\nadd_textbox|`4Warning!`` By giving the `5Guild Key`` to another Member, you are effectively `rpromoting`` them to Guild Leader and `4demoting`` yourself at the same time!|left|\nadd_textbox|Are you sure you want to take the `5" + items[key_].name + "`` from the " + items[block_->fg].name + "?|left|\nend_dialog|confirmguildlockkey|Cancel|OK|\nadd_quick_exit|");
								return;
							}
							int c_ = 0;
							if (to_lower(pInfo(p_)->tankIDName) == to_lower(world_->owner_name)) Inventory::Modify(p_, 1424, c_);
							if (c_ != 0) break;
							if (world_->admins.size() != 0) {
								VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You'll first need to remove all players with access to your " + items[block_->fg].name + " to get a " + items[key_].name + ".", 0, 0);
								return;
							}
							c_ = 1;
							if (Inventory::Modify(p_, key_, c_) == 0) {
								CAction::Log(p_, "action|play_sfx\nfile|audio/use_lock.wav\ndelayMS|0");
								VarList::OnTalkBubble(p_, pInfo(p_)->netID, "You got a `#" + items[key_].name + "``! You can now trade this world to other players.", 0, 0);
							}
						}
						else if (a_[b_] == "upg_gems_world") {
							if (to_lower(pInfo(p_)->tankIDName) != to_lower(world_->owner_name)) break;
							string cost = "", x_g = "";
							if (world_->gems_lvl == 0) cost = "5.000.000", x_g = "3";
							if (world_->gems_lvl == 2) cost = "7.000.000", x_g = "5";
							if (world_->gems_lvl == 3) cost = "10.000.000", x_g = "7";
							if (world_->gems_lvl == 4) cost = "15.000.000", x_g = "10";
							if (world_->gems_lvl == 5) cost = "17.000.000", x_g = "12";
							if (world_->gems_lvl == 6) cost = "20.000.000", x_g = "14";
							if (world_->gems_lvl == 7) cost = "23.000.000", x_g = "15";
							if (world_->gems_lvl == 8) cost = "25.000.000", x_g = "16";
							if (world_->gems_lvl == 9) cost = "26.000.000", x_g = "17";
							if (world_->gems_lvl == 10) cost = "28.000.000", x_g = "18";
							if (world_->gems_lvl == 11) cost = "30.000.000", x_g = "20";
							VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`$Upgrade Gems Bonus|left|112|\nadd_spacer|small|\nadd_textbox|`$Upgrading in this world allows you to get up to `5x" + x_g + " `$more Gems, but remember, this bonus only applies in this world. This upgrade process is a simple way to increase the amount of your Gems, helping you progress faster in the game without excessive complications.|left|\nadd_spacer|small|\nadd_smalltext|`$This upgrade cost `2" + cost + " `$Gems to activate (`2x" + x_g + "`$).|left|\nend_dialog|upgrade_world_gems|Cancel|`2Upgrade|");
						}
						else if (a_[b_] == "sessionlength_dialog") {
							if (pInfo(p_)->tankIDName != world_->owner_name) break;
							if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994) break;
							VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSet World Timer``|left|1482|\nembed_data|tilex|" + to_string(x_) + "\nembed_data|tiley|" + to_string(y_) + "\nadd_spacer|small|\nadd_textbox|Select a time limit for how long guests can stay in your world.|left|\nmax_checks|1|\nadd_checkbox|checkbox_0|None|" + (world_->World_Time == 0 ? "1" : "0") + "\nadd_checkbox|checkbox_5|5 minutes|" + (world_->World_Time == 5 ? "1" : "0") + "\nadd_checkbox|checkbox_10|10 minutes|" + (world_->World_Time == 10 ? "1" : "0") + "\nadd_checkbox|checkbox_20|20 minutes|" + (world_->World_Time == 20 ? "1" : "0") + "\nadd_checkbox|checkbox_30|30 minutes|" + (world_->World_Time == 30 ? "1" : "0") + "\nadd_checkbox|checkbox_40|40 minutes|" + (world_->World_Time == 40 ? "1" : "0") + "\nadd_checkbox|checkbox_50|50 minutes|" + (world_->World_Time == 50 ? "1" : "0") + "\nadd_checkbox|checkbox_60|60 minutes|" + (world_->World_Time == 60 ? "1" : "0") + "\nend_dialog|sessionlength_edit|Nevermind|Set Time Limit|");
						}
						else if (a_[b_] == "changecat") {
							if (to_lower(pInfo(p_)->tankIDName) != to_lower(world_->owner_name)) break;
							string types = "";
							for (int i = 1; i < Environment()->world_rate_types.size(); i++) types += "\nadd_button|" + to_string(i) + "|" + (world_category(i)) + "|noflags|0|0|";
							VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|-1\nadd_textbox|Select a category for your world.|left|\nadd_button|cat0|None|noflags|0|0|\nadd_button|cat1|Adventure|noflags|0|0|\nadd_button|cat2|Art|noflags|0|0|\nadd_button|cat3|Farm|noflags|0|0|\nadd_button|cat4|Game|noflags|0|0|\nadd_button|cat5|Information|noflags|0|0|\nadd_button|cat_14|Music|noflags|0|0|\nadd_button|cat6|Parkour|noflags|0|0|\nadd_button|cat_13|Puzzle|noflags|0|0|\nadd_button|cat7|Roleplay|noflags|0|0|\nadd_button|cat8|Shop|noflags|0|0|\nadd_button|cat9|Social|noflags|0|0|\nadd_button|cat_10|Storage|noflags|0|0|\nadd_button|cat_11|Story|noflags|0|0|\nadd_button|cat_12|Trade|noflags|0|0|\nadd_button|cat_15|Guild|noflags|0|0|\nadd_button|cat_16|Casino|noflags|0|0|\nadd_smalltext|Worlds in the category \"None\" can't be rated by players, and they are only listed on the normal World Select screen.|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world.|left|\nend_dialog|worldcategory|Nevermind||");
						}
						else if (a_[b_] == "minimum_entry_level" and isdigit(a_[b_ + 1][0])) {
							if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) break;
							int new_entry_level_ = std::atoi(a_[b_ + 1].c_str());
							if (world_->entry_level != new_entry_level_) {
								if (new_entry_level_ < 1) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Minimum entry level can't be lower than 1.", 0, 0);
									break;
								}
								if (new_entry_level_ > 250) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Minimum entry level can't be higher than 250.", 0, 0);
									break;
								}
								world_->entry_level = new_entry_level_;
								VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Minimum entry level for this world is set to `2Level " + to_string(new_entry_level_) + "``.", 0, 0);
							}
						}
						else if (a_[b_] == "tempo" and isdigit(a_[b_ + 1][0])) {
							if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) break;
							uint32_t new_bpm_ = std::atoi(a_[b_ + 1].c_str());
							if (world_->music_bpm != new_bpm_) {
								if (new_bpm_ < 20 or new_bpm_ > 200) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Tempo must be from 20-200 BPM.", 0, 0);
									break;
								}
								world_->music_bpm = new_bpm_, upd_ = true;
							}
						}
						else if (a_[b_] == "recalcLock") {
							if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) {
								reapply = true;
							}
						}
						else if (a_[b_] == "abondonguildconfirm") {
							if (block_->fg == 5814) {
								uint32_t guild_id = world_->guild_id;
								vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
								if (p != guilds.end()) {
									Guild* guild_information = &guilds[p - guilds.begin()];
									if (guild_information->guild_members.size() != 1) {
										VarList::OnDialogRequest(p_, SetColor(p_) + "\nadd_label_with_icon|big|`wAbandon Guild ``|left|5814|\nadd_textbox|`1You need to kick all guild members before you can abandon the guild! `` |left|\nadd_button|back|Back|noflags|0|0|\nend_dialog|abondonguild|||");
										break;
									}
									VarList::OnDialogRequest(p_, SetColor(p_) + "\nadd_label_with_icon|big|`wAbandon Guild ``|left|5814|\nadd_spacer|small|\nadd_item_picker|selectworldlock|`wSelect a World Lock``|Choose a World Lock to replace the Guild Lock|\nadd_button|back|Back|noflags|0|0|\nend_dialog|abondonguild|||");
								}
								break;
							}
						}
						else if (a_[b_] == "create_guild_mascot") {
							if (block_->fg == 5814) guild_flag_dialog(p_);
							break;
						}
						else if (a_[b_] == "upgrade_guild") {
							if (block_->fg == 5814) {
								uint32_t guild_id = pInfo(p_)->guild_id;
								if (guild_id == 0) return;
								vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
								if (p != guilds.end()) {
									Guild* guild_information = &guilds[p - guilds.begin()];
									if (guild_information->guild_level <= (int)guild_lvl.size()) {
										for (GuildMember member_search : guild_information->guild_members) {
											if (to_lower(member_search.member_name) == to_lower(pInfo(p_)->tankIDName)) {
												if (member_search.role_id == 3) {
													VarList::OnDialogRequest(p_, SetColor(p_) + "\nadd_label_with_icon|big|`wGuild Upgrade``|left|5814|\nadd_textbox|Are you sure you want to upgrade the guild for `2" + to_string(guild_lvl[guild_information->guild_level - 1][1]) + " Gems``?|left|\nadd_spacer|small|\nadd_button|upgrade_guild|`wUpgrade Guild``|noflags|0|0|\nadd_button|back|Back|noflags|0|0|\nend_dialog|upgrading_guild|||");
												}
											}
										}
									}
								}
							}
							break;
						}
						else if (a_[b_].substr(0, 9) == "checkbox_") {
							std::vector<std::string> target_ = explode("_", a_[b_]);
							if (!isValidCheckboxInput(a_[b_ + 1])) break;
							if (target_[1] == "disable") {
								if (target_.size() == 4 and target_[2] == "music" and target_[3] == "render") {
									if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) break;
									if (a_[b_ + 1] == "1") world_->world_settings |= SETTINGS::SETTINGS_6;
									else world_->world_settings &= ~SETTINGS::SETTINGS_6;
									upd_ = true;
								}
								else if (target_[2] == "music") {
									if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) break;
									if (a_[b_ + 1] == "1") world_->world_settings |= SETTINGS::SETTINGS_5;
									else world_->world_settings &= ~SETTINGS::SETTINGS_5;
									upd_ = true;
								}
							}
							else if (target_.size() == 5 and target_[4] == "world") {
								if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) break;
								if (pInfo(p_)->home_world != world_->name and a_[b_ + 1] == "1") {
									pInfo(p_)->home_world = world_->name;
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, world_->name + " has been set as your home world!", 0, 0);
								}
								else if (pInfo(p_)->home_world == world_->name and a_[b_ + 1] == "0") {
									pInfo(p_)->home_world = "";
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, world_->name + " has been removed as your home world!", 0, 0);
								}
							}
							else if (target_[1] == "member") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 5814) {
									uint32_t guild_id = world_->guild_id;
									vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
									if (p != guilds.end()) {
										Guild* guild_information = &guilds[p - guilds.begin()];
										if (a_[b_ + 1] == "1") guild_information->guild_settings |= SETTINGS::SETTINGS_1;
										else guild_information->guild_settings &= ~SETTINGS::SETTINGS_1;
										upd_ = true;
									}
								}
							}
							else if (target_[1] == "elder") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 5814) {
									uint32_t guild_id = world_->guild_id;
									vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
									if (p != guilds.end()) {
										Guild* guild_information = &guilds[p - guilds.begin()];
										if (a_[b_ + 1] == "1") guild_information->guild_settings |= SETTINGS::SETTINGS_2;
										else guild_information->guild_settings &= ~SETTINGS::SETTINGS_2;
										upd_ = true;
									}
								}
							}
							else if (target_[1] == "coleader") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 5814) {
									uint32_t guild_id = world_->guild_id;
									vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
									if (p != guilds.end()) {
										Guild* guild_information = &guilds[p - guilds.begin()];
										if (a_[b_ + 1] == "1") guild_information->guild_settings |= SETTINGS::SETTINGS_3;
										else guild_information->guild_settings &= ~SETTINGS::SETTINGS_3;
										upd_ = true;
									}
								}
							}
							else if (target_[1] == "displaymascot") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 5814) {
									uint32_t guild_id = world_->guild_id;
									vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
									if (p != guilds.end()) {
										Guild* guild_information = &guilds[p - guilds.begin()];
										if (a_[b_ + 1] == "1") guild_information->guild_settings |= SETTINGS::SETTINGS_4;
										else guild_information->guild_settings &= ~SETTINGS::SETTINGS_4;
										upd_ = true;
									}
								}
							}
							else if (target_[1] == "ignore") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) {
									block_->invert = std::atoi(a_[b_ + 1].c_str());
								}
							}
							else if (target_[1] == "buildonly") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 4994) block_->build_only = std::atoi(a_[b_ + 1].c_str()); upd_ = true;
							}
							else if (target_[1] == "admins") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 4994) block_->limit_admins = std::atoi(a_[b_ + 1].c_str());
							}
							else if (target_[1] == "silence") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 4802) {
									if (std::atoi(a_[b_ + 1].c_str())) world_->world_settings |= SETTINGS::SETTINGS_8;
									else world_->world_settings &= ~SETTINGS::SETTINGS_8;
								}
							}
							else if (target_[1] == "rainbows") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 4802) {
									if (std::atoi(a_[b_ + 1].c_str())) world_->world_settings |= SETTINGS::SETTINGS_7;
									else world_->world_settings &= ~SETTINGS::SETTINGS_7;
									upd_ = true;
								}
							}
							else if (target_[1] == "public") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) {
									block_->spin = std::atoi(a_[b_ + 1].c_str());
									upd_ = true;
								}
								else {
									bool upda = false;
									if (world_->world_settings & SETTINGS::SETTINGS_4) {
										if (a_[b_ + 1] == "0") {
											world_->world_settings &= ~SETTINGS::SETTINGS_4;
											upda = true;
										}
									}
									else {
										if (a_[b_ + 1] == "1") {
											world_->world_settings |= SETTINGS::SETTINGS_4;
											upda = true;
										}
									}
									if (upda) {
										if (not Role::Moderator(p_) and not Role::Administrator(p_)) pInfo(p_)->name_color = "`2";
										for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
											if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
											if (pInfo(cp_)->world == world_->name) {
												VarList::OnConsoleMessage(cp_, get_player_nick(p_) + " has set the `$World Lock`` to " + (world_->world_settings & SETTINGS::SETTINGS_4 ? "`$PUBLIC" : "`4PRIVATE") + "");
											}
										}
									}

								}
							}
							else if (target_[1] == "drop") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (std::atoi(a_[b_ + 1].c_str())) world_->world_settings |= SETTINGS::SETTINGS_2;
								else world_->world_settings &= ~SETTINGS::SETTINGS_2;
								upd_ = true;
							}
							else if (target_[1] == "magplant") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (std::atoi(a_[b_ + 1].c_str())) world_->world_settings |= SETTINGS::SETTINGS_10;
								else world_->world_settings &= ~SETTINGS::SETTINGS_10;
								upd_ = true;
							}
							else if (target_[1] == "cheater") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (std::atoi(a_[b_ + 1].c_str())) world_->world_settings |= SETTINGS::SETTINGS_12;
								else world_->world_settings &= ~SETTINGS::SETTINGS_12;
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL or pInfo(cp_)->world != world_->name) continue;
									if (to_lower(pInfo(cp_)->tankIDName) != to_lower(world_->owner_name)) {
										pInfo(cp_)->disable_cheater = world_->world_settings & SETTINGS::SETTINGS_12;
									}
								}
								upd_ = true;
							}
							else if (target_[1] == "vending") {
								if (!isValidCheckboxInput(a_[b_ + 1])) break;
								if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) {
								}
								else {
									bool upda = false;
									if (world_->world_settings & SETTINGS::SETTINGS_3) {
										if (a_[b_ + 1] == "0") {
											world_->world_settings &= ~SETTINGS::SETTINGS_3;
											upda = true;
										}
									}
									else {
										if (a_[b_ + 1] == "1") {
											world_->world_settings |= SETTINGS::SETTINGS_3;
											upda = true;
										}
									}
									if (upda) {
										if (not Role::Moderator(p_) and not Role::Administrator(p_))pInfo(p_)->name_color = "`2";
										for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
											if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
											if (pInfo(cp_)->world == world_->name) {
												VarList::OnConsoleMessage(cp_, get_player_nick(p_) + " has set the `$Vending`` to " + (world_->world_settings & SETTINGS::SETTINGS_3 ? "`$PUBLIC" : "`4PRIVATE") + "");
											}
										}
									}
								}
							}
							else if (target_.size() == 2 and a_[b_ + 1] == "0") {
								string user_name = target_[1].erase(0, 1);
								if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) {
									if (find(block_->admins.begin(), block_->admins.end(), user_name) != block_->admins.end()) {
										block_->admins.erase(remove(block_->admins.begin(), block_->admins.end(), user_name), block_->admins.end());
										tile_update(p_, world_, block_, x_, y_);
										for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
											if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
											if (pInfo(cp_)->world == world_->name) {
												VarList::OnConsoleMessage(cp_, user_name + " was removed from a " + items[block_->fg].name + ".");
											}
											if (to_lower(pInfo(cp_)->tankIDName) == to_lower(user_name)) {
												CAction::Log(cp_, "action|play_sfx\nfile|audio/dialog_cancel.wav\ndelayMS|0");
												if (not Role::Moderator(p_) and not Role::Administrator(p_))pInfo(p_)->name_color = "`2";
												VarList::OnTalkBubble(cp_, pInfo(p_)->netID, get_player_nick(p_) + " has `4removed`` your access from a lock on world `w" + world_->name + "``.", 0, 0);
											}
										}
									}
								}
								else if (find(world_->admins.begin(), world_->admins.end(), user_name) != world_->admins.end()) {
									world_->admins.erase(remove(world_->admins.begin(), world_->admins.end(), user_name), world_->admins.end());
									tile_update(p_, world_, block_, x_, y_);
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
										if (pInfo(cp_)->world == world_->name) {
											VarList::OnConsoleMessage(cp_, user_name + " was removed from a " + items[block_->fg].name + ".");
										}
										if (to_lower(pInfo(cp_)->tankIDName) == to_lower(user_name)) {
											CAction::Log(cp_, "action|play_sfx\nfile|audio/dialog_cancel.wav\ndelayMS|0");
											if (not Role::Moderator(p_) and not Role::Administrator(p_))pInfo(p_)->name_color = "`2";
											VarList::OnTalkBubble(cp_, pInfo(p_)->netID, get_player_nick(p_) + " has `4removed`` your access from a lock on world `w" + world_->name + "``.", 0, 0);
											pInfo(cp_)->name_color = (Role::Developer(cp_) ? "`6@" : Role::Administrator(cp_) ? "`e@" : Role::Moderator(cp_) ? "`#@" : Role::Vip(cp_) ? "`w[`1VIP`w] " : (to_lower(pInfo(cp_)->tankIDName) == to_lower(world_->owner_name)) ? "`2" : "`0");
											VisualHandle::Nick(cp_, NULL);
										}
									}
								}
							}
						}
						else if (a_[b_] == "playerNetID" and isdigit(a_[b_ + 1][0])) {
							if (block_->fg == 202 or block_->fg == 204 or block_->fg == 206 or block_->fg == 4994 or block_->fg == 10000) {
								if (block_->admins.size() >= 26) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Lock has reached access limit!", 0, 0);
									break;
								}
								size_t id_ = std::atoi(a_[b_ + 1].c_str());
								if (id_ == pInfo(p_)->netID) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "I already have access!", 0, 0);
									break;
								}
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(cp_)->netID == id_ and pInfo(cp_)->world == world_->name) {
										if (pInfo(cp_)->access_offers.find(pInfo(p_)->netID) != pInfo(cp_)->access_offers.end()) break;
										string user_name = to_lower(pInfo(cp_)->tankIDName);
										if (user_name == to_lower(block_->owner_name) or find(block_->admins.begin(), block_->admins.end(), user_name) != block_->admins.end()) {
											VarList::OnTalkBubble(p_, pInfo(p_)->netID, get_player_nick(cp_) + " already has access to the lock.", 0, 0);
											break;
										}
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Offered " + get_player_nick(cp_) + " access to lock.", 0, 0);
										pInfo(cp_)->access_offers.insert({ pInfo(p_)->netID, (x_ + (y_ * 100)) });
										CAction::Log(cp_, "action|play_sfx\nfile|audio/secret.wav\ndelayMS|0");
										VarList::OnConsoleMessage(cp_, get_player_nick(p_) + " wants to add you to a " + items[block_->fg].name + ". Wrench yourself to accept.");
										break;
									}
								}
							}
							else {
								if (world_->admins.size() >= 26) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "World has reached access limit!", 0, 0);
									break;
								}
								size_t id_ = std::atoi(a_[b_ + 1].c_str());
								if (id_ == pInfo(p_)->netID) {
									VarList::OnTalkBubble(p_, pInfo(p_)->netID, "I already have access!", 0, 0);
									break;
								}
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(cp_)->netID == id_ and pInfo(cp_)->world == world_->name) {
										if (pInfo(cp_)->access_offers.find(pInfo(p_)->netID) != pInfo(cp_)->access_offers.end()) break;
										string user_name = to_lower(pInfo(cp_)->tankIDName);
										if (user_name == to_lower(world_->owner_name) or find(world_->admins.begin(), world_->admins.end(), user_name) != world_->admins.end()) {
											VarList::OnTalkBubble(p_, pInfo(p_)->netID, get_player_nick(cp_) + " already has access to the lock.", 0, 0);
											break;
										}
										VarList::OnTalkBubble(p_, pInfo(p_)->netID, "Offered " + get_player_nick(cp_) + " access to lock.", 0, 0);
										pInfo(cp_)->access_offers.insert({ pInfo(p_)->netID, (x_ + (y_ * 100)) });
										CAction::Log(cp_, "action|play_sfx\nfile|audio/secret.wav\ndelayMS|0");
										VarList::OnConsoleMessage(cp_, get_player_nick(p_) + " wants to add you to a " + items[block_->fg].name + ". Wrench yourself to accept.");
										break;
									}
								}
							}
							break;
						}
						if (upd_) {
							if (block_->fg == 5814) {
								uint32_t guild_id = world_->guild_id;
								vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
								if (p != guilds.end()) {
									Guild* guild_information = &guilds[p - guilds.begin()];
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
										if (pInfo(cp_)->world == world_->name) {
											if (pInfo(cp_)->guild_id == guild_information->guild_id) {
												if (guild_access(cp_, guild_id)) {
													VisualHandle::Nick(cp_, NULL);
												}
												else if (find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(cp_)->tankIDName)) == world_->admins.end()) {
													if (to_lower(pInfo(cp_)->tankIDName) != to_lower(world_->owner_name)) {
														VisualHandle::Nick(cp_, NULL);
													}
												}
											}
										}
									}
								}
							}
							tile_update(p_, world_, block_, x_, y_);
						}
					}
					if (reapply) {
						{
							vector<int> new_tiles{};
							vector<WorldBlock> shadow_copy_2 = world_->blocks;
							new_tiles.push_back(x_ + (y_ * 100));
							int ySize = world_->blocks.size() / 100, xSize = world_->blocks.size() / ySize;
							for (int i2 = 0; i2 < new_tiles.size(); i2++) {
								int s_x_ = new_tiles[i2] % 100, s_y_ = new_tiles[i2] / 100;
								if (s_x_ < (world_->max_x - 1) and shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].locked and shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].lock_origin == (x_ + (y_ * 100))) {
									if (not shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].scanned) {
										shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].scanned = true;
										new_tiles.push_back(s_x_ + 1 + (s_y_ * 100));
										world_->blocks[s_x_ + 1 + (s_y_ * 100)].locked = false;
										world_->blocks[s_x_ + 1 + (s_y_ * 100)].lock_origin = -1;
									}
								}
								if (s_x_ > 0 and shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].locked and shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].lock_origin == (x_ + (y_ * 100))) {
									if (not shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].scanned) {
										shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].scanned = true;
										new_tiles.push_back(s_x_ - 1 + (s_y_ * 100));
										world_->blocks[s_x_ - 1 + (s_y_ * 100)].locked = false;
										world_->blocks[s_x_ - 1 + (s_y_ * 100)].lock_origin = -1;
									}
								}
								if (s_y_ < (world_->max_y - 1) and shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].locked and shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].lock_origin == (x_ + (y_ * 100))) {
									if (not shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].scanned) {
										shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].scanned = true;
										new_tiles.push_back(s_x_ + ((s_y_ + 1) * 100));
										world_->blocks[s_x_ + ((s_y_ + 1) * 100)].locked = false;
										world_->blocks[s_x_ + ((s_y_ + 1) * 100)].lock_origin = -1;
									}
								}
								if (s_y_ > 0 and shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].locked and shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].lock_origin == (x_ + (y_ * 100))) {
									if (not shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].scanned) {
										shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].scanned = true;
										new_tiles.push_back(s_x_ + ((s_y_ - 1) * 100));
										world_->blocks[s_x_ + ((s_y_ - 1) * 100)].locked = false;
										world_->blocks[s_x_ + ((s_y_ - 1) * 100)].lock_origin = -1;
									}
								}
							}
						}
						int lock_size = block_->fg == 202 ? 10 : (block_->fg == 204 || block_->fg == 10000 ? 48 : (block_->fg == 202 ? 200 : 200));
						if (not block_->invert) {
							{
								vector<vector<int>> tiles_ = lock_tiles(world_, x_, y_, lock_size);
								lock_size = tiles_.size();
								PlayerMoving data_{};
								data_.packetType = 15, data_.punchX = x_, data_.punchY = y_, data_.characterState = 0x8;
								data_.netID = name_to_number(to_lower(pInfo(p_)->tankIDName));
								data_.plantingTree = block_->fg;
								BYTE* raw;
								if (tiles_.size() != 0) {
									int alloc = alloc_(world_, block_);
									raw = packPlayerMoving(&data_, 112 + (lock_size * 2) + alloc);
									int lalala = 8;
									Memory_Copy(raw + 8, &lock_size, 2);
									Memory_Copy(raw + 12, &lalala, 2);
									BYTE* blc = raw + 56;
									for (int i_ = 0; i_ < tiles_.size(); i_++) {
										vector<int> update_tiles = tiles_[i_];
										int x = update_tiles[0];
										int y = update_tiles[1];
										int sq_ = x + (y * 100);
										WorldBlock* block_ = &world_->blocks[sq_];
										if (block_->locked) {
										}
										else {
											block_->locked = true;
											block_->lock_origin = x_ + (y_ * 100);
											Memory_Copy(blc + (i_ * 2), &sq_, 2);
										}
									}
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
										if (pInfo(cp_)->world == name_) {
											CAction::Log(cp_, "action|play_sfx\nfile|audio/use_lock.wav\ndelayMS|0");
											send_raw(cp_, 4, raw, 112 + (lock_size * 2) + alloc, ENET_PACKET_FLAG_RELIABLE);
										}
									}
									delete[] raw; // FIX MEMLEAK: blc = raw + 56, jangan delete.
								}
								else {
									raw = packPlayerMoving(&data_, 56);
									for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
										if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
										if (pInfo(cp_)->world == name_) {
											CAction::Log(cp_, "action|play_sfx\nfile|audio/use_lock.wav\ndelayMS|0");
											send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										}
									}
									delete[] raw;
								}
							}
						}
						else {
							vector<int> new_tiles{};
							vector<int> lock_tiles{};
							vector<WorldBlock> shadow_copy_2 = world_->blocks;
							new_tiles.push_back(x_ + (y_ * 100));
							int ySize = world_->blocks.size() / 100, xSize = world_->blocks.size() / ySize;
							for (int i2 = 0; i2 < new_tiles.size(); i2++) {
								if (lock_tiles.size() >= lock_size) break;
								int s_x_ = new_tiles[i2] % 100, s_y_ = new_tiles[i2] / 100;
								if (s_x_ < (world_->max_x - 1) and not shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].locked and shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].fg != 0 and not_mod(shadow_copy_2[s_x_ + 1 + (s_y_ * 100)], s_x_ + 1 + (s_y_ * 100), world_->blocks.size()) or s_x_ < (world_->max_x - 1) and not shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].locked and shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].bg != 0 and shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].fg == 0) {
									if (not shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].scanned) {
										shadow_copy_2[s_x_ + 1 + (s_y_ * 100)].scanned = true;
										new_tiles.push_back(s_x_ + 1 + (s_y_ * 100));
										lock_tiles.push_back(s_x_ + 1 + (s_y_ * 100));
										world_->blocks[s_x_ + 1 + (s_y_ * 100)].locked = true;
										world_->blocks[s_x_ + 1 + (s_y_ * 100)].lock_origin = x_ + (y_ * 100);
									}
								}
								if (s_x_ > 0 and not shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].locked and shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].fg != 0 and not_mod(shadow_copy_2[s_x_ - 1 + (s_y_ * 100)], s_x_ - 1 + (s_y_ * 100), world_->blocks.size()) or s_x_ > 0 and not shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].locked and shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].bg != 0 and shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].fg == 0) {
									if (not shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].scanned) {
										shadow_copy_2[s_x_ - 1 + (s_y_ * 100)].scanned = true;
										new_tiles.push_back(s_x_ - 1 + (s_y_ * 100));
										lock_tiles.push_back(s_x_ - 1 + (s_y_ * 100));
										world_->blocks[s_x_ - 1 + (s_y_ * 100)].locked = true;
										world_->blocks[s_x_ - 1 + (s_y_ * 100)].lock_origin = x_ + (y_ * 100);
									}
								}
								if (s_y_ < (world_->max_y - 1) and not shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].locked and shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].fg != 0 and not_mod(shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)], s_x_ + ((s_y_ + 1) * 100), world_->blocks.size()) or s_y_ < (world_->max_y - 1) and not shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].locked and shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].bg != 0 and shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].fg == 0) {
									if (not shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].scanned) {
										shadow_copy_2[s_x_ + ((s_y_ + 1) * 100)].scanned = true;
										new_tiles.push_back(s_x_ + ((s_y_ + 1) * 100));
										lock_tiles.push_back(s_x_ + ((s_y_ + 1) * 100));
										world_->blocks[s_x_ + ((s_y_ + 1) * 100)].locked = true;
										world_->blocks[s_x_ + ((s_y_ + 1) * 100)].lock_origin = x_ + (y_ * 100);
									}
								}
								if (s_y_ > 0 and not shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].locked and shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].fg != 0 and not_mod(shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)], s_x_ + ((s_y_ - 1) * 100), world_->blocks.size()) or s_y_ > 0 and not shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].locked and shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].bg != 0 and shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].fg == 0) {
									if (not shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].scanned) {
										shadow_copy_2[s_x_ + ((s_y_ - 1) * 100)].scanned = true;
										new_tiles.push_back(s_x_ + ((s_y_ - 1) * 100));
										lock_tiles.push_back(s_x_ + ((s_y_ - 1) * 100));
										world_->blocks[s_x_ + ((s_y_ - 1) * 100)].locked = true;
										world_->blocks[s_x_ + ((s_y_ - 1) * 100)].lock_origin = x_ + (y_ * 100);
									}
								}
							}
							lock_size = lock_tiles.size();
							if (lock_tiles.size() > 0) {
								PlayerMoving data_{};
								data_.packetType = 15, data_.punchX = x_, data_.punchY = y_, data_.characterState = 0x8;
								data_.netID = to_lower(block_->owner_name) == to_lower(pInfo(p_)->tankIDName) ? name_to_number(to_lower(pInfo(p_)->tankIDName)) : -1;
								data_.plantingTree = block_->fg;
								BYTE* raw;
								int alloc = alloc_(world_, block_);
								raw = packPlayerMoving(&data_, 112 + (lock_size * 2) + alloc);
								int lalala = 8;
								Memory_Copy(raw + 8, &lock_size, 2);
								Memory_Copy(raw + 12, &lalala, 2);
								BYTE* blc = raw + 56;
								for (int i_ = 0; i_ < lock_tiles.size(); i_++) Memory_Copy(blc + (i_ * 2), &lock_tiles[i_], 2);
								PlayerMoving data_2{};
								data_2.packetType = 5, data_2.punchX = x_, data_2.punchY = y_, data_2.characterState = 0x8;
								BYTE* raw2 = packPlayerMoving(&data_2, 112 + alloc_(world_, block_));
								BYTE* blc2 = raw2 + 56;
								form_visual(blc2, *block_, *world_, p_, false);
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (pInfo(cp_)->world == world_->name) {
										CAction::Log(cp_, "action|play_sfx\nfile|audio/friend_logoff.wav\ndelayMS|0");
										send_raw(cp_, 4, raw, 112 + (lock_size * 2) + alloc, ENET_PACKET_FLAG_RELIABLE);
										send_raw(cp_, 4, raw2, 112 + alloc, ENET_PACKET_FLAG_RELIABLE);
									}
								}
								delete[] raw;  // FIX MEMLEAK: dulu `delete[] raw, blc, raw2, blc2;`
								delete[] raw2; // hanya raw yg di-delete (comma op), raw2 leak.
								// blc = raw + 56, blc2 = raw2 + 56 ? keduanya offset, JANGAN di-delete.
							}
							else {
								PlayerMoving data_{};
								data_.packetType = 15, data_.punchX = x_, data_.punchY = y_, data_.characterState = 0x8;
								data_.netID = to_lower(block_->owner_name) == to_lower(pInfo(p_)->tankIDName) ? name_to_number(to_lower(pInfo(p_)->tankIDName)) : -1;
								data_.plantingTree = block_->fg;
								BYTE* raw = packPlayerMoving(&data_, 56);
								for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
									if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
									if (world_->name == pInfo(cp_)->world) {
										CAction::Log(cp_, "action|play_sfx\nfile|audio/use_lock.wav\ndelayMS|0");
										send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										PlayerMoving data_2{};
										data_2.packetType = 5, data_2.punchX = x_, data_2.punchY = y_, data_2.characterState = 0x8;
										int alloc = alloc_(world_, block_);
										BYTE* raw2 = packPlayerMoving(&data_2, 112 + alloc);
										BYTE* blc2 = raw2 + 56;
										form_visual(blc2, *block_, *world_, cp_, false);
										send_raw(cp_, 4, raw2, 112 + alloc, ENET_PACKET_FLAG_RELIABLE);
										delete[] raw2; // FIX MEMLEAK: blc2 = raw2 + 56, jangan delete.
									}
								}
								delete[] raw;
							}
						}
					}
				}
			}
			break;
		}
		else if (a_[i_] == "create_guild_mascot") {
			if (a_.size() < 7) break;
			string button_clicked = a_[5], button_name = a_[4];
			if (button_name == "select_bg" || button_name == "select_fg") {
				int item = std::atoi(a_[5].c_str());
				if (item <= 0 && item > items.size()) break;
				string error = "";
				if (items[item].blockType == BlockTypes::SEED) {
					VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`wYou can't set the Guild Mascot to seed!``", 0, 0);
					error = "Seeds are not valid Guild Mascot elements! Please choose something else.";
				}
				if (items[item].blockType == BlockTypes::BACKGROUND) {
					pInfo(p_)->set_guild_bg = item;
					VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`wYou set Guild Background to " + items[item].ori_name + "``", 0, 0);
				}
				else {
					pInfo(p_)->set_guild_fg = item;
					VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`wYou set Guild Foreground to " + items[item].ori_name + "``", 0, 0);
				}
				guild_flag_dialog(p_, error);
			}
			else {
				if (button_clicked == "create") {
					bool taken_ = false;
					if (pInfo(p_)->set_guild_fg + pInfo(p_)->set_guild_bg != 0) {
						for (Guild check_guild_name : guilds) {
							if (check_guild_name.guild_mascot[0] == pInfo(p_)->set_guild_fg && check_guild_name.guild_mascot[1] == pInfo(p_)->set_guild_bg) {
								taken_ = true;
								break;
							}
						}
						if (taken_ == false) {
							int price = 100000;
							string name_ = pInfo(p_)->world;
							std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(p_)->lastwrenchx + (pInfo(p_)->lastwrenchy * 100)];
								if (block_->fg != 5814) break;
								uint32_t guild_id = pInfo(p_)->guild_id;
								vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
								if (p != guilds.end()) {
									Guild* guild_information = &guilds[p - guilds.begin()];
									for (GuildMember member_search : guild_information->guild_members) {
										if (to_lower(member_search.member_name) == to_lower(pInfo(p_)->tankIDName)) {
											if (member_search.role_id == 3) {
												if (guild_information->guild_mascot[0] == 0 && guild_information->guild_mascot[1] == 0) price = -100000;
												else price = -250000;
												if (pInfo(p_)->gems >= abs(price)) {
													VarList::OnBuxGems(p_, price);
													VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`wYou have " + a + (price == -250000 ? "changed" : "created") + " Guild Flag!``", 0, 0);
													guild_information->guild_mascot[0] = pInfo(p_)->set_guild_fg;
													guild_information->guild_mascot[1] = pInfo(p_)->set_guild_bg;
													PlayerMoving data_{};
													data_.packetType = 5, data_.punchX = pInfo(p_)->lastwrenchx, data_.punchY = pInfo(p_)->lastwrenchy, data_.characterState = 0x8;
													BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
													BYTE* blc = raw + 56;
													form_visual(blc, *block_, *world_, p_, false);
													for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
														if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
														if (pInfo(cp_)->world == pInfo(p_)->world) {
															send_raw(cp_, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
														}
														if (pInfo(cp_)->guild_id == guild_id) {
															Clothing_V2::Update_Value(cp_, true);
															Clothing_V2::Update(cp_);
															VarList::OnConsoleMessage(cp_, "`5[GUILD ALERT]`` " + pInfo(p_)->tankIDName + " " + (price == -250000 ? "changed" : "created") + " Guild Flag!");
														}
													}
												}
												else {
													VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`wYou don't have that much gems!``", 0, 0);
												}
												break;
											}
										}
									}
								}
							}
						}
					}
				}
				break;
			}
		}
		else if (a_[i_] == "abondonguild") {
			if (a_.size() != 7 and a_.size() != 8 and a_.size() != 11) break;
			if (pInfo(p_)->guild_id == 0) break;
			if (a_.size() == 8 or a_.size() == 11) {
				string button_clicked = a_.size() == 8 ? a_[5] : a_[8];
				if (button_clicked == "back") {
					string name_ = pInfo(p_)->world;
					std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
					if (p != worlds.end()) {
						World* world_ = &worlds[p - worlds.begin()];
						int x_ = 0, y_ = 0;
						int ySize = (int)world_->blocks.size() / 100, xSize = (int)world_->blocks.size() / ySize;
						{
							std::vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [&](const WorldBlock& a) { return a.fg == 5814; });
							if (p != world_->blocks.end()) {
								int i_ = p - world_->blocks.begin();
								x_ = i_ % xSize;
								y_ = i_ / xSize;
							}
						}
						WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
						DialogHandle::Lock(p_, world_, block_->fg, x_, y_);
					}
				}
				else if (button_clicked == "abondonguild") {
					if (not isdigit(a_[5][0])) break;
					int planting_id_ = std::atoi(a_[5].c_str());
					if (planting_id_ >= items.size() or planting_id_ < 0) break;
					if (items[planting_id_].blockType != LOCK or planting_id_ == 202 or planting_id_ == 204 or planting_id_ == 206 or planting_id_ == 4994 or planting_id_ == 10000) {
						break;
					}
					string name_ = pInfo(p_)->world;
					std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
					if (p != worlds.end()) {
						World* world_ = &worlds[p - worlds.begin()];
						uint32_t guild_id = world_->guild_id;
						vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
						if (p != guilds.end()) {
							Guild* guild_information = &guilds[p - guilds.begin()];
							for (GuildMember member_search : guild_information->guild_members) {
								if (to_lower(member_search.member_name) == to_lower(pInfo(p_)->tankIDName)) {
									if (member_search.role_id != 3) break;
									int ySize = (int)world_->blocks.size() / 100, xSize = (int)world_->blocks.size() / ySize;
									{
										std::vector<WorldBlock>::iterator p = find_if(world_->blocks.begin(), world_->blocks.end(), [&](const WorldBlock& a) { return a.fg == 5814; });
										if (p != world_->blocks.end()) {
											int i_ = p - world_->blocks.begin();
											int x_ = i_ % xSize;
											int y_ = i_ / xSize;
											WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
											int c_ = -1;
											if (Inventory::Modify(p_, planting_id_, c_) == 0) {
												leave_guild(p_);
												block_->fg = planting_id_;
												world_->guild_id = 0;
												PlayerMoving data_{};
												data_.packetType = 15, data_.punchX = x_, data_.punchY = y_, data_.characterState = 0x8;
												data_.netID = name_to_number(to_lower(pInfo(p_)->tankIDName));
												data_.plantingTree = block_->fg;
												BYTE* raw = packPlayerMoving(&data_, 56);
												PlayerMoving data_2{};
												data_2.packetType = 5, data_2.punchX = x_, data_2.punchY = y_, data_2.characterState = 0x8;
												BYTE* raw2 = packPlayerMoving(&data_2, 112 + alloc_(world_, block_));
												BYTE* blc = raw2 + 56;
												form_visual(blc, *block_, *world_, p_, false);
												for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
													if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
													if (pInfo(cp_)->world == world_->name) {
														send_raw(cp_, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
														send_raw(cp_, 4, raw2, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
													}
												}
												delete[] raw;  // FIX MEMLEAK: dulu `delete[] raw, raw2, blc;` (comma op)
												delete[] raw2; // ? raw2 leak per guild abandon. blc = raw2 + 56 ? offset.
												VarList::OnConsoleMessage(p_, "`5[GUILD ALERT]`` Guild Abandoned!");
												for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
													if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
													if (pInfo(cp_)->world == pInfo(p_)->world) {
														gamepacket_t p(0, pInfo(p_)->netID);
														p.Insert("OnGuildDataChanged"), p.Insert(0), p.Insert(0), p.Insert(0), p.Insert(0), p.CreatePacket(cp_);
														gamepacket_t p2(0, pInfo(p_)->netID);
														p2.Insert("OnNameChanged"), p2.Insert(get_player_nick(p_)), p2.Insert(get_player_custom_wrench(p_)), p2.CreatePacket(cp_);
													}
												}
												std::vector<std::pair<long long int, string>>::iterator pz = find_if(top_guild.begin(), top_guild.end(), [&](const pair < long long int, string>& element) { return element.second == to_string(pInfo(p_)->guild_id); });
												if (pz != top_guild.end()) top_guild.erase(pz);
												guild_information->guild_settings &= ~SETTINGS::SETTINGS_0;
												guild_information->guild_settings |= SETTINGS::SETTINGS_0;
												guild_information->guild_name = "";
											}
										}
									}
									break;
								}
							}
						}
					}
				}
				break;
			}
			if (not isdigit(a_[5][0])) break;
			int planting_id_ = std::atoi(a_[5].c_str());
			if (planting_id_ >= items.size() or planting_id_ < 0) break;
			if (items[planting_id_].blockType != LOCK or planting_id_ == 202 or planting_id_ == 204 or planting_id_ == 206 or planting_id_ == 4994 or planting_id_ == 10000) {
				VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wAbandon Guild ``|left|5814|\nadd_textbox|`4You need to select a World Lock!|left||\nadd_button|back|Back|noflags|0|0|\nend_dialog|abondonguild|||");
				break;
			}
			VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\nadd_label_with_icon|big|`wAbandon Guild ``|left|5814|\nadd_textbox|`1Are you sure you want to abandon the guild?``|left|\nadd_textbox|`4BEWARE:``|left|\nadd_spacer|small|\nadd_textbox|`8- The Guild Lock will NOT return to your inventory!``|left|\nadd_textbox|`8- If you purchased a Guild Mascot this will be lost and will be made available to other guilds to buy!``|left|\nadd_textbox|`8- You will not get back any of the Gems you invested in the Guild!``|left|\nadd_textbox|`8- You will lose all Guild XP and levels the guild accomplished!``|left|\nadd_textbox|`8- When the Guild is abandoned it cannot be recovered!``|left|\nembed_data|worldlockId|" + to_string(planting_id_) + "\nadd_spacer|small|\nadd_button|abondonguild|Abandon Guild|noflags|0|0|\nadd_button|back|Back|noflags|0|0|\nend_dialog|abondonguild|||");
			break;
		}	
		else if (a_[i_] == "trash_item" or a_[i_] == "drop_item" or a_[i_] == "trash_item2") {
			if (a_.size() != 10 and a_[i_] != "trash_item2") break;
			if (a_[i_] == "trash_item2" and a_.size() != 11) break;
			if (not isdigit(a_[i_ + 2][0]) or not isdigit(a_[i_ + 5][0])) break;
			int id_ = std::atoi(a_[i_ + 2].c_str()), c_ = std::atoi(a_[i_ + 5].c_str()), c_1 = std::atoi(a_[i_ + 5].c_str()), count = 0, drop_fish = 0, invTotal = 0;
			if (c_1 <= 0 || c_1 > 200) break;
			int owned_count = Inventory::Contains(p_, id_);
			if (c_1 > owned_count) break;
			Inventory::Modify(p_, id_, invTotal);
			if (invTotal < 1 || invTotal == 0) break;
			if (id_ == 10328 and c_ > 1 and a_[i_] == "drop_item" and Event()->Halloween and pInfo(p_)->world == "GROWGANOTH") c_ = 1, c_1 = 1;
			if (items[id_].blockType == BlockTypes::FISH) {
				for (int i_ = 0, remove = 0; i_ < pInfo(p_)->inv.size(); i_++) if (pInfo(p_)->inv[i_].first == id_) drop_fish = pInfo(p_)->inv[i_].second, c_ = pInfo(p_)->inv[i_].second;
			}
			if (id_ >= items.size() or id_ == 18 or id_ == 32 or id_ == 6336 or id_ == 9384) break;
			string name_ = pInfo(p_)->world;
			std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				c_ = c_ * -1;
				if (a_[i_] == "trash_item" or a_[i_] == "trash_item2") {
					if (a_[i_] == "trash_item" and items[id_].untradeable) {
						if (c_ * -1 > Inventory::Contains(p_, id_)) break;
						else VarList::OnDialogRequest(p_, SetColor(p_) + "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`4Recycle`` " + to_string(c_ * -1) + " `w" + items[id_].name + "``|left|" + to_string(id_) + "|\nembed_data|itemID|" + to_string(id_) + "\nembed_data|count|" + to_string(c_ * -1) + "\nadd_textbox|You are recycling an `9UNTRADEABLE`` item. Are you absolutely sure you want to do this? There is no way to get the item back if you select yes.|left|\nend_dialog|trash_item2|NO!|Yes, I am sure|");
						break;
					}
					if (Inventory::Modify(p_, id_, c_) == 0) {
						CAction::Log(p_, "action|play_sfx\nfile|audio/trash.wav\ndelayMS|0");
						a_[i_ + 5].erase(0, min(a_[i_ + 5].find_first_not_of('0'), a_[i_ + 5].size() - 1));
						gamepacket_t p;
						p.Insert("OnConsoleMessage");
						if (pInfo(p_)->supp != 0) {
							int item = id_, maxgems = 0, receivegems = 0;
							int count = std::atoi(a_[i_ + 5].c_str());
							if (id_ % 2 != 0) item -= 1;
							maxgems = items[item].max_gems2;
							if (items[item].max_gems2 != 0) if (maxgems != 0) for (int i = 0; i < count; i++) receivegems += rand() % maxgems;
							if (items[item].max_gems3 != 0) receivegems = count * items[item].max_gems3;
							if (receivegems != 0) VarList::OnBuxGems(p_, receivegems);
							p.Insert((items[id_].blockType == BlockTypes::FISH ? (to_string(abs(c_))) + "lb." : a_[i_ + 5]) + " `w" + items[id_].ori_name + "`` recycled, `0" + Set_Count(receivegems) + "`` gems earned.");
						}
						else p.Insert((items[id_].blockType == BlockTypes::FISH ? (to_string(abs(c_))) + "lb." : a_[i_ + 5]) + " `w" + items[id_].ori_name + "`` trashed.");
						p.CreatePacket(p_);
						break;
					}
					break;
				}
				if (find(world_->active_jammers.begin(), world_->active_jammers.end(), 4758) != world_->active_jammers.end()) {
					if (to_lower(world_->owner_name) == to_lower(pInfo(p_)->tankIDName) || find(world_->admins.begin(), world_->admins.end(), to_lower(pInfo(p_)->tankIDName)) != world_->admins.end()) {

					}
					else {
						VarList::OnTalkBubble(p_, pInfo(p_)->netID, "The Mini-Mod says no dropping items in this world!", 0, 0);
						break;
					}
				}
				if (items[id_].untradeable or id_ == 1424 or id_ == 5816) {
					VarList::OnTextOverlay(p_, "You can't drop that.");
					break;
				}
				string name_ = pInfo(p_)->world;
				std::vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
				if (p != worlds.end()) {
					World* world_ = &worlds[p - worlds.begin()];
					WorldDrop drop_{};
					drop_.id = id_;
					if (drop_fish == 0) drop_.count = c_1;
					else drop_.count = drop_fish;
					int a_ = rand() % 12;
					drop_.x = (pInfo(p_)->state == 16 ? pInfo(p_)->x - (a_ + 20) : (pInfo(p_)->x + 20) + a_);
					drop_.y = pInfo(p_)->y + rand() % 16;
					if (drop_.x / 32 < 0 or drop_.x / 32 >= world_->max_x or drop_.y / 32 < 0 or drop_.y / 32 >= world_->max_y) {
						VarList::OnTextOverlay(p_, "You can't drop that here, face somewhere with open space.");
						break;
					}
					BlockTypes type_ = FOREGROUND;
					int as_ = rand() % 12;
					int x = (pInfo(p_)->state == 16 ? pInfo(p_)->x - (a_ + 20) : (pInfo(p_)->x + 20) + as_);
					int y = pInfo(p_)->y + rand() % 16;
					int where_ = (pInfo(p_)->state == 16 ? x / 32 : round((double)x / 32)) + (y / 32 * 100);
					WorldBlock* block_ = &world_->blocks[(pInfo(p_)->state == 16 ? drop_.x / 32 : round((double)drop_.x / 32)) + (drop_.y / 32 * 100)];
					int xy = (pInfo(p_)->state == 16 ? drop_.x / 32 : round((double)drop_.x / 32)) + (drop_.y / 32 * 100);
					if (items[block_->fg].collisionType == 1 || block_->fg == 6 || items[block_->fg].blockType == BlockTypes::GATEWAY || items[block_->fg].toggleable and is_false_state(world_->blocks[(pInfo(p_)->state == 16 ? x / 32 : round((double)x / 32)) + (y / 32 * 100)], 0x00400000)) {
						VarList::OnTextOverlay(p_, items[block_->fg].blockType == BlockTypes::MAIN_DOOR ? "You can't drop items on the white door." : "You can't drop that here, face somewhere with open space.");
						break;
					}
					int count_ = 0;
					bool dublicated = false;
					world_->fresh_world = true;
					for (int i_ = 0; i_ < world_->drop_new.size(); i_++) {
						if (abs(world_->drop_new[i_][4] - drop_.y) <= 16 and abs(world_->drop_new[i_][3] - drop_.x) <= 16) {
							count_ += 1;
						}
						if (world_->drop_new[i_][0] == id_) if (world_->drop_new[i_][1] + drop_.count < 200) dublicated = true;
					}
					if (!dublicated) {
						if (count_ > 20) {
							VarList::OnTextOverlay(p_, "You can't drop that here, find an emptier spot!");
							break;
						}
					}
					int dropped = c_;
					if (Inventory::Modify(p_, id_, c_) == 0) {
						ServerPool::Logs::Add(pInfo(p_)->tankIDName + " in [" + pInfo(p_)->world + "] Dropped  `w" + to_string(abs(dropped)) + " " + (items[id_].blockType == BlockTypes::FISH ? "lb." : "") + " " + items[id_].ori_name + "``." + (items[id_].rarity > 363 ? "" : " Rarity: `w" + to_string(items[id_].rarity) + "``"), "Drop Item");
						bool droppeds = false;
						if (world_->name == "GROWGANOTH" and Event()->Halloween) {
							if (drop_.x / 32 == 47 || drop_.x / 32 == 48 || drop_.x / 32 == 49 || drop_.x / 32 == 50 || drop_.x / 32 == 51) {
								if (drop_.y / 32 == 14 || drop_.y / 32 == 15) {
									int give_ = abs(dropped), give_back = abs(dropped);
									int itemid = (items[id_].consume_prize.size() == 0 ? 0 : items[id_].consume_prize[rand() % items[id_].consume_prize.size()]);
									bool consume = false;
									if (itemid != 0 && items[id_].consume_needed == 0) droppeds = true, consume = true;
									else if ((id_ == 10328 && abs(dropped) == 1) or items[id_].rarity != 0) {
										give_ = 1;
										int prize = 0;
										switch (id_) {
										case 1950: prize = 4152;  break;
										case 2722: prize = 3114;  break;
										case 910: prize = 1250;  break;
										case 11046: prize = 11314;  break;
										case 94: case 604: case 2636: case 2908: prize = 3108;  break;
										case 274: case 276: prize = 1956;  break;
										case 10730: prize = 11332;  break;
										case 10732: prize = 11334;  break;
										case 10734: prize = 11336;  break;
										case 6176: prize = 9042;  break;
										case 3040: prize = 3100;  break;
										case 11666: prize = 12414;  break;
										case 6144: prize = 7104;  break;
										case 7998: prize = 9048;  break;
										case 1162: prize = 3126;  break;
										case 9322: prize = 10184;  break;
										case 7696: prize = 10186;  break;
										case 1474: prize = 1990;  break;
										case 1506: prize = 1968;  break;
										case 2386: prize = 4166;  break;
										case 9364: prize = 10206;  break;
										case 10576: prize = 11322;  break;
										case 4960: prize = 5208;  break;
										case 6196: prize = 9056;  break;
										case 4326: prize = 7122;  break;
										case 2860: case 2268: prize = 4172;  break;
										case 1114: prize = 4156;  break;
										case 362: prize = 1234;  break;
										case 11574: prize = 12772;  break;
										case 4334: prize = 5250;  break;
										case 1408: prize = 5254;  break;
										case 4338: prize = 5252;  break;
										case 3288: prize = 4138;  break;
										case 3296: prize = 4146;  break;
										case 3290: prize = 4140;  break;
										case 3292: prize = 4142;  break;
										case 3298: prize = 4148;  break;
										case 3294: prize = 4144;  break;
										case 900: case 7754: case 7136: case 1576: case 7752: case 1378: case 7758: case 7760: case 7748:  prize = 2000;  break;
										case 1746: prize = 1960;  break;
										case 5018: prize = 5210;  break;
										case 1252: prize = 1948;  break;
										case 1190: prize = 1214;  break;
										case 2868: prize = 7100;  break;
										case 11638: prize = 12780;  break;
										case 11534: prize = 12416;  break;
										case 1830: prize = 1966;  break;
										case 920: case 914: case 918: case 916: case 924: case 922: case 2206: case 4444: case 2038: prize = 1962;  break;
										case 3556: { vector<int>random_prize = { 4186,4188 }; prize = random_prize[rand() % random_prize.size()]; } break;
										case 762: prize = 4190;  break;
										case 3818: prize = 10192;  break;
										case 366: prize = 4136;  break;
										case 9262: prize = 10212;  break;
										case 1294: prize = 5236;  break;
										case 1242: prize = 5216;  break;
										case 1244: prize = 5218;  break;
										case 1246: prize = 5220;  break;
										case 1248: prize = 5214;  break;
										case 7028: case 7030: case 7026: prize = 12396;  break;
										case 3016: case 5528: case 3018: case 5526: prize = 4248;  break;
										case 242: prize = 1212;  break;
										case 2972: prize = 4182;  break;
										case 3014: case 3012: case 2914: prize = 4246;  break;
										case 1460: prize = 1970;  break;
										case 2392: prize = 9114;  break;
										case 3218: case 3020: prize = 3098;  break;
										case 3792: prize = 5244;  break;
										case 1198: prize = 5256;  break;
										case 8468: prize = 10232;  break;
										case 2984: prize = 3118;  break;
										case 4360: prize = 10194;  break;
										case 10148: prize = 12418;  break;
										case 5012: case 1018: case 592: prize = 1178;  break;
										case 10406: prize = 11316;  break;
										case 10652: prize = 12420;  break;
										case 882: prize = 1232;  break;
										case 1934: prize = 3124;  break;
										case 10626: prize = 11312;  break;
										case 6160: prize = 9040;  break;
										case 3794: prize = 10190;  break;
										case 8018: prize = 9034;  break;
										case 2390: prize = 3122;  break;
										case 5246: { vector<int>random_prize = { 9050,9052,9054 }; prize = random_prize[rand() % random_prize.size()]; } break;
										case 6798: prize = 7126;  break;
										case 9722: case 9724: prize = 10200;  break;
										case 9388: prize = 10234;  break;
										case 4732: prize = 7124;  break;
										case 10804: prize = 11320;  break;
										default: prize = 0;
										}
										if (prize == 0 and items[id_].rarity < 367) {
											vector<int> growganoth, rare_growganoth;
											if (items[id_].rarity >= 1) growganoth.push_back(1208);
											if (items[id_].rarity >= 5) growganoth.insert(growganoth.end(), { 1992,5256,11348 });
											if (items[id_].rarity >= 10) growganoth.insert(growganoth.end(), { 1198,1222,11342,12786 });
											if (items[id_].rarity >= 15) growganoth.insert(growganoth.end(), { 1982,1200 });
											if (items[id_].rarity >= 20) growganoth.insert(growganoth.end(), { 1220,4160,10198 });
											if (items[id_].rarity >= 25) growganoth.insert(growganoth.end(), { 1202,5240 });
											if (items[id_].rarity >= 30) growganoth.insert(growganoth.end(), { 1238,4162,5238,7116,10196,10316 });
											if (items[id_].rarity >= 35) growganoth.insert(growganoth.end(), { 1168,9044 });
											if (items[id_].rarity >= 40) growganoth.insert(growganoth.end(), { 1172,3116,4164,12788 });
											if (items[id_].rarity >= 45) growganoth.push_back(1230);
											if (items[id_].rarity >= 50)  growganoth.insert(growganoth.end(), { 1192,1194,4164,10202,11340 });
											if (items[id_].rarity >= 55)  growganoth.insert(growganoth.end(), { 1226,7108 });
											if (items[id_].rarity >= 60)  growganoth.insert(growganoth.end(), { 1196,12784 });
											if (items[id_].rarity >= 65)  growganoth.insert(growganoth.end(), { 1236,9046 });
											if (items[id_].rarity >= 70)  growganoth.insert(growganoth.end(), { 1182,1184,1186,1188,1972,1980,1994,7110,11330 });
											if (items[id_].rarity >= 73)  growganoth.insert(growganoth.end(), { 3102,4154,5222,5224,5232,10204 });
											if (items[id_].rarity >= 75)  growganoth.push_back(1988);
											if (items[id_].rarity >= 80)  rare_growganoth.insert(rare_growganoth.end(), { 1170,4158,7128,12790 });
											if (items[id_].rarity >= 85)  rare_growganoth.push_back(1984);
											if (items[id_].rarity >= 90)  rare_growganoth.insert(rare_growganoth.end(), { 1216,1218,5248,7112,7114,10322 });
											if (items[id_].rarity >= 97)  rare_growganoth.push_back(3106);
											if (items[id_].rarity >= 100)  rare_growganoth.push_back(7130);
											if (items[id_].rarity >= 101)  rare_growganoth.push_back(3110);
											if (id_ == 10328) {
												growganoth = { 3110, 7130, 3106,1216,1218,5248,7114,10322,7112, 12772, 12396, 10236,4152  , 3114  , 1250  , 11314  , 3108  , 1956  , 11332  , 11336  , 11334  , 9042  ,9048  , 3126  , 10186  , 1990  , 1968   , 5208  , 9056  , 7122  , 4172  , 4156  , 1234  , 5250  , 5254  , 5252  , 4138  , 4146  , 4140  , 4142  , 4148  , 4144  , 2000  , 1960  , 5210  , 1948  , 1214  , 7100  , 1966  , 1962  , 4188  , 4186  , 4190  , 10192  , 4136  , 10212  , 5236  , 5216  , 5218  , 5220  , 5214  , 4248  , 4182  , 4246  , 3098  , 5244  , 5256  , 10232  , 3118  , 10194  , 4192  , 1178  , 11316  , 1232  , 3124  , 11312  , 9040  , 10190  , 9034  , 3122  , 9050  , 9054  , 9052  , 7126  , 10200  , 10234  , 7124  , 11320  , 1210 };
												if (rand() % 270 < 1) growganoth = { 4166, 12418, 12416, 12780, 12420, 12414, 1970,9114, 11322 ,7104, 10206, 11318, 10184 ,3100 , 7102 };
											}
											else {
												if (items[id_].rarity >= 80 && rand() % 440 - give_back - items[id_].rarity < 1) itemid = rare_growganoth[rand() % rare_growganoth.size()];
											}
											if (itemid == 0) itemid = growganoth[rand() % growganoth.size()];
										}
										else itemid = prize;
										droppeds = true;
									}
									else {
										droppeds = true;
										Player_Respawn(p_, true, 0, 1);
										itemid = 0;
									}
									if (droppeds) {
										if (itemid != 0) {
											if (items[id_].consume_prize.size() == 0) {
												if (itemid == 4166 || itemid == 12418 || itemid == 12416 || itemid == 12780 || itemid == 12420 || itemid == 12414 || itemid == 1970 || itemid == 9114 || itemid == 11322 || itemid == 7104 || itemid == 10206 || itemid == 11318 || itemid == 10184 || itemid == 3100 || itemid == 7102) {
													for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; ++cp_) {
														if (cp_->state != ENET_PEER_STATE_CONNECTED or cp_->data == NULL) continue;
														VarList::OnConsoleMessage(cp_, "`4Growganoth`` honors " + pInfo(p_)->name_color + pInfo(p_)->tankIDName + "`` with a `5Rare " + items[itemid].name + "`` for their " + items[id_].ori_name + "!");
													}
												}
											}
											if (consume) {
												if (id_ == 10328) {
													DailyEvent_Task::Halloween(p_, "1", 1);
													pInfo(p_)->DarkKing_Sacrifices += 1;
													VarList::OnProgressUIUpdateValue(p_, 10328, pInfo(p_)->DarkKing_Sacrifices);
													VarList::OnProgressUISet(p_, 10328, pInfo(p_)->DarkKing_Sacrifices, 15);
												}
												if (id_ == 242) {
													DailyEvent_Task::Halloween(p_, "7", c_);
												}
												VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`4Growganoth turns your " + items[id_].ori_name + " into an " + items[itemid].ori_name + " and hatches it with evil!``", 0, 0);
												VarList::OnConsoleMessage(p_, "`4Growganoth turns your " + items[id_].ori_name + " into an " + items[itemid].ori_name + " and hatches it with evil!``");
											}
											else {
												VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`4Growganoth is pleased by your offering and grants a reward! Also, he eats you.``", 0, 0);
												VarList::OnConsoleMessage(p_, "`4Growganoth is pleased by your offering and grants a reward! Also, he eats you.``");
											}
											VarList::OnConsoleMessage(p_, "`2" + items[id_].ori_name + "`` was devoured by Growganoth!");
											if (Inventory::Modify(p_, itemid, give_) == 0) {
												if (itemid == 1970) DailyEvent_Task::Halloween(p_, "9", 1);
												VarList::OnTalkBubble(p_, pInfo(p_)->netID, "A `2" + items[itemid].ori_name + "`` was bestowed upon you!", 0, 0);
												VarList::OnConsoleMessage(p_, "A `2" + items[itemid].ori_name + "`` was bestowed upon you!");
											}
											else {
												Inventory::Modify(p_, id_, give_back);
												VarList::OnConsoleMessage(p_, "No inventory space.");
											}
										}
										else {
											VarList::OnTalkBubble(p_, pInfo(p_)->netID, "`4Growganoth rejects your puny offering and eats you instead!``", 0, 0);
											VarList::OnConsoleMessage(p_, "`4Growganoth rejects your puny offering and eats you instead!``");
											Inventory::Modify(p_, id_, give_back);
										}
										Player_Respawn(p_, true, 0, 1);
									}
								}
							}
						}
						if (droppeds == false) {
							if (id_ == 611 && block_->fg == 610 && Only_Access(p_, world_, block_)) {
								int x_ = xy % 100, y_ = xy / 100;
								add_magic_egg(p_, world_, block_, x_, y_, abs(dropped));
							}
							else {
								ServerPool::CctvLogs::Add(p_, "dropped", to_string(abs(dropped)) + " " + items[id_].name);
								VisualHandle::Drop(world_, drop_, pInfo(p_)->netID);
							}
							ServerPool::SaveDatabase::Players(pInfo(p_), false);
						}
					}
				}
			}
			break;
		}
	}
	return;
}