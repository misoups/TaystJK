# TaystJK
[![build](https://github.com/misoups/TaystJK/actions/workflows/build.yml/badge.svg)](https://github.com/misoups/TaystJK/actions/workflows/build.yml)

## License
[![License](https://img.shields.io/github/license/eternalcodes/EternalJK.svg)](https://github.com/misoups/TaystJK/blob/master/LICENSE.txt)
OpenJK is licensed under GPLv2 as free software. You are free to use, modify and redistribute OpenJK following the terms in LICENSE.txt.

## Maintainers
* [misoups](https://github.com/misoups)

## Upstream Repositories
This project is maintained against the following repositories:
* [OpenJK](https://github.com/JACoders/OpenJK)
* [jaPRO](https://github.com/videoP/jaPRO)
* [EternalJK](https://github.com/eternalcodes/EternalJK)
* [EternalJK-Vulkan](https://github.com/JKSunny/EternalJK)
* [TaystJK](https://github.com/taysta/TaystJK)

# OpenJK

OpenJK is a community effort to maintain and improve the game and engine powering Jedi Academy, while maintaining _full backwards compatibility_ with the existing games and mods.  
This fork focuses on the jaPRO integration and Client Engine modifications.

Our aims are to:

- Improve the stability of the engine by fixing bugs and improving performance.
- Support more hardware (x86_64, Arm, Apple Silicon) and software platforms (Linux, macOS)
- Provide a clean base from which new code modifications can be made.

## Notable changes from [taysta/TaystJK](https://github.com/taysta/TaystJK)

### Race Mode

- **Base JKA clients now enter race mode** — Removed `isJAPRO` guards from `ClientBegin`, `ClientSpawn`, and the per-frame `ClientThink_real` check that were stripping race mode from non-plugin clients. Base clients stay in race mode with timers and commands working; their movement style is locked to MV_JKA since they can't predict custom physics.
- **`/move` requires the jaPRO plugin** — Base clients get a clear message pointing to the download instead of having their style silently overridden every frame.
- **Checkpoint personal bests** — Checkpoint times are stored per (username, map, course, style) in the database. On crossing a checkpoint, shows a green/red delta vs your PB. Auto-saves if improved.
- **`/savepos` / `/respos`** — Saves and restores position, angles, and velocity. In competitive race mode acts like `/amtele` (resets timer); in practice mode teleports without touching the timer.
- **`/resetspawn`** — Clears telemark and saved spawn position, resets to map default spawn.
- **`/amtelemarkreset`** — Clears telemark without resetting the run timer.
- **Jetpack tracking** — Using jetpack thrust before the start timer fires invalidates the run. `/move` is blocked until `/resetspawn` or `/kill` if jetpack was activated this life.
- **Noclip invalidation** — `/noclip` sets a flag that blocks the start trigger from firing until `/amtele` or `/kill`.
- **No-reactivate start trigger** — Start trigger cannot fire twice in the same run, preventing pre-speed building by repeatedly brushing the trigger (matches Q3 Defrag behaviour).
- **Soft death in race mode** — Dying teleports you to your telemark (or map spawn if none set) instead of a full respawn sequence. `/kill` in race mode uses the same path.
- **Auto-spec idle players** — `g_autoSpec` moves idle players to spectator after a configurable timeout, with per-second warnings in the final 10 seconds. Never specs a player mid-race, and skips auto-spec if only one non-spectator is on the server.

### Map System

- **Q3 Defrag timer format support** — `target_startTimer`, `target_stopTimer`, and `target_checkpoint` entities are recognised and converted at map load via `G_ConvertQ3DefragTimers()`.
- **Twi mod timer format support** — `Twi_timer` brush entities are also converted at map load.
- **Defrag arc jumppad support** (`trigger_push_velocity`) — Preserves player XY velocity and only applies Z, matching defrag arc pad behaviour. Standard jumppads replace velocity entirely as before.
- **Per-map haste config** — Server admins can place `mapconfigs/<mapname>.cfg` with `seta g_mapHaste 1` to enable haste for specific maps. Server-configured haste is always valid for race submissions; manual haste pickups are not.
- **`/maplist` rewrite** — Enumerates all `.bsp` files on the server (not arena-file dependent), sorts alphabetically, excludes all stock base-JKA maps. Numbers are stable and shared with `/callvote mapnum`.
- **`/callvote randommap`** — Votes for a random map from the server's full BSP list, excluding base JKA maps.
- **`/callvote mapnum <n>`** — Votes for a map by its index from `/maplist`.
- **Map change lockout** — `randommap` and `mapnum` votes are blocked for 10 minutes after a map load, same as the existing `map` vote lockout.

### New Movement Styles

- **MV_QUAJK (style 19)** added — Available via `/move quajk`. Does not receive haste speed bonus.
- **MV_SICKO (style 20)** added — Available via `/move sicko`. Ported from MVSDK/EternalJK2: same Q2-style ramp physics and rampjumps as QuaJK, but with dynamic air acceleration (accel rate scales with missing speed, capped at 200) instead of QuaJK's CPM-blend, and reduced water friction. Does not receive haste speed bonus.

### Cross-Server Communication (Linux)

- **POSIX FIFO-based IPC** (`g_crossserver.c`) — Same-machine servers can share a cluster via `g_crossServerPorts` cvar. Events (connect, join, quit, chat, run completion) are broadcast to peer servers and displayed with a colour-coded `[server-name]` prefix. Compiles out on Windows builds.
- **`/say_cross <message>`** — Sends a chat message to all servers in the cluster.
- **PB broadcast** — Global personal bests are announced across the cluster with badges: `(WR)`, `(SR+PB)`, `(SR)`, `(PB)`.

### Combat

- **Removed saber block RNG** — Deleted the `g_reducesaberblock` random check that could cause saber blocks to fail unpredictably.

### Client / Engine

- **`cl_reconnectArgs` converted from cvar to static buffer** — Removes cvar system overhead and potential memory issues with reconnect state.
- **Removed obsolete feature flags** from `bg_public.h`: `TAYSTJK_INFO_FLIPKICK`, `TAYSTJK_INFO_GRAPPLE`, `TAYSTJK_INFO_FIXROLL_1/2/3`.

### Build & CI

- **Release workflow robustness** — Fixed `gh api` jq query with `// empty` for safe null handling; added commit validation before diffing.

---

## For players

To install TaystJK, you will first need Jedi Academy installed. If you don't already own the game you can buy it from online stores such as [Steam](https://store.steampowered.com/app/6020/), [Amazon](https://www.amazon.com/Star-Wars-Jedi-Knight-Academy-Pc/dp/B0000A2MCN) or [GOG](https://www.gog.com/game/star_wars_jedi_knight_jedi_academy).

Download the [latest build](https://github.com/misoups/TaystJK/releases/tag/latest) for your operating system.

Installing and running TaystJK:

1. Extract the contents of the file into the Jedi Academy `GameData/` folder. For Steam users, this will be in `<Steam Folder>/steamapps/common/Jedi Academy/GameData/`.
2. Run `taystjk.x86.exe` (Windows), `taystjk.i386` (Linux 32-bit), `taystjk.x86_64` (Linux 64-bit) or the `TaystJK` app bundle (macOS), depending on your operating system.

### Linux Instructions

If you do not have an existing JKA installation and need to download the base game:

1. Download and Install SteamCMD [SteamCMD](https://developer.valvesoftware.com/wiki/SteamCMD#Linux).
2. Set the download path using steamCMD: `force_install_dir /path/to/install/jka/`
3. Using SteamCMD Set the platform to windows to download any windows game on steam. `@sSteamCmdForcePlatformType "windows"`
4. Using SteamCMD download the game, `app_update 6020`.

Extract the contents of the file into the Jedi Academy `GameData/` folder. For Steam users, this will be in `<Steam Folder>/steamapps/common/Jedi Academy/GameData/`.

### macOS Instructions

If you have the Mac App Store Version of Jedi Academy, follow these steps to get TaystJK running under macOS:

1. Install [Homebrew](https://brew.sh/) if you don't have it.
2. Open the Terminal app, and enter the command `brew install sdl2`.
3. Extract the contents of the TaystJK DMG into the game directory `/Applications/Star Wars Jedi Knight: Jedi Academy.app/Contents/`
4. Run `TaystJK.app`
5. Savegames, Config Files and Log Files are stored in `/Users/$USER/Library/Application Support/TaystJK/`

## For Developers

### Building TaystJK

- [Compilation guide](https://github.com/JACoders/OpenJK/wiki/Compilation-guide)
- [Debugging guide](https://github.com/JACoders/OpenJK/wiki/Debugging)

### Contributing to TaystJK

- [Fork](https://github.com/misoups/TaystJK/fork) the project on GitHub
- Create a new branch and make your changes
- Send a [pull request](https://help.github.com/articles/creating-a-pull-request) to upstream (misoups/TaystJK)

### Using TaystJK as a base for a new mod

- [Fork](https://github.com/misoups/TaystJK/fork) the project on GitHub
- Change the `GAMEVERSION` define in [codemp/game/g_local.h](https://github.com/misoups/TaystJK/blob/master/codemp/game/g_local.h) from "OpenJK" to your project name
- If you make a nice change, please consider back-porting to upstream via pull request as described above. This is so everyone benefits without having to reinvent the wheel for every project.
