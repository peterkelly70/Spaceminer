extends Node

const Logger = preload("res://scripts/class/Logger.gd")
var log_level: int = 1

# Game state management singleton
# This class manages the game's state system and notifies registered components of state changes
@export var version = "0.0.2"

# Current game state
var _current_state: GameState.GameState = GameState.GameState.MAIN_MENU
var _previous_state: GameState.GameState = GameState.GameState.MAIN_MENU

# State aware views and controllers
var _state_aware_views = []
var _state_emitters = []

func _ready() -> void:
	Logger.info(self, "Initialized")
	
	# Set process input to handle global key events
	set_process_input(true)

# Register a view or controller that needs to be notified of state changes
func register_state_aware_view(view) -> void:
	if view not in _state_aware_views:
		_state_aware_views.append(view)
		Logger.info(self, "Registered view: %s" % view.name)

# Register a component that can emit state change requests
func register_state_emitter(emitter) -> void:
	if emitter not in _state_emitters:
		_state_emitters.append(emitter)
		emitter.connect("request_state_change", Callable(self, "_on_state_change_requested"))
		Logger.info(self, "Registered emitter: %s" % emitter.name)

# get Version info
func get_version() -> String:	
	return version

# Get the current game state
func get_current_state() -> GameState.GameState:
	return _current_state

# Get the previous game state
func get_previous_state() -> GameState.GameState:
	return _previous_state

# Return to the previous game state
func return_to_previous_state() -> void:
	change_state(_previous_state)

# Change the game state
func change_state(new_state: GameState.GameState) -> void:
	var old_state = _current_state
	
	# Check if state is changing or staying the same
	var is_same_state = (new_state == _current_state)
	
	# Update state tracking variables
	_current_state = new_state
	if not is_same_state:
		_previous_state = old_state
	
	# Log state change or refresh
	if is_same_state:
		Logger.info(self, "State refreshed: %s (triggering view updates)" % GameState.GameState.keys()[new_state])
	else:
		Logger.info(self, "State changed: %s -> %s" % [GameState.GameState.keys()[old_state], GameState.GameState.keys()[new_state]])
	
	# Notify all registered views and controllers
	# Always notify, even for same state, to allow phase updates
	for view in _state_aware_views:
		if is_instance_valid(view):
			view.receive_state_ping(new_state)

# Handle state change requests from emitters
func _on_state_change_requested(new_state: GameState.GameState) -> void:
	change_state(new_state)
