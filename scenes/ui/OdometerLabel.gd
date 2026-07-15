extends Control

signal value_changed(new_value)

@export var digit_count: int = 3
@export var value: int = 0:
	set(new_value):
		if value != new_value:
			value = new_value
			_update_display()
			value_changed.emit(new_value)

@export var font: Font
@export var font_size: int = 16
@export var text_color: Color = Color.WHITE

var _digits = []

func _ready() -> void:
	# Create the digit labels
	for i in range(digit_count):
		var label = Label.new()
		label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		label.custom_minimum_size = Vector2(size.x / digit_count, size.y)
		label.mouse_filter = Control.MOUSE_FILTER_IGNORE
		
		# Apply font settings
		if font:
			label.add_theme_font_override("font", font)
		label.add_theme_font_size_override("font_size", font_size)
		label.add_theme_color_override("font_color", text_color)
		
		$HBoxContainer.add_child(label)
		_digits.append(label)
	
	# Initial update
	_update_display()

func _update_display() -> void:
	var val_str = str(value).pad_zeros(digit_count)
	
	# If the value is too large for the digit count, just show the maximum
	if val_str.length() > digit_count:
		val_str = "9".repeat(digit_count)
	
	# Update each digit label
	for i in range(digit_count):
		if i < val_str.length():
			_digits[i].text = val_str[val_str.length() - 1 - i]
		else:
			_digits[i].text = "0"
