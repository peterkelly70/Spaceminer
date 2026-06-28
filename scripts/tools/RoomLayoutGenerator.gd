extends RefCounted
class_name RoomLayoutGenerator

# ── Room coordinate system (centre-origin, matching AsteroidCampaignGenerator) ──
const ROOM_HALF_W  := 360
const ROOM_HALF_H  := 200
# Floor solid: body.y = 152 (position 144 + anchor "top" half-height 8).
# LevelBuilder puts CollisionShape2D at body centre, so walkable surface
# (shape top) = 152 - 8 = 144 = 9 × TILE_SZ.  Tile row 9 draws at y=144.
# FLOOR_TOP_Y MUST be a multiple of TILE_SZ so platform tiles align with their
# collision surfaces (integer tile-row division).
const FLOOR_Y_CTR  := 160   # floor StaticBody2D position reference
const FLOOR_TOP_Y  := 144   # walkable floor surface = 9 × TILE_SZ
const WALL_X       := 352
const PLAT_THICK   := 16    # 1 tile
const TILE_SZ      := 16    # tileset cell size in pixels

# Jump physics  (must match LevelSolvabilityValidator and player.gd JUMP_SPEED).
# Player is 32 px tall; LEVEL_STEP gives vertical clearance to stand under and
# jump up onto the next level.
const MAX_JUMP_UP  := 64.0
const MAX_JUMP_GAP := 96.0

# Platform grid — 6 columns, 5 height levels, all snapped to the 16 px tile grid.
#
# Column centres: ±56, ±168, ±280  (each = 40 + n×112, left/right edges on 16 px boundaries)
#   Left edges:  -320, -208, -96,  16, 128, 240   (all multiples of 16)
#   Right edges: -240, -128, -16,  96, 208, 320
#   Edge gap between adjacent columns: 112 - 80 = 32 px (2 tiles), always jumpable.
#
# Level heights (surface = FLOOR_TOP_Y - (n+1)×LEVEL_STEP), LEVEL_STEP=48:
#   Level 0 → top  96
#   Level 1 → top  48
#   Level 2 → top   0
#   Level 3 → top -48
#   Level 4 → top -96
const COL_COUNT    := 6
const COL_X        := [-280, -168, -56, 56, 168, 280]
const PLAT_W       := 80    # 5 tiles wide; with centres above, both edges on 16 px grid
const LEVEL_COUNT  := 6     # levels 0..5 — use more of the room's vertical space
const LEVEL_STEP   := 48    # 3 tiles per step — clearance for the 32px-tall player

const ORE_SCENE      := "res://scenes/prototype/OreFragment.tscn"
const AIR_SCENE      := "res://scenes/prototype/AirCanister.tscn"
const FUEL_SCENE     := "res://scenes/prototype/FuelCell.tscn"
const BATTERY_SCENE  := "res://scenes/prototype/BatteryPack.tscn"
const STATION_SCENE  := "res://scenes/prototype/ResupplyStation.tscn"
const DRONE_SCENE    := "res://scenes/units/PatrolDrone.tscn"
const DALEK_SCENE    := "res://scenes/units/Dalek.tscn"
const UFO_SCENE      := "res://scenes/units/Ufo.tscn"
const HAZARD_SCENE      := "res://scenes/prototype/HazardZone.tscn"
const FIRE_PIPE_SCENE    := "res://scenes/prototype/FirePipe.tscn"
const MOVING_SPIKE_SCENE := "res://scenes/prototype/MovingSpike.tscn"
const ACID_BATH_SCENE    := "res://scenes/prototype/AcidBath.tscn"
const CONVEYOR_SCENE     := "res://scenes/prototype/ConveyorBelt.tscn"
const MOVING_PLAT_SCENE  := "res://scenes/prototype/MovingPlatform.tscn"
const SLIDING_WALL_SCENE := "res://scenes/prototype/SlidingWall.tscn"

enum LayoutTheme { ASCENT, DESCENT, VALLEY, PEAK, PLATEAU, ZIGZAG }

# Distinct structural room shapes — each builds a different kind of space.
enum RoomArchetype { STAIRCASE, SHAFT, ISLANDS, TOWERS, VAULT, PIT, CAVERN }

# ── Room archetypes ────────────────────────────────────────────────────────────

func _pick_archetype(rng: RandomNumberGenerator, room_index: int) -> int:
	if room_index == 0:
		return RoomArchetype.STAIRCASE   # gentle intro room
	var pool := [
		RoomArchetype.STAIRCASE, RoomArchetype.SHAFT, RoomArchetype.ISLANDS,
		RoomArchetype.TOWERS, RoomArchetype.VAULT, RoomArchetype.PIT,
		RoomArchetype.CAVERN,
	]
	return pool[rng.randi() % pool.size()]

# Minimum vertical gap between horizontally-overlapping platforms. The player is
# 32px tall; anything closer than this leaves no head-clearance and produces the
# cramped "walk under a platform" look. 2 × LEVEL_STEP (96) gives comfortable room.
const MIN_STACK_GAP := 80

# Returns { "platfs": Array, "floor_gaps": Array[[x0,x1]] }
func _build_archetype(arche: int, rng: RandomNumberGenerator, room_index: int, exits: Array) -> Dictionary:
	var build: Dictionary
	match arche:
		RoomArchetype.SHAFT:     build = {"platfs": _arch_shaft(rng)}
		RoomArchetype.ISLANDS:   build = {"platfs": _arch_islands(rng)}
		RoomArchetype.TOWERS:    build = {"platfs": _arch_towers(rng)}
		RoomArchetype.VAULT:     build = {"platfs": _arch_vault(rng)}
		RoomArchetype.PIT:       build = _arch_pit(rng)
		RoomArchetype.CAVERN:    build = {"platfs": _arch_cavern(rng, exits)}
		_:                       build = {"platfs": _arch_staircase(rng)}
	build["platfs"] = _declutter_platforms(build["platfs"])
	return build

# Drops platforms that sit too close above another overlapping platform, so the
# player always has clear head-room and rooms don't look cramped.
func _declutter_platforms(platfs: Array) -> Array:
	var kept: Array = []
	# Keep lower platforms first (larger top_y), discard the cramped one above.
	var sorted := platfs.duplicate()
	sorted.sort_custom(func(a, b): return float(a["top_y"]) > float(b["top_y"]))
	for p in sorted:
		var ok := true
		for k in kept:
			if _x_overlap(p, k) and absf(float(p["top_y"]) - float(k["top_y"])) < MIN_STACK_GAP:
				ok = false
				break
		if ok:
			kept.append(p)
	return kept

# Ensure spawn area (around x=-300 or x=0, y=128) has clearance for player.
# Player is 32px tall; needs at least 50px headroom above floor.
func _ensure_spawn_clearance(platfs: Array) -> Array:
	var spawn_xs := [-300.0, 0.0]  # both main and branch spawn x coords
	var spawn_y := 128.0
	var min_clearance := 50.0     # minimum space above floor for player

	var filtered: Array = []
	for p in platfs:
		var px := float(p["cx"])
		var py_top := float(p["top_y"])
		var pw := float(p["width"]) * 0.5

		# Check if platform overlaps spawn zone horizontally
		var overlaps_spawn := false
		for sx in spawn_xs:
			if absf(px - sx) < pw + 20.0:  # 20px margin around spawn
				overlaps_spawn = true
				break

		# If overlaps and would trap player, skip it
		if overlaps_spawn and py_top < spawn_y - min_clearance:
			continue

		filtered.append(p)

	return filtered if filtered.size() > 0 else platfs

func _x_overlap(a: Dictionary, b: Dictionary) -> bool:
	var ahw := float(a["width"]) * 0.5
	var bhw := float(b["width"]) * 0.5
	# +12 margin (~player half-width) so near-touching columns count as overlapping
	return absf(float(a["cx"]) - float(b["cx"])) < ahw + bhw + 12.0

# Helper: a platform dict with derived cy/level, snapped to the tile grid.
func _mk_plat(name: String, cx: int, top_y: int, width: int) -> Dictionary:
	var sx := (cx / TILE_SZ) * TILE_SZ
	var sy := (top_y / TILE_SZ) * TILE_SZ
	var sw := maxi(TILE_SZ * 2, (width / TILE_SZ) * TILE_SZ)
	return {
		"name": name, "kind": "platform", "anchor": "top",
		"cx": sx, "cy": sy + PLAT_THICK, "top_y": sy,
		"width": sw, "level": _level_of(sy),
	}

func _level_of(top_y: int) -> int:
	return clampi(int(round(float(FLOOR_TOP_Y - top_y) / float(LEVEL_STEP))) - 1, 0, LEVEL_COUNT - 1)

# Diagonal flight of steps climbing across the room, with the odd back-step.
func _arch_staircase(rng: RandomNumberGenerator) -> Array:
	var out: Array = []
	var dir := 1 if rng.randf() < 0.5 else -1
	var steps := rng.randi_range(5, 7)
	var lvl := 0
	for i in range(steps):
		var cx := -260 * dir + dir * int(round(float(i) * 520.0 / float(steps - 1)))
		lvl = clampi(lvl + rng.randi_range(0, 1), 0, LEVEL_COUNT - 1)
		var w: int = [64, 64, 80][rng.randi() % 3]
		out.append(_mk_plat("Step%d" % i, cx, _level_top_y(lvl), w))
	return out

# A vertical climbing shaft: zig-zag rungs up one side, a wide landing on top.
func _arch_shaft(rng: RandomNumberGenerator) -> Array:
	var out: Array = []
	var side := -1 if rng.randf() < 0.5 else 1
	for k in range(1, LEVEL_COUNT):
		var cx := side * (40 if k % 2 == 0 else 150)
		out.append(_mk_plat("Rung%d" % k, cx, _level_top_y(k), 80))
	out.append(_mk_plat("ShaftTop", side * 200, _level_top_y(LEVEL_COUNT - 1), 128))
	# a lower platform on the opposite side to start the climb
	out.append(_mk_plat("ShaftBase", -side * 180, _level_top_y(0), 96))
	return out

# Scattered floating islands at varied heights with deliberate gaps.
func _arch_islands(rng: RandomNumberGenerator) -> Array:
	var out: Array = []
	var n := rng.randi_range(6, 9)
	for i in range(n):
		var cx := rng.randi_range(-280, 280)
		var lvl := rng.randi_range(0, LEVEL_COUNT - 1)
		var w: int = [48, 48, 64, 80][rng.randi() % 4]
		out.append(_mk_plat("Isle%d" % i, cx, _level_top_y(lvl), w))
	return out

# Two or three vertical stacks separated by gaps, crossable near the top.
func _arch_towers(rng: RandomNumberGenerator) -> Array:
	var out: Array = []
	var count := rng.randi_range(2, 3)
	for t in range(count):
		var tx := -240 + t * int(480.0 / float(maxi(count - 1, 1)))
		# Stack every 2nd level so there's clear head-room between rungs
		var rungs := rng.randi_range(2, 3)
		for r in range(rungs):
			var k := r * 2
			if k >= LEVEL_COUNT:
				break
			out.append(_mk_plat("Tower%d_%d" % [t, r], tx, _level_top_y(k), 64))
	return out

# A guarded reward: a climb leading to a high corner alcove (rich, deep loot).
func _arch_vault(rng: RandomNumberGenerator) -> Array:
	var out: Array = []
	var corner := 1 if rng.randf() < 0.5 else -1
	# stepping platforms climbing toward the corner
	for k in range(LEVEL_COUNT):
		var cx := -corner * 220 + corner * int(round(float(k) * 440.0 / float(LEVEL_COUNT - 1)))
		out.append(_mk_plat("Climb%d" % k, cx, _level_top_y(k), 72))
	# the vault ledge tucked in the top corner
	out.append(_mk_plat("Vault", corner * 270, _level_top_y(LEVEL_COUNT - 1), 96))
	return out

# A chasm in the floor with a narrow bridge across and side ledges.
func _arch_pit(rng: RandomNumberGenerator) -> Dictionary:
	var out: Array = []
	var half := rng.randi_range(96, 144)
	out.append(_mk_plat("Bridge", 0, _level_top_y(1), 96))
	out.append(_mk_plat("LedgeL", -240, _level_top_y(0), 96))
	out.append(_mk_plat("LedgeR", 240, _level_top_y(0), 96))
	out.append(_mk_plat("PerchL", -120, _level_top_y(2), 64))
	out.append(_mk_plat("PerchR", 120, _level_top_y(2), 64))
	return {"platfs": out, "floor_gaps": [[-half, half]]}

# Organic scatter — the original height-walk plus extra shelves, more size variety.
func _arch_cavern(rng: RandomNumberGenerator, exits: Array) -> Array:
	var target := _exit_target_level(exits, "east")
	var levels := _gen_levels(rng, _pick_theme(rng, 1), target)
	var out := _levels_to_platforms(levels)
	out.append_array(_extra_platforms(rng, out, exits))
	return out

# ── Public API ─────────────────────────────────────────────────────────────────

func generate_main(rng: RandomNumberGenerator, room_index: int, exits: Array) -> Dictionary:
	var build := _build_archetype(_pick_archetype(rng, room_index), rng, room_index, exits)
	var platfs: Array = build["platfs"]
	var floor_gaps: Array = _merge_south_gaps(build.get("floor_gaps", []), exits)
	platfs.append_array(_exit_landings(exits))
	platfs = _ensure_spawn_clearance(platfs)
	var solids := _build_solids(platfs, exits, floor_gaps)
	_ensure_path(solids, rng)
	var depths := _platform_depths(platfs)
	var enemy_result := _place_enemies_tracked(rng, platfs, room_index, depths)
	var ladders := _build_ladders(rng, platfs)
	ladders.append_array(_exit_ladders(exits))
	var mechanisms := _place_mechanisms(rng, platfs, room_index)
	return {
		"spawn":        [-300, 128],
		"solids":       solids,
		"ladders":      ladders,
		"decor":        _build_decor(rng, room_index),
		"collectibles": _place_ore(rng, platfs, room_index, enemy_result[1], depths, floor_gaps),
		"hazards":      _place_hazards(rng, platfs, room_index, floor_gaps),
		"enemies":      enemy_result[0],
		"mechanisms":   mechanisms,
		"tile_layers":  _build_tiles(platfs, room_index, floor_gaps),
	}

func generate_resupply(rng: RandomNumberGenerator, room_index: int, exits: Array) -> Dictionary:
	# Safe room: flat floor, 3 refill stations, no enemies or hazards.
	# Acts as a Metroid-style save point.
	var solids: Array = []
	solids.append({"name": "Floor", "kind": "floor", "anchor": "top", "position": [0, FLOOR_Y_CTR - PLAT_THICK], "size": [660, 16]})
	_add_walls(solids, exits)
	# Three platform pedestals for the stations
	var pedestal_xs := [-160, 0, 160]
	var pedestal_cy := float(FLOOR_Y_CTR - PLAT_THICK)   # body.y=144, surface at 128
	for i in range(3):
		solids.append({
			"name": "Pedestal%d" % i,
			"position": [float(pedestal_xs[i]), pedestal_cy],
			"size": [64.0, float(PLAT_THICK)],
		})
	var station_types := ["air", "fuel", "battery"]
	var station_labels := ["AIR", "FUEL", "BATT"]
	var pickups: Array = []
	for i in range(3):
		pickups.append({
			"name":         "Station_%s" % station_types[i],
			"scene":        STATION_SCENE,
			"position":     [float(pedestal_xs[i]), pedestal_cy - 24.0],
			"props": {
				"station_type": station_types[i],
				"label_text":   station_labels[i],
			},
		})
	return {
		"spawn":        [-260, 128],
		"solids":       solids,
		"decor":        [],
		"collectibles": pickups,
		"hazards":      [],
		"enemies":      [],
		"tile_layers":  _build_tiles([], room_index),
	}

func generate_branch(rng: RandomNumberGenerator, room_index: int, exits: Array) -> Dictionary:
	# Branches use the vertical-friendly archetypes (shaft / towers / islands)
	var arche: int = [RoomArchetype.SHAFT, RoomArchetype.TOWERS, RoomArchetype.ISLANDS][rng.randi() % 3]
	var build := _build_archetype(arche, rng, room_index, exits)
	var platfs: Array = build["platfs"]
	var floor_gaps: Array = _merge_south_gaps(build.get("floor_gaps", []), exits)
	platfs.append_array(_exit_landings(exits))
	platfs = _ensure_spawn_clearance(platfs)
	var solids := _build_solids(platfs, exits, floor_gaps)
	_ensure_path(solids, rng)
	var depths := _platform_depths(platfs)
	var enemy_result := _place_enemies_tracked(rng, platfs, room_index, depths)
	var ladders := _build_ladders(rng, platfs)
	ladders.append_array(_exit_ladders(exits))
	var mechanisms := _place_mechanisms(rng, platfs, room_index)
	return {
		"spawn":        [0, 128],
		"solids":       solids,
		"ladders":      ladders,
		"decor":        _build_decor(rng, room_index),
		"collectibles": _place_ore(rng, platfs, room_index, enemy_result[1], depths, floor_gaps),
		"hazards":      _place_hazards(rng, platfs, room_index, floor_gaps),
		"enemies":      enemy_result[0],
		"mechanisms":   mechanisms,
		"tile_layers":  _build_tiles(platfs, room_index, floor_gaps),
	}

# ── Ladders ──────────────────────────────────────────────────────────────────
# Connect the floor to the tallest platform columns so the player can reach high
# ledges without precise multi-jump chains. Climbed with up/down.
func _build_ladders(rng: RandomNumberGenerator, platfs: Array) -> Array:
	var ladders: Array = []
	var sorted := platfs.duplicate()
	sorted.sort_custom(func(a, b): return float(a["top_y"]) < float(b["top_y"]))
	var used_x: Array = []
	var made := 0
	for p in sorted:
		if made >= 2:
			break
		var cx := int(p["cx"])
		if cx in used_x:
			continue
		var top := float(p["top_y"])
		# Only worth a ladder when the platform is more than ~1.5 jumps up
		if float(FLOOR_TOP_Y) - top < float(LEVEL_STEP) * 1.5:
			continue
		var bottom := float(FLOOR_TOP_Y)
		ladders.append({
			"name": "Ladder_%s" % str(p["name"]),
			"position": [float(cx), (top + bottom) * 0.5],
			"size": [14.0, bottom - top],
		})
		used_x.append(cx)
		made += 1
	return ladders

# ── Door reachability ──────────────────────────────────────────────────────────
# Every door gets a ledge to stand on, plus a ladder from the floor when it sits
# high up, so doors are never stranded in mid-air. South doors instead carve a
# hole in the floor so the player can drop through them.

func _exit_landings(exits: Array) -> Array:
	var out: Array = []
	for e in exits:
		var dir := str((e as Dictionary).get("direction", ""))
		var pos := _v2((e as Dictionary).get("position", [0, 0]))
		var sz := _v2((e as Dictionary).get("size", [40, 72]))
		match dir:
			"east":
				out.append(_mk_plat("EastLanding", ROOM_HALF_W - 40, _door_stand_y(pos, dir), 80))
			"west":
				out.append(_mk_plat("WestLanding", -(ROOM_HALF_W - 40), _door_stand_y(pos, dir), 80))
			"north":
				out.append(_mk_plat("NorthLanding", int(pos.x), _door_stand_y(pos, dir), maxi(80, int(sz.x))))
			# south doors use a floor gap (see _merge_south_gaps), no ledge
	return out

# Top-y of the ledge a door is entered onto — at the door's centre height so the
# validator counts the door as grounded, snapped to the tile grid.
func _door_stand_y(pos: Vector2, dir: String) -> int:
	match dir:
		"north":
			return -ROOM_HALF_H + 40
		_:
			return (int(pos.y) / TILE_SZ) * TILE_SZ

func _exit_ladders(exits: Array) -> Array:
	var out: Array = []
	for e in exits:
		var dir := str((e as Dictionary).get("direction", ""))
		var pos := _v2((e as Dictionary).get("position", [0, 0]))
		var sz := _v2((e as Dictionary).get("size", [40, 72]))
		var top := 0
		var lx := 0
		match dir:
			"east":
				top = _door_stand_y(pos, dir); lx = ROOM_HALF_W - 40
			"west":
				top = _door_stand_y(pos, dir); lx = -(ROOM_HALF_W - 40)
			"north":
				top = _door_stand_y(pos, dir); lx = int(pos.x)
			_:
				continue
		if FLOOR_TOP_Y - top > int(float(LEVEL_STEP) * 1.2):
			var bottom := FLOOR_TOP_Y
			out.append({
				"name": "DoorLadder_%s" % dir,
				"position": [float(lx), float(top + bottom) * 0.5],
				"size": [14.0, float(bottom - top)],
			})
	return out

func _merge_south_gaps(gaps: Array, exits: Array) -> Array:
	var out: Array = gaps.duplicate()
	for e in exits:
		if str((e as Dictionary).get("direction", "")) == "south":
			var pos := _v2((e as Dictionary).get("position", [0, 0]))
			var sz := _v2((e as Dictionary).get("size", [40, 72]))
			out.append([pos.x - sz.x * 0.5 - 10.0, pos.x + sz.x * 0.5 + 10.0])
	return out

# ── Platform reachability depth (difficulty) ───────────────────────────────────
# Hop-distance of each platform from the floor, using the same jump reach the
# player has. Higher depth = harder to get to = more rewarding loot, and the
# platforms that gate access to deeper ones make good enemy posts.
func _platform_depths(platfs: Array) -> Dictionary:
	var n := platfs.size()
	var depth: Dictionary = {}   # index -> hop count
	var queue: Array = []
	# Seed: platforms the player can jump to directly from the floor
	for i in range(n):
		var top := float(platfs[i]["top_y"])
		if float(FLOOR_TOP_Y) - top <= MAX_JUMP_UP and top <= float(FLOOR_TOP_Y):
			depth[i] = 1
			queue.append(i)
	# BFS across jumpable platform pairs
	while not queue.is_empty():
		var cur: int = queue.pop_front()
		for j in range(n):
			if j == cur or depth.has(j):
				continue
			if _platforms_jumpable(platfs[cur], platfs[j]):
				depth[j] = int(depth[cur]) + 1
				queue.append(j)
	var out: Dictionary = {}
	for i in range(n):
		# Unreachable platforms (rare; _ensure_path bridges most) count as deep
		out[str(platfs[i]["name"])] = int(depth.get(i, 6))
	return out

func _platforms_jumpable(a: Dictionary, b: Dictionary) -> bool:
	var ahw := float(a["width"]) * 0.5
	var bhw := float(b["width"]) * 0.5
	var ax := float(a["cx"])
	var bx := float(b["cx"])
	var hg := 0.0
	if ax + ahw < bx - bhw:
		hg = (bx - bhw) - (ax + ahw)
	elif bx + bhw < ax - ahw:
		hg = (ax - ahw) - (bx + bhw)
	var vg := absf(float(a["top_y"]) - float(b["top_y"]))
	return hg <= MAX_JUMP_GAP and vg <= MAX_JUMP_UP

# A platform is a "gateway" if it can reach a strictly deeper platform — i.e. it
# guards the route to harder-to-reach loot, so it's a natural enemy post.
func _is_gateway(p: Dictionary, platfs: Array, depths: Dictionary) -> bool:
	var my_depth := int(depths.get(str(p["name"]), 6))
	for other in platfs:
		if other["name"] == p["name"]:
			continue
		if int(depths.get(str(other["name"]), 6)) > my_depth and _platforms_jumpable(p, other):
			return true
	return false

# ── Extra platforms for JSW variety and door connectivity ─────────────────────
# Adds 2-4 narrow "shelf" platforms between the main grid columns, and a
# landing ledge just inside any east/west door at the right height.

func _extra_platforms(rng: RandomNumberGenerator, existing: Array, exits: Array) -> Array:
	var extras: Array = []
	var used_xs: Array = []
	for p in existing:
		used_xs.append(int(p["cx"]))

	# 2-3 random shelf platforms at half-column positions and varied heights
	var shelf_count := rng.randi_range(2, 3)
	for i in range(shelf_count):
		var cx := rng.randi_range(-240, 240)
		# Snap to 16 px grid and avoid existing column centres
		cx = (cx / 16) * 16
		var too_close := false
		for ux in used_xs:
			if abs(cx - ux) < 48:
				too_close = true
				break
		if too_close:
			continue
		var level := rng.randi_range(0, LEVEL_COUNT - 1)
		var top_y := _level_top_y(level)
		var w := 48 + (rng.randi_range(0, 2) * 16)  # 48, 64, or 80 px
		extras.append({
			"name":  "Shelf%02d" % i,
			"kind":  "platform",
			"anchor": "top",
			"cx":    cx,
			"cy":    top_y + PLAT_THICK,
			"top_y": top_y,
			"width": w,
			"level": level,
		})
		used_xs.append(cx)

	return extras

# ── Theme ──────────────────────────────────────────────────────────────────────

func _pick_theme(rng: RandomNumberGenerator, room_index: int) -> LayoutTheme:
	if room_index == 0:
		return LayoutTheme.ASCENT
	var themes := [LayoutTheme.ASCENT, LayoutTheme.DESCENT, LayoutTheme.VALLEY, LayoutTheme.PEAK, LayoutTheme.PLATEAU, LayoutTheme.ZIGZAG]
	return themes[rng.randi() % themes.size()]

# ── WFC level assignment ───────────────────────────────────────────────────────
# Constrained random walk: levels[0]=0 (reachable from floor), levels[last]=target.
# Each adjacent pair differs by at most 1  (40 px ≤ MAX_JUMP_UP=64).

func _gen_levels(rng: RandomNumberGenerator, theme: LayoutTheme, target: int) -> Array[int]:
	var levels: Array[int] = []
	levels.resize(COL_COUNT)
	levels[0] = 0
	levels[COL_COUNT - 1] = target

	# Fill middle columns with a theme-biased constrained random walk
	for i in range(1, COL_COUNT - 1):
		var prev  := levels[i - 1]
		var remaining := COL_COUNT - 1 - i
		# Range that still allows us to reach `target` in `remaining` steps
		var lo := maxi(0, target - remaining)
		var hi := mini(LEVEL_COUNT - 1, target + remaining)
		# Also must be within 1 of previous
		lo = maxi(lo, prev - 1)
		hi = mini(hi, prev + 1)

		var bias := _theme_bias(theme, i, target)
		var ideal := clampi(prev + bias + rng.randi_range(-1, 1), lo, hi)
		levels[i] = clampi(ideal, lo, hi)

	return levels

func _theme_bias(theme: LayoutTheme, col: int, target: int) -> int:
	var mid := float(COL_COUNT - 1) * 0.5
	match theme:
		LayoutTheme.ASCENT:
			return 1
		LayoutTheme.DESCENT:
			# Climb fast then drop, re-climb at the end (handled by constraint)
			return 1 if col <= 2 else -1
		LayoutTheme.VALLEY:
			# Climb then dip in middle then climb
			return 1 if float(col) < mid else -1
		LayoutTheme.PEAK:
			# Rise in the middle
			return 1 if float(col) <= mid else -1
		LayoutTheme.PLATEAU:
			# Reach top quickly then stay flat
			return 1 if col <= 2 else 0
		LayoutTheme.ZIGZAG:
			return 1 if col % 2 == 0 else -1
		_:
			return 0

# ── Platform geometry ──────────────────────────────────────────────────────────

func _level_top_y(level: int) -> int:
	return FLOOR_TOP_Y - (level + 1) * LEVEL_STEP

func _levels_to_platforms(levels: Array[int]) -> Array:
	var out: Array = []
	for i in range(levels.size()):
		var top_y := _level_top_y(levels[i])
		var cx: int = COL_X[i]
		out.append({
			"name":  "Plat%s" % char(65 + i),
			"kind":  "platform",
			"anchor": "top",
			"cx":    cx,
			"cy":    top_y + PLAT_THICK,
			"top_y": top_y,
			"width": PLAT_W,
			"level": levels[i],
		})
	return out

func _branch_platforms(rng: RandomNumberGenerator, _exits: Array) -> Array:
	var out: Array = []
	for i in range(5):
		var top_y := _level_top_y(i)
		var side  := (i % 2 == 0)
		var cx: int = -80 if side else 80
		if rng.randf() < 0.35:
			cx = -cx
		out.append({
			"name":  "Shaft%s" % char(65 + i),
			"kind":  "platform",
			"anchor": "top",
			"cx":    cx,
			"cy":    top_y + PLAT_THICK,
			"top_y": top_y,
			"width": 120,
			"level": i,
		})
	return out

# ── Solids (floor + walls + platforms) ────────────────────────────────────────

func _build_solids(platfs: Array, exits: Array, floor_gaps: Array = []) -> Array:
	var solids: Array = []
	# Floor built as segments so archetypes can carve chasms into it
	var fy := FLOOR_Y_CTR - PLAT_THICK
	var idx := 0
	for seg in _floor_segments(floor_gaps, -330, 330):
		var sx0: float = seg[0]
		var sx1: float = seg[1]
		if sx1 - sx0 < 8.0:
			continue
		solids.append({"name": "Floor%d" % idx, "kind": "floor", "anchor": "top",
			"position": [(sx0 + sx1) * 0.5, fy], "size": [sx1 - sx0, 16]})
		idx += 1
	_add_walls(solids, exits)
	# Ceiling at the top
	solids.append({"name": "Ceiling", "kind": "ceiling", "anchor": "bottom",
		"position": [0, -float(ROOM_HALF_H)], "size": [660, 16]})
	for p in platfs:
		solids.append({
			"name":     p["name"],
			"kind":     str(p.get("kind", "platform")),
			"anchor":   str(p.get("anchor", "top")),
			"position": [p["cx"], p["top_y"]],
			"size":     [p["width"], PLAT_THICK],
			"one_way":  true,
		})
	return solids

# Returns the solid x-spans of the floor with the given gaps carved out.
func _floor_segments(gaps: Array, left: float, right: float) -> Array:
	if gaps.is_empty():
		return [[left, right]]
	var sorted := gaps.duplicate()
	sorted.sort_custom(func(a, b): return float(a[0]) < float(b[0]))
	var segs: Array = []
	var cursor := left
	for g in sorted:
		var g0: float = maxf(left, float(g[0]))
		var g1: float = minf(right, float(g[1]))
		if g0 > cursor:
			segs.append([cursor, g0])
		cursor = maxf(cursor, g1)
	if cursor < right:
		segs.append([cursor, right])
	return segs

func _in_gaps(x: float, gaps: Array) -> bool:
	for g in gaps:
		if x >= float(g[0]) and x <= float(g[1]):
			return true
	return false

func _add_walls(solids: Array, exits: Array) -> void:
	for side in ["east", "west", "north", "south"]:
		_wall_segment(solids, side, _find_exit(exits, side))

func _wall_segment(solids: Array, side: String, gap: Dictionary) -> void:
	if side in ["east", "west"]:
		var wx := WALL_X if side == "east" else -WALL_X
		if gap.is_empty():
			solids.append({"name": "%sWall" % side.capitalize(), "kind": "wall", "anchor": "center", "position": [wx, 0], "size": [16, 360]})
			return
		var gp  := _v2(gap.get("position", [0, 0]))
		var gs  := _v2(gap.get("size", [40, 72]))
		var th  := maxf(0.0, (float(ROOM_HALF_H) - gp.y) - gs.y * 0.5)
		var bh  := maxf(0.0, (float(ROOM_HALF_H) + gp.y) - gs.y * 0.5)
		if th > 0.0:
			solids.append({"name": "%sWallTop" % side.capitalize(),
				"kind": "wall", "anchor": "center",
				"position": [wx, -float(ROOM_HALF_H) + th * 0.5], "size": [16.0, th]})
		if bh > 0.0:
			solids.append({"name": "%sWallBot" % side.capitalize(),
				"kind": "wall", "anchor": "center",
				"position": [wx, gp.y + gs.y * 0.5 + bh * 0.5], "size": [16.0, bh]})
	else:
		var wy := -float(ROOM_HALF_H) if side == "north" else float(ROOM_HALF_H)
		if gap.is_empty():
			solids.append({"name": "%sWall" % side.capitalize(), "kind": "wall", "anchor": "center", "position": [0.0, wy], "size": [720.0, 16.0]})
			return
		var gp  := _v2(gap.get("position", [0, 0]))
		var gs  := _v2(gap.get("size", [40, 72]))
		var lw  := maxf(0.0, (float(ROOM_HALF_W) + gp.x) - gs.x * 0.5)
		var rw  := maxf(0.0, (float(ROOM_HALF_W) - gp.x) - gs.x * 0.5)
		if lw > 0.0:
			solids.append({"name": "%sWallL" % side.capitalize(),
				"kind": "wall", "anchor": "center",
				"position": [-float(ROOM_HALF_W) + lw * 0.5, wy], "size": [lw, 16.0]})
		if rw > 0.0:
			solids.append({"name": "%sWallR" % side.capitalize(),
				"kind": "wall", "anchor": "center",
				"position": [gp.x + gs.x * 0.5 + rw * 0.5, wy], "size": [rw, 16.0]})

# ── Graph completion ───────────────────────────────────────────────────────────
# Verify spawn→all platforms connected; insert bridge solids if not.

func _ensure_path(solids: Array, rng: RandomNumberGenerator) -> void:
	var spawn := Vector2(-300.0, 130.0)
	for _attempt in range(8):
		var surfs   := _extract_surfaces(solids)
		var adj     := _build_adj(surfs)
		var root    := _find_support(spawn, surfs)
		if root < 0:
			return
		var reached := _flood(adj, root)
		var unreach := _unreachable_platforms(surfs, reached)
		if unreach.is_empty():
			return
		# Bridge the nearest reachable ↔ unreachable pair
		var bridge := _bridge(surfs, reached, unreach, rng)
		if bridge.is_empty():
			return
		solids.append(bridge)

func _extract_surfaces(solids: Array) -> Array:
	var out: Array = []
	for i in range(solids.size()):
		var s    : Dictionary = solids[i]
		var name := str(s.get("name", "")).to_lower()
		if "wall" in name:
			continue
		var sz := _v2(s.get("size", [0, 0]))
		if sz.x <= 0.0 or sz.y <= 0.0 or sz.x < sz.y:
			continue
		var pos  := _v2(s.get("position", [0, 0]))
		var top  := pos.y - sz.y * 0.5
		out.append({"id": i, "rect": Rect2(pos - sz * 0.5, sz), "top_y": top})
	return out

func _build_adj(surfs: Array) -> Dictionary:
	var adj: Dictionary = {}
	for s in surfs:
		adj[int(s["id"])] = PackedInt32Array()
	for i in range(surfs.size()):
		for j in range(i + 1, surfs.size()):
			var a : Dictionary = surfs[i]
			var b : Dictionary = surfs[j]
			var ra : Rect2 = a["rect"]
			var rb : Rect2 = b["rect"]
			var hg := 0.0
			if ra.end.x < rb.position.x:
				hg = rb.position.x - ra.end.x
			elif rb.end.x < ra.position.x:
				hg = ra.position.x - rb.end.x
			var vg := absf(float(a["top_y"]) - float(b["top_y"]))
			if hg <= MAX_JUMP_GAP and vg <= MAX_JUMP_UP:
				adj[int(a["id"])].append(int(b["id"]))
				adj[int(b["id"])].append(int(a["id"]))
	return adj

func _flood(adj: Dictionary, start: int) -> Dictionary:
	var vis: Dictionary = {}
	var q: Array[int] = [start]
	while not q.is_empty():
		var cur: int = q.pop_front()
		if vis.has(cur):
			continue
		vis[cur] = true
		for nb: int in adj.get(cur, PackedInt32Array()):
			if not vis.has(nb):
				q.append(nb)
	return vis

func _find_support(pt: Vector2, surfs: Array) -> int:
	var best := -1
	var best_y := -INF
	for s in surfs:
		var r   : Rect2 = s["rect"]
		var top : float = s["top_y"]
		if pt.x < r.position.x - 12.0 or pt.x > r.end.x + 12.0:
			continue
		if pt.y < top - 32.0 or pt.y > top + 96.0:
			continue
		if top > best_y:
			best = int(s["id"])
			best_y = top
	return best

func _unreachable_platforms(surfs: Array, reached: Dictionary) -> Array:
	var out: Array = []
	for s in surfs:
		var name := str(s.get("name", ""))
		# Only flag named Plat/Shaft solids (not floor/walls)
		if not reached.has(int(s["id"])) and (name.find("Plat") >= 0 or name.find("Shaft") >= 0):
			out.append(s)
	return out

func _bridge(surfs: Array, reached: Dictionary, unreach: Array, rng: RandomNumberGenerator) -> Dictionary:
	var best_d  := INF
	var best_rx := 0.0
	var best_ry := 0.0
	var best_ux := 0.0
	var best_uy := 0.0
	for s in surfs:
		if not reached.has(int(s["id"])):
			continue
		var ra : Rect2 = s["rect"]
		for u in unreach:
			var rb : Rect2 = u["rect"]
			var cx_a := ra.get_center().x
			var cx_b := rb.get_center().x
			var d := absf(cx_a - cx_b) + absf(float(s["top_y"]) - float(u["top_y"])) * 2.0
			if d < best_d:
				best_d = d
				best_rx = cx_a; best_ry = float(s["top_y"])
				best_ux = cx_b; best_uy = float(u["top_y"])
	if best_d == INF:
		return {}
	var mx := (best_rx + best_ux) * 0.5
	var my := (best_ry + best_uy) * 0.5 + float(PLAT_THICK) * 0.5
	return {"name": "Bridge%02d" % (rng.randi() % 100),
		"position": [mx, my], "size": [PLAT_W, PLAT_THICK], "one_way": true}

# ── Collectibles ───────────────────────────────────────────────────────────────

# Half the on-screen height of each collectible sprite, so it rests ON a surface
# (node origin is centred, so centre = surface - half-height).
func _item_rest_offset(scene: String) -> float:
	match scene:
		ORE_SCENE:     return 12.0   # ruby 32px × 0.75 = 24 → half 12
		BATTERY_SCENE: return 10.0   # yellowball 32px × 0.6 ≈ 19 → half ~10
		FUEL_SCENE:    return 5.0    # fuelrods 16px × 0.6 ≈ 10 → half 5
		AIR_SCENE:     return 4.0    # SpaceMiner 16px × 0.5 = 8 → half 4
	return 8.0

func _place_ore(rng: RandomNumberGenerator, platfs: Array, room_index: int, enemy_plat_names: Array = [], depths: Dictionary = {}, floor_gaps: Array = []) -> Array:
	var items: Array = []
	var idx := 0
	for p in platfs:
		# Skip platforms with enemies — player must navigate around them to reach adjacent items
		if p["name"] in enemy_plat_names:
			continue
		# Difficulty tier = how hard the platform is to reach. Deeper = richer loot.
		var depth := int(depths.get(str(p["name"]), 1))
		var tier := maxi(int(p["level"]), depth - 1)
		var count := 1 + (1 if tier >= 2 else 0) + (1 if tier >= 4 else 0)
		for j in range(count):
			var ox := float(p["cx"]) + float(j) * 20.0 - 10.0
			var roll := rng.randf()
			var scene := ORE_SCENE
			var name_prefix := "Ore"
			if tier >= 3 and roll < 0.25:
				scene = AIR_SCENE
				name_prefix = "Air"
			elif tier >= 2 and roll < 0.15:
				scene = FUEL_SCENE
				name_prefix = "Fuel"
			elif tier >= 2 and roll < 0.08:
				scene = BATTERY_SCENE
				name_prefix = "Bat"
			# Don't place a pickup where the player can't fit to grab it
			if not _has_headroom(ox, float(p["top_y"]), platfs):
				continue
			var oy := float(p["top_y"]) - _item_rest_offset(scene)
			var entry := {"name": "%s_%02d" % [name_prefix, idx],
				"scene": scene, "position": [ox, oy]}
			# Deeper ore is worth more
			if scene == ORE_SCENE and depth >= 2:
				entry["props"] = {"amount": 1 + mini(depth - 1, 4)}
			items.append(entry)
			idx += 1
	for i in range(rng.randi_range(1, 3)):
		var scene := AIR_SCENE if rng.randf() < 0.3 else ORE_SCENE
		var fx := float(rng.randi_range(-280, 280))
		if _in_gaps(fx, floor_gaps):
			continue
		# Skip floor pickups tucked under a low platform with no head-room
		if not _has_headroom(fx, float(FLOOR_TOP_Y), platfs):
			continue
		items.append({"name": "Floor_%02d" % i, "scene": scene,
			"position": [fx, float(FLOOR_TOP_Y) - _item_rest_offset(scene)]})
	return items

# True if the player (32px tall) can stand at x on surface_y without a platform
# directly overhead blocking access to a pickup placed there.
const PICKUP_HEADROOM := 44.0
func _has_headroom(x: float, surface_y: float, platfs: Array) -> bool:
	for p in platfs:
		if absf(x - float(p["cx"])) > float(p["width"]) * 0.5 + 10.0:
			continue
		var p_bottom := float(p["top_y"]) + PLAT_THICK
		# platform sits above the surface, with too little clearance
		if p_bottom <= surface_y and surface_y - p_bottom < PICKUP_HEADROOM:
			return false
	return true

# ── Hazards ────────────────────────────────────────────────────────────────────

func _place_hazards(rng: RandomNumberGenerator, platfs: Array, room_index: int, floor_gaps: Array = []) -> Array:
	if room_index < 2:
		return []
	var hz: Array = []
	var spike_chance := clampf(0.12 + float(room_index) * 0.03, 0.12, 0.55)
	var idx := 0

	for p in platfs:
		if rng.randf() < spike_chance:
			var roll := rng.randf()
			var hx := float(p["cx"]) + rng.randi_range(-24, 24)
			if _in_gaps(hx, floor_gaps):
				continue

			# 70% spikes, 15% fire pipes, 15% moving spikes
			if roll < 0.70:
				hz.append({"name": "Spikes_%d" % idx, "scene": HAZARD_SCENE,
					"position": [hx, float(FLOOR_Y_CTR - PLAT_THICK - 8)]})
			elif roll < 0.85:
				var fire_dirs := ["up", "down", "left", "right"]
				hz.append({"name": "FirePipe_%d" % idx, "scene": FIRE_PIPE_SCENE,
					"position": [hx, float(p["top_y"]) - 24.0],
					"props": {
						"period": rng.randf_range(1.5, 2.5),
						"on_time": rng.randf_range(0.5, 0.9),
						"phase": rng.randf_range(0.0, 2.0),
						"direction": fire_dirs[rng.randi() % fire_dirs.size()],
					}})
			else:
				hz.append({"name": "MovingSpike_%d" % idx, "scene": MOVING_SPIKE_SCENE,
					"position": [hx, float(p["top_y"]) - 32.0],
					"props": {
						"travel": rng.randf_range(48.0, 80.0),
						"speed": rng.randf_range(40.0, 80.0),
						"vertical": rng.randf() < 0.7,
					}})
			idx += 1

	# Acid baths in some floor gaps (chasms)
	for gap in floor_gaps:
		if rng.randf() < 0.3:
			var gap_x := (float(gap[0]) + float(gap[1])) * 0.5
			hz.append({"name": "AcidBath_%d" % idx, "scene": ACID_BATH_SCENE,
				"position": [gap_x, float(FLOOR_Y_CTR - 8)],
				"size": [float(gap[1]) - float(gap[0]), 16.0],
				"anchor": "top"})
			idx += 1

	return hz

# ── Enemies ────────────────────────────────────────────────────────────────────

# Returns [enemies_array, enemy_platform_name_array]
func _place_enemies_tracked(rng: RandomNumberGenerator, platfs: Array, room_index: int, depths: Dictionary = {}) -> Array:
	if room_index < 1:
		return [[], []]
	var enm: Array = []
	var enemy_names: Array = []
	var base_chance := clampf(0.15 + float(room_index) * 0.025, 0.15, 0.55)
	var idx := 0
	# Daleks patrol platforms (gateways get a higher chance)
	for p in platfs:
		if _is_door_landing_platform(p):
			continue
		var chance := base_chance
		if _is_gateway(p, platfs, depths):
			chance = minf(0.8, base_chance + 0.35)
		if rng.randf() < chance:
			var hunter := room_index >= 4 and rng.randf() < 0.4
			enm.append({
				"name":  "Dalek_%d" % idx,
				"scene": DALEK_SCENE,
				"position": [float(p["cx"]), float(p["top_y"]) - 12.0],
				"scale":    [1.0, 1.0],
				"props": {
					"span":    maxf(32.0, float(p["width"]) * 0.5),
					"speed":   34.0 + float(room_index) * 1.5,
					"variant": "hunter" if hunter else "patrol",
				},
			})
			enemy_names.append(p["name"])
			idx += 1
	# UFOs fly in the open air above the floor
	var ufo_count := clampi(int(float(room_index) * 0.12), 0, 3)
	var variants := ["drifter", "diver", "speeder"]
	for u in range(ufo_count):
		var ux := float(rng.randi_range(-220, 220))
		var uy := float(_level_top_y(rng.randi_range(2, LEVEL_COUNT - 1)) - 20)
		enm.append({
			"name":  "Ufo_%d" % u,
			"scene": UFO_SCENE,
			"position": [ux, uy],
			"scale":    [1.0, 1.0],
			"props": {
				"span":    float(rng.randi_range(90, 200)),
				"speed":   46.0 + float(room_index) * 1.5,
				"variant": variants[rng.randi() % variants.size()],
			},
		})
	return [enm, enemy_names]

func _place_mechanisms(rng: RandomNumberGenerator, platfs: Array, room_index: int) -> Array:
	if room_index < 3:
		return []
	var mech: Array = []
	var idx := 0
	var mech_chance := clampf(0.08 + float(room_index) * 0.02, 0.08, 0.35)

	for p in platfs:
		if _is_door_landing_platform(p):
			continue
		if rng.randf() >= mech_chance:
			continue

		var roll := rng.randf()
		if roll < 0.6:
			# 60% conveyor belt (left or right)
			var direction := 1 if rng.randf() < 0.5 else -1
			mech.append({
				"name": "Conveyor_%d" % idx,
				"scene": CONVEYOR_SCENE,
				"position": [float(p["cx"]), float(p["top_y"]) - 8.0],
				"props": {
					"direction": direction,
					"speed": 100.0 + float(room_index) * 10.0,
					"width": float(p["width"]),
				},
			})
		elif roll < 0.85:
			# 25% moving platform (vertical movement)
			var travel_dist := float(rng.randi_range(60, 140))
			mech.append({
				"name": "MovingPlat_%d" % idx,
				"scene": MOVING_PLAT_SCENE,
				"position": [float(p["cx"]), float(p["top_y"]) - 8.0],
				"props": {
					"target_pos": [0.0, -travel_dist],
					"speed": 30.0 + float(room_index) * 3.0,
					"pause_time": 0.5,
				},
			})
		else:
			# 15% sliding wall (gating mechanic)
			var wall_dir := "horizontal" if rng.randf() < 0.5 else "vertical"
			mech.append({
				"name": "SlidingWall_%d" % idx,
				"scene": SLIDING_WALL_SCENE,
				"position": [float(p["cx"]), float(p["top_y"]) - 24.0],
				"props": {
					"direction": wall_dir,
					"slide_distance": 80.0,
					"speed": 50.0,
					"open_time": 2.0,
					"closed_time": 1.0,
				},
			})
		idx += 1

	return mech

func _is_door_landing_platform(p: Dictionary) -> bool:
	var name := str(p.get("name", "")).to_lower()
	return name.find("landing") >= 0 or name.find("door") >= 0 or name.find("exit") >= 0

func _place_enemies(rng: RandomNumberGenerator, platfs: Array, room_index: int) -> Array:
	return _place_enemies_tracked(rng, platfs, room_index)[0]

# ── Tile layers ────────────────────────────────────────────────────────────────

func _build_tiles(platfs: Array, room_index: int, floor_gaps: Array = []) -> Array:
	var layers: Array = []
	var floor_col     := _room_color(room_index)

	# Floor tile row: floor solid body.y=160, LevelBuilder shifts shape by -8,
	# so shape top (walkable surface) = 160-16 = 144 = 9×16 → tile row 9.
	var floor_tile_y  := (FLOOR_Y_CTR - PLAT_THICK) / TILE_SZ   # = 9

	# Floor strip — full room width wall-to-wall, minus any carved chasms
	var fc: Array = []
	var wall_tx := WALL_X / TILE_SZ   # = 22
	for i in range(wall_tx * 2):
		var tx := i - wall_tx
		var world_cx := float(tx) * TILE_SZ + TILE_SZ * 0.5
		if _in_gaps(world_cx, floor_gaps):
			continue
		fc.append({"x": tx, "y": floor_tile_y, "tile": [175, 176, 177, 178][i % 4]})
	layers.append({"name": "FloorTiles", "modulate": floor_col, "z_index": 1, "cells": fc})

	# Roof strip — mirrors the floor wall-to-wall so the room is enclosed on the
	# top edge as well (matching the side walls and floor border).
	var roof_tile_y := -(ROOM_HALF_H / TILE_SZ) - 1   # = -13, aligned with wall tops
	var rc: Array = []
	for i in range(wall_tx * 2):
		var rtx := i - wall_tx
		rc.append({"x": rtx, "y": roof_tile_y, "tile": [175, 176, 177, 178][i % 4]})
	layers.append({"name": "RoofTiles", "modulate": floor_col, "z_index": 1, "cells": rc})

	# Platform strips — tile row matches the collision surface precisely.
	# body.y = top + 16; shape offset = -8; shape_top = body.y - 16 = top.
	# tile_row = top / TILE_SZ.
	var pc: Array = []
	for p in platfs:
		var cy    := int(p["cy"])
		var ty    := (cy - PLAT_THICK) / TILE_SZ
		var tx0   := (int(p["cx"]) - int(p["width"]) / 2) / TILE_SZ
		var count := int(p["width"]) / TILE_SZ
		for j in range(count):
			# Use the solid block tiles (same family as the floor) so platforms
			# read as solid ledges instead of dashed outlines
			pc.append({"x": tx0 + j, "y": ty, "tile": [175, 176, 177, 178][j % 4]})
	if not pc.is_empty():
		# Cool steel tint so platforms are clearly distinct from the coloured floor
		layers.append({"name": "PlatTiles", "modulate": "#9FB4D8", "z_index": 2, "cells": pc})

	# Wall accents — full height, at actual wall tile column (±22).
	var wc: Array = []
	var top_ty := -(ROOM_HALF_H / TILE_SZ) - 1   # -13: one above the room top
	for ty in range(top_ty, floor_tile_y + 1):
		wc.append({"x": -wall_tx, "y": ty, "tile": 225 if ty % 2 == 0 else 226})
		wc.append({"x":  wall_tx, "y": ty, "tile": 225 if ty % 2 == 0 else 226})
	layers.append({"name": "WallTiles", "modulate": "#F4F4F4", "z_index": 2, "cells": wc})

	return layers

func _room_color(room_index: int) -> String:
	var palette := ["#8cff00", "#00D7FF", "#FF8C00", "#FF00DC", "#00FFB3", "#FFE600",
					"#FF4444", "#AA88FF"]
	return palette[room_index % palette.size()]

# ── Decor ──────────────────────────────────────────────────────────────────────

func _build_decor(_rng: RandomNumberGenerator, _room_index: int) -> Array:
	return []

# ── Helpers ────────────────────────────────────────────────────────────────────

func _exit_target_level(exits: Array, direction: String) -> int:
	var ex := _find_exit(exits, direction)
	if ex.is_empty():
		return 3
	var pos := _v2(ex.get("position", [0, 0]))
	var sz  := _v2(ex.get("size", [40, 72]))
	# Support top = exit_pos.y + exit_h/2 + 8 - plat_h/2
	var sup_top := pos.y + sz.y * 0.5 + 8.0 - float(PLAT_THICK) * 0.5
	# Find nearest level
	var best_lv  := 0
	var best_err := INF
	for lv in range(LEVEL_COUNT):
		var err := absf(float(_level_top_y(lv)) - sup_top)
		if err < best_err:
			best_err = err
			best_lv  = lv
	return best_lv

func _find_exit(exits: Array, direction: String) -> Dictionary:
	for ev in exits:
		var e: Dictionary = ev
		if str(e.get("direction", "")) == direction:
			return e
	return {}

func _v2(val) -> Vector2:
	if val is Vector2:
		return val
	if val is Array and val.size() >= 2:
		return Vector2(float(val[0]), float(val[1]))
	return Vector2.ZERO
