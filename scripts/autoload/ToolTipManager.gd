extends Node

var log_level: int = 1

# Tooltip container
var tooltip_container: Control = null
var active_tooltip: Control = null
var tooltip_offset = Vector2(15, 15)
var tooltip_margin = Vector2(10, 10)

# Tooltip theme
var tooltip_theme = null

# Initialize the tooltip manager
func _ready() -> void:
	print("Initializing...")
	
	# Create a canvas layer for tooltips to ensure they appear on top
	var tooltip_layer = CanvasLayer.new()
	tooltip_layer.layer = 99  # Just below notifications
	tooltip_layer.name = "TooltipLayer"
	add_child(tooltip_layer)
	
	# Create a container for tooltips
	tooltip_container = Control.new()
	tooltip_container.name = "TooltipContainer"
	tooltip_container.anchor_right = 1.0
	tooltip_container.anchor_bottom = 1.0
	tooltip_container.mouse_filter = Control.MOUSE_FILTER_IGNORE
	tooltip_layer.add_child(tooltip_container)
	
	print("Initialized")

# Register a control to show a tooltip
func register_tooltip(control: Control, tooltip_text: String) -> void:
	if not control:
		push_error("Cannot register tooltip: control is null")
		return
	
	# Connect mouse entered/exited signals
	if not control.mouse_entered.is_connected(Callable(self, "_on_control_mouse_entered")):
		control.mouse_entered.connect(_on_control_mouse_entered.bind(control, tooltip_text))
	
	if not control.mouse_exited.is_connected(Callable(self, "_on_control_mouse_exited")):
		control.mouse_exited.connect(_on_control_mouse_exited.bind(control))
	
	# Store tooltip text in control metadata
	control.set_meta("tooltip_text", tooltip_text)
	
	print("Registered tooltip for: %s" % control.name)

# Set the tooltip theme
func set_tooltip_theme(theme: Theme) -> void:
	tooltip_theme = theme
	
	print("Set tooltip theme")

# Show a tooltip at the current mouse position
func show_tooltip(tooltip_text: String) -> void:
	# Hide any existing tooltip
	hide_tooltip()
	
	# Create a new tooltip
	var tooltip = _create_tooltip(tooltip_text)
	tooltip_container.add_child(tooltip)
	active_tooltip = tooltip
	
	# Position the tooltip
	_update_tooltip_position()

# Hide the current tooltip
func hide_tooltip() -> void:
	if active_tooltip:
		active_tooltip.queue_free()
		active_tooltip = null

# Create a tooltip control
func _create_tooltip(tooltip_text: String) -> Control:
	# Create a panel container for the tooltip
	var panel = PanelContainer.new()
	panel.name = "Tooltip"
	
	# Apply theme if available
	if tooltip_theme:
		panel.theme = tooltip_theme
	
	# Create a margin container
	var margin = MarginContainer.new()
	margin.add_theme_constant_override("margin_left", 8)
	margin.add_theme_constant_override("margin_right", 8)
	margin.add_theme_constant_override("margin_top", 4)
	margin.add_theme_constant_override("margin_bottom", 4)
	panel.add_child(margin)
	
	# Create a label for the tooltip text
	var label = Label.new()
	label.text = tooltip_text
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	margin.add_child(label)
	
	# Set initial position off-screen
	panel.position = Vector2(-1000, -1000)
	
	return panel

# Update the tooltip position based on the mouse position
func _update_tooltip_position() -> void:
	if not active_tooltip:
		return
	
	# Get the mouse position
	var mouse_pos = get_viewport().get_mouse_position()
	
	# Calculate the tooltip position
	var tooltip_pos = mouse_pos + tooltip_offset
	
	# Ensure the tooltip stays within the viewport
	var viewport_size = get_viewport().get_visible_rect().size
	var tooltip_size = active_tooltip.get_combined_minimum_size() + tooltip_margin
	
	# Adjust horizontal position if needed
	if tooltip_pos.x + tooltip_size.x > viewport_size.x:
		tooltip_pos.x = mouse_pos.x - tooltip_size.x - tooltip_offset.x
	
	# Adjust vertical position if needed
	if tooltip_pos.y + tooltip_size.y > viewport_size.y:
		tooltip_pos.y = mouse_pos.y - tooltip_size.y - tooltip_offset.y
	
	# Set the tooltip position
	active_tooltip.position = tooltip_pos

# Global input handling removed to avoid autoload capturing input

# Signal handlers
func _on_control_mouse_entered(control: Control, tooltip_text: String) -> void:
	# Show the tooltip
	show_tooltip(tooltip_text)

func _on_control_mouse_exited(_control: Control) -> void:
	# Hide the tooltip
	hide_tooltip()
