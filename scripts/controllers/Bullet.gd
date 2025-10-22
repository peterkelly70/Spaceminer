extends Node2D

@export var velocity: Vector2 = Vector2.ZERO
@export var lifetime: float = 1.5
@export var radius: float = 3.0
@export var color: Color = Color(1, 0.9, 0.3, 1)

var _time: float = 0.0

func _ready() -> void:
	add_to_group("bullets")

func init(v: Vector2) -> void:
	velocity = v

func _process(delta: float) -> void:
	_time += delta
	if _time >= lifetime:
		queue_free()
		return
	global_position += velocity * delta
	queue_redraw()

func _draw() -> void:
	draw_circle(Vector2.ZERO, radius, color)
