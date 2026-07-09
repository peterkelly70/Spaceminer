extends StateAwareController
class_name IntroNarrativeController

@onready var space_black: ColorRect = $SpaceBlack
@onready var star_layer: Control = $StarLayer
@onready var background: TextureRect = $Background
@onready var backdrop_dim: ColorRect = $BackdropDim
@onready var scene_layer: Control = $SceneLayer
@onready var dock_glow: ColorRect = $SceneLayer/DockGlow
@onready var hangar_bay: ColorRect = $SceneLayer/HangarBay
@onready var hangar_edge_top: ColorRect = $SceneLayer/HangarEdgeTop
@onready var hangar_edge_bottom: ColorRect = $SceneLayer/HangarEdgeBottom
@onready var hangar_door: ColorRect = $SceneLayer/HangarDoor
@onready var ship: TextureRect = $SceneLayer/SpaceShip
@onready var asteroid: TextureRect = $SceneLayer/Asteroid
@onready var impact_flash: ColorRect = $SceneLayer/ImpactFlash
@onready var player: TextureRect = $SceneLayer/Player
@onready var dialogue_panel: Control = $DialoguePanel
@onready var title_label: Label = $DialoguePanel/MarginContainer/VBoxContainer/TitleLabel
@onready var narration_label: RichTextLabel = $DialoguePanel/MarginContainer/VBoxContainer/NarrationLabel
@onready var skip_button: Button = $DialoguePanel/MarginContainer/VBoxContainer/ButtonRow/SkipButton
@onready var continue_button: Button = $DialoguePanel/MarginContainer/VBoxContainer/ButtonRow/ContinueButton
@onready var prompt_label: Label = $DialoguePanel/MarginContainer/VBoxContainer/ButtonRow/PromptLabel

var _intro_started := false
var _intro_finished := false
var _skip_requested := false
var _active_tween: Tween = null
var _rng := RandomNumberGenerator.new()

const INTRO_BEATS := [
	{
		"title": "ASTEROID STRIKE",
		"body": "[color=#d9f6ff]A rogue rock tears into the shuttle. Fuel bleeds out.[/color]",
		"hold": 1.0
	},
	{
		"title": "MINING STATION AHEAD",
		"body": "[color=#f5e4a8]The pilot keeps the nose pointed right and limps for the hangar.[/color]",
		"hold": 1.0
	},
	{
		"title": "WAKE IN THE BAY",
		"body": "[color=#ffffff]You start here. Find the parts. Fix the ship. Get out alive.[/color]",
		"hold": 1.0
	}
]

func _ready() -> void:
	show_in_states = [AppState.State.SPLASH]
	_setup_intro_scene()
	_build_starfield()
	if skip_button and not skip_button.pressed.is_connected(Callable(self, "_on_skip_pressed")):
		skip_button.pressed.connect(Callable(self, "_on_skip_pressed"))
	if continue_button and not continue_button.pressed.is_connected(Callable(self, "_on_continue_pressed")):
		continue_button.pressed.connect(Callable(self, "_on_continue_pressed"))
	super._ready()

func _setup_intro_scene() -> void:
	_rng.randomize()
	if space_black:
		space_black.color = Color(0, 0, 0, 1)
	if background:
		background.visible = false
	if title_label:
		title_label.text = "ASTEROID STRIKE"
		title_label.modulate = Color(1, 1, 1, 1)
	if narration_label:
		narration_label.text = ""
	if continue_button:
		continue_button.visible = false
	if prompt_label:
		prompt_label.text = "Any key skips the intro"
	if backdrop_dim:
		backdrop_dim.color = Color(0, 0, 0, 0.0)
	if dock_glow:
		dock_glow.color = Color(0, 0.92, 1, 0.00)
	if hangar_bay:
		hangar_bay.color = Color(0.02, 0.02, 0.03, 0.0)
	if hangar_edge_top:
		hangar_edge_top.color = Color(0, 0.86, 1, 0.0)
	if hangar_edge_bottom:
		hangar_edge_bottom.color = Color(0, 0.86, 1, 0.0)
	if hangar_door:
		hangar_door.color = Color(0.06, 0.12, 0.16, 0.0)
	if ship:
		ship.position = Vector2(-340, 330)
		ship.scale = Vector2(1.0, 1.0)
		ship.rotation_degrees = 0.0
		ship.flip_h = false
		ship.modulate = Color(1, 1, 1, 0.0)
	if asteroid:
		asteroid.position = Vector2(1260, -130)
		asteroid.modulate = Color(1, 1, 1, 0.0)
	if impact_flash:
		impact_flash.color = Color(1, 1, 1, 0.0)
	if player:
		player.position = Vector2(1320, 600)
		player.modulate = Color(1, 1, 1, 0.0)
	if scene_layer:
		scene_layer.position = Vector2.ZERO
	if dialogue_panel:
		dialogue_panel.position = Vector2.ZERO

func on_enter_state(_state: int) -> void:
	visible = true
	if _intro_started:
		return
	if SettingsManager.get_setting("gameplay", "hide_cutscenes", false):
		call_deferred("_finish_intro")
		return
	_intro_started = true
	_skip_requested = false
	_intro_finished = false
	call_deferred("_play_intro")

func on_exit_state(_state: int) -> void:
	visible = false
	_kill_active_tween()

func _unhandled_input(event: InputEvent) -> void:
	if not visible or _intro_finished:
		return
	if event is InputEventKey and event.pressed and not event.echo:
		_request_skip()
	elif event is InputEventMouseButton and event.pressed:
		_request_skip()

func _on_skip_pressed() -> void:
	_request_skip()

func _on_continue_pressed() -> void:
	_finish_intro()

func _request_skip() -> void:
	if _intro_finished:
		return
	_skip_requested = true
	_finish_intro()

func _play_intro() -> void:
	if _intro_finished:
		return
	await _show_beat("ASTEROID STRIKE", "[color=#d9f6ff]A rogue rock tears into the shuttle.[/color]", 0.75)
	if _skip_requested:
		return
	await _animate_ship_approach()
	if _skip_requested:
		return
	await _show_impact_and_drift()
	if _skip_requested:
		return
	await _show_beat("MINING STATION AHEAD", "[color=#f5e4a8]The pilot keeps the nose pointed right and limps for the hangar.[/color]", 0.9)
	if _skip_requested:
		return
	await _wake_in_hangar()
	if _skip_requested:
		return
	await _show_beat("WAKE IN THE BAY", "[color=#ffffff]Find the parts. Carry them back. Fix the ship.[/color]", 1.0)
	if _skip_requested:
		return
	if continue_button:
		continue_button.visible = true
		continue_button.grab_focus()
	if prompt_label:
		prompt_label.text = "Press Continue, or any key"
	await get_tree().create_timer(2.0).timeout
	if not _skip_requested:
		_finish_intro()

func _show_beat(title: String, text: String, hold_seconds: float = 0.85) -> void:
	if narration_label:
		narration_label.text = text
	if title_label:
		title_label.text = title
		title_label.modulate = Color(1, 1, 1, 1)
	await get_tree().create_timer(hold_seconds).timeout

func _animate_ship_approach() -> void:
	_kill_active_tween()
	if ship:
		ship.modulate = Color(1, 1, 1, 1)
		ship.rotation_degrees = 0.0
		ship.scale = Vector2(0.98, 0.98)
		_active_tween = create_tween()
		_active_tween.set_parallel(true)
		_active_tween.tween_property(ship, "position", Vector2(180, 330), 2.35).set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_SINE)
		_active_tween.tween_property(ship, "scale", Vector2(1.0, 1.0), 2.35).set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_SINE)
		await _active_tween.finished
	await _shake_scene(0.14, 2.0)

func _show_impact_and_drift() -> void:
	_kill_active_tween()
	if asteroid:
		asteroid.modulate = Color(1, 1, 1, 1)
		asteroid.position = Vector2(1240, -110)
		_active_tween = create_tween()
		_active_tween.set_parallel(true)
		_active_tween.tween_property(asteroid, "position", Vector2(400, 210), 0.75).set_ease(Tween.EASE_IN).set_trans(Tween.TRANS_QUAD)
		_active_tween.tween_property(asteroid, "rotation_degrees", 280.0, 0.75).set_ease(Tween.EASE_IN_OUT).set_trans(Tween.TRANS_QUAD)
		await _active_tween.finished

	if impact_flash:
		impact_flash.color = Color(1, 1, 1, 0.85)
		var flash_tween := create_tween()
		flash_tween.tween_property(impact_flash, "color", Color(1, 1, 1, 0.0), 0.35)
	await _shake_scene(0.36, 5.5)

	if ship:
		ship.modulate = Color(1, 1, 1, 1)
		var ship_tween := create_tween()
		ship_tween.set_parallel(true)
		ship_tween.tween_property(ship, "position", Vector2(970, 338), 2.2).set_ease(Tween.EASE_IN_OUT).set_trans(Tween.TRANS_SINE)
		ship_tween.tween_property(ship, "rotation_degrees", 3.0, 2.2).set_ease(Tween.EASE_IN_OUT).set_trans(Tween.TRANS_SINE)
		await ship_tween.finished

func _wake_in_hangar() -> void:
	_kill_active_tween()
	if dock_glow:
		var glow_tween := create_tween()
		glow_tween.tween_property(dock_glow, "color", Color(0, 0.92, 1, 0.15), 0.4)
	if hangar_bay:
		var bay_tween := create_tween()
		bay_tween.tween_property(hangar_bay, "color", Color(0.02, 0.02, 0.03, 0.9), 0.4)
	if hangar_edge_top:
		var top_tween := create_tween()
		top_tween.tween_property(hangar_edge_top, "color", Color(0, 0.86, 1, 1), 0.35)
	if hangar_edge_bottom:
		var bottom_tween := create_tween()
		bottom_tween.tween_property(hangar_edge_bottom, "color", Color(0, 0.86, 1, 1), 0.35)
	if hangar_door:
		var door_tween := create_tween()
		door_tween.tween_property(hangar_door, "color", Color(0.04, 0.08, 0.12, 1), 0.35)
	if player:
		player.modulate = Color(1, 1, 1, 0.0)
		var player_tween := create_tween()
		player_tween.set_parallel(true)
		player_tween.tween_property(player, "modulate", Color(1, 1, 1, 1.0), 0.45)
		player_tween.tween_property(player, "position", Vector2(1340, 606), 0.45).set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_BACK)
		await player_tween.finished
	await get_tree().create_timer(0.7).timeout

func _build_starfield() -> void:
	if not star_layer:
		return
	for child in star_layer.get_children():
		child.queue_free()
	var star_count := 52
	for i in range(star_count):
		var star := ColorRect.new()
		star.mouse_filter = Control.MOUSE_FILTER_IGNORE
		var size := 1 + int(_rng.randi_range(0, 2))
		star.custom_minimum_size = Vector2(size, size)
		star.size = Vector2(size, size)
		var color_pick := _rng.randi_range(0, 5)
		match color_pick:
			0:
				star.color = Color(1, 1, 1, 1)
			1:
				star.color = Color(0, 0.9, 1, 1)
			2:
				star.color = Color(1, 0.8, 0.15, 1)
			3:
				star.color = Color(1, 0.35, 0.85, 1)
			4:
				star.color = Color(0.75, 0.95, 1, 1)
			_:
				star.color = Color(1, 1, 0.55, 1)
		star.position = Vector2(
			_rng.randf_range(0.0, 1530.0),
			_rng.randf_range(0.0, 760.0)
		)
		star_layer.add_child(star)
		if star.size.x > 1:
			var twin := ColorRect.new()
			twin.mouse_filter = Control.MOUSE_FILTER_IGNORE
			twin.custom_minimum_size = Vector2(1, 1)
			twin.size = Vector2(1, 1)
			twin.color = star.color
			twin.position = star.position + Vector2(1, 0)
			star_layer.add_child(twin)

func _finish_intro() -> void:
	if _intro_finished:
		return
	_intro_finished = true
	_kill_active_tween()
	if continue_button:
		continue_button.visible = false
	if prompt_label:
		prompt_label.text = ""
	if SettingsManager.get_setting("gameplay", "hide_cutscenes", false):
		State_Manager.change_state(AppState.State.MAIN_MENU)
		return
	State_Manager.change_state(AppState.State.MAIN_MENU)

func _shake_scene(duration: float, intensity: float) -> void:
	if not scene_layer:
		return
	var start_scene := scene_layer.position
	var start_panel := dialogue_panel.position if dialogue_panel else Vector2.ZERO
	var elapsed := 0.0
	while elapsed < duration and not _skip_requested:
		var offset := Vector2(
			_rng.randf_range(-intensity, intensity),
			_rng.randf_range(-intensity, intensity)
		)
		scene_layer.position = start_scene + offset
		if dialogue_panel:
			dialogue_panel.position = start_panel + offset * 0.25
		await get_tree().create_timer(0.016).timeout
		elapsed += 0.016
	scene_layer.position = start_scene
	if dialogue_panel:
		dialogue_panel.position = start_panel

func _kill_active_tween() -> void:
	if _active_tween and is_instance_valid(_active_tween):
		_active_tween.kill()
	_active_tween = null
