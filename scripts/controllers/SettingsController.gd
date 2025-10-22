 
extends StateAwareController
class_name SettingsController

# UI References - Updated for correct structure
@onready var tab_container: TabContainer = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer
@onready var back_button: Button = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/ButtonMargin/ButtonContainer/BackButton

# Video settings
@onready var fullscreen_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Video/VideoSettings/SettingsMargin/SettingsContainer/FullscreenContainer/FullscreenCheck
@onready var vsync_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Video/VideoSettings/SettingsMargin/SettingsContainer/VsyncContainer/VsyncCheck
@onready var resolution_option: OptionButton = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Video/VideoSettings/SettingsMargin/SettingsContainer/ResolutionContainer/ResolutionOption

# Audio settings
@onready var music_enabled_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicEnabledContainer/MusicEnabledCheck
@onready var music_volume_slider: HSlider = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicVolumeContainer/MusicVolumeSlider
@onready var sfx_enabled_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/SfxEnabledContainer/SfxEnabledCheck
@onready var sfx_volume_slider: HSlider = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/SfxVolumeContainer/SfxVolumeSlider

# Music folder/track selection UI
@onready var music_folder_option: OptionButton = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicFolderContainer/MusicFolderOption
@onready var music_track_option: OptionButton = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicTrackContainer/MusicTrackOption
@onready var reload_music_button: Button = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicTrackContainer/ReloadMusicButton

var _settings_signal_connected := false

# Gameplay settings
@onready var show_timer_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowTimerContainer/ShowTimerCheck
@onready var show_completed_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowCompletedContainer/ShowCompletedCheck
@onready var show_clue_highlight_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowClueHighlightContainer/ShowClueHighlightCheck
@onready var show_grid_highlight_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowGridHighlightContainer/ShowGridHighlightCheck
@onready var hide_cutscenes_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/HideCutscenesContainer/HideCutscenesCheck
@onready var notification_duration_slider: HSlider = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/NotificationDurationContainer/NotificationDurationSlider
@onready var duration_value_label: Label = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/NotificationDurationContainer/LabelContainer/DurationValueLabel

# Gameplay notification toggles
@onready var show_welcome_notif_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowWelcomeNotifContainer/ShowWelcomeNotifCheck
@onready var show_music_notif_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowMusicNotifContainer/ShowMusicNotifCheck

# Resolution options
var resolutions = [
	Vector2i(1280, 720),
	Vector2i(1600, 900),
	Vector2i(1920, 1080)
]

func _ready() -> void:
	Logger.info(self, "Initializing...")
	show_in_states = [GameState.GameState.SETTINGS]
	super._ready()
	
	# Connect button signals
	if back_button:
		back_button.pressed.connect(_on_back_button_pressed)
	else:
		Logger.error(self, "BackButton node not found")
	
	# Connect video control signals
	if fullscreen_check:
		fullscreen_check.toggled.connect(_on_fullscreen_toggled)
	
	if vsync_check:
		vsync_check.toggled.connect(_on_vsync_toggled)
	
	if resolution_option:
		resolution_option.item_selected.connect(_on_resolution_selected)
	
	# Connect audio control signals
	if music_enabled_check:
		music_enabled_check.toggled.connect(_on_music_enabled_toggled)
	
	if music_volume_slider:
		music_volume_slider.value_changed.connect(_on_music_volume_changed)
		# Set max value to 100 since we're working with percentages
		music_volume_slider.max_value = 100
	
	if sfx_enabled_check:
		sfx_enabled_check.toggled.connect(_on_sfx_enabled_toggled)
	
	if sfx_volume_slider:
		sfx_volume_slider.value_changed.connect(_on_sfx_volume_changed)
		# Set max value to 100 since we're working with percentages
		sfx_volume_slider.max_value = 100

	# Connect music folder/track UI
	if music_folder_option:
		music_folder_option.item_selected.connect(_on_music_folder_selected)
	if music_track_option:
		music_track_option.item_selected.connect(_on_music_track_selected)
	if reload_music_button:
		reload_music_button.pressed.connect(_on_reload_music_pressed)
	
	# Connect gameplay control signals
	if show_timer_check:
		show_timer_check.toggled.connect(_on_show_timer_toggled)
	
	if show_completed_check:
		show_completed_check.toggled.connect(_on_show_completed_toggled)
	
	if show_clue_highlight_check:
		show_clue_highlight_check.toggled.connect(_on_show_clue_highlight_toggled)
	
	if show_grid_highlight_check:
		show_grid_highlight_check.toggled.connect(_on_show_grid_highlight_toggled)
	
	if hide_cutscenes_check:
		hide_cutscenes_check.toggled.connect(_on_hide_cutscenes_toggled)

	# Connect notification toggles
	if show_welcome_notif_check:
		show_welcome_notif_check.toggled.connect(_on_show_welcome_notif_toggled)
	if show_music_notif_check:
		show_music_notif_check.toggled.connect(_on_show_music_notif_toggled)
	
	# Connect notification duration slider
	if notification_duration_slider:
		notification_duration_slider.value_changed.connect(_on_notification_duration_changed)
	
	# Load current settings
	_load_settings()

	# Populate music folders and tracks after UI exists
	_populate_music_folders()
	_restore_music_selection()
	# If we have a saved folder/track, register folder and try to play saved track
	var saved_folder_on_open: String = SettingsManager.get_setting("audio", "music_folder", "")
	if not saved_folder_on_open.is_empty():
		_register_music_from_folder(saved_folder_on_open, false)
		var saved_track_on_open: String = SettingsManager.get_setting("audio", "music_track", "")
		if not saved_track_on_open.is_empty() and Audio_Manager:
			if Audio_Manager.play_and_set_persistent_music(saved_track_on_open) == false:
				# If not found, register again replacing cache and retry
				_register_music_from_folder(saved_folder_on_open, true)
				Audio_Manager.play_and_set_persistent_music(saved_track_on_open)
	
	Logger.info(self, "Ready")

	# Ensure we respond to SettingsManager play requests exactly once
	if not _settings_signal_connected and SettingsManager.has_signal("music_track_requested"):
		SettingsManager.music_track_requested.connect(_on_music_track_requested)
		_settings_signal_connected = true

func _load_settings() -> void:
	Logger.info(self, "Loading settings...")
	
	# Get settings manager
	var settings_manager = SettingsManager
	
	# Load video settings
	if fullscreen_check:
		fullscreen_check.button_pressed = settings_manager.get_setting("video", "fullscreen", true)
	
	if vsync_check:
		vsync_check.button_pressed = settings_manager.get_setting("video", "vsync", true)
	
	if resolution_option:
		var current_res = settings_manager.get_setting("video", "resolution", Vector2i(1920, 1080))
		for i in range(resolutions.size()):
			if resolutions[i] == current_res:
				resolution_option.select(i)
				break
	
	# Load audio settings
	if music_volume_slider:
		music_volume_slider.value = settings_manager.get_setting("audio", "music_volume", 0.8) * 100
	
	if sfx_volume_slider:
		sfx_volume_slider.value = settings_manager.get_setting("audio", "sfx_volume", 0.8) * 100
	
	if music_enabled_check:
		music_enabled_check.button_pressed = settings_manager.get_setting("audio", "music_enabled", true)
	
	if sfx_enabled_check:
		sfx_enabled_check.button_pressed = settings_manager.get_setting("audio", "sfx_enabled", true)

	# Set initial selections if available
	if music_folder_option:
		var saved_folder: String = settings_manager.get_setting("audio", "music_folder", "")
		if not saved_folder.is_empty():
			_select_option_by_text(music_folder_option, saved_folder)

	if music_track_option:
		var saved_track: String = settings_manager.get_setting("audio", "music_track", "")
		if not saved_track.is_empty():
			_select_option_by_text(music_track_option, saved_track)
	
	# Load gameplay settings
	if show_timer_check:
		show_timer_check.button_pressed = settings_manager.get_setting("gameplay", "show_timer", true)
	
	if show_completed_check:
		show_completed_check.button_pressed = settings_manager.get_setting("gameplay", "show_completed", true)
	
	if show_clue_highlight_check:
		show_clue_highlight_check.button_pressed = settings_manager.get_setting("gameplay", "show_clue_highlight", true)
	
	if show_grid_highlight_check:
		show_grid_highlight_check.button_pressed = settings_manager.get_setting("gameplay", "show_grid_highlight", true)
	
	if hide_cutscenes_check:
		hide_cutscenes_check.button_pressed = settings_manager.is_cutscenes_hidden()

	# Load notification toggles
	if show_welcome_notif_check:
		show_welcome_notif_check.button_pressed = settings_manager.get_setting("gameplay", "show_welcome_notification", true)
	if show_music_notif_check:
		show_music_notif_check.button_pressed = settings_manager.get_setting("gameplay", "show_music_notifications", true)
	
	# Load notification duration
	if notification_duration_slider and duration_value_label:
		var duration = settings_manager.get_setting("gameplay", "notification_duration", 10.0)
		notification_duration_slider.value = duration
		duration_value_label.text = str(duration) + "s"

# Video control handlers
func _on_fullscreen_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Fullscreen toggled: %s" % toggled_on)
	SettingsManager.set_setting("video", "fullscreen", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_vsync_toggled(toggled_on: bool) -> void:
	Logger.info(self, "VSync toggled: %s" % toggled_on)
	SettingsManager.set_setting("video", "vsync", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_resolution_selected(index: int) -> void:
	if index >= 0 and index < resolutions.size():
		Logger.info(self, "Resolution selected: %s" % resolutions[index])
		SettingsManager.set_setting("video", "resolution", resolutions[index])
		SettingsManager.apply_settings()
		SettingsManager.save_settings()

# Audio control handlers
func _on_music_enabled_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Music enabled toggled: %s" % toggled_on)
	SettingsManager.set_setting("audio", "music_enabled", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_sfx_enabled_toggled(toggled_on: bool) -> void:
	Logger.info(self, "SFX enabled toggled: %s" % toggled_on)
	SettingsManager.set_setting("audio", "sfx_enabled", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_music_volume_changed(value: float) -> void:
	Logger.info(self, "Music volume changed: %s" % value)
	# Convert from percentage (0-100) to float (0.0-1.0)
	SettingsManager.set_setting("audio", "music_volume", value / 100.0)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_sfx_volume_changed(value: float) -> void:
	Logger.info(self, "SFX volume changed: %s" % value)
	# Convert from percentage (0-100) to float (0.0-1.0)
	SettingsManager.set_setting("audio", "sfx_volume", value / 100.0)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

# Gameplay control handlers
func _on_show_timer_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Show timer toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_timer", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_show_completed_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Show completed toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_completed", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_show_clue_highlight_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Show clue highlight toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_clue_highlight", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_show_grid_highlight_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Show grid highlight toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_grid_highlight", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_hide_cutscenes_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Hide cutscenes toggled: %s" % toggled_on)
	SettingsManager.set_hide_cutscenes(toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_notification_duration_changed(value: float) -> void:
	Logger.info(self, "Notification duration changed: %s" % value)
	SettingsManager.set_setting("gameplay", "notification_duration", value)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()
	# Update the label
	if duration_value_label:
		duration_value_label.text = str(value) + "s"

func _on_show_welcome_notif_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Show welcome notification toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_welcome_notification", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_show_music_notif_toggled(toggled_on: bool) -> void:
	Logger.info(self, "Show music notifications toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_music_notifications", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_back_button_pressed() -> void:
	Logger.info(self, "Back button pressed")
	
	# Return to the previous state instead of always going to MAIN_MENU
	State_Manager.return_to_previous_state()
	
	# Play click sound if available
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func on_enter_state(_state: int) -> void:
	Logger.info(self, "Entering state")
	_load_settings()
	visible = true

func on_exit_state(_state: int) -> void:
	Logger.info(self, "Exiting state")
	visible = false

# Music folder/track handlers and helpers
const MUSIC_ROOT := "res://assets/audio/music/"
const AUDIO_EXTS := ["mp3", "ogg", "wav"]

func _on_music_folder_selected(index: int) -> void:
	var folder = music_folder_option.get_item_text(index)
	Logger.info(self, "Music folder selected: %s" % folder)
	SettingsManager.set_setting("audio", "music_folder", folder)
	_populate_music_tracks(folder)
	# Proactively register this folder so tracks are available immediately
	_register_music_from_folder(folder, true)
	var saved_track: String = SettingsManager.get_setting("audio", "music_track", "")
	if not saved_track.is_empty():
		_select_option_by_text(music_track_option, saved_track)

func _on_music_track_selected(index: int) -> void:
	var track = music_track_option.get_item_text(index)
	Logger.info(self, "Music track selected: %s" % track)
	SettingsManager.set_setting("audio", "music_track", track)
	if Audio_Manager:
		if Audio_Manager.play_and_set_persistent_music(track) == false:
			# Auto-register current folder then retry
			var folder: String = SettingsManager.get_setting("audio", "music_folder", "")
			if not folder.is_empty():
				_register_music_from_folder(folder, true)
				if Audio_Manager.play_and_set_persistent_music(track) == false:
					Logger.warn(self, "Track still not registered after reload: %s" % track)

func _on_reload_music_pressed() -> void:
	Logger.info(self, "Reloading music resources...")
	var folder = SettingsManager.get_setting("audio", "music_folder", "")
	if folder.is_empty():
		Logger.warn(self, "No music folder selected; nothing to reload")
		return
	_register_music_from_folder(folder, true)
	var track = SettingsManager.get_setting("audio", "music_track", "")
	if not track.is_empty():
		Audio_Manager.play_and_set_persistent_music(track)

func _on_music_track_requested(track_name: String) -> void:
	Logger.info(self, "SettingsManager requested music track: %s" % track_name)
	if Audio_Manager:
		if Audio_Manager.play_and_set_persistent_music(track_name) == false:
			var folder: String = SettingsManager.get_setting("audio", "music_folder", "")
			if not folder.is_empty():
				_register_music_from_folder(folder, true)
				Audio_Manager.play_and_set_persistent_music(track_name)
	_select_option_by_text(music_track_option, track_name)

func _populate_music_folders() -> void:
	music_folder_option.clear()
	var dirs := DirAccess.get_directories_at(MUSIC_ROOT)
	for d in dirs:
		music_folder_option.add_item(d)

func _populate_music_tracks(folder: String) -> void:
	music_track_option.clear()
	var full_path := MUSIC_ROOT.path_join(folder)
	var files := DirAccess.get_files_at(full_path)
	for f in files:
		var ext := f.get_extension().to_lower()
		if AUDIO_EXTS.has(ext):
			var track_label := f.get_basename()
			music_track_option.add_item(track_label)

func _restore_music_selection() -> void:
	var saved_folder: String = SettingsManager.get_setting("audio", "music_folder", "")
	if not saved_folder.is_empty():
		_select_option_by_text(music_folder_option, saved_folder)
		_populate_music_tracks(saved_folder)
	var saved_track: String = SettingsManager.get_setting("audio", "music_track", "")
	if not saved_track.is_empty():
		_select_option_by_text(music_track_option, saved_track)

func _select_option_by_text(ob: OptionButton, text: String) -> void:
		for i in range(ob.item_count):
			if ob.get_item_text(i) == text:
				ob.select(i)
				return

func _register_music_from_folder(folder: String, replace_cache: bool = true) -> void:
		var full_path := MUSIC_ROOT.path_join(folder)
		var files := DirAccess.get_files_at(full_path)
		for f in files:
			var ext := f.get_extension().to_lower()
			if not AUDIO_EXTS.has(ext):
				continue
			var res_path := full_path.path_join(f)
			var cache_mode
			if replace_cache:
				cache_mode = ResourceLoader.CACHE_MODE_REPLACE
			else:
				cache_mode = ResourceLoader.CACHE_MODE_IGNORE
			var stream: Resource = ResourceLoader.load(res_path, "", cache_mode)
			if not stream:
				Logger.warn(self, "Failed to load stream: %s" % res_path)
				continue
			var track_name := f.get_basename()
			Audio_Manager.register_music(res_path, track_name, true, false)
			Logger.info(self, "Registered track: %s from %s" % [track_name, res_path])
