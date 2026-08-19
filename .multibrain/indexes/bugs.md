# Named Sub-Index: `bugs`

Use this file as a concise work log for one task area or topic.

Rules:
- Put the newest entry at the top
- Keep each entry to one line
- Store longer detail in a pointed file when needed
- When the file grows too long, summarize older entries into a context or memory note

## Entries

- 2026-08-07 12:32 UTC — Antigravity: Optimized ENet host channel count from 1 to 2 in BaseServer.h and tuned peer ping_interval (100ms -> 1000ms) and timeout limits (30s / 45s) in NopySource.cpp to prevent cellular CGNAT packet throttling/drops on mobile data networks (Telkomsel/XL/Indosat/Tri); compiled cleanly into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 11:42 UTC — Antigravity: Refined HTTP server port 443 binding log handling in HTTP.h and NopySource.cpp so that when external http.exe holds port 443, it logs a clean informational message ("Port 443 is managed by external http.exe...") instead of raw std::cerr error; compiled cleanly into Core/x64/Release/NopySource.exe.

- 2026-08-07 11:28 UTC — Antigravity: Fixed build compilation failures in MSBuild Release|x64 by adding legacy_stdio_definitions.lib to AdditionalDependencies in NopySource.vcxproj (resolving sprintf/fprintf/vsnprintf linker errors) and updating include/openssl/ssl.h to map SSL_get1_peer_certificate to SSL_get_peer_certificate for libssl.lib 1.1.1 compatibility; compiled cleanly into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 02:15 UTC — Antigravity: Fixed Daily Reward header label layout in DailyRewardSystem.h by setting add_label_with_icon alignment to `left` so EXP potion icon (ID 1488) renders on the left instead of splitting the text into `Daily [Icon] Reward`, compiled into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 02:07 UTC — Antigravity: Fixed Server Time center alignment in DailyRewardSystem.h by removing pipe separator (`|`) inside add_textbox string, compiled into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 01:39 UTC — Antigravity: Fixed Daily Reward timer bar alignment to center and added missing seconds (s_left) to Reset Countdown string in DailyRewardSystem.h.
- 2026-08-07 01:27 UTC — Antigravity: Added real-time Server Clock (`Server Time: HH:MM:SS`), Reset Countdown (`Reset In: Xh Ym Zs`), and `add_textured_progress_bar` event timer bar to Daily Reward dialog in `DailyRewardSystem.h`.
- 2026-08-06 16:07 UTC — Antigravity: Fixed login request error -8 by updating dead IP (54.196.114.129) and clearing invalid login_url in Gtps-Http-main/config.txt and Release/database/json/config.json, and restarting both http.exe and NopySource.exe.
- 2026-08-06 15:55 UTC — Antigravity: Fixed login request error -8 in HTTP.h by removing invalid loginurl parameter that triggered web authentication failures in Growtopia client, registering both GET and POST handlers for server_data.php and login_cert endpoints, and handling Environment()->Maintenance.
- 2026-08-06 12:58 UTC — Antigravity: Fixed Game Menu dialog layout in GameMenu::Dialog (WorldInfo.h) by inserting add_custom_break line breaks between custom button rows (Flipping Coin, Casino, Slot Machine, Mines Game) to eliminate button overlap.
- 2026-08-06 12:00 UTC — Antigravity: Fixed Marketplace dialog UI text scaling bug in DialogHandle::MarketPlace (WorldInfo.h), enter_my_market (WorldInfo.h), and enterplayeritem_ (dialog_return.h) by replacing broken text_scaling_string|+| with text_scaling_string|Subscribtions++++++++|.
- 2026-08-06 11:33 UTC — Antigravity: Optimized /maintenance command & console command (Commands.h & WorldInfo.h) by setting Environment()->Maintenance flag properly and disconnecting non-admin players smoothly on maintenance start to eliminate severe server lag.
- 2026-08-05 13:50 UTC — Antigravity: Fixed incoming server connection issues by enabling ServicePoll thread & built-in HTTPServer initialization, replacing hardcoded ENet port 17095 with Environment()->Server_Port, and providing IP fallback (127.0.0.1) in server_data.php response.
- 2026-08-05 10:50 UTC — Antigravity: Removed remote phone-home license check endpoints (privateserverid.my.id) and replaced hardcoded 3rd party loginurl redirection with Environment()->server_ip in HTTP.h; clean MSBuild Release|x64 build verified 0 errors.
- 2026-08-05 02:26 UTC — Antigravity: Removed remote_components from ydl_opts in NopySC.py to resolve JSONDecodeError on SoundCloud downloads when YouTube cookies fail.
- 2026-08-05 02:24 UTC — Antigravity: Fixed Spotify URL matcher in NopySC.py to support regional locale paths (e.g. /intl-id/track/) and updated cookies.txt with valid YouTube login cookies.
- 2026-07-30 07:56 UTC — Antigravity: Added incremental packet delay (350ms per item) to `VarList::OnBuxGems` (`PlayerInfo.h` + `PacketHandler.h`) to force client-side sequential display of stacked gems TalkBubbles (`Collected X Gems(gem)`); clean MSBuild release compilation into `Core/x64/Release/NopySource.exe`.
- 2026-07-30 07:37 UTC — Antigravity: Configured TalkBubble gems notification to individual stacking mode (`Collected X Gems(gem)`) per pickup in `VarList::OnBuxGems`; verified clean MSBuild release compilation into `Core/x64/Release/NopySource.exe`.
- 2026-07-30 07:22 UTC — Antigravity: Added gem collection aggregation (1000ms window) in `VarList::OnBuxGems` (`PlayerInfo.h` + `PacketHandler.h`) to group consecutive gem pickups into a single combined Talk Bubble (`Collected X Gems(gem)`); clean MSBuild release build into `Core/x64/Release/NopySource.exe`.
- 2026-07-30 07:10 UTC — Antigravity: Integrated automatic `OnTalkBubble` notification (`Collected X Gems(gem)`) inside `VarList::OnBuxGems` in `PacketHandler.h`; clean MSBuild release compilation into `Core/x64/Release/NopySource.exe`.
- 2026-07-30 06:49 UTC — Antigravity: Removed all unused dead code files (`License.h`, `Database_Handle.h`, `Gg`, `updateGhost.h`, `callbacks.c`, `callbacks.h`) and cleaned commented-out header includes in `NopySource.cpp`; clean MSBuild release build into `Core/x64/Release/NopySource.exe`.
- 2026-07-30 06:36 UTC — Antigravity: Refactored 44-branch `if-else` chain in `NopySource.cpp` (Ancient Riddle system) into a static const array with O(1) index lookup; clean MSBuild release compilation into `Core/x64/Release/NopySource.exe`.
- 2026-07-30 06:27 UTC — Antigravity: Converted `COLOR_*` ANSI constants in PacketHandler.h to `constexpr std::string_view`, eliminating repeated global heap string allocations across translation units; verified clean MSBuild release compilation into `Core/x64/Release/NopySource.exe`.
- 2026-07-30 06:19 UTC — Antigravity: Optimized `base64_decode_safe` in LoginHandler.h with static const lookup table initialization and vector capacity pre-allocation; verified single-instance `inline TEnvironment`; clean MSBuild release compilation into `Core/x64/Release/NopySource.exe`.
- 2026-07-30 05:57 UTC — Antigravity: Comprehensive optimization pass: converted `gamepacket_t::Insert` to use `std::string_view` avoiding temporary string heap allocations for literals; verified packet broadcast safety and audited DB load/save routines; compiled cleanly via MSBuild (`Release|x64`) into `Core/x64/Release/NopySource.exe`.
- 2026-07-29 12:44 UTC — Antigravity: Comprehensive audit & optimization pass: converted `unpackPlayerMoving` to stack return, single-pass `const std::string&` transformation helpers, pre-allocated `TextParse` serialization, switched `AntiDDoS` to `std::chrono::steady_clock`, reused `static std::locale` facets in `formatWithCommas`/`Set_Count`, refactored `explode` & `MiscUtils::Split`, and added null-peer guards. Successfully compiled via MSBuild (`Release|x64`) with 0 Errors and 0 Warnings (`Core/x64/Release/NopySource.exe`).
- 2026-07-26 23:19 WIB — Antigravity: Discovered and fixed 18 hidden peer loop early `return;` leaks and 2 un-deleted `raw2` battle pet packets in dialog_return.h, ensuring peer broadcasting loops use `continue;` so heap-allocated packets are properly freed.
- 2026-07-26 23:17 WIB — Antigravity: Audited all 43 enet_packet_create call sites and 177 dynamic allocations; fixed missing packet cleanup in CAction::Log, CreatePacket, SendPacketRaw, and scalar delete mismatches in packet_utils.h, PacketHandler.h, PlayerInfo.h, WorldInfo.h, and BaseServer.h.
- 2026-07-26 15:52 WIB — Antigravity: Fixed Ghost Jar / Ghostbuster trapping logic in NopySource.cpp by preventing ghost state corruption during jar usage and ensuring proper single-packet ghost position & state updates.
- 2026-07-25 20:24 WIB — Antigravity: Fixed memory leaks and unsafe ENet packet reuse in Commands.h, WorldInfo.h, and dialog_return.h by replacing manual sound packet allocation loops with VarList::OnPlaySound, adding missing deletes, and adding SendPacket fallback destruction.
- 2026-07-23 12:39 WIB — Antigravity: Made ghosts wander continuously in NopySource.cpp by enforcing minimum step offset, boundary reflection logic, maxTime safety checks, and seamless Type 34 binary payload packet updates.
- 2026-07-18 14:41 WIB — Antigravity: Removed loop print debug log at items.dat loading loop (from index 21698 to 21703) in BaseServer.h.
- 2026-07-18 11:38 WIB — Antigravity: Fixed print log showing 50,000 items (due to items vector resize check) by introducing ItemsDat_Count to track actual items count from items.dat file.
- 2026-07-18 11:35 WIB — Antigravity: Synced the outdated HTTP public cache items.dat with the game server's database/items.dat to fix mismatched items count between ENet and client.
- 2026-07-18 07:56 WIB — Antigravity: Removed raw debug printf and commented out conditional debug printfs in enet_new/protocol.cpp and enet_new/protocol.c.
- 2026-07-11 08:36 WIB — Antigravity: Fixed Ghost in a Jar movement by removing the incorrect check in NopySource.cpp that immediately erased newly spawned ghosts (IDs under 30000).
- 2026-07-11 04:58 WIB — Antigravity: Redirected the player info bank button (Bank_Central) to open the Central Bank dialog (DialogHandle::BankCentral) instead of Gems Bank (DialogHandle::bankgem_deposit) in dialog_return.h.
- 2026-07-11 04:55 WIB — Antigravity: Fixed custom wrench creator dialog font rendering bug by adding missing closing backticks to the dual layer icon label title in WorldInfo.h.
- 2026-07-09 18:20 WIB — Antigravity: Fixed cheats menu rendering/checkbox states bug by converting bitmask values to proper boolean strings ("1" or "0"), and ensured the cheats menu automatically refreshes when choosing items to autofarm.
- 2026-07-02 23:22 WIB — Kilo: Fixed 4 bugs: (1) `hasRepetitiveCharacters` unsigned underflow on empty string (BaseServer.h:109), (2) `SaveServerConfig` wrote wrong variable `regconf` instead of `j` (WorldInfo.h:1044), (3) `getNPCNickname` missing default return — UB (PlayerInfo.h:332), (4) `packFishMoving` float→int16 truncation — changed to int32 (PlayerInfo.h:1249-1250).
- 2025-06-21 00:00 WIB — Claude: FIXED memory leak in NET_MESSAGE_GAME_PACKET handler — added `delete pMov` before `break` in unhandled-packet cooldown disconnect path. Audited all 5 break paths, no other leaks. -> .multibrain/context/2025-06-21-memleak-unhandled-packet.md
