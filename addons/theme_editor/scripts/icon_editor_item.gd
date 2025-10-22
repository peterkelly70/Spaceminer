@tool
extends PanelContainer

signal icon_changed(icon)

@onready var type_label: Label = $VBoxContainer/HeaderContainer/TypeLabel
@onready var name_label: Label = $VBoxContainer/HeaderContainer/NameLabel
@onready var icon_path_edit: LineEdit = $VBoxContainer/IconContainer/IconPathEdit
@onready var browse_button: Button = $VBoxContainer/IconContainer/BrowseButton
@onready var preview_texture: TextureRect = $VBoxContainer/PreviewContainer/PreviewTexture

# Highlight effect
var _highlight_timer: Timer
var _original_color: Color
var _highlight_color: Color = Color(1, 0.5, 0, 0.3)

var _type: String
var _name: String
var _icon: Texture2D
var _file_dialog: FileDialog

func _ready() -> void:
	# Connect signals
	browse_button.pressed.connect(_on_browse_button_pressed)
	
	# Create file dialog
	_file_dialog = FileDialog.new()
	add_child(_file_dialog)
	
	# Configure file dialog
	_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	_file_dialog.add_filter("*.png,*.jpg,*.jpeg,*.svg")
	
	# Connect file dialog signals
	_file_dialog.file_selected.connect(_on_icon_file_selected)

func setup(type: String, name: String, icon: Texture2D) -> void:
	_type = type
	_name = name
	_icon = icon
	
	# Update UI
	type_label.text = type
	name_label.text = name
	
	# Set icon path if available
	if icon:
		icon_path_edit.text = icon.resource_path
	
	# Update preview
	_update_preview()
	
	# Setup highlight timer
	_setup_highlight_timer()
	
	# Store original color
	_original_color = self.get_theme_stylebox("panel", "PanelContainer").bg_color

func _update_preview() -> void:
	# Apply icon to preview texture rect
	preview_texture.texture = _icon

func _on_browse_button_pressed() -> void:
	_file_dialog.popup_centered_ratio(0.7)

func _on_icon_file_selected(path: String) -> void:
	# Load the icon resource
	var icon_resource = ResourceLoader.load(path)
	
	if icon_resource is Texture2D:
		_icon = icon_resource
		icon_path_edit.text = path
		
		# Update preview
		_update_preview()
		
		# Emit signal
		icon_changed.emit(_icon)
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
