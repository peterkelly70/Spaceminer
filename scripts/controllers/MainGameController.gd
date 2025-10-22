extends StateAwareController
class_name MainGameController

# UI References
@onready var game_background = $GameBackground
@onready var sidebar = $HBoxContainer/Sidebar
@onready var ui_area = $HBoxContainer/ContentArea/GameContent
@onready var cutscene_container = $CutsceneLayer/CutsceneContainer

# Game state
var current_section: String = "WILL_AND_WAY"
var game_initialized: bool = false
var introduction_played: bool = false
var game_phase_manager = null

# Cutscene player
var cutscene_player = null

func _ready() -> void:
	Logger.info(self, "Initializing...")
	
	# Set states where this controller should be visible
	show_in_states = [GameState.GameState.PLAYING]
	super._ready()
	
	# Connect to GameManager signals
	_connect_game_manager_signals()
	
	# Initialize the cutscene player
	_initialize_cutscene_player()
	Logger.info(self, "Ready")

func _initialize_cutscene_player() -> void:
	Logger.info(self, "Initializing cutscene player...")
	
	# Instance the CutscenePlayer scene instead of creating a new object
	var cutscene_scene = load("res://scenes/CutscenePlayer.tscn")
	if not cutscene_scene:
		Logger.error(self, "Failed to load CutscenePlayer scene")
		return
		
	cutscene_player = cutscene_scene.instantiate()
	cutscene_container.add_child(cutscene_player)
	
	# Set up the cutscene player
	cutscene_player.size_flags_horizontal = Control.SIZE_FILL
	cutscene_player.size_flags_vertical = Control.SIZE_FILL
	cutscene_player.anchors_preset = Control.PRESET_FULL_RECT
	
	# Connect signals
	cutscene_player.cutscene_completed.connect(_on_cutscene_finished)
	
	Logger.info(self, "Cutscene player initialized")

func _initialize_game() -> void:
	Logger.info(self, "Initializing game...")

	# [Fail Fast] Start new game logic in GameManager
	if Game_Manager:
		Game_Manager.start_new_game()
		Logger.info(self, "Called Game_Manager.start_new_game()!")
	else:
		Logger.error(self, "Game_Manager not found! Game will not start.")
		return

	# Make the controller visible first
	visible = true
	# Wait for one frame to ensure UI elements are properly initialized
	await get_tree().process_frame
	
	# Update resource displays
	_update_resource_displays()
	
	# Wait for one more frame to ensure everything is properly initialized
	await get_tree().process_frame
	
	# Set game as initialized
	game_initialized = true

	Logger.info(self, "Game initialization complete")
	
	# Wait a short time to ensure everything is fully initialized
	await get_tree().create_timer(0.5).timeout
	
	# Show turn start popup after game is fully initialized
	_show_turn_start_popup()


func _update_resource_displays() -> void:
	# Update resource values and tooltips in the sidebar
	if not sidebar:
		return

	var container = sidebar.get_node_or_null("Container")
	if not container:
		return

	# Get resource data from Game_Manager
	var resource_array = Game_Manager.get_resource_list()
	var resource_tooltip_map = {}
	for res in resource_array:
		if res.has("id") and res.has("name") and res.has("description"):
			resource_tooltip_map[res.id] = {
				"title": res.name,
				"desc": res.description
			}

	# Get current resource values and generation from Game_Manager (replace with real calls)
	var resources = Game_Manager.get_resource_state() if Game_Manager.has_method("get_resource_state") else {}
	var generation = Game_Manager.get_all_resource_generation() if Game_Manager.has_method("get_all_resource_generation") else {}

	# For each resource in the loaded resource list, update value/generation and set tooltips
	for resource_id in resource_tooltip_map.keys():
		# Update resource display without tooltips
		
		# Update resource value display
		var value_label = container.get_node_or_null("MarginContainer/VBoxContainer/ResourceSection/ResourceDisplay/ResourcePanel/ResourceGrid/" + resource_id.capitalize() + "Container/" + resource_id.capitalize() + "Value")
		if value_label:
			value_label.text = str(resources.get(resource_id, 0))

		# Update resource generation display
		var gen_label = container.get_node_or_null("MarginContainer/VBoxContainer/ResourceGeneration/Panel/Grid/" + resource_id.capitalize() + "GenContainer/" + resource_id.capitalize() + "GenValue")
		if gen_label:
			var value = generation.get(resource_id, 0)
			gen_label.text = ("+" if value > 0 else "") + str(value)
			# Set color based on value
			if value > 0:
				gen_label.add_theme_color_override("font_color", Color(0.2, 0.8, 0.2))
			elif value < 0:
				gen_label.add_theme_color_override("font_color", Color(0.8, 0.2, 0.2))
			else:
				gen_label.remove_theme_color_override("font_color")
	
	# Update season and year
	var season_label = container.get_node_or_null("MarginContainer/VBoxContainer/SeasonSection/SeasonInfo/VBoxContainer/SeasonLabel")
	var year_label = container.get_node_or_null("MarginContainer/VBoxContainer/SeasonSection/SeasonInfo/VBoxContainer/YearLabel")
	
	if season_label:
		season_label.text = "Spring"
	
	if year_label:
		year_label.text = "Year 1"

func _play_introduction_cutscene() -> void:
	Logger.info(self, "Playing introduction cutscene...")
	
	# Check if cutscenes are disabled in settings
	if SettingsManager.get_setting("gameplay", "hide_cutscenes", false):
		Logger.info(self, "Cutscenes are disabled in settings, skipping introduction")
		introduction_played = true
		_initialize_game()
		return
	
	# Hide the sidebar during the cutscene - with proper null checks and error handling
	if sidebar:
		Logger.debug(self, "[Fail Loud] Hiding sidebar for cutscene")
		sidebar.visible = false
		var container = sidebar.get_node_or_null("Container")
		if container:
			container.visible = false
	else:
		Logger.warn(self, "[Fail Loud] Sidebar not found when trying to hide for cutscene")
	
	# Check if we have a valid cutscene player
	if cutscene_player:
		# Try to load the introduction cutscene
		load_cutscene_async("res://assets/cutscenes/Introduction.tres")
	else:
		Logger.error(self, "Cutscene player not initialized")
		_initialize_game()

# Async wrapper for loading cutscenes
func load_cutscene_async(path: String) -> void:
	var task = func():
		var success = await cutscene_player.load_cutscene(path)
		
		if success:
			introduction_played = true
			Logger.info(self, "Introduction cutscene started")
		else:
			Logger.error(self, "Failed to load introduction cutscene")
			# If cutscene fails to load, just initialize the game
			_initialize_game()
	
	# Start the async task
	task.call()

func _on_cutscene_finished() -> void:
	Logger.info(self, "Cutscene finished")
	
	# Reset cutscene player state
	if cutscene_player:
		cutscene_player.visible = false
		# Reset any cutscene state variables
		cutscene_player.is_playing = false
		cutscene_player.auto_advance_timer_active = false
		cutscene_player.current_frame = 0
	
	# Wait for one frame to ensure UI updates
	await get_tree().process_frame
	
	# Initialize the game if it's the introduction cutscene
	if not game_initialized:
		_initialize_game()
	else:
		# For other cutscenes, just return to the playing state
		request_state(GameState.GameState.PLAYING)
		# Show turn start popup after returning to playing state
		_show_turn_start_popup()
		
		# Restore sidebar visibility if needed
		if sidebar:
			sidebar.visible = true
			var container = sidebar.get_node_or_null("Container")
			if container:
				container.visible = true
			else:
				Logger.error(self, "[Fail Fast] Sidebar container not found after cutscene!")

func _on_will_button_pressed() -> void:
	Logger.info(self, "Will and the Way button pressed")
	request_state(GameState.GameState.PLAYING)


func on_enter_state(state: int) -> void:
	Logger.debug(self, "[Fail Loud] Entering state: %s" % GameState.GameState.keys()[state])
	# [Fail Fast] Make sure we're visible first
	visible = true
	# sidebar.visible = true
	Logger.debug(self, "Controller visibility set to: %s" % visible)
	
	# Set current section based on state
	# If this is the first time entering a game state and introduction hasn't been played
	if not introduction_played and state == GameState.GameState.PLAYING:
		_play_introduction_cutscene()
	elif not game_initialized:
		# Initialize the game if not already done
		_initialize_game()
	elif game_initialized and state == GameState.GameState.PLAYING:
		# Always update resource displays
		_update_resource_displays()
		# Always update phase visibility
		_update_phase_visibility()

# Control phase visibility based on current game phase
func _update_phase_visibility() -> void:
	# [Fail Fast] Ensure Game_Manager is available
	if not Game_Manager:
		Logger.error(self, "[Fail Fast] Game_Manager not found!")
		return
	
	# Get current phase from Game_Manager
	var current_phase = Game_Manager.get_current_phase()
	var phase_name = Game_Manager.get_current_phase_name()
	Logger.debug(self, "[Fail Loud] Updating phase visibility for current phase: %s (index: %d)" % [phase_name, current_phase])
	
	# [Fail Fast] Ensure content area exists
	if not ui_area:
		Logger.error(self, "[Fail Fast] Content area not found!")
		return
	
	# Debug content area
	Logger.debug(self, "[Fail Loud] Content area path: %s" % ui_area.get_path())
	Logger.debug(self, "[Fail Loud] Content area children count: %d" % ui_area.get_child_count())
	
	# List all children for debugging
	Logger.debug(self, "[Fail Loud] Content area children:")
	for i in range(ui_area.get_child_count()):
		var child = ui_area.get_child(i)
		Logger.debug(self, "  - %s (visible: %s)" % [child.name, child.visible])
	
	# Find all phase nodes (phase01, phase02, etc.)
	var phase_nodes = []
	for child in ui_area.get_children():
		if child.name.begins_with("phase"):
			phase_nodes.append(child)
			Logger.debug(self, "[Fail Loud] Found phase node: %s" % child.name)
	
	# [Fail Fast] Check if we found any phase nodes
	if phase_nodes.is_empty():
		Logger.error(self, "[Fail Fast] No phase nodes found in content area!")
		
		# EMERGENCY FALLBACK: Create a placeholder phase01 node if none exists
		Logger.warn(self, "[Fail Loud] Creating emergency placeholder phase01 node")
		var placeholder = Label.new()
		placeholder.name = "phase01"
		placeholder.text = "PHASE 01: ASSIGN WORKERS"
		placeholder.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		placeholder.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		placeholder.add_theme_color_override("font_color", Color(1, 1, 1))
		placeholder.add_theme_font_size_override("font_size", 32)
		ui_area.add_child(placeholder)
		phase_nodes.append(placeholder)
		Logger.warn(self, "[Fail Loud] Added emergency placeholder phase01 node")
	
	# Hide all phase nodes first
	for node in phase_nodes:
		node.visible = false
		Logger.debug(self, "[Fail Loud] Set %s visibility to false" % node.name)
	
	


func on_exit_state(state: int) -> void:
	Logger.info(self, "Exiting state: %s" % GameState.GameState.keys()[state])
	visible = false

# Connect to all Game_Manager signals
func _connect_game_manager_signals() -> void:
	Logger.info(self, "Connecting to Game_Manager signals...")
	
	# [Fail Fast] Ensure Game_Manager is available
	if not Game_Manager:
		Logger.error(self, "[Fail Fast] Game_Manager not found! Cannot connect signals.")
		return
	
	# Connect to all relevant Game_Manager signals
	Game_Manager.resources_updated.connect(_on_game_manager_resources_updated)
	Game_Manager.workers_updated.connect(_on_game_manager_workers_updated)
	Game_Manager.turn_started.connect(_on_game_manager_turn_started)
	Game_Manager.turn_ended.connect(_on_game_manager_turn_ended)
	Game_Manager.phase_changed.connect(_on_game_manager_phase_changed)
	Game_Manager.game_loaded.connect(_on_game_manager_game_loaded)
	Game_Manager.game_reset.connect(_on_game_manager_game_reset)
	Game_Manager.update_sidebar.connect(_on_game_manager_update_sidebar)
	
	Logger.info(self, "Connected to Game_Manager signals")

# Signal handler functions
func _on_game_manager_resources_updated() -> void:
	Logger.info(self, "Resources updated signal received")
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot update resources.")
		return
	
	# Update resource displays
	_update_resource_displays()
	
	# Forward the signal to the sidebar if it has the appropriate method
	if sidebar.has_method("update_resources"):
		sidebar.update_resources()

# Handle workers updated signal
func _on_game_manager_workers_updated() -> void:
	Logger.info(self, "Workers updated signal received")
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot update workers.")
		return
	
	# Forward the signal to the sidebar if it has the appropriate method
	if sidebar.has_method("update_workers"):
		sidebar.update_workers()

# Handle turn started signal
func _on_game_manager_turn_started(combined_season: String, year: int) -> void:
	Logger.info(self, "Turn started signal received: %s, Year %d" % [combined_season, year])
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot update turn.")
		return
	
	# Update all sidebar elements
	_update_sidebar_with_turn_info(combined_season, year)

# Handle turn ended signal
func _on_game_manager_turn_ended() -> void:
	Logger.info(self, "Turn ended signal received")
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot handle turn end.")
		return
	
	# Forward the signal to the sidebar if it has the appropriate method
	if sidebar.has_method("handle_turn_ended"):
		sidebar.handle_turn_ended()

# Handle phase changed signal
func _on_game_manager_phase_changed(new_phase) -> void:
	Logger.info(self, "Phase changed signal received: %s" % new_phase)
	
	# Update phase visibility in the game view
	_update_phase_visibility()
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot update phase.")
		return
	
	# Forward the signal to the sidebar if it has the appropriate method
	if sidebar.has_method("update_phase_display"):
		sidebar.update_phase_display(Game_Manager.get_current_phase_name())

# Handle game loaded signal
func _on_game_manager_game_loaded() -> void:
	Logger.info(self, "Game loaded signal received")
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot handle game loaded.")
		return
	
	# Forward the signal to the sidebar if it has the appropriate method
	if sidebar.has_method("handle_game_loaded"):
		sidebar.handle_game_loaded()
		
	# Update resource displays
	_update_resource_displays()
	
	# Update phase visibility
	_update_phase_visibility()

# Handle game reset signal
func _on_game_manager_game_reset() -> void:
	Logger.info(self, "Game reset signal received")
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot handle game reset.")
		return
	
	# Forward the signal to the sidebar if it has the appropriate method
	if sidebar.has_method("handle_game_reset"):
		sidebar.handle_game_reset()

# Centralized method to update all sidebar elements with turn information
func _update_sidebar_with_turn_info(season_string: String, year: int) -> void:
	Logger.info(self, "Updating sidebar with turn info: %s, Year %d" % [season_string, year])
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot update sidebar.")
		return
	
	# Update season display
	if sidebar.has_method("update_season_display"):
		sidebar.update_season_display(season_string, year)
	
	# Update resources
	if sidebar.has_method("update_resources"):
		sidebar.update_resources()
	
	# Update workers
	if sidebar.has_method("update_workers"):
		sidebar.update_workers()
	
	Logger.info(self, "Sidebar updated successfully")

# Direct handler for update_sidebar signal
func _on_game_manager_update_sidebar() -> void:
	Logger.info(self, "Received direct update_sidebar signal")
	
	# [Fail Fast] Ensure sidebar exists
	if not sidebar:
		Logger.error(self, "[Fail Fast] Sidebar not found! Cannot update sidebar directly.")
		return
	
	# Get current season and month information from Game_Manager
	var season_name = Game_Manager.get_current_season_name()
	var month_name = Game_Manager.get_current_month_name()
	var year = Game_Manager.get_current_year()
	
	# Create combined season string
	var combined_season = "%s - %s" % [season_name, month_name]
	
	# Update all sidebar elements
	_update_sidebar_with_turn_info(combined_season, year)
	
	Logger.info(self, "Sidebar directly updated via update_sidebar signal")

# Method to get the sidebar reference - used by Game_Manager for direct access
func get_sidebar() -> Node:
	if sidebar:
		return sidebar
	else:
		Logger.error(self, "Sidebar reference requested but not found!")
		return null

# Show the turn start popup using Game_Manager
func _show_turn_start_popup() -> void:
	Logger.info(self, "Showing turn start popup")
	
	# [Fail Fast] Ensure Game_Manager exists
	if not Game_Manager:
		Logger.error(self, "[Fail Fast] Game_Manager not found! Cannot show turn start popup.")
		return
	
	# Call the Game_Manager method to show the turn start popup
	if Game_Manager.has_method("_show_initial_turn_popup"):
		Game_Manager._show_initial_turn_popup()
		Logger.info(self, "Turn start popup shown")
	else:
		Logger.error(self, "[Fail Fast] Game_Manager does not have _show_initial_turn_popup method!")
