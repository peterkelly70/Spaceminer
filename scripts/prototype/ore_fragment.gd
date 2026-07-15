extends Area2D
class_name OreFragment

signal collected(amount: int)

@export var amount: int = 1

const NEON_COLORS: Array[Color] = [
	Color("#00D7FF"),
	Color("#FFD21A"),
	Color("#8CFF00"),
	Color("#FF00C8"),
	Color("#F4F4F4"),
]

var _phase := 0.0
@onready var gem: Sprite2D = $Gem

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _process(delta: float) -> void:
	_phase += delta * 10.0
	gem.modulate = NEON_COLORS[int(_phase) % NEON_COLORS.size()]

func _on_body_entered(body: Node) -> void:
	if not body.is_in_group("prototype_player") and not body.is_in_group("player"):
		return
	collected.emit(amount)
	queue_free()
