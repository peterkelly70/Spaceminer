extends Control
class_name Starfield

@export var star_count: int = 250
@export var speed_min: float = 30.0
@export var speed_max: float = 160.0
@export var star_size_min: float = 1.0
@export var star_size_max: float = 2.8
@export var twinkle_speed: float = 2.0
@export var direction: Vector2 = Vector2.DOWN
@export var color_near: Color = Color(0.95, 0.98, 1.0, 0.95)
@export var color_far: Color = Color(0.55, 0.65, 0.9, 0.7)
@export var extra_colors: Array[Color] = [
	Color(1.0, 0.85, 0.4, 1.0),   # warm yellow
	Color(0.8, 0.6, 1.0, 1.0),    # violet
	Color(0.6, 0.9, 1.0, 1.0),    # cyan
	Color(1.0, 0.6, 0.5, 1.0),    # orange
	Color(0.7, 1.0, 0.7, 1.0)     # mint
]
@export_range(0.0, 1.0, 0.01) var extra_color_ratio: float = 0.35

# Pixel mode for crisp arcade dots
@export var pixel_mode: bool = true
@export var pixel_size: int = 1

# Each star is a Dictionary with keys: "pos" Vector2, "vel" float, "size" float, "phase" float, "near" bool
var _stars: Array[Dictionary] = []
var _time: float = 0.0

func _ready() -> void:
	mouse_filter = MOUSE_FILTER_IGNORE
	direction = direction.normalized()
	# Ensure we occupy the full viewport even if parent is Node2D
	size = get_viewport_rect().size
	get_viewport().size_changed.connect(_on_screen_resized)
	rebuild()
	set_process(true)

func rebuild() -> void:
	_stars.clear()
	var w: float = max(size.x, 1.0)
	var h: float = max(size.y, 1.0)
	for i in star_count:
		var is_near := randf() > 0.6
		var vel_mul := 1.0
		if not is_near:
			vel_mul = 0.5
		var size_mul := 1.0
		if not is_near:
			size_mul = 0.7
		var base_col: Color = color_far
		if is_near:
			base_col = color_near
		var col := base_col
		if randf() < extra_color_ratio and extra_colors.size() > 0:
			var idx := randi() % extra_colors.size()
			var accent: Color = extra_colors[idx]
			# Blend accent color with base to keep subtlety
			col = base_col.lerp(accent, 0.7)
		_stars.append({
			"pos": Vector2(randf() * w, randf() * h),
			"vel": randf_range(speed_min, speed_max) * vel_mul,
			"size": randf_range(star_size_min, star_size_max) * size_mul,
			"phase": randf() * TAU,
			"near": is_near,
			"col": col,
		})
	queue_redraw()

func _process(delta: float) -> void:
	_time += delta
	var w: float = max(size.x, 1.0)
	var h: float = max(size.y, 1.0)
	for s in _stars:
		s["pos"] += direction * float(s["vel"]) * delta
		var p: Vector2 = s["pos"]
		if direction.y > 0.5 and p.y > h:
			p.y = -1.0
			p.x = randf() * w
		elif direction.y < -0.5 and p.y < -1.0:
			p.y = h + 1.0
			p.x = randf() * w
		elif direction.x > 0.5 and p.x > w:
			p.x = -1.0
			p.y = randf() * h
		elif direction.x < -0.5 and p.x < -1.0:
			p.x = w + 1.0
			p.y = randf() * h
		s["pos"] = p
	queue_redraw()

func _draw() -> void:
	# Solid black background to ensure pure space behind stars
	draw_rect(Rect2(Vector2.ZERO, size), Color.BLACK, true)
	for s in _stars:
		var t := 0.5 + 0.5 * sin(_time * twinkle_speed + float(s["phase"]))
		var col: Color = s["col"]
		col.a *= clampf(t, 0.2, 1.0)
		if pixel_mode:
			var p: Vector2 = s["pos"]
			p.x = floor(p.x)
			p.y = floor(p.y)
			draw_rect(Rect2(p, Vector2(float(max(pixel_size, 1)), float(max(pixel_size, 1)))), col, true)
		else:
			draw_circle(s["pos"], float(s["size"]), col)

func _notification(what: int) -> void:
	if what == NOTIFICATION_RESIZED:
		rebuild()

func _on_screen_resized() -> void:
	# Sync to new viewport size and rebuild star positions
	size = get_viewport_rect().size
	rebuild()
