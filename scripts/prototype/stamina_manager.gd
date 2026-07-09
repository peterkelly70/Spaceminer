extends Resource
class_name StaminaManager

signal stamina_changed(current: float, max_value: float)

var _max_stamina: float = 100.0

@export var max_stamina: float = 100.0 :
	get:
		return _max_stamina
	set(value):
		_max_stamina = max(value, 1.0)
		current_stamina = clamp(current_stamina, 0.0, _max_stamina)
		stamina_changed.emit(current_stamina, _max_stamina)

@export var regen_rate: float = 18.0
@export var regen_delay: float = 0.35

var current_stamina: float = _max_stamina
var _cooldown_timer: float = 0.0

func _init() -> void:
	current_stamina = max_stamina

func reset() -> void:
	current_stamina = _max_stamina
	_cooldown_timer = 0.0
	stamina_changed.emit(current_stamina, _max_stamina)

func tick(delta: float) -> void:
	if _cooldown_timer > 0.0:
		_cooldown_timer = max(_cooldown_timer - delta, 0.0)
		return
	_update_value(current_stamina + regen_rate * delta)

func try_consume(amount: float) -> bool:
	if current_stamina < amount:
		return false
	consume(amount)
	return true

func consume(amount: float) -> void:
	_cooldown_timer = regen_delay
	_update_value(current_stamina - amount)

func _update_value(value: float) -> void:
	var new_value = clamp(value, 0.0, _max_stamina)
	if !is_equal_approx(new_value, current_stamina):
		current_stamina = new_value
		stamina_changed.emit(current_stamina, _max_stamina)
	else:
		current_stamina = new_value
