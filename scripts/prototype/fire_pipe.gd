extends Area2D
class_name FirePipe

# A pipe that periodically jets flame. Only hurts while the flame is active.

signal player_hit

@export var period: float = 1.8     # full on+off cycle
@export var on_time: float = 0.7    # how long the flame is active each cycle
@export var phase: float = 0.0      # stagger so neighbouring pipes differ

var _t: float = 0.0
var _active: bool = false

@onready var flame: Sprite2D = $Flame

func _ready() -> void:
	add_to_group("hazard")
	body_entered.connect(_on_body_entered)
	_t = phase
	_refresh()

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

func _on_body_entered(b: Node) -> void:
	if not _active:
		return
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		player_hit.emit()
