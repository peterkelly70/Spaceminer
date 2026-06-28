extends Area2D
class_name DalekEnemy

# Ground enemy. Patrols a span; the "hunter" variant charges toward the player
# when they're on the same level. Colour reflects the variant. Emits player_hit.

signal player_hit

@export var span: float = 120.0          # patrol distance from origin
@export var speed: float = 50.0
@export var variant: String = "patrol"   # patrol | hunter

const FRAME_TIME := 0.18

var _origin: Vector2 = Vector2.ZERO
var _dir: float = 1.0
var _frame_t: float = 0.0

@onready var body: Sprite2D = $Body

func _ready() -> void:
	add_to_group("enemy")
	body_entered.connect(_on_body_entered)
	_origin = global_position
	_apply_variant()

func _apply_variant() -> void:
	match variant:
		"hunter":
			if body: body.modulate = Color("#FF4D4D")
		_:
			if body: body.modulate = Color("#C8A24A")

func _physics_process(delta: float) -> void:
	_animate(delta)
	var pos := global_position
	var spd := speed
	if variant == "hunter":
		var pl := _player()
		if pl and absf(pl.global_position.y - pos.y) < 40.0:
			_dir = signf(pl.global_position.x - pos.x)
			spd = speed * 2.0
	pos.x += _dir * spd * delta
	if pos.x > _origin.x + span:
		pos.x = _origin.x + span
		_dir = -1.0
	elif pos.x < _origin.x - span:
		pos.x = _origin.x - span
		_dir = 1.0
	global_position = pos
	if body:
		body.flip_h = _dir < 0.0

func _animate(delta: float) -> void:
	if not body:
		return
	_frame_t += delta
	if _frame_t >= FRAME_TIME:
		_frame_t = 0.0
		body.frame = (body.frame + 1) % maxi(body.hframes, 1)

func _player() -> Node2D:
	return get_tree().get_first_node_in_group("player") as Node2D

func _on_body_entered(b: Node) -> void:
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		player_hit.emit()
