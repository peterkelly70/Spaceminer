extends Node2D
class_name PrototypeRoom

@onready var player: PrototypePlayer = $Player
@onready var respawn_point: Marker2D = $RespawnPoint
@onready var collectibles_root: Node = $Collectibles
@onready var hazards_root: Node = $Hazards
@onready var ore_label: Label = $"HUDLayer/HUD/Panel/MarginContainer/VBoxContainer/OreLabel"
@onready var stamina_bar: ProgressBar = $"HUDLayer/HUD/Panel/MarginContainer/VBoxContainer/StaminaPanel/StaminaBar"
@onready var stamina_title: Label = $"HUDLayer/HUD/Panel/MarginContainer/VBoxContainer/StaminaPanel/StaminaLabel"
@onready var tip_label: Label = $"HUDLayer/HUD/Panel/MarginContainer/VBoxContainer/TipLabel"
const PROTOTYPE_TRACK_NAME := "prototype_room_theme"
const PROTOTYPE_TRACK_PATH := "res://assets/audio/music/Dark.Shadows/Moonlit Shadows.mp3"

var audio_manager := Audio_Manager

var ore_found := 0
var ore_total := 0
var stamina_warning_threshold := 0.25

func _ready() -> void:
	_ensure_audio_assets()
	player.stamina_changed.connect(_on_player_stamina_changed)
	player.player_defeated.connect(_on_player_defeated)
	_register_collectibles()
	_register_hazards()
	stamina_bar.max_value = player.stamina.max_stamina
	stamina_bar.value = player.stamina.current_stamina
	_update_ore_label()
	if audio_manager:
		audio_manager.play_music(PROTOTYPE_TRACK_NAME)

func _register_collectibles() -> void:
	ore_found = 0
	ore_total = 0
	for child in collectibles_root.get_children():
		if child is OreFragment:
			ore_total += child.amount
			child.collected.connect(_on_ore_collected)
	_update_ore_label()

func _register_hazards() -> void:
	for child in hazards_root.get_children():
		if child is HazardZone:
			child.player_hit.connect(_on_hazard_triggered)


func _ensure_audio_assets() -> void:
	if not audio_manager:
		return
	if not audio_manager.music_tracks.has(PROTOTYPE_TRACK_NAME):
		audio_manager.register_music(PROTOTYPE_TRACK_PATH, PROTOTYPE_TRACK_NAME, true, false)
	var prototype_sfx = {
		"jump": "res://assets/audio/sfx/ui/button.mp3",
		"collect": "res://assets/audio/sfx/ui/build.mp3",
		"damage": "res://assets/audio/sfx/ui/error.mp3"
	}
	for name in prototype_sfx.keys():
		if not audio_manager.sound_effects.has(name):
			audio_manager.register_sfx(prototype_sfx[name], name)

func _on_ore_collected(amount: int) -> void:
	ore_found += amount
	_update_ore_label()
	if audio_manager:
		audio_manager.play_sfx("collect")
	tip_label.text = "Nice haul!"
	await get_tree().create_timer(1.0).timeout
	tip_label.text = "Arrows to move, Space to jump, W/Up to thrust, F to grapple"


func _on_hazard_triggered() -> void:
	if player.handle_hazard_hit():
		if audio_manager:
			audio_manager.play_sfx("damage")

func _on_player_defeated() -> void:
	tip_label.text = "Ouch! Respawning..."
	await get_tree().create_timer(0.1).timeout
	player.respawn(respawn_point.global_position)
	await get_tree().create_timer(1.0).timeout
	tip_label.text = "Watch the spikes"
	await get_tree().create_timer(1.5).timeout
	tip_label.text = "Arrows to move, Space to jump, W/Up to thrust, F to grapple"

func _on_player_stamina_changed(current: float, max_value: float) -> void:
	stamina_bar.max_value = max_value
	stamina_bar.value = current
	var ratio := current / max_value
	if ratio < stamina_warning_threshold:
		stamina_title.text = "Stamina - Rest!"
	else:
		stamina_title.text = "Stamina"

func _update_ore_label() -> void:
	ore_label.text = "Ore Collected: %d / %d" % [ore_found, ore_total]
