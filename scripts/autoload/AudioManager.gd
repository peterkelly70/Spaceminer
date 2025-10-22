extends Node

var log_level: int = 1

# Audio players
var music_player: AudioStreamPlayer
var voiceover_player: AudioStreamPlayer
var sfx_players: Array = []
const SFX_PLAYER_COUNT: int = 8

# Default audio tracks
var music_tracks: Dictionary = {}  # Will load dynamically to avoid missing file errors
# Map track_name -> original file path to enrich notifications and attribution
var music_track_paths: Dictionary = {}

# Configuration for looping
var music_config: Dictionary = {
	"persistent": true,
	"menu": true,
	"gameplay": true,
	"victory": false
}

# Sound effects
var sound_effects: Dictionary = {}  # Will load dynamically to avoid missing file errors

# Settings
var music_volume: float = 0.8
var sfx_volume: float = 1.0
var music_muted: bool = false
var current_track: String = ""
var persistent_track: String = "persistent"
var using_scene_music: bool = false

# Voiceover queue for chained playback
var voiceover_queue: Array = []
var on_voiceover_finished: Callable = Callable()

## Play a sequence of voiceover files in order. Only initializes playback once.
## Usage: play_voiceover_sequence(["res://voice1.ogg", "res://voice2.ogg"], my_callback)
func play_voiceover_sequence(paths: Array, finished_callback: Callable = Callable()) -> void:
	voiceover_queue = paths.duplicate()
	on_voiceover_finished = finished_callback
	_play_next_voiceover_in_queue()

func _play_next_voiceover_in_queue():
	if voiceover_player.playing:
		voiceover_player.stop()
	if voiceover_queue.size() == 0:
		if on_voiceover_finished.is_valid():
			on_voiceover_finished.call()
		return
	var next_path = voiceover_queue.pop_front()
	var stream = load(next_path)
	if not stream:
		Logger.warn(self, "Could not load voiceover: %s" % next_path)
		_play_next_voiceover_in_queue() # skip to next
		return
	voiceover_player.stream = stream
	voiceover_player.finished.connect(_on_voiceover_finished, CONNECT_ONE_SHOT)
	voiceover_player.volume_db = linear_to_db(sfx_volume)
	voiceover_player.play()
	Logger.info(self, "Playing voiceover: %s" % next_path)

func _on_voiceover_finished():
	_play_next_voiceover_in_queue()

## Play a voiceover file, always using the dedicated player
func play_voiceover(file_path: String) -> void:
	play_voiceover_sequence([file_path])

## Stop the dedicated voiceover player and clear the queue
func stop_voiceover() -> void:
	voiceover_queue.clear()
	if voiceover_player.playing:
		voiceover_player.stop()

## Push a new voiceover file to the queue. If nothing is playing, start immediately.
func voiceover_queue_push(path: String) -> void:
	if voiceover_queue.is_empty() and not voiceover_player.playing:
		play_voiceover_sequence([path])
	else:
		voiceover_queue.append(path)

## Clear the voiceover queue and stop playback immediately.
func voiceover_queue_clear() -> void:
	voiceover_queue.clear()
	if voiceover_player.playing:
		voiceover_player.stop()

## Always start music on cutscene load if not already playing
func ensure_music_playing():
	if not music_player.playing:
		play_persistent_music()

# Persistence
func _save_settings() -> void:
	var cfg = ConfigFile.new()
	cfg.set_value("audio", "music_volume", music_volume)
	cfg.set_value("audio", "sfx_volume", sfx_volume)
	cfg.set_value("audio", "music_muted", music_muted)
	cfg.save("user://audio_settings.cfg")
	Logger.info(self, "Saved audio settings")

func _load_settings() -> Dictionary:
	var cfg = ConfigFile.new()
	var settings = {"music_volume": 0.8, "sfx_volume": 1.0, "music_muted": false}
	
	if cfg.load("user://audio_settings.cfg") == OK:
		settings["music_volume"] = cfg.get_value("audio", "music_volume", settings["music_volume"])
		settings["sfx_volume"] = cfg.get_value("audio", "sfx_volume", settings["sfx_volume"])
		settings["music_muted"] = cfg.get_value("audio", "music_muted", settings["music_muted"])
		Logger.info(self, "Loaded audio settings")
	else:
		Logger.info(self, "No saved audio settings found, using defaults")
	
	return settings

func _apply_settings() -> void:
	var s = _load_settings()
	music_volume = s.music_volume
	sfx_volume = s.sfx_volume
	music_muted = s.music_muted
	
	# Apply immediately
	set_music_volume(music_volume)
	set_sfx_volume(sfx_volume)
	set_mute(music_muted)
	
	# Save back
	_save_settings()
	Logger.info(self, "Applied audio settings")

# Button connection
func _setup_buttons() -> void:
	# Wait until next frame to ensure all buttons are in the scene
	await get_tree().process_frame
	
	var buttons = get_tree().get_nodes_in_group("Button")
	for btn in buttons:
		register_button(btn)
	
	Logger.info(self, "Connected to %d buttons from Button group" % buttons.size())

# Register a button to play sounds when pressed, released, hovered, and focused
func register_button(button: BaseButton, press_sound: String = "click", release_sound: String = "button_release", hover_sound: String = "button_hover", focus_sound: String = "button_focus") -> void:
	if button:
		# Set press sound (when button is pushed down)
		button.set_meta("press_sound", press_sound)
		button.button_down.connect(_on_button_down.bind(button))
		
		# Set release sound (when button is released)
		button.set_meta("release_sound", release_sound)
		button.button_up.connect(_on_button_up.bind(button))
		
		# Set hover sound (when mouse first enters button)
		button.set_meta("hover_sound", hover_sound)
		button.mouse_entered.connect(_on_button_hover.bind(button))
		
		# Set focus sound (when button gains focus)
		button.set_meta("focus_sound", focus_sound)
		button.focus_entered.connect(_on_button_focus.bind(button))
		
		Logger.info(self, "Registered button with sounds: press=%s, release=%s, hover=%s, focus=%s" % [press_sound, release_sound, hover_sound, focus_sound])

# Register all buttons in a container recursively
func register_buttons_in_container(container: Node, press_sound: String = "click", release_sound: String = "button_release", hover_sound: String = "button_hover") -> void:
	for child in container.get_children():
		if child is BaseButton:
			register_button(child, press_sound, release_sound, hover_sound)
		
		# Recursively check children
		if child.get_child_count() > 0:
			register_buttons_in_container(child, press_sound, release_sound, hover_sound)
	
	Logger.info(self, "Registered all buttons in container: %s" % container.name)

# Handle button press (button down) events
func _on_button_down(button: BaseButton = null) -> void:
	var sound = "click"
	
	# If button is provided, check for custom press sound
	if button and button.has_meta("press_sound"):
		sound = button.get_meta("press_sound")
	
	# Play the press sound (stone_btn_in.mp3)
	play_sfx(sound)

# Handle button release (button up) events
func _on_button_up(button: BaseButton = null) -> void:
	var sound = "button_hover"
	
	# If button is provided, check for custom release sound
	if button and button.has_meta("release_sound"):
		sound = button.get_meta("release_sound")
	
	# Play the release sound (stone_btn_out.mp3)
	play_sfx(sound, 0.8)

# Handle button hover events
func _on_button_hover(button: BaseButton = null) -> void:
	var sound = "button_hover"
	
	# If button is provided, check for custom hover sound
	if button and button.has_meta("hover_sound"):
		sound = button.get_meta("hover_sound")
	
	# Play the hover sound at a lower volume
	play_sfx(sound, 0.5)

# Handle button focus events
func _on_button_focus(button: BaseButton = null) -> void:
	var sound = "button_focus"
	
	# If button is provided, check for custom focus sound
	if button and button.has_meta("focus_sound"):
		sound = button.get_meta("focus_sound")
	
	# Play the focus sound at a moderate volume
	play_sfx(sound, 0.7)

func _ready() -> void:
	Logger.info(self, "Initializing...")
	
	# Create audio buses first
	_ensure_audio_buses_exist()
	
	# Create music player
	music_player = AudioStreamPlayer.new()
	music_player.bus = "Music"
	add_child(music_player)
	
	# Create dedicated voiceover player
	voiceover_player = AudioStreamPlayer.new()
	voiceover_player.bus = "SFX"
	add_child(voiceover_player)
	
	# Create SFX players
	for i in range(SFX_PLAYER_COUNT):
		var player = AudioStreamPlayer.new()
		player.bus = "SFX"
		sfx_players.append(player)
		add_child(player)
	
	# Music tracks will be loaded by MusicManager or other systems
	# AudioManager only handles the core audio playback functionality
	
	# Try to load default sound effects if they exist
	var sfx_paths = {
		"click": "res://assets/audio/sfx/ui/button.mp3",
		# Remapped to existing asset to ensure immediate availability
		"button_hover": "res://assets/audio/sfx/ui/button.mp3",
		"button_release": "res://assets/audio/sfx/ui/stone_btn_out.mp3",
		# Remapped broken path to an existing asset
		"button_focus": "res://assets/audio/sfx/ui/notification.mp3",
		"build": "res://assets/audio/sfx/ui/build.mp3",
		"error": "res://assets/audio/sfx/ui/error.mp3",
		"success": "res://assets/audio/sfx/ui/success.mp3",
		"notification": "res://assets/audio/sfx/ui/notification.mp3"
	}
	
	var loaded_sfx_count = 0
	for sfx_name in sfx_paths:
		var file = FileAccess.open(sfx_paths[sfx_name], FileAccess.READ)
		if file:
			load_sfx(sfx_name, sfx_paths[sfx_name])
			file.close()
			loaded_sfx_count += 1
		else:
			Logger.warn(self, "SFX file not found: %s" % sfx_paths[sfx_name])
	
	if loaded_sfx_count == 0:
		Logger.warn(self, "No UI sound effects found - buttons will be silent")
	else:
		Logger.info(self, "Loaded %d UI sound effects" % loaded_sfx_count)
	
	# Load settings
	_apply_settings()

	# Music will be started by MusicManager or other systems
	# AudioManager is now ready for music playback requests

	Logger.info(self, "Initialized")

	# After the first frame, register all buttons in the scene tree for SFX
	await get_tree().process_frame
	register_buttons_in_container(get_tree().root)

func _ensure_audio_buses_exist() -> void:
	Logger.info(self, "Setting up audio buses...")
	
	# Master bus always exists at index 0
	if AudioServer.get_bus_index("Master") < 0:
		Logger.error(self, "Master bus missing - this should never happen!")
		return
	
	# Add Music bus if it doesn't exist
	if AudioServer.get_bus_index("Music") < 0:
		var idx = AudioServer.bus_count
		AudioServer.add_bus()
		AudioServer.set_bus_name(idx, "Music")
		AudioServer.set_bus_send(idx, "Master")
		Logger.info(self, "Created Music bus")
	
	# Add SFX bus if it doesn't exist
	if AudioServer.get_bus_index("SFX") < 0:
		var idx2 = AudioServer.bus_count
		AudioServer.add_bus()
		AudioServer.set_bus_name(idx2, "SFX")
		AudioServer.set_bus_send(idx2, "Master")
		Logger.info(self, "Created SFX bus")

func play_music(track_name: String) -> void:
	if not music_tracks.has(track_name):
		Logger.warn(self, "Cannot play music track '%s' - not loaded" % track_name)
		return
	
	Logger.info(self, "Attempting to play music track: %s" % track_name)
	
	# Show notification for the new track if it's different from the current one
	if track_name != current_track:
		var notification_manager = get_node_or_null("/root/Notification_Manager")
		if notification_manager and notification_manager.has_method("show_music_notification"):
			var track_path := ""
			if music_track_paths.has(track_name):
				track_path = music_track_paths[track_name]
			else:
				# Fallback to using the name as a path-like value; NotificationManager will degrade gracefully
				track_path = track_name
			notification_manager.show_music_notification(track_path)
	Logger.debug(self, "Current mute state: %s" % music_muted)
	
	if current_track == track_name and music_player.playing:
		Logger.debug(self, "Already playing this track")
		return
	
	using_scene_music = track_name != persistent_track
	current_track = track_name
	
	# Set the stream and configure
	music_player.stream = music_tracks[track_name]
	
	# Set looping if applicable
	if music_player.stream is AudioStreamMP3:
		music_player.stream.loop = music_config.get(track_name, true)
	
	# Set volume based on mute state
	if is_muted():
		music_player.volume_db = linear_to_db(0)
		Logger.debug(self, "Setting volume to 0 (muted)")
	else:
		music_player.volume_db = linear_to_db(music_volume)
		Logger.debug(self, "Setting volume to %s" % music_volume)
	
	# Play the track
	music_player.play()
	Logger.info(self, "Started playing music track: %s - player.playing=%s" % [track_name, music_player.playing])

func play_scene_music(scene_name: String) -> void:
	if music_tracks.has(scene_name):
		play_music(scene_name)
	else:
		Logger.info(self, "Scene music '%s' not found, playing persistent track instead" % scene_name)
		play_persistent_music()

func stop_music() -> void:
	if music_player:
		music_player.stop()
		current_track = ""
		Logger.info(self, "Stopped music playback")

# Helper function to get an available SFX player
func _get_available_sfx_player() -> AudioStreamPlayer:
	# Find an available player
	var player = null
	for p in sfx_players:
		if not p.playing:
			player = p
			break
	
	# If all players are busy, use the first one
	if player == null and sfx_players.size() > 0:
		player = sfx_players[0]
		
	return player

func play_sfx(sfx_name: String, volume_scale: float = 1.0) -> void:
	if not sound_effects.has(sfx_name):
		Logger.warn(self, "Cannot play SFX '%s' - not loaded" % sfx_name)
		return
	
	# Find an available player
	var player = _get_available_sfx_player()
	if player:
		player.stream = sound_effects[sfx_name]
		
		# Apply volume scaling (volume_scale is a multiplier for the base sfx_volume)
		var final_volume = sfx_volume * volume_scale
		player.volume_db = linear_to_db(final_volume)
		
		player.play()
		Logger.info(self, "Playing SFX: %s with volume scale: %s" % [sfx_name, volume_scale])
	else:
		Logger.warn(self, "No available SFX players to play: %s" % sfx_name)

func set_music_volume(vol: float) -> void:
	music_volume = clamp(vol, 0.0, 1.0)
	
	# Set volume on the audio bus
	var music_bus_idx = AudioServer.get_bus_index("Music")
	if music_bus_idx >= 0:
		# Only apply volume if not muted
		var effective_volume = 0.0 if music_muted else music_volume
		AudioServer.set_bus_volume_db(music_bus_idx, linear_to_db(effective_volume))
		Logger.debug(self, "Set music bus volume to: %s" % effective_volume)
	
	# Also apply volume directly to the music player for immediate effect
	if music_player and !music_muted:
		music_player.volume_db = linear_to_db(music_volume)
		Logger.debug(self, "Set music player volume to: %s" % music_volume)
	
	# Save settings to ensure persistence
	_save_settings()

func set_sfx_volume(vol: float) -> void:
	sfx_volume = clamp(vol, 0.0, 1.0)
	
	# Set volume on the audio bus
	var sfx_bus_idx = AudioServer.get_bus_index("SFX")
	if sfx_bus_idx >= 0:
		AudioServer.set_bus_volume_db(sfx_bus_idx, linear_to_db(sfx_volume))
		Logger.debug(self, "Set SFX volume to: %s" % sfx_volume)
	
	# Save settings to ensure persistence
	_save_settings()

func set_mute(on: bool) -> void:
	Logger.info(self, "set_mute called with on=%s" % on)
	
	# Only proceed if the state is actually changing
	if music_muted == on:
		Logger.debug(self, "Mute state already matches requested state: %s" % on)
		return
		
	music_muted = on
	Logger.info(self, "Mute state changed to: %s" % on)
	
	# Calculate volumes based on mute state
	var music_bus_idx = AudioServer.get_bus_index("Music")
	var sfx_bus_idx = AudioServer.get_bus_index("SFX")
	
	# Set the volume on the audio bus
	if music_bus_idx >= 0:
		var mv = 0.0 if on else music_volume
		AudioServer.set_bus_volume_db(music_bus_idx, linear_to_db(mv))
		Logger.debug(self, "Set music bus volume to: %s" % mv)
	
	if sfx_bus_idx >= 0:
		var sv = 0.0 if on else sfx_volume
		AudioServer.set_bus_volume_db(sfx_bus_idx, linear_to_db(sv))
		Logger.debug(self, "Set SFX bus volume to: %s" % sv)
	
	# Also set the volume directly on the music player for immediate effect
	if music_player:
		var volume = 0.0 if on else music_volume
		music_player.volume_db = linear_to_db(volume)
		Logger.debug(self, "Set music player volume to: %s" % volume)
		
		# Ensure the music is playing if we're unmuting
		if !on and !music_player.playing:
			play_persistent_music()
	else:
		# No music player, create one if we're unmuting
		if !on:
			play_persistent_music()
		
	# Save settings to ensure persistence
	_save_settings()

func toggle_mute() -> bool:
	set_mute(not music_muted)
	return music_muted

func is_muted() -> bool:
	return music_muted

## Check if persistent music is currently playing
func is_persistent_music_playing() -> bool:
	return music_player.playing and current_track == persistent_track

## Restore the persistent music track after it was stopped (e.g., by a cutscene)
func restore_persistent_music() -> void:
	Logger.info(self, "Restoring persistent music track: %s" % persistent_track)
	
	if persistent_track.is_empty():
		Logger.warn(self, "No persistent track set to restore")
		return
		
	if not music_tracks.has(persistent_track):
		Logger.warn(self, "Persistent track not found in registered tracks: %s" % persistent_track)
		return
		
	# Only play if we're not already playing the persistent track
	if not (music_player.playing and current_track == persistent_track):
		play_music(persistent_track)
		
	# Ensure we mark this as the persistent track in case it was changed
	current_track = persistent_track
	using_scene_music = false

## Play music directly from a file path without registering it
func play_music_file(file_path: String, volume: float = 1.0) -> void:
	if not file_path or file_path.is_empty():
		Logger.warn(self, "Cannot play music - empty file path")
		return
	
	Logger.info(self, "Playing music file: %s with volume: %s" % [file_path, volume])
	
	# Load the music file
	var stream = load(file_path)
	if not stream:
		Logger.warn(self, "Could not load music file: %s" % file_path)
		return
	
	# Set up the music player
	music_player.stream = stream
	music_player.volume_db = linear_to_db(volume * music_volume)
	music_player.play()
	current_track = file_path
	Logger.info(self, "Started music playback with volume: %s" % volume)

## Play a sequence of voiceover files with a specific volume
func play_voiceover_sequence_with_volume(paths: Array, volume: float = 1.0, finished_callback: Callable = Callable()) -> void:
	voiceover_queue = paths.duplicate()
	on_voiceover_finished = finished_callback
	
	# Store the custom volume
	voiceover_player.set_meta("custom_volume", volume)
	
	# Start playing the sequence
	_play_next_voiceover_in_queue_with_volume()

func _play_next_voiceover_in_queue_with_volume():
	if voiceover_player.playing:
		voiceover_player.stop()
	if voiceover_queue.size() == 0:
		if on_voiceover_finished.is_valid():
			on_voiceover_finished.call()
		return
	
	var next_path = voiceover_queue.pop_front()
	var stream = load(next_path)
	if not stream:
		Logger.warn(self, "Could not load voiceover: %s" % next_path)
		_play_next_voiceover_in_queue_with_volume() # skip to next
		return
	
	# Get the custom volume if set
	var custom_volume = 1.0
	if voiceover_player.has_meta("custom_volume"):
		custom_volume = voiceover_player.get_meta("custom_volume")
	
	voiceover_player.stream = stream
	voiceover_player.finished.connect(_on_voiceover_finished_with_volume, CONNECT_ONE_SHOT)
	voiceover_player.volume_db = linear_to_db(custom_volume * sfx_volume)
	voiceover_player.play()
	Logger.info(self, "Playing voiceover: %s with volume: %s" % [next_path, custom_volume])

func _on_voiceover_finished_with_volume():
	_play_next_voiceover_in_queue_with_volume()

## Register music with more detailed configuration
## state: The game state this music is associated with (e.g., "menu", "battle")
## loop: Whether the music should loop
## persistent: Whether this is a persistent track that should be remembered
func register_music(path: String, state: String = "", loop: bool = true, persistent: bool = false) -> void:
	Logger.info(self, "Registering music: %s, state: %s, loop: %s, persistent: %s" % [path, state, loop, persistent])
	
	# Check if file exists
	var file = FileAccess.open(path, FileAccess.READ)
	if not file:
		Logger.warn(self, "Could not register music - file not found: %s" % path)
		return
	file.close()
	
	# Load the audio stream
	var stream = load(path)
	if not stream:
		Logger.warn(self, "Could not load music from path: %s" % path)
		return
	
	# Use the state as the track name, or the filename if no state provided
	var track_name = state
	if track_name.is_empty():
		track_name = path.get_file().get_basename()
	
	# Store the music in our tracks dictionary
	music_tracks[track_name] = stream
	# Remember the original file path for notifications/attribution
	music_track_paths[track_name] = path
	
	# Configure looping if it's an MP3
	if stream is AudioStreamMP3:
		stream.loop = loop
	
	# Update music configuration
	music_config[track_name] = persistent
	
	# If this is a persistent track and we don't have one yet, set it
	if persistent and persistent_track.is_empty():
		persistent_track = track_name
	
	Logger.info(self, "Registered music track: %s from %s" % [track_name, path])

## Register a sound effect with a specific name
func register_sfx(path: String, sfx_name: String = "") -> void:
	Logger.info(self, "Registering SFX: %s, name: %s" % [path, sfx_name])
	
	# Check if file exists
	var file = FileAccess.open(path, FileAccess.READ)
	if not file:
		Logger.warn(self, "Could not register SFX - file not found: %s" % path)
		return
	file.close()
	
	# Load the audio stream
	var stream = load(path)
	if not stream:
		Logger.warn(self, "Could not load SFX from path: %s" % path)
		return
	
	# Use the provided name or the filename if no name provided
	var effect_name = sfx_name
	if effect_name.is_empty():
		effect_name = path.get_file().get_basename()
	
	# Store the sound effect in our dictionary
	sound_effects[effect_name] = stream
	
	Logger.info(self, "Registered SFX: %s from %s" % [effect_name, path])

## Suspend the persistent music (for cutscenes, etc.)
func suspend_persistent_music() -> void:
	Logger.info(self, "Suspending persistent music")
	# Stop any current music
	stop_music()

## Resume the persistent music after it was suspended
func resume_persistent_music() -> void:
	Logger.info(self, "Resuming persistent music")
	play_persistent_music()

## Check if any SFX is currently playing
func is_sfx_playing() -> bool:
	for player in sfx_players:
		if player.playing:
			return true
	return false

## Get the approximate duration of an SFX file
func get_sfx_duration(file_path: String) -> float:
	# Try to load the audio stream
	var stream = load(file_path)
	if not stream:
		Logger.warn(self, "Could not load audio file to get duration: %s" % file_path)
		return 0.0
		
	# Check if it's a valid audio stream
	if stream is AudioStream:
		# For most audio streams, we can get the length directly
		if stream.has_method("get_length"):
			return stream.get_length()
		# Check specific audio stream types in Godot 4.x
		elif "get_length" in stream:
			return stream.get_length()
		elif stream is AudioStreamWAV:
			# For WAV files, calculate based on sample rate and sample count
			var sample_count = stream.get_sample_count()
			var sample_rate = stream.get_mix_rate()
			if sample_rate > 0:
				return float(sample_count) / float(sample_rate)
	
	# Default duration if we can't determine it
	return 3.0

## Get the remaining play time of the currently playing SFX
## Returns the longest remaining time if multiple SFX are playing
func get_remaining_sfx_time() -> float:
	var longest_time = 0.0
	
	for player in sfx_players:
		if player.playing and player.stream:
			var total_length = 0.0
			
			# Try to get the length of the audio stream
			if player.stream.has_method("get_length"):
				total_length = player.stream.get_length()
			# Check specific audio stream types in Godot 4.x
			elif "get_length" in player.stream:
				total_length = player.stream.get_length()
			
			# Calculate remaining time
			if total_length > 0:
				var playback_pos = player.get_playback_position()
				var remaining = total_length - playback_pos
				if remaining > longest_time:
					longest_time = remaining
	
	# Return at least 0.5 seconds if we found any playing audio
	return max(0.5, longest_time)

## Stop all currently playing sound effects
func stop_sfx() -> void:
	Logger.info(self, "Stopping all SFX")
	for player in sfx_players:
		if player.playing:
			player.stop()

## Play the persistent background track
func play_persistent_music() -> void:
	Logger.info(self, "Attempting to play persistent track: %s" % persistent_track)
	
	if persistent_track.is_empty():
		Logger.warn(self, "No persistent track set")
		return
		
	if not music_tracks.has(persistent_track):
		Logger.warn(self, "Persistent track not found in registered tracks: %s" % persistent_track)
		return
		
	play_music(persistent_track)
	
	# Ensure we mark this as the persistent track in case it was changed
	current_track = persistent_track
	using_scene_music = false



## Play a registered persistent track by name and set it as the current persistent track
func play_and_set_persistent_music(track_name: String) -> bool:
	Logger.info(self, "Attempting to play and set persistent track: %s" % track_name)
	
	if not music_tracks.has(track_name):
		Logger.warn(self, "Cannot play persistent track - track not registered: %s" % track_name)
		return false
	
	# Set this as the persistent track
	persistent_track = track_name
	music_config[track_name] = true
	
	# Play the track
	play_music(track_name)
	
	# Mark that we're using the persistent track
	using_scene_music = false
	
	Logger.info(self, "Now playing persistent track: %s" % track_name)
	return true

## Setter for persistent_track
func set_persistent_track(track_name: String) -> bool:
	Logger.info(self, "Setting persistent track to: %s" % track_name)
	
	if not music_tracks.has(track_name):
		Logger.warn(self, "Cannot set persistent track - track not registered: %s" % track_name)
		return false
	
	persistent_track = track_name
	
	# Update the music configuration to mark this as persistent
	music_config[track_name] = true
	
	Logger.info(self, "Set persistent track to: %s" % track_name)
	return true

## Dynamically load a music track at runtime
func load_music(track_name: String, path: String) -> void:
	var stream = load(path)
	if stream:
		music_tracks[track_name] = stream
		music_track_paths[track_name] = path
		Logger.info(self, "Loaded music track: %s from %s" % [track_name, path])
	else:
		Logger.warn(self, "Could not load music from path: %s" % path)

## Load a sound effect at runtime
func load_sfx(sfx_name: String, path: String) -> void:
	# Try to load the audio stream
	var stream = load(path)
	if stream and stream is AudioStream:
		sound_effects[sfx_name] = stream
		Logger.info(self, "Loaded SFX: %s from %s" % [sfx_name, path])
	else:
		# Try alternative loading method for imported files
		if FileAccess.file_exists(path):
			Logger.warn(self, "File exists but failed to load as AudioStream: %s" % path)
			Logger.warn(self, "This may be due to import settings - file will be skipped")
		else:
			Logger.warn(self, "SFX file not found: %s" % path)
		
		# Don't increment the loaded count for failed loads
		return
