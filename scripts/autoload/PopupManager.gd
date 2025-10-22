extends Node

var log_level: int = 1
# Removed class_name to avoid hiding the autoload singleton

signal popup_closed(popup_name: String, result: Dictionary)

# Dictionary to store loaded popup scenes
var _popup_scenes: Dictionary = {}
var _current_popup: Control = null
var _popup_canvas_layer: CanvasLayer = null
var _popup_container: Control = null

# Constants
const POPUP_DIRECTORY = "res://scenes/ui/popups/"
const DEFAULT_TRANSITION_TIME = 0.3

func _ready() -> void:
	Logger.info(self, "Initializing...")
	
	# Create a canvas layer for popups if not already present
	if not _popup_canvas_layer:
		_popup_canvas_layer = CanvasLayer.new()
		_popup_canvas_layer.layer = 100  # High layer to ensure popups are on top
		_popup_canvas_layer.name = "PopupLayer"
		add_child(_popup_canvas_layer)
		
		# Create a container for popups
		_popup_container = Control.new()
		_popup_container.name = "PopupContainer"
		_popup_container.anchor_right = 1.0
		_popup_container.anchor_bottom = 1.0
		# IMPORTANT: Do not intercept input unless a popup is actually visible
		_popup_container.mouse_filter = Control.MOUSE_FILTER_IGNORE
		_popup_canvas_layer.add_child(_popup_container)
	
	Logger.info(self, "Initialized successfully")

# Show a popup by name
func show_popup(popup_name: String, params: Dictionary = {}) -> void:
	Logger.info(self, "Showing popup: %s" % popup_name)
	
	# If there's already a popup showing, hide it first
	if _current_popup:
		hide_current_popup()
	
	# Load the popup scene if not already loaded
	var popup_instance = _get_popup_instance(popup_name)
	if not popup_instance:
		Logger.error(self, "Failed to load popup: %s" % popup_name)
		return
	
	# While a popup is presented, block background interaction
	_popup_container.mouse_filter = Control.MOUSE_FILTER_STOP
	if popup_instance is Control:
		popup_instance.mouse_filter = Control.MOUSE_FILTER_STOP

	# Add the popup to the container
	_popup_container.add_child(popup_instance)
	_current_popup = popup_instance
	
	# Initialize the popup with parameters if it has the method
	if popup_instance.has_method("init_popup"):
		popup_instance.init_popup(params)
	
	# Connect signals
	if popup_instance.has_signal("closed"):
		if not popup_instance.closed.is_connected(_on_popup_closed):
			popup_instance.closed.connect(_on_popup_closed.bind(popup_name))
	
	# Show the popup with animation
	if popup_instance.has_method("show_popup"):
		popup_instance.show_popup()
	else:
		# Default animation if the popup doesn't have its own
		popup_instance.modulate.a = 0
		popup_instance.visible = true
		var tween = create_tween()
		tween.tween_property(popup_instance, "modulate:a", 1.0, DEFAULT_TRANSITION_TIME)

# Hide the current popup
func hide_current_popup(result: Dictionary = {}) -> void:
	if not _current_popup:
		return
	
	var popup_name = _current_popup.name
	
	# Use custom hide method if available
	if _current_popup.has_method("hide_popup"):
		_current_popup.hide_popup()
	else:
		# Default animation
		var tween = create_tween()
		tween.tween_property(_current_popup, "modulate:a", 0.0, DEFAULT_TRANSITION_TIME)
		tween.tween_callback(func(): _cleanup_popup(popup_name, result))

	# As we begin hiding, ensure we restore background interactivity after cleanup
	# (Actual switch back to IGNORE happens in _cleanup_popup)
	
	# Emit signal
	emit_signal("popup_closed", popup_name, result)

# Get a popup instance by name
func _get_popup_instance(popup_name: String) -> Control:
	# Check if we already have this popup loaded
	if _popup_scenes.has(popup_name):
		return _popup_scenes[popup_name].instantiate()
	
	# Try to load the popup scene
	var popup_path = POPUP_DIRECTORY + popup_name + ".tscn"
	if ResourceLoader.exists(popup_path):
		var popup_scene = load(popup_path)
		_popup_scenes[popup_name] = popup_scene
		return popup_scene.instantiate()
	
	Logger.error(self, "Popup scene not found: %s" % popup_path)
	return null

# Cleanup popup and process queue
func _cleanup_popup(_popup_name: String, _result: Dictionary = {}) -> void:
	if _current_popup:
		_current_popup.queue_free()
		_current_popup = null
	# No popup active -> do not intercept input
	if _popup_container:
		_popup_container.mouse_filter = Control.MOUSE_FILTER_IGNORE

# Signal handler for popup closed
func _on_popup_closed(popup_name: String, result: Dictionary = {}) -> void:
	_cleanup_popup(popup_name, result)
