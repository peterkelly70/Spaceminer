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
	"CreditsView": preload("res://scenes/states/CreditsView.tscn"),
	"MainGameView": load("res://scenes/states/MainGameView.tscn"),
}

const PAUSE_POPUP_SCENE := preload("res://scenes/ui/popups/PausePopup.tscn")
const MAP_POPUP_SCENE := preload("res://scenes/ui/popups/MapPopup.tscn")

# Dictionary of themes that can be accessed throughout the game
var themes := {
	"DEFAULT": preload("res://assets/themes/default_theme.tres"),
	"PARCHMENT": preload("res://assets/themes/parchment_theme.tres"),
	# Add more themes as needed
}

var scene_nodes: Dictionary = {}
var pause_popup: Control = null
var map_popup: Control = null

func _input(event: InputEvent) -> void:
	# CapsLock toggles music mute
	if event is InputEventKey and event.pressed and not event.echo:
		if event.keycode == KEY_CAPSLOCK:
			var am := get_node_or_null("/root/Audio_Manager")
			if am and am.has_method("toggle_music_mute"):
				am.toggle_music_mute()
			elif am and am.has_method("set_music_muted"):
				am.set_music_muted(not am.music_muted)
			get_viewport().set_input_as_handled()
			return

		if event.is_action_pressed("map_toggle"):
			var current_state := State_Manager.get_current_state()
			if current_state == AppState.State.PLAYING:
				_toggle_map_popup()
				get_viewport().set_input_as_handled()
				return
			if current_state == AppState.State.PAUSE and map_popup and map_popup.visible:
				_close_map_popup()
				get_viewport().set_input_as_handled()
				return

	# P key pauses/resumes while playing
	if event.is_action_pressed("pause_game"):
		var current_state := State_Manager.get_current_state()
		if current_state == AppState.State.PLAYING:
			_open_pause_popup()
			get_viewport().set_input_as_handled()
			return
		if current_state == AppState.State.PAUSE:
			_resume_from_pause()
			get_viewport().set_input_as_handled()
			return

	if event.is_action_pressed("ui_cancel"):
		var current_state := State_Manager.get_current_state()
		if map_popup and map_popup.visible:
			_close_map_popup()
			_open_pause_popup()
			get_viewport().set_input_as_handled()
			return
		if current_state == AppState.State.MAIN_MENU:
			print("ESC detected in menu; requesting game quit")
			get_tree().quit()
			return
		if current_state == AppState.State.PLAYING:
			_open_pause_popup()
			get_viewport().set_input_as_handled()
			return
		if current_state == AppState.State.PAUSE:
			_resume_from_pause()
			get_viewport().set_input_as_handled()
			return

func _unhandled_input(event: InputEvent) -> void:
	pass

func _ready() -> void:
	print("Instancing scenes...")
	process_mode = Node.PROCESS_MODE_ALWAYS
	set_process_input(true)
	set_process_unhandled_input(true)
	
	
	# Register game audio
	_register_game_audio()
	
	# Register notification themes and types
	_register_notification_settings()

	# Ensure this root does not block mouse input; delegate to scene children
	mouse_filter = Control.MOUSE_FILTER_IGNORE

	for key in scenes:
		var packed = scenes[key]
		if packed == null:
			push_warning("Scene '%s' failed to load; skipping instantiation" % key)
			continue
		var inst = packed.instantiate()
		container.add_child(inst)
		scene_nodes[key] = inst

		# Controllers self-register via StateAwareController; no centralized registration here
		print(" - Instanced: %s" % key)

	# CRITICAL: Force HIDE all scenes first
	for name in scene_nodes:
		var node = scene_nodes[name]
		# Only set visibility properties on Control nodes
		if node is Control:
			node.visible = false
			print("Hide: %s" % name)
		else:
			print("Skip non-Control node: %s" % name)
	
	# CRITICAL: Only enable MenuView
	if scene_nodes.has("MenuView"):
		scene_nodes["MenuView"].visible = true
		print("SHOW MENU EXCLUSIVELY")
	else:
		push_error("MenuView not found!")
		
	# CRITICAL: Force hide other scenes that may intercept input
	if scene_nodes.has("Splash") and scene_nodes["Splash"] is Control:
		scene_nodes["Splash"].visible = false
		_hide_scene_ui(scene_nodes["Splash"]) # Also hide any Controls under CanvasLayers

	if scene_nodes.has("MainGameView") and scene_nodes["MainGameView"] is Control:
		scene_nodes["MainGameView"].visible = false
		# Do NOT recursively hide children; StateAwareController will toggle visibility when state changes
	
	# Add pause popup on top of the scene stack
	if pause_popup == null:
		pause_popup = PAUSE_POPUP_SCENE.instantiate()
		container.add_child(pause_popup)
		pause_popup.visible = false
		scene_nodes["PausePopup"] = pause_popup
	else:
		pause_popup.visible = false

	if map_popup == null:
		map_popup = MAP_POPUP_SCENE.instantiate()
		container.add_child(map_popup)
		map_popup.visible = false
		scene_nodes["MapPopup"] = map_popup
	else:
		map_popup.visible = false

	# CRITICAL: Trigger MAIN_MENU state so MenuController becomes interactive
	await get_tree().process_frame
	if State_Manager.get_current_state() == AppState.State.MAIN_MENU:
		print("Transitioning to MAIN_MENU state")
		State_Manager.change_state(AppState.State.MAIN_MENU)
	else:
		print("Skipping MAIN_MENU reset; state already changed to %s" % AppState.State.keys()[State_Manager.get_current_state()])

	print("Child tree state (post-visibility):")
	for node in container.get_children():
		print(" - %s" % node.name)
		print("    Class:         %s" % node.get_class())
		print("    In tree:       %s" % node.is_inside_tree())
		# Only print visible property for Control nodes
		if node is Control:
			print("    Visible:       %s" % node.visible)
		else:
			print("    Visible:       N/A (not a Control node)")
		print("    Script:        %s" % (node.get_script() if node.get_script() else "None"))

func _open_pause_popup() -> void:
	var current_state := State_Manager.get_current_state()
	if current_state != AppState.State.PLAYING:
		return
	_close_map_popup()
	get_tree().paused = true
	State_Manager.change_state(AppState.State.PAUSE)
	if pause_popup:
		pause_popup.visible = true
		pause_popup.raise()
		if pause_popup.has_method("focus_default"):
			pause_popup.call("focus_default")

func _resume_from_pause() -> void:
	get_tree().paused = false
	State_Manager.change_state(AppState.State.PLAYING)
	_hide_pause_popup()

func _toggle_map_popup() -> void:
	if map_popup and map_popup.visible:
		_close_map_popup()
	else:
		_open_map_popup()

func _open_map_popup() -> void:
	var current_state := State_Manager.get_current_state()
	if current_state != AppState.State.PLAYING:
		return
	_hide_pause_popup()
	# Map is a non-pausing overlay — the game keeps running while it's open
	if map_popup:
		var seed_text := ""
		if RunManager and RunManager.has_method("get_active_seed_display"):
			seed_text = RunManager.get_active_seed_display()
		var room_name: String = ""
		if RunManager and RunManager.has_method("get_current_room_id"):
			room_name = RunManager.get_current_room_id()
		var campaign_name := ""
		var room_count := 0
		var manifest: Dictionary = {}
		if RunManager and RunManager.has_method("get_active_campaign_manifest"):
			manifest = RunManager.get_active_campaign_manifest()
		if RunManager:
			campaign_name = str(RunManager.active_run.get("campaign", ""))
			room_count = int(RunManager.active_run.get("room_count", 0))
		if map_popup.has_method("setup"):
			map_popup.call("setup", seed_text, room_name, campaign_name, room_count, manifest)
		map_popup.visible = true
		map_popup.raise()

func _close_map_popup() -> void:
	get_tree().paused = false
	if map_popup:
		map_popup.visible = false

func _hide_pause_popup() -> void:
	if pause_popup:
		pause_popup.visible = false

# Recursively hide any Control descendants (including those under CanvasLayer) for a given scene node
func _hide_scene_ui(root: Node) -> void:
	for child in root.get_children():
		if child is Control:
			child.visible = false
		_hide_scene_ui(child)

# Initialize the music system using MusicManager and resource files
func _register_game_audio() -> void:
	print("Initializing music system...")
	var audio_manager = get_node_or_null("/root/Audio_Manager")
	if not audio_manager:
		push_error("Audio_Manager not found!")
		return

	# Try to play user-saved track first; fall back to bluedanube
	var saved_folder: String = SettingsManager.get_setting("audio", "music_folder", "8bit")
	if saved_folder.is_empty():
		saved_folder = "8bit"
	var saved_track: String = SettingsManager.get_setting("audio", "music_track", "")

	# Register tracks from catalogue (reliable at runtime), fall back to DirAccess
	var music_root := "res://assets/audio/music/"
	var folder_path := music_root + saved_folder
	var catalog_path := folder_path + "/music_catalog.gd"
	var registered := false
	if ResourceLoader.exists(catalog_path):
		var cat: GDScript = load(catalog_path)
		if cat:
			var constants: Dictionary = cat.get_script_constant_map()
			if constants.has("MUSIC_TRACKS"):
				for track in constants["MUSIC_TRACKS"]:
					audio_manager.register_music(str(track["path"]), str(track["name"]), true, bool(track.get("persistent", false)))
				registered = true
	if not registered:
		var audio_exts := ["mp3", "ogg", "wav"]
		var dir := DirAccess.open(folder_path)
		if dir:
			dir.list_dir_begin()
			var fname := dir.get_next()
			while fname != "":
				if not dir.current_is_dir() and audio_exts.has(fname.get_extension().to_lower()):
					var full := folder_path + "/" + fname
					var track_name := fname.get_basename()
					audio_manager.register_music(full, track_name, true, track_name == "bluedanube")
				fname = dir.get_next()
			dir.list_dir_end()

	# Play: saved preference → bluedanube → first registered track
	var played := false
	if saved_track != "" and audio_manager.music_tracks.has(saved_track):
		played = audio_manager.play_and_set_persistent_music(saved_track)
	if not played and audio_manager.music_tracks.has("bluedanube"):
		played = audio_manager.play_and_set_persistent_music("bluedanube")
	if not played:
		for tname in audio_manager.music_tracks:
			played = audio_manager.play_and_set_persistent_music(tname)
			if played:
				break
	print("Music boot: %s" % ("playing" if played else "no track found"))

	# Register sound effects
	var sound_effects = {
		"click": "res://assets/audio/sfx/retro/retro_button_click.wav",
		"button_hover": "res://assets/audio/sfx/retro/retro_button_hover.wav",
		"button_release": "res://assets/audio/sfx/retro/retro_button_release.wav",
		"button_focus": "res://assets/audio/sfx/retro/retro_button_focus.wav",
		"build": "res://assets/audio/sfx/ui/build.mp3",
		"error": "res://assets/audio/sfx/ui/error.mp3"
	}
	
	# Register all sound effects
	for sfx_name in sound_effects:
		audio_manager.register_sfx(sound_effects[sfx_name], sfx_name)
	
	# Register cutscene voiceovers
	var voiceovers_dir = "res://assets/audio/voiceover/introduction/"
	var vo_dir = DirAccess.open(voiceovers_dir)
	if vo_dir:
		vo_dir.list_dir_begin()
		var file_name = vo_dir.get_next()
		while file_name != "":
			if not vo_dir.current_is_dir() and (file_name.ends_with(".mp3") or file_name.ends_with(".wav") or file_name.ends_with(".ogg")):
				var full_path = voiceovers_dir + file_name
				var vo_name = "vo_" + file_name.get_basename()
				audio_manager.register_sfx(full_path, vo_name)
			file_name = vo_dir.get_next()
	else:
		push_warning("Could not access voiceovers directory: %s" % voiceovers_dir)
	
	print("Music and audio initialization complete")

# Called when the music track changes
func _on_music_track_changed(track: MusicTrack) -> void:
	print("Music track changed: %s" % track.get_display_string())

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
	print("Registering notification settings...")
	
	# Get the notification_manager
	var notification_manager = get_node_or_null("/root/Notification_Manager")
	if not notification_manager:
		push_error("Notification_Manager not found!")
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
	
	print("Notification settings registration complete")

# Get a theme by name
func get_ui_theme(theme_name: String):
	if themes.has(theme_name):
		return themes[theme_name]
	return null
