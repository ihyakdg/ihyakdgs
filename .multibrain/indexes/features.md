# Named Sub-Index: `features`

Use this file as a concise work log for one task area or topic.

Rules:
- Put the newest entry at the top
- Keep each entry to one line
- Store longer detail in a pointed file when needed
- When the file grows too long, summarize older entries into a context or memory note

## Entries

- 2026-08-10 00:48 UTC — Antigravity: Removed website top leaderboard file export (toprich.json in WorldInfo.h) and HTTP endpoints (/toprich.json & /api/toprich in HTTP.h & router.hpp); retained in-game /top menu.
- 2026-08-10 00:38 UTC — Antigravity: Removed website online/offline status file generation (status.json and online.txt writing in NopySource.cpp) and HTTP status endpoints in HTTP.h; deleted public status files.
- 2026-08-10 00:22 UTC — Antigravity: Completely removed RenderWorld feature (/renderworld, /render commands, render_kontol dialog, WorldRenderer.cpp, WorldRenderer.h, RenderWorldJS-main directory) from codebase; compiled into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.
- 2026-08-09 14:13 UTC — Antigravity: Fixed WorldRenderer C++ accuracy bugs (safe getJsonInt helper to prevent null field crashes, weather background texture pattern tiling across 3200x1920 canvas, bilinear scaling blit for inner items/drops, and rounding off-by-one pixel colors); compiled into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.


- 2026-08-09 12:30 UTC — Antigravity: Enhanced RenderWorldJS app.js to render Vending Machine & Display Box inner items (`b.id`), dropped items on floor (`drop_new`), and active weather backgrounds (`w.weather` ID 76, 2, 3, 68, etc.); successfully re-rendered MARKET.png (2.66 MB).

- 2026-08-09 10:22 UTC — Antigravity: Fixed RENDERWORLD command execution by adding dynamic up-to-4-level CWD path resolution for `RenderWorldJS-main/src/app.js` and stdout/stderr redirection to `database/server_logs/render.log`; compiled cleanly into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-09 10:05 UTC — Antigravity: Integrated RenderWorldJS with GTPS C++ server by adding `/renderworld` & `/render` commands in Commands.h (auto-saves world and spawns async node thread), installed canvas & sharp npm dependencies, updated config.js & app.js for dynamic command-line world rendering; compiled cleanly into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-08 17:47 UTC — Antigravity: Added automatic `online.txt` file generation (raw online player count) in NopySource.cpp status loop alongside `status.json`; compiled cleanly into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-08 17:36 UTC — Antigravity: Disabled built-in HTTP.h server initialization in NopySource.cpp so HTTP requests are exclusively handled by GTPS HTTP (http.exe); compiled cleanly into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 03:03 UTC — Antigravity: Reverted Close button back to Back button (`add_button|back...|`wBack|`) in `cc_program` dialog in `dialog_return.h` per user request (no build executed).

- 2026-08-07 02:30 UTC — Antigravity: Removed Chat Spammer feature from WorldInfo.h (dialog UI), dialog_return.h (request handler), and NopySource.cpp (autofarmdelay loop); compiled into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 02:25 UTC — Antigravity: Removed Slots input field from Auto-Farm section in `/cheats` dialog in WorldInfo.h, compiled into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 01:59 UTC — Antigravity: Removed 1-second live auto-refresh loop from Daily Reward dialog in PlayerInfo.h, DailyRewardSystem.h, Action.h, and NopySource.cpp; compiled cleanly into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 01:54 UTC — Antigravity: Reverted rainbow bar/text back to clean standard progress bar (`gui_event_bar2.rttex`) with green text (`2), compiled into Core/x64/Release/NopySource.exe with 0 errors and 0 warnings.

- 2026-08-07 01:47 UTC — Antigravity: Verified clean MSBuild Release|x64 compilation into Core/x64/Release/NopySource.exe (0 errors, 0 warnings) containing centered Daily Reward timer bar, 1s live auto-refresh clock, and dynamic rainbow color & bar texture cycling.

- 2026-08-07 01:42 UTC — Antigravity: Added dynamic rainbow color & bar texture cycling (`gui_event_bar1`..`bar4`) per second for Daily Reward timer bar and reset countdown text in DailyRewardSystem.h.

- 2026-08-07 01:41 UTC — Antigravity: Implemented 1-second live auto-refresh loop for Daily Reward dialog (Server Time clock & Reset Countdown bar) in PlayerInfo.h, DailyRewardSystem.h, Action.h, and NopySource.cpp.

- 2026-08-06 16:33 UTC — Antigravity: Removed Bank PIN prompts and PIN checks in DialogHandle::BankCentral (WorldInfo.h:14812) and dialog_return.h (Confirm_Depo, Confirm_Wd, Confirm_Transfer) so opening Bank Central opens directly to the main bank menu (no build per user request).
- 2026-08-06 16:28 UTC — Antigravity: Configured Credit Card (item ID 9950) in WorldInfo.h (switch(p_) case 9950) to equip item and trigger Bank Central menu dialog when used/worn on body (no build per user request).
- 2026-08-06 16:19 UTC — Antigravity: Enabled AutoConvertButton in PacketHandler.h (action: open_autocv, icon: 1796, order: 24, active: true) as active button #7 under GameMenuButton (no build per user request).
- 2026-08-06 15:52 UTC — Antigravity: Added add_spacer|small| between custom buttons and text descriptions in GameMenu::Dialog (WorldInfo.h:12364) to prevent button box overlapping text lines (no build per user request).
- 2026-08-06 15:46 UTC — Antigravity: Added GameMenuButton event button in PacketHandler.h (action: game_menu, icon: 758, order: 24, active: true) as active button #6 and registered actionMap["game_menu"] in Action.h to open GameMenu::Dialog(peer). MSBuild Release|x64 verified 0 errors.
- 2026-08-06 14:58 UTC — Antigravity: Deleted all unused files, test audio samples, temporary report logs, heavy binaries (ffmpeg.exe, ffprobe.exe, python-installer.exe), and leftover directories.
- 2026-08-06 14:56 UTC — Antigravity: Removed all NopySC files, directories (NopySC/, NopySC.py, NopySource/NopySC.py, run_nopysc.bat, run_nopysc.exe, run_nopysc_src.cpp, run_bot.bat) and cleaned path references.
- 2026-08-06 12:36 UTC — Antigravity: Built solution with MSBuild Release|x64 (Core/x64/Release/NopySource.exe built successfully with 0 warnings and 0 errors).
- 2026-08-06 12:31 UTC — Antigravity: Updated MarketplaceButton icon ID from 20488 to 13812 in PacketHandler.h.
- 2026-08-06 12:29 UTC — Antigravity: Built solution with MSBuild Release|x64 (Core/x64/Release/NopySource.exe built successfully with 0 warnings and 0 errors).
- 2026-08-06 12:24 UTC — Antigravity: Fixed EventButtons order values in PacketHandler.h (TopRichButton order=30 [highest=top], MarketplaceButton order=29, Farm Settings order=28) so TopRichButton appears at the very top of the screen side menu in Growtopia client.
- 2026-08-06 12:19 UTC — Antigravity: Built solution with MSBuild Release|x64 (Core/x64/Release/NopySource.exe built successfully with 0 warnings and 0 errors).
- 2026-08-06 12:14 UTC — Antigravity: Reordered JSON array and order values in PacketHandler.h (TopRichButton first order=20, MarketplaceButton second order=21, AutoPthtButton third order=22) so TopRichButton appears at the very top of the side menu in Growtopia client.
- 2026-08-06 12:07 UTC — Antigravity: Built solution with MSBuild Release|x64 (Core/x64/Release/NopySource.exe built successfully with 0 warnings and 0 errors).
- 2026-08-06 12:02 UTC — Antigravity: Corrected EventButtons order direction in PacketHandler.h (higher order value renders higher on client screen: TopRichButton order=28, MarketplaceButton order=27, Farm Settings order=26, Fish Exchange order=25, Event Exchange order=24).
- 2026-08-06 12:00 UTC — Antigravity: Built solution with MSBuild Release|x64 (Core/x64/Release/NopySource.exe built successfully with 0 warnings and 0 errors).
- 2026-08-06 12:05 UTC — Antigravity: Updated EventButtons order in PacketHandler.h so TopRichButton (order: 20) is at the top, followed by MarketplaceButton (order: 21), Farm Settings (order: 22), Fish Exchange (order: 23), and Event Exchange (order: 24).
- 2026-08-06 11:51 UTC — Antigravity: Updated MarketplaceButton text in PacketHandler.h to "Buy/Sell\nMarket" (Buy/Sell on top, Market on bottom).
- 2026-08-06 11:43 UTC — Antigravity: Built solution with MSBuild Release|x64 (NopySource.exe built successfully with 0 warnings and 0 errors).
- 2026-08-06 11:32 UTC — Antigravity: Updated MarketplaceButton icon ID from 13812 to 20488 in PacketHandler.h.
- 2026-08-06 10:53 UTC — Antigravity: Updated MarketplaceButton text in PacketHandler.h to "Sell/Buy\nMarketPlace" (Sell/Buy on top, MarketPlace on bottom).
- 2026-08-06 10:52 UTC — Antigravity: Added MarketplaceButton event button in PacketHandler.h (action: open_marketplace, icon: 13812, text: "Market\nplace") and registered actionMap["open_marketplace"] in Action.h to open Player Marketplace dialog (`DialogHandle::MarketPlace`).
- 2026-08-06 10:44 UTC — Antigravity: Disabled AutoConvertButton (active: false) in PacketHandler.h OnSuperMainStartAcceptLogon payload without building.
- 2026-08-06 10:37 UTC — Antigravity: Configured custom background/border colors (`add_custom_button`) for GameMenu buttons (Green Flipping Coin, Gold Casino, Purple Slot Machine, Red Mines Game) in WorldInfo.h without building.
- 2026-08-06 10:35 UTC — Antigravity: Replaced rttex image banner and buttons in GameMenu::Dialog (WorldInfo.h) with standard GTPS add_button items (flipping_coin, casino, slot_machine, mines) without building.
- 2026-08-06 09:59 UTC — Antigravity: Reverted eventButtons order in PacketHandler.h, placing AutoConvertButton back before TopRichButton (order: 25 & 26).
- 2026-08-06 09:41 UTC — Antigravity: Updated AutoPthtButton icon ID to 9726 in PacketHandler.h.
- 2026-08-06 09:35 UTC — Antigravity: Built solution with MSBuild Release|x64 (NopySource.exe built successfully with 0 warnings and 0 errors).
- 2026-08-06 09:31 UTC — Antigravity: Added TopRichButton event button in PacketHandler.h (action: toprichmenu, icon: 14406, text: "Top\nRich").
- 2026-08-06 09:30 UTC — Antigravity: Updated AutoPthtButton text in PacketHandler.h to "Farm\nSettings" (Farm on top, Settings on bottom).
- 2026-08-06 09:09 UTC — Antigravity: Updated ClashExchangeButton text in PacketHandler.h to "Event\nExchange" (Event on top, Exchange on bottom).
- 2026-08-06 09:08 UTC — Antigravity: Updated SellFishButton text in PacketHandler.h to "Fish\nExchange" (Fish on top, Exchange on bottom).
- 2026-08-06 09:01 UTC — Antigravity: Added AutoConvertButton event button in PacketHandler.h (action: open_autocv, icon: 1796, text: "Auto\nConvert") and registered actionMap["open_autocv"] in Action.h to open /autocv menu.
- 2026-08-06 08:58 UTC — Antigravity: Disabled Guild Event button (active: false for GuildEventButton) in PacketHandler.h OnSuperMainStartAcceptLogon payload without building.
- 2026-08-06 04:54 UTC — Antigravity: Configured HUD Event Button action to guild_event and registered actionMap[guild_event] to open Guild Event / Guild Clash menu in PacketHandler.h and Action.h. MSBuild Release|x64 verified 0 errors, Server online PID 8972.
- 2026-08-06 02:22 UTC — Antigravity: Removed auto-hash fixer feature (`RunHashFixer` execution and definition) upon ENet initialization in `NopySource.cpp`.
- 2026-08-05 10:04 UTC — Antigravity: Phase 2 deep C++ performance optimization (LoginHandler zero-allocation single-pass login parser, const std::string& base64_decode_safe, fast ASCII character manipulation in MiscUtils.hpp, MSBuild Release|x64 build verified 0 errors).
- 2026-08-05 09:35 UTC — Antigravity: Comprehensive deep C++ performance optimization (gamepacket_t SBO 512B stack buffer eliminating 95%+ ENet heap allocations, const std::string& parameter conversions across vz::* and CAction::* helpers, optimized TextParse::Set key search algorithm, and MSBuild Release|x64 build verified 0 errors).
- 2026-08-05 06:51 UTC — Antigravity: Added automatic dialog refresh OpenDialog(cp_) in ResetAllPlayersDailyReward inside DailyRewardSystem.h so /resetdaily immediately updates the active screen to Day 1 Claim Now!.
- 2026-08-05 06:43 UTC — Antigravity: Fixed status evaluation bug in DailyRewardSystem.h (auto-linked past days and claimed today status so Day 1 is always claimable and claimed items show CLAIMED! status correctly).
- 2026-08-05 06:37 UTC — Antigravity: Removed Locked label text from upcoming daily reward grid items in DailyRewardSystem.h so all 30 days display clean Day X labels.
- 2026-08-05 05:40 UTC — Antigravity: Added ResetAllPlayersDailyReward helper and /resetdaily admin command in DailyRewardSystem.h and Commands.h to reset all players' daily reward progress back to Day 1.
- 2026-08-05 05:38 UTC — Antigravity: Changed claimed daily reward grid item label text from Day X: Claimed to `2CLAIMED! in DailyRewardSystem.h.
- 2026-08-05 05:37 UTC — Antigravity: Configured claimed daily reward grid items in DailyRewardSystem.h to display item ID 15634 icon and Claimed status text.
- 2026-08-05 05:32 UTC — Antigravity: Removed disabled flag from all daily reward grid items in DailyRewardSystem.h so all 30 days appear brightly lit and active without dimming.
- 2026-08-05 05:25 UTC — Antigravity: Unified /daily system with /setdaily configuration across RewardConfig.h, dialog_return.h, and WorldInfo.h (synced dailyReward map and daily_reward.json so item changes made via /setdaily are instantly reflected in /daily).
- 2026-08-05 05:05 UTC — Antigravity: Reverted Daily Reward grid item layout in DailyRewardSystem.h back to pre-Kit layout (yellow frames, custom text scaling string spacing, space after status colon in labels, single END_LIST grid flow).
- 2026-08-05 04:58 UTC — Antigravity: Configured Daily Reward dialog claim flow to match Kit Pass system (item grid click opens Kit-style info popup with icon, description, status, green CLAIM button, and BACK button) across DailyRewardSystem.h and dialog_return.h.
- 2026-08-05 04:47 UTC — Antigravity: Added bottom Close button (without add_quick_exit X button) to Daily Reward dialog in DailyRewardSystem.h.
- 2026-08-05 04:38 UTC — Antigravity: Added spacing to Daily Reward dialog grid items in DailyRewardSystem.h (expanded text scaling string width, added space after status colon in labels, and removed no_padding_x frame flag).
- 2026-08-05 01:50 UTC — Antigravity: Converted Talking to the Moon audio to ogg and registered track in spotify_custom.json, custom_audio_registry.json, and newnih.txt.
- 2026-08-04 22:24 UTC — Antigravity: Enhanced run_nopysc.exe with automatic quiet Python installation (python-installer.exe) and auto pip requirements installation if missing.
- 2026-08-04 22:19 UTC — Antigravity: Created native C++ launcher run_nopysc.exe in root and NopySC folder to run NopySC.py directly or prompt Python installer if missing.
- 2026-08-04 14:16 UTC — Antigravity: Added automatic execution of hash_fixer.js on NopySource.exe startup with auto-synchronization to database/items.dat and HTTPS cache locations in NopySource.cpp and hash_fixer.js.
- 2026-07-27 03:32 WIB — Antigravity: Fixed /ghost command permission checks in Commands.h so all roles with /ghost permission or level > 0 (VIP, Mod, Admin, Dev, Staff, Donatur, Streamer, God, Unlimited, Coder, etc.) can use /ghost in any world.
- 2026-07-27 03:07 WIB — Antigravity: Enhanced Fake Online system with seamless /online integration, random DB player usernames, device/country distribution, and /fakeonline admin command in BaseServer.h, WorldInfo.h, and Commands.h.
- 2026-07-27 02:20 WIB — Antigravity: Configured Daily Reward dialog to match exact Kit Dialog structure (`kit_dialog` / `Kit_Pass`) with 500 custom dialog width in Commands.h and DailyRewardSystem.h.
- 2026-07-27 00:57 WIB — Antigravity: Restored clean 5x6 Kitt Pass layout for Daily Reward dialog across Commands.h, DailyRewardSystem.h, and RewardConfig.h per user feedback.
- 2026-07-27 00:51 WIB — Antigravity: Fixed Daily Reward dialog visual layout bugs from user screenshot (replaced missing header banner texture with standard icon label, adjusted grid from overflowing 9 columns to 6 columns x 5 rows, added proper row spacers to prevent vertical button overlap).
- 2026-07-27 00:36 WIB — Antigravity: Updated Daily Reward dialog layout to match Kit Dialog (`kit_dialog` / `Kit_Pass` format in WorldInfo.h) across Commands.h and DailyRewardSystem.h.
- 2026-07-27 00:27 WIB — Antigravity: Reverted Daily Reward dialog and notification changes back to original implementation across Commands.h, DailyRewardSystem.h, RewardConfig.h, and dialog_return.h per user request.
- 2026-07-27 00:16 WIB — Antigravity: Made Daily Reward dialog layout compact & rapat across Commands.h and DailyRewardSystem.h (6 items per row, eliminated unnecessary vertical spacer padding, set tight button text scaling).
- 2026-07-27 00:11 WIB — Antigravity: Removed on-screen daily reward notifications and reminders (`OnTextOverlay`, `OnTalkBubble`, `OnAddNotification`, and welcome tip text) across dialog_return.h, DailyRewardSystem.h, and WelcomeReward.h, keeping feedback clean in console logs.
- 2026-07-27 00:04 WIB — Antigravity: Updated Daily Reward dialog layout to match the Kit Dialog (`kit_dialog` / `Kit_Pass` format in WorldInfo.h) using top banner texture, Kit Pass progress bar, `text_scaling_string|Subscribtions+++|`, `no_padding_x` icon grid cards with `staticYellowFrame` / `staticBlueFrame` / `staticGreyFrame`, and bottom NOTE info box.
- 2026-07-26 15:17 WIB — Antigravity: Fixed Ghost rendering visibility in WorldInfo.h by auto-fallback visual_pos to last_pos when uninitialized (avoiding 0,0 spawn coordinates) and syncing full bulk NPC packets via sendWorldGhosts.
- 2026-07-26 15:13 WIB — Antigravity: Implemented ghost system update from `update ghost` folder (updateGhost packet 34, retRandomValuesFloat, ghostThread loop with player sliming & jar catching, thread launch in NopySource.cpp).
- 2026-07-26 14:20 WIB — Antigravity: Updated TopRichButton eventbutton icon ID from 7188 to 14406 in PacketHandler.h OnSuperMainStartAcceptLogon payload.
- 2026-07-26 12:35 WIB — Antigravity: Eliminated autoharvest stuttering/jeda in NopySource.cpp & WorldInfo.h by moving PthtAutoHarvest::tick_jobs() out of the 500ms world tick loop to execute every main loop iteration (~1ms) with smooth 30ms per-job ticking (kTreesPerTick=12), making harvesting 100% continuous at 33 FPS without pauses.
- 2026-07-26 12:18 WIB — Antigravity: Boosted autoharvest speed 7.5x in WorldInfo.h (kTreesPerTick=40, kScanPerTick=6000, kTickIntervalMs=100ms), increasing throughput from 53 trees/sec to ~400 trees/sec and eliminating scan delays across empty tiles.
- 2026-07-26 12:15 WIB — Antigravity: Fixed Magplant seed & stock detection in Commands.h and dialog_return.h by adding get_mag_stock_ref (handling block.c_ for item 6954/GAIA/SUCKER and block.pr for 5638/5930/9850/etc) and get_mag_seed_id (converting block ID to seed ID if needed), preventing Magplant stock from being skipped and falling back to 200 inventory seeds.
- 2026-07-26 12:07 WIB — Antigravity: Fixed autoplant 200 seed limit bug in dialog_return.h and WorldInfo.h by removing invalid block->bg == 0 check from planting search loop, allowing seeds from Magplant/inventory to plant on all empty soil tiles regardless of background walls.
- 2026-07-26 12:00 WIB — Antigravity: Fixed autoplant world reset bug in dialog_return.h by removing unnecessary Exit_World and Enter_World calls upon planting completion, which previously caused player count to drop to 0 and triggered save_world(world, true) memory unloading.
- 2026-07-26 11:56 WIB — Antigravity: Optimized PthtAutoHarvest in WorldInfo.h with timestamp caching (now_t), smooth ENet network batching (8 trees / 150ms), simplified ready tree check, and cached World pointer lookups to eliminate lag spikes and CPU overhead.
- 2026-07-26 11:14 WIB — Antigravity: Updated /autoplant and PthtAutoPlant in Commands.h and dialog_return.h to prioritize planting seeds from the player's active linked Magplant Remote (magnetron), with support for all Magplant variants (5638, 5930, 9850, 10266, 21220, Gaia, SUCKERS).
- 2026-07-26 10:54 WIB — Antigravity: Removed OnSendToServer reconnect packet and disconnect call from Get_GrowID in dialog_return.h, allowing players to create accounts in-game and continue playing seamlessly without any disconnect or loading screen while saving credentials via SetHasGrowID.
- 2026-07-26 10:14 WIB — Antigravity: Added /register in-game chat command in Commands.h, updated OnSendToServer port from hardcoded 55000 to dynamic Environment()->Server_Port in dialog_return.h to ensure credentials get saved to save.dat upon registration.

- 2026-07-26 06:16 WIB — Antigravity: Applied navy blue background (0,52,102,178 - 30% transparency) and solid thick white border (255,255,255,255) across all dialogs across the entire project (Action.h, Commands.h, dialog_return.h, DailyRewardSystem.h, DailyReward.h, DialogReturn.h, PacketHandler.h, WelcomeReward.h, WorldInfo.h, server_pool.h).
- 2026-07-26 06:10 WIB — Antigravity: Adjusted /online dialog background transparency to 30% (alpha 178 / 70% opacity) in Commands.h (ONLINEIYAGAMI and ONLINE_CHECK).
- 2026-07-26 06:02 WIB — Antigravity: Updated /online dialog border color to solid thick white (255,255,255,255) and background color to (0,52,102,160) in Commands.h (ONLINEIYAGAMI and ONLINE_CHECK).
- 2026-07-26 05:41 WIB — Antigravity: Added Top Rich event button (TopRichButton) directly under Sell Fish button (order 24) with BGL icon (ID 7188) in PacketHandler.h, registered toprichmenu in Action.h actionMap to open the Top Rich leaderboard dialog.
- 2026-07-26 05:24 WIB — Antigravity: Added Sell Fish event button (SellFishButton) directly under Exchange button (order 23) with item ID 3000 in PacketHandler.h, registered exchange_fish in Action.h actionMap to open the Sell Fish dialog.
- 2026-07-26 05:16 WIB — Antigravity: Configured Exchange event button (ClashExchangeButton) to directly open the Exchange Items dialog (Commands::ExchangeDialog) with icon 12158 and automatic directory/file initialization for exchange_items.json in Action.h and Commands.h.
- 2026-07-26 01:51 WIB — Antigravity: Increased maximum drop limit for /drop and fast drop commands (e.g., /drop <id> <count>, /wl, /dl, /bgl) from 200 to 10,000 items in Commands.h.
- 2026-07-26 01:48 WIB — Antigravity: Enabled displaying of all Lock items in Display Box / Display Block by removing BlockTypes::LOCK restriction in WorldInfo.h and setting block_possible_put for LOCK blockType in BaseServer.h.
- 2026-07-26 00:46 WIB — Antigravity: Matched /online dialog background color to dark oceanic teal (18,62,86,210) and crisp white border (255,255,255,255) based on the user's screenshot sample in Commands.h.
- 2026-07-26 00:41 WIB — Antigravity: Adjusted /online dialog background transparency to exactly 20% (alpha 204 / 80% opacity) in Commands.h.
- 2026-07-26 00:38 WIB — Antigravity: Updated /online dialog background color to transparent light blue (40,160,240,160) and border color to white (255,255,255,255) in Commands.h (ONLINEIYAGAMI and ONLINE_CHECK).
- 2026-07-26 00:28 WIB — Antigravity: Updated /online command dialog in Commands.h (ONLINEIYAGAMI) to display the player's current world name (or EXIT) next to username instead of round-trip ping (ms).
- 2026-07-26 00:19 WIB — Antigravity: Added full Ubuntu/Linux cross-platform support with CMakeLists.txt, build_ubuntu.sh script, and #ifdef _WIN32 preprocessor guards for Windows-only APIs in BaseServer.h and CrashHandler.cpp.
- 2026-07-26 00:13 WIB — Antigravity: Updated Magplant capacity text tags (get_mag_cap_tag) in WorldInfo.h to remove space after opening parenthesis (e.g. (25.000) instead of ( 25.000)) across all Magplant types (5638, 6946/6948/6954, 9850, 10266, 5930, 21220).
- 2026-07-25 22:10 WIB — Antigravity: Updated Special Magplant (ID 9850) capacity from 15,000 to 50,000 (50k) across use_mag, tile placement/loading block serialization, dialog limits, and header tags in WorldInfo.h and DialogReturn.h.
- 2026-07-25 22:09 WIB — Antigravity: Updated Magplant 5000 (ID 5638) capacity from 5,000 to 25,000 (25k) across use_mag, tile serialization, dialog limits, and header tags in WorldInfo.h and DialogReturn.h.
- 2026-07-25 21:39 WIB — Antigravity: Fixed Growtopia color code formatting in get_mag_cap_tag by adding space after green color code (`2), ensuring Magplant 5000 displays as ( 5.000 ) instead of ( 25.000 ), and reverted Magplant 5000 capacity back to 5000 in WorldInfo.h and DialogReturn.h.
- 2026-07-25 21:30 WIB — Antigravity: Updated Ultra Magplant (ID 10266) capacity from 100,000 to 500,000 across use_mag, placement/loading block serialization, dialog checks, and header tags in WorldInfo.h and DialogReturn.h.
- 2026-07-25 21:23 WIB — Antigravity: Added dynamic capacity text tags next to Magplant names in dialog headers (e.g. Magplant 5000 (5.000), Super Magplant (1.000.000), Unlimited Magplant (Unlimited)) in WorldInfo.h.
- 2026-07-25 21:15 WIB — Antigravity: Created new Unlimited Magplant item ID 21220 with 2,000,000,000 capacity, registered as SUCKER blockType in BaseServer.h, updated use_mag, machine placement/loading, remote interactions, and commands in WorldInfo.h, DialogReturn.h, Commands.h, and dialog_return.h.
- 2026-07-25 21:09 WIB — Antigravity: Unified all Magplant variants (5638, 5930, 9850, 10266) to use Remote ID 5640 (Magplant 5000 Remote) universally for remote issuance, planting/building storage access, and unlinking in DialogReturn.h and WorldInfo.h.
- 2026-07-25 17:26 WIB — Antigravity: Restricted particle 186 emission to block destruction only (block_->hp == 0) during autofarm in WorldInfo.h.
- 2026-07-25 17:21 WIB — Antigravity: Fixed particle rendering by switching to variants::OnParticleEffect standard call in WorldInfo.h and Commands.h.
- 2026-07-25 17:01 WIB — Antigravity: Added Particle 186 effect emission on tile destruction while autofarm is active in WorldInfo.h.
- 2026-07-25 16:53 WIB — Antigravity: Enhanced getAutoFarmSlot for extended Far Punch items and integrated real-time autofarm slot auto-sync in Clothing_V2::Update upon equipment changes.
- 2026-07-24 01:25 WIB — Antigravity: Added dedicated header overlay banner (`gtps_overlays.rttex 0|8`) directly above the 4 Immortal item buttons in shop tab 1, keeping the Event Day section separate.
- 2026-07-24 01:15 WIB — Antigravity: Reverted layout as requested (kept Immortal item buttons above Event Day banner in shop tab 1).
- 2026-07-24 01:12 WIB — Antigravity: Positioned the Event Day banner (0|6 and event banner button) above all Immortal item buttons in shop tab 1.
- 2026-07-24 01:07 WIB — Antigravity: Moved all Immortal item buttons (10820, 10822, 10824, 10826) to above the Event Day banner in shop tab 1.
- 2026-07-24 01:02 WIB — Antigravity: Moved Subscription banner and subscription buttons (8188, 8186, 9266) to the very top of shop tab 1.
- 2026-07-24 00:55 WIB — Antigravity: Added Immortal Alaskan King Crab Crown button in shop tab 1 using store_buttons44.rttex texture (frame 3|2) and created shop database JSON files (-10826.json & -immortal_alaskan_king_crab_crown.json).
- 2026-07-24 00:52 WIB — Antigravity: Added Immortal Da Vinci Wings button in shop tab 1 using store_buttons44.rttex texture (frame 2|2) and created shop database JSON files (-10824.json & -immortal_da_vinci_wings.json).
- 2026-07-24 00:50 WIB — Antigravity: Added Immortal Swordfish Sword button in shop tab 1 using store_buttons44.rttex texture (frame 1|2) and created shop database JSON files (-10822.json & -immortal_swordfish_sword.json).
- 2026-07-24 00:25 WIB — Antigravity: Added Immortal Dirt button in shop tab 1 (main menu) using store_buttons44.rttex texture and created shop database JSON files (-10820.json & -immortal_dirt.json).
- 2026-07-18 08:08 WIB — Antigravity: Speeded up fishing by reducing base wait time to 1000ms, incorporating the player's fish_speed_mult multiplier, and increasing the tick bite probability to 90%-100%.
- 2026-07-11 14:19 WIB — Antigravity: Reverted all global title/role unlock announcements and notifications back to the original clean state as requested.
- 2026-07-11 12:37 WIB — Antigravity: Formatted Top Rich rank as `9TOP RICH PLAYER `w[`4#N`w] and added Challenge Crown icon (ID 3138) at the end using `add_label_with_icon`.
- 2026-07-11 12:23 WIB — Antigravity: Repositioned Top Rich rank under the level bar, changed its format to `cTop Rich [`2#`w], and removed the card battle pet button from the profile menu.
- 2026-07-11 11:59 WIB — Antigravity: Added Top Rich rank display inside the player info dialog (wrenched player profile) for self, online and offline players.
- 2026-07-11 08:20 WIB — Antigravity: Modified /setmusic deletion logic to only clear the custom music setting database field, preserving block music placed in the world.
- 2026-07-11 07:59 WIB — Antigravity: Beautified the player info profile dialog for self and other players with structured formatting and clear section headers.
- 2026-07-11 07:37 WIB — Antigravity: Modified /setmusic command to remove the custom world music when run with no arguments.
- 2026-07-11 07:35 WIB — Antigravity: Added automatic /wave action with a 1000ms delay for players upon entering a world.
- 2026-07-11 07:15 WIB — Antigravity: Modified automatic AFK sleep detection timeout from 30 seconds to 10 seconds.
- 2026-07-11 07:03 WIB — Antigravity: Removed the Masterless Starglitter feature, disabling its regeneration tick, quest progression, Fate gacha shop exchanges, and dialog handlers.
- 2026-07-11 05:32 WIB — Antigravity: Reverted the "Edit Item" button style in item info dialogs back to a standard button (add_button) with white text, removing the custom green background style.
- 2026-07-11 05:26 WIB — Antigravity: Repositioned the OK button to the bottom of the normal item info dialog while keeping the "Edit Item" button above the "Item Properties" (props_section) section.
- 2026-07-11 05:19 WIB — Antigravity: Repositioned the OK button vertically below the "Edit Item" button in item info dialogs by moving it to the dialog body and disabling standard end_dialog buttons.
- 2026-07-11 05:13 WIB — Antigravity: Styled the "Edit Item" button in item info dialogs as a green button with white text using add_custom_button.
- 2026-07-11 05:03 WIB — Antigravity: Added "Edit Item" button in all item info dialogs (both seeds and normal items) except for locks (BlockTypes::LOCK), visible only to admins/creators (Role::Clist), which directly opens the EditItemPro dialog.
- 2026-07-11 04:56 WIB — Antigravity: Removed "nopy" username restriction from SETNEWGET and SETMUSIC commands in Commands.h so all players/admins can run them.
- 2026-07-10 12:47 WIB — Antigravity: Implemented Custom Titles list manager. Extended PlayerInfo with custom_titles vector, added db serialization, registered /ctitle command, and implemented custom_titles_manager dialog to support addition, deletion, and toggling of up to 20 titles.
- 2026-07-10 12:25 WIB — Antigravity: Implemented Custom Titles feature. Added custom_title and show_custom_title database fields, updated name tag formatting to prepend custom titles, added a toggle in /title (Choose Title dialog), and created `/titlecustom` command & custom_title_dialog handle. Fully compiled and tested.
- 2026-07-09 22:18 WIB — Antigravity: Added automatic AFK detection that triggers the `/sleep` command when a player is inactive for 30 seconds.
- 2026-07-09 21:42 WIB — Antigravity: Repositioned Chat Spammer settings to the bottom of the cheats menu dialog and adjusted interval input unit from milliseconds (ms) to seconds (s).
- 2026-07-09 21:28 WIB — Antigravity: Removed OnConsoleMessage calls from SaveDropItemMoreTimes to eliminate chat console spam and network lag during item collection.
- 2026-07-09 21:20 WIB — Antigravity: Removed break, place, and take item action delays from packet validation and increased network rate limit thresholds to prevent disconnects on fast spam.
- 2026-07-09 21:15 WIB — Antigravity: Added Chat Spammer feature in cheatmenu (allowing custom message & interval) and persisted settings in player database JSON.
- 2026-07-09 20:50 WIB — Antigravity: Made autofarm slots capped dynamically based on user's active far-punch equipment and slot level upgrades.
- 2026-07-09 20:46 WIB — Antigravity: Added a slot input field `Slots (1-10):` under the Auto-Farm settings in the `/cheats` menu to allow users to directly configure autofarm slots.
- 2026-07-09 19:54 WIB — Antigravity: Removed automatic reopening/refresh of the cheatsmenu dialog when clicking the "OK" (Update) button.
- 2026-07-09 19:42 WIB — Antigravity: Adjusted font size of item names in the `/newget` dialog to a medium scale by setting `text_scaling_string` to `Day 30 +9999999999`.
- 2026-07-09 19:39 WIB — Antigravity: Modified cheatsmenu callback in dialog_return.h to prevent the cheats dialog from reopening ONLY when autofarm is enabled via the item picker, while keeping the automatic refresh when clicking "OK".
- 2026-07-09 15:46 WIB — Antigravity: Modified Top Rich Leaderboard to refresh immediately on opening the dialog or tab by centralizing the code in DialogHandle::recalc_richest() and calling it in Event_Leaderboard and dialog_return.h.
- 2026-07-09 14:11 WIB — Antigravity: Updated choosemusic path in PacketHandler.h from audio/theme_lobby.ogg to audio/ogg/theme_lobby.ogg. Fully compiled and verified using MSBuild.
- 2026-07-09 05:35 WIB — Antigravity: Modified autocollect gems logic in WorldInfo.h (gems_ function) to split the single collected gem bubble into a stream of spammed/delayed bubbles (up to 15 bubbles) for better visual feedback during autocollect. Fully compiled and verified using MSBuild.
- 2026-07-09 04:22 WIB — Antigravity: Removed Shadow Farm feature entirely from the project. Deleted ShadowFarm.h & ShadowFarmImpl.h from both NopySource and GrowRabbid directories. Cleaned up BaseServer, PlayerInfo, WorldInfo, Commands, dialog_return, action, and NopySource tick/initialization hooks. Fully compiled and verified in Release mode.
- 2026-07-03 11:50 WIB — Kilo: Refactored Daily Reward system into 3-file structure (DailyReward/RewardConfig.h, PlayerDailyData.h, DailyRewardSystem.h). Fixed skip-missed-days bug: player now advances by DaysElapsed() instead of +1, missed days permanently lost (status = skipped). Added std::chrono-based calendar date validation, anti-clock-manipulation (gap capped 1000 days), RewardType enum (ITEM/GEMS/COINS/OPC), Status[30] array tracking (locked/claimed/skipped), TotalClaim + MissedDaysTotal stats. Updated DailyReward.h as backward-compat bridge. All 8 required functions implemented: LoadPlayerData, SavePlayerData, CanClaimToday, ClaimReward, SkipMissedReward, GetCurrentReward, ResetRewardCycle, BuildDialog.
- 2026-07-03 09:55 WIB — Kilo: Reverted lock info removal. Restored lock_info block (description + count + convert buttons) in Action.h info(), LOCK end_dlg branch, dialogMap["convert_star"] registration, and handle_converter_lock_dialog function in dialog_return.h.
- 2026-07-03 09:41 WIB — Kilo: Removed lock info special effect from item info popup. Deleted lock_info block (description + count + convert buttons) in Action.h info(), removed LOCK special case for end_dialog name (now always generic), deleted dialogMap["convert_star"] registration and dead handle_converter_lock_dialog function in dialog_return.h.
- 2026-07-02 15:45 WIB — Kilo: Rewrote item info dialog layout (Action.h info()). New layout: title with chi icon, description ($/yellow), stats (Extra Gems/Xp/Bonus), Extra Drops list with icons, splice info ($/yellow), Item Properties section. Removed undefined `a` variable. Restored convert_star handler for lock items.
- 2026-07-02 14:53 WIB — Kilo: Added lock description + count to item info popup (Action.h). Shows "A lock makes it so only you..." + "You have: X <lockname>" when item blockType is LOCK.
- 2026-07-02 14:47 WIB — Kilo: Deleted convert_star (converter lock) feature entirely. Removed handle_converter_lock_dialog, dialogMap entry, NPC phone converter (btn 11111), convert buttons from item info popup, and dead converter_lock_dialog in Commands.h.
- 2026-06-27 08:55 WIB — Claude: Auto-enable /hidestatus (hs=1) on new account registration. Set pl->hs=1 in RegisterFromWebAndAutoLogin (WorldInfo.h:25815) and dialog_return.h:33278 registration path, before save. hs is saved (save_["hs"]) and loaded, controls state_player bit 24.
- 2026-06-27 08:35 WIB — Claude: Added growmoji (wl) World Lock emoji (ā / U+0101, UTF-8 0xC4 0x81) next to player name in get_player_nick() so it shows in chat + nameplate. Touched both branches (d_name custom nick and default tankIDName).
- 2025-06-21 00:30 WIB — Claude: Implemented Daily Reward system with 30-day cycle, JSON config, /daily and /setdaily commands. -> .multibrain/context/2025-06-21-daily-reward-system.md
