# Daily Reward System

## Goal

Implement a complete Daily Reward system for the GTPS server with 30-day cycle, JSON-based config, and admin editable rewards.

## Summary

Implemented full Daily Reward system with:
- 30-day streak cycle (resets to Day 1 after Day 30)
- JSON config file (`database/json/dailyreward.json`) for easy reward editing
- `/daily` command for players to view and claim rewards
- `/setdaily` command for admins to edit rewards
- Dialog with title, description, 30-day reward list, and footer note
- Status: "Claim Now!" for today, "Claimed" for past days, "Day X (Y day(s) left)" for future
- Progress saved across sessions (uses `is_day`, `claimed_daily_today`, `daily_login_day` fields in Player struct)

## Changes

### New Files
- `GrowRabbid/Handle/DailyReward.h` — Complete Daily Reward system:
  - `DailyRewardItem` struct (item_id, amount)
  - `DailyRewardConfig` struct with load/save to JSON
  - `DailyReward_IsNewDay()` — checks if day changed since last claim
  - `DailyReward_BuildDialog()` — builds player-facing dialog
  - `DailyReward_Claim()` — processes reward claim
  - `DailyReward_BuildEditDialog()` — builds admin edit dialog
  - `DailyReward_BuildEditDayDialog()` — builds per-day edit dialog

### Modified Files
- `growrabbid.cpp` — Added `#include "GrowRabbid/Handle/DailyReward.h"`
- `GrowRabbid/Commands/Commands.h` — Added `DAILY_REWARD()` and `SET_REWARD()` methods to Commands class
- `GrowRabbid/Dialog/dialog_return.h`:
  - Added `#include "GrowRabbid/Handle/DailyReward.h"`
  - Added `dailyRewards()` dialog handler (handles "claim_daily_reward" button)
  - Added `setDaily()` dialog handler (handles admin edit: save text, reset, edit day)
  - Added `setDaily_edit()` dialog handler (handles per-day reward editing)
  - Registered `setDaily_edit` in dialogMap

### Existing Player Fields Used (already in PlayerInfo.h)
- `is_day` (int) — current day in the 30-day cycle (1-30)
- `claimed_daily_today` (bool) — whether player claimed today's reward
- `daily_login_day` (long long) — timestamp of last claim

### JSON Config Format (`database/json/dailyreward.json`)
```json
{
    "title": "Daily Rewards",
    "description": "Claim your daily reward every day!...",
    "footer_note": "Note: Streak resets if you miss a day...",
    "rewards": [
        {"item_id": 1796, "amount": 1},
        {"item_id": 1796, "amount": 2},
        ...
    ]
}
```

## Features
1. **30-day cycle** — After Day 30, automatically resets to Day 1
2. **Claimed status** — Past days show "Claimed", today shows "Claim Now!" or "Claimed" if already claimed
3. **Progress saved** — Uses timestamp comparison to detect day change; when new day detected and player already claimed, advances to next day
4. **JSON config** — Rewards fully configurable via JSON file, no code changes needed
5. **Admin panel** — `/setdaily` command lets admins edit title, description, footer note, and per-day rewards
6. **Struct-based** — Uses `DailyRewardItem` and `DailyRewardConfig` structs to avoid code duplication
7. **Dialog UI** — Shows title, description, 30-day list with item icons, claim button, and footer note

## Files
- `GrowRabbid/Handle/DailyReward.h` — New file, complete system
- `growrabbid.cpp` — Added include
- `GrowRabbid/Commands/Commands.h` — Added DAILY_REWARD() and SET_REWARD() methods
- `GrowRabbid/Dialog/dialog_return.h` — Added dialog handlers

## Verification
- Read back all modified sections to confirm correctness
- Verified include chain: growrabbid.cpp → Commands.h → DailyReward.h
- Verified dialogMap registrations: dailyReward, setDaily, setDaily_edit

## Next
- [ ] Compile and test in Visual Studio
- [ ] Test day transition logic (claim → wait 1 day → verify next day unlocks)
- [ ] Test Day 30 → Day 1 reset
- [ ] Consider adding streak break logic (if player misses a day, reset to Day 1)
