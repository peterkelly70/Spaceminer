extends Area2D
class_name DoorZone

signal door_selected(target_room_id: String)
signal door_blocked(required_items: Array[String])

@export var target_room_id: String = ""
@export var door_label: String = "Door"
@export var door_direction: String = "east"
@export var requires: Array[String] = []

@onready var collision_shape: CollisionShape2D = $CollisionShape2D
@onready var body_visual: Polygon2D = $BodyVisual
@onready var panel_left: Polygon2D = $PanelLeft if has_node("PanelLeft") else null
@onready var panel_right: Polygon2D = $PanelRight if has_node("PanelRight") else null
@onready var title_label: Label = $Label

var _player_near := false

const OPEN_COLOR := Color("#00D7FF")
const LOCKED_COLOR := Color("#FF1E2D")
const EXIT_COLOR := Color("#FFD21A")
const PANEL_COLOR := Color("#1B1B24")
const GLOW_COLOR := Color("#00D7FF")

func _ready() -> void:
	body_entered.connect(_on_body_entered)
	body_exited.connect(_on_body_exited)
	if title_label:
		title_label.add_theme_font_size_override("font_size", 18)
		title_label.add_theme_color_override("font_outline_color", Color.BLACK)
		title_label.add_theme_constant_override("outline_size", 6)
	_refresh_visual()

func configure(data: Dictionary) -> void:
	target_room_id = str(data.get("target_room_id", target_room_id))
	door_label = str(data.get("label", data.get("name", door_label)))
	door_direction = str(data.get("direction", door_direction))
	requires = []
	var requirement_values: Array = data.get("requires", [])
	for value in requirement_values:
		requires.append(str(value))
	var size := _as_vec2(data.get("size", [40, 72]))
	if collision_shape and collision_shape.shape is RectangleShape2D:
		(collision_shape.shape as RectangleShape2D).size = size
	if body_visual:
		body_visual.scale = Vector2(1.0, 1.0)
	if panel_left:
		panel_left.scale = size / Vector2(40.0, 72.0)
	if panel_right:
		panel_right.scale = size / Vector2(40.0, 72.0)
	if title_label:
		title_label.position = Vector2(-size.x * 0.5, -size.y * 0.5 - 20.0)
	_refresh_visual()

func _refresh_visual() -> void:
	if title_label:
		var text := door_label.to_upper()
		if not requires.is_empty():
			text += " (NEEDS %s)" % str(requires[0]).replace("_", " ").to_upper()
		title_label.text = text.strip_edges()
		title_label.modulate = EXIT_COLOR if target_room_id == "campaign_complete" else (LOCKED_COLOR if not requires.is_empty() else Color.WHITE)
		title_label.visible = true

	if body_visual:
		var open := requires.is_empty() and _player_near
		var color := Color.WHITE if open else GLOW_COLOR.darkened(0.35)
		if target_room_id == "campaign_complete":
			color = EXIT_COLOR if _player_near else EXIT_COLOR.darkened(0.35)
		elif not requires.is_empty():
			color = _requirement_color().lightened(0.1) if _player_near else _requirement_color().darkened(0.25)
		body_visual.color = color
		body_visual.self_modulate = color
	if panel_left:
		panel_left.color = PANEL_COLOR if requires.is_empty() else _requirement_color().darkened(0.45)
		panel_left.position = Vector2(-10.0 if _player_near and requires.is_empty() else 0.0, 0.0)
	if panel_right:
		panel_right.color = PANEL_COLOR if requires.is_empty() else _requirement_color().darkened(0.45)
		panel_right.position = Vector2(10.0 if _player_near and requires.is_empty() else 0.0, 0.0)

func _on_body_entered(body: Node) -> void:
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return
	_player_near = true
	_refresh_visual()
	if _is_locked():
		door_blocked.emit(requires.duplicate())
		_play_locked_sfx()
		return
	door_selected.emit(target_room_id)
	_play_open_sfx()

func _on_body_exited(body: Node) -> void:
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return
	_player_near = false
	_refresh_visual()

func _is_locked() -> bool:
	if requires.is_empty():
		return false
	var run_manager := get_node_or_null("/root/RunManager")
	if not run_manager:
		return true
	for item_id in requires:
		if not run_manager.has_method("has_equipment") or not run_manager.has_equipment(item_id):
			return true
	return false

func _play_locked_sfx() -> void:
	var audio_manager := get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("error")

func _play_open_sfx() -> void:
	var audio_manager := get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("door_open")

func _requirement_color() -> Color:
	if requires.is_empty():
		return OPEN_COLOR
	match requires[0]:
		"jetpack":
			return Color("#00D7FF")
		"laser_pistol":
			return Color("#FF1E2D")
		"grappling_hook":
			return Color("#FFD21A")
		"magnetic_boots":
			return Color("#8CFF00")
		"visibility_cloak":
			return Color("#FF00C8")
		"shield":
			return Color("#1D4FFF")
		_:
			return LOCKED_COLOR

func _as_vec2(value) -> Vector2:
	if value is Vector2:
		return value
	if value is Array and value.size() >= 2:
		return Vector2(float(value[0]), float(value[1]))
	return Vector2(40, 72)
