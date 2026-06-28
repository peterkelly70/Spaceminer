extends Area2D
class_name SlidingWall

# A wall that slides open/closed on a timed cycle, blocking passage when closed.

@export var direction: String = "horizontal"   # horizontal | vertical
@export var slide_distance: float = 80.0
@export var speed: float = 60.0
@export var open_time: float = 2.0            # how long wall stays open
@export var closed_time: float = 1.0           # how long wall stays closed
@export var visual_color: Color = Color(1.0, 0.3, 0.3, 0.85)

var _origin: Vector2 = Vector2.ZERO
var _is_open := true
var _time_in_state := 0.0

@onready var body: Sprite2D = $Body
@onready var collision: CollisionShape2D = $CollisionShape2D

func _ready() -> void:
	add_to_group("wall")
	_origin = global_position
	if body:
		body.modulate = visual_color

func _physics_process(delta: float) -> void:
	_time_in_state += delta
	var target_state := _is_open
	var state_time := open_time if _is_open else closed_time

	if _time_in_state >= state_time:
		_time_in_state = 0.0
		_is_open = not _is_open
		target_state = _is_open

	var target_pos := _origin
	if not _is_open:
		if direction == "horizontal":
			target_pos.x += slide_distance
		else:
			target_pos.y += slide_distance

	var dist := global_position.distance_to(target_pos)
	if dist > 0.5:
		var step := speed * delta
		global_position = global_position.move_toward(target_pos, step)

	if collision:
		collision.disabled = _is_open

func get_state() -> String:
	return "open" if _is_open else "closed"
