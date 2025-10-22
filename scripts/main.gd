extends Control
 
var log_level: int = 1

# Popup confirmation options for consistent use across the project
const CONFIRM := {
	"ONLY": true,   # Show only the confirm button (informational popup)
	"MODAL": false, # Whether the popup should be modal (blocks background interaction)
	"ONLY_MODE": true, # Use this for informational popups with just an OK button
	"CONFIRM_MODE": false # Use this for confirmation popups with YES/NO buttons
}

@onready var container = self  # Change if you want to add to a specific panel/child

var scenes := {
	"Splash": preload("res://scenes/states/Splash.tscn"),
	"MenuView": preload("res://scenes/states/MenuView.tscn"),
	"SettingsView": preload("res://scenes/states/SettingsView.tscn"),
	"CreditsView": load("res://scenes/states/CreditsView.tscn"),
	"GameSetupView": preload("res://scenes/states/GameSetupView.tscn"),
	"MainGameView": load("res://scenes/states/MainGameView.tscn"),
}

# Dictionary of themes that can be accessed throughout the game
var themes := {
	"DEFAULT": preload("res://assets/themes/default_theme.tres"),
	"PARCHMENT": preload("res://assets/themes/parchment_theme.tres"),
	# Add more themes as needed
}

var scene_nodes: Dictionary = {}

func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_cancel"):
		var current_state := State_Manager.get_current_state()
		if current_state == GameState.GameState.MAIN_MENU:
			Logger.info(self, "ESC detected in menu; requesting game quit")
			get_tree().quit()
			return
		Logger.info(self, "ESC detected; returning to main menu from state: %s" % GameState.GameState.keys()[current_state])
		State_Manager.change_state(GameState.GameState.MAIN_MENU)
		for name in scene_nodes:
			var node = scene_nodes[name]
			if node is Control:
				node.visible = (name == "MenuView")
			else:
				continue

func _ready() -> void:
	Logger.info(self, "Instancing scenes...")
	
	
	# Register game audio
	_register_game_audio()
	
	# Register notification themes and types
	_register_notification_settings()

	# Ensure this root does not block mouse input; delegate to scene children
	mouse_filter = Control.MOUSE_FILTER_IGNORE

	for key in scenes:
		var packed = scenes[key]
		if packed == null:
			Logger.warn(self, "Scene '%s' failed to load; skipping instantiation" % key)
			continue
		var inst = packed.instantiate()
		container.add_child(inst)
		scene_nodes[key] = inst

		# Controllers self-register via StateAwareController; no centralized registration here
		Logger.debug(self, " - Instanced: %s" % key)

	# CRITICAL: Force HIDE all scenes first
	for name in scene_nodes:
		var node = scene_nodes[name]
		# Only set visibility properties on Control nodes
		if node is Control:
			node.visible = false
			Logger.debug(self, "Hide: %s" % name)
		else:
			Logger.debug(self, "Skip non-Control node: %s" % name)
	
	# CRITICAL: Only enable MenuView
	if scene_nodes.has("MenuView"):
		scene_nodes["MenuView"].visible = true
		Logger.info(self, "SHOW MENU EXCLUSIVELY")
	else:
		Logger.error(self, "MenuView not found!")
		
	# CRITICAL: Force hide other scenes that may intercept input
	if scene_nodes.has("Splash") and scene_nodes["Splash"] is Control:
		scene_nodes["Splash"].visible = false
		_hide_scene_ui(scene_nodes["Splash"]) # Also hide any Controls under CanvasLayers
	
	if scene_nodes.has("CreditsView") and scene_nodes["CreditsView"] is Control:
		scene_nodes["CreditsView"].visible = false
		# Do NOT recursively hide children; StateAwareController will toggle visibility when state changes

	if scene_nodes.has("MainGameView") and scene_nodes["MainGameView"] is Control:
		scene_nodes["MainGameView"].visible = false
		# Do NOT recursively hide children; StateAwareController will toggle visibility when state changes

	# CRITICAL: Trigger MAIN_MENU state so MenuController becomes interactive
	await get_tree().process_frame
	Logger.info(self, "Transitioning to MAIN_MENU state")
	State_Manager.change_state(GameState.GameState.MAIN_MENU)

	Logger.debug(self, "Child tree state (post-visibility):")
	for node in container.get_children():
		Logger.debug(self, " - %s" % node.name)
		Logger.debug(self, "    Class:         %s" % node.get_class())
		Logger.debug(self, "    In tree:       %s" % node.is_inside_tree())
		# Only print visible property for Control nodes
		if node is Control:
			Logger.debug(self, "    Visible:       %s" % node.visible)
		else:
			Logger.debug(self, "    Visible:       N/A (not a Control node)")
		Logger.debug(self, "    Script:        %s" % (node.get_script() if node.get_script() else "None"))

# Recursively hide any Control descendants (including those under CanvasLayer) for a given scene node
func _hide_scene_ui(root: Node) -> void:
	for child in root.get_children():
		if child is Control:
			child.visible = false
		_hide_scene_ui(child)

# Initialize the music system using MusicManager and resource files
func _register_game_audio() -> void:
	Logger.info(self, "Initializing music system...")
	
	# Create and setup MusicManager
	var music_manager = MusicManager.new()
	music_manager.name = "MusicManager"
	add_child(music_manager)
	
	# Determine preferred music folder from settings if present
	var saved_folder: String = SettingsManager.get_setting("audio", "music_folder", "")

	# Load music collection using preferred folder when available
	var collection_loaded = music_manager.load_music_collection_from_directory(saved_folder)
	if collection_loaded:
		Logger.info(self, "Karl Casey collection loaded successfully")

		# Connect to track change signal for notifications
		music_manager.track_changed.connect(_on_music_track_changed)
		Logger.info(self, "Connected to music_manager.track_changed signal")

		# Respect user-selected persistent track if set
		var audio_manager = get_node_or_null("/root/Audio_Manager")
		var saved_track: String = SettingsManager.get_setting("audio", "music_track", "")
		var played_user_track := false
		if saved_track != "" and audio_manager:
			# If already loaded via collection or previous session, just play it
			if audio_manager.music_tracks.has(saved_track):
				played_user_track = audio_manager.play_and_set_persistent_music(saved_track)
			else:
				# Try to locate the file in the saved folder
				if saved_folder != "":
					var root := "res://assets/audio/music/" + saved_folder
					var exts = ["mp3", "ogg", "wav"]
					var dir := DirAccess.open(root)
					if dir:
						dir.list_dir_begin()
						var fname := dir.get_next()
						while fname != "":
							if not dir.current_is_dir():
								var base := fname.get_basename()
								if base == saved_track and exts.has(fname.get_extension().to_lower()):
									var full := root + "/" + fname
									audio_manager.register_music(full, saved_track, true, true)
									played_user_track = audio_manager.play_and_set_persistent_music(saved_track)
									break
							fname = dir.get_next()
						dir.list_dir_end()
		
		if played_user_track:
			# If user selection is respected, do not auto-switch or play random
			music_manager.setup_auto_switching(false)
			Logger.info(self, "Playing user-selected persistent track: %s" % saved_track)
		else:
			# Fallback behaviour: auto-switch and start a random track after a short delay
			music_manager.setup_auto_switching(true)
			await get_tree().create_timer(1.0).timeout
			music_manager.play_random_track()
	else:
		Logger.error(self, "Failed to load Karl Casey music collection")
	
	# Get the Audio_Manager for sound effects registration
	var audio_manager = get_node_or_null("/root/Audio_Manager")
	if not audio_manager:
		Logger.error(self, "Audio_Manager not found!")
		return
	
	# Register sound effects
	var sound_effects = {
		"click": "res://assets/audio/sfx/ui/button.mp3",
		"build": "res://assets/audio/sfx/ui/build.mp3",
		"error": "res://assets/audio/sfx/ui/error.mp3",
		# Temporary remap: use button.mp3 for hover until button_hover.mp3 is imported
		"button_hover": "res://assets/audio/sfx/ui/button.mp3"
	}
	
	# Register all sound effects
	for sfx_name in sound_effects:
		audio_manager.register_sfx(sound_effects[sfx_name], sfx_name)
	
	# Register cutscene voiceovers
	var voiceovers_dir = "res://assets/audio/voiceover/introduction/"
	var dir = DirAccess.open(voiceovers_dir)
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			if not dir.current_is_dir() and (file_name.ends_with(".mp3") or file_name.ends_with(".wav") or file_name.ends_with(".ogg")):
				var full_path = voiceovers_dir + file_name
				var vo_name = "vo_" + file_name.get_basename()
				audio_manager.register_sfx(full_path, vo_name)
			file_name = dir.get_next()
	else:
		Logger.warn(self, "Could not access voiceovers directory: %s" % voiceovers_dir)
	
	Logger.info(self, "Music and audio initialization complete")

# Called when the music track changes
func _on_music_track_changed(track: MusicTrack) -> void:
	Logger.info(self, "Music track changed: %s" % track.get_display_string())

# Extract track number from file path
func _extract_track_number(file_path: String) -> String:
	# Look for pattern like "- 01 " or "- 02 " in the filename
	var filename = file_path.get_file()
	var regex = RegEx.new()
	regex.compile("- (\\d{2}) ")
	var result = regex.search(filename)
	if result:
		return result.get_string(1)
	return ""

# Register notification settings with the Notification_Manager
func _register_notification_settings() -> void:
	Logger.info(self, "Registering notification settings...")
	
	# Get the notification_manager
	var notification_manager = get_node_or_null("/root/Notification_Manager")
	if not notification_manager:
		Logger.error(self, "Notification_Manager not found!")
		return
	
	# Register notification type mappings
	notification_manager.register_theme_mapping(
		notification_manager.NotificationType.MUSIC, 
		"DEFAULT"
	)
	
	# Register notification location mappings
	notification_manager.register_location_mapping(
		notification_manager.NotificationType.MUSIC, 
		notification_manager.Position.BOTTOM_LEFT
	)
	
	Logger.info(self, "Notification settings registration complete")

# Get a theme by name
func get_ui_theme(theme_name: String):
	if themes.has(theme_name):
		return themes[theme_name]
	return null
