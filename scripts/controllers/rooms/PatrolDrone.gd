extends Area2D
class_name PatrolDrone

signal player_hit

@export var path_points: PackedVector2Array = PackedVector2Array()
@export var speed: float = 64.0
@export var pause_time: float = 0.4

var _target_index: int = 1
var _pause_timer: float = 0.0
var _base_position: Vector2 = Vector2.ZERO
@onready var body_sprite: AnimatedSprite2D = $Body

func _ready() -> void:
	body_entered.connect(_on_body_entered)
	if path_points.size() > 0:
		_base_position = global_position
		global_position = _base_position + path_points[0]
	_play_state("patrol")

func _physics_process(delta: float) -> void:
	if path_points.size() < 2:
		return

	if _pause_timer > 0.0:
		_play_state("idle")
		_pause_timer = maxf(_pause_timer - delta, 0.0)
		return

	_play_state("patrol")
	var target := _base_position + path_points[_target_index]
	var to_target := target - global_position
	var step := speed * delta
	if to_target.length() <= step:
		global_position = target
		_target_index = (_target_index + 1) % path_points.size()
		if _target_index == 0:
			_target_index = 1
		_pause_timer = pause_time
		return

	global_position += to_target.normalized() * step

func _on_body_entered(body: Node) -> void:
	if body.is_in_group("player") or body.is_in_group("prototype_player"):
		_play_state("alert")
		player_hit.emit()

func _play_state(state_name: StringName) -> void:
	if body_sprite.animation == state_name:
		return
	body_sprite.play(state_name)
