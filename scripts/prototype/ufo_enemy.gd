extends Area2D
class_name UfoEnemy

# Flying enemy. Sweeps horizontally and bobs; the "diver" variant swoops toward
# the player. Colour reflects the variant. Emits player_hit on contact.

signal player_hit

@export var span: float = 160.0          # horizontal sweep distance from origin
@export var speed: float = 36.0
@export var variant: String = "drifter"  # drifter | diver | speeder

const FRAME_TIME := 0.15

var _origin: Vector2 = Vector2.ZERO
var _dir: float = 1.0
var _t: float = 0.0
var _frame_t: float = 0.0

@onready var body: Sprite2D = $Body

func _ready() -> void:
	add_to_group("enemy")
	body_entered.connect(_on_body_entered)
	_origin = global_position
	_dir = 1.0 if randf() < 0.5 else -1.0
	_apply_variant()

func _apply_variant() -> void:
	match variant:
		"speeder":
			speed *= 1.15
			if body: body.modulate = Color("#FFE600")
		"diver":
			if body: body.modulate = Color("#FF3B3B")
		_:
			if body: body.modulate = Color("#5BE0A0")

func _physics_process(delta: float) -> void:
	_t += delta
	_animate(delta)
	var pos := global_position
	pos.x += _dir * speed * delta
	if pos.x > _origin.x + span:
		pos.x = _origin.x + span
		_dir = -1.0
	elif pos.x < _origin.x - span:
		pos.x = _origin.x - span
		_dir = 1.0
	var target_y := _origin.y + sin(_t * 1.5) * 10.0
	if variant == "diver":
		var pl := _player()
		if pl and absf(pl.global_position.x - pos.x) < 90.0:
			target_y = pl.global_position.y - 18.0
	pos.y = lerpf(pos.y, target_y, 0.08)
	global_position = pos

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
