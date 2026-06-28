extends Area2D
class_name MovingPlatform

# A platform that rides between two positions on a cycle.

@export var target_pos: Vector2 = Vector2.ZERO  # position to move toward
@export var speed: float = 40.0                  # pixels per second
@export var pause_time: float = 0.5              # pause at each endpoint
@export var visual_color: Color = Color(1.0, 0.65, 0.0, 0.85)

var _origin: Vector2 = Vector2.ZERO
var _target: Vector2 = Vector2.ZERO
var _pause_counter: float = 0.0
var _player_on_platform := false
var _player: Node2D = null
var _player_offset: Vector2 = Vector2.ZERO

@onready var body: Sprite2D = $Body

func _ready() -> void:
	add_to_group("platform")
	body_entered.connect(_on_body_entered)
	body_exited.connect(_on_body_exited)
	_origin = global_position
	_target = _origin + target_pos
	if body:
		body.modulate = visual_color

func _physics_process(delta: float) -> void:
	if _pause_counter > 0:
		_pause_counter -= delta
		return

	var dist := global_position.distance_to(_target)
	if dist < 2.0:
		global_position = _target
		var temp := _target
		_target = _origin
		_origin = temp
		_pause_counter = pause_time
		return

	var dir := (_target - global_position).normalized()
	global_position += dir * speed * delta

	if _player_on_platform and is_instance_valid(_player):
		_player.global_position += dir * speed * delta

func _on_body_entered(b: Node) -> void:
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		_player_on_platform = true
		_player = b as Node2D
		if _player:
			_player_offset = _player.global_position - global_position

func _on_body_exited(b: Node) -> void:
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		_player_on_platform = false
		_player = null
