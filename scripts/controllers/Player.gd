extends CharacterBody2D

@export var max_speed: float = 420.0
@export var arrival_radius: float = 160.0
@export var accel_lerp: float = 6.0
@export var turn_speed: float = 10.0

# The sprite's visual forward direction (nose). Many ship sprites face UP.
# This defines what direction corresponds to rotation = 0.
@export var forward_basis: Vector2 = Vector2.UP

@export var fire_interval: float = 0.12
@export var bullet_speed: float = 900.0
@export var bullet_scene: PackedScene = preload("res://scenes/units/Bullet.tscn")
@export var bullet_nose_inset: float = 2.0 # pixels pulled back from sprite nose

@export var dash_speed: float = 1400.0
@export var dash_duration: float = 0.18
@export var dash_cooldown: float = 0.6

var _fire_cooldown: float = 0.0
var _dash_time_left: float = 0.0
var _dash_cooldown_left: float = 0.0

# Shield & survivability
@export var radius: float = 16.0
@export var shield_max: float = 100.0
@export var shield_regen_rate: float = 15.0 # per second
@export var shield_hit_cooldown: float = 0.1 # seconds without hits to start regen
var shield: float = 100.0
var _since_last_hit: float = 0.0
var _invulnerable: bool = false

signal player_killed

@onready var _muzzle: Node2D = %Muzzle
@onready var _sprite: Sprite2D = %Sprite

func _ready() -> void:
	print("Player ready")
	add_to_group("player")
	shield = shield_max

func _physics_process(delta: float) -> void:
	# Update timers
	_fire_cooldown = maxf(0.0, _fire_cooldown - delta)
	_dash_cooldown_left = maxf(0.0, _dash_cooldown_left - delta)
	_dash_time_left = maxf(0.0, _dash_time_left - delta)

	var mouse_global := get_global_mouse_position()
	var to_mouse := mouse_global - global_position
	var dist := to_mouse.length()

	# Desired heading always faces cursor, smoothed, accounting for sprite basis
	if dist > 0.001:
		var basis_angle := forward_basis.angle()
		var target_angle := to_mouse.angle() - basis_angle
		rotation = lerp_angle(rotation, target_angle, clampf(turn_speed * delta, 0.0, 1.0))

	if _dash_time_left > 0.0:
		# During dash, maintain high forward velocity and ignore steering
		velocity = forward_basis.rotated(rotation) * dash_speed
	else:
		# Steering with arrival: target speed scales down when close to cursor
		var target_speed := max_speed
		if dist < arrival_radius:
			target_speed = max_speed * (dist / arrival_radius)
		var desired := to_mouse.normalized() * target_speed
		velocity = velocity.lerp(desired, clampf(accel_lerp * delta, 0.0, 1.0))

	move_and_slide()

	_handle_fire()
	_handle_dash()

	# I-frames active during dash
	_invulnerable = _dash_time_left > 0.0

	# Shield regen logic
	_since_last_hit += delta
	if _since_last_hit >= shield_hit_cooldown and shield < shield_max:
		shield = minf(shield_max, shield + shield_regen_rate * delta)
		_notify_hud()

func _handle_fire() -> void:
	if Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT):
		if _fire_cooldown <= 0.0:
			_fire()
			_fire_cooldown = fire_interval

	_check_enemy_bullets()

func _check_enemy_bullets() -> void:
	for b in get_tree().get_nodes_in_group("enemy_bullet"):
		if not is_instance_valid(b) or not (b is Node2D):
			continue
		var d: float = (b.global_position - global_position).length()
		var br: float = b.radius
		if d <= (radius + br):
			_on_hit(1)
			b.queue_free()

func _on_hit(damage: float) -> void:
	if _invulnerable:
		return
	shield = maxf(0.0, shield - damage)
	_since_last_hit = 0.0
	_notify_hud()
	if shield <= 0.0:
		emit_signal("player_killed")
		queue_free()

func _notify_hud() -> void:
	var gw := get_tree().get_first_node_in_group("gameworld")
	if gw and gw.has_method("set_player_shield"):
		gw.set_player_shield(shield, shield_max)

func _fire() -> void:
	var b: Node2D = bullet_scene.instantiate()
	get_tree().current_scene.add_child(b)
	var dir := forward_basis.rotated(rotation)
	# Default: compute visual nose from sprite size and facing basis
	var origin := global_position
	if _sprite and _sprite.texture:
		var tex_size := _sprite.texture.get_size()
		var forward := forward_basis.rotated(rotation).normalized()
		var half_len: float = float(tex_size.y) * abs(_sprite.scale.y) * 0.5
		origin = global_position + forward * maxf(0.0, half_len - bullet_nose_inset)
	# If a Muzzle is placed away from center (> 1px), use that instead
	if _muzzle and (_muzzle is Node2D) and _muzzle.global_position.distance_to(global_position) > 1.0:
		origin = _muzzle.global_position
	b.global_position = origin
	b.add_to_group("player_bullet")
	if "color" in b:
		b.color = Color(0.4, 0.8, 1.0, 1.0)
	# Provide an init if present
	if b.has_method("init"):
		b.init(dir * bullet_speed)
	else:
		# Fallback: set velocity property if exposed
		if "velocity" in b:
			b.velocity = dir * bullet_speed

func _handle_dash() -> void:
	if Input.is_mouse_button_pressed(MOUSE_BUTTON_RIGHT):
		if _dash_cooldown_left <= 0.0 and _dash_time_left <= 0.0:
			_dash_time_left = dash_duration
			_dash_cooldown_left = dash_cooldown
