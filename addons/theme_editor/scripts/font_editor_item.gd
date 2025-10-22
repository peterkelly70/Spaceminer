@tool
extends PanelContainer

signal font_changed(font)

@onready var type_label: Label = $VBoxContainer/HeaderContainer/TypeLabel
@onready var name_label: Label = $VBoxContainer/HeaderContainer/NameLabel
@onready var font_path_edit: LineEdit = $VBoxContainer/FontContainer/FontPathEdit
@onready var browse_button: Button = $VBoxContainer/FontContainer/BrowseButton
@onready var preview_label: Label = $VBoxContainer/PreviewContainer/PreviewLabel

var _type: String
var _name: String
var _font: Font
var _file_dialog: FileDialog

func _ready() -> void:
	# Connect signals
	browse_button.pressed.connect(_on_browse_button_pressed)
	
	# Create file dialog
	_file_dialog = FileDialog.new()
	add_child(_file_dialog)
	
	# Configure file dialog
	_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	_file_dialog.add_filter("*.ttf,*.otf")
	
	# Connect file dialog signals
	_file_dialog.file_selected.connect(_on_font_file_selected)

func setup(type: String, name: String, font: Font) -> void:
	_type = type
	_name = name
	_font = font
	
	# Update UI
	type_label.text = type
	name_label.text = name
	
	# Set font path if available
	if font is FontFile:
		font_path_edit.text = font.resource_path
	
	# Update preview
	_update_preview()

func _update_preview() -> void:
	# Apply font to preview label
	preview_label.add_theme_font_override("font", _font)
	preview_label.text = "The quick brown fox jumps over the lazy dog"

func _on_browse_button_pressed() -> void:
	_file_dialog.popup_centered_ratio(0.7)

func _on_font_file_selected(path: String) -> void:
	# Load the font resource
	var font_resource = ResourceLoader.load(path)
	
	if font_resource is FontFile:
		_font = font_resource
		font_path_edit.text = path
		
		# Update preview
		_update_preview()
		
		# Emit signal
		font_changed.emit(_font)
	else:
		printerr("Selected file is not a valid font resource")
