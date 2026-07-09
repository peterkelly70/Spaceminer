extends AnimatableBody2D
class_name MovingLift

@export var top_y: float = 72.0
@export var bottom_y: float = 176.0
@export var speed: float = 48.0
@export var pause_time: float = 0.6

var _direction: float = -1.0
var _pause_timer: float = 0.0

func _ready() -> void:
	if top_y > bottom_y:
		var tmp := top_y
		top_y = bottom_y
		bottom_y = tmp

func _physics_process(delta: float) -> void:
	if _pause_timer > 0.0:
		_pause_timer = maxf(_pause_timer - delta, 0.0)
		return

	position.y += _direction * speed * delta

	if position.y <= top_y:
		position.y = top_y
		_direction = 1.0
		_pause_timer = pause_time
	elif position.y >= bottom_y:
		position.y = bottom_y
		_direction = -1.0
		_pause_timer = pause_time
