#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
#include <numeric>
#include <iterator>
#include <sstream>
#include <format>

// ============================================================
//  Game Generator – ported from GameGene.h (NiceTopia source)
//  Adapted for GrowRabbid packet/BinaryWriter2 API
// ============================================================

enum Modifier {
    SCORE,
    LIVE,
    HIT
};

enum GameAction {
    START             = 0,
    END               = 1,
    UPDATE_PLAYER_SCORE = 2,
    UPDATE_TEAM_SCORE  = 3,
    CAPTURE           = 4,
    UNK_VAL           = 5,
    LEAVE             = 6,
    JOIN              = 7
};

enum GameOptions {
    RESPAWN_ON_SCORE      = 1,
    RESET_ON_SCORE        = 2,
    OWNER_PLAYS           = 4,
    LATE_JOIN             = 8,
    SMASH_ENEMY_BLOCK     = 16,
    SMASH_OWN_BLOCK       = 32,
    DISABLE_MUSIC         = 64,
    DONT_USE_CHAT_LOG     = 128,
    RESTART_AUTOMATICALLY = 256
};

enum GameTeams {
    RED_RABBITS     = 0,
    BLUE_BOMBERS    = 1,
    YELLOW_YAKS     = 2,
    PURPLE_PENGUINS = 3,
    FREE_FOR_ALL    = 4
};

static const std::vector<std::string> TeamName{
    "Red Rabbits",
    "Blue Bombers",
    "Yellow Yaks",
    "Purple Penguins",
    "Spooky Skull"
};

struct WorldGame {
    std::string name{ "Classic Deathmatch" };
    uint8_t  teamSize{ 4 };           // min 0 max 4
    uint8_t  lives{ 3 };              // min 0 max 100
    uint32_t time{ 60 };              // min 0 max 3600 seconds
    int32_t  goals{ 5 };              // min 0 max 1 million
    uint8_t  min_team_size{ 0 };      // min 0 max 4
    uint8_t  max_team_size{ 0 };      // min 0 max 4

    // Point values (all min -100 max 100)
    int8_t pv_kill{ 1 };
    int8_t pv_last_standing{ 0 };
    int8_t pv_goal{ 0 };
    int8_t pv_flag{ 0 };
    int8_t pv_smash{ 0 };
    int8_t pv_die{ -1 };

    // Scoring
    uint8_t player_hits{ 3 };         // min 0 max 20
    uint8_t block_hits{ 3 };          // min 0 max 100

    uint32_t flags{ 0 };              // combined GameOptions
    std::vector<std::string> managers{};

    double timeStarted = 0;
    bool   started{ false };

    struct PlayerData {
        uint32_t netID;
        uint8_t  team;
        int32_t  score;
        int8_t   lives;
        int8_t   hits;
        std::pair<std::pair<int8_t, int8_t>, int8_t> capture =
            std::make_pair(std::make_pair(-1, -1), -1);
    };

    struct TeamData {
        int32_t score;
        std::vector<uint8_t> members{};
        void Reset() { score = 0; members.clear(); }
    };

    struct WinnersData {
        bool team;
        std::vector<uint8_t> members{};
    };

    struct BlockData {
        int8_t   x, y, teamId;
        uint16_t foreground;
        uint16_t background;
        uint32_t flags;
    };

    std::vector<PlayerData> players;
    std::vector<TeamData>   teams{ {}, {}, {}, {} };
    std::vector<BlockData>  destroyed_blocks{};
    WinnersData             winners;

    bool isOptionEnabled(GameOptions option) const {
        return (flags & option) != 0;
    }

    void editOption(GameOptions option, bool enabled) {
        if (enabled) flags |= option;
        else         flags &= ~option;
    }

    void Reset() {
        name           = "Classic Deathmatch";
        teamSize       = 4;
        lives          = 3;
        time           = 60;
        goals          = 5;
        min_team_size  = 0;
        max_team_size  = 0;
        pv_kill        = 1;
        pv_last_standing = 0;
        pv_goal        = 0;
        pv_flag        = 0;
        pv_smash       = 0;
        pv_die         = -1;
        player_hits    = 3;
        block_hits     = 3;
        flags          = 0;
        started        = false;
        managers.clear();
        teams          = { {}, {}, {}, {} };
        winners.team   = false;
        winners.members.clear();
        players.clear();
        destroyed_blocks.clear();
        timeStarted    = 0;
    }

    // ----------------------------------------------------------------
    //  Generate dialog string for GAME_GENERATOR or GAME_BLOCK wrench
    // ----------------------------------------------------------------
    std::string generateDialog(int id, int x, int y) const {
        std::string popup = "set_default_color|`o\n";
        popup += std::format("embed_data|tilex|{}\n", x);
        popup += std::format("embed_data|tiley|{}\n", y);

        if (items[id].blockType == BlockTypes::GAME_GENERATOR) {
            popup += "add_label_with_icon|big|`wGame Generator``|left|1660\n";
            popup += "add_spacer|small\n";

            if (!started) {
                popup += "add_button|start|`4Start Game!``|noflags|0|0\n";
                popup += "add_url_button|comment|`wGet Help``|noflags|https://growtopiagame.com/game_gen.php|Open Game Generator instructions in your browser?|0|0|\n";

                popup += std::format("add_text_input|name|Name:|{}|64\n",             name);
                popup += std::format("add_text_input|teamSize|Teams:|{}|2\n",          (int)teamSize);
                popup += std::format("add_text_input|lives|Lives:|{}|3\n",             (int)lives);
                popup += std::format("add_text_input|time|Time (Seconds):|{}|6\n",     time);
                popup += std::format("add_text_input|goals|Goal Score:|{}|8\n",        goals);
                popup += std::format("add_text_input|min_team_size|Min Team Size:|{}|3\n", (int)min_team_size);
                popup += std::format("add_text_input|max_team_size|Max Team Size:|{}|3\n", (int)max_team_size);

                popup += "add_textbox|Point Values:|left\n";
                popup += std::format("add_text_input|pv_kill|- Kill:|{}|4\n",             (int)pv_kill);
                popup += std::format("add_text_input|pv_last_standing|- Last Standing:|{}|4\n", (int)pv_last_standing);
                popup += std::format("add_text_input|pv_goal|- Goal:|{}|4\n",             (int)pv_goal);
                popup += std::format("add_text_input|pv_flag|- Flag:|{}|4\n",             (int)pv_flag);
                popup += std::format("add_text_input|pv_smash|- Smash:|{}|4\n",           (int)pv_smash);
                popup += std::format("add_text_input|pv_die|- Die:|{}|4\n",               (int)pv_die);

                popup += std::format("add_checkbox|respawn_on_score|Respawn on Score|{:d}\n",       isOptionEnabled(RESPAWN_ON_SCORE));
                popup += std::format("add_checkbox|reset_on_score|Reset on Score|{:d}\n",           isOptionEnabled(RESET_ON_SCORE));
                popup += std::format("add_checkbox|owner_plays|Owner Plays|{:d}\n",                 isOptionEnabled(OWNER_PLAYS));
                popup += std::format("add_checkbox|late_joins|Late Join|{:d}\n",                    isOptionEnabled(LATE_JOIN));
                popup += std::format("add_checkbox|smash_enemy_blocks|Smash Enemy Blocks|{:d}\n",   isOptionEnabled(SMASH_ENEMY_BLOCK));
                popup += std::format("add_checkbox|smash_own_blocks|Smash Own Blocks|{:d}\n",       isOptionEnabled(SMASH_OWN_BLOCK));
                popup += std::format("add_checkbox|disable_game_music|Disable Music|{:d}\n",        isOptionEnabled(DISABLE_MUSIC));
                popup += std::format("add_checkbox|dont_use_chat_log|Don't Use Chat Log|{:d}\n",    isOptionEnabled(DONT_USE_CHAT_LOG));
                popup += std::format("add_checkbox|restart_automatically|Restart Automatically|{:d}\n", isOptionEnabled(RESTART_AUTOMATICALLY));

                popup += std::format("add_text_input|player_hits|Player Hits:|{}|2\n", (int)player_hits);
                popup += std::format("add_text_input|block_hits|Block Hits:|{}|3\n",   (int)block_hits);
                popup += "add_spacer|small\n";
                popup += "end_dialog|game_update|Cancel|Update";
            }
            else {
                popup += "add_button|cancel|`4Cancel Game!``|noflags|0|0\n";
                popup += std::format("add_textbox|{}``|64\n", name);
                popup += std::format("add_smalltext|{}|left\n",
                    teamSize > 0 ? std::format("{} Teams", (int)teamSize) : "Free-for-all");
                if (lives > 0)  popup += std::format("add_smalltext|{} Lives|left\n",      (int)lives);
                if (time  > 0)  popup += std::format("add_smalltext|Time Limit: {}s|left\n", time);
                if (goals > 0)  popup += std::format("add_smalltext|Score Limit: {}|left\n", goals);

                popup += "add_textbox|Point Values:|64\n";
                if (pv_kill != 0)          popup += std::format("add_smalltext|- Kill a player: {}|left\n",       (int)pv_kill);
                if (pv_last_standing != 0) popup += std::format("add_smalltext|- Last player standing: {}|left\n",(int)pv_last_standing);
                if (pv_goal != 0)          popup += std::format("add_smalltext|- Touch Game Goal: {}|left\n",     (int)pv_goal);
                if (pv_flag != 0)          popup += std::format("add_smalltext|- Capture a Game Flag: {}|left\n", (int)pv_flag);
                if (pv_smash != 0)         popup += std::format("add_smalltext|- Smash a Game Block: {}|left\n",  (int)pv_smash);
                if (pv_die != 0)           popup += std::format("add_smalltext|- Die: {}|left\n",                 (int)pv_die);

                popup += "add_spacer|small\n";
                if (isOptionEnabled(RESPAWN_ON_SCORE))      popup += "add_smalltext|Players respawn when they score.|left\n";
                if (isOptionEnabled(RESET_ON_SCORE))        popup += "add_smalltext|All players, flags, and blocks reset when someone scores.|left\n";
                if (isOptionEnabled(LATE_JOIN))             popup += "add_smalltext|Players who arrive late can join.|left\n";
                if (player_hits > 0)                        popup += std::format("add_smalltext|Players can kill enemies in {} hits.|left\n", (int)player_hits);
                if (block_hits > 0)                         popup += std::format("add_smalltext|Players can smash all Game Blocks in {} hits|left\n", (int)block_hits);
                if (block_hits == 0)                        popup += "add_smalltext|Game Blocks are indestructible|left\n";
                if (isOptionEnabled(RESTART_AUTOMATICALLY)) popup += "add_smalltext|The game will automatically restart when it ends.|left\n";

                popup += "add_spacer|small\n";
                popup += "end_dialog|game_update|Exit|";
            }
        }
        else if (items[id].blockType == BlockTypes::GAME_BLOCK) {
            std::string blkname = items[id].name;
            popup += std::format("add_label_with_icon|big|`w{}``|left|{}\n", blkname, id);
            popup += "add_spacer|small\n";
            popup += "add_textbox|Choose a team:|left|\n";
            popup += "text_scaling_string|Penguins|\n";
            popup += "add_button_with_icon|team0|`$Rabbits``|noflags|-10|\n";
            popup += "add_button_with_icon|team1|`$Bombers``|noflags|-11|\n";
            popup += "add_button_with_icon|team2|`$Yaks``|noflags|-12|\n";
            popup += "add_button_with_icon|team3|`$Penguins``|noflags|-13|\n";
            popup += "add_button_with_icon|team4|`$None``|noflags|-14|\n";
            popup += "add_button_with_icon||END_LIST|noflags|0||\n";
            popup += "add_spacer|small\n";
            popup += "end_dialog|team_edit|Cancel|\n";
        }
        return popup;
    }

    PlayerData* GetPlayer(uint32_t netID) {
        auto it = std::find_if(players.begin(), players.end(),
            [netID](const PlayerData& p) { return p.netID == netID; });
        return (it != players.end()) ? &(*it) : nullptr;
    }

    void ModifyPlayer(ENetPeer* peer, uint8_t type, int8_t modifier) {
        auto player = GetPlayer(pInfo(peer)->netID);
        if (player == nullptr) return;
        if (type == SCORE) {
            int32_t score = 0;
            if (player->team == 4) {
                manageBattle(peer, UPDATE_PLAYER_SCORE, pInfo(peer)->netID, modifier);
                player->score += modifier;
                score = player->score;
            }
            else {
                manageBattle(peer, UPDATE_TEAM_SCORE, player->team, modifier);
                teams[player->team].score += modifier;
                score = teams[player->team].score;
            }
            if (goals != 0) {
                if ((goals < 0 && score <= goals) || (goals > 0 && score >= goals))
                    manageBattle(peer, END);
            }
        }
        else if (type == LIVE) player->lives += modifier;
        else if (type == HIT)  player->hits  += modifier;
    }

    bool isManager(const std::string& playerName) {
        return std::find(managers.begin(), managers.end(), playerName) != managers.end();
    }

    // ----------------------------------------------------------------
    //  Process dialog_return for game_update dialog
    // ----------------------------------------------------------------
    void processInput(ENetPeer* peer, const std::string& cch,
                      const std::vector<std::string>& managersInput)
    {
        std::stringstream ss(cch);
        std::string to, btn;
        bool doStart  = false;
        bool doCancel = false;

        managers = managersInput;

        while (std::getline(ss, to, '\n')) {
            std::vector<std::string> infoDat;
            size_t pos = 0;
            std::string cur = to;
            while ((pos = cur.find('|')) != std::string::npos) {
                infoDat.push_back(cur.substr(0, pos));
                cur.erase(0, pos + 1);
            }
            infoDat.push_back(cur);

            if (infoDat.empty()) continue;

            if (infoDat[0] == "buttonClicked") {
                btn = infoDat.size() > 1 ? infoDat[1] : "";
                if (btn == "start"  && !started) doStart  = true;
                else if (btn == "cancel" && started) doCancel = true;
            }
            else if (infoDat.size() == 2) {
                if (!started) {
                    try {
                        if      (infoDat[0] == "name")             name           = infoDat[1];
                        else if (infoDat[0] == "teamSize")         teamSize       = std::clamp<uint8_t>( std::stoi(infoDat[1]), 0, 4);
                        else if (infoDat[0] == "lives")            lives          = std::clamp<uint8_t>( std::stoi(infoDat[1]), 0, 100);
                        else if (infoDat[0] == "time")             time           = std::clamp<uint32_t>(std::stoi(infoDat[1]), 0u, 3600u);
                        else if (infoDat[0] == "goals")            goals          = std::clamp<int32_t>( std::stoi(infoDat[1]), 0, 1'000'000);
                        else if (infoDat[0] == "min_team_size")    min_team_size  = std::clamp<uint8_t>( std::stoi(infoDat[1]), 0, 64);
                        else if (infoDat[0] == "max_team_size")    max_team_size  = std::clamp<uint8_t>( std::stoi(infoDat[1]), 0, 64);
                        else if (infoDat[0] == "pv_kill")          pv_kill        = std::clamp<int8_t>(  std::stoi(infoDat[1]), -100, 100);
                        else if (infoDat[0] == "pv_last_standing") pv_last_standing = std::clamp<int8_t>(std::stoi(infoDat[1]), -100, 100);
                        else if (infoDat[0] == "pv_goal")          pv_goal        = std::clamp<int8_t>(  std::stoi(infoDat[1]), -100, 100);
                        else if (infoDat[0] == "pv_flag")          pv_flag        = std::clamp<int8_t>(  std::stoi(infoDat[1]), -100, 100);
                        else if (infoDat[0] == "pv_smash")         pv_smash       = std::clamp<int8_t>(  std::stoi(infoDat[1]), -100, 100);
                        else if (infoDat[0] == "pv_die")           pv_die         = std::clamp<int8_t>(  std::stoi(infoDat[1]), -100, 100);
                        else if (infoDat[0] == "respawn_on_score")     editOption(RESPAWN_ON_SCORE,      std::stoi(infoDat[1]));
                        else if (infoDat[0] == "reset_on_score")       editOption(RESET_ON_SCORE,        std::stoi(infoDat[1]));
                        else if (infoDat[0] == "owner_plays")          editOption(OWNER_PLAYS,           std::stoi(infoDat[1]));
                        else if (infoDat[0] == "late_joins")           editOption(LATE_JOIN,             std::stoi(infoDat[1]));
                        else if (infoDat[0] == "smash_enemy_blocks")   editOption(SMASH_ENEMY_BLOCK,     std::stoi(infoDat[1]));
                        else if (infoDat[0] == "smash_own_blocks")     editOption(SMASH_OWN_BLOCK,       std::stoi(infoDat[1]));
                        else if (infoDat[0] == "disable_game_music")   editOption(DISABLE_MUSIC,         std::stoi(infoDat[1]));
                        else if (infoDat[0] == "dont_use_chat_log")    editOption(DONT_USE_CHAT_LOG,     std::stoi(infoDat[1]));
                        else if (infoDat[0] == "restart_automatically") editOption(RESTART_AUTOMATICALLY, std::stoi(infoDat[1]));
                        else if (infoDat[0] == "player_hits")    player_hits = std::clamp<uint8_t>(std::stoi(infoDat[1]), 0, 20);
                        else if (infoDat[0] == "block_hits")     block_hits  = std::clamp<uint8_t>(std::stoi(infoDat[1]), 0, 100);
                    }
                    catch (...) {}
                }
            }
        }

        if (doStart)       manageBattle(peer, START);
        else if (doCancel) manageBattle(peer, END);
    }

    // ----------------------------------------------------------------
    //  Return destroyed blocks to world after game ends
    // ----------------------------------------------------------------
    void ReturnBlocks(ENetPeer* peer) {
        if (destroyed_blocks.empty()) return;

        BinaryWriter2 br;
        for (const auto& dBlock : destroyed_blocks) {
            br.write<uint32_t>((uint32_t)dBlock.x);
            br.write<uint32_t>((uint32_t)dBlock.y);
            br.write<uint16_t>(dBlock.foreground);
            br.write<uint16_t>(dBlock.background);
            br.write<uint32_t>(dBlock.flags);
            br.write<uint8_t>(0x10);
            br.write<uint8_t>((uint8_t)dBlock.teamId);
        }
        br.write<uint32_t>((uint32_t)-1);

        GUP::GameUpdatePacket* multiPacket =
            (GUP::GameUpdatePacket*)std::malloc(sizeof(GUP::GameUpdatePacket) + br.get_pos());
        multiPacket->m_type      = 6;
        multiPacket->m_flags     = 0x8;
        multiPacket->m_int_x     = (uint32_t)-1;
        multiPacket->m_int_y     = (uint32_t)-1;
        multiPacket->m_data_size = (uint32_t)br.get_pos();
        std::memcpy(&multiPacket->m_data, br.get(), br.get_pos());

        for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
            if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data ||
                pInfo(peer)->world != pInfo(cp)->world) continue;
            GUP::send_packet(cp, 4, multiPacket,
                sizeof(GUP::GameUpdatePacket) + br.get_pos(), ENET_PACKET_FLAG_RELIABLE);
        }
        std::free(multiPacket);
    }

    // ----------------------------------------------------------------
    //  Core battle state machine
    // ----------------------------------------------------------------
    void manageBattle(ENetPeer* peer, uint32_t action,
                      uint8_t teamIdOrNetId = 0, int32_t modifier = 0)
    {
        GUP::GameUpdatePacket packet{};

        switch (action) {

        case START: {
            std::vector<ENetPeer*> worldPlayers;
            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;
                if (isManager(pInfo(cp)->tankIDName) && !isOptionEnabled(OWNER_PLAYS)) continue;
                worldPlayers.push_back(cp);
            }

            int min_team = (min_team_size == 0) ? 1  : (int)min_team_size;
            int max_team = (max_team_size == 0) ? 64 : (int)max_team_size;

            if ((int)worldPlayers.size() < (int)teamSize * min_team) {
                gamepacket_t p;
                p.Insert("OnConsoleMessage");
                p.Insert("Not enough players to start!");
                p.CreatePacket(peer);
                break;
            }

            std::vector<uint32_t> playerTeams(worldPlayers.size(), 0);
            if (teamSize > 0) {
                std::vector<uint32_t> teamCounts(teamSize, 0);
                for (uint32_t i = 0; i < worldPlayers.size(); ++i) {
                    uint8_t assignedTeam = i % teamSize;
                    uint8_t attempts = 0;
                    while (teamCounts[assignedTeam] >= (uint32_t)max_team && attempts < teamSize) {
                        assignedTeam = (assignedTeam + 1) % teamSize;
                        ++attempts;
                    }
                    if (teamCounts[assignedTeam] < (uint32_t)max_team) {
                        playerTeams[i] = assignedTeam;
                        teamCounts[assignedTeam]++;
                    }
                }
            }
            else {
                std::fill(playerTeams.begin(), playerTeams.end(), 4);
            }

            int h = time / 3600;
            int m = (time / 60) % 60;
            int s = time % 60;
            std::string timeStr = (h > 0)
                ? std::format("{}:{:02}:{:02}", h, m, s)
                : std::format("{}:{:02}", m, s);

            std::string msg = std::format(
                "`2{}:`` `o{}{}. {} (`5{}``) ``",
                name,
                teamSize > 0 ? std::format("{} Teams battle it out", (int)teamSize)
                             : "Free-for-all battle",
                lives > 1 ? std::format(" with {} lives", (int)lives) : "",
                goals > 0 ? std::format("{} points to win!", goals) : "Highest score wins!",
                timeStr
            );

            // Build START packet data
            BinaryWriter2 buffer;
            buffer.write(name);
            buffer.write<uint32_t>(time);
            buffer.write<uint8_t>(teamSize);
            for (int idx = 0; idx < (int)teamSize; ++idx)
                buffer.write<uint32_t>(0);

            GUP::GameUpdatePacket* startPacket =
                (GUP::GameUpdatePacket*)std::malloc(
                    sizeof(GUP::GameUpdatePacket) + buffer.get_pos());
            startPacket->m_type      = 28;
            startPacket->m_flags     = 0x8;
            startPacket->m_int_x     = START;
            startPacket->m_int_y     = flags;
            startPacket->m_data_size = (uint32_t)buffer.get_pos();
            std::memcpy(&startPacket->m_data, buffer.get(), buffer.get_pos());

            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;

                gamepacket_t pNotif;
                pNotif.Insert("OnAddNotification");
                pNotif.Insert("interface/large/game_on.rttex");
                pNotif.Insert(msg);
                pNotif.Insert("audio/race_start.wav");
                pNotif.CreatePacket(cp);

                gamepacket_t pConsole;
                pConsole.Insert("OnConsoleMessage");
                pConsole.Insert(msg);
                pConsole.CreatePacket(cp);

                GUP::send_packet(cp, 4, startPacket,
                    sizeof(GUP::GameUpdatePacket) + buffer.get_pos(),
                    ENET_PACKET_FLAG_RELIABLE);
            }
            std::free(startPacket);
            started = true;

            for (uint32_t i = 0; i < (uint32_t)worldPlayers.size(); ++i)
                manageBattle(worldPlayers[i], JOIN, (uint8_t)playerTeams[i]);
            break;
        }

        case END: {
            // Determine winners
            if (teamSize == 0) {
                winners.team = false;
                if (!players.empty()) {
                    int32_t maxScore = std::max_element(players.begin(), players.end(),
                        [](const PlayerData& a, const PlayerData& b) {
                            return a.score < b.score;
                        })->score;
                    for (const auto& pl : players)
                        if (pl.score == maxScore)
                            winners.members.push_back((uint8_t)pl.netID);
                }
            }
            else {
                winners.team = true;
                if (!teams.empty()) {
                    int32_t maxScore = std::max_element(teams.begin(), teams.end(),
                        [](const TeamData& a, const TeamData& b) {
                            return a.score < b.score;
                        })->score;
                    for (int idx = 0; idx < (int)teamSize; ++idx)
                        if (teams[idx].score == maxScore)
                            winners.members.push_back((uint8_t)idx);
                }
            }

            auto getPlayerName = [&](uint32_t netID) -> std::string {
                for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                    if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                    if (pInfo(peer)->world != pInfo(cp)->world) continue;
                    if ((uint32_t)pInfo(cp)->netID == netID)
                        return pInfo(cp)->tankIDName;
                }
                return "";
            };

            std::string winnerStr;
            if (winners.team && winners.members.size() <= 1) winnerStr += "The ";
            for (size_t i = 0; i < winners.members.size(); ++i) {
                if (i > 0)
                    winnerStr += (i == winners.members.size() - 1) ? " and " : ", ";
                winnerStr += winners.team
                    ? TeamName[winners.members[i]]
                    : getPlayerName(winners.members[i]);
            }

            int32_t winScore = 0;
            if (!winners.members.empty()) {
                winScore = winners.team
                    ? teams[winners.members[0]].score
                    : (GetPlayer(winners.members[0]) ? GetPlayer(winners.members[0])->score : 0);
            }

            std::string result = std::format(
                "`2{}:`` `o{}{}{} win with {}!``",
                name,
                timeStarted >= (double)time ? "Time's up! " : "",
                winners.members.size() > 1 ? "It's a tie! " : "",
                winnerStr,
                winScore
            );

            ReturnBlocks(peer);

            // Reset state before sending packets
            started = false;
            managers.clear();
            teams   = { {}, {}, {}, {} };
            winners.team = false;
            winners.members.clear();
            players.clear();
            destroyed_blocks.clear();
            timeStarted = 0;

            packet.m_type      = 28;
            packet.m_flags     = 0x0;
            packet.m_net_id    = -1;
            packet.m_int_y     = 0;
            packet.m_int_x     = END;
            packet.m_data_size = 0;

            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;

                gamepacket_t pNotif;
                pNotif.Insert("OnAddNotification");
                pNotif.Insert("interface/large/game_over.rttex");
                pNotif.Insert(result);
                pNotif.Insert("audio/race_end.wav");
                pNotif.CreatePacket(cp);

                gamepacket_t pConsole;
                pConsole.Insert("OnConsoleMessage");
                pConsole.Insert(result);
                pConsole.CreatePacket(cp);

                GUP::send_packet(cp, 4, &packet, sizeof(GUP::GameUpdatePacket),
                    ENET_PACKET_FLAG_RELIABLE);
            }
            break;
        }

        case UPDATE_TEAM_SCORE: {
            packet.m_type      = 28;
            packet.m_flags     = 0x0;
            packet.m_net_id    = -1;
            packet.m_int_data  = teamIdOrNetId;
            packet.m_int_x     = UPDATE_TEAM_SCORE;
            packet.m_int_y     = (uint32_t)modifier;
            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;
                GUP::send_packet(cp, 4, &packet, sizeof(GUP::GameUpdatePacket), ENET_PACKET_FLAG_RELIABLE);
            }
            break;
        }

        case UPDATE_PLAYER_SCORE: {
            packet.m_type  = 28;
            packet.m_flags = 0x0;
            packet.m_net_id = teamIdOrNetId;
            packet.m_int_x  = UPDATE_PLAYER_SCORE;
            packet.m_int_y  = (uint32_t)modifier;
            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;
                GUP::send_packet(cp, 4, &packet, sizeof(GUP::GameUpdatePacket), ENET_PACKET_FLAG_RELIABLE);
            }
            break;
        }

        case CAPTURE: {
            packet.m_type   = 28;
            packet.m_net_id = teamIdOrNetId;
            packet.m_int_x  = CAPTURE;
            packet.m_int_y  = (uint32_t)modifier;
            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;
                GUP::send_packet(cp, 4, &packet, sizeof(GUP::GameUpdatePacket), ENET_PACKET_FLAG_RELIABLE);
            }
            break;
        }

        case UNK_VAL: {
            packet.m_type      = 28;
            packet.m_flags     = 0x0;
            packet.m_net_id    = teamIdOrNetId;
            packet.m_int_data  = 0;
            packet.m_int_x     = UNK_VAL;
            packet.m_int_y     = 255;
            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;
                GUP::send_packet(cp, 4, &packet, sizeof(GUP::GameUpdatePacket), ENET_PACKET_FLAG_RELIABLE);
            }
            break;
        }

        case LEAVE: {
            packet.m_type  = 28;
            packet.m_flags = 0x0;
            packet.m_net_id = teamIdOrNetId;
            packet.m_int_x  = LEAVE;

            if (started) {
                auto player = GetPlayer(teamIdOrNetId);
                auto it = std::remove_if(players.begin(), players.end(),
                    [&](const PlayerData& pl) { return pl.netID == teamIdOrNetId; });
                if (it != players.end()) {
                    if (player && player->team != 4) {
                        auto& members = teams[player->team].members;
                        auto it2 = std::find(members.begin(), members.end(),
                            (uint8_t)pInfo(peer)->netID);
                        if (it2 != members.end())
                            members.erase(it2);
                    }
                    players.erase(it, players.end());
                }
            }

            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;
                GUP::send_packet(cp, 4, &packet, sizeof(GUP::GameUpdatePacket), ENET_PACKET_FLAG_RELIABLE);
            }
            break;
        }

        case JOIN: {
            packet.m_type   = 27;
            packet.m_net_id = pInfo(peer)->netID;
            packet.m_int_x  = teamIdOrNetId;

            if (started) {
                auto player = GetPlayer(pInfo(peer)->netID);
                if (player == nullptr) {
                    players.push_back({
                        .netID  = (uint32_t)pInfo(peer)->netID,
                        .team   = teamIdOrNetId,
                        .score  = 0,
                        .lives  = (int8_t)lives
                    });
                }
                else {
                    player->team = teamIdOrNetId;
                }

                if (teamIdOrNetId != 4) {
                    auto& members = teams[teamIdOrNetId].members;
                    if (std::find(members.begin(), members.end(),
                            (uint8_t)pInfo(peer)->netID) == members.end())
                        members.push_back((uint8_t)pInfo(peer)->netID);
                }
            }

            for (ENetPeer* cp = server->peers; cp < &server->peers[server->peerCount]; ++cp) {
                if (cp->state != ENET_PEER_STATE_CONNECTED || !cp->data) continue;
                if (pInfo(peer)->world != pInfo(cp)->world) continue;
                GUP::send_packet(cp, 4, &packet, sizeof(GUP::GameUpdatePacket), ENET_PACKET_FLAG_RELIABLE);
            }
            break;
        }

        default: break;
        }
    }
};
