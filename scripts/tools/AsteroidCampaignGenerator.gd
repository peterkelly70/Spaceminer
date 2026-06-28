extends RefCounted
class_name AsteroidCampaignGenerator

const LEVEL_SOLVABILITY_VALIDATOR_SCRIPT := preload("res://scripts/tools/LevelSolvabilityValidator.gd")
const ROOM_LAYOUT_GENERATOR_SCRIPT       := preload("res://scripts/tools/RoomLayoutGenerator.gd")

const TOTAL_ROOMS := 56
const MAIN_PATH_ROOMS := 20
const ROOM_SIZE := Vector2(720, 400)
const ROOM_HALF := Vector2(360, 200)
const DWARF_SCENE := "res://scenes/prototype/DwarfMiner.tscn"
const FLOOR_SURFACE_Y := 144   # matches RoomLayoutGenerator floor surface
const DWARF_LINES := [
	"Arr! Another spaceman in me tunnels. Mind the spikes, lad!",
	"Dig deep, dig safe. The ore's richer the further ye go.",
	"Watch yer oxygen out here. These rocks don't breathe for ye.",
	"I lost me pickaxe three caverns back. If ye see it, it's mine!",
	"Them floatin' drones? Give 'em a wide berth, spaceman.",
]
const SAVE_ROOT := "user://saves/spaceminer/campaigns"

const MAIN_ROOM_NAMES := [
	"Landing Bay Alpha",
	"Central Cavern",
	"Ore Stitch",
	"Pressure Spine",
	"Relay Walk",
	"Magnet Hall",
	"Forked Shaft",
	"Echo Gallery",
	"Lift Pocket",
	"Blue Channel",
	"Crystal Spur",
	"Vent Traverse",
	"Fork Lift",
	"Core Bypass",
	"Furnace Run",
	"Shuttle Cut",
	"Key Vault",
	"Terminal Ridge",
	"Escape Spine",
	"Exit Chamber"
]

const BRANCH_ATTACHMENTS := [3, 5, 7, 9, 11, 13, 14, 16, 17]
const BRANCH_LENGTHS := [5, 4, 4, 4, 4, 4, 4, 4, 3]
const BRANCH_DIRECTIONS := ["north", "south", "north", "south", "north", "south", "north", "south", "north"]
const BRANCH_GATES := ["jetpack", "grappling_hook", "cargo_key", "magnetic_boots", "laser_pistol", "visibility_cloak", "", "", ""]
const EQUIPMENT_PICKUPS := {
	2: "jetpack",
	4: "grappling_hook",
	6: "cargo_key",
	8: "magnetic_boots",
	10: "laser_pistol",
	12: "visibility_cloak",
	14: "shield",
}
# Critical path rooms that act as Metroid-style resupply save points
const RESUPPLY_ROOM_INDICES := [4, 9, 14]

func generate_campaign(seed_text: String, room_count: int = TOTAL_ROOMS) -> Dictionary:
	var seed := seed_text.strip_edges()
	if seed.is_empty():
		seed = _generate_seed()

	var rng := RandomNumberGenerator.new()
	rng.seed = _seed_to_int(seed)
	var validator := LEVEL_SOLVABILITY_VALIDATOR_SCRIPT.new()

	var campaign_slug := _slugify(seed)
	var campaign_dir := SAVE_ROOT.path_join(campaign_slug)
	var room_dir := campaign_dir.path_join("rooms")
	DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(room_dir))

	var graph := _build_graph(seed, rng)
	var layout_gen := ROOM_LAYOUT_GENERATOR_SCRIPT.new()
	var room_files: Dictionary = {}
	var room_summaries: Dictionary = {}

	for room_id in graph["room_order"]:
		var room_info: Dictionary = graph["rooms"][room_id]
		var room_data := _build_room_data(room_info, graph, rng, layout_gen)
		var validation_result: Dictionary = validator.validate_level(room_data, {"validate_reachability": false})
		if not bool(validation_result.get("ok", true)):
			push_warning("AsteroidCampaignGenerator: %s has validation issues: %s" % [room_id, ", ".join(PackedStringArray(validation_result.get("errors", [])))])
		var room_path := room_dir.path_join("%s.json" % room_id)
		_write_json(room_path, room_data)
		room_files[room_id] = room_path
		room_summaries[room_id] = {
			"room_id": room_id,
			"room_name": str(room_data.get("room_name", room_id)),
			"role": str(room_info.get("role", "critical")),
			"depth": int(room_info.get("depth", 0)),
			"branch_id": int(room_info.get("branch_id", -1)),
			"branch_step": int(room_info.get("branch_step", -1)),
			"branch_direction": str(room_info.get("branch_direction", "")),
			"map_pos": room_info.get("map_pos", [0, 0]),
			"is_resupply": bool(room_info.get("is_resupply", false)),
			"file_path": room_path,
			"validation_ok": bool(validation_result.get("ok", true)),
			"validation_errors": validation_result.get("errors", []),
		}

	var campaign_validation := _validate_campaign_graph(graph)
	if not bool(campaign_validation.get("ok", true)):
		push_warning("AsteroidCampaignGenerator: campaign graph validation failed: %s" % ", ".join(PackedStringArray(campaign_validation.get("errors", []))))

	var manifest := {
		"seed": seed,
		"campaign_name": "Asteroid 56",
		"room_count": room_count,
		"main_path_rooms": MAIN_PATH_ROOMS,
		"start_room_id": "room_000",
		"exit_room_id": "room_019",
		"campaign_dir": campaign_dir,
		"manifest_path": campaign_dir.path_join("manifest.json"),
		"room_files": room_files,
		"rooms": room_summaries,
		"map_links": graph["map_links"],
		"critical_path": graph["critical_path"],
		"required_equipment": ["jetpack", "grappling_hook", "cargo_key", "magnetic_boots", "laser_pistol", "visibility_cloak", "shield"],
		"campaign_validation": campaign_validation,
	}

	_write_json(str(manifest["manifest_path"]), manifest)
	return manifest

func get_room_data(manifest: Dictionary, room_id: String) -> Dictionary:
	var room_files: Dictionary = manifest.get("room_files", {})
	var room_path := str(room_files.get(room_id, ""))
	if room_path.is_empty():
		return {}
	var text := FileAccess.get_file_as_string(room_path)
	if text.is_empty():
		return {}
	var parsed: Variant = JSON.parse_string(text)
	if typeof(parsed) != TYPE_DICTIONARY:
		return {}
	return parsed

func _build_graph(seed: String, rng: RandomNumberGenerator) -> Dictionary:
	var rooms: Dictionary = {}
	var room_order: Array[String] = []
	var critical_path: Array[String] = []
	var map_links: Array = []

	for i in range(MAIN_PATH_ROOMS):
		var room_id := _room_id(i)
		rooms[room_id] = {
			"room_id": room_id,
			"index": i,
			"role": "critical",
			"is_resupply": i in RESUPPLY_ROOM_INDICES,
			"depth": i,
			"branch_id": -1,
			"branch_step": -1,
			"branch_direction": "",
			"map_pos": [i * 3, 0],
			"doors": [],
			"pickups": [],
		}
		room_order.append(room_id)
		critical_path.append(room_id)

	var next_room_index := MAIN_PATH_ROOMS
	for branch_id in range(BRANCH_ATTACHMENTS.size()):
		var attach_index := int(BRANCH_ATTACHMENTS[branch_id])
		var attach_room_id := _room_id(attach_index)
		var direction := str(BRANCH_DIRECTIONS[branch_id])
		var gate := str(BRANCH_GATES[branch_id])
		var length := int(BRANCH_LENGTHS[branch_id])
		var parent_room_id := attach_room_id
		for step in range(length):
			var room_id := _room_id(next_room_index)
			rooms[room_id] = {
				"room_id": room_id,
				"index": next_room_index,
				"role": "branch",
				"depth": attach_index + step + 1,
				"branch_id": branch_id,
				"branch_step": step,
				"branch_direction": direction,
				"map_pos": [attach_index * 3, (step + 1) * (-1 if direction == "north" else 1)],
				"doors": [],
				"pickups": [],
			}
			room_order.append(room_id)
			_add_link(rooms, parent_room_id, room_id, direction, gate if step == 0 else "")
			map_links.append({
				"from": parent_room_id,
				"to": room_id,
				"direction": direction,
				"requires": [gate] if step == 0 and not gate.is_empty() else [],
			})
			parent_room_id = room_id
			next_room_index += 1

	for i in range(MAIN_PATH_ROOMS - 1):
		var from_id := _room_id(i)
		var to_id := _room_id(i + 1)
		_add_link(rooms, from_id, to_id, "east", "")
		map_links.append({
			"from": from_id,
			"to": to_id,
			"direction": "east",
			"requires": [],
		})

	for room_id in rooms.keys():
		var room_index := int((rooms[room_id] as Dictionary).get("index", 0))
		if EQUIPMENT_PICKUPS.has(room_index):
			var pickup_id := str(EQUIPMENT_PICKUPS[room_index])
			(rooms[room_id] as Dictionary)["pickups"].append({
				"item_id": pickup_id,
				"display_name": pickup_id.replace("_", " "),
				"position": _pickup_position_for_room(room_id, rooms[room_id]),
				"scene": "res://scenes/prototype/EquipmentPickup.tscn",
			})

	return {
		"rooms": rooms,
		"room_order": room_order,
		"critical_path": critical_path,
		"map_links": map_links,
	}

func _add_link(rooms: Dictionary, from_room_id: String, to_room_id: String, direction: String, requirement: String) -> void:
	var from_room: Dictionary = rooms[from_room_id]
	var to_room: Dictionary = rooms[to_room_id]
	from_room["doors"].append(_make_door(from_room_id, to_room_id, direction, requirement, true))
	to_room["doors"].append(_make_door(to_room_id, from_room_id, _opposite_direction(direction), "", false))
	rooms[from_room_id] = from_room
	rooms[to_room_id] = to_room

func _make_door(from_room_id: String, target_room_id: String, direction: String, requirement: String, is_forward: bool) -> Dictionary:
	var position := _door_position(direction, is_forward)
	var door_label := "Exit" if target_room_id == "campaign_complete" else target_room_id.replace("room_", "R")
	var requires: Array[String] = []
	if not requirement.is_empty():
		requires.append(requirement)
	door_label = door_label if is_forward else "Back"
	if target_room_id == "campaign_complete":
		door_label = "Exit"
	return {
		"name": "%s_to_%s" % [from_room_id, target_room_id],
		"label": door_label,
		"kind": "door",
		"anchor": "center",
		"direction": direction,
		"position": position,
		"size": [40, 72],
		"target_room_id": target_room_id,
		"requires": requires,
		"scene": "res://scenes/prototype/DoorZone.tscn",
	}

func _build_room_data(room_info: Dictionary, graph: Dictionary, rng: RandomNumberGenerator, layout_gen: RefCounted) -> Dictionary:
	var room_id := str(room_info.get("room_id", "room_000"))
	var role := str(room_info.get("role", "critical"))
	var index := int(room_info.get("index", 0))
	var exits: Array = []
	for door_variant in room_info.get("doors", []):
		var door: Dictionary = door_variant
		exits.append(door.duplicate(true))
	if room_id == "room_019":
		exits.append({
			"name": "campaign_exit",
			"label": "EXIT",
			"direction": "east",
			"position": [320, -56],
			"size": [40, 72],
			"target_room_id": "campaign_complete",
			"requires": [],
			"scene": "res://scenes/prototype/DoorZone.tscn",
		})

	var layout := _build_layout(role, room_info, exits, rng, layout_gen)
	var support_solids := _make_exit_supports(exits)
	layout["solids"].append_array(support_solids)
	var resolved_pickups := _resolve_pickups(room_info.get("pickups", []), layout["solids"], room_info)
	var room_name := _room_name_for(room_info, index)
	return {
		"room_id":    room_id,
		"name":       room_name,
		"room_name":  room_name,
		"is_resupply": bool(room_info.get("is_resupply", false)),
		"spawn":      layout["spawn"],
		"spawns":     _build_spawns(exits),
		"room_size":  [ROOM_SIZE.x, ROOM_SIZE.y],
		"tile_layers": layout["tile_layers"],
		"solids":     layout["solids"],
		"ladders":    layout.get("ladders", []),
		"npcs":       _build_npcs(role, index, room_info, rng),
		"decor":      layout["decor"],
		"collectibles": layout["collectibles"],
		"pickups":    resolved_pickups,
		"hazards":    layout["hazards"],
		"enemies":    layout["enemies"],
		"exits":      exits,
	}

func _build_layout(role: String, room_info: Dictionary, exits: Array, rng: RandomNumberGenerator, layout_gen: RefCounted) -> Dictionary:
	var index := int(room_info.get("index", 0))
	# Seed a per-room RNG so room layout is deterministic from (campaign_seed, room_index)
	var room_rng := RandomNumberGenerator.new()
	room_rng.seed = rng.seed ^ (index * 2654435761)
	if role == "branch":
		return layout_gen.generate_branch(room_rng, index, exits)
	if bool(room_info.get("is_resupply", false)):
		return layout_gen.generate_resupply(room_rng, index, exits)
	return layout_gen.generate_main(room_rng, index, exits)

# Place a dwarf miner NPC in roughly every third non-resupply room, standing on
# the floor away from the spawn corner.
func _build_npcs(role: String, index: int, room_info: Dictionary, rng: RandomNumberGenerator) -> Array:
	if role == "branch" or bool(room_info.get("is_resupply", false)):
		return []
	if index == 0 or index % 3 != 0:
		return []
	var dwarf_x := float(rng.randi_range(40, 240))
	return [{
		"scene": DWARF_SCENE,
		"name": "Dwarf",
		"position": [dwarf_x, float(FLOOR_SURFACE_Y - 16)],
		"props": {
			"dialogue": str(DWARF_LINES[rng.randi() % DWARF_LINES.size()]),
		},
	}]

func _make_exit_supports(exits: Array) -> Array:
	var supports: Array = []
	for exit_variant in exits:
		var exit_data: Dictionary = exit_variant
		var pos := _as_vec2(exit_data.get("position", [0, 0]))
		var sz  := _as_vec2(exit_data.get("size", [40, 72]))
		var dir := str(exit_data.get("direction", "east"))
		var plat: Dictionary
		match dir:
			"east":
				# Ledge just inside the east wall; player drops onto it from above
				plat = {"name": "%s_landing" % str(exit_data.get("name", "Exit")),
					"position": [ROOM_HALF.x - 40.0, pos.y + sz.y * 0.5 + 8.0],
					"size": [64.0, 16.0], "one_way": true, "type": "platform"}
			"west":
				plat = {"name": "%s_landing" % str(exit_data.get("name", "Exit")),
					"position": [-ROOM_HALF.x + 40.0, pos.y + sz.y * 0.5 + 8.0],
					"size": [64.0, 16.0], "one_way": true, "type": "platform"}
			"north":
				# Ledge just below the north opening
				plat = {"name": "%s_landing" % str(exit_data.get("name", "Exit")),
					"position": [pos.x, -ROOM_HALF.y + 40.0],
					"size": [maxf(64.0, sz.x), 16.0], "one_way": true, "type": "platform"}
			"south":
				# Ledge just above the south opening
				plat = {"name": "%s_landing" % str(exit_data.get("name", "Exit")),
					"position": [pos.x, ROOM_HALF.y - 40.0],
					"size": [maxf(64.0, sz.x), 16.0], "one_way": true, "type": "platform"}
			_:
				plat = {"name": "%s_landing" % str(exit_data.get("name", "Exit")),
					"position": [pos.x, pos.y + sz.y * 0.5 + 8.0],
					"size": [64.0, 16.0], "one_way": true, "type": "platform"}
		supports.append(plat)
	return supports

# Returns per-direction spawn points — player starts on the landing platform
# of whichever door they entered through.
func _build_spawns(exits: Array) -> Dictionary:
	var spawns := {}
	for exit_variant in exits:
		var exit_data: Dictionary = exit_variant
		var pos := _as_vec2(exit_data.get("position", [0, 0]))
		var sz  := _as_vec2(exit_data.get("size", [40, 72]))
		var dir := str(exit_data.get("direction", ""))
		if dir.is_empty():
			continue
		match dir:
			"east":
				# Player stands on east landing platform
				spawns["east"] = [ROOM_HALF.x - 40.0, pos.y + sz.y * 0.5 - 8.0]
			"west":
				spawns["west"] = [-ROOM_HALF.x + 40.0, pos.y + sz.y * 0.5 - 8.0]
			"north":
				spawns["north"] = [pos.x, -ROOM_HALF.y + 24.0]
			"south":
				spawns["south"] = [pos.x, ROOM_HALF.y - 56.0]
	return spawns

func _build_main_layout(solids: Array, decor: Array, collectibles: Array, hazards: Array, enemies: Array, tile_layers: Array, room_info: Dictionary, exits: Array, rng: RandomNumberGenerator) -> void:
	var floor_y := 148
	var left_gap := _find_exit(exits, "west")
	var east_gap := _find_exit(exits, "east")
	var north_gap := _find_exit(exits, "north")
	var south_gap := _find_exit(exits, "south")

	_add_wall_segments(solids, "west", left_gap, floor_y)
	_add_wall_segments(solids, "east", east_gap, floor_y)
	_add_wall_segments(solids, "north", north_gap, floor_y)
	_add_wall_segments(solids, "south", south_gap, floor_y)

	solids.append_array([
		{"name": "Floor", "position": [0, floor_y], "size": [660, 16]},
		{"name": "StepA", "position": [-220, 96], "size": [120, 16]},
		{"name": "StepB", "position": [-100, 56], "size": [120, 16]},
		{"name": "StepC", "position": [20, 16], "size": [120, 16]},
		{"name": "StepD", "position": [140, -24], "size": [120, 16]},
		{"name": "StepE", "position": [260, -64], "size": [120, 16]},
	])

	collectibles.append_array(_make_ore_line([-240, 96], 4))
	collectibles.append_array(_make_ore_line([-60, 56], 2))
	if room_info.get("index", 0) % 3 == 0:
		hazards.append({"name": "Spikes", "scene": "res://scenes/prototype/HazardZone.tscn", "position": [120, 132]})
	if int(room_info.get("index", 0)) % 2 == 0:
		enemies.append(_make_patrol_drone([[-20, 0], [60, 0], [60, 20], [-20, 20]], Vector2(60, 16), 0.7))

	tile_layers.append(_tile_strip_layer("FloorTiles", "#8cff00", 1, -16, floor_y, 33, [175, 176, 177, 178, 179]))
	tile_layers.append(_tile_strip_layer("StepTiles", "#F4F4F4", 2, -240, 96, 30, [170, 171, 172, 173, 174]))
	tile_layers.append(_wall_tile_layer("WallTiles", "#F4F4F4", 2, -15, 0, 14, 13))

	decor.append({"type": "sprite", "name": "StalactiteA", "texture": "res://assets/images/willyinspace/cave_stalTite_0006.png", "position": [-160, -164], "scale": [1.0, 1.0]})
	decor.append({"type": "sprite", "name": "StalactiteB", "texture": "res://assets/images/willyinspace/cave_stalmite_0005.png", "position": [140, -164], "scale": [1.0, 1.0]})

func _build_vertical_layout(solids: Array, decor: Array, collectibles: Array, hazards: Array, enemies: Array, tile_layers: Array, room_info: Dictionary, exits: Array, rng: RandomNumberGenerator) -> void:
	var top_gap := _find_exit(exits, "north")
	var bottom_gap := _find_exit(exits, "south")
	_add_wall_segments(solids, "west", null, 0)
	_add_wall_segments(solids, "east", null, 0)
	_add_wall_segments(solids, "north", top_gap, 0)
	_add_wall_segments(solids, "south", bottom_gap, 0)

	solids.append_array([
		{"name": "Floor", "position": [0, 148], "size": [380, 16]},
		{"name": "ShaftA", "position": [0, 84], "size": [160, 16]},
		{"name": "ShaftB", "position": [0, 36], "size": [160, 16]},
		{"name": "ShaftC", "position": [0, -12], "size": [160, 16]},
		{"name": "ShaftD", "position": [0, -60], "size": [160, 16]},
		{"name": "ShaftE", "position": [0, -108], "size": [160, 16]},
	])

	collectibles.append_array(_make_ore_line([-44, 84], 2))
	collectibles.append_array(_make_ore_line([44, -12], 2))
	if int(room_info.get("branch_step", 0)) % 2 == 0:
		hazards.append({"name": "Spikes", "scene": "res://scenes/prototype/HazardZone.tscn", "position": [0, 136]})
	enemies.append(_make_patrol_drone([[-20, 0], [20, 0], [20, 20], [-20, 20]], Vector2(0, -20), 0.7))

	tile_layers.append(_tile_strip_layer("FloorTiles", "#00D7FF", 1, -10, 148, 19, [175, 176, 177, 178, 179]))
	tile_layers.append(_tile_strip_layer("ShaftTiles", "#F4F4F4", 2, -80, 84, 9, [170, 171, 172, 173, 174]))
	tile_layers.append(_wall_tile_layer("WallTiles", "#F4F4F4", 2, -15, 0, 14, 13))

	decor.append({"type": "sprite", "name": "Cable", "texture": "res://assets/images/willyinspace/cave_half_ledge.png", "position": [-120, -160], "scale": [0.8, 0.8]})

func _make_ore_line(base: Array, count: int) -> Array:
	var result: Array = []
	for i in range(count):
		result.append({
			"name": "Ore_%d" % i,
			"scene": "res://scenes/prototype/OreFragment.tscn",
			"position": [int(base[0]) + (i * 24), int(base[1]) - (i % 2) * 8],
		})
	return result

func _make_patrol_drone(points: Array, base_position: Vector2, scale_factor: float) -> Dictionary:
	return {
		"name": "PatrolDrone",
		"scene": "res://scenes/units/PatrolDrone.tscn",
		"position": [base_position.x, base_position.y],
		"scale": [scale_factor, scale_factor],
		"props": {
			"path_points": points,
			"speed": 42.0,
			"pause_time": 0.25,
		},
	}

func _tile_strip_layer(name: String, modulate: String, z_index: int, start_x: int, y: int, count: int, tiles: Array[int]) -> Dictionary:
	var cells: Array = []
	for i in range(count):
		cells.append({
			"x": start_x + i,
			"y": int(round(float(y) / 20.0)),
			"tile": tiles[i % tiles.size()],
		})
	return {
		"name": name,
		"modulate": modulate,
		"z_index": z_index,
		"cells": cells,
	}

func _wall_tile_layer(name: String, modulate: String, z_index: int, x: int, start_y: int, top_count: int, bottom_count: int) -> Dictionary:
	var cells: Array = []
	for i in range(top_count):
		cells.append({"x": x, "y": i - 6, "tile": 225 if i % 2 == 0 else 226})
		cells.append({"x": -x, "y": i - 6, "tile": 225 if i % 2 == 0 else 226})
	return {
		"name": name,
		"modulate": modulate,
		"z_index": z_index,
		"cells": cells,
	}

func _add_wall_segments(solids: Array, side: String, gap_exit: Variant, _floor_y: int) -> void:
	var wall_x := 352 if side == "east" else -352 if side == "west" else 0
	var wall_y := 0
	var wall_height := 360
	var wall_width := 16
	if side == "east" or side == "west":
		if gap_exit is Dictionary and not gap_exit.is_empty():
			var gap_pos := _as_vec2(gap_exit.get("position", [0, 0]))
			var gap_size := _as_vec2(gap_exit.get("size", [40, 72]))
			var top_height := maxf(0.0, (ROOM_HALF.y - gap_pos.y) - (gap_size.y * 0.5))
			var bottom_height := maxf(0.0, (ROOM_HALF.y + gap_pos.y) - (gap_size.y * 0.5))
			if top_height > 0.0:
				solids.append({"name": "%sWallTop" % side.capitalize(), "kind": "wall", "anchor": "center", "position": [wall_x, -ROOM_HALF.y + top_height * 0.5], "size": [wall_width, top_height]})
			if bottom_height > 0.0:
				solids.append({"name": "%sWallBottom" % side.capitalize(), "kind": "wall", "anchor": "center", "position": [wall_x, gap_pos.y + (gap_size.y * 0.5) + (bottom_height * 0.5)], "size": [wall_width, bottom_height]})
			return
		solids.append({"name": "%sWall" % side.capitalize(), "kind": "wall", "anchor": "center", "position": [wall_x, wall_y], "size": [wall_width, wall_height]})
		return

	if side == "north" or side == "south":
		if gap_exit is Dictionary and not gap_exit.is_empty():
			var gap_pos := _as_vec2(gap_exit.get("position", [0, 0]))
			var gap_size := _as_vec2(gap_exit.get("size", [40, 72]))
			var left_width := maxf(0.0, (ROOM_HALF.x + gap_pos.x) - (gap_size.x * 0.5))
			var right_width := maxf(0.0, (ROOM_HALF.x - gap_pos.x) - (gap_size.x * 0.5))
			var wall_y_pos := -ROOM_HALF.y if side == "north" else ROOM_HALF.y
			if left_width > 0.0:
				solids.append({"name": "%sWallLeft" % side.capitalize(), "kind": "wall", "anchor": "center", "position": [(-ROOM_HALF.x + left_width * 0.5), wall_y_pos], "size": [left_width, 16]})
			if right_width > 0.0:
				solids.append({"name": "%sWallRight" % side.capitalize(), "kind": "wall", "anchor": "center", "position": [(gap_pos.x + (gap_size.x * 0.5) + (right_width * 0.5)), wall_y_pos], "size": [right_width, 16]})
			return
		solids.append({"name": "%sWall" % side.capitalize(), "kind": "wall", "anchor": "center", "position": [0, wall_y], "size": [ROOM_HALF.x * 2.0, 16]})

func _find_exit(exits: Array, direction: String) -> Dictionary:
	for exit_variant in exits:
		var exit_data: Dictionary = exit_variant
		if str(exit_data.get("direction", "")) == direction:
			return exit_data
	return {}

func _door_position(direction: String, is_forward: bool) -> Array:
	match direction:
		"east":
			return [320, 24]
		"west":
			return [-320, 24]
		"north":
			return [-40 if is_forward else 40, -164]
		"south":
			return [40 if is_forward else -40, 164]
		_:
			return [320, 24]

func _opposite_direction(direction: String) -> String:
	match direction:
		"east":
			return "west"
		"west":
			return "east"
		"north":
			return "south"
		"south":
			return "north"
		_:
			return "west"

func _pickup_position_for_room(room_id: String, room_info: Dictionary) -> Array:
	var index := int(room_info.get("index", 0))
	if index == 14:
		return [-180, 80]
	if room_info.get("role", "critical") == "branch":
		return [0, -20 if int(room_info.get("branch_step", 0)) % 2 == 0 else 60]
	match index % 4:
		0:
			return [-180, 80]
		1:
			return [-20, 40]
		2:
			return [140, 0]
		_:
			return [260, -40]

func _resolve_pickups(pickups: Array, solids: Array, room_info: Dictionary) -> Array:
	var resolved: Array = []
	for pickup_variant in pickups:
		var pickup: Dictionary = pickup_variant.duplicate(true)
		var hint := _as_vec2(pickup.get("position", [0, 0]))
		var support := _find_support_surface_from_solids(hint, solids)
		if not support.is_empty():
			var top_y := float(support["top_y"])
			var rect: Rect2 = support["rect"]
			pickup["position"] = [rect.position.x + rect.size.x * 0.5, top_y - 24.0]
		else:
			pickup["position"] = [hint.x, hint.y]
		resolved.append(pickup)
	return resolved

func _find_support_surface_from_solids(point: Vector2, solids: Array) -> Dictionary:
	var best_surface: Dictionary = {}
	var best_surface_y := INF
	for i in range(solids.size()):
		var solid: Dictionary = solids[i]
		var solid_name := str(solid.get("name", "Solid")).to_lower()
		if solid_name.find("wall") >= 0:
			continue
		var size := _as_vec2(solid.get("size", [0, 0]))
		if size.x <= 0.0 or size.y <= 0.0:
			continue
		if size.x < size.y and not _is_walkable_solid_name(solid_name):
			continue
		var position := _as_vec2(solid.get("position", [0, 0]))
		var anchor := str(solid.get("anchor", ""))
		var top_y := _surface_top_y(position, size, anchor, solid_name)
		var left_x := position.x - (size.x * 0.5)
		var right_x := position.x + (size.x * 0.5)
		if point.x < left_x - 12.0 or point.x > right_x + 12.0:
			continue
		if top_y < point.y - 8.0:
			continue
		if top_y < best_surface_y:
			best_surface = {
				"id": i,
				"top_y": top_y,
				"rect": Rect2(Vector2(left_x, top_y), size),
			}
			best_surface_y = top_y
	return best_surface

func _surface_top_y(position: Vector2, size: Vector2, anchor: String, solid_name: String) -> float:
	match anchor:
		"top", "north", "ceiling":
			return position.y
		"bottom", "south", "floor":
			return position.y - size.y
		"left", "west", "right", "east":
			return position.y - (size.y * 0.5)
		_:
			if size.y > size.x:
				return position.y - (size.y * 0.5)
			if _is_walkable_solid_name(solid_name):
				return position.y - size.y
			return position.y - size.y

func _is_walkable_solid_name(solid_name: String) -> bool:
	return solid_name.find("floor") >= 0 or solid_name.find("step") >= 0 or solid_name.find("ledge") >= 0 or solid_name.find("platform") >= 0 or solid_name.find("pedestal") >= 0 or solid_name.find("bridge") >= 0 or solid_name.find("run") >= 0 or solid_name.find("exit") >= 0

func _room_name_for(room_info: Dictionary, index: int) -> String:
	var role := str(room_info.get("role", "critical"))
	if role == "critical":
		if index >= 0 and index < MAIN_ROOM_NAMES.size():
			return MAIN_ROOM_NAMES[index]
		return "Main Spine %02d" % index
	return "Branch %02d-%02d" % [int(room_info.get("branch_id", 0)) + 1, int(room_info.get("branch_step", 0)) + 1]

func _room_id(index: int) -> String:
	return "room_%03d" % index

func _write_json(path: String, data: Dictionary) -> void:
	var file := FileAccess.open(path, FileAccess.WRITE)
	if not file:
		push_error("AsteroidCampaignGenerator: unable to write %s" % path)
		return
	file.store_string(JSON.stringify(data, "\t"))
	file.close()

func _seed_to_int(seed: String) -> int:
	return abs(seed.hash())

func _slugify(value: String) -> String:
	var result := ""
	for character in value.to_lower():
		if character.is_valid_int() or (character >= "a" and character <= "z"):
			result += character
		else:
			result += "_"
	return result.strip_edges().replace("__", "_")

func _generate_seed() -> String:
	var rng := RandomNumberGenerator.new()
	rng.randomize()
	return "%08d" % int(rng.randi() % 100000000)

func _validate_campaign_graph(graph: Dictionary) -> Dictionary:
	var errors: Array[String] = []
	var rooms: Dictionary = graph.get("rooms", {})
	var room_order: Array = graph.get("room_order", [])
	var start_room_id := "room_000"
	var visited: Dictionary = {}
	var queue: Array[String] = [start_room_id]
	while not queue.is_empty():
		var room_id := str(queue.pop_front())
		if visited.has(room_id):
			continue
		visited[room_id] = true
		var room_info: Dictionary = rooms.get(room_id, {})
		for door_variant in room_info.get("doors", []):
			var door: Dictionary = door_variant
			var target_room_id := str(door.get("target_room_id", ""))
			if target_room_id.is_empty() or target_room_id == "campaign_complete":
				continue
			if not visited.has(target_room_id):
				queue.append(target_room_id)

	for room_id in room_order:
		if not visited.has(str(room_id)):
			errors.append("Room %s is disconnected from the start room" % str(room_id))

	if not room_order.has("room_019"):
		errors.append("Exit room room_019 is missing from the room order")
	elif not visited.has("room_019"):
		errors.append("Exit room room_019 is not reachable from the start room")

	for branch_id in range(BRANCH_ATTACHMENTS.size()):
		var gate := str(BRANCH_GATES[branch_id])
		if gate.is_empty():
			continue
		var attach_index := int(BRANCH_ATTACHMENTS[branch_id])
		var pickup_index := -1
		for key in EQUIPMENT_PICKUPS.keys():
			if str(EQUIPMENT_PICKUPS[key]) == gate:
				pickup_index = int(key)
				break
		if pickup_index < 0:
			errors.append("No pickup was assigned for gate %s" % gate)
		elif pickup_index >= attach_index:
			errors.append("Pickup %s appears too late for branch gate at room_%03d" % [gate, attach_index])

	return {
		"ok": errors.is_empty(),
		"errors": errors,
		"reachable_rooms": visited.keys(),
	}

func _as_vec2(value) -> Vector2:
	if value is Vector2:
		return value
	if value is Array and value.size() >= 2:
		return Vector2(float(value[0]), float(value[1]))
	return Vector2.ZERO
