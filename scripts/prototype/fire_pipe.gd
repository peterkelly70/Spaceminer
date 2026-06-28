extends Area2D
class_name FirePipe

# A pipe that periodically jets flame. Only hurts while the flame is active.

signal player_hit

@export var period: float = 1.8     # full on+off cycle
@export var on_time: float = 0.7    # how long the flame is active each cycle
@export var phase: float = 0.0      # stagger so neighbouring pipes differ
@export var direction: String = "up"  # up | down | left | right

var _t: float = 0.0
var _active: bool = false

@onready var flame: Sprite2D = $Flame
@onready var body: Sprite2D = $Body

func _ready() -> void:
	add_to_group("hazard")
	body_entered.connect(_on_body_entered)
	_t = phase
	_apply_direction()
	_refresh()

func _apply_direction() -> void:
	var rotation_angle := 0.0
	match direction:
		"down":
			rotation_angle = PI
		"left":
			rotation_angle = PI / 2.0
		"right":
			rotation_angle = -PI / 2.0
	if body:
		body.rotation = rotation_angle
	if flame:
		flame.rotation = rotation_angle

func _process(delta: float) -> void:
	_t += delta
	var was := _active
	_active = fmod(_t, period) < on_time
	if was != _active:
		_refresh()
		if _active:
			for b in get_overlapping_bodies():
				_on_body_entered(b)

func _refresh() -> void:
	if flame:
		flame.visible = _active
		if _active:
			_spawn_fire_particles()

func _spawn_fire_particles() -> void:
	for i in range(3):
		var particle := Node2D.new()
		var rot_rad := 0.0
		match direction:
			"down":
				rot_rad = PI
			"left":
				rot_rad = PI / 2.0
			"right":
				rot_rad = -PI / 2.0
		var angle := rot_rad + randf_range(-0.3, 0.3)
		var speed := 80.0
		var vel := Vector2(cos(angle), sin(angle)) * speed

		var sprite := Sprite2D.new()
		sprite.texture = load("res://assets/tiles/Transparent/tile_0022.png")  # use gem tile
		sprite.modulate = Color(1, 0.7, 0.2, 0.8)
		sprite.scale = Vector2(0.6, 0.6)
		particle.add_child(sprite)

		particle.position = global_position
		add_sibling(particle)

		var lifetime := 0.4
		var tween := particle.create_tween()
		tween.set_trans(Tween.TRANS_QUAD)
		tween.set_ease(Tween.EASE_OUT)
		tween.parallel().tween_property(particle, "position", particle.position + vel * lifetime, lifetime)
		tween.parallel().tween_property(sprite, "scale", Vector2(0.1, 0.1), lifetime)
		tween.parallel().tween_property(sprite, "modulate:a", 0.0, lifetime)
		tween.tween_callback(particle.queue_free)

func _on_body_entered(b: Node) -> void:
	if not _active:
		return
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		player_hit.emit()
