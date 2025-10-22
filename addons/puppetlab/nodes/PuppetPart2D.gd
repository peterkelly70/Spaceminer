@tool
extends Sprite2D
@class_name PuppetPart2D

@export var anchor: Vector2 = Vector2(0.5, 0.5) : set = set_anchor

func set_anchor(a: Vector2) -> void:
	anchor = a
	centered = true
	offset = -texture.get_size() * (anchor - Vector2(0.5, 0.5))
