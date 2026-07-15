extends Area2D
class_name BatteryPack

signal collected(amount: float)

# Shared battery powers: magnetic boots, shield, incindiery
@export var amount: float = 40.0  # percent of max battery restored

@onready var sprite: Sprite2D = $Sprite2D

var _phase := 0.0

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _process(delta: float) -> void:
	_phase += delta * 8.0
	sprite.modulate = Color.from_hsv(0.75, 0.8, 0.5 + sin(_phase) * 0.5)

func _on_body_entered(body: Node) -> void:
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return
	collected.emit(amount)
	queue_free()
