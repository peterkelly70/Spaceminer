extends CharacterBody2D
class_name PrototypePlayer

signal player_defeated

@export var move_speed: float = 180.0
@export var gravity:    float = 1000.0
@export var air_control_accel: float = 2200.0
@export var ground_friction: float = 3200.0
# Jump height = JUMP_SPEED² / (2·gravity) = 340²/2000 ≈ 58 px, clearing the
# 48 px LEVEL_STEP between platform levels with margin.
const JUMP_SPEED := 340.0
const JUMP_HORIZONTAL_SPEED := 150.0

# Jetpack
const JETPACK_THRUST         := 1040.0  # counters gravity each physics tick
const JETPACK_FUEL_DRAIN      := 8.0     # % per second while thrusting
const JETPACK_IGNITION_DELAY  := 0.08    # moonlander-style spool up

# Grappling hook
const GRAPPLE_RANGE        := 320.0
const GRAPPLE_PULL         := 550.0
const GRAPPLE_REEL_SPEED   := 1200.0
const GRAPPLE_REEL_SECONDS  := 0.16

# Laser pistol
const LASER_RANGE    := 600.0
const LASER_DURATION := 0.10        # seconds the beam stays visible

# Magnetic boots
const BOOT_DRAIN := 12.0            # battery % per second when active
const MAGNET_SNAP_RANGE := 24.0
const MAGNET_STICK_FORCE := 2400.0
const MAGNET_ROTATE_SPEED := 16.0
const CLOAK_DRAIN_PER_DAMAGE := 18.0

# ── State ──────────────────────────────────────────────────────────────────────
var facing_dir       := 1.0
var _fall_sfx_played := false

var _grapple_anchor      := Vector2.ZERO
var _grappling           := false
var _grapple_reeling     := false
var _grapple_reel_timer  := 0.0
var _grapple_line        : Line2D = null
var _grapple_aiming      := false
var _grapple_aim_phase   := 0.0          # drives oscillation across arc
var _grapple_aim_dir     := Vector2.UP
var _grapple_arc         : Line2D = null  # semicircle indicator
var _grapple_aim_line    : Line2D = null  # translucent direction needle

const GRAPPLE_AIM_RADIUS := 72.0
const GRAPPLE_AIM_SPEED  := 2.2           # radians/sec of sweep

var _mag_active      := false
var _mag_attached    := false
var _mag_surface_normal := Vector2.UP
var _cloak_active := false
var _jetpack_sfx_armed := false
var _jetpack_hold_time := 0.0
var _jetpack_active := false
var _jump_active := false
var _jump_lock_dir := 1.0

# Ladder climbing
const CLIMB_SPEED := 110.0
const FEET_OFFSET := 16.0          # half of the 32px collision box
const LADDER_JUMP_IGNORE_SECONDS := 0.3  # grace period after jumping off so we don't instantly re-grab it
var _climbing := false
var _climb_saved_mask := 0
var _ladder_ignore_timer := 0.0

# Drop-through: one-way platforms live on collision layer 2 (mask bit 2). Clearing
# that bit briefly lets the player fall down through the platform they're stood on.
const PLATFORM_MASK_BIT := 2
const DROP_THROUGH_TIME := 0.20
var _drop_timer := 0.0

# Equipment hotkeys: tap = activate, hold = drop the item where the player stands.
const EQUIPMENT_SLOT_COUNT := 6
const EQUIPMENT_DROP_HOLD_SECONDS := 0.6
const EQUIPMENT_PICKUP_SCENE := preload("res://scenes/prototype/EquipmentPickup.tscn")
const DROPPED_PICKUP_GRACE_SECONDS := 1.2
var _eq_hold_time: Array[float] = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
var _eq_hold_consumed: Array[bool] = [false, false, false, false, false, false]

var _laser_line      : Line2D = null
var _laser_timer     : float  = 0.0
var _jetpack_rig     : Node2D = null
var _jetpack_pack    : Polygon2D = null
var _jetpack_flame_l : Polygon2D = null
var _jetpack_flame_r : Polygon2D = null
var _grapple_head    : Polygon2D = null
var _grounded_last_frame := false
var _fall_origin_y := 0.0

@onready var body_sprite: AnimatedSprite2D = $Body

# ── Lifecycle ─────────────────────────────────────────────────────────────────

func _ready() -> void:
	add_to_group("prototype_player")
	add_to_group("player")
	_ensure_fx_nodes()
	_update_jetpack_visuals(false, false)
	_update_grapple_fx(false)

func respawn(at_position: Vector2) -> void:
	global_position  = at_position
	velocity         = Vector2.ZERO
	_fall_sfx_played = false
	_grappling       = false
	_grapple_reeling = false
	_grapple_reel_timer = 0.0
	_mag_active      = false
	_mag_attached = false
	_mag_surface_normal = Vector2.UP
	_jetpack_hold_time = 0.0
	_jetpack_active = false
	_jump_active = false
	_jump_lock_dir = 1.0
	rotation = 0.0
	body_sprite.rotation = 0.0
	body_sprite.modulate = Color.WHITE
	if _climbing:
		collision_mask = _climb_saved_mask
		_climbing = false
	if _drop_timer > 0.0:
		_drop_timer = 0.0
		collision_mask |= PLATFORM_MASK_BIT
	_grounded_last_frame = false
	_fall_origin_y = at_position.y
	if _grapple_line:
		_grapple_line.queue_free()
		_grapple_line = null
	_update_jetpack_visuals(false, false)
	_update_grapple_fx(false)

# ── Input (single-press actions) ──────────────────────────────────────────────

func _unhandled_input(event: InputEvent) -> void:
	if not event.is_pressed() or event.is_echo():
		return

	if event.is_action_pressed("boots_toggle") and _has_equipment("magnetic_boots"):
		_mag_active = not _mag_active
		get_viewport().set_input_as_handled()
		return

	if event.is_action_pressed("grapple") and _has_equipment("grappling_hook"):
		_begin_grapple_aim()
		get_viewport().set_input_as_handled()
		return

	# Laser: laser_fire action or Shift+movement
	if event.is_action_pressed("laser_fire") and _has_equipment("laser_pistol"):
		_fire_laser(facing_dir)
		get_viewport().set_input_as_handled()
		return

	if _has_equipment("laser_pistol") and (event is InputEventKey) and event.shift_pressed:
		if event.is_action_pressed("move_left"):
			_fire_laser(-1.0)
			get_viewport().set_input_as_handled()
		elif event.is_action_pressed("move_right"):
			_fire_laser(1.0)
			get_viewport().set_input_as_handled()

	# Equipment slots — activate equipment by slot index
	# Equipment slots are polled in _physics_process (_update_equipment_hotkeys):
	# tap activates, holding drops the item at the player's feet.

	# Map overlay toggle
	if event.is_action_pressed("map_toggle"):
		get_tree().call_group("map_overlay", "toggle")
		get_viewport().set_input_as_handled()

# ── Physics ───────────────────────────────────────────────────────────────────

func _physics_process(delta: float) -> void:
	var h            := _h_input()
	var jump_pressed := Input.is_action_just_pressed("jump") and not Input.is_key_pressed(KEY_SHIFT)
	var grounded    := is_on_floor()
	var has_jetpack := _has_equipment("jetpack")
	var has_cloak := _has_equipment("visibility_cloak")

	# Restore one-way platform collision once the drop-through window elapses.
	if _drop_timer > 0.0:
		_drop_timer -= delta
		if _drop_timer <= 0.0:
			collision_mask |= PLATFORM_MASK_BIT

	# Reset on landing
	if grounded:
		_fall_sfx_played = false
		_fall_origin_y = global_position.y
		_grappling       = false
		_grapple_reeling = false
		_grapple_reel_timer = 0.0
		_jetpack_hold_time = 0.0
		_jetpack_sfx_armed = false
		_jetpack_active = false
		_jump_active = false
		if _grapple_line and not _grappling:
			_grapple_line.queue_free()
			_grapple_line = null
		_grapple_aiming = false
		_clear_grapple_aim_visuals()

	# Grapple aiming / release
	if _grapple_aiming:
		_update_grapple_aim(delta)
		if Input.is_action_just_released("grapple"):
			_fire_grapple(_grapple_aim_dir)
			_grapple_aiming = false
			_clear_grapple_aim_visuals()
	elif _grappling or _grapple_reeling:
		# F or jump releases an attached/reeling grapple
		if Input.is_action_just_pressed("grapple") or Input.is_action_just_pressed("jump"):
			_grappling = false
			_grapple_reeling = false
			_grapple_reel_timer = 0.0
			if _grapple_line:
				_grapple_line.queue_free()
				_grapple_line = null
			_update_grapple_fx(false)
	elif not Input.is_action_pressed("grapple"):
		_grappling = false
		if _grapple_reeling:
			_grapple_reel_timer = maxf(_grapple_reel_timer, 0.02)

	if jump_pressed and has_jetpack and not _climbing and not _grapple_aiming and not _grappling and not _grapple_reeling:
		# Jetpack can be armed straight off the ground/a platform, not just
		# mid-air — press the jump key to lift off directly.
		_jetpack_active = true
		jump_pressed = false
	elif grounded:
		_jetpack_active = false

	# Ladder climbing — up/down move along a ladder; jetpack is disabled while climbing
	if _ladder_ignore_timer > 0.0:
		_ladder_ignore_timer = maxf(_ladder_ignore_timer - delta, 0.0)
	var ladder := _current_ladder()
	var climb_up := _up_held()
	var climb_down := _down_held()
	if ladder and not _climbing and _ladder_ignore_timer <= 0.0 and (climb_up or climb_down):
		_begin_climb()
	if _climbing and ladder == null:
		_end_climb()

	# Drop through a one-way platform: hold down + jump while standing on one.
	if not _climbing and grounded and climb_down and jump_pressed:
		_drop_through_platform()
		jump_pressed = false

	# Jetpack uses an airborne jump-press to arm, then WASD/arrow keys steer.
	var thrusting := _apply_jetpack_thrust(_jetpack_active and not _climbing, has_jetpack, climb_up, climb_down, delta)
	if not has_jetpack or _get_fuel() <= 0.0:
		_jetpack_active = false
	_update_jetpack_visuals(has_jetpack, thrusting)

	# Grapple pull
	if _grappling:
		_apply_grapple(delta)
	elif _grapple_reeling:
		_apply_grapple_reel(delta)

	# Magnetic boots
	var boots_attached := false
	if _mag_active and _has_equipment("magnetic_boots"):
		boots_attached = _apply_magnetic_boots(h, jump_pressed, delta)
	else:
		_mag_attached = false
		up_direction = Vector2.UP
		body_sprite.rotation = lerp_angle(body_sprite.rotation, 0.0, clampf(delta * MAGNET_ROTATE_SPEED, 0.0, 1.0))

	if not boots_attached:
		_apply_horizontal_move(h, grounded and not _climbing, thrusting, delta)
		if _climbing:
			_apply_ladder_climb(ladder, h, jump_pressed, climb_up, climb_down, delta)
		else:
			_apply_vertical_move(jump_pressed, grounded, has_jetpack, thrusting, delta)

	# Laser line fade
	if _laser_line:
		_laser_timer -= delta
		if _laser_timer <= 0.0:
			_laser_line.queue_free()
			_laser_line = null

	_update_equipment_hotkeys(delta)

	_cloak_active = has_cloak and _get_battery() > 0.0
	_update_cloak_visuals()
	_update_animation(h, grounded or _climbing or boots_attached)
	move_and_slide()
	_grounded_last_frame = grounded

func handle_hazard_hit(damage: float = 1.0) -> bool:
	if _cloak_active and _has_equipment("visibility_cloak") and _get_battery() > 0.0:
		var battery_cost := maxf(damage, 0.0) * CLOAK_DRAIN_PER_DAMAGE
		_drain_battery(battery_cost)
		_cloak_active = _get_battery() > 0.0
		_update_cloak_visuals()
		return false
	player_defeated.emit()
	return true

# ── Movement ─────────────────────────────────────────────────────────────────

func _h_input() -> float:
	var left  := Input.is_action_pressed("move_left")
	var right := Input.is_action_pressed("move_right")
	if left == right: return 0.0
	return -1.0 if left else 1.0

# Vertical intent — W/Up climbs ladders; jump can also feed the jetpack once airborne.
func _up_held() -> bool:
	return Input.is_action_pressed("move_up") or Input.is_action_pressed("ui_up")

func _down_held() -> bool:
	return Input.is_action_pressed("move_down") or Input.is_action_pressed("ui_down")

# Temporarily stop colliding with one-way platforms so gravity carries the player
# down through the platform they're standing on. Solid floor/walls (layer 1) stay
# active, so the player still lands on the next surface below.
func _drop_through_platform() -> void:
	if _drop_timer > 0.0:
		return
	collision_mask &= ~PLATFORM_MASK_BIT
	_drop_timer = DROP_THROUGH_TIME
	# Nudge downward so we immediately separate from the platform surface.
	velocity.y = maxf(velocity.y, 40.0)
	global_position.y += 1.0

func _apply_horizontal_move(h: float, grounded: bool, air_control: bool, delta: float) -> void:
	if grounded:
		if h != 0.0:
			velocity.x = h * move_speed
			facing_dir = sign(h)
			body_sprite.flip_h = facing_dir < 0.0
		else:
			velocity.x = move_toward(velocity.x, 0.0, ground_friction * delta)
			if absf(velocity.x) < 1.0:
				velocity.x = 0.0
		return

	# Old-school fixed jump arc: horizontal speed is locked at takeoff and cannot
	# be steered mid-air. Only the jetpack (a deliberate equipment override) grants
	# air control, via the `air_control` flag passed in while thrusting.
	if air_control and h != 0.0:
		velocity.x = move_toward(velocity.x, h * move_speed, air_control_accel * delta)
		facing_dir = sign(h)
		body_sprite.flip_h = facing_dir < 0.0

func _apply_magnetic_boots(h: float, jump_pressed: bool, delta: float) -> bool:
	if not _mag_active or not _has_equipment("magnetic_boots"):
		_mag_attached = false
		return false
	if _get_battery() <= 0.0:
		_mag_active = false
		_mag_attached = false
		return false

	_drain_battery(BOOT_DRAIN * delta)
	if _get_battery() <= 0.0:
		_mag_active = false
		_mag_attached = false
		return false

	var probe := _probe_magnetic_surface()
	if not probe.is_empty():
		_mag_surface_normal = probe.get("normal", _mag_surface_normal)
		_mag_attached = true
	elif not _mag_attached:
		up_direction = Vector2.UP
		return false

	if _mag_surface_normal == Vector2.ZERO:
		_mag_surface_normal = Vector2.UP

	# Treat the contacted surface as "floor" and keep the player glued to it.
	up_direction = _mag_surface_normal
	var surface_rotation := (-_mag_surface_normal).angle() - PI / 2.0
	body_sprite.rotation = lerp_angle(body_sprite.rotation, surface_rotation, clampf(delta * MAGNET_ROTATE_SPEED, 0.0, 1.0))

	var tangent := Vector2(_mag_surface_normal.y, -_mag_surface_normal.x)
	var tangential_speed := velocity.dot(tangent)
	if h != 0.0:
		tangential_speed = move_toward(tangential_speed, h * move_speed, air_control_accel * delta)
	elif absf(tangential_speed) > 1.0:
		tangential_speed = move_toward(tangential_speed, 0.0, ground_friction * delta)
	else:
		tangential_speed = 0.0
	if jump_pressed:
		velocity = tangent * tangential_speed + _mag_surface_normal * -JUMP_SPEED * 0.85
		_mag_attached = false
		_mag_active = false
		return false

	var stick_speed := maxf(MAGNET_STICK_FORCE * delta, 80.0)
	velocity = tangent * tangential_speed + (-_mag_surface_normal * stick_speed)
	facing_dir = sign(tangent.dot(Vector2.RIGHT) * h if h != 0.0 else facing_dir)
	if h != 0.0:
		body_sprite.flip_h = h < 0.0
	return true

func _probe_magnetic_surface() -> Dictionary:
	var probes := [
		{"offset": Vector2(0.0, 14.0), "dir": Vector2.DOWN},
		{"offset": Vector2(0.0, -14.0), "dir": Vector2.UP},
		{"offset": Vector2(-12.0, 0.0), "dir": Vector2.LEFT},
		{"offset": Vector2(12.0, 0.0), "dir": Vector2.RIGHT},
	]
	var best := {}
	var best_dist := INF
	for probe in probes:
		var from: Vector2 = global_position + probe["offset"]
		var to: Vector2 = from + probe["dir"] * MAGNET_SNAP_RANGE
		var query := PhysicsRayQueryParameters2D.create(from, to, 0x7FFFFFFF)
		query.exclude = [self]
		var result := get_world_2d().direct_space_state.intersect_ray(query)
		if result.is_empty():
			continue
		var dist: float = from.distance_to(result["position"])
		if dist < best_dist:
			best_dist = dist
			best = result
	return best

func _apply_jetpack_thrust(active: bool, has_jetpack: bool, up_held: bool, down_held: bool, delta: float) -> bool:
	if not has_jetpack or not active:
		_jetpack_hold_time = 0.0
		_jetpack_sfx_armed = false
		return false

	var thrust_dir := 0.0
	if up_held:
		thrust_dir -= 1.0
	if down_held:
		thrust_dir += 1.0

	if is_zero_approx(thrust_dir) or _get_fuel() <= 0.0:
		_jetpack_hold_time = 0.0
		_jetpack_sfx_armed = false
		return false

	_jetpack_hold_time += delta
	velocity.y += thrust_dir * JETPACK_THRUST * delta
	_drain_fuel(JETPACK_FUEL_DRAIN * delta)
	if not _jetpack_sfx_armed:
		_play_jetpack_sfx()
		_jetpack_sfx_armed = true
	return true

func _apply_vertical_move(jump_pressed: bool, grounded: bool, has_jetpack: bool, thrusting: bool, delta: float) -> void:
	if jump_pressed and grounded:
		_jump_active = true
		_jump_lock_dir = facing_dir
		velocity.y = -JUMP_SPEED
		velocity.x = _jump_lock_dir * JUMP_HORIZONTAL_SPEED
		_play_jump_sfx()
		_fall_sfx_played = false

	if has_jetpack and thrusting:
		_jump_active = false
		velocity.y += gravity * delta
		return

	velocity.y += gravity * delta
	if not grounded and _grounded_last_frame and not _fall_sfx_played and velocity.y > 250.0:
		if global_position.y - _fall_origin_y > 120.0:
			_play_fall_sfx()
			_fall_sfx_played = true

# ── Ladder climbing ────────────────────────────────────────────────────────────

func _current_ladder() -> Node:
	for l in get_tree().get_nodes_in_group("ladder"):
		if l.has_method("contains_point") and l.contains_point(global_position):
			return l
	return null

func _begin_climb() -> void:
	_climbing = true
	_jump_active = false
	_grappling = false
	_grapple_reeling = false
	_grapple_aiming = false
	_clear_grapple_aim_visuals()
	# Pass freely through one-way platforms while on the ladder
	_climb_saved_mask = collision_mask
	collision_mask = 0

func _end_climb() -> void:
	if not _climbing:
		return
	_climbing = false
	collision_mask = _climb_saved_mask

func _apply_ladder_climb(ladder: Node, h: float, jump_pressed: bool, up: bool, down: bool, delta: float) -> void:
	# Jump off the ladder — h reflects whichever direction (if any) is held so
	# the player can launch left or right, not just straight up.
	if jump_pressed:
		_end_climb()
		_ladder_ignore_timer = LADDER_JUMP_IGNORE_SECONDS
		_jump_active = true
		_jump_lock_dir = h if h != 0.0 else facing_dir
		velocity.y = -JUMP_SPEED
		velocity.x = _jump_lock_dir * JUMP_HORIZONTAL_SPEED
		_play_jump_sfx()
		return
	# Step off sideways when not actively climbing
	if h != 0.0 and not up and not down:
		_end_climb()
		return
	# Snap horizontally onto the ladder centre
	global_position.x = move_toward(global_position.x, ladder.center_x(), 160.0 * delta)
	velocity.x = 0.0
	velocity.y = 0.0

	var feet := global_position.y + FEET_OFFSET
	var top: float = ladder.top_y()
	var bottom: float = ladder.bottom_y()
	if up:
		if feet <= top + 1.0:
			# Reached the top — step onto the platform
			global_position.y = top - FEET_OFFSET
			_end_climb()
			return
		velocity.y = -CLIMB_SPEED
	elif down:
		if feet >= bottom - 1.0:
			global_position.y = bottom - FEET_OFFSET
			_end_climb()
			return
		velocity.y = CLIMB_SPEED

# ── Grappling hook ────────────────────────────────────────────────────────────

func _begin_grapple_aim() -> void:
	_grappling = false
	_grapple_reeling = false
	_grapple_reel_timer = 0.0
	_grapple_aiming = true
	_grapple_aim_phase = 0.0
	_grapple_aim_dir = Vector2.UP
	_update_grapple_fx(true)
	_ensure_grapple_aim_visuals()

func _update_grapple_aim(delta: float) -> void:
	_grapple_aim_phase += delta * GRAPPLE_AIM_SPEED
	# angle=0 → straight up (-Y), sweeps ±90° covering full upper semicircle
	var angle := sin(_grapple_aim_phase) * (PI / 2.0)
	# sin(angle)=X, -cos(angle)=-Y  →  angle=0 gives (0,-1) = straight up
	_grapple_aim_dir = Vector2(sin(angle), -cos(angle))
	_grapple_anchor = _calculate_grapple_target(_grapple_aim_dir)
	_update_grapple_aim_visuals()

func _ensure_grapple_aim_visuals() -> void:
	if not _grapple_arc or not is_instance_valid(_grapple_arc):
		_grapple_arc = Line2D.new()
		_grapple_arc.width = 2.0
		_grapple_arc.default_color = Color(0.0, 0.85, 1.0, 0.22)
		_grapple_arc.z_index = 199
		_grapple_arc.top_level = false
		add_child(_grapple_arc)
		# Draw the static semicircle arc (upper half)
		_grapple_arc.clear_points()
		var segs := 32
		for s in range(segs + 1):
			var t := float(s) / float(segs)  # 0..1
			var a := PI + t * PI              # PI → 2*PI  (upper semicircle in Godot coords)
			_grapple_arc.add_point(Vector2(cos(a), sin(a)) * GRAPPLE_AIM_RADIUS)
	if not _grapple_aim_line or not is_instance_valid(_grapple_aim_line):
		_grapple_aim_line = Line2D.new()
		_grapple_aim_line.width = 3.0
		_grapple_aim_line.default_color = Color(0.0, 0.85, 1.0, 0.55)
		_grapple_aim_line.z_index = 200
		_grapple_aim_line.top_level = false
		add_child(_grapple_aim_line)

func _update_grapple_aim_visuals() -> void:
	if not _grapple_aim_line or not is_instance_valid(_grapple_aim_line):
		return
	_grapple_aim_line.clear_points()
	_grapple_aim_line.add_point(Vector2.ZERO)
	var target_local := _grapple_anchor - global_position
	_grapple_aim_line.add_point(target_local)

func _clear_grapple_aim_visuals() -> void:
	if _grapple_arc and is_instance_valid(_grapple_arc):
		_grapple_arc.queue_free()
		_grapple_arc = null
	if _grapple_aim_line and is_instance_valid(_grapple_aim_line):
		_grapple_aim_line.queue_free()
		_grapple_aim_line = null

func _fire_grapple(dir: Vector2) -> void:
	var origin := global_position
	var query  := PhysicsRayQueryParameters2D.create(origin, origin + dir * GRAPPLE_RANGE, 0x7FFFFFFF)
	query.exclude   = [self]
	query.collide_with_areas = true
	var result := get_world_2d().direct_space_state.intersect_ray(query)
	var hit_pos := _calculate_grapple_target(dir)
	if not result.is_empty():
		hit_pos = result["position"]
		_grappling = true
		_grapple_reeling = false
	else:
		_grappling = false
		_grapple_reeling = true
		_grapple_reel_timer = GRAPPLE_REEL_SECONDS
	_grapple_anchor = hit_pos
	_play_grapple_sfx()
	_ensure_grapple_line()
	_update_grapple_fx(true)
	_update_grapple_line()

func _calculate_grapple_target(dir: Vector2) -> Vector2:
	var origin := global_position
	var query  := PhysicsRayQueryParameters2D.create(origin, origin + dir * GRAPPLE_RANGE, 0x7FFFFFFF)
	query.exclude = [self]
	query.collide_with_areas = true
	var result := get_world_2d().direct_space_state.intersect_ray(query)
	if not result.is_empty():
		return result["position"]
	return origin + dir * GRAPPLE_RANGE

func _apply_grapple(delta: float) -> void:
	var to := _grapple_anchor - global_position
	if to.length() < 24.0:
		_grappling = false
		_grapple_reeling = true
		_grapple_reel_timer = GRAPPLE_REEL_SECONDS
		return
	velocity += to.normalized() * GRAPPLE_PULL * delta
	_update_grapple_line()

func _apply_grapple_reel(delta: float) -> void:
	if not _grapple_line:
		return
	_grapple_reel_timer = maxf(_grapple_reel_timer - delta, 0.0)
	_grapple_anchor = _grapple_anchor.move_toward(global_position, GRAPPLE_REEL_SPEED * delta)
	_update_grapple_line()
	if _grapple_reel_timer <= 0.0 or _grapple_anchor.distance_to(global_position) < 10.0:
		_grapple_reeling = false
		_grappling = false
		_grapple_line.queue_free()
		_grapple_line = null
		_update_grapple_fx(false)

func _ensure_grapple_line() -> void:
	if _grapple_line and is_instance_valid(_grapple_line):
		_update_grapple_line()
		return
	_grapple_line = Line2D.new()
	_grapple_line.width = 5.0
	_grapple_line.default_color = Color("#00D7FF")
	_grapple_line.z_index = 200
	_grapple_line.top_level = false
	add_child(_grapple_line)
	_ensure_grapple_head()
	_update_grapple_line()

func _update_grapple_line() -> void:
	if not _grapple_line:
		return
	_grapple_line.clear_points()
	var target := _grapple_anchor - global_position
	var sag := clampf(target.length() * 0.12, 10.0, 38.0)
	_grapple_line.add_point(Vector2.ZERO)
	_grapple_line.add_point(target * 0.25 + Vector2(0.0, sag))
	_grapple_line.add_point(target * 0.50 + Vector2(0.0, sag * 0.75))
	_grapple_line.add_point(target * 0.75 + Vector2(0.0, sag * 0.35))
	_grapple_line.add_point(target)
	if _grapple_head:
		_grapple_head.position = _grapple_anchor - global_position

func _ensure_fx_nodes() -> void:
	if not _jetpack_rig:
		_jetpack_rig = Node2D.new()
		_jetpack_rig.name = "JetpackRig"
		_jetpack_rig.z_index = body_sprite.z_index - 1
		add_child(_jetpack_rig)
	if not _jetpack_pack:
		_jetpack_pack = Polygon2D.new()
		_jetpack_pack.name = "Pack"
		_jetpack_pack.polygon = PackedVector2Array([
			Vector2(-4, -6), Vector2(4, -6), Vector2(4, 6), Vector2(-4, 6)
		])
		_jetpack_pack.color = Color("#3E3E46")
		_jetpack_pack.z_index = body_sprite.z_index - 1
		_jetpack_rig.add_child(_jetpack_pack)
	if not _jetpack_flame_l:
		_jetpack_flame_l = Polygon2D.new()
		_jetpack_flame_l.name = "FlameLeft"
		_jetpack_flame_l.polygon = PackedVector2Array([
			Vector2(-3, 6), Vector2(-1, 6), Vector2(-2, 14)
		])
		_jetpack_flame_l.color = Color("#FFD21A")
		_jetpack_flame_l.z_index = body_sprite.z_index - 1
		_jetpack_rig.add_child(_jetpack_flame_l)
	if not _jetpack_flame_r:
		_jetpack_flame_r = Polygon2D.new()
		_jetpack_flame_r.name = "FlameRight"
		_jetpack_flame_r.polygon = PackedVector2Array([
			Vector2(1, 6), Vector2(3, 6), Vector2(2, 14)
		])
		_jetpack_flame_r.color = Color("#FF8A00")
		_jetpack_flame_r.z_index = body_sprite.z_index - 1
		_jetpack_rig.add_child(_jetpack_flame_r)
	if not _grapple_head:
		_grapple_head = Polygon2D.new()
		_grapple_head.name = "GrappleHead"
		_grapple_head.polygon = PackedVector2Array([
			Vector2(-3, -3), Vector2(3, -3), Vector2(3, 3), Vector2(-3, 3)
		])
		_grapple_head.color = Color("#00D7FF")
		_grapple_head.z_index = 210
		_grapple_head.visible = false
		add_child(_grapple_head)

func _update_jetpack_visuals(has_jetpack: bool, thrusting: bool) -> void:
	if not _jetpack_rig:
		return
	_jetpack_rig.visible = has_jetpack
	if not has_jetpack:
		return
	var back_offset := -7.0 if not body_sprite.flip_h else 7.0
	_jetpack_rig.position = Vector2(back_offset, -2.0)
	if _jetpack_pack:
		_jetpack_pack.visible = true
	if _jetpack_flame_l:
		_jetpack_flame_l.visible = thrusting
	if _jetpack_flame_r:
		_jetpack_flame_r.visible = thrusting
	if thrusting:
		var pulse := 1.0 + 0.15 * sin(Time.get_ticks_msec() * 0.02)
		if _jetpack_flame_l:
			_jetpack_flame_l.scale = Vector2(1.0, pulse)
		if _jetpack_flame_r:
			_jetpack_flame_r.scale = Vector2(1.0, pulse * 0.85)
	else:
		if _jetpack_flame_l:
			_jetpack_flame_l.scale = Vector2.ONE
		if _jetpack_flame_r:
			_jetpack_flame_r.scale = Vector2.ONE

func _ensure_grapple_head() -> void:
	if _grapple_head:
		_grapple_head.visible = true

func _update_grapple_fx(active: bool) -> void:
	if _grapple_head:
		_grapple_head.visible = active
	if not active and _grapple_line == null and _grapple_head:
		_grapple_head.position = Vector2.ZERO

# ── Laser pistol ─────────────────────────────────────────────────────────────

func _fire_laser(dir: float) -> void:
	facing_dir         = sign(dir)
	body_sprite.flip_h = facing_dir < 0.0

	var origin  := global_position
	var end_pos := origin + Vector2(facing_dir * LASER_RANGE, 0.0)

	var query := PhysicsRayQueryParameters2D.create(origin, end_pos)
	query.exclude = [self]
	var result := get_world_2d().direct_space_state.intersect_ray(query)
	if not result.is_empty():
		end_pos = result["position"]

	if _laser_line:
		_laser_line.queue_free()
	_laser_line = Line2D.new()
	_laser_line.add_point(origin - global_position)
	_laser_line.add_point(end_pos - global_position)
	_laser_line.default_color = Color(0.0, 0.85, 1.0, 0.9)
	_laser_line.width         = 4.0
	add_child(_laser_line)
	_laser_timer = LASER_DURATION

# ── Resource helpers ──────────────────────────────────────────────────────────

func _drain_fuel(amount: float) -> void:
	var rm := get_node_or_null("/root/RunManager")
	if rm and rm.has_method("update_fuel"):
		rm.update_fuel(clampf(rm.get_fuel_pct() - amount, 0.0, 100.0))

func _drain_battery(amount: float) -> void:
	var rm := get_node_or_null("/root/RunManager")
	if rm and rm.has_method("update_battery"):
		rm.update_battery(clampf(rm.get_battery_pct() - amount, 0.0, 100.0))

func _get_fuel() -> float:
	var rm := get_node_or_null("/root/RunManager")
	return float(rm.get_fuel_pct()) if rm and rm.has_method("get_fuel_pct") else 100.0

func _get_battery() -> float:
	var rm := get_node_or_null("/root/RunManager")
	return float(rm.get_battery_pct()) if rm and rm.has_method("get_battery_pct") else 100.0

func _has_equipment(item: String) -> bool:
	var rm := get_node_or_null("/root/RunManager")
	return rm.has_equipment(item) if rm and rm.has_method("has_equipment") else false

# Fixed slot → equipment type mapping (Insert, Home, PgUp, Del, End, PgDn),
# matching the always-in-this-order equipment cards shown in the HUD. Unlike
# a pickup-order hotbar, these keys always mean the same gear regardless of
# the order it was collected in — slot 1 is always the jetpack, slot 2 the
# laser, etc.
const EQUIPMENT_SLOT_ORDER := [
	"jetpack", "laser_pistol", "grappling_hook",
	"magnetic_boots", "visibility_cloak", "shield",
]

# Tap a slot key (1-6) to activate that item; hold it to drop the item at the
# player's feet as a pickup other rooms' logic can re-collect.
func _update_equipment_hotkeys(delta: float) -> void:
	for i in range(1, EQUIPMENT_SLOT_COUNT + 1):
		var action := "equipment_%d" % i
		if not InputMap.has_action(action):
			continue
		var idx := i - 1
		if Input.is_action_just_pressed(action):
			_eq_hold_time[idx] = 0.0
			_eq_hold_consumed[idx] = false
		elif Input.is_action_pressed(action):
			_eq_hold_time[idx] += delta
			if not _eq_hold_consumed[idx] and _eq_hold_time[idx] >= EQUIPMENT_DROP_HOLD_SECONDS:
				_eq_hold_consumed[idx] = true
				_drop_equipment_slot(i)
		elif Input.is_action_just_released(action):
			if not _eq_hold_consumed[idx]:
				_activate_equipment_slot(i)
			_eq_hold_time[idx] = 0.0
			_eq_hold_consumed[idx] = false

func _drop_equipment_slot(slot: int) -> void:
	var rm := get_node_or_null("/root/RunManager")
	if rm == null or not rm.has_method("remove_equipment"):
		return
	var idx := slot - 1
	if idx < 0 or idx >= EQUIPMENT_SLOT_ORDER.size():
		return
	var item_id: String = EQUIPMENT_SLOT_ORDER[idx]
	if not _has_equipment(item_id):
		return
	rm.remove_equipment(item_id)
	# Dropping active gear also shuts it off
	match item_id:
		"magnetic_boots":
			_mag_active = false
			_mag_attached = false
			body_sprite.rotation = 0.0
		"jetpack":
			_jetpack_active = false
		"visibility_cloak":
			_cloak_active = false
			_update_cloak_visuals()
	var pickup := EQUIPMENT_PICKUP_SCENE.instantiate()
	if pickup.has_method("configure"):
		pickup.configure({"item_id": item_id, "display_name": item_id.replace("_", " ")})
	pickup.global_position = global_position
	# Grace period so the drop doesn't instantly re-collect under the player's feet
	pickup.set_deferred("monitoring", false)
	get_parent().add_child(pickup)
	var re_arm := get_tree().create_timer(DROPPED_PICKUP_GRACE_SECONDS)
	re_arm.timeout.connect(func() -> void:
		if is_instance_valid(pickup):
			pickup.set_deferred("monitoring", true))
	if pickup.has_signal("picked_up"):
		pickup.picked_up.connect(func(picked_id: String) -> void:
			var run_mgr := get_node_or_null("/root/RunManager")
			if run_mgr and run_mgr.has_method("grant_equipment"):
				run_mgr.grant_equipment(picked_id))

# Activate equipment by slot number (1-based). Maps slot to a fixed equipment type.
func _activate_equipment_slot(slot: int) -> void:
	var idx := slot - 1
	if idx < 0 or idx >= EQUIPMENT_SLOT_ORDER.size():
		return
	var item_id: String = EQUIPMENT_SLOT_ORDER[idx]
	if not _has_equipment(item_id):
		return
	match item_id:
		"magnetic_boots":
			_mag_active = not _mag_active
			if not _mag_active:
				_mag_attached = false
				body_sprite.rotation = 0.0
		"grappling_hook":
			_begin_grapple_aim()
		"laser_pistol":
			_fire_laser(facing_dir)
		"visibility_cloak":
			# Passive ability; selection just refreshes the visual state.
			_cloak_active = _get_battery() > 0.0
			_update_cloak_visuals()
		_:
			pass  # future equipment types handled here

# ── Animation ─────────────────────────────────────────────────────────────────

func _update_animation(h: float, grounded_like: bool) -> void:
	if not grounded_like:
		_play_animation("jump")
	elif h != 0.0:
		_play_animation("walk")
	else:
		_play_animation("idle")

func _play_animation(anim: StringName) -> void:
	if body_sprite.animation != anim:
		body_sprite.play(anim)

func _update_cloak_visuals() -> void:
	if not body_sprite:
		return
	var active := _cloak_active and _has_equipment("visibility_cloak") and _get_battery() > 0.0
	body_sprite.modulate = Color(1.0, 1.0, 1.0, 0.38 if active else 1.0)

# ── Audio ─────────────────────────────────────────────────────────────────────

func _play_jump_sfx() -> void:
	var am := get_node_or_null("/root/Audio_Manager")
	if am and am.has_method("play_sfx"): am.play_sfx("jump")

func _play_fall_sfx() -> void:
	var am := get_node_or_null("/root/Audio_Manager")
	if am and am.has_method("play_sfx"): am.play_sfx("fall", 0.8)

func _play_jetpack_sfx() -> void:
	var am := get_node_or_null("/root/Audio_Manager")
	if am and am.has_method("play_sfx"):
		am.play_sfx("jump_short", 0.75)

func _play_grapple_sfx() -> void:
	var am := get_node_or_null("/root/Audio_Manager")
	if am and am.has_method("play_sfx"):
		am.play_sfx("collect", 0.85)
