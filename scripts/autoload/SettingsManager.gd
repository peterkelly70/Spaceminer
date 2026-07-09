extends Node

var log_level: int = 1

# Settings file path
const CONFIG_FILE_PATH = "user://settings.cfg"


# Default settings
var settings = {
	"video": {
		"fullscreen": true,
		"vsync": true,
		"resolution": Vector2i(1920, 1080),
		"preferred_mode": DisplayServer.WINDOW_MODE_FULLSCREEN
	},
	"audio": {
		"music_enabled": true,
		"music_volume": 0.8,
		"sfx_enabled": true,
		"sfx_volume": 0.8,
		"music_muted": false,
		"music_folder": "",
		"music_track": ""
	},
	"gameplay": {
		"show_timer": true,
		"hints_enabled": true,
		"show_completed": true,
		"show_clue_highlight": true,
		"show_grid_highlight": true,
		"hide_cutscenes": false,
		"show_welcome_notification": false,
		"show_music_notifications": false,
		"notification_duration": 30.0
	}
}

# Signal for when settings change
signal settings_changed(section, key, value)
signal show_completed_changed(is_visible)
signal timer_visibility_changed(is_visible)
signal music_muted_changed(is_muted)
signal clue_highlight_changed(is_visible)
signal grid_highlight_changed(is_visible)
signal hide_cutscenes_changed(is_hidden)
signal notification_duration_changed(duration)
signal music_track_requested(track_name)

# Get AudioManager singleton reference
@onready var audio_manager = get_node("/root/Audio_Manager") if has_node("/root/Audio_Manager") else null

func _ready():
	print("Initializing...")
	load_settings()
	apply_settings()
	print("Settings loaded and applied")

# Load settings from file
func load_settings() -> void:
	var config = ConfigFile.new()
	var err = config.load(CONFIG_FILE_PATH)
	
	if err == OK:
		print("Settings file found, loading values")
		
		# Load video settings
		if config.has_section("video"):
			if config.has_section_key("video", "fullscreen"):
				settings.video.fullscreen = config.get_value("video", "fullscreen")
			if config.has_section_key("video", "vsync"):
				settings.video.vsync = config.get_value("video", "vsync")
			if config.has_section_key("video", "resolution"):
				settings.video.resolution = config.get_value("video", "resolution")
		
		# Load audio settings
		if config.has_section("audio"):
			var has_music_enabled := config.has_section_key("audio", "music_enabled")
			var has_music_muted := config.has_section_key("audio", "music_muted")
			if has_music_enabled:
				settings.audio.music_enabled = config.get_value("audio", "music_enabled")
			if config.has_section_key("audio", "music_volume"):
				settings.audio.music_volume = config.get_value("audio", "music_volume")
			if config.has_section_key("audio", "sfx_enabled"):
				settings.audio.sfx_enabled = config.get_value("audio", "sfx_enabled")
			if config.has_section_key("audio", "sfx_volume"):
				settings.audio.sfx_volume = config.get_value("audio", "sfx_volume")
			if has_music_muted:
				settings.audio.music_muted = config.get_value("audio", "music_muted")
			if config.has_section_key("audio", "music_folder"):
				settings.audio.music_folder = config.get_value("audio", "music_folder")
			if config.has_section_key("audio", "music_track"):
				settings.audio.music_track = config.get_value("audio", "music_track")
			if has_music_enabled:
				settings.audio.music_muted = not settings.audio.music_enabled
			elif has_music_muted:
				settings.audio.music_enabled = not settings.audio.music_muted
		
		# Load gameplay settings
		if config.has_section("gameplay"):
			if config.has_section_key("gameplay", "show_timer"):
				settings.gameplay.show_timer = config.get_value("gameplay", "show_timer")
			if config.has_section_key("gameplay", "hints_enabled"):
				settings.gameplay.hints_enabled = config.get_value("gameplay", "hints_enabled")
			if config.has_section_key("gameplay", "show_completed"):
				settings.gameplay.show_completed = config.get_value("gameplay", "show_completed")
			if config.has_section_key("gameplay", "show_clue_highlight"):
				settings.gameplay.show_clue_highlight = config.get_value("gameplay", "show_clue_highlight")
			if config.has_section_key("gameplay", "show_grid_highlight"):
				settings.gameplay.show_grid_highlight = config.get_value("gameplay", "show_grid_highlight")
			if config.has_section_key("gameplay", "hide_cutscenes"):
				settings.gameplay.hide_cutscenes = config.get_value("gameplay", "hide_cutscenes")
			if config.has_section_key("gameplay", "show_welcome_notification"):
				settings.gameplay.show_welcome_notification = config.get_value("gameplay", "show_welcome_notification")
			if config.has_section_key("gameplay", "show_music_notifications"):
				settings.gameplay.show_music_notifications = config.get_value("gameplay", "show_music_notifications")
			if config.has_section_key("gameplay", "notification_duration"):
				settings.gameplay.notification_duration = config.get_value("gameplay", "notification_duration")
	else:
		push_warning("No settings file found, using defaults")
		save_settings()

# Save settings to file
func save_settings() -> void:
	var config = ConfigFile.new()
	
	# Save video settings
	config.set_value("video", "fullscreen", settings.video.fullscreen)
	config.set_value("video", "vsync", settings.video.vsync)
	config.set_value("video", "resolution", settings.video.resolution)
	
	# Save audio settings
	config.set_value("audio", "music_enabled", settings.audio.music_enabled)
	config.set_value("audio", "music_volume", settings.audio.music_volume)
	config.set_value("audio", "sfx_enabled", settings.audio.sfx_enabled)
	config.set_value("audio", "sfx_volume", settings.audio.sfx_volume)
	config.set_value("audio", "music_muted", settings.audio.music_muted)
	config.set_value("audio", "music_folder", settings.audio.music_folder)
	config.set_value("audio", "music_track", settings.audio.music_track)
	
	# Save gameplay settings
	config.set_value("gameplay", "show_timer", settings.gameplay.show_timer)
	config.set_value("gameplay", "hints_enabled", settings.gameplay.hints_enabled)
	config.set_value("gameplay", "show_completed", settings.gameplay.show_completed)
	config.set_value("gameplay", "show_clue_highlight", settings.gameplay.show_clue_highlight)
	config.set_value("gameplay", "show_grid_highlight", settings.gameplay.show_grid_highlight)
	config.set_value("gameplay", "hide_cutscenes", settings.gameplay.hide_cutscenes)
	config.set_value("gameplay", "show_welcome_notification", settings.gameplay.show_welcome_notification)
	config.set_value("gameplay", "show_music_notifications", settings.gameplay.show_music_notifications)
	config.set_value("gameplay", "notification_duration", settings.gameplay.notification_duration)
	
	var err = config.save(CONFIG_FILE_PATH)
	if err == OK:
		print("Settings saved successfully")
	else:
		push_error("Failed to save settings: %s" % err)

# Get setting value
func get_setting(section: String, key: String, default = null):
	if not settings.has(section):
		return default
	
	if not settings[section].has(key):
		return default
	
	return settings[section][key]

# Set setting value and trigger updates
func set_setting(section: String, key: String, value) -> void:
	# Check if section exists
	if not settings.has(section):
		settings[section] = {}
	
	# Only continue if the value actually changed
	if settings[section].has(key) and settings[section][key] == value:
		return
	
	# Update setting
	settings[section][key] = value
	if section == "audio":
		match key:
			"music_enabled":
				settings.audio.music_muted = not bool(value)
			"music_muted":
				settings.audio.music_enabled = not bool(value)
	
	# Save immediately
	save_settings()
	
	# Emit signal
	settings_changed.emit(section, key, value)
	
	# Apply relevant settings
	match section:
		"audio":
			_apply_audio_settings()
			if key == "music_track":
				music_track_requested.emit(value)
		"video":
			_apply_video_settings()
		"gameplay":
			_apply_gameplay_settings()
			

# Apply all settings at once
func apply_settings() -> void:
	print("Applying all settings...")
	_apply_video_settings()
	_apply_audio_settings()
	_apply_gameplay_settings()
	# Any other settings categories...

# Apply video settings
func _apply_video_settings() -> void:
	print("Applying video settings")
	
	# Set fullscreen
	if settings.video.fullscreen:
		var current_mode = DisplayServer.window_get_mode()
		if current_mode != DisplayServer.WINDOW_MODE_FULLSCREEN:
			DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
			print("Set window mode to fullscreen")
	else:
		var current_mode = DisplayServer.window_get_mode()
		if current_mode != DisplayServer.WINDOW_MODE_WINDOWED:
			DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
			print("Set window mode to windowed")
	
	# Set vsync
	DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_ENABLED if settings.video.vsync else DisplayServer.VSYNC_DISABLED)
	
	# Set resolution (if needed and in windowed mode)
	if not settings.video.fullscreen:
		var current_size = DisplayServer.window_get_size()
		if current_size != settings.video.resolution:
			DisplayServer.window_set_size(settings.video.resolution)
			print("Set window size to: %s" % settings.video.resolution)
	
	# Center the window if in windowed mode
	if not settings.video.fullscreen:
		var screen_size = DisplayServer.screen_get_size()
		var window_size = DisplayServer.window_get_size()
		var centered_pos = (screen_size - window_size) / 2
		DisplayServer.window_set_position(centered_pos)
		print("Centered window on screen")

# Apply audio settings
func _apply_audio_settings() -> void:
	print("Applying audio settings")
	
	# Emit signals for any UI that needs to update
	if settings.has("audio"):
		if settings.audio.has("music_muted"):
			emit_signal("settings_changed", "audio", "music_muted", settings.audio.music_muted)
			emit_signal("music_muted_changed", settings.audio.music_muted)
	
	# First try using the AudioManager
	if audio_manager:
		# Keep the enabled and muted flags in sync, then apply the active state.
		var is_muted = not is_music_enabled()
		settings.audio.music_muted = is_muted
		settings.audio.music_enabled = not is_muted
		audio_manager.set_mute(is_muted)
		print("Applied mute setting to AudioManager: %s" % is_muted)
		
		# Set volume levels
		audio_manager.set_music_volume(settings.audio.music_volume)
		audio_manager.set_sfx_volume(settings.audio.sfx_volume)
		
		print("Applied settings to AudioManager")
	else:
		# Fallback to direct AudioServer manipulation
		push_warning("AudioManager not found, using fallback")
		var music_bus_idx = AudioServer.get_bus_index("Music")
		var sfx_bus_idx = AudioServer.get_bus_index("SFX")
		
		if music_bus_idx >= 0:
			AudioServer.set_bus_mute(music_bus_idx, is_music_muted())
			AudioServer.set_bus_volume_db(music_bus_idx, linear_to_db(settings.audio.music_volume))
		
		if sfx_bus_idx >= 0:
			AudioServer.set_bus_volume_db(sfx_bus_idx, linear_to_db(settings.audio.sfx_volume))

# Apply gameplay settings
func _apply_gameplay_settings()->void:
	print("Applying gameplay settings")
	
	# Emit signals for any UI that needs to update
	if settings.has("gameplay"):
		if settings.gameplay.has("show_timer"):
			emit_signal("settings_changed", "gameplay", "show_timer", settings.gameplay.show_timer)
			emit_signal("timer_visibility_changed", settings.gameplay.show_timer)
		
		if settings.gameplay.has("show_completed"):
			emit_signal("show_completed_changed", settings.gameplay.show_completed)
		
		if settings.gameplay.has("show_clue_highlight"):
			emit_signal("clue_highlight_changed", settings.gameplay.show_clue_highlight)
		
		if settings.gameplay.has("show_grid_highlight"):
			emit_signal("grid_highlight_changed", settings.gameplay.show_grid_highlight)
		
		if settings.gameplay.has("hide_cutscenes"):
			emit_signal("hide_cutscenes_changed", settings.gameplay.hide_cutscenes)
		
		if settings.gameplay.has("notification_duration"):
			emit_signal("notification_duration_changed", settings.gameplay.notification_duration)

# Helpers for specific settings
func is_timer_visible() -> bool:
	return get_setting("gameplay", "show_timer", true)

func is_music_enabled() -> bool:
	return get_setting("audio", "music_enabled", true)

func is_music_muted() -> bool:
	return get_setting("audio", "music_muted", false)

func is_show_completed() -> bool:
	return get_setting("gameplay", "show_completed", true)

func set_show_completed(visible: bool) -> void:
	set_setting("gameplay", "show_completed", visible)
	
func toggle_show_completed() -> bool:
	var current = is_show_completed()
	set_show_completed(!current)
	return !current

func toggle_music_muted() -> bool:
	var current = is_music_muted()
	set_music_muted(!current)
	return !current

func toggle_music_enabled() -> bool:
	var current = is_music_enabled()
	set_music_enabled(!current)
	return !current

func toggle_timer_visible() -> bool:
	var current = is_timer_visible()
	set_timer_visible(!current)
	return !current

func set_timer_visible(visible:bool):
	set_setting("gameplay","show_timer",visible)
	# Signal is already emitted in _apply_gameplay_settings, so we don't need to emit it here

func set_music_muted(muted:bool):
	set_setting("audio","music_muted",muted)
	# Signal is already emitted in _apply_audio_settings, so we don't need to emit it here

func set_music_enabled(enabled: bool) -> void:
	set_setting("audio", "music_enabled", enabled)
	# Signal is already emitted in _apply_audio_settings, so we don't need to emit it here

func is_clue_highlight_visible() -> bool:
	return get_setting("gameplay", "show_clue_highlight", true)

func is_grid_highlight_visible() -> bool:
	return get_setting("gameplay", "show_grid_highlight", true)

func set_clue_highlight_visible(visible:bool):
	set_setting("gameplay","show_clue_highlight",visible)
	# Signal is already emitted in _apply_gameplay_settings, so we don't need to emit it here

func set_grid_highlight_visible(visible:bool):
	set_setting("gameplay","show_grid_highlight",visible)
	# Signal is already emitted in _apply_gameplay_settings, so we don't need to emit it here

func toggle_clue_highlight_visible() -> bool:
	var current = is_clue_highlight_visible()
	set_clue_highlight_visible(!current)
	return !current
	
func toggle_grid_highlight_visible() -> bool:
	var current = is_grid_highlight_visible()
	set_grid_highlight_visible(!current)
	return !current

func is_cutscenes_hidden() -> bool:
	return get_setting("gameplay", "hide_cutscenes", false)

func set_hide_cutscenes(hidden: bool) -> void:
	set_setting("gameplay", "hide_cutscenes", hidden)
	# Signal is already emitted in _apply_gameplay_settings, so we don't need to emit it here

func toggle_hide_cutscenes() -> bool:
	var current = is_cutscenes_hidden()
	set_hide_cutscenes(!current)
	return !current

# Get notification duration
func get_notification_duration() -> float:
	return get_setting("gameplay", "notification_duration", 5.0)

# Set notification duration
func set_notification_duration(duration: float) -> void:
	set_setting("gameplay", "notification_duration", duration)
	# Signal is already emitted in _apply_gameplay_settings, so we don't need to emit it here

# Toggle fullscreen mode
func toggle_fullscreen() -> bool:
	var current = is_fullscreen()
	set_fullscreen(!current)
	return !current

# Check if fullscreen is enabled
func is_fullscreen() -> bool:
	return get_setting("video", "fullscreen", true)

# Set fullscreen mode
func set_fullscreen(enable: bool) -> void:
	set_setting("video", "fullscreen", enable)
	# Signal is already emitted in _apply_video_settings, so we don't need to emit it here
	
	# Apply immediately
	if enable:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
		print("Enabled fullscreen mode")
	else:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
		
		# Ensure correct resolution and center window
		DisplayServer.window_set_size(settings.video.resolution)
		var screen_size = DisplayServer.screen_get_size()
		var window_size = DisplayServer.window_get_size()
		var centered_pos = (screen_size - window_size) / 2
		DisplayServer.window_set_position(centered_pos)
		print("Disabled fullscreen mode")

# Set resolution
func set_resolution(width: int, height: int) -> void:
	set_setting("video", "resolution", Vector2i(width, height))
	
	# Apply immediately if in windowed mode
	if not is_fullscreen():
		DisplayServer.window_set_size(Vector2i(width, height))
		
		# Center window
		var screen_size = DisplayServer.screen_get_size()
		var window_size = DisplayServer.window_get_size()
		var centered_pos = (screen_size - window_size) / 2
		DisplayServer.window_set_position(centered_pos)

# For debugging
func print_settings() -> void:
	print("Current settings:")
	print(JSON.stringify(settings, "  "))
