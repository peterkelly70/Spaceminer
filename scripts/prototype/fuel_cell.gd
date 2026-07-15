extends Area2D
class_name FuelCell

signal collected(amount: float)

@export var amount: float = 30.0  # percent of max fuel restored

@onready var sprite: Sprite2D = $Sprite2D

var _phase := 0.0

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _process(delta: float) -> void:
	_phase += delta * 6.0
	sprite.modulate = Color.from_hsv(fmod(_phase * 0.03 + 0.1, 1.0), 0.9, 1.0)

func _on_body_entered(body: Node) -> void:
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return
	collected.emit(amount)
	queue_free()
