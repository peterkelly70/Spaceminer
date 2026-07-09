extends TextureRect
class_name NeonOreIcon

const NEON_COLORS: Array[Color] = [
	Color("#00D7FF"),
	Color("#FFD21A"),
	Color("#8CFF00"),
	Color("#FF00C8"),
	Color("#F4F4F4"),
]

@export var cycle_speed: float = 9.0

var _collected := false
var _phase := 0.0

func _ready() -> void:
	custom_minimum_size = Vector2(18, 18)
	expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	set_collected(_collected)

func _process(delta: float) -> void:
	if not _collected:
		return
	_phase += delta * cycle_speed
	modulate = NEON_COLORS[int(_phase) % NEON_COLORS.size()]

func set_collected(value: bool) -> void:
	_collected = value
	_phase = 0.0
	if _collected:
		modulate = NEON_COLORS[0]
	else:
		modulate = Color(0.15, 0.15, 0.18, 1.0)
