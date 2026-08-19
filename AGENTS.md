# Agent Instructions — GtpsProject (Nopy)

## Multi Brain (MANDATORY)

- Read `.multibrain/session.md` before starting work.
- Use `.multibrain/session.md` as the master index only.
- Open only the `.multibrain/indexes/*.md` bucket files that match the current task.
- Open `.multibrain/context/*.md` only when the selected bucket points to deeper context that matters.
- After meaningful work, update the relevant named bucket and refresh the master index if needed.

## Project Overview

- **Type:** C++ Growtopia Private Server (GTPS)
- **Main source file:** `NopySource.cpp` (~3100+ lines)
- **Headers:** `NopySource/Handle/` contains `BaseServer.h`, `PacketHandler.h`, `PlayerInfo.h`, `WorldInfo.h`, `ItemDefination.h`
- **Utils:** `NopySource/Utils/Utils.hpp`
- **Build:** Visual Studio project (`NopySource.vcxproj`, `NopySource.sln`)

## Environment Notes

- Bash shell is NOT available in this environment (no Git Bash)
- PowerShell scripts can be written to disk but cannot be executed via the bash tool
- The `edit` tool requires exact whitespace matching — this file uses tabs and CRLF (`\r\n`) line endings
