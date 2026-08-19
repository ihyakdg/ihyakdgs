#pragma once
// ==============================================================================
//  WelcomeReward.h — Sistem Welcome Reward (Starter Pack)
// ==============================================================================
//  Memberikan item starter otomatis saat player baru membuat akun (GrowID).
//  Reward hanya bisa di-claim satu kali per akun (tersimpan permanen).
//
//  Struktur OOP:
//    - StarterItem  : struct untuk definisi item reward
//    - RewardManager: class yang mengelola pemberian reward & dialog
//
//  Item ID reference (Growtopia items.dat):
//    Wrench        = 2420  (tool)
//    Rare Item     = 1780  (Legendary Sword — representative rare)
//    Grow Spray    = 1778  (Deluxe Grow Spray)
//    Blue Potion   = 432   (Blue Potion)
//    Diamond Lock  = 1796
//    Magic Wand    = 4988  (Magic Wand / Growtopia Wand)
//    Dirt Block    = 2     (Dirt)
//    Angel Wings   = 490   (Angel Wings clothing)
//    World Lock    = 242
// ==============================================================================

#include "NopySource/Handle/PlayerInfo.h"
#include "NopySource/Handle/PacketHandler.h"
#include "NopySource/Handle/WorldInfo.h"

// ==============================================================================
//  Struct: StarterItem
//  Menyimpan definisi satu item reward (ID + jumlah + nama display).
// ==============================================================================
struct StarterItem {
	int item_id;
	int amount;
	std::string display_name;
};

// ==============================================================================
//  Class: RewardManager
//  Mengelola seluruh logika Welcome Reward:
//    - Daftar item starter
//    - Cek apakah reward sudah di-claim
//    - Berikan starter pack ke inventory player
//    - Tampilkan dialog welcome
//    - Logging server
// ==============================================================================
class RewardManager {
public:
	// ------------------------------------------------------------------
	//  hasClaimedReward
	//  Cek apakah player sudah pernah claim starter pack.
	//  Status tersimpan di field welcome_reward_claimed (player JSON).
	// ------------------------------------------------------------------
	static bool hasClaimedReward(ENetPeer* peer) {
		if (!peer || !peer->data) return true;
		return pInfo(peer)->welcome_reward_claimed;
	}

	// ------------------------------------------------------------------
	//  giveStarterPack
	//  Memberikan item starter dari new_get.list (Newbie Get config)
	//  ke inventory player. Dipanggil saat akun baru berhasil dibuat.
	//  Return: jumlah item yang berhasil diberikan.
	// ------------------------------------------------------------------
	static int giveStarterPack(ENetPeer* peer) {
		if (!peer || !peer->data) return 0;
		if (hasClaimedReward(peer)) return 0;

		int given = 0;

		for (int c_ = 0; c_ < (int)new_get.list.size(); c_++) {
			// Gems
			if (new_get.list[c_].Gemss != 0) {
				VarList::OnBuxGems(peer, new_get.list[c_].Gemss);
				given++;
			}
			// Coin
			if (new_get.list[c_].Coin != 0) {
				pInfo(peer)->gtwl += new_get.list[c_].Coin;
				given++;
			}
			// Seasonal Token
			if (new_get.list[c_].Seasonals_Token != 0) {
				int token_amt = new_get.list[c_].Seasonals_Token;
				Inventory::Modify(peer, guild_event_id, token_amt);
				given++;
			}
			// Items
			for (const auto& it_ : new_get.list[c_].items) {
				int id_ = it_.first, amt_ = it_.second;
				if (id_ <= 1 || id_ >= (int)items.size()) continue;
				int add = amt_;
				Inventory::Modify(peer, id_, add, true, false);
				given++;
			}
		}

		// Tandai sebagai sudah claim
		pInfo(peer)->welcome_reward_claimed = true;

		// Save ke database
		ServerPool::SaveDatabase::Players(pInfo(peer), false);

		return given;
	}

	// ------------------------------------------------------------------
	//  showWelcomeDialog
	//  Menampilkan dialog welcome dengan daftar item dari new_get.list.
	//  Dipanggil setelah akun berhasil dibuat.
	// ------------------------------------------------------------------
	static void showWelcomeDialog(ENetPeer* peer) {
		if (!peer || !peer->data) return;

		// Build reward list dengan tampilan modern
		std::string items_list = "";
		int reward_count = 0;

		for (int c_ = 0; c_ < (int)new_get.list.size(); c_++) {
			// Gems
			if (new_get.list[c_].Gemss != 0) {
				items_list += "\nadd_button_with_icon||`2" + Set_Count(new_get.list[c_].Gemss) + "x `wGems``|staticYellowFrame|14590|" + Set_Count(new_get.list[c_].Gemss) + "|";
				reward_count++;
			}
			// Coin
			if (new_get.list[c_].Coin != 0) {
				items_list += "\nadd_button_with_icon||`2" + Set_Count(new_get.list[c_].Coin) + "x `w" + Environment()->server_name + " Coin``|staticYellowFrame|244|" + Set_Count(new_get.list[c_].Coin) + "|";
				reward_count++;
			}
			// Seasonal Token
			if (new_get.list[c_].Seasonals_Token != 0) {
				items_list += "\nadd_button_with_icon||`2" + Set_Count(new_get.list[c_].Seasonals_Token) + "x `w" + guild_event_type + " Tokens``|staticYellowFrame|" + std::to_string(guild_event_id) + "|" + Set_Count(new_get.list[c_].Seasonals_Token) + "|";
				reward_count++;
			}
			// Items
			for (const auto& it_ : new_get.list[c_].items) {
				int id_ = it_.first;
				std::string name_ = (id_ > 0 && id_ < (int)items.size()) ? items[id_].name : "Unknown";
				items_list += "\nadd_button_with_icon||`2" + Set_Count(it_.second) + "x `w" + name_ + "``|staticYellowFrame|" + std::to_string(id_) + "|" + Set_Count(it_.second) + "|";
				reward_count++;
			}
		}

		std::string dialog =
			"set_default_color|`o\n"
			"set_bg_color|0,52,102,178|\n"
			"set_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`wWelcome to `#" + Environment()->server_name + "`w!``|left|5016|\n"
			"add_spacer|small|\n"
			"add_textbox|`2Your GrowID has been successfully created!``|left|\n"
			"add_spacer|small|\n"
			"add_textbox|`oAs a new player, you'll receive some `2starter benefits`` to help you begin your journey:|left|\n"
			"add_spacer|small|\n"
			"add_label_with_icon|small|`$Starter Pack `9(" + std::to_string(reward_count) + " items)``|left|9474|\n"
			"add_spacer|small|\n"
			"text_scaling_string|Subscription++++++++|" + items_list +
			"\nadd_button_with_icon||END_LIST|noflags|0||\n"
			"add_spacer|small|\n"
			"add_textbox|`oAll items have been `2added to your inventory``. Enjoy your adventure in `#" + Environment()->server_name + "`o!|left|\n"
			"add_spacer|small|\n"
			"add_smalltext|`5Tip: Use /help to see available commands. Type /daily to claim your daily reward!|left|\n"
			"add_spacer|small|\n"
			"add_quick_exit|\n"
			"end_dialog|welcome_reward||`wContinue|";

		VarList::OnDialogRequest(peer, dialog);
	}

	// ------------------------------------------------------------------
	//  processWelcomeReward
	//  Fungsi utama yang dipanggil saat akun baru dibuat.
	//  Memberikan item, tampilkan dialog, dan log ke server.
	// ------------------------------------------------------------------
	static void processWelcomeReward(ENetPeer* peer, const std::string& growid) {
		if (!peer || !peer->data) return;
		if (hasClaimedReward(peer)) return;

		// Log: new player registered
		Logger::Info("REGISTER", "[INFO] New player registered: " + growid);
		ServerPool::Logs::Add("[INFO] New player registered: " + growid, "WelcomeReward");

		// Berikan starter pack
		int given = giveStarterPack(peer);

		if (given > 0) {
			// Log: starter pack delivered
			Logger::Info("WELCOME_REWARD", "[INFO] Starter pack successfully delivered to: " + growid);
			ServerPool::Logs::Add("[INFO] Starter pack successfully delivered to: " + growid, "WelcomeReward");

			// Update inventory visuals
			Inventory::Visuals(peer);

			// Tampilkan dialog welcome
			showWelcomeDialog(peer);

			// Play sound
			CAction::Positioned(peer, pInfo(peer)->netID, "audio/double_chance.wav", 0);
		}
	}
};
