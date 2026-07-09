extends Node

var log_level: int = 1
# Removed class_name to avoid hiding the autoload singleton

# Default resolution
const DEFAULT_WIDTH: int = 1920
const DEFAULT_HEIGHT: int = 1080

func _ready():
	print("Initializing...")
	
	# Set the window to fullscreen on startup
	if OS.has_feature("standalone"):  # Only force fullscreen in standalone builds
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
	
	# Ensure correct resolution
	var current_size = DisplayServer.window_get_size()
	if current_size.x != DEFAULT_WIDTH or current_size.y != DEFAULT_HEIGHT:
		DisplayServer.window_set_size(Vector2i(DEFAULT_WIDTH, DEFAULT_HEIGHT))
		
	# Center the window on screen
	var screen_size = DisplayServer.screen_get_size()
	var window_size = DisplayServer.window_get_size()
	var centered_pos = (screen_size - window_size) / 2
	DisplayServer.window_set_position(centered_pos)
	
	print("Window set to: %s" % DisplayServer.window_get_size())
	print("Window mode: %s" % DisplayServer.window_get_mode())
	
	# Connect to window resize signals
	get_window().size_changed.connect(_on_window_size_changed)
	
	print("Initialized")

func _on_window_size_changed():
	var new_size = DisplayServer.window_get_size()
	print("Window resized to: %s" % new_size)
	
	# You could implement additional logic here for handling different resolutions

func toggle_fullscreen():
	var current_mode = DisplayServer.window_get_mode()
	
	if current_mode == DisplayServer.WINDOW_MODE_FULLSCREEN:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
		print("Switched to windowed mode")
	else:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
		print("Switched to fullscreen mode")
	
	return DisplayServer.window_get_mode() == DisplayServer.WINDOW_MODE_FULLSCREEN

func set_fullscreen(enable: bool):
	if enable:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
		print("Enabled fullscreen mode")
	else:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
		print("Disabled fullscreen mode")
