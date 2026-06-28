extends Area2D
class_name ResupplyStation

signal activated(station_type: String)

# type: "air" | "fuel" | "battery" | "full"
@export var station_type: String = "full"
@export var label_text: String = "RESUPPLY"

@onready var sprite: Sprite2D = $Sprite2D if has_node("Sprite2D") else null
@onready var label: Label = $Label if has_node("Label") else null
@onready var hold_bar: ProgressBar = $HoldBar if has_node("HoldBar") else null

var _player_inside := false
var _used_this_visit := false
var _hold_progress := 0.0
var _hold_seconds := 0.9
var _ore_cost := 1

func _ready() -> void:
	body_entered.connect(_on_body_entered)
	body_exited.connect(_on_body_exited)
	_refresh_visual()
	if label:
		label.text = label_text
	if hold_bar:
		hold_bar.visible = false
		hold_bar.value = 0.0

func _input(event: InputEvent) -> void:
	if not _player_inside:
		return
	if _is_hold_action(event):
		_update_hold(0.0, true)

func _process(delta: float) -> void:
	if not _player_inside:
		_update_hold(0.0, false)
		return
	var pressed := _is_hold_action()
	if pressed:
		_update_hold(delta, true)
		return
	_update_hold(delta, false)

func _is_hold_action(event: InputEvent = null) -> bool:
	if event:
		if InputMap.has_action("interact") and event.is_action_pressed("interact"):
			return true
		if event.is_action_pressed("ui_accept"):
			return true
		if event.is_action_pressed("ui_down"):
			return true
		return false
	if InputMap.has_action("interact") and Input.is_action_pressed("interact"):
		return true
	return Input.is_action_pressed("ui_accept") or Input.is_action_pressed("ui_down")

func _on_body_entered(body: Node) -> void:
	if body.is_in_group("player") or body.is_in_group("prototype_player"):
		_player_inside = true
		_hold_progress = 0.0
		_update_hold_bar()

func _on_body_exited(body: Node) -> void:
	if body.is_in_group("player") or body.is_in_group("prototype_player"):
		_player_inside = false
		_hold_progress = 0.0
		_update_hold_bar()

func _activate() -> void:
	var rm := get_node_or_null("/root/RunManager")
	# Don't waste ore if the supply this station refills is already full
	if _is_supply_full(rm):
		_play_error_sfx()
		_hold_progress = 0.0
		_update_hold_bar()
		return
	if rm and rm.has_method("get_ore_count") and rm.get_ore_count() < _ore_cost:
		_play_error_sfx()
		return
	if rm and rm.has_method("update_ore"):
		rm.update_ore(maxi(int(rm.get_ore_count()) - _ore_cost, 0))
	activated.emit(station_type)
	_play_collect_sfx()
	_hold_progress = 0.0
	_update_hold_bar()

# True if the resource this station refills is already at capacity (no point buying).
func _is_supply_full(rm: Node) -> bool:
	if not rm:
		return false
	match station_type.to_lower():
		"air":
			if rm.has_method("get_oxygen_pct"):
				return float(rm.get_oxygen_pct()) >= 99.5
		"fuel":
			if rm.has_method("get_fuel_pct"):
				var cap := 100.0
				if rm.has_method("get_fuel_capacity_pct"):
					cap = float(rm.get_fuel_capacity_pct())
				return float(rm.get_fuel_pct()) >= cap - 0.5
		"battery":
			if rm.has_method("get_battery_pct"):
				return float(rm.get_battery_pct()) >= 99.5
		_:
			var air_full := not rm.has_method("get_oxygen_pct") or float(rm.get_oxygen_pct()) >= 99.5
			var fuel_full := not rm.has_method("get_fuel_pct") or float(rm.get_fuel_pct()) >= 99.5
			var batt_full := not rm.has_method("get_battery_pct") or float(rm.get_battery_pct()) >= 99.5
			return air_full and fuel_full and batt_full
	return false

func _refresh_visual() -> void:
	if sprite:
		sprite.texture = _icon_for_station(station_type)
		sprite.modulate = _color_for_station(station_type)

func _icon_for_station(type_name: String) -> Texture2D:
	var path := ""
	match type_name.to_lower():
		"air":
			path = "res://assets/tiles/Transparent/tile_0405.png"
		"fuel":
			path = "res://assets/tiles/Transparent/tile_0410.png"
		"battery":
			path = "res://assets/tiles/Transparent/tile_0409.png"
		_:
			path = "res://assets/tiles/Transparent/tile_0405.png"
	var texture := load(path)
	if texture is Texture2D:
		return texture
	var image := Image.new()
	if image.load(path) == OK:
		return ImageTexture.create_from_image(image)
	return null

func _update_hold(delta: float, is_holding: bool) -> void:
	if not hold_bar:
		if is_holding and _player_inside:
			_activate()
		return
	hold_bar.visible = _player_inside
	if not _player_inside:
		return
	if is_holding:
		_hold_progress = minf(_hold_progress + delta, _hold_seconds)
		if is_equal_approx(_hold_progress, _hold_seconds) or _hold_progress >= _hold_seconds:
			_activate()
			return
	else:
		_hold_progress = maxf(_hold_progress - (delta * 2.5), 0.0)
	_update_hold_bar()

func _update_hold_bar() -> void:
	if not hold_bar:
		return
	hold_bar.visible = _player_inside
	hold_bar.value = (_hold_progress / _hold_seconds) * 100.0 if _hold_seconds > 0.0 else 0.0

func _play_collect_sfx() -> void:
	var audio_manager := get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("collect")

func _play_error_sfx() -> void:
	var audio_manager := get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("error")

func _color_for_station(type_name: String) -> Color:
	match type_name.to_lower():
		"air":
			return Color("#00D7FF")
		"fuel":
			return Color("#FF8A00")
		"battery":
			return Color("#5A2DFF")
		_:
			return Color.WHITE
