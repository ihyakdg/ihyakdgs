# Memory Leak: pMov not freed on unhandled packet cooldown disconnect

## Goal

Fix a memory leak in `growrabbid.cpp` where `PlayerMoving* pMov` (allocated via `new` inside `unpackPlayerMoving()`) is not freed when the server disconnects a peer during the unhandled-packet rate-limit cooldown.

## Summary

**FIXED.** Added `if (pMov != NULL) delete pMov;` before the `break;` in the unhandled-packet cooldown disconnect path. All `break` paths between `unpackPlayerMoving()` and the bottom-of-case `delete pMov` have been audited — no other leaks found.

## The Leak (before fix)

**File:** `growrabbid.cpp` (~line 3057, inside the `NET_MESSAGE_GAME_PACKET` case in the ENet receive event handler)

**Flow:**
1. `PlayerMoving* pMov = unpackPlayerMoving(tankUpdatePacket);` allocates with `new PlayerMoving`
2. If the packet type is NOT in `eventMap` (unhandled), a cooldown check runs:
   ```cpp
   if (duration_cast<seconds>(now - pInfo(peer)->lastPacketTime) < cooldownDuration) {
       Peer_Disconnect(peer, 0);
       break;  // ← pMov LEAKED — delete at bottom of case is never reached
   }
   ```
3. `if (pMov != NULL) delete pMov;` at the bottom of the case is skipped by the `break`

## The Fix (applied)

Added `if (pMov != NULL) delete pMov;` before the `break;` in the unhandled-packet cooldown block:

```cpp
if (duration_cast<seconds>(now - pInfo(peer)->lastPacketTime) < cooldownDuration) {
    Peer_Disconnect(peer, 0);
    if (pMov != NULL) delete pMov;   // ← ADDED
    break;
}
```

## Break Path Audit

All `break` statements after `pMov` allocation in the `NET_MESSAGE_GAME_PACKET` case:

| Path | Location | Status |
|------|----------|--------|
| `tankIDName.empty()` early exit | ~line 3016 | ✅ Before `pMov` allocation — no leak |
| `tankUpdatePacket == nullptr` early exit | ~line 3018 | ✅ Before `pMov` allocation — no leak |
| `pps24 >= todc` rate-limit disconnect | ~line 3030 | ✅ Already had `delete pMov` (previously fixed with `// FIX MEMLEAK` comment) |
| Unhandled-packet cooldown disconnect | ~line 3057 | ✅ **Fixed in this session** |
| Normal exit (bottom of case) | ~line 3066 | ✅ Already had `delete pMov` |

## Files

- `growrabbid.cpp` — main server file, contains the leak and the fix (line ~3060)
- `GrowRabbid/Handle/PacketHandler.h` — defines `unpackPlayerMoving()` (returns `new PlayerMoving`) and `packPlayerMoving()` (returns `new BYTE[]`)

## Key Memory Management Patterns (for future reference)

- `unpackPlayerMoving()` → returns `new PlayerMoving` → caller must `delete`
- `packPlayerMoving()` → returns `new BYTE[]` → caller must `delete[]`
- `SendPacketRaw112` / `SendPacketRaw1` → free `packetData` internally
- `send_raw` → does NOT free buffers (caller responsibility)
- `Peer_Disconnect` = `enet_peer_disconnect_later` (from `BaseServer.h`)

## Verification

- Read back the edited section (lines 3049-3073) — confirmed `if (pMov != NULL) delete pMov;` is present before `break;` in the cooldown path
- Audited all 5 `break` paths after `pMov` allocation — no other leaks found

## Next

- [ ] Consider refactoring to use `std::unique_ptr<PlayerMoving>` or a `scope_guard` to prevent future leaks automatically
- [ ] Clean up temp files created during debugging: `fix_memleak.ps1`, `check_tabs.ps1`
- [ ] Compile and test the project in Visual Studio
