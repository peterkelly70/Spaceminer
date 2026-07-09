extends Area2D
class_name Ladder

# A climbable ladder. Built procedurally from `height`/`width` so LevelBuilder can
# instantiate it directly (no .tscn needed). The player detects overlap via the
# "ladder" group and climbs with up/down.

@export var height: float = 64.0
@export var width: float = 14.0

const RAIL_COLOR := Color(0.85, 0.72, 0.28, 0.9)

func _ready() -> void:
	add_to_group("ladder")
	collision_layer = 0
	collision_mask = 0
	monitorable = false
	monitoring = false
	z_index = 1
	_build_visual()

# Surfaces (global): top = where you stand when you reach the top,
# bottom = where you stand at the base.
func top_y() -> float:
	return global_position.y - height * 0.5

func bottom_y() -> float:
	return global_position.y + height * 0.5

func center_x() -> float:
	return global_position.x

# True when a point (the player origin) is within the ladder's climb column.
func contains_point(p: Vector2) -> bool:
	if absf(p.x - global_position.x) > width * 0.5 + 16.0:
		return false
	return p.y >= top_y() - 24.0 and p.y <= bottom_y() + 8.0

func _build_visual() -> void:
	var hw := width * 0.5
	var top := -height * 0.5
	var bottom := height * 0.5
	_add_line(Vector2(-hw, top), Vector2(-hw, bottom))
	_add_line(Vector2(hw, top), Vector2(hw, bottom))
	var y := top + 4.0
	while y < bottom:
		_add_line(Vector2(-hw, y), Vector2(hw, y))
		y += 8.0

func _add_line(a: Vector2, b: Vector2) -> void:
	var ln := Line2D.new()
	ln.width = 2.0
	ln.default_color = RAIL_COLOR
	ln.add_point(a)
	ln.add_point(b)
	ln.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	add_child(ln)
