extends Node

const Logger = preload("res://scripts/class/Logger.gd")
var log_level: int = 1

# Game state enums (centralized)
enum GameState {
	SPLASH,			 # Splash Screen
	GAMESETUP,		 # GAMESETUP
	MAIN_MENU,		 # Main Menu
	SETTINGS,		 # SETTINGS SCREEN
	CREDITS,		 # CREDITS SCREEN
	PLAYING,         # Playing
	PAUSE			 # PAUSE 
}

enum GamePhase {
	PHASE_01,
	PHASE_02,
	PHASE_03,
	PHASE_04,
	PHASE_05
}


# Current game state
var _current_state = GameState.MAIN_MENU

# State aware views and controllers
var _state_aware_views = []
var _state_emitters = []

func _ready() -> void:
	Logger.info(self, "Initialized")

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

# Get the current game state
func get_current_state() -> GameState:
	return _current_state

# Change the game state
func change_state(new_state: GameState) -> void:
	if new_state == _current_state:
		return
		
	var old_state = _current_state
	_current_state = new_state
	
	Logger.info(self, "State changed: %s -> %s" % [GameState.keys()[old_state], GameState.keys()[new_state]])
	
	# Notify all registered views and controllers
	for view in _state_aware_views:
		if is_instance_valid(view):
			view.receive_state_ping(new_state)

# Handle state change requests from emitters
func _on_state_change_requested(new_state: GameState) -> void:
	change_state(new_state)
