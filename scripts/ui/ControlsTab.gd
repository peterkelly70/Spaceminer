@tool
extends ScrollContainer
class_name ControlsTab

var _listening_for_action := ""
var _listening_slot := 0
var _listen_button: Button = null
var _rows: Dictionary = {}  # action -> { key0: Label, key1: Label, btn0: Button, btn1: Button }

@onready var _right_btn: Button = $ControlsVBox/PresetRow/RightHandedBtn
@onready var _left_btn: Button = $ControlsVBox/PresetRow/LeftHandedBtn
@onready var _action_rows: VBoxContainer = $ControlsVBox/ActionRows

func _ready() -> void:
	if Engine.is_editor_hint():
		return

	var retro_theme: Theme = load("res://assets/themes/retro_theme.tres")
	if retro_theme:
		theme = retro_theme

	_right_btn.pressed.connect(func(): ControlsManager.apply_preset("right_handed"))
	_left_btn.pressed.connect(func(): ControlsManager.apply_preset("left_handed"))

	_build_action_rows()

	if ControlsManager.has_signal("bindings_changed"):
		ControlsManager.bindings_changed.connect(_refresh_labels)

func _build_action_rows() -> void:
	for child in _action_rows.get_children():
		child.queue_free()

	for action in ControlsManager.GAME_ACTIONS:
		var row := HBoxContainer.new()
		row.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		_action_rows.add_child(row)

		var name_lbl := Label.new()
		name_lbl.text = ControlsManager.get_action_display_name(action)
		name_lbl.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		name_lbl.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		name_lbl.add_theme_font_size_override("font_size", 24)
		row.add_child(name_lbl)

		var key0_lbl := Label.new()
		key0_lbl.text = ControlsManager.get_key_label(action, 0)
		key0_lbl.custom_minimum_size = Vector2(160, 0)
		key0_lbl.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		key0_lbl.add_theme_font_size_override("font_size", 24)
		row.add_child(key0_lbl)

		var btn0 := Button.new()
		btn0.text = "Remap"
		btn0.custom_minimum_size = Vector2(110, 36)
		btn0.add_theme_font_size_override("font_size", 20)
		btn0.pressed.connect(_on_remap_pressed.bind(action, 0, key0_lbl, btn0))
		row.add_child(btn0)

		var key1_lbl := Label.new()
		key1_lbl.text = ControlsManager.get_key_label(action, 1)
		key1_lbl.custom_minimum_size = Vector2(160, 0)
		key1_lbl.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		key1_lbl.add_theme_font_size_override("font_size", 24)
		row.add_child(key1_lbl)

		var btn1 := Button.new()
		btn1.text = "Remap"
		btn1.custom_minimum_size = Vector2(110, 36)
		btn1.add_theme_font_size_override("font_size", 20)
		btn1.pressed.connect(_on_remap_pressed.bind(action, 1, key1_lbl, btn1))
		row.add_child(btn1)

		_rows[action] = {"key0": key0_lbl, "key1": key1_lbl, "btn0": btn0, "btn1": btn1}

func _input(event: InputEvent) -> void:
	if _listening_for_action.is_empty():
		return
	if not event.is_pressed() or event.is_echo():
		return
	if event is InputEventKey and event.keycode == KEY_ESCAPE:
		_cancel_listen()
		return
	if event is InputEventKey or event is InputEventJoypadButton:
		ControlsManager.remap_action_slot(_listening_for_action, _listening_slot, event)
		_cancel_listen()
		get_viewport().set_input_as_handled()

func _on_remap_pressed(action: String, slot: int, key_lbl: Label, btn: Button) -> void:
	if not _listening_for_action.is_empty():
		_cancel_listen()
	_listening_for_action = action
	_listening_slot = slot
	_listen_button = btn
	btn.text = "Press key…"
	key_lbl.text = "…"

func _cancel_listen() -> void:
	if _listen_button:
		_listen_button.text = "Remap"
	if _rows.has(_listening_for_action):
		var slot_key := "key%d" % _listening_slot
		_rows[_listening_for_action][slot_key].text = ControlsManager.get_key_label(_listening_for_action, _listening_slot)
	_listening_for_action = ""
	_listening_slot = 0
	_listen_button = null

func _refresh_labels() -> void:
	for action in _rows:
		_rows[action]["key0"].text = ControlsManager.get_key_label(action, 0)
		_rows[action]["key1"].text = ControlsManager.get_key_label(action, 1)
		for k in ["btn0", "btn1"]:
			if _rows[action][k].text != "Press key…":
				_rows[action][k].text = "Remap"
