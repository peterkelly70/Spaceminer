extends Node

var log_level: int = 1
# Remove the class_name to avoid hiding the autoload singleton
# class_name GameStateConnector

# This class connects the Game_Manager with the State_Manager
# It ensures that game phases are properly synchronized with the state system

func _ready():
	Logger.info(self, "Initializing...")
	
	# This node acts as a bridge. It's not a view, so it can't be registered
	# with the StateManager in the same way. This connection might need review
	# as StateManager does not appear to emit a `state_changed` signal.
	var state_manager = get_node_or_null("/root/StateManager")
	if state_manager and state_manager.has_signal("state_changed"):
		state_manager.connect("state_changed", Callable(self, "_on_state_changed"))
	else:
		Logger.warn(self, "Could not connect to StateManager's state_changed signal. It may not exist.")

	# Connect to Game_Manager signals
	Game_Manager.connect("turn_started", Callable(self, "_on_turn_started"))
	
	Logger.info(self, "Ready")

func _on_state_changed(new_state):
	Logger.info(self, "State changed to: %s" % GameState.GameState.keys()[new_state])
	
	# If we're entering the PLAYING state after a cutscene, start the game
	if new_state == GameState.GameState.PLAYING and not Game_Manager.is_game_started():
		Logger.info(self, "Starting new game after cutscene")
		Game_Manager.start_new_game()

func _on_turn_started(season, year):
	Logger.info(self, "Turn started: %s - Year %d" % [season, year])
	
	# Update the sidebar with new season/year information
	var sidebar = get_node_or_null("/root/MainGameView/HBoxContainer/Sidebar")
	if sidebar:
		var container = sidebar.get_node_or_null("Container")
		if container:
			var season_label = container.get_node_or_null("MarginContainer/VBoxContainer/SeasonSection/SeasonInfo/VBoxContainer/SeasonLabel")
			var year_label = container.get_node_or_null("MarginContainer/VBoxContainer/SeasonSection/SeasonInfo/VBoxContainer/YearLabel")
			
			if season_label:
				season_label.text = season
			
			if year_label:
				year_label.text = "Year " + str(year)
