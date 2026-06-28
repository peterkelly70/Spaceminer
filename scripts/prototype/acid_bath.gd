extends Area2D
class_name AcidBath

# Acid bath hazard with periodic bubble emissions.

signal player_hit

@export var bubble_interval: float = 0.5  # seconds between bubble spawns
@export var bubble_count: int = 2         # bubbles per spawn burst

var _bubble_timer: float = 0.0

func _ready() -> void:
	add_to_group("hazard")
	body_entered.connect(_on_body_entered)
	_bubble_timer = randf_range(0, bubble_interval)

func _process(delta: float) -> void:
	_bubble_timer -= delta
	if _bubble_timer <= 0:
		_bubble_timer = bubble_interval
		_spawn_bubbles()

func _spawn_bubbles() -> void:
	for i in range(bubble_count):
		var bubble := Node2D.new()
		bubble.name = "Bubble_%d" % i
		bubble.position = global_position + Vector2(randf_range(-30, 30), 0)
		add_sibling(bubble)

		var sprite := Sprite2D.new()
		sprite.texture = load("res://assets/tiles/Transparent/tile_0000.png")  # small circle
		sprite.modulate = Color(0.6, 1.0, 0.5, 0.7)
		sprite.scale = Vector2(0.5, 0.5)
		bubble.add_child(sprite)

		var lifetime := 1.0
		var tween := bubble.create_tween()
		tween.set_trans(Tween.TRANS_CUBIC)
		tween.set_ease(Tween.EASE_OUT)
		tween.parallel().tween_property(bubble, "position", bubble.position + Vector2(randf_range(-20, 20), -60), lifetime)
		tween.parallel().tween_property(sprite, "scale", Vector2(0.1, 0.1), lifetime)
		tween.parallel().tween_property(sprite, "modulate:a", 0.0, lifetime)
		tween.tween_callback(bubble.queue_free)

func _on_body_entered(b: Node) -> void:
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		player_hit.emit()
