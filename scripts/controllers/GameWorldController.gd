extends Node2D

@export var player_scene: PackedScene = preload("res://scenes/units/Player.tscn")
@export var enemy_scene: PackedScene = preload("res://scenes/units/Enemy.tscn")
@export var enemy_spawn_interval: float = 1.2
@export var max_enemies: int = 25
@export var hud_scene: PackedScene = preload("res://scenes/ui/HUD.tscn")

var _player: Node2D
var _spawn_cd: float = 0.0
var _is_playing: bool = false
var _hud: CanvasLayer
var score: int = 0
var _elapsed: float = 0.0

func _ready() -> void:
	print("GameWorldController ready")
	add_to_group("gameworld")
	State_Manager.register_state_aware_view(self)
	State_Manager.register_state_emitter(self)
	# Sync to current state
	receive_state_ping(State_Manager.get_current_state())

func receive_state_ping(state: int) -> void:
	var is_playing := state == AppState.State.PLAYING
	visible = is_playing
	if is_playing:
		_on_enter_playing()
	else:
		_on_exit_playing()

func _on_enter_playing() -> void:
	if _player and is_instance_valid(_player):
		return
	var units := get_node("Units")
	_player = player_scene.instantiate()
	units.add_child(_player)
	_player.global_position = get_viewport_rect().size * 0.5
	# HUD
	if not _hud:
		_hud = hud_scene.instantiate()
		add_child(_hud)
		_update_hud()
	_spawn_cd = 0.0
	_is_playing = true

func _on_exit_playing() -> void:
	_is_playing = false
	if _hud and is_instance_valid(_hud):
		_hud.queue_free()
		_hud = null

func _process(delta: float) -> void:
	if not _is_playing:
		return
	_elapsed += delta
	# Gradually ramp difficulty
	enemy_spawn_interval = maxf(0.3, enemy_spawn_interval - 0.05 * delta)
	max_enemies = clamp(10 + int(_elapsed * 0.5), 10, 200)
	_spawn_cd -= delta
	if _spawn_cd <= 0.0:
		_spawn_cd = enemy_spawn_interval
		_maybe_spawn_enemy()

func _maybe_spawn_enemy() -> void:
	var units := get_node("Units")
	if units.get_child_count() > max_enemies + 1: # +1 for player
		return
	var e := enemy_scene.instantiate()
	units.add_child(e)
	e.global_position = _random_edge_position()
	if e.has_signal("killed"):
		e.connect("killed", Callable(self, "_on_enemy_killed"))

func _random_edge_position() -> Vector2:
	var rect := get_viewport_rect()
	var w := rect.size.x
	var h := rect.size.y
	var side := randi() % 4
	match side:
		0: # top
			return Vector2(randf() * w, -40)
		1: # bottom
			return Vector2(randf() * w, h + 40)
		2: # left
			return Vector2(-40, randf() * h)
		_:
			return Vector2(w + 40, randf() * h)

func _on_enemy_killed() -> void:
	score += 100
	_update_hud()

func _update_hud() -> void:
	if _hud:
		if _hud.has_method("set_score"):
			_hud.set_score(score)

func set_player_shield(current: float, maximum: float) -> void:
	if _hud and _hud.has_method("set_shield"):
		_hud.set_shield(current, maximum)
