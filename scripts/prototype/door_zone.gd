extends Area2D
class_name DoorZone

signal door_selected(target_room_id: String)
signal door_blocked(required_items: Array[String])

@export var target_room_id: String = ""
@export var door_label: String = "Door"
@export var door_direction: String = "east"
@export var requires: Array[String] = []
@export var interact_hold_seconds: float = 0.28

@onready var collision_shape: CollisionShape2D = $CollisionShape2D
@onready var body_visual: Sprite2D = $BodyVisual
@onready var panel_left: Polygon2D = $PanelLeft if has_node("PanelLeft") else null
@onready var panel_right: Polygon2D = $PanelRight if has_node("PanelRight") else null
@onready var title_label: Label = $Label
@onready var hold_bar: ProgressBar = $HoldBar if has_node("HoldBar") else null

var _player_near := false
var _tracked_player: Node2D = null
var _hold_progress := 0.0
var _selection_fired := false
var _art_offset := Vector2.ZERO
var _door_direction := ""

const OPEN_COLOR := Color("#00D7FF")
const LOCKED_COLOR := Color("#FF1E2D")
const EXIT_COLOR := Color("#FFD21A")
const PANEL_COLOR := Color("#1B1B24")
const GLOW_COLOR := Color("#00D7FF")

func _ready() -> void:
	body_entered.connect(_on_body_entered)
	body_exited.connect(_on_body_exited)
	set_process(true)
	if hold_bar:
		hold_bar.visible = false
		hold_bar.value = 0.0
	if title_label:
		title_label.add_theme_font_size_override("font_size", 18)
		title_label.add_theme_color_override("font_outline_color", Color.BLACK)
		title_label.add_theme_constant_override("outline_size", 6)
	_refresh_visual()

func configure(data: Dictionary) -> void:
	target_room_id = str(data.get("target_room_id", target_room_id))
	door_label = str(data.get("label", data.get("name", door_label)))
	door_direction = str(data.get("direction", door_direction))
	_door_direction = door_direction.to_lower()
	requires = []
	var requirement_values: Array = data.get("requires", [])
	for value in requirement_values:
		requires.append(str(value))
	var size := _normalize_size(_as_vec2(data.get("size", _default_size_for_direction())))
	var interaction_size := _interaction_size_for_direction(size)
	_art_offset = _art_offset_for_direction(size)
	if collision_shape and collision_shape.shape is RectangleShape2D:
		(collision_shape.shape as RectangleShape2D).size = interaction_size
		collision_shape.position = _art_offset + _zone_offset_for_direction(size, interaction_size)
	if body_visual:
		body_visual.position = _art_offset
		body_visual.scale = Vector2.ONE
	if panel_left:
		panel_left.position = _art_offset
		panel_left.scale = size / Vector2(40.0, 72.0)
	if panel_right:
		panel_right.position = _art_offset
		panel_right.scale = size / Vector2(40.0, 72.0)
	if title_label:
		title_label.position = _art_offset + _label_offset(size)
	if hold_bar:
		hold_bar.position = _art_offset + Vector2(-24.0, -size.y * 0.5 - 42.0)
		hold_bar.custom_minimum_size = Vector2(48.0, 10.0)
		hold_bar.value = 0.0
	_hold_progress = 0.0
	_selection_fired = false
	_refresh_visual()

func _process(delta: float) -> void:
	if not _player_near:
		_hold_progress = 0.0
		_selection_fired = false
		_tracked_player = null
		if hold_bar:
			hold_bar.visible = false
			hold_bar.value = 0.0
		return
	if _is_locked():
		_hold_progress = 0.0
		_selection_fired = false
		if hold_bar:
			hold_bar.visible = false
			hold_bar.value = 0.0
		return
	if hold_bar:
		hold_bar.visible = true
		if _tracked_player:
			hold_bar.global_position = _tracked_player.global_position + Vector2(-24.0, -58.0)
	if InputMap.has_action("interact") and Input.is_action_pressed("interact"):
		_hold_progress = minf(_hold_progress + delta, interact_hold_seconds)
		if not _selection_fired and _hold_progress >= interact_hold_seconds:
			_selection_fired = true
			_select_door()
	else:
		_hold_progress = 0.0
		_selection_fired = false
	if hold_bar:
		hold_bar.value = (_hold_progress / interact_hold_seconds) * 100.0 if interact_hold_seconds > 0.0 else 0.0

func _refresh_visual() -> void:
	if title_label:
		var text := door_label.to_upper()
		if not requires.is_empty():
			text += " (NEEDS %s)" % str(requires[0]).replace("_", " ").to_upper()
		title_label.text = text.strip_edges()
		title_label.modulate = EXIT_COLOR if target_room_id == "campaign_complete" else (LOCKED_COLOR if not requires.is_empty() else Color.WHITE)
		title_label.visible = false

	if body_visual:
		body_visual.position = _art_offset
		var color := Color.WHITE
		# Tint based on lock requirement (subtle overlay)
		if target_room_id == "campaign_complete":
			color = EXIT_COLOR
		elif not requires.is_empty():
			# Subtle tint: darken the requirement color and blend with white
			var req_color := _requirement_color()
			color = Color.WHITE.lerp(req_color, 0.25)  # 25% tint towards requirement color
		body_visual.self_modulate = color
	if panel_left:
		panel_left.position = _art_offset + Vector2(-10.0 if _player_near and requires.is_empty() else 0.0, 0.0)
		panel_left.color = PANEL_COLOR if requires.is_empty() else _requirement_color().darkened(0.45)
	if panel_right:
		panel_right.position = _art_offset + Vector2(10.0 if _player_near and requires.is_empty() else 0.0, 0.0)
		panel_right.color = PANEL_COLOR if requires.is_empty() else _requirement_color().darkened(0.45)

func _on_body_entered(body: Node) -> void:
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return
	_player_near = true
	_tracked_player = body as Node2D
	_hold_progress = 0.0
	_selection_fired = false
	if hold_bar:
		hold_bar.visible = true
		hold_bar.value = 0.0
	_refresh_visual()
	if _is_locked():
		door_blocked.emit(requires.duplicate())
		_play_locked_sfx()
		return

func _on_body_exited(body: Node) -> void:
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return
	_player_near = false
	_tracked_player = null
	_hold_progress = 0.0
	_selection_fired = false
	if hold_bar:
		hold_bar.visible = false
		hold_bar.value = 0.0
	_refresh_visual()

func _select_door() -> void:
	if not _player_near or _is_locked():
		return
	door_selected.emit(target_room_id)
	_play_open_sfx()

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
	return _default_size_for_direction()

func _default_size_for_direction() -> Vector2:
	match _door_direction:
		"north", "south", "up", "down":
			return Vector2(48, 16)
		_:
			return Vector2(16, 48)

func _normalize_size(size: Vector2) -> Vector2:
	var expected := _default_size_for_direction()
	return Vector2(minf(size.x, expected.x), minf(size.y, expected.y))

func _interaction_size_for_direction(size: Vector2) -> Vector2:
	var out := size
	match _door_direction:
		"east", "west":
			# Give the player a tile of forgiveness on either side of the door.
			out.x = maxf(out.x, size.x + 32.0)
		"north", "south", "up", "down":
			# Reach 48px into the room so a player standing on the landing ledge
			# (48px below a ceiling door / above a floor door) overlaps the zone.
			out.y = maxf(out.y, size.y + 48.0)
	return out

# Ceiling/floor door zones grow toward the room interior only: keep the
# wall-side edge where the door sits and push the extra reach inward.
func _zone_offset_for_direction(size: Vector2, interaction_size: Vector2) -> Vector2:
	var reach := (interaction_size.y - size.y) * 0.5
	match _door_direction:
		"north", "up":
			return Vector2(0.0, reach)
		"south", "down":
			return Vector2(0.0, -reach)
		_:
			return Vector2.ZERO

func _art_offset_for_direction(size: Vector2) -> Vector2:
	# Pin the visible tile to the BOTTOM edge of the wall-door zone so the door
	# art sits directly on its landing platform (whose top is flush with the
	# zone bottom — see RoomLayoutGenerator._door_landing_top_y).
	match _door_direction:
		"east", "west":
			return Vector2(0.0, size.y * 0.5 - 8.0)
		"north", "south", "up", "down":
			return Vector2(-size.x * 0.5 + 8.0, 0.0)
		_:
			return Vector2.ZERO

# Places the label clear of the door art: above the door for every direction
# except south/down, whose door sits at the bottom edge (label goes below it).
# The label box is 128 px wide, so -64 centres it horizontally over the door.
func _label_offset(size: Vector2) -> Vector2:
	if _door_direction in ["south", "down"]:
		return Vector2(-64.0, size.y * 0.5 + 8.0)
	return Vector2(-64.0, -size.y * 0.5 - 24.0)
