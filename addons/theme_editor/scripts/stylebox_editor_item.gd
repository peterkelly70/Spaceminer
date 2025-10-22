@tool
extends PanelContainer

signal stylebox_changed(stylebox)

@onready var type_label: Label = $VBoxContainer/HeaderContainer/TypeLabel
@onready var name_label: Label = $VBoxContainer/HeaderContainer/NameLabel
@onready var stylebox_type_option: OptionButton = $VBoxContainer/TypeContainer/StyleboxTypeOption
@onready var properties_container: VBoxContainer = $VBoxContainer/PropertiesContainer
@onready var preview_panel: Panel = $VBoxContainer/PreviewContainer/PreviewPanel

# Highlight effect
var _highlight_timer: Timer
var _original_color: Color
var _highlight_color: Color = Color(1, 0.5, 0, 0.3)

var _type: String
var _name: String
var _stylebox: StyleBox
var _file_dialog: FileDialog
var _property_editors: Dictionary = {}

func _ready() -> void:
	# Connect signals
	stylebox_type_option.item_selected.connect(_on_stylebox_type_selected)
	
	# Create file dialog for texture selection
	_file_dialog = FileDialog.new()
	add_child(_file_dialog)
	
	# Configure file dialog
	_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	_file_dialog.add_filter("*.png,*.jpg,*.jpeg,*.svg")
	
	# Connect file dialog signals
	_file_dialog.file_selected.connect(_on_texture_file_selected)
	
	# Setup stylebox type options
	_setup_stylebox_types()

func setup(type: String, name: String, stylebox: StyleBox) -> void:
	_type = type
	_name = name
	_stylebox = stylebox
	
	# Update UI
	type_label.text = type
	name_label.text = name
	
	# Select appropriate stylebox type in dropdown
	_select_current_stylebox_type()
	
	# Create property editors for this stylebox
	_create_property_editors()
	
	# Update preview
	_update_preview()
	
	# Setup highlight timer
	_setup_highlight_timer()
	
	# Store original color
	_original_color = self.get_theme_stylebox("panel", "PanelContainer").bg_color

func _setup_stylebox_types() -> void:
	stylebox_type_option.clear()
	stylebox_type_option.add_item("StyleBoxFlat", 0)
	stylebox_type_option.add_item("StyleBoxTexture", 1)
	stylebox_type_option.add_item("StyleBoxLine", 2)
	stylebox_type_option.add_item("StyleBoxEmpty", 3)

func _select_current_stylebox_type() -> void:
	if _stylebox is StyleBoxFlat:
		stylebox_type_option.select(0)
	elif _stylebox is StyleBoxTexture:
		stylebox_type_option.select(1)
	elif _stylebox is StyleBoxLine:
		stylebox_type_option.select(2)
	elif _stylebox is StyleBoxEmpty:
		stylebox_type_option.select(3)

func _create_property_editors() -> void:
	# Clear existing property editors
	for child in properties_container.get_children():
		child.queue_free()
	
	_property_editors.clear()
	
	# Create property editors based on stylebox type
	if _stylebox is StyleBoxFlat:
		_create_flat_stylebox_editors()
	elif _stylebox is StyleBoxTexture:
		_create_texture_stylebox_editors()
	elif _stylebox is StyleBoxLine:
		_create_line_stylebox_editors()
	# Empty stylebox has no properties to edit

func _create_flat_stylebox_editors() -> void:
	var flat_stylebox = _stylebox as StyleBoxFlat
	
	# Background color
	_add_color_property("bg_color", "Background Color", flat_stylebox.bg_color)
	
	# Border width
	_add_section_label("Border Width")
	_add_int_property("border_width_left", "Left", flat_stylebox.border_width_left)
	_add_int_property("border_width_top", "Top", flat_stylebox.border_width_top)
	_add_int_property("border_width_right", "Right", flat_stylebox.border_width_right)
	_add_int_property("border_width_bottom", "Bottom", flat_stylebox.border_width_bottom)
	
	# Border color
	_add_color_property("border_color", "Border Color", flat_stylebox.border_color)
	
	# Corner radius
	_add_section_label("Corner Radius")
	_add_int_property("corner_radius_top_left", "Top Left", flat_stylebox.corner_radius_top_left)
	_add_int_property("corner_radius_top_right", "Top Right", flat_stylebox.corner_radius_top_right)
	_add_int_property("corner_radius_bottom_right", "Bottom Right", flat_stylebox.corner_radius_bottom_right)
	_add_int_property("corner_radius_bottom_left", "Bottom Left", flat_stylebox.corner_radius_bottom_left)
	
	# Content margins
	_add_section_label("Content Margins")
	_add_int_property("content_margin_left", "Left", flat_stylebox.content_margin_left)
	_add_int_property("content_margin_top", "Top", flat_stylebox.content_margin_top)
	_add_int_property("content_margin_right", "Right", flat_stylebox.content_margin_right)
	_add_int_property("content_margin_bottom", "Bottom", flat_stylebox.content_margin_bottom)
	
	# Shadow
	_add_section_label("Shadow")
	_add_color_property("shadow_color", "Shadow Color", flat_stylebox.shadow_color)
	_add_int_property("shadow_size", "Shadow Size", flat_stylebox.shadow_size)
	_add_vector2_property("shadow_offset", "Shadow Offset", flat_stylebox.shadow_offset)

func _create_texture_stylebox_editors() -> void:
	var texture_stylebox = _stylebox as StyleBoxTexture
	
	# Texture
	_add_texture_property("texture", "Texture", texture_stylebox.texture)
	
	# Content margin
	_add_section_label("Content Margin")
	_add_float_property("content_margin_left", "Left", texture_stylebox.content_margin_left)
	_add_float_property("content_margin_top", "Top", texture_stylebox.content_margin_top)
	_add_float_property("content_margin_right", "Right", texture_stylebox.content_margin_right)
	_add_float_property("content_margin_bottom", "Bottom", texture_stylebox.content_margin_bottom)
	
	# Expand margin
	_add_section_label("Expand Margin")
	_add_float_property("expand_margin_left", "Left", texture_stylebox.expand_margin_left)
	_add_float_property("expand_margin_top", "Top", texture_stylebox.expand_margin_top)
	_add_float_property("expand_margin_right", "Right", texture_stylebox.expand_margin_right)
	_add_float_property("expand_margin_bottom", "Bottom", texture_stylebox.expand_margin_bottom)
	
	# Region rect
	_add_section_label("Region")
	_add_bool_property("region_enabled", "Region Enabled", texture_stylebox.region_enabled)
	_add_rect2_property("region_rect", "Region Rect", texture_stylebox.region_rect)

func _create_line_stylebox_editors() -> void:
	var line_stylebox = _stylebox as StyleBoxLine
	
	# Color
	_add_color_property("color", "Color", line_stylebox.color)
	
	# Thickness
	_add_int_property("thickness", "Thickness", line_stylebox.thickness)
	
	# Vertical
	_add_bool_property("vertical", "Vertical", line_stylebox.vertical)
	
	# Grow
	_add_float_property("grow_begin", "Grow Begin", line_stylebox.grow_begin)
	_add_float_property("grow_end", "Grow End", line_stylebox.grow_end)

func _add_section_label(text: String) -> void:
	var label = Label.new()
	label.text = text
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	properties_container.add_child(label)
	
	var separator = HSeparator.new()
	properties_container.add_child(separator)

func _add_color_property(property: String, label_text: String, initial_value: Color) -> void:
	var container = HBoxContainer.new()
	container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	var label = Label.new()
	label.text = label_text
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(label)
	
	var color_picker = ColorPickerButton.new()
	color_picker.color = initial_value
	color_picker.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	color_picker.custom_minimum_size = Vector2(60, 0)
	container.add_child(color_picker)
	
	properties_container.add_child(container)
	
	# Connect signal
	color_picker.color_changed.connect(_on_property_changed.bind(property, "color"))
	
	# Store reference
	_property_editors[property] = color_picker

func _add_int_property(property: String, label_text: String, initial_value: int) -> void:
	var container = HBoxContainer.new()
	container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	var label = Label.new()
	label.text = label_text
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(label)
	
	var spin_box = SpinBox.new()
	spin_box.min_value = 0
	spin_box.max_value = 100
	spin_box.value = initial_value
	spin_box.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(spin_box)
	
	properties_container.add_child(container)
	
	# Connect signal
	spin_box.value_changed.connect(_on_property_changed.bind(property, "int"))
	
	# Store reference
	_property_editors[property] = spin_box

func _add_float_property(property: String, label_text: String, initial_value: float) -> void:
	var container = HBoxContainer.new()
	container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	var label = Label.new()
	label.text = label_text
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(label)
	
	var spin_box = SpinBox.new()
	spin_box.min_value = -100
	spin_box.max_value = 100
	spin_box.step = 0.1
	spin_box.value = initial_value
	spin_box.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(spin_box)
	
	properties_container.add_child(container)
	
	# Connect signal
	spin_box.value_changed.connect(_on_property_changed.bind(property, "float"))
	
	# Store reference
	_property_editors[property] = spin_box

func _add_bool_property(property: String, label_text: String, initial_value: bool) -> void:
	var container = HBoxContainer.new()
	container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	var label = Label.new()
	label.text = label_text
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(label)
	
	var check_box = CheckBox.new()
	check_box.button_pressed = initial_value
	container.add_child(check_box)
	
	properties_container.add_child(container)
	
	# Connect signal
	check_box.toggled.connect(_on_property_changed.bind(property, "bool"))
	
	# Store reference
	_property_editors[property] = check_box

func _add_vector2_property(property: String, label_text: String, initial_value: Vector2) -> void:
	var container = HBoxContainer.new()
	container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	var label = Label.new()
	label.text = label_text
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(label)
	
	var x_spin = SpinBox.new()
	x_spin.min_value = -100
	x_spin.max_value = 100
	x_spin.value = initial_value.x
	x_spin.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(x_spin)
	
	var y_spin = SpinBox.new()
	y_spin.min_value = -100
	y_spin.max_value = 100
	y_spin.value = initial_value.y
	y_spin.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(y_spin)
	
	properties_container.add_child(container)
	
	# Connect signals
	x_spin.value_changed.connect(_on_vector2_x_changed.bind(property, y_spin))
	y_spin.value_changed.connect(_on_vector2_y_changed.bind(property, x_spin))
	
	# Store references
	_property_editors[property + "_x"] = x_spin
	_property_editors[property + "_y"] = y_spin

func _add_rect2_property(property: String, label_text: String, initial_value: Rect2) -> void:
	# Add section label
	var section_label = Label.new()
	section_label.text = label_text
	properties_container.add_child(section_label)
	
	# Position
	var pos_container = HBoxContainer.new()
	pos_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	var pos_label = Label.new()
	pos_label.text = "Position"
	pos_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	pos_container.add_child(pos_label)
	
	var x_spin = SpinBox.new()
	x_spin.min_value = -10000
	x_spin.max_value = 10000
	x_spin.value = initial_value.position.x
	x_spin.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	pos_container.add_child(x_spin)
	
	var y_spin = SpinBox.new()
	y_spin.min_value = -10000
	y_spin.max_value = 10000
	y_spin.value = initial_value.position.y
	y_spin.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	pos_container.add_child(y_spin)
	
	properties_container.add_child(pos_container)
	
	# Size
	var size_container = HBoxContainer.new()
	size_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	var size_label = Label.new()
	size_label.text = "Size"
	size_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	size_container.add_child(size_label)
	
	var w_spin = SpinBox.new()
	w_spin.min_value = 0
	w_spin.max_value = 10000
	w_spin.value = initial_value.size.x
	w_spin.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	size_container.add_child(w_spin)
	
	var h_spin = SpinBox.new()
	h_spin.min_value = 0
	h_spin.max_value = 10000
	h_spin.value = initial_value.size.y
	h_spin.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	size_container.add_child(h_spin)
	
	properties_container.add_child(size_container)
	
	# Connect signals
	x_spin.value_changed.connect(_on_rect2_changed.bind(property, "x", y_spin, w_spin, h_spin))
	y_spin.value_changed.connect(_on_rect2_changed.bind(property, "y", x_spin, w_spin, h_spin))
	w_spin.value_changed.connect(_on_rect2_changed.bind(property, "w", x_spin, y_spin, h_spin))
	h_spin.value_changed.connect(_on_rect2_changed.bind(property, "h", x_spin, y_spin, w_spin))
	
	# Store references
	_property_editors[property + "_x"] = x_spin
	_property_editors[property + "_y"] = y_spin
	_property_editors[property + "_w"] = w_spin
	_property_editors[property + "_h"] = h_spin

func _add_texture_property(property: String, label_text: String, initial_value: Texture2D) -> void:
	var container = HBoxContainer.new()
	container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	var label = Label.new()
	label.text = label_text
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	container.add_child(label)
	
	var texture_path = LineEdit.new()
	texture_path.text = initial_value.resource_path if initial_value else ""
	texture_path.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	texture_path.editable = false
	container.add_child(texture_path)
	
	var browse_button = Button.new()
	browse_button.text = "Browse"
	container.add_child(browse_button)
	
	properties_container.add_child(container)
	
	# Connect signal
	browse_button.pressed.connect(_on_browse_texture_pressed.bind(property, texture_path))
	
	# Store reference
	_property_editors[property] = texture_path

func _update_preview() -> void:
	# Apply stylebox to preview panel
	preview_panel.add_theme_stylebox_override("panel", _stylebox)

func _on_stylebox_type_selected(index: int) -> void:
	# Create a new stylebox based on the selected type
	var new_stylebox: StyleBox
	
	match index:
		0: # StyleBoxFlat
			new_stylebox = StyleBoxFlat.new()
		1: # StyleBoxTexture
			new_stylebox = StyleBoxTexture.new()
		2: # StyleBoxLine
			new_stylebox = StyleBoxLine.new()
		3: # StyleBoxEmpty
			new_stylebox = StyleBoxEmpty.new()
	
	# Update stylebox
	_stylebox = new_stylebox
	
	# Create property editors for the new stylebox
	_create_property_editors()
	
	# Update preview
	_update_preview()
	
	# Emit signal
	stylebox_changed.emit(_stylebox)

func _on_property_changed(value, property: String, type: String) -> void:
	match type:
		"color":
			if _stylebox is StyleBoxFlat and property in ["bg_color", "border_color", "shadow_color"]:
				_stylebox.set(property, value)
			elif _stylebox is StyleBoxLine and property == "color":
				_stylebox.set(property, value)
		"int":
			if _stylebox is StyleBoxFlat and property in ["border_width_left", "border_width_top", "border_width_right", "border_width_bottom", 
														"corner_radius_top_left", "corner_radius_top_right", "corner_radius_bottom_right", "corner_radius_bottom_left",
														"shadow_size", "content_margin_left", "content_margin_top", "content_margin_right", "content_margin_bottom"]:
				_stylebox.set(property, value)
			elif _stylebox is StyleBoxLine and property == "thickness":
				_stylebox.set(property, value)
		"float":
			if _stylebox is StyleBoxTexture and property in ["content_margin_left", "content_margin_top", "content_margin_right", "content_margin_bottom",
													   "expand_margin_left", "expand_margin_top", "expand_margin_right", "expand_margin_bottom"]:
				_stylebox.set(property, value)
			elif _stylebox is StyleBoxLine and property in ["grow_begin", "grow_end"]:
				_stylebox.set(property, value)
		"bool":
			if _stylebox is StyleBoxTexture and property == "region_enabled":
				_stylebox.set(property, value)
			elif _stylebox is StyleBoxLine and property == "vertical":
				_stylebox.set(property, value)
	
	# Update preview
	_update_preview()
	
	# Emit signal
	stylebox_changed.emit(_stylebox)

func _on_vector2_x_changed(value: float, property: String, y_spin: SpinBox) -> void:
	var vector = Vector2(value, y_spin.value)
	
	if _stylebox is StyleBoxFlat and property == "shadow_offset":
		_stylebox.shadow_offset = vector
	
	# Update preview
	_update_preview()
	
	# Emit signal
	stylebox_changed.emit(_stylebox)

func _on_vector2_y_changed(value: float, property: String, x_spin: SpinBox) -> void:
	var vector = Vector2(x_spin.value, value)
	
	if _stylebox is StyleBoxFlat and property == "shadow_offset":
		_stylebox.shadow_offset = vector
	
	# Update preview
	_update_preview()
	
	# Emit signal
	stylebox_changed.emit(_stylebox)

func _on_rect2_changed(value: float, property: String, component: String, 
					  spin1: SpinBox, spin2: SpinBox, spin3: SpinBox) -> void:
	var rect = Rect2()
	
	match component:
		"x":
			rect.position.x = value
			rect.position.y = spin1.value
			rect.size.x = spin2.value
			rect.size.y = spin3.value
		"y":
			rect.position.x = spin1.value
			rect.position.y = value
			rect.size.x = spin2.value
			rect.size.y = spin3.value
		"w":
			rect.position.x = spin1.value
			rect.position.y = spin2.value
			rect.size.x = value
			rect.size.y = spin3.value
		"h":
			rect.position.x = spin1.value
			rect.position.y = spin2.value
			rect.size.x = spin3.value
			rect.size.y = value
	
	if _stylebox is StyleBoxTexture and property == "region_rect":
		_stylebox.region_rect = rect
	
	# Update preview
	_update_preview()
	
	# Emit signal
	stylebox_changed.emit(_stylebox)

func _on_browse_texture_pressed(property: String, path_edit: LineEdit) -> void:
	# Store current property and path edit for use in callback
	_file_dialog.meta = {"property": property, "path_edit": path_edit}
	_file_dialog.popup_centered_ratio(0.7)

func _on_texture_file_selected(path: String) -> void:
	var property = _file_dialog.meta["property"]
	var path_edit = _file_dialog.meta["path_edit"]
	
	# Load the texture resource
	var texture_resource = ResourceLoader.load(path)
	
	if texture_resource is Texture2D:
		path_edit.text = path
		
		if _stylebox is StyleBoxTexture and property == "texture":
			_stylebox.texture = texture_resource
		
		# Update preview
		_update_preview()
		
		# Emit signal
		stylebox_changed.emit(_stylebox)
	else:
		printerr("Selected file is not a valid texture resource")

# Setup highlight timer
func _setup_highlight_timer() -> void:
	if _highlight_timer == null:
		_highlight_timer = Timer.new()
		_highlight_timer.one_shot = true
		_highlight_timer.timeout.connect(_on_highlight_timeout)
		add_child(_highlight_timer)

# Get the type of this editor item
func get_type() -> String:
	return _type

# Get the name of this editor item
func get_name() -> String:
	return _name

# Highlight this item when selected from preview
func highlight() -> void:
	# Create a stylebox for highlighting
	var panel_style = get_theme_stylebox("panel", "PanelContainer").duplicate()
	if panel_style is StyleBoxFlat:
		panel_style.bg_color = _highlight_color
		add_theme_stylebox_override("panel", panel_style)
		
		# Start timer to remove highlight
		_highlight_timer.start(2.0) # 2 second highlight

# Remove highlight when timer expires
func _on_highlight_timeout() -> void:
	# Restore original style
	var panel_style = get_theme_stylebox("panel", "PanelContainer").duplicate()
	if panel_style is StyleBoxFlat:
		panel_style.bg_color = _original_color
		add_theme_stylebox_override("panel", panel_style)
