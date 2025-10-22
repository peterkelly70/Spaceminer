extends Control
class_name BasePopup

var log_level: int = 1

# Signals
signal closed
signal action(action_name: String, data: Dictionary)

# Optional properties that can be set by derived popups
var title: String = "Popup"
var can_close_with_escape: bool = true
var modal: bool = true
var exclusive: bool = true

# Theme reference
const DEFAULT_THEME_PATH := "res://assets/themes/default_theme.tres"

# Internal state
var _initialized: bool = false
var _params: Dictionary = {}

func _ready() -> void:
	# Set up modal behavior
	if modal:
		set_process_input(true)
	
	# Find close button if it exists
	var close_button = get_node_or_null("CenterContainer/PanelContainer/MarginContainer/VBoxContainer/TitleContainer/CloseButton")
	if close_button and close_button is Button:
		close_button.pressed.connect(_on_close_button_pressed)
	
	# Apply any custom styling or theming here
	_apply_theme()

func _apply_theme() -> void:
	# Load the default theme
	var default_theme = load(DEFAULT_THEME_PATH)
	if not default_theme:
		Logger.error(self, "Failed to load default theme from: %s" % [DEFAULT_THEME_PATH])
		return
	
	# Apply the theme to this popup
	theme = default_theme
	
	# Get nodes
	var panel = get_node_or_null("CenterContainer/PanelContainer")
	var title_container = get_node_or_null("CenterContainer/PanelContainer/MarginContainer/VBoxContainer/TitleContainer")
	var title_label = title_container.get_node_or_null("TitleLabel") if title_container else null
	var content_label = get_node_or_null("CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/ContentLabel")
	var confirm_button = get_node_or_null("CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/ConfirmButton")
	var cancel_button = get_node_or_null("CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/CancelButton")
	
	# Apply specific overrides if needed
	if panel:
		# Use the PanelContainer style from the theme
		panel.add_theme_stylebox_override("panel", default_theme.get_stylebox("panel", "PanelContainer"))
	
	# Set title container style
	if title_container:
		# Use TabContainer selected tab style for the title header
		title_container.add_theme_stylebox_override("panel", default_theme.get_stylebox("tab_selected", "TabContainer"))
	
	# Set title font
	if title_label:
		title_label.add_theme_font_override("font", default_theme.get_font("font", "Label"))
		title_label.add_theme_font_size_override("font_size", 24)
		title_label.add_theme_color_override("font_color", default_theme.get_color("font_color", "Label"))
	
	# Set content font
	if content_label:
		# Load SourceSansPro font for paragraph text
		var paragraph_font = load("res://assets/fonts/SourceSansPro-Regular.ttf")
		if paragraph_font:
			content_label.add_theme_font_override("font", paragraph_font)
		else:
			content_label.add_theme_font_override("font", default_theme.get_font("font", "Label"))
		
		content_label.add_theme_font_size_override("font_size", 18)
		content_label.add_theme_color_override("font_color", default_theme.get_color("font_color", "Label"))
	
	# Apply button styles from the theme
	if confirm_button:
		# Use Button styles from the theme
		confirm_button.theme = default_theme
	
	if cancel_button:
		# Use Button styles from the theme
		cancel_button.theme = default_theme

func _input(event: InputEvent) -> void:
	if can_close_with_escape and event is InputEventKey:
		if event.pressed and event.keycode == KEY_ESCAPE:
			_close()
			get_viewport().set_input_as_handled()

# Initialize the popup with parameters
func init_popup(params: Dictionary = {}) -> void:
	_params = params
	_initialized = true
	
	# Set title if provided
	if params.has("title"):
		title = params.title
		var title_label = get_node_or_null("TitleLabel")
		if title_label and title_label is Label:
			title_label.text = title
	
	# Call custom initialization in derived popups
	_init_custom(params)

# Override in derived classes for custom initialization
func _init_custom(_init_params: Dictionary) -> void:
	pass

# Close the popup
func _close() -> void:
	emit_signal("closed")
	# Hide this popup
	hide()
	# If we're part of a PopupManager, it will handle the rest via the signal

# Handle close button press
func _on_close_button_pressed() -> void:
	_close()

# Emit an action signal
func _emit_action(action_name: String, data: Dictionary = {}) -> void:
	emit_signal("action", action_name, data)

# Handle confirm button press
func _on_confirm_button_pressed() -> void:
	_emit_action("confirm")
	_close()

# Handle cancel button press
func _on_cancel_button_pressed() -> void:
	_emit_action("cancel")
	_close()
