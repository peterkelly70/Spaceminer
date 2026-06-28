extends RefCounted
class_name RoomLayoutGenerator

# ── Room coordinate system (centre-origin, matching AsteroidCampaignGenerator) ──
const ROOM_HALF_W  := 360
const ROOM_HALF_H  := 200
# Floor solid body.y = 160.  LevelBuilder shifts CollisionShape2D by (0,-8) so
# shape centre = 152, shape top (walkable surface) = 144 = 9 × TILE_SZ.
const FLOOR_Y_CTR  := 160   # floor StaticBody2D position.y
const FLOOR_TOP_Y  := 152   # shape-centre reference (body.y - 8); used in _level_top_y
const WALL_X       := 352
const PLAT_THICK   := 16    # 1 tile
const TILE_SZ      := 16    # tileset cell size in pixels

# Jump physics  (must match LevelSolvabilityValidator)
const MAX_JUMP_UP  := 64.0
const MAX_JUMP_GAP := 96.0

# Platform grid — 6 columns, 5 height levels, all snapped to the 16 px tile grid.
#
# Column centres: ±56, ±168, ±280  (each = 40 + n×112, left/right edges on 16 px boundaries)
#   Left edges:  -320, -208, -96,  16, 128, 240   (all multiples of 16)
#   Right edges: -240, -128, -16,  96, 208, 320
#   Edge gap between adjacent columns: 112 - 80 = 32 px (2 tiles), always jumpable.
#
# Level heights (surface = FLOOR_TOP_Y - (n+1)×LEVEL_STEP):
#   Level 0 → top 120, body centre 136
#   Level 1 → top  88, body centre 104
#   Level 2 → top  56, body centre  72
#   Level 3 → top  24, body centre  40
#   Level 4 → top  -8, body centre   8
const COL_COUNT    := 6
const COL_X        := [-280, -168, -56, 56, 168, 280]
const PLAT_W       := 80    # 5 tiles wide; with centres above, both edges on 16 px grid
const LEVEL_COUNT  := 5     # levels 0..4
const LEVEL_STEP   := 32    # 2 tiles per step

const ORE_SCENE      := "res://scenes/prototype/OreFragment.tscn"
const AIR_SCENE      := "res://scenes/prototype/AirCanister.tscn"
const FUEL_SCENE     := "res://scenes/prototype/FuelCell.tscn"
const BATTERY_SCENE  := "res://scenes/prototype/BatteryPack.tscn"
const STATION_SCENE  := "res://scenes/prototype/ResupplyStation.tscn"
const DRONE_SCENE    := "res://scenes/units/PatrolDrone.tscn"
const HAZARD_SCENE   := "res://scenes/prototype/HazardZone.tscn"

enum LayoutTheme { ASCENT, DESCENT, VALLEY, PEAK, PLATEAU, ZIGZAG }

# ── Public API ─────────────────────────────────────────────────────────────────

func generate_main(rng: RandomNumberGenerator, room_index: int, exits: Array) -> Dictionary:
	var target := _exit_target_level(exits, "east")
	var theme   := _pick_theme(rng, room_index)
	var levels  := _gen_levels(rng, theme, target)
	var platfs  := _levels_to_platforms(levels)
	platfs.append_array(_extra_platforms(rng, platfs, exits))
	var solids  := _build_solids(platfs, exits)
	_ensure_path(solids, rng)
	var enemy_result := _place_enemies_tracked(rng, platfs, room_index)
	return {
		"spawn":        [-300, 130],
		"solids":       solids,
		"decor":        _build_decor(rng, room_index),
		"collectibles": _place_ore(rng, platfs, room_index, enemy_result[1]),
		"hazards":      _place_hazards(rng, platfs, room_index),
		"enemies":      enemy_result[0],
		"tile_layers":  _build_tiles(platfs, room_index),
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
		"spawn":        [-260, 130],
		"solids":       solids,
		"decor":        [],
		"collectibles": pickups,
		"hazards":      [],
		"enemies":      [],
		"tile_layers":  _build_tiles([], room_index),
	}

func generate_branch(rng: RandomNumberGenerator, room_index: int, exits: Array) -> Dictionary:
	var platfs := _branch_platforms(rng, exits)
	var solids := _build_solids(platfs, exits)
	_ensure_path(solids, rng)
	var enemy_result := _place_enemies_tracked(rng, platfs, room_index)
	return {
		"spawn":        [0, 130],
		"solids":       solids,
		"decor":        _build_decor(rng, room_index),
		"collectibles": _place_ore(rng, platfs, room_index, enemy_result[1]),
		"hazards":      _place_hazards(rng, platfs, room_index),
		"enemies":      enemy_result[0],
		"tile_layers":  _build_tiles(platfs, room_index),
	}

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

func _build_solids(platfs: Array, exits: Array) -> Array:
	var solids: Array = []
	solids.append({"name": "Floor", "kind": "floor", "anchor": "top", "position": [0, FLOOR_Y_CTR - PLAT_THICK], "size": [660, 16]})
	_add_walls(solids, exits)
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

func _place_ore(rng: RandomNumberGenerator, platfs: Array, room_index: int, enemy_plat_names: Array = []) -> Array:
	var items: Array = []
	var idx := 0
	for p in platfs:
		# Skip platforms with enemies — player must navigate around them to reach adjacent items
		if p["name"] in enemy_plat_names:
			continue
		var count := 1 + (1 if p["level"] >= 2 else 0)
		for j in range(count):
			var ox := float(p["cx"]) + float(j) * 20.0 - 10.0
			var oy := float(p["top_y"]) - 8.0
			var roll := rng.randf()
			var scene := ORE_SCENE
			var name_prefix := "Ore"
			if p["level"] >= 3 and roll < 0.25:
				scene = AIR_SCENE
				name_prefix = "Air"
			elif p["level"] >= 2 and roll < 0.15:
				scene = FUEL_SCENE
				name_prefix = "Fuel"
			elif p["level"] >= 2 and roll < 0.08:
				scene = BATTERY_SCENE
				name_prefix = "Bat"
			items.append({"name": "%s_%02d" % [name_prefix, idx],
				"scene": scene, "position": [ox, oy]})
			idx += 1
	for i in range(rng.randi_range(1, 3)):
		var scene := AIR_SCENE if rng.randf() < 0.3 else ORE_SCENE
		items.append({"name": "Floor_%02d" % i, "scene": scene,
			"position": [float(rng.randi_range(-280, 280)), float(FLOOR_TOP_Y) - 8.0]})
	return items

# ── Hazards ────────────────────────────────────────────────────────────────────

func _place_hazards(rng: RandomNumberGenerator, platfs: Array, room_index: int) -> Array:
	if room_index < 3:
		return []
	var hz: Array = []
	var chance := minf(0.45, float(room_index) * 0.022)
	for p in platfs:
		if rng.randf() < chance:
			# Offset spike from platform centre so it's a gap hazard, not an item blocker
			var hx := float(p["cx"]) + rng.randi_range(-24, 24)
			hz.append({"name": "Spikes_%s" % p["name"], "scene": HAZARD_SCENE,
				"position": [hx, float(FLOOR_Y_CTR - PLAT_THICK - 8)]})
	return hz

# ── Enemies ────────────────────────────────────────────────────────────────────

# Returns [enemies_array, enemy_platform_name_array]
func _place_enemies_tracked(rng: RandomNumberGenerator, platfs: Array, room_index: int) -> Array:
	if room_index < 2:
		return [[], []]
	var enm: Array = []
	var enemy_names: Array = []
	var chance := minf(0.4, float(room_index) * 0.02)
	for p in platfs:
		if rng.randf() < chance:
			var pw := float(p["width"])
			enm.append({
				"name":  "Drone_%s" % p["name"],
				"scene": DRONE_SCENE,
				"position": [float(p["cx"]) - pw * 0.5, float(p["top_y"]) - 8.0],
				"scale":    [0.7, 0.7],
				"props": {
					"path_points": [[0, 0], [pw, 0]],
					"speed":       35.0 + float(room_index) * 1.5,
					"pause_time":  0.4,
				},
			})
			enemy_names.append(p["name"])
	return [enm, enemy_names]

func _place_enemies(rng: RandomNumberGenerator, platfs: Array, room_index: int) -> Array:
	return _place_enemies_tracked(rng, platfs, room_index)[0]

# ── Tile layers ────────────────────────────────────────────────────────────────

func _build_tiles(platfs: Array, room_index: int) -> Array:
	var layers: Array = []
	var floor_col     := _room_color(room_index)

	# Floor tile row: floor solid body.y=160, LevelBuilder shifts shape by -8,
	# so shape top (walkable surface) = 160-16 = 144 = 9×16 → tile row 9.
	var floor_tile_y  := (FLOOR_Y_CTR - PLAT_THICK) / TILE_SZ   # = 9

	# Floor strip — full room width wall-to-wall (44 tiles: x=-22..21)
	var fc: Array = []
	var wall_tx := WALL_X / TILE_SZ   # = 22
	for i in range(wall_tx * 2):
		fc.append({"x": i - wall_tx, "y": floor_tile_y, "tile": [175, 176, 177, 178][i % 4]})
	layers.append({"name": "FloorTiles", "modulate": floor_col, "z_index": 1, "cells": fc})

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
			pc.append({"x": tx0 + j, "y": ty, "tile": [170, 171, 172, 173][j % 4]})
	if not pc.is_empty():
		layers.append({"name": "PlatTiles", "modulate": "#F4F4F4", "z_index": 2, "cells": pc})

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
