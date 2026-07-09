# 🪓 Space Miner  
*Spiritual successor to Jet Set Willy & Manic Miner*  
**Genre:** Retro platform adventure  
**Engine:** Godot 4.4  
**Target Platforms:** PC (Linux, Windows), Web  

---

## 📖 Concept Overview
*Space Miner* is a precision platform adventure set on a derelict asteroid mining complex.  
Players control a lone miner exploring ancient shafts and alien ruins, gathering resources and unlocking sealed sectors. The design pays homage to *Jet Set Willy* and *Manic Miner*: fixed jump arcs, room-by-room exploration, and collectible-driven progression — now expanded to fifty interconnected screens forming a single asteroid base.

---

## 🎯 Design Goals

1. **Faithful 8-bit feel:** authentic physics, sound, and visual constraints.  
2. **Expanded scope:** fifty rooms with light Metroidvania structure (keys, powerups).  
3. **Exploration challenge:** each screen is a self-contained puzzle with precise timing.  
4. **Zero RNG:** full memorisation and mastery possible.  
5. **Playable on modern systems:** smooth scaling, controller support, optional checkpoints.

---

## ⚙️ Core Mechanics

### Movement
- **Walk:** Player moves 1 tile per 0.2 seconds.  
- **Jump:** Fixed-height and fixed-distance (≈1.5 tiles high × 3 tiles wide).  
- **No mid-air control:** once committed, the jump trajectory is locked.  
- **Landing:** instantly stops all horizontal momentum.  
- **Fall:** constant gravity, no acceleration curve.  
- **Death on contact:** spikes, enemies, or environmental hazards reset the room.  

```gdscript
if is_on_floor() and jump_pressed:
    velocity.y = -JUMP_SPEED
    velocity.x = facing_dir * MOVE_SPEED
    jumping = true

if jumping and is_on_floor():
    jumping = false
    velocity.x = 0
```

### Controls
| Action | Input | Description |
|--------|--------|-------------|
| Move | ← / → | Walk left or right |
| Jump | Space / A | Lock jump direction + arc |
| Interact / Pickup | ↓ or E | Collect item or trigger console |
| Pause | ESC | Shows room title + menu overlay |

### Oxygen & Survival
- Player has a limited oxygen supply.  
- Oxygen resets on reaching a **pressure door** or **base zone**.  
- Timer encourages efficient routing without introducing combat stress.

---

## 🧩 Room & Level Structure

### Format
- 50 interconnected **single-screen** rooms.  
- Each screen loads instantly (no scrolling).  
- Each has a unique name (e.g., “The Reactor Shaft”, “Vault of Echoes”).  
- Player starts in **Landing Bay Alpha** and gradually expands access deeper into the asteroid.

### Layout Rules
- Platforms spaced to respect fixed jump arcs (3-tile horizontal, 1–2 vertical).  
- Hazards positioned to make route planning central.  
- Safe zones placed for timing windows, not luck.  
- Exits only at screen edges (doors, lifts, or gravity shafts).  

### Procedural Asteroid Layouts
- The broader asteroid map should be built from a 56-room interior graph, not a flat list of isolated screens.
- Generation should guarantee at least one valid route from the start room to the exit room, plus a reachable route to each mandatory ore room.
- Use a path-first pass to place the critical rooms and connect them with a solvable route, then branch out with optional side rooms and rewards.
- Use a second pass for local variation and room dressing so the asteroid feels hand-authored rather than mechanically tiled.
- Treat wave-function-collapse style rules as a local decoration / adjacency tool, not as the main solvability system.
- Every generated layout must be validated before it is accepted by the game.

### Suggested Map Topology
```
   [Landing Bay]
        |
 [Control Hub]--[Refinery]--[Core Access]
        |             |           |
 [Dorms]--[Workshop]--[Vault]--[Fusion Chamber]
        |                         |
     [Oxygen Garden]---[Deep Mines]
```

> *Each major area contains 5–10 screens and 1 power-up.*

---

## 🔑 Progression System

### Power-ups
| Power-up | Effect | Location Type |
|-----------|---------|----------------|
| Jetpack | Extends jump height by 1 tile | Found in Refinery |
| Gravity Boots | Allow landing on magnetic ceilings | Found in Core Access |
| Laser Cutter | Breaks specific ore tiles | Found in Workshop |
| Oxygen Tank | Doubles survival time | Found in Oxygen Garden |
| Access Card | Opens matching colour doors | Multiple locations |

### Keys & Doors
- Classic *coloured key* system: Red, Blue, Green, Yellow.  
- Keys are persistent until used.  
- Some rooms require power-ups *and* matching keys to enter.  

### Collectibles
- **Ore Fragments:** increase score; 100 per run for completion.  
- **Alien Relics:** optional secrets unlocking bonus rooms.  
- **Energy Cells:** used to reactivate machinery (narrative triggers).

---

## 🪨 Setting & Aesthetic

### Environment
- The entire game takes place **inside and around a hollow asteroid**.  
- Themes: rusted metal, flickering consoles, alien bioluminescence.  
- Background layers emulate 8-bit parallax with only two scrolling planes.

### Art Direction
See [Space_Miner_Art_Bible.md](./Space_Miner_Art_Bible.md) for the current production spec.

| Element | Style |
|----------|--------|
| Grid | `8x8` sprite/tile unit; rooms may fill the screen and should still snap to the grid |
| Tiles | Hard-edged, low-count room palettes; no smoothing or gradients |
| Player | Tiny miner silhouette with limited frames and strong contrast |
| Enemies | Simple looped patterns, instantly readable shapes |
| UI | Blocky retro readout text with room title, oxygen, and ore |
| Palette | Black void base with bright miner accents, hazard reds, and CPC-style flashes |

### Production Palette

The canonical Godot palette resource is `res://assets/palettes/space_miner_palette.tres`.

| Name | Hex | Role |
|------|-----|------|
| Void Black | `#000000` | Primary background and negative space |
| Deep Space Blue | `#07111F` | Secondary dark background tone |
| Panel Charcoal | `#1B1B24` | UI panels and quiet machinery |
| Metal Grey | `#8C8C8C` | Neutral structural metal |
| Bright White | `#F4F4F4` | High contrast sprite and UI detail |
| CPC Cyan | `#00D7FF` | Critical readable tech, oxygen, highlights |
| Deep Blue | `#1D4FFF` | Cold machinery, shadowed tech detail |
| Miner Yellow | `#FFD21A` | Player focus, pickups, important markers |
| Hazard Orange | `#FF8A00` | Heat, warning edges, hazard accents |
| Alarm Red | `#FF1E2D` | Death hazards and urgent warnings |
| Alien Green | `#00D66B` | Alien vegetation or safe weird-tech glow |
| Toxic Lime | `#8CFF00` | Toxic platforms, active alien energy |
| Power Purple | `#5A2DFF` | Weird tech, portals, alien devices |
| Pickup Magenta | `#FF00C8` | Bonus pickups and high-value items |
| Chroma Magenta | `#FF00FF` | Mask/debug/chroma key colour only |

#### Practical Palette Rules

- Each room uses Void Black plus 3 to 5 main colours.
- Do not use the whole palette in every room.
- Use Miner Yellow, CPC Cyan, and Bright White for objects the player must notice.
- Use Alarm Red, Hazard Orange, and Pickup Magenta for hazards or danger-adjacent rewards.
- Use Alien Green, Toxic Lime, and Power Purple for alien or weird technology.
- Keep backgrounds dark and quiet; foreground gameplay silhouettes must read first.
- Sprite sheets should be authored as indexed-feeling pixel art with hard edges, no gradients, and no smoothing.

---

## 🎵 Music & Audio Direction  

### Overall Approach
- **Style:** 8-bit arrangements of classical works in AY-chip or SID-style instrumentation.  
- **Goal:** evoke humour and grandeur – the absurd heroism of mining an asteroid to the tune of famous symphonies.  
- **All pieces public domain** (composers pre-1920 → no copyright issues).

### Track List Proposal
| Area / Screen Group | Classical Theme (Composer) | Tone / Reasoning |
|----------------------|----------------------------|------------------|
| **Title Screen / Menu** | *Also Sprach Zarathustra – Fanfare* (R. Strauss) | Ironic space grandeur |
| **Landing Bay Alpha** | *The Blue Danube* (J. Strauss II) | Elite homage; calm orbital waltz |
| **Control Hub** | *Eine kleine Nachtmusik* (Mozart) | Precise rhythm; orderly machinery |
| **Refinery Zone** | *In the Hall of the Mountain King* (Grieg) | Builds tension |
| **Oxygen Garden** | *Morning Mood* (Grieg) | Safe, serene zone |
| **Vault & Core Access** | *Toccata and Fugue in D minor* (Bach) | Gothic descent |
| **Fusion Chamber / Finale** | *Symphony No. 5 – Allegro con brio* (Beethoven) | Triumphant escape |
| **Game Over** | *Funeral March of a Marionette* (Gounod) | Comic death |
| **Ending Credits** | *Ode to Joy* (Beethoven) | Optimistic closure |

### Elite Homage
- Docking and airlock transitions play a brisk “Docking Waltz” remix of *The Blue Danube*.
- Achievement: **“Right On, Miner.”** for perfect docking without oxygen loss.
- Optional rotating docking collar background during transition.

---

## 🧠 Game Logic & Flow

1. Player begins at **Landing Bay Alpha**.  
2. Explore rooms, collecting ore and keys.  
3. Reach the **Control Hub**, unlocking deeper zones.  
4. Acquire power-ups to access previously sealed areas.  
5. Survive descending oxygen levels as you delve deeper.  
6. Reach **Fusion Chamber** to restore power and complete the game.  

---

## 🪐 Difficulty & Balance

- Every hazard has a deterministic pattern.  
- Room solutions rely on planning jump order, not twitch reaction.  
- Lives system optional:  
  - *Classic Mode:* 3 lives, restart from Landing Bay.  
  - *Modern Mode:* Infinite retries, room resets only.  
- Checkpoints every 10 rooms (auto-save).

---

## 💾 Technical Notes (Godot 4.4)

- **Top Node:** `Main` singleton controlling room transitions, oxygen timer, and save data.  
- **Rooms:** `Room_xx.tscn`, each with static tilemap, entities, and exit signals.  
- **Player:** `CharacterBody2D` with state machine for idle/walk/jump/dead.  
- **Hazards:** `Area2D` triggers instant respawn.  
- **Transitions:** handled via `change_scene_to_file()` with fade effect.  
- **Audio:** single `AudioManager` autoload controlling per-room track.

---

## 🧾 Narrative & Tone

### Backstory
You’re a contract miner sent to reactivate an abandoned asteroid facility near the Jovian belt.  
Long since silent, the mine still hums with residual power and whispers of alien discovery.  
Rumour says the miners found something beneath the ore — and didn’t survive long enough to report it.

### Tone
Dry humour meets existential sci-fi minimalism — think *Red Dwarf* meets *The Expanse*.  
Room names carry wit and menace:  
- “A Shaft Too Far”  
- “The Gravity of the Situation”  
- “Drill Baby Drill”  
- “The Core of the Matter”

---

## 🧱 Development Milestones

| Phase | Duration | Goals |
|--------|-----------|--------|
| **Prototype (1 month)** | Build player physics, 5 sample rooms, item pickup logic |
| **Alpha (2 months)** | 25 rooms, 3 power-ups, save/load, audio |
| **Beta (3 months)** | 50 rooms, all art and music, testing |
| **Release** | Export to PC & Web; Steam & itch.io |

---

## 🧾 License & Attribution
- **Original concept and assets:** © Peter Kelly (Computer Wizard Games)  
- **Engine:** Godot 4.4  
- **Fonts:** ZX Spectrum & C64 public domain sets  
- **Music:** Classical public-domain arrangements (J. Strauss II, Grieg, Bach, Beethoven, Gounod, Mozart)  
- **Audio:** Composed in FamiTracker / MilkyTracker by Computer Wizard Games  

---

## 🧩 Optional Expansions
- “**Asteroid Delta**” — new 25-room DLC with gravity inversion.  
- Time Trial / Score Rush modes.  
- Cooperative mode (split screen two miners).  
