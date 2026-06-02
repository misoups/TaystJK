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

## Notable changes from upstream (taysta/TaystJK)

### Race mode
- **Q3 Defrag & Twi timer format support** — servers now recognise `target_startTimer` / `target_stopTimer` / `target_checkpoint` point entities (Q3 defrag format) and `Twi_timer` brush entities in addition to the existing `df_trigger_*` classnames, bringing compatibility with a wider range of race maps
- **`/amtelemark` locked mid-run** — setting a telemark is blocked while the race timer is active, or after `/noclip` / `/respos` use, preventing mark abuse
- **`/respos` (save/restore position)** — `/savepos` snapshots origin, angles and velocity; `/respos` restores it (resets timer in competitive mode, preserves velocity for practice mode)
- **Cleaner invalidation message** — the race-state-invalidated center-print now reads `^1Error: your race state is invalidated. ^7Please /kill or /amtele to reset.`

### Server quality-of-life
- **Auto-spec solo exemption** — `g_autoSpec` no longer moves a player to spectator when they are the only non-spectator on the server, regardless of how many spectators are watching
- **Console map/time display** — opening the console shows the current map filename, client version with build date, and local clock (matching EternalJK2MV style)

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
