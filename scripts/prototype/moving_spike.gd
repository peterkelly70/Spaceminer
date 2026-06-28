extends Area2D
class_name MovingSpike

# A spike that slides back and forth between its origin and origin+travel.

signal player_hit

@export var travel: float = 64.0
@export var speed: float = 60.0
@export var vertical: bool = true

var _origin: Vector2 = Vector2.ZERO
var _dir: float = 1.0

func _ready() -> void:
	add_to_group("hazard")
	body_entered.connect(_on_body_entered)
	_origin = global_position

func _process(delta: float) -> void:
	var pos := global_position
	var step := speed * delta * _dir
	if vertical:
		pos.y += step
		if pos.y >= _origin.y + travel:
			pos.y = _origin.y + travel
			_dir = -1.0
		elif pos.y <= _origin.y:
			pos.y = _origin.y
			_dir = 1.0
	else:
		pos.x += step
		if pos.x >= _origin.x + travel:
			pos.x = _origin.x + travel
			_dir = -1.0
		elif pos.x <= _origin.x:
			pos.x = _origin.x
			_dir = 1.0
	global_position = pos

func _on_body_entered(b: Node) -> void:
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		player_hit.emit()
