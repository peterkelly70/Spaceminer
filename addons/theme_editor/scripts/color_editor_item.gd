@tool
extends PanelContainer

signal color_changed(color)

@onready var type_label: Label = $VBoxContainer/HeaderContainer/TypeLabel
@onready var name_label: Label = $VBoxContainer/HeaderContainer/NameLabel
@onready var color_picker: ColorPickerButton = $VBoxContainer/ColorContainer/ColorPicker
@onready var hex_edit: LineEdit = $VBoxContainer/ColorContainer/HexEdit

var _type: String
var _name: String
var _color: Color

func _ready() -> void:
	# Connect signals
	color_picker.color_changed.connect(_on_color_picker_changed)
	hex_edit.text_submitted.connect(_on_hex_edit_submitted)

func setup(type: String, name: String, color: Color) -> void:
	_type = type
	_name = name
	_color = color
	
	# Update UI
	type_label.text = type
	name_label.text = name
	color_picker.color = color
	hex_edit.text = color.to_html(true)

func _on_color_picker_changed(color: Color) -> void:
	_color = color
	hex_edit.text = color.to_html(true)
	color_changed.emit(_color)

func _on_hex_edit_submitted(hex_value: String) -> void:
	# Validate hex format
	if hex_value.begins_with("#"):
		hex_value = hex_value.substr(1)
	
	# Ensure it's a valid hex color
	if hex_value.length() == 6 or hex_value.length() == 8:
		var color = Color.from_string(hex_value, Color.BLACK)
		_color = color
		color_picker.color = color
		color_changed.emit(_color)
	else:
		# Reset to current color if invalid
		hex_edit.text = _color.to_html(true)
