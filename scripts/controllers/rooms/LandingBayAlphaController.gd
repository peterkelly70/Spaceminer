extends Node2D
class_name LandingBayAlphaController

signal request_room_change(next_room_id: String)
signal room_status_changed(status: Dictionary)

const LandingBayAlphaModelScript = preload("res://scripts/models/LandingBayAlphaModel.gd")

@onready var player: PrototypePlayer = $Player
@onready var respawn_point: Marker2D = $RespawnPoint
@onready var collectibles_root: Node = $Collectibles
@onready var hazards_root: Node = $Hazards
@onready var enemies_root: Node = $Enemies
@onready var exit_zone: Area2D = $ExitZone

var room_model := LandingBayAlphaModelScript.new()
var oxygen_warning_threshold := 0.25
var current_status := "Door sealed"
var current_tip := "Arrows to move, Space to jump, W/Up to thrust, F to grapple"
var max_lives := 10
var lives_remaining := 10
var room_bounds := Rect2(Vector2(-320, -180), Vector2(640, 360))
var _respawning := false

func _ready() -> void:
	room_model.ore_changed.connect(_on_ore_changed)
	room_model.oxygen_changed.connect(_on_oxygen_changed)
	room_model.tip_changed.connect(_on_tip_changed)
	room_model.oxygen_depleted.connect(_on_oxygen_depleted)
	player.player_defeated.connect(_on_player_defeated)
	_register_collectibles()
	_register_hazards()
	_register_enemies()
	_register_exit_zone()
	room_model.reset()
	_sync_oxygen_from_run_manager()
	room_model.set_tip("Arrows to move, Space to jump, W/Up to thrust, F to grapple")
	_set_status("Door sealed")
	_emit_room_status()

func _process(delta: float) -> void:
	room_model.drain_oxygen(delta)
	_sync_oxygen_to_run_manager()
	_check_out_of_bounds()

func _register_collectibles() -> void:
	for child in collectibles_root.get_children():
		if child is OreFragment:
			room_model.register_ore(child.amount)
			child.collected.connect(_on_ore_collected)

func _register_hazards() -> void:
	for child in hazards_root.get_children():
		if child is HazardZone:
			child.player_hit.connect(_on_hazard_triggered)

func _register_enemies() -> void:
	for child in enemies_root.get_children():
		if child.has_signal("player_hit"):
			child.connect("player_hit", Callable(self, "_on_hazard_triggered"))

func _register_exit_zone() -> void:
	if exit_zone:
		exit_zone.body_entered.connect(_on_exit_body_entered)

func _on_ore_collected(amount: int) -> void:
	room_model.collect_ore(amount)
	var rm := get_node_or_null("/root/RunManager")
	if rm and rm.has_method("add_ore"):
		rm.add_ore(amount)
	room_model.set_tip("Nice haul!")
	_play_collect_feedback()

func _on_hazard_triggered() -> void:
	if player.handle_hazard_hit():
		_play_damage_feedback()

func _on_exit_body_entered(body: Node) -> void:
	if not body.is_in_group("player"):
		return
	room_model.refill_oxygen()
	_sync_oxygen_to_run_manager()
	room_model.set_tip("Pressure door cycled.")
	_set_status("Exit reached")
	var next_room := _get_next_room_id()
	if not next_room.is_empty():
		request_room_change.emit(next_room)

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
	room_model.set_tip("Arrows to move, Space to jump, W/Up to thrust, F to grapple")
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
	room_model.set_tip("Arrows to move, Space to jump, W/Up to thrust, F to grapple")
	_respawning = false

func _on_ore_changed(collected: int, total: int) -> void:
	_emit_room_status()

func _on_oxygen_changed(current: float, max_value: float) -> void:
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
	var rm := get_node_or_null("/root/RunManager")
	if rm and rm.has_method("get_oxygen_pct"):
		room_model.set_oxygen_pct(float(rm.get_oxygen_pct()))

func _sync_oxygen_to_run_manager() -> void:
	var rm := get_node_or_null("/root/RunManager")
	if rm and rm.has_method("update_oxygen"):
		rm.update_oxygen(room_model.get_oxygen_pct())

func _set_status(text: String) -> void:
	current_status = text
	_emit_room_status()

func _mark_life_lost() -> void:
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
	var rm := get_node_or_null("/root/RunManager")
	room_status_changed.emit({
		"room_name": room_model.room_name,
		"ore_collected": room_model.ore_collected,
		"ore_total": room_model.ore_total,
		"ore_count": int(rm.get_ore_count()) if rm and rm.has_method("get_ore_count") else room_model.ore_collected,
		"oxygen_percent": int(round((room_model.current_oxygen / room_model.max_oxygen) * 100.0)),
		"lives_remaining": lives_remaining,
		"lives_total": max_lives,
		"status": current_status,
		"tip": current_tip,
	})

func _get_next_room_id() -> String:
	match room_model.room_name:
		"Landing Bay Alpha":
			return "Control Hub"
		"Control Hub":
			return "Ore Shaft"
		"Ore Shaft":
			return "Landing Bay Alpha"
		_:
			return "Control Hub"
