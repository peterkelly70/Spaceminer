extends Area2D
class_name AirCanister

signal collected(amount: float)

@export var amount: float = 25.0  # percent of max air restored

@onready var sprite: Sprite2D = $Sprite2D

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _process(delta: float) -> void:
	sprite.rotation += delta * 1.2

func _on_body_entered(body: Node) -> void:
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return
	collected.emit(amount)
	queue_free()
