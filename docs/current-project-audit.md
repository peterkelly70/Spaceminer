# Space Miner — Current Project Audit

Date: 2026-07-08
Scope: static inspection of `/mnt/Tome/Godot/SpaceMiner` plus one headless boot (`godot --headless --path . --quit`) and a `--check-only` boot that was killed after confirming no immediate parse errors.

---

## Key tension / open question for the user

**This project is not a hand-authored JSW-style single-screen platformer, and it is not built on a clean platformer codebase either. It is two (arguably three) different games sharing one project.**

1. **Procedural rooms, not hand-authored rooms.** The spec (`doc/jsw_platformer.txt`) explicitly says "add procedural rooms before hand-authored rooms work" is an ABSOLUTE DO NOT, and asks for scenes named `Room_001_LandingBayAlpha.tscn` etc. The actual project generates a 56-room "asteroid campaign" at runtime from a seed via `scripts/tools/AsteroidCampaignGenerator.gd` (1140 lines) + `scripts/tools/RoomLayoutGenerator.gd` (1264 lines), instantiating a single generic scene, `scenes/levels/GeneratedAsteroidRoom.tscn`, and configuring it per-room from generated JSON (`RunManager.get_room_path()` in `scripts/autoload/RunManager.gd:232`). There is exactly one hand-authored gameplay room scaffold, `scripts/tools/build_landing_bay_alpha.gd`, and no `Room_001_LandingBayAlpha.tscn` exists anywhere.

2. **The player controller has drifted into a metroidvania-equipment platformer, not a JSW fixed-arc platformer.** The spec is explicit: fixed jump arc, no mid-air steering, instant stop on landing, and no additional equipment beyond the GDD's 5 power-ups. The actual controller in play (`scripts/prototype/player.gd`, `class_name PrototypePlayer`, 857 lines) has: air control (`air_control_accel`, used in `_apply_horizontal_move` for airborne steering — a direct violation of "no mid-air steering"), a jetpack with WASD-thrust vectoring, a grappling hook with aim-arc and physics pull, a laser pistol, magnetic boots with surface-normal reorientation, a visibility cloak with battery drain, ladder climbing, and a 6-slot equipment hotbar (`equipment_1`..`equipment_6` input actions in `project.godot`). `grapple`, `laser_fire`, `boots_toggle`, `equipment_1..6`, and `map_toggle` input actions exist in `project.godot` and are not in the base spec at all.

3. **There is a second, entirely unrelated player controller and a second, entirely unrelated game's worth of autoload state still live in the project.** `scripts/controllers/Player.gd` is a twin-stick space-shooter controller (mouse-aim rotation, dash, regenerating shield, bullet firing) used by `scenes/units/Player.tscn` / `scenes/states/GameWorld.tscn`, unrelated to the platformer. The `Game_Manager` autoload (`scripts/autoload/GameManager.gd`) defines a turn-based kingdom-builder game state machine (`GamePhase.ASSIGN_WORKERS/BUILD_UPGRADE/BUY_LAND/EQUIP_SOLDIERS/PRODUCTION/EVENTS/END_TURN`, seasons, gold/gems/mana/crown/lore/runes/warrior resources, building/worker/land/military dictionaries) and is loaded and initialized on every single boot (`GameManager.gd:270,286` prints "Loaded 14 building definitions" / "Loaded 12 worker definitions" every run). `run.log` at the project root shows an even older boot of what looks like the same base template under a different name — "Welcome to Groot!", `groot_theme.tres`, `GameSetupView`, and music from "Karl Casey - Dark Synthwave Collection" playing a track called "Subterranean Terror" (`ironhaven` notification theme referenced too). This strongly suggests Space Miner was built by forking an existing city-builder/strategy game template ("Groot"/"Ironhaven") and layering the platformer on top, rather than starting clean. None of this is gameplay-blocking today (dead code paths, apparently unreachable from the real boot flow) but it inflates the codebase, autoload list, and boot cost, and risks confusing future maintenance.

The user needs to decide, before more work proceeds:
- Commit to procedural room generation (finish/polish the generator, drop the hand-authored `Room_001_...` naming convention from the spec) **or** pivot to hand-authored rooms as the spec demands.
- Commit to the metroidvania-equipment direction (jetpack/grapple/laser/magnetic-boots/cloak, air control) **or** strip the controller back to the spec's fixed-arc, no-air-control JSW feel.
- Decide whether to delete the dead twin-stick-shooter and kingdom-builder code (`scripts/controllers/Player.gd`, `scenes/units/*`, `scenes/states/GameWorld.tscn`, `GameManager.gd`'s builder-game contents, `groot_theme.tres`/`ironhaven` references) or leave it as inert cruft.

---

## Godot version confirmed

- Installed engine binary: `godot` on PATH → `godot --version` reports **`4.6.stable.official.89cea1439`**.
- `project.godot` line 19: `config/features=PackedStringArray("4.6", "Forward Plus")` — project is saved as Godot **4.6**, not 4.4 as the spec assumes.
- Renderer is downgraded to compatibility mode: `project.godot` lines 322-323, `renderer/rendering_method="gl_compatibility"`.
- The project **boots successfully headless** (`godot --headless --path . --quit`) with no script parse errors and no `SCRIPT ERROR:` lines in ~520 lines of boot log. Only issues at exit are benign resource-cleanup warnings (`WARNING: ObjectDB instances leaked at exit`, `ERROR: 2 resources still in use at exit`) typical of `--quit` cutting off async audio streams mid-playback, not gameplay bugs.
- A plain `--check-only` boot does not actually exit on its own within 60s (it runs the full game loop rather than doing a static-only check); use `--quit` for a bounded headless smoke test instead.

## Scenes found

Two clusters, `scenes/` top-level plus subfolders:

- **Boot / meta-flow**: `scenes/MainView.tscn` (main scene, `run/main_scene` in `project.godot:18`), `scenes/states/Splash.tscn`, `MenuView.tscn`, `SettingsView.tscn`, `CreditsView.tscn`, `MainGameView.tscn`, `GameWorld.tscn` (dead — twin-stick shooter, see tension section).
- **Cutscene**: `scenes/cutscene/CutscenePlayer.tscn`, `IntroNarrative.tscn`.
- **UI**: `scenes/ui/HUD.tscn` (dead — belongs to the twin-stick shooter, see below), `Dial.tscn`, `EquipmentStatusCard.tscn`, `NotificationPopup(.Iron).tscn`, `OdometerLabel.tscn`, plus `scenes/ui/popups/*` (BasePopup, GameOverPopup, GenericConfirmPopup, MainMenuPopup, MapPopup, MessagePopup, MusicSelectionPopup, PausePopup, QuitConfirmationDialog/Popup, TurnStartPopup).
- **Gameplay ("prototype") pieces**, `scenes/prototype/`: `Player.tscn` (the real platformer character, uses `scripts/prototype/player.gd`), `PrototypeRoom.tscn`, `AcidBath.tscn`, `AirCanister.tscn`, `BatteryPack.tscn`, `ConveyorBelt.tscn`, `DoorZone.tscn`, `DwarfMiner.tscn`, `EquipmentPickup.tscn`, `FirePipe.tscn`, `FuelCell.tscn`, `HazardZone.tscn`, `MovingPlatform.tscn`, `MovingSpike.tscn`, `OreFragment.tscn`, `ResupplyStation.tscn`, `SlidingWall.tscn`, `Teleporter.tscn`.
- **Procedural room shell**: `scenes/levels/GeneratedAsteroidRoom.tscn` — the single scene every generated room instantiates (see `MainGameController.gd:150`, `GENERATED_ROOM_SCENE`).
- **Units (dead — shooter game)**: `scenes/units/Bullet.tscn`, `Dalek.tscn`, `Enemy.tscn`, `MovingLift.tscn`, `PatrolDrone.tscn`, `Player.tscn`, `Ufo.tscn`.
- **Debug/test**: `scenes/debug/PaletteSheet.tscn`, `RoomEditor.tscn`, `RetroSpeechTest.tscn`; `scenes/tests/buttontest.tscn`, `cutscene_test.tscn`, `Maptest.tscn`.

Naming convention: **not** `Room_NNN_Name.tscn` as the spec suggests — there is no such file anywhere in the project. The convention actually used is one generic room-shell scene (`GeneratedAsteroidRoom.tscn`) plus per-room JSON layout data written by the generator to `user://saves/spaceminer/campaigns/` at runtime (not checked into the repo — layouts do not exist until a run is started).

## Scripts found

`scripts/` (grouped by subfolder):

- **`autoload/`** (13 files, one per registered autoload plus 4 orphans — see Autoloads section): `AppState.gd`, `AudioManager.gd`, `ControlsManager.gd`, `DataManager.gd` *(orphan — not registered as autoload, not referenced anywhere in `scripts/`/`scenes/`)*, `GameManager.gd`, `InputMap.gd` *(orphan, unreferenced)*, `NotificationManager.gd`, `PopupManager.gd`, `RunManager.gd`, `ScreenManager.gd`, `SettingsManager.gd`, `StateManager.gd`, `TextUtils.gd`, `ToolTipManager.gd` + `TooltipManager.gd` *(two similarly-named files; only `ToolTipManager.gd` is registered)*, `UIManager.gd` *(orphan, unreferenced)*.
- **`audio/`**: RetroAudioUtil, RetroSpeech, SamBridge, SamParser, SamReciter, SamRenderer, SamVoice — a SAM (Software Automatic Mouth) speech-synthesis subsystem, unrelated to the JSW spec but functioning (has its own `doc/RetroSpeech_README.md`).
- **`class/`**: `CutscenePlayer.gd`, `StateAwareController.gd` (base class most controllers extend to receive `AppState` pings).
- **`controllers/`**: `Bullet.gd`, `CreditsController.gd`, `Enemy.gd` (shooter-game enemy), `GameStateConnector.gd`, `GameWorldController.gd` (shooter game), `MainGameController.gd` (**the real gameplay driver** — room load/unload, HUD wiring, pause), `MenuController.gd`, `SettingsController.gd`, `TestButton.gd`, `Player.gd` (**dead — twin-stick shooter controller**, not the platformer), plus `controllers/phases/phas01–05.gd` (unclear purpose, likely leftover from the builder-game phase system) and `controllers/rooms/`: `GeneratedAsteroidRoomController.gd`, `LandingBayAlphaController.gd`, `LevelBuilder.gd`, `MovingLift.gd`, `PatrolDrone.gd`.
- **`debug/`**: `RoomEditor.gd`.
- **`helpers/`**: `setup_audio_buses.gd`.
- **`main.gd`** — script on `MainView.tscn`, orchestrates Splash → Menu → gameplay handoff.
- **`managers/`**: `MusicManager.gd`.
- **`models/`**: `GameModel.gd`, `LandingBayAlphaModel.gd`.
- **`prototype/`** — **this is where the actual platformer lives**: `player.gd` (the real controller, see tension section), `acid_bath.gd`, `air_canister.gd`, `battery_pack.gd`, `conveyor_belt.gd`, `dalek_enemy.gd`, `door_zone.gd`, `dwarf_miner.gd`, `equipment_pickup.gd`, `fire_pipe.gd`, `fuel_cell.gd`, `hazard_zone.gd`, `ladder.gd`, `moving_platform.gd`, `moving_spike.gd`, `ore_fragment.gd`, `prototype_room.gd`, `resupply_station.gd`, `sliding_wall.gd`, `stamina_manager.gd`, `teleporter.gd`, `ufo_enemy.gd`.
- **`resources/`**: `ColorPalette.gd` (custom `Resource` class the palette `.tres` uses), `MusicCollection.gd`, `MusicTrack.gd`.
- **`state/`**: `enemy_states.gd`, `game_states.gd`, `player_states.gd`.
- **`tools/`**: `AsteroidCampaignGenerator.gd` (1140 lines — the procedural campaign/room generator), `RoomLayoutGenerator.gd` (1264 lines — per-room tile/hazard/pickup layout), `RoomNameGenerator.gd`, `LevelSolvabilityValidator.gd` (a `RefCounted` solvability checker — good sign, means someone thought about the "no softlocks" rule), `build_landing_bay_alpha.gd` (procedural builder for a hand-tuned Landing Bay Alpha, referencing `scenes/prototype/Player.tscn`), `space_miner_palette_sheet.gd`, `generate_space_miner_sprite_sheets.py`.
- **`ui/`**: `AnimatedLifeIcon.gd`, `ControlsTab.gd`, `EquipmentStatusCard.gd`, `HUD.gd` (**dead — belongs to the shooter game's `shield_bar`/`score_label` HUD**, not the platformer HUD), `NeonOreIcon.gd`, `NotificationPopup(Iron).gd`, `Starfield.gd`, `ui/popups/*.gd` (BasePopup, GameOverPopup, GenericConfirmPopup, MainMenuPopup, MapGraph, MapPopup, MusicSelectionPopup, PausePopup, QuitConfirmationDialog).
- **`views/`**: `intro_narrative_controller.gd`, `sidebar.gd`.

The real platformer HUD is not `scenes/ui/HUD.tscn`/`HUD.gd` — it is wired directly inline inside `scripts/controllers/MainGameController.gd` (node paths like `$Layout/StatusArea/StatusPanel/.../OreValue`, `.../FuelIcon`, `.../Fuel/ProgressBar`, an `air_bar` for oxygen, `score_label`), reading node paths straight out of `scenes/states/MainGameView.tscn`.

## Autoloads found

From `project.godot` `[autoload]` (lines 26-40):

| Autoload name | Script | Exists? |
|---|---|---|
| `AppState` | `res://scripts/autoload/AppState.gd` | Yes |
| `State_Manager` | `res://scripts/autoload/StateManager.gd` | Yes |
| `Audio_Manager` | `res://scripts/autoload/AudioManager.gd` | Yes |
| `Notification_Manager` | `res://scripts/autoload/NotificationManager.gd` | Yes |
| `Popup_Manager` | `res://scripts/autoload/PopupManager.gd` | Yes |
| `Settings_Manager` | `res://scripts/autoload/SettingsManager.gd` | Yes |
| `Text_Utils` | `res://scripts/autoload/TextUtils.gd` | Yes |
| `Screen_Manager` | `res://scripts/autoload/ScreenManager.gd` | Yes |
| `Game_Manager` | `res://scripts/autoload/GameManager.gd` | Yes, but its contents are the leftover kingdom-builder game state machine (see tension section) — not used by the platformer gameplay loop as far as static references show |
| `RunManager` | `res://scripts/autoload/RunManager.gd` | Yes — **this is the real platformer save/campaign/equipment/oxygen/fuel/lives state**, actively used by `MainGameController.gd` and `player.gd` |
| `TooltipManager` | `res://scripts/autoload/ToolTipManager.gd` | Yes (note casing: registered path is `ToolTipManager.gd`, but a second, unregistered `TooltipManager.gd` also exists in the same folder — confusing but not currently broken) |
| `SettingsManager` | `res://scripts/autoload/SettingsManager.gd` | Yes — **registered twice** under two different global names (`Settings_Manager` at line 33 and `SettingsManager` at line 39), both pointing at the same script file. Two singleton instances of the same class exist simultaneously at runtime. |
| `ControlsManager` | `res://scripts/autoload/ControlsManager.gd` | Yes |

Orphaned autoload-folder scripts that are **not** registered in `project.godot` and are not referenced anywhere else in `scripts/`/`scenes/` by name: `DataManager.gd`, `InputMap.gd`, `UIManager.gd`. A fourth stray file, `GameState.gd.uid`, has **no matching `GameState.gd`** — an orphaned `.uid` file with no source (harmless, but a sign of a script that was deleted without cleaning up its import metadata).

Headless boot log confirms all 13 registered autoloads initialize without error (`Initialized`/`Initialized successfully` prints from each), and `Game_Manager` prints `Loaded 14 building definitions`, `Loaded 12 worker definitions`, `Loaded 4 season definitions`, `Loaded 10 resource definitions`, `All game data loaded successfully!` on every boot — confirming the kingdom-builder data model actually loads and runs on startup even though (per static reference search) nothing in the platformer gameplay path reads from it.

## Broken paths

Sampled ~114 unique `res://*.{tscn,gd,tres}` path references across `scripts/` and `scenes/` and checked each against the filesystem. 7 were missing:

- `res://addons/simple_cutscene_editor/cutscene_player.tscn` — referenced but the `addons/simple_cutscene_editor/` folder does not exist in this checkout (note: current `git status` shows the `addons/AsepriteWizard` plugin has been deleted from disk but not yet committed either — the `addons/` folder is mid-change).
- `res://assets/themes/ironhaven_theme.tres` — referenced (leftover from the "Ironhaven"/builder-game template mentioned above), file absent.
- `res://scenes/cutscene_test.gd` — referenced from somewhere in `scenes/tests`/`scenes/cutscene`; actual file is `scenes/tests/cutscene_test.tscn`'s script, path mismatch.
- `res://scenes/levels/LandingBayAlpha.tscn` — **this is the one most spec-relevant miss**: something still expects a hand-authored `LandingBayAlpha.tscn` under `scenes/levels/`, but only `GeneratedAsteroidRoom.tscn` exists there now. Confirms the project is mid-migration from hand-authored to procedural rooms.
- `res://scripts/classes/CutscenePlayer.gd` — real file is at `res://scripts/class/CutscenePlayer.gd` (folder is `class`, not `classes`) — likely a simple typo/broken reference.
- `res://scripts/ui/ButtonHoverEffect.gd` — referenced, not present.
- `res://scripts/ui/TurnStartPopup.gd` — `scenes/ui/popups/TurnStartPopup.tscn` exists but its script does not; this is leftover builder-game UI.

This was a targeted grep-based sample, not exhaustive — resource `uid://` references (used heavily in `.tscn` files for engine-managed resource identity) were not individually resolved; the headless `--quit` boot log did not surface any `ERROR: Cannot open file` for these during the boot path actually exercised (Splash → Menu), which only means the broken paths above are not on that particular code path, not that they're safe.

## Missing assets

- `res://assets/palettes/space_miner_palette.tres` **exists** (`assets/palettes/space_miner_palette.tres`), but its contents do not match the Art Bible / spec palette. Full file:
  ```
  [gd_resource type="ColorPalette" format=3 uid="uid://g5xgy6yl5tqy"]
  [resource]
  colors = PackedColorArray(0.8, 0.6, 0.2, 1, 0.011764706, 0.9764706, 0.9843137, 1, 13.117, 0, 0, 1)
  ```
  This is only **3 colors** (a muddy orange, CPC cyan, and a broken value — `13.117, 0, 0, 1` is a red channel over 13x normal range, almost certainly a leftover HDR/bloom color accidentally saved, not a valid `#RRGGBB` intent). The spec calls for a 15-color palette (Void Black, Deep Space Blue, Panel Charcoal, Metal Grey, Bright White, CPC Cyan, Deep Blue, Miner Yellow, Hazard Orange, Alarm Red, Alien Green, Toxic Lime, Power Purple, Pickup Magenta, Chroma Magenta). **This resource needs to be rebuilt from the GDD palette list.**
  - Instrumentation exists to *build* one correctly: `scripts/tools/space_miner_palette_sheet.gd` and `scenes/debug/PaletteSheet.tscn`.
- `ironhaven_theme.tres` referenced but missing (see Broken paths).
- Two theme files load fine and are the ones actually in active use: `assets/themes/retro_theme.tres` (used by `MainGameView.tscn`), `assets/themes/groot_theme.tres` and `assets/themes/parchment_theme.tres` (both leftover-template themes, still referenced in `project.godot`'s `[theme_editor] recent_themes` and by the notification system for the `"Welcome to Groot!"` toast seen in `run.log`).
- Did not do an exhaustive audit of `assets/tiles/`, `assets/audio/`, `assets/sprites/` contents vs. references — spot checks of tile references in `MainGameView.tscn` (`tile_0400.png`, `tile_0401.png`) and audio files logged during boot (button.mp3, jump.wav, retro_coin.wav, etc.) all resolved successfully in the `--quit` boot log with no "Loaded SFX: ... FAILED" style errors.

## Current playable state

- **The project does boot to a menu.** Headless run confirms `Splash` → (after `hide_cutscenes` check) → `MenuView` becomes visible with no console errors up to that point.
- **There is a real state machine** (`AppState.State`: SPLASH, GAMESETUP, MAIN_MENU, SETTINGS, CREDITS, PLAYING, PAUSE, QUIT) driven by `StateManager.gd`, and `main.gd` wires ESC/pause/map-toggle input against it.
- **There is a real save/run system**: `RunManager.gd` generates a seeded 56-room campaign, persists it as JSON under `user://saves/spaceminer/`, tracks lives (`STARTING_LIVES := 10`), score, ore, oxygen%, fuel%, battery%, equipment, and security cards, and supports checkpointing (`save_checkpoint`).
- **Room transitions have real plumbing**: `MainGameController._load_room()` / `_on_room_change_requested()` / `_load_generated_room()` load a fresh `GeneratedAsteroidRoom.tscn` instance per room id, using `RunManager.get_room_path()` to find that room's generated JSON layout, and there's a `_room_transition_locked` guard against double-transitions.
- **The player controller (`scripts/prototype/player.gd`) is a large, working, but non-spec-compliant CharacterBody2D**: it has gravity, jump, air control, jetpack, grapple, laser, magnetic boots, ladder climbing, one-way-platform drop-through, and hazard-hit handling (`handle_hazard_hit`) all implemented — this is much more built-out than "broken," it's just built to a different design than the spec asks for (see tension section: air control directly contradicts "no mid-air steering").
- **Whether an actual playable route from Landing Bay Alpha to an ending exists could not be confirmed by static inspection** — room content is generated at runtime from a seed and not checked into the repo, so there is no way to inspect "the" campaign without actually running a session and playing it (out of scope for this static/headless audit). The `LevelSolvabilityValidator.gd` tool's existence suggests someone attempted to guarantee solvability programmatically, which is a good sign, but its actual invocation/coverage was not traced in this pass.
- **The real HUD** (inline in `MainGameController.gd` + `MainGameView.tscn`) shows room name (`overlay_label`/`MissionLabel`), ore count, oxygen (`air_bar`), fuel bar (jetpack), battery bar (magnetic boots/cloak), and score — this covers most of the spec's HUD requirements. It does not show keys/security-card status directly in the always-visible bar (that's routed through `EquipmentStatusCard.tscn`/`.gd` instead, not verified in this pass whether it's currently visible during play).
- **Two dead/unrelated games' worth of code still execute on every boot** (twin-stick shooter's `Player.gd`/`GameWorld.tscn` is present but not instantiated by the Splash→Menu→MainGameView flow that was traced; the kingdom-builder `Game_Manager` autoload *does* run its full data-loading routine on every single boot regardless of which game mode the player ends up in).
- **Net assessment**: the game has clearly moved past "broken prototype" — it boots, has a menu, a seeded procedural campaign generator, a working (if elaborate) player controller, room transition plumbing, save/load, and a working HUD wired to real game state. But it has drifted from "JSW-style single-screen precision platformer with hand-authored rooms" toward "seeded roguelite-ish metroidvania with procedural rooms and an equipment loadout," which is a different game than the spec describes, layered on top of remnants of a third, unrelated game (kingdom builder / "Groot").

## Current blocker list

Ordered roughly by how much they block "the game as the spec describes it" rather than "the game as it currently exists":

1. **No hand-authored rooms exist at all** — `Room_001_LandingBayAlpha.tscn` and siblings the spec explicitly asks for do not exist; `res://scenes/levels/LandingBayAlpha.tscn` is referenced by at least one script but missing on disk. Before any spec-compliant "minimum 10-15 polished rooms" milestone can be hit, a decision is needed: build those rooms by hand, or accept the procedural generator as the intended architecture and update the plan instead.
2. **Player controller violates the spec's core movement rule** — mid-air horizontal steering (`air_control_accel`) is implemented and active; the spec's jump model (commit velocity at jump, instant stop on landing, no air steering) is not what's running today. Reconciling this either means stripping steering out of `player.gd`, or the user formally accepting the current feel as the new target.
3. **`space_miner_palette.tres` is broken/incomplete** — only 3 colors, one of which is out-of-range, versus the Art Bible's 15-color spec. Any art pass built on top of the current resource will be palette-inconsistent from the start.
4. **Two unrelated player controllers and a whole second game's worth of autoload state (`GameManager.gd`'s kingdom-builder logic, `groot_theme.tres`, "Ironhaven" references) coexist in the project** — not currently gameplay-blocking (dead code, per the traced boot path), but a real risk for future confusion, accidental wrong-scene wiring, and wasted maintenance effort. `GameManager.gd`'s builder-game data actually loads on every boot even though it appears unused.
5. **7 broken `res://` references found by sampling** (see Broken paths) — none observed to break the traced Splash→Menu boot path, but at least one (`LandingBayAlpha.tscn`) is directly relevant to the spec's room-naming ask and should be resolved either by creating the file or removing the reference.
6. **No campaign/room content is checked into version control** — rooms are generated into `user://saves/spaceminer/campaigns/` at runtime from a seed, so "is there a complete playable route" cannot be verified by reading the repo; it can only be verified by actually running the game. This makes design review and regression testing harder than a hand-authored room set would.
7. **Duplicate/ambiguous autoload registrations** — `SettingsManager` is registered under two global names pointing at the same script (two live singleton instances), and `TooltipManager.gd`/`ToolTipManager.gd` both exist with only one registered — low risk of an actual bug today, but worth cleaning up so future autoload references are unambiguous.
8. **Uncommitted, in-progress `addons/` deletion** — `git status` shows the entire `addons/AsepriteWizard` plugin deleted from disk but not committed, which is unrelated to gameplay but means the working tree is currently in a partially-cleaned-up state; worth resolving (commit or restore) before further work so the audit's file-existence findings stay valid.

## What is already working

Being specific and not underselling this — a substantial amount of real systems work exists:

- **Clean headless boot** on Godot 4.6 with zero script parse errors across the whole autoload + Splash + Menu path.
- **A real app-level state machine** (`AppState`/`StateManager`) with pause, settings, credits, and a `StateAwareController` base class that scenes hook into to receive state-change notifications — this is a solid, reusable pattern, not spaghetti.
- **A full seeded save/run system** (`RunManager.gd`) — JSON-serialized runs keyed by seed, one save-slot-per-seed collapsing logic, lives/score/ore/oxygen/fuel/battery/equipment/security-card tracking, and checkpoint support (`save_checkpoint`/`get_checkpoint_room_id`) that resets fuel/battery on checkpoint. This is more save-system infrastructure than the spec's "avoid complex save systems until the main loop works" guidance calls for at this stage, but it works and is not broken.
- **A large, feature-complete (if over-scoped relative to spec) player controller**: gravity, jump, jetpack with fuel drain and directional thrust, grapple with an aiming arc and rope-sag visual, laser pistol with raycast hit detection, magnetic boots with surface-normal-following movement, ladder climbing, one-way-platform drop-through, and a hazard-hit hook (`handle_hazard_hit`) that a visibility cloak can absorb via battery drain instead of dying. All of this is implemented, not stubbed.
- **Procedural room generation is genuinely deep**, not a toy: `AsteroidCampaignGenerator.gd` builds a 56-room critical-path-plus-branches campaign with named rooms (both a curated `MAIN_ROOM_NAMES` list and a large `EPISODE_NAMES` pool for flavor), and `RoomLayoutGenerator.gd` places platforms, hazards, teleporters (`_place_teleporters`, 40% chance per room from room 3 onward — matches the recent git history: "Add teleporters to room generation", "Make teleporters chance-based, not guaranteed"), doors, and pickups.
- **A solvability validator exists** (`scripts/tools/LevelSolvabilityValidator.gd`) — someone already built tooling toward the spec's "no softlocks / every room solvable without RNG" requirement, which is a promising sign even though its current coverage wasn't traced here.
- **Hazard variety matches the spec's allowed list well**: `acid_bath.gd`, `moving_spike.gd`, `fire_pipe.gd`, `conveyor_belt.gd`, `sliding_wall.gd`, `moving_platform.gd`, plus enemy scripts (`dwarf_miner.gd`, `dalek_enemy.gd`, `ufo_enemy.gd`, `PatrolDrone.gd`).
- **Power-up/equipment coverage exceeds the spec's minimum set**: jetpack, magnetic boots, laser pistol, grappling hook, visibility cloak, oxygen/air canister (`air_canister.gd`), fuel cell, battery pack, ore fragments, security cards, and door zones (`door_zone.gd`) are all implemented with dedicated scripts and scenes.
- **A working inline HUD** showing room name, ore, oxygen, fuel, battery, and score, driven by real `RunManager`/room-status data rather than placeholders.
- **Recent git history is focused and incremental**, not thrashing: the last ~15 commits are coherent room-generation/door/teleporter/jump fixes (`a37dd3f Expand door landing platforms...`, `32f84b0 Make teleporters chance-based...`, `d705701 Add teleporters to room generation`, `1744a7c Fix jump and improve door tinting`), suggesting active, purposeful iteration on the procedural-room system specifically.

## What must be fixed first

Prioritized for getting to a state where the spec's Phase 1/2 ("Make It Run" / "Core Loop") can be honestly claimed:

1. **Resolve the strategic tension** (see top section) — get an explicit decision from the user on procedural-vs-hand-authored rooms and on the equipment/air-control scope, before spending more implementation time, since it changes almost every other line item below.
2. **Fix or remove the broken `LandingBayAlpha.tscn` reference** and decide whether a hand-authored intro room is still wanted per spec, or whether the generated `room_000` of a fixed/curated seed should be treated as "Landing Bay Alpha" going forward.
3. **Rebuild `assets/palettes/space_miner_palette.tres`** from the GDD's 15-color list using the existing `space_miner_palette_sheet.gd` tooling — this is small, mechanical, and unblocks any subsequent art-consistency pass.
4. **Decide the fate of the dead code**: twin-stick shooter (`scripts/controllers/Player.gd`, `scenes/units/*`, `scenes/states/GameWorld.tscn`) and the kingdom-builder `GameManager.gd` contents. At minimum, stop `GameManager.gd` from doing its full data-load on every boot if it's confirmed unused, to reduce boot noise and confusion; ideally remove the dead scenes/scripts entirely once confirmed unreferenced.
5. **If keeping the platformer controller's current equipment scope**, explicitly re-scope it against the spec (or update project documentation to supersede the spec) so "no mid-air steering" isn't a standing, silently-violated rule; if reverting to spec, strip `air_control_accel`-driven airborne movement out of `scripts/prototype/player.gd`.
6. **Clean up duplicate autoload registrations** (`SettingsManager` registered twice; `TooltipManager.gd` vs `ToolTipManager.gd`) and delete the orphaned `.uid` for the missing `GameState.gd`.
7. **Resolve the in-progress uncommitted `addons/AsepriteWizard` deletion** and the other working-tree changes shown in `git status`, so the repo is in a known-clean state before further audits/work.
8. **Verify an actual complete playable route exists** by running a real (not headless) session end-to-end, since campaign content is generated at runtime and isn't inspectable from the repo alone — this is required to honestly answer the spec's "at least one complete playable route from Landing Bay Alpha to an ending room" success criterion.
