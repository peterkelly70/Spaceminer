extends Node2D
class_name GeneratedAsteroidRoomController

signal request_room_change(next_room_id: String, entry_direction: String)
signal room_status_changed(status: Dictionary)

const ROOM_MODEL_SCRIPT := preload("res://scripts/models/LandingBayAlphaModel.gd")

@export var room_json_path: String = ""

@onready var player: PrototypePlayer = $Player
@onready var respawn_point: Marker2D = $RespawnPoint
@onready var collectibles_root: Node = $Collectibles
@onready var hazards_root: Node = $Hazards
@onready var enemies_root: Node = $Enemies
@onready var pickups_root: Node = $Pickups
@onready var doors_root: Node = $Doors
@onready var level_builder: LevelBuilder = $Level
@onready var room_camera: Camera2D = $RoomCamera

var room_model := ROOM_MODEL_SCRIPT.new()
var current_room_id: String = "room_000"
var current_status := "Door sealed"
var current_tip := "Move: WASD/Arrows  Jump: Space  Thrust: W/Up  Grapple: F  Map: M"
var max_lives: int = 10
var lives_remaining: int = 10
var room_bounds := Rect2(Vector2(-360, -200), Vector2(720, 400))
var _respawning := false
var _room_data: Dictionary = {}
var _fuel_pct: float = 100.0
var _battery_pct: float = 100.0
var _is_resupply: bool = false
var _entry_direction: String = ""

func _ready() -> void:
	room_model.ore_changed.connect(_on_ore_changed)
	room_model.oxygen_changed.connect(_on_oxygen_changed)
	room_model.tip_changed.connect(_on_tip_changed)
	room_model.oxygen_depleted.connect(_on_oxygen_depleted)
	player.player_defeated.connect(_on_player_defeated)

	_load_room_data()
	_build_room()
	_register_collectibles()
	_register_hazards()
	_register_enemies()
	_register_pickups()
	_register_doors()
	room_model.reset()
	_sync_oxygen_from_run_manager()
	room_model.set_tip("Move: WASD/Arrows  Jump: Space  Thrust: W/Up  Grapple: F  Map: M  Pause: P")
	_set_status("Door sealed")
	_emit_room_status()

func _process(delta: float) -> void:
	room_model.drain_oxygen(delta)
	_sync_oxygen_to_run_manager()
	_check_out_of_bounds()

func configure_room(json_path: String, room_id: String, entry_direction: String = "") -> void:
	room_json_path = json_path
	current_room_id = room_id
	_entry_direction = entry_direction

func _load_room_data() -> void:
	if room_json_path.is_empty():
		return
	var text := FileAccess.get_file_as_string(room_json_path)
	if text.is_empty():
		push_error("GeneratedAsteroidRoomController: could not read room file %s" % room_json_path)
		return
	var parsed: Variant = JSON.parse_string(text)
	if typeof(parsed) != TYPE_DICTIONARY:
		push_error("GeneratedAsteroidRoomController: invalid room JSON in %s" % room_json_path)
		return
	_room_data = parsed
	room_model.room_name = str(_room_data.get("room_name", _room_data.get("name", current_room_id)))
	var room_size := _as_vec2(_room_data.get("room_size", [720, 400]))
	room_bounds = Rect2(Vector2(-room_size.x * 0.5, -room_size.y * 0.5), room_size)
	# Lives are a run-level resource — read the persisted value, don't reset per room
	var rm_lives := _get_run_manager()
	if rm_lives and rm_lives.has_method("get_lives"):
		max_lives = int(rm_lives.get_max_lives())
		lives_remaining = int(rm_lives.get_lives())
	else:
		max_lives = int(_room_data.get("lives", 10))
		lives_remaining = max_lives
	_is_resupply = bool(_room_data.get("is_resupply", false))
	_apply_camera_limits(room_size)
	var rm := _get_run_manager()
	if rm and rm.has_method("get_oxygen_pct"):
		room_model.set_oxygen_pct(float(rm.get_oxygen_pct()))
	if rm and rm.has_method("get_fuel_pct"):
		_fuel_pct = float(rm.get_fuel_pct())
		_battery_pct = float(rm.get_battery_pct())

func _build_room() -> void:
	if level_builder and room_json_path:
		level_builder.auto_build = false
		level_builder.build_from_json(room_json_path)
	# Override spawn to match the door the player entered through
	if not _entry_direction.is_empty():
		var spawns: Dictionary = _room_data.get("spawns", {})
		if spawns.has(_entry_direction):
			var sp := _as_vec2(spawns[_entry_direction])
			if player:
				player.position = sp
			if respawn_point:
				respawn_point.position = sp

func _apply_camera_limits(room_size: Vector2) -> void:
	if not room_camera:
		return
	room_camera.limit_left = int(-room_size.x * 0.5)
	room_camera.limit_right = int(room_size.x * 0.5)
	room_camera.limit_top = int(-room_size.y * 0.5)
	room_camera.limit_bottom = int(room_size.y * 0.5)

func _register_collectibles() -> void:
	for child in collectibles_root.get_children():
		var script_path := ""
		var child_script: Script = child.get_script()
		if child_script:
			script_path = str(child_script.resource_path)
		if script_path.ends_with("ore_fragment.gd"):
			room_model.register_ore(child.amount)
			child.collected.connect(_on_ore_collected)
		elif script_path.ends_with("resupply_station.gd"):
			child.activated.connect(_on_resupply_activated)
		elif script_path.ends_with("air_canister.gd"):
			child.collected.connect(_on_air_collected)
		elif script_path.ends_with("fuel_cell.gd"):
			child.collected.connect(_on_fuel_collected)
		elif script_path.ends_with("battery_pack.gd"):
			child.collected.connect(_on_battery_collected)
	# Auto-checkpoint when entering a resupply room
	if _is_resupply:
		_trigger_checkpoint()

func _register_pickups() -> void:
	for child in pickups_root.get_children():
		if child.has_signal("picked_up"):
			child.picked_up.connect(_on_pickup_collected)

func _register_hazards() -> void:
	for child in hazards_root.get_children():
		if child.has_signal("player_hit"):
			child.player_hit.connect(_on_hazard_triggered)

func _register_enemies() -> void:
	for child in enemies_root.get_children():
		if child.has_signal("player_hit"):
			child.connect("player_hit", Callable(self, "_on_hazard_triggered"))

func _register_doors() -> void:
	for child in doors_root.get_children():
		if child.has_signal("door_selected"):
			child.door_selected.connect(_on_door_selected)
		if child.has_signal("door_blocked"):
			child.door_blocked.connect(_on_door_blocked)

func _on_ore_collected(amount: int) -> void:
	room_model.collect_ore(amount)
	var rm := _get_run_manager()
	if rm and rm.has_method("add_ore"):
		rm.add_ore(amount)
	if rm and rm.has_method("add_score"):
		rm.add_score(amount * 100)
	room_model.set_tip("Nice haul!")
	_play_collect_feedback()

func _on_air_collected(amount: float) -> void:
	room_model.current_oxygen = minf(room_model.current_oxygen + room_model.max_oxygen * amount / 100.0, room_model.max_oxygen)
	_sync_oxygen_to_run_manager()
	room_model.set_tip("Air restored!")
	_play_collect_feedback()
	_emit_room_status()

func _on_fuel_collected(amount: float) -> void:
	var fuel_capacity := _get_fuel_capacity()
	_fuel_pct = minf(_fuel_pct + amount, fuel_capacity)
	var rm := _get_run_manager()
	if rm and rm.has_method("update_fuel"):
		rm.update_fuel(_fuel_pct)
	room_model.set_tip("Fuel recovered!")
	_play_collect_feedback()
	_emit_room_status()

func _on_battery_collected(amount: float) -> void:
	_battery_pct = minf(_battery_pct + amount, 100.0)
	var rm := _get_run_manager()
	if rm and rm.has_method("update_battery"):
		rm.update_battery(_battery_pct)
	room_model.set_tip("Battery charged!")
	_play_collect_feedback()
	_emit_room_status()

func _on_resupply_activated(station_type: String) -> void:
	var ore_cost_applied := false
	match station_type:
		"air":
			room_model.current_oxygen = minf(room_model.current_oxygen + 10.0, room_model.max_oxygen)
			_sync_oxygen_to_run_manager()
			room_model.set_tip("AIR +10.")
			ore_cost_applied = true
		"fuel":
			_fuel_pct = minf(_fuel_pct + 10.0, _get_fuel_capacity())
			room_model.set_tip("FUEL +10.")
			ore_cost_applied = true
		"battery":
			_battery_pct = minf(_battery_pct + 10.0, 100.0)
			room_model.set_tip("BATTERY +10.")
			ore_cost_applied = true
		_:
			room_model.current_oxygen = minf(room_model.current_oxygen + 10.0, room_model.max_oxygen)
			_fuel_pct = minf(_fuel_pct + 10.0, 100.0)
			_battery_pct = minf(_battery_pct + 10.0, 100.0)
			room_model.set_tip("All systems +10.")
			ore_cost_applied = true
	if ore_cost_applied:
		var rm_ore := _get_run_manager()
		if rm_ore and rm_ore.has_method("get_ore_count") and rm_ore.has_method("update_ore"):
			rm_ore.update_ore(maxi(int(rm_ore.get_ore_count()) - 1, 0))
	var rm := _get_run_manager()
	if rm and rm.has_method("update_fuel"):
		rm.update_fuel(_fuel_pct)
		rm.update_battery(_battery_pct)
	_play_collect_feedback()
	_emit_room_status()

func _trigger_checkpoint() -> void:
	var rm := _get_run_manager()
	if rm and rm.has_method("save_checkpoint"):
		rm.save_checkpoint(current_room_id)
	if rm and rm.has_method("get_fuel_pct"):
		_fuel_pct = float(rm.get_fuel_pct())
	room_model.set_tip("Checkpoint saved.")
	_set_status("Resupply")

func _on_pickup_collected(item_id: String) -> void:
	var run_manager := _get_run_manager()
	if run_manager and item_id.begins_with("security_") and run_manager.has_method("grant_security_card"):
		run_manager.grant_security_card(item_id)
	elif run_manager and run_manager.has_method("grant_equipment"):
		run_manager.grant_equipment(item_id)
	room_model.set_tip("%s acquired" % item_id.replace("_", " ").to_upper())
	_set_status("Equipment acquired")
	_emit_room_status()

func _on_hazard_triggered() -> void:
	_play_damage_feedback()
	player.handle_hazard_hit()

func _on_door_selected(next_room_id: String) -> void:
	if next_room_id.is_empty():
		return
	room_model.refill_oxygen()
	_sync_oxygen_to_run_manager()
	room_model.set_tip("Pressure door cycled.")
	_set_status("Exit reached")
	var audio_manager := get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("door_open")
	# Find which direction we exited so the next room can spawn player at the matching door
	var exit_dir := _exit_direction_for(next_room_id)
	var entry_dir := _opposite_dir(exit_dir)
	request_room_change.emit(next_room_id, entry_dir)

func _exit_direction_for(target_room_id: String) -> String:
	for exit_variant in _room_data.get("exits", []):
		var ex: Dictionary = exit_variant
		if str(ex.get("target_room_id", "")) == target_room_id:
			return str(ex.get("direction", ""))
	return ""

func _opposite_dir(dir: String) -> String:
	match dir:
		"east":  return "west"
		"west":  return "east"
		"north": return "south"
		"south": return "north"
	return ""

func _on_door_blocked(required_items: Array[String]) -> void:
	if required_items.is_empty():
		_set_status("Door locked")
		room_model.set_tip("Door is locked.")
		return
	var readable: Array[String] = []
	for item_id in required_items:
		readable.append(item_id.replace("_", " "))
	_set_status("Locked")
	room_model.set_tip("Need %s" % ", ".join(readable))

func _on_player_defeated() -> void:
	if _respawning:
		return
	_respawning = true
	_mark_life_lost()
	if lives_remaining <= 0:
		room_model.set_tip("The foot got you. Restart same seed?")
		_set_status("Foot stomped")
		_respawning = false
		return
	room_model.set_tip("Ouch! Respawning...")
	_set_status("Respawning")
	await get_tree().create_timer(0.1).timeout
	player.respawn(respawn_point.global_position)
	room_model.refill_oxygen()
	_sync_oxygen_to_run_manager()
	await get_tree().create_timer(0.75).timeout
	room_model.set_tip("Watch the spikes")
	_set_status("Door sealed")
	await get_tree().create_timer(1.25).timeout
	room_model.set_tip("Move: WASD/Arrows  Jump: Space  Thrust: W/Up  Grapple: F  Map: M")
	_respawning = false

func _on_oxygen_depleted() -> void:
	if _respawning:
		return
	_respawning = true
	_mark_life_lost()
	if lives_remaining <= 0:
		room_model.set_tip("The foot got you. Restart same seed?")
		_set_status("Foot stomped")
		_respawning = false
		return
	room_model.set_tip("Oxygen depleted. Resetting room.")
	_set_status("Oxygen low")
	player.respawn(respawn_point.global_position)
	room_model.refill_oxygen()
	_sync_oxygen_to_run_manager()
	await get_tree().create_timer(0.75).timeout
	room_model.set_tip("Move: WASD/Arrows  Jump: Space  Thrust: W/Up  Grapple: F  Map: M")
	_respawning = false

func _on_ore_changed(_collected: int, _total: int) -> void:
	_emit_room_status()

func _on_oxygen_changed(_current: float, _max_value: float) -> void:
	_emit_room_status()

func _on_tip_changed(text: String) -> void:
	current_tip = text
	_emit_room_status()

func _play_collect_feedback() -> void:
	var audio_manager := get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("collect")

func _play_damage_feedback() -> void:
	var audio_manager := get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("damage")

func _sync_oxygen_from_run_manager() -> void:
	var rm := _get_run_manager()
	if rm and rm.has_method("get_oxygen_pct"):
		room_model.set_oxygen_pct(float(rm.get_oxygen_pct()))

func _sync_oxygen_to_run_manager() -> void:
	var rm := _get_run_manager()
	if rm and rm.has_method("update_oxygen"):
		rm.update_oxygen(room_model.get_oxygen_pct())

func _set_status(text: String) -> void:
	current_status = text
	_emit_room_status()

func _mark_life_lost() -> void:
	# Decrement the persistent run-level life count (survives rooms and saves)
	var rm := _get_run_manager()
	if rm and rm.has_method("lose_life"):
		lives_remaining = int(rm.lose_life())
	else:
		lives_remaining = maxi(lives_remaining - 1, 0)
	_emit_room_status()

func _check_out_of_bounds() -> void:
	if not player:
		return
	if player.global_position.y > room_bounds.end.y + 64.0:
		_on_player_defeated()
		return
	if player.global_position.x < room_bounds.position.x - 64.0:
		_on_player_defeated()
		return
	if player.global_position.x > room_bounds.end.x + 64.0:
		_on_player_defeated()
		return

func _emit_room_status() -> void:
	var rm := _get_run_manager()
	var status := {
		"room_id":           current_room_id,
		"room_name":         room_model.room_name,
		"ore_collected":     room_model.ore_collected,
		"ore_total":         room_model.ore_total,
		"ore_count":         int(rm.get_ore_count()) if rm and rm.has_method("get_ore_count") else room_model.ore_collected,
		"oxygen_percent":    int(round((room_model.current_oxygen / room_model.max_oxygen) * 100.0)),
		"lives_remaining":   lives_remaining,
		"lives_total":       max_lives,
		"score":             int(rm.get_score()) if rm and rm.has_method("get_score") else 0,
		"status":            current_status,
		"tip":               current_tip,
		"has_jetpack":       bool(rm and rm.has_method("has_equipment") and rm.has_equipment("jetpack")),
		"has_magnetic_boots": bool(rm and rm.has_method("has_equipment") and rm.has_equipment("magnetic_boots")),
		"has_grappling_hook": bool(rm and rm.has_method("has_equipment") and rm.has_equipment("grappling_hook")),
		"has_visibility_cloak": bool(rm and rm.has_method("has_equipment") and rm.has_equipment("visibility_cloak")),
		"has_laser_pistol": bool(rm and rm.has_method("has_equipment") and rm.has_equipment("laser_pistol")),
		"has_shield": bool(rm and rm.has_method("has_equipment") and rm.has_equipment("shield")),
		"fuel_pct":          _fuel_pct,
		"fuel_max_pct":      _get_fuel_capacity(),
		"battery_pct":       _battery_pct,
		"is_resupply":       _is_resupply,
	}
	room_status_changed.emit(status)

func _get_fuel_capacity() -> float:
	var rm := _get_run_manager()
	if rm and rm.has_method("get_fuel_capacity_pct"):
		return float(rm.get_fuel_capacity_pct())
	return 100.0

func _as_vec2(value) -> Vector2:
	if value is Vector2:
		return value
	if value is Array and value.size() >= 2:
		return Vector2(float(value[0]), float(value[1]))
	return Vector2.ZERO

func _get_run_manager() -> Node:
	return get_node_or_null("/root/RunManager")
