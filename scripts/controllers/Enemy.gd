extends CharacterBody2D
signal killed

const Logger = preload("res://scripts/class/Logger.gd")

@export var max_speed: float = 200.0
@export var accel_lerp: float = 5.0
@export var turn_speed: float = 8.0
@export var radius: float = 16.0
@export var health: int = 1

# Visual forward direction of the sprite (nose). Enemy sprite faces UP.
@export var forward_basis: Vector2 = Vector2.UP

# Shooting
@export var fire_interval: float = 0.5
@export var bullet_speed: float = 600.0
@export var bullet_scene: PackedScene = preload("res://scenes/units/Bullet.tscn")

var _fire_cooldown: float = 0.0

func _ready() -> void:
	add_to_group("enemies")

func _physics_process(delta: float) -> void:
	var player := _get_player()
	if not player:
		return
	var to_player := player.global_position - global_position
	var dist := to_player.length()
	if dist > 0.001:
		var target_angle := to_player.angle() - forward_basis.angle()
		rotation = lerp_angle(rotation, target_angle, clamp(turn_speed * delta, 0.0, 1.0))
	var desired := to_player.normalized() * max_speed
	velocity = velocity.lerp(desired, clamp(accel_lerp * delta, 0.0, 1.0))
	move_and_slide()
	_check_bullet_hits()

	_fire_cooldown = maxf(0.0, _fire_cooldown - delta)
	if _fire_cooldown <= 0.0:
		_fire_at_player()
		_fire_cooldown = fire_interval

func _get_player() -> Node2D:
	return get_tree().get_first_node_in_group("player")

func _check_bullet_hits() -> void:
	for b in get_tree().get_nodes_in_group("player_bullet"):
		if not is_instance_valid(b) or not (b is Node2D):
			continue
		var d: float = (b.global_position - global_position).length()
		var br: float = b.radius
		if d <= (radius + br):
			_take_damage(1)
			b.queue_free()

func _take_damage(amount: int) -> void:
	health -= amount
	if health <= 0:
		emit_signal("killed")
		queue_free()

func _fire_at_player() -> void:
	var b: Node2D = bullet_scene.instantiate()
	get_tree().current_scene.add_child(b)
	var dir := forward_basis.rotated(rotation)
	var origin := global_position
	var muzzle := get_node_or_null("Muzzle")
	if muzzle and muzzle is Node2D:
		origin = muzzle.global_position
	b.global_position = origin
	b.add_to_group("enemy_bullet")
	if "color" in b:
		b.color = Color(1.0, 0.35, 0.35, 1.0)
	if b.has_method("init"):
		b.init(dir * bullet_speed)
	else:
		if "velocity" in b:
			b.velocity = dir * bullet_speed
