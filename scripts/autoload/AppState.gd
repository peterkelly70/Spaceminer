extends Node

var log_level: int = 1

# Application-level states (global screens/flows)
enum State {
	SPLASH,			 # Splash Screen
	GAMESETUP,		 # Game setup panel
	MAIN_MENU,		 # Main Menu
	SETTINGS,		 # Settings screen
	CREDITS,		 # Credits screen
	PLAYING,		 # Gameplay viewport
	PAUSE,			 # Pause overlay
	QUIT			 # Quit sentinel for menu_view fallback
}

# Current app state
var _current_state: State = State.MAIN_MENU

# State aware views and controllers
var _state_aware_views = []
var _state_emitters = []

func _ready() -> void:
	print("Initialized")

# Register a view or controller that needs to be notified of state changes
func register_state_aware_view(view) -> void:
	if view not in _state_aware_views:
		_state_aware_views.append(view)
		print("Registered view: %s" % view.name)

# Register a component that can emit state change requests
func register_state_emitter(emitter) -> void:
	if emitter not in _state_emitters:
		_state_emitters.append(emitter)
		emitter.connect("request_state_change", Callable(self, "_on_state_change_requested"))
		print("Registered emitter: %s" % emitter.name)

# Get the current game state
func get_current_state() -> State:
	return _current_state

# Change the game state
func change_state(new_state: State) -> void:
	if new_state == _current_state:
		return
		
	var old_state = _current_state
	_current_state = new_state

	print("State changed: %s -> %s" % [State.keys()[old_state], State.keys()[new_state]])
	
	# Notify all registered views and controllers
	for view in _state_aware_views:
		if is_instance_valid(view):
			view.receive_state_ping(new_state)

# Handle state change requests from emitters
func _on_state_change_requested(new_state: State) -> void:
	change_state(new_state)
