extends Node

signal tooltip_about_to_show(control: Control, text: String)

# adjust to match ProjectSettings → gui/tooltip/default_delay
const TOOLTIP_DELAY := 0.5

var _hovered: Control = null
var _timer: Timer

func _ready() -> void:
	# create a one-shot timer
	_timer = Timer.new()
	_timer.wait_time = TOOLTIP_DELAY
	_timer.one_shot = true
	add_child(_timer)
	_timer.timeout.connect(_on_timeout)
	# Global input capture removed; do not call set_process_input(true)

# _input removed to avoid autoloads intercepting input globally

# Helper function to find the control under a given position
func _get_control_at_position(position: Vector2) -> Control:
	# Get the root viewport control
	var root = get_tree().root
	
	# Recursively find the deepest control at the position
	return _find_control_at_position(root, position)

# Recursively search for the deepest control at a position
func _find_control_at_position(node: Node, position: Vector2) -> Control:
	# Check if this node is a control and contains the position
	if node is Control:
		var control = node as Control
		
		# Skip if not visible or if mouse filter is set to ignore
		if !control.visible or control.mouse_filter == Control.MOUSE_FILTER_IGNORE:
			return null
		
		# Check if the point is within this control
		if !control.get_global_rect().has_point(position):
			return null
		
		# Search children in reverse order (top-most first)
		for i in range(node.get_child_count() - 1, -1, -1):
			var child = node.get_child(i)
			var found = _find_control_at_position(child, position)
			if found:
				return found
		
		# If no child contains the point, return this control
		return control
	
	# For non-Control nodes, just search their children
	else:
		for i in range(node.get_child_count() - 1, -1, -1):
			var child = node.get_child(i)
			var found = _find_control_at_position(child, position)
			if found:
				return found
	
	# Nothing found
	return null

func _on_timeout() -> void:
	if _hovered and _hovered.tooltip_text != "":
		emit_signal("tooltip_about_to_show", _hovered, _hovered.tooltip_text)
