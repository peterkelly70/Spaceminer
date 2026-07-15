extends Resource
class_name LandingBayAlphaModel

signal ore_changed(collected: int, total: int)
signal oxygen_changed(current: float, max_value: float)
signal tip_changed(text: String)
signal oxygen_depleted

@export var room_name: String = "Landing Bay Alpha"
@export var max_oxygen: float = 100.0
@export var oxygen_drain_per_second: float = 0.35

var current_oxygen: float = 100.0
var ore_collected: int = 0
var ore_total: int = 0

func reset() -> void:
	current_oxygen = max_oxygen
	ore_collected = 0
	emit_signal("oxygen_changed", current_oxygen, max_oxygen)
	emit_signal("ore_changed", ore_collected, ore_total)

func register_ore(amount: int) -> void:
	ore_total += amount
	emit_signal("ore_changed", ore_collected, ore_total)

func collect_ore(amount: int) -> void:
	ore_collected += amount
	emit_signal("ore_changed", ore_collected, ore_total)

func drain_oxygen(delta: float) -> void:
	if current_oxygen <= 0.0:
		return
	current_oxygen = maxf(current_oxygen - (oxygen_drain_per_second * delta), 0.0)
	emit_signal("oxygen_changed", current_oxygen, max_oxygen)
	if is_zero_approx(current_oxygen):
		emit_signal("oxygen_depleted")

func refill_oxygen() -> void:
	current_oxygen = max_oxygen
	emit_signal("oxygen_changed", current_oxygen, max_oxygen)

func set_oxygen_pct(pct: float) -> void:
	current_oxygen = clampf(max_oxygen * pct / 100.0, 0.0, max_oxygen)
	emit_signal("oxygen_changed", current_oxygen, max_oxygen)

func get_oxygen_pct() -> float:
	if max_oxygen <= 0.0:
		return 0.0
	return (current_oxygen / max_oxygen) * 100.0

func set_tip(text: String) -> void:
	emit_signal("tip_changed", text)
