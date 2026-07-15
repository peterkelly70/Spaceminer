 
extends StateAwareController
class_name SettingsController

# UI References - Updated for correct structure
@onready var tab_container: TabContainer = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer") as TabContainer
@onready var back_button: Button = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/ButtonMargin/ButtonContainer/BackButton") as Button

# Video settings
@onready var fullscreen_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Video/VideoSettings/SettingsMargin/SettingsContainer/FullscreenContainer/FullscreenCheck") as CheckBox
@onready var vsync_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Video/VideoSettings/SettingsMargin/SettingsContainer/VsyncContainer/VsyncCheck") as CheckBox
@onready var resolution_option: OptionButton = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Video/VideoSettings/SettingsMargin/SettingsContainer/ResolutionContainer/ResolutionOption") as OptionButton

# Audio settings
@onready var music_enabled_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicEnabledContainer/MusicEnabledCheck") as CheckBox
@onready var music_volume_slider: HSlider = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicVolumeContainer/MusicVolumeSlider") as HSlider
@onready var sfx_enabled_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/SfxEnabledContainer/SfxEnabledCheck") as CheckBox
@onready var sfx_volume_slider: HSlider = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/SfxVolumeContainer/SfxVolumeSlider") as HSlider

# Music folder/track selection UI
@onready var music_folder_option: OptionButton = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicFolderContainer/MusicFolderOption") as OptionButton
@onready var music_track_option: OptionButton = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicTrackContainer/MusicTrackOption") as OptionButton
@onready var reload_music_button: Button = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/MusicTrackContainer/ReloadMusicButton") as Button
@onready var speech_test_input: LineEdit = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/SpeechTestContainer/SpeechRow/SpeechInput") as LineEdit
@onready var speech_test_button: Button = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/SpeechTestContainer/SpeechRow/SpeechSpeakButton") as Button
@onready var speech_test_output: Label = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Audio/AudioSettings/SettingsMargin/SettingsContainer/SpeechTestContainer/SpeechOutput") as Label
@onready var speech_player: AudioStreamPlayer = get_node_or_null("SpeechPlayer") as AudioStreamPlayer

var _settings_signal_connected := false
var _speech := RetroSpeech.new()

# Gameplay settings
@onready var show_timer_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowTimerContainer/ShowTimerCheck") as CheckBox
@onready var show_completed_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowCompletedContainer/ShowCompletedCheck") as CheckBox
@onready var show_clue_highlight_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowClueHighlightContainer/ShowClueHighlightCheck") as CheckBox
@onready var show_grid_highlight_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowGridHighlightContainer/ShowGridHighlightCheck") as CheckBox
@onready var hide_cutscenes_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/HideCutscenesContainer/HideCutscenesCheck") as CheckBox
@onready var notification_duration_slider: HSlider = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/NotificationDurationContainer/NotificationDurationSlider") as HSlider
@onready var duration_value_label: Label = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/NotificationDurationContainer/LabelContainer/DurationValueLabel") as Label

# Gameplay notification toggles
@onready var show_welcome_notif_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowWelcomeNotifContainer/ShowWelcomeNotifCheck") as CheckBox
@onready var show_music_notif_check: CheckBox = get_node_or_null("MainContainer/CenterContainer/VBoxContainer/TabContainer/Gameplay/GameplaySettings/SettingsMargin/SettingsContainer/ShowMusicNotifContainer/ShowMusicNotifCheck") as CheckBox

# Resolution options
var resolutions = [
	Vector2i(1280, 720),
	Vector2i(1600, 900),
	Vector2i(1920, 1080)
]

func _ready() -> void:
	print("Initializing...")
	show_in_states = [AppState.State.SETTINGS]
	super._ready()
	
	# Connect button signals
	if back_button:
		back_button.pressed.connect(_on_back_button_pressed)
	else:
		push_error("BackButton node not found")
	
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
	if speech_test_button:
		speech_test_button.pressed.connect(_on_speech_test_pressed)
	if speech_test_input:
		speech_test_input.text_submitted.connect(_on_speech_test_input_submitted)
	print("Audio tab wiring: speech_button=%s speech_input=%s speech_output=%s speech_player=%s" % [
		str(speech_test_button != null),
		str(speech_test_input != null),
		str(speech_test_output != null),
		str(speech_player != null)
	])
	
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
	# Default to 8bit if nothing saved yet
	var default_folder: String = SettingsManager.get_setting("audio", "music_folder", "8bit")
	if default_folder.is_empty():
		default_folder = "8bit"
	SettingsManager.set_setting("audio", "music_folder", default_folder)
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
	
	print("Ready")

	# Ensure we respond to SettingsManager play requests exactly once
	if not _settings_signal_connected and SettingsManager.has_signal("music_track_requested"):
		SettingsManager.music_track_requested.connect(_on_music_track_requested)
		_settings_signal_connected = true

func _load_settings() -> void:
	print("Loading settings...")
	
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

	if speech_test_input and speech_test_input.text.is_empty():
		speech_test_input.text = "BEWARE THE FOREST"
	if speech_test_output:
		speech_test_output.text = "Dirty SAM-style speech test ready."
	if speech_player:
		speech_player.bus = "Master"

# Video control handlers
func _on_fullscreen_toggled(toggled_on: bool) -> void:
	print("Fullscreen toggled: %s" % toggled_on)
	SettingsManager.set_setting("video", "fullscreen", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_vsync_toggled(toggled_on: bool) -> void:
	print("VSync toggled: %s" % toggled_on)
	SettingsManager.set_setting("video", "vsync", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_resolution_selected(index: int) -> void:
	if index >= 0 and index < resolutions.size():
		print("Resolution selected: %s" % resolutions[index])
		SettingsManager.set_setting("video", "resolution", resolutions[index])
		SettingsManager.apply_settings()
		SettingsManager.save_settings()

# Audio control handlers
func _on_music_enabled_toggled(toggled_on: bool) -> void:
	print("Music enabled toggled: %s" % toggled_on)
	SettingsManager.set_music_enabled(toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_sfx_enabled_toggled(toggled_on: bool) -> void:
	print("SFX enabled toggled: %s" % toggled_on)
	SettingsManager.set_setting("audio", "sfx_enabled", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_music_volume_changed(value: float) -> void:
	print("Music volume changed: %s" % value)
	# Convert from percentage (0-100) to float (0.0-1.0)
	SettingsManager.set_setting("audio", "music_volume", value / 100.0)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_sfx_volume_changed(value: float) -> void:
	print("SFX volume changed: %s" % value)
	# Convert from percentage (0-100) to float (0.0-1.0)
	SettingsManager.set_setting("audio", "sfx_volume", value / 100.0)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

# Gameplay control handlers
func _on_show_timer_toggled(toggled_on: bool) -> void:
	print("Show timer toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_timer", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_show_completed_toggled(toggled_on: bool) -> void:
	print("Show completed toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_completed", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_show_clue_highlight_toggled(toggled_on: bool) -> void:
	print("Show clue highlight toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_clue_highlight", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_show_grid_highlight_toggled(toggled_on: bool) -> void:
	print("Show grid highlight toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_grid_highlight", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_hide_cutscenes_toggled(toggled_on: bool) -> void:
	print("Hide cutscenes toggled: %s" % toggled_on)
	SettingsManager.set_hide_cutscenes(toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_notification_duration_changed(value: float) -> void:
	print("Notification duration changed: %s" % value)
	SettingsManager.set_setting("gameplay", "notification_duration", value)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()
	# Update the label
	if duration_value_label:
		duration_value_label.text = str(value) + "s"

func _on_show_welcome_notif_toggled(toggled_on: bool) -> void:
	print("Show welcome notification toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_welcome_notification", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_show_music_notif_toggled(toggled_on: bool) -> void:
	print("Show music notifications toggled: %s" % toggled_on)
	SettingsManager.set_setting("gameplay", "show_music_notifications", toggled_on)
	SettingsManager.apply_settings()
	SettingsManager.save_settings()

func _on_back_button_pressed() -> void:
	print("Back button pressed")
	
	# Return to the previous state instead of always going to MAIN_MENU
	State_Manager.return_to_previous_state()
	
	# Play click sound if available
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func on_enter_state(_state: int) -> void:
	print("Entering state")
	_load_settings()
	if Audio_Manager and Audio_Manager.has_method("ensure_music_playing"):
		Audio_Manager.ensure_music_playing()
	visible = true

func on_exit_state(_state: int) -> void:
	print("Exiting state")
	visible = false

func _unhandled_input(event: InputEvent) -> void:
	if not visible:
		return
	if event.is_action_pressed("ui_cancel"):
		State_Manager.change_state(AppState.State.MAIN_MENU)
		get_viewport().set_input_as_handled()

# Music folder/track handlers and helpers
const MUSIC_ROOT := "res://assets/audio/music/"
const AUDIO_EXTS := ["mp3", "ogg", "wav"]

func _on_music_folder_selected(index: int) -> void:
	var folder = music_folder_option.get_item_text(index)
	print("Music folder selected: %s" % folder)
	SettingsManager.set_setting("audio", "music_folder", folder)
	_populate_music_tracks(folder)
	# Proactively register this folder so tracks are available immediately
	_register_music_from_folder(folder, true)
	var saved_track: String = SettingsManager.get_setting("audio", "music_track", "")
	if not saved_track.is_empty():
		_select_option_by_text(music_track_option, saved_track)

func _on_music_track_selected(index: int) -> void:
	var track = music_track_option.get_item_text(index)
	print("Music track selected: %s" % track)
	SettingsManager.set_setting("audio", "music_track", track)
	if Audio_Manager:
		if Audio_Manager.play_and_set_persistent_music(track) == false:
			# Auto-register current folder then retry
			var folder: String = SettingsManager.get_setting("audio", "music_folder", "")
			if not folder.is_empty():
				_register_music_from_folder(folder, true)
				if Audio_Manager.play_and_set_persistent_music(track) == false:
					push_warning("Track still not registered after reload: %s" % track)

func _on_reload_music_pressed() -> void:
	print("Reloading music resources...")
	var folder = SettingsManager.get_setting("audio", "music_folder", "")
	if folder.is_empty():
		push_warning("No music folder selected; nothing to reload")
		return
	_register_music_from_folder(folder, true)
	var track = SettingsManager.get_setting("audio", "music_track", "")
	if not track.is_empty():
		Audio_Manager.play_and_set_persistent_music(track)

func _on_speech_test_input_submitted(_text: String) -> void:
	_on_speech_test_pressed()

func _on_speech_test_pressed() -> void:
	print("Speech synth test button pressed")
	if not speech_test_input:
		print("Speech synth test aborted: missing input line edit")
		return
	var text := speech_test_input.text.strip_edges()
	if text.is_empty():
		text = "BEWARE THE FOREST"
	var voice := _build_dirty_speech_voice()
	var phonemes := _speech.text_to_phoneme_string(text)
	print("Speech synth test text: '%s'" % text)
	print("Speech synth phonemes: %s" % phonemes)
	if speech_test_output:
		speech_test_output.text = "Phonemes: %s" % phonemes
	if not speech_player:
		push_warning("SpeechPlayer node missing; cannot play speech test")
		return
	var wav := _speech.render_to_stream(text, voice)
	if wav == null:
		print("Speech synth render failed: returned null AudioStreamWAV")
		if speech_test_output:
			speech_test_output.text += "\n[ERROR: speech render failed]"
		return
	print("Speech synth rendered stream: %s" % wav)
	if wav.has_method("get_length"):
		print("Speech synth stream length: %s" % wav.get_length())
	if speech_player:
		speech_player.bus = "Master"
		speech_player.volume_db = 0.0
		speech_player.stream_paused = false
		speech_player.stream = wav
		speech_player.play()
		print("Speech synth playback started on %s" % speech_player.bus)
	elif Audio_Manager and Audio_Manager.has_method("play_stream"):
		Audio_Manager.play_stream(wav, 1.0)
	else:
		push_warning("No audio player available for speech test")

func _build_dirty_speech_voice() -> SamVoice:
	var voice := RetroSpeech.VOICE_OLD_COMPUTER.duplicate_profile()
	voice.sample_rate = 22050
	voice.pitch = 64
	voice.speed = 0.82
	voice.throat = 128
	voice.mouth = 124
	voice.crunch = 0.0
	voice.crunch_bits = 8
	voice.volume = 1.0
	return voice

func _on_music_track_requested(track_name: String) -> void:
	print("SettingsManager requested music track: %s" % track_name)
	if Audio_Manager:
		if Audio_Manager.play_and_set_persistent_music(track_name) == false:
			var folder: String = SettingsManager.get_setting("audio", "music_folder", "")
			if not folder.is_empty():
				_register_music_from_folder(folder, true)
				Audio_Manager.play_and_set_persistent_music(track_name)
	_select_option_by_text(music_track_option, track_name)

func _populate_music_folders() -> void:
	if not music_folder_option:
		return
	music_folder_option.clear()
	var dirs := DirAccess.get_directories_at(MUSIC_ROOT)
	for d in dirs:
		music_folder_option.add_item(d)

func _populate_music_tracks(folder: String) -> void:
	if not music_track_option:
		return
	music_track_option.clear()
	var tracks := _load_catalog_tracks(folder)
	if not tracks.is_empty():
		for track in tracks:
			music_track_option.add_item(str(track["name"]))
		return
	# Fallback: scan directory
	var full_path := MUSIC_ROOT.path_join(folder)
	var files := DirAccess.get_files_at(full_path)
	for f in files:
		var ext := f.get_extension().to_lower()
		if AUDIO_EXTS.has(ext):
			music_track_option.add_item(f.get_basename())

func _restore_music_selection() -> void:
	var saved_folder: String = SettingsManager.get_setting("audio", "music_folder", "")
	if not saved_folder.is_empty():
		_select_option_by_text(music_folder_option, saved_folder)
		_populate_music_tracks(saved_folder)
	var saved_track: String = SettingsManager.get_setting("audio", "music_track", "")
	if not saved_track.is_empty():
		_select_option_by_text(music_track_option, saved_track)

func _select_option_by_text(ob: OptionButton, text: String) -> void:
	if not ob:
		return
	for i in range(ob.item_count):
		if ob.get_item_text(i) == text:
			ob.select(i)
			return

func _load_catalog_tracks(folder: String) -> Array:
	var catalog_path := MUSIC_ROOT.path_join(folder).path_join("music_catalog.gd")
	if not ResourceLoader.exists(catalog_path):
		return []
	var script: GDScript = load(catalog_path)
	if not script:
		return []
	var constants: Dictionary = script.get_script_constant_map()
	if constants.has("MUSIC_TRACKS"):
		return constants["MUSIC_TRACKS"]
	return []

func _register_music_from_folder(folder: String, replace_cache: bool = true) -> void:
	var tracks := _load_catalog_tracks(folder)
	if not tracks.is_empty():
		for track in tracks:
			Audio_Manager.register_music(str(track["path"]), str(track["name"]), true, bool(track.get("persistent", false)))
		return
	# Fallback: scan directory
	var full_path := MUSIC_ROOT.path_join(folder)
	var files := DirAccess.get_files_at(full_path)
	for f in files:
		var ext := f.get_extension().to_lower()
		if not AUDIO_EXTS.has(ext):
			continue
		Audio_Manager.register_music(full_path.path_join(f), f.get_basename(), true, false)
