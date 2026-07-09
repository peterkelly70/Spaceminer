# Space Miner Art Bible

## Core Target
Space Miner should read like a proper 8-bit single-screen platformer, not a modern game wearing pixel art.

Primary references:
- `Manic Miner`
- `Jet Set Willy`
- Amstrad CPC-era high-contrast platform rooms

The visual language should be:
- compact
- readable at a glance
- aggressively contrasty
- room-specific
- technically simple

Asset references:
- Palette resource: `res://assets/palettes/space_miner_palette.tres`
- Palette sheet scene: `res://scenes/debug/PaletteSheet.tscn`

## Grid Rules
- Use `8x8` as the base sprite and tile unit.
- Rooms can use as much of the screen as needed; do not interpret `8x8` as the room size.
- Compose tiles, hazards, props, and UI glyphs from `8x8` blocks first, then combine into larger shapes when needed.
- Use `16x16` only as a display convenience if necessary; the underlying art should still snap to `8x8`.
- Sprites should remain small and silhouette-driven.
- Avoid smoothing, soft gradients, and modern shading.

## Room Composition
Each room should feel like a fixed puzzle box.

Rules:
- Large black or near-black negative space is allowed and encouraged.
- Platforms should be chunky, geometric, and clearly separable from background detail.
- Each room gets a small, intentional palette.
- Backgrounds must never compete with gameplay objects.
- Important interactions should be readable even from a static screenshot.

## Palette Direction
The palette should support two moods:
- strict retro miner rooms with 3 to 5 active colors
- bright CPC-style accent bursts for effects and highlights

Suggested color roles:
- background: black, charcoal, or deep void blue
- structural metal/stone: red, cyan, green, grey, ochre
- hazards: hot red, magenta, orange
- pickups: bright yellow, electric cyan, white
- UI text: white or yellow on black

Rules:
- Prefer a small palette per room.
- Use bright colors as signals, not decoration.
- High-brightness flashes are reserved for firing, pickups, alarms, and transitions.
- Palette cycling is preferred over complex lighting for “juice”.

## Sprite Rules
Player:
- tiny, readable miner silhouette
- no realistic anatomy
- limited frames, ideally 2 to 4 useful poses
- strong contrast against the room

Hazards:
- instantly identifiable by shape and color
- can be animated by simple alternating frames or palette swap

Pickups:
- jewel-like or mineral-like forms
- bright enough to pop against black space
- should read as collectable at room scale

## HUD Rules
The HUD should feel like part of the same retro machine.

Suggested layout:
- room title
- oxygen or pressure
- ore count
- score or progress

Rules:
- Keep the HUD compact and screen-edge aligned.
- Use blocky, high-contrast typography.
- Separate HUD from gameplay with clear horizontal bars or framed panels.
- Title text should feel like a room label or machine readout, not a modern overlay.

## Juice Rules
Use polish that preserves the exactness of the gameplay.

Safe effects:
- tiny landing squash
- brief pickup flash
- palette pulse on firing or hazard activation
- small screen shake on heavy impact only
- short hit stop on death or major interaction
- simple particles built from single-color pixels or tiny blocks

Avoid:
- variable jump assist as a “juice” upgrade
- heavy blur
- soft lighting bloom
- large modern VFX trails
- anything that hides platform edges or hazard timing

## Landing Bay Alpha
The first screen should establish the whole game.

Goals:
- immediate readability
- one-screen puzzle clarity
- strong 8-bit mood
- black void plus bright platform accents

Recommended features:
- a mostly dark room with a few vivid structural colors
- a bright title strip or room label
- one or two strong hazard colors
- one standout pickup color
- a visible exit or pressure door

The screen should say:
- this is old-school
- this is exact
- this is hostile but fair

## Do Not Drift
Do not accidentally drift toward:
- cave explorer platformer genericism
- modern metroidvania soft art
- muddy background gradients
- detailed props that break room readability
- too many palette colors per screen
