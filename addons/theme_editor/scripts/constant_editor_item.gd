@tool
extends PanelContainer

signal constant_changed(value)

@onready var type_label: Label = $VBoxContainer/HeaderContainer/TypeLabel
@onready var name_label: Label = $VBoxContainer/HeaderContainer/NameLabel
@onready var value_spin: SpinBox = $VBoxContainer/ValueContainer/ValueSpin

var _type: String
var _name: String
var _value: int

func _ready() -> void:
	# Connect signals
	value_spin.value_changed.connect(_on_value_changed)

func setup(type: String, name: String, value: int) -> void:
	_type = type
	_name = name
	_value = value
	
	# Update UI
	type_label.text = type
	name_label.text = name
	value_spin.value = value

func _on_value_changed(value: float) -> void:
	_value = int(value)
	constant_changed.emit(_value)
