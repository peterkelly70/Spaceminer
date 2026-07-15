extends "res://scripts/ui/popups/BasePopup.gd"
class_name MusicSelectionPopup

# Set the log level for this script
var log_level: int = 1

# UI References
@onready var music_list = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/MusicList
@onready var play_button = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/PlayButton
@onready var stop_button = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/StopButton
@onready var close_x_button = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/TitleContainer/CloseButton
@onready var repeat_toggle = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/OptionsContainer/RepeatToggle

# Volume controls
@onready var volume_slider = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/VolumeContainer/HBoxContainer/VolumeSlider
@onready var mute_button = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/VolumeContainer/HBoxContainer/MuteButton

# Music tracks database
var music_database = {}
var music_tracks = {}

# Path to the music database file
const MUSIC_DATABASE_PATH = "res://assets/database/music.json"

# Currently playing track
var current_track = ""

# Music settings
var repeat_enabled = true

func _ready():
	super._ready()
	
	# Set popup title
	title = "SELECT MUSIC TRACK"
	var title_label = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/TitleContainer/TitleLabel
	if title_label:
		title_label.text = title
	
	# Configure music list for scrolling
	if music_list:
		# Clear any existing items first
		music_list.clear()
		
		# Configure list properties
		music_list.allow_reselect = true
		music_list.fixed_icon_size = Vector2(16, 16)
		music_list.size_flags_vertical = Control.SIZE_EXPAND_FILL
		music_list.custom_minimum_size.y = 250
		music_list.visible = true
		
		# Connect double-click signal (Signal object API)
		if not music_list.item_activated.is_connected(Callable(self, "_on_music_list_item_activated")):
			music_list.item_activated.connect(Callable(self, "_on_music_list_item_activated"))
		
		print("Music list configured")
	else:
		push_error("Music list not found!")
	
	# Connect button signals - ensure all buttons work properly
	if play_button:
		# Disconnect any existing connections to avoid duplicates
		if play_button.pressed.is_connected(Callable(self, "_on_play_button_pressed")):
			play_button.pressed.disconnect(Callable(self, "_on_play_button_pressed"))
		# Connect the pressed signal
		play_button.pressed.connect(Callable(self, "_on_play_button_pressed"))
		print("Connected play button")
	
	if stop_button:
		# Disconnect any existing connections to avoid duplicates
		if stop_button.pressed.is_connected(Callable(self, "_on_stop_button_pressed")):
			stop_button.pressed.disconnect(Callable(self, "_on_stop_button_pressed"))
		# Connect the pressed signal
		stop_button.pressed.connect(Callable(self, "_on_stop_button_pressed"))
		print("Connected stop button")
	
	# Connect X button in title bar
	if close_x_button:
		# Disconnect any existing connections to avoid duplicates
		if close_x_button.pressed.is_connected(Callable(self, "_on_close_button_pressed")):
			close_x_button.pressed.disconnect(Callable(self, "_on_close_button_pressed"))
		# Connect the pressed signal to close methods
		close_x_button.pressed.connect(Callable(self, "_on_close_button_pressed"))
		close_x_button.pressed.connect(Callable(self, "_close"))
		print("Connected close X button")
	
	# Initialize and connect repeat toggle
	if repeat_toggle:
		# Set initial state (on by default)
		repeat_toggle.button_pressed = repeat_enabled
		if repeat_toggle.toggled.is_connected(Callable(self, "_on_repeat_toggled")):
			repeat_toggle.toggled.disconnect(Callable(self, "_on_repeat_toggled"))
		repeat_toggle.toggled.connect(Callable(self, "_on_repeat_toggled"))
	
	# Initialize volume controls
	_setup_volume_controls()
	
	# Load the music database
	_load_music_database()
	
	# Populate the music list
	_populate_music_list()
	
	# Force update the list visibility
	if music_list:
		# Ensure the list is visible
		music_list.visible = true
		
		# Request a redraw of the control
		music_list.queue_redraw()
		
		# Set the initial selection
		if music_list.get_item_count() > 0:
			music_list.select(0)
			print("Selected first track")
		else:
			push_warning("No items in music list!")

func _init_custom(_init_params: Dictionary) -> void:
	# Custom initialization if needed
	pass

# Load the music database from JSON
func _load_music_database():
	# Load the music database file
	var file = FileAccess.open(MUSIC_DATABASE_PATH, FileAccess.READ)
	if not file:
		push_error("Failed to open music database file: " + MUSIC_DATABASE_PATH)
		return
	
	# Parse the JSON data
	var json_text = file.get_as_text()
	file.close()
	
	var json = JSON.new()
	var error = json.parse(json_text)
	if error != OK:
		push_error("Failed to parse music database JSON: " + json.get_error_message())
		return
	
	# Store the parsed data
	var data = json.data
	if not data or not data.has("tracks") or not data["tracks"] is Array:
		push_error("Invalid music database format")
		return
	
	# Process the tracks
	music_database = {}
	music_tracks = {}
	
	for track in data["tracks"]:
		if track.has("id") and track.has("title") and track.has("file"):
			var track_id = track["id"]
			music_database[track_id] = track
			music_tracks[track["title"]] = track["file"]
			print("Loaded track: " + track["title"])

# Populate the music list from the database
func _populate_music_list():
	if not music_list:
		return
	
	# Clear the list first
	music_list.clear()
	
	# Try to load music database
	var music_db_file = FileAccess.open(MUSIC_DATABASE_PATH, FileAccess.READ)
	if music_db_file:
		var json_text = music_db_file.get_as_text()
		music_db_file.close()
		
		var json = JSON.new()
		var error = json.parse(json_text)
		if error == OK:
			var data = json.data
			if data is Dictionary and data.has("tracks") and data["tracks"] is Array:
				var tracks = data["tracks"]
				for i in range(tracks.size()):
					var track = tracks[i]
					if track is Dictionary and track.has("title") and track.has("file"):
						# Format track number with leading zero
						var track_num = i + 1
						var display_text = "%02d. %s" % [track_num, track["title"]]
						
						# Add to music list
						music_list.add_item(display_text)
						
						# Store in our database with the display text as the key
						music_database[display_text] = {
							"title": track["title"],
							"path": track["file"]
						}
						
						# Also store in our music tracks dictionary for backwards compatibility
						music_tracks[track["title"]] = track["file"]
						
						print("Added track: " + display_text)
			else:
				push_error("Invalid music database format")
		else:
			push_error("Failed to parse music database: " + json.get_error_message())
	
	# If no tracks were added, add some fallbacks
	if music_list.get_item_count() == 0:
		var default_tracks = [
			"The Hall Under the Mountain",
			"Enter the Dwarven King",
			"Call of War",
			"Dwarven Military Elite",
			"Studies in Dwarven Magic",
			"Hall of the Mountain King",
			"Dance of the Sugar Plum Fairies"
		]
		
		for i in range(default_tracks.size()):
			var track_num = i + 1
			var _display_text = "%02d. " % track_num + default_tracks[i]
			music_list.add_item(_display_text)
			print("Added fallback track: " + _display_text)
	
	# Make sure the list is visible
	music_list.visible = true
	print("Music list populated with " + str(music_list.get_item_count()) + " items")

func _on_play_button_pressed():
	# Play click sound
	if Audio_Manager and Audio_Manager.has_method("play_sfx"):
		Audio_Manager.play_sfx("click")
	
	# Check if music list exists
	if not music_list:
		push_error("No music list found")
		return
	
	# Get selected track
	var selected_items = music_list.get_selected_items()
	if selected_items.size() > 0:
		var selected_index = selected_items[0]
		var selected_track_text = music_list.get_item_text(selected_index)
		
		print("Selected track: " + selected_track_text)
		
		# Try to find the track in our database
		if music_database.has(selected_track_text):
			var track_data = music_database[selected_track_text]
			var track_path = track_data["path"]
			
			# Play the track using direct method calls
			if Audio_Manager:
				# Stop current music
				if Audio_Manager.has_method("stop_music"):
					Audio_Manager.stop_music()
				
				# Set the track to repeat if enabled
				var stream = load(track_path)
				if stream and (stream is AudioStreamMP3 or stream is AudioStreamOggVorbis):
					stream.loop = repeat_enabled
				
				# Play the music directly
				if Audio_Manager.has_method("play_music_file"):
					print("Playing track via play_music_file: " + track_path)
					Audio_Manager.play_music_file(track_path, 1.0, repeat_enabled)
					current_track = selected_track_text
					
					# Update music config in AudioManager
					Audio_Manager.music_config[selected_track_text] = repeat_enabled
					print("Set repeat for track to: " + str(repeat_enabled))
					
					# Emit action signal
					_emit_action("play_music", {
						"track": selected_track_text, 
						"path": track_path,
						"repeat": repeat_enabled
					})
				else:
					# Fallback to direct music player manipulation
					print("Fallback: Playing track directly through music_player")
					if Audio_Manager.music_player and stream:
						Audio_Manager.music_player.stream = stream
						Audio_Manager.music_player.play()
						current_track = selected_track_text
			else:
				push_error("AudioManager not available")
		else:
			# Fallback: try to find the track in our music_tracks dictionary
			print("Track not found in database, trying fallback")
			
			# Extract track title without number if present
			var track_title = selected_track_text
			var dot_pos = selected_track_text.find(". ")
			if dot_pos >= 0:
				track_title = selected_track_text.substr(dot_pos + 2)
			
			print("Using fallback method with title: " + track_title)
			if music_tracks.has(track_title):
				_play_track_by_name(track_title)
			else:
				push_error("Track not found in music_tracks: " + track_title)
	else:
		push_error("No track selected")

# Method for playing tracks by name - fail fast approach
func _play_track_by_name(item_name: String):
	print("Attempting to play track: " + item_name)
	
	# Check if AudioManager exists
	if not Audio_Manager:
		push_error("AudioManager not found")
		return false
	
	# Check if AudioManager has the required method
	if not Audio_Manager.has_method("play_music"):
		push_error("AudioManager missing play_music method")
		return false
	
	# Try to play using the exact name
	if Audio_Manager.music_tracks.has(item_name):
		print("Found track in AudioManager.music_tracks: " + item_name)
		Audio_Manager.play_sfx("click")
		Audio_Manager.play_music(item_name)
		current_track = item_name
		return true
	
	# Try with the track name without number prefix
	var dot_pos = item_name.find(". ")
	if dot_pos >= 0:
		var clean_name = item_name.substr(dot_pos + 2)
		if Audio_Manager.music_tracks.has(clean_name):
			print("Found track with clean name: " + clean_name)
			Audio_Manager.play_sfx("click")
			Audio_Manager.play_music(clean_name)
			current_track = clean_name
			return true
	
	# Fail loudly
	push_error("Track not found in AudioManager: " + item_name)
	return false

func _on_stop_button_pressed():
	# Play click sound
	if Audio_Manager and Audio_Manager.has_method("play_sfx"):
		Audio_Manager.play_sfx("click")
	
	# Stop the currently playing music
	if Audio_Manager and Audio_Manager.has_method("stop_music"):
		Audio_Manager.stop_music()
		current_track = ""
		print("Stopped music playback")
		
		# Emit action signal
		_emit_action("stop_music")

# Setup volume controls
func _setup_volume_controls():
	# Initialize volume slider
	if volume_slider:
		# Set initial value from SettingsManager
		if SettingsManager:
			# Get the actual music volume from SettingsManager (0.0-1.0) and convert to percentage (0-100)
			var music_volume = SettingsManager.get_setting("audio", "music_volume", 0.8) * 100
			print("Initial music volume from SettingsManager: " + str(music_volume) + "%")
			volume_slider.value = music_volume
		else:
			volume_slider.value = 80  # Default to 80%
		
		# Connect value changed signal
		if volume_slider.value_changed.is_connected(Callable(self, "_on_volume_changed")):
			volume_slider.value_changed.disconnect(Callable(self, "_on_volume_changed"))
		volume_slider.value_changed.connect(Callable(self, "_on_volume_changed"))
	
	# Initialize mute button - using CheckButton
	if mute_button:
		# Set initial state from SettingsManager
		if SettingsManager:
			# Get the music_enabled setting and invert it for the mute button
			var music_enabled = SettingsManager.get_setting("audio", "music_enabled", true)
			print("Initial music_enabled from SettingsManager: " + str(music_enabled))
			mute_button.button_pressed = !music_enabled  # Invert since mute is the opposite of enabled
		else:
			mute_button.button_pressed = false
		
		# Connect toggled signal for CheckButton
		if mute_button.toggled.is_connected(Callable(self, "_on_mute_toggled")):
			mute_button.toggled.disconnect(Callable(self, "_on_mute_toggled"))
		mute_button.toggled.connect(Callable(self, "_on_mute_toggled"))
		print("Connected mute button")
	
	# Apply current settings to AudioManager if available
	_apply_audio_settings()

# Apply current audio settings to the AudioManager
func _apply_audio_settings():
	if Audio_Manager:
		# Apply volume setting
		if SettingsManager:
			var volume = SettingsManager.get_setting("audio", "music_volume", 0.8)
			if Audio_Manager.has_method("set_music_volume"):
				Audio_Manager.set_music_volume(volume)
				print("Applied music volume to AudioManager: " + str(volume))
			
			# Apply mute setting
			var music_enabled = SettingsManager.get_setting("audio", "music_enabled", true)
			if Audio_Manager.has_method("set_music_muted"):
				Audio_Manager.set_music_muted(!music_enabled)
				print("Applied music mute to AudioManager: " + str(!music_enabled))

# Handle volume slider change
func _on_volume_changed(value: float):
	# Play click sound
	if Audio_Manager and Audio_Manager.has_method("play_sfx"):
		Audio_Manager.play_sfx("click")
	
	# Update settings through SettingsManager
	if SettingsManager:
		# Convert from percentage (0-100) to float (0.0-1.0)
		var volume_value = value / 100.0
		SettingsManager.set_setting("audio", "music_volume", volume_value)
		SettingsManager.apply_settings()
		SettingsManager.save_settings()
		print("Volume set to: " + str(value) + "%")
		
		# Apply to AudioManager directly
		if Audio_Manager and Audio_Manager.has_method("set_music_volume"):
			Audio_Manager.set_music_volume(volume_value)
		
		# Emit action signal
		_emit_action("set_volume", {"value": volume_value})

# Store previous volume when muting
var previous_volume: float = 0.8  # Default to 80%

# Handle mute button toggle
func _on_mute_toggled(toggled_on: bool):
	# Play click sound
	if Audio_Manager and Audio_Manager.has_method("play_sfx"):
		Audio_Manager.play_sfx("click")
	
	# Apply mute state directly to AudioManager
	if Audio_Manager and Audio_Manager.has_method("set_mute"):
		print("Setting mute state to: " + str(toggled_on))
		Audio_Manager.set_mute(toggled_on)
		
		# Store current volume before muting if we're muting
		if toggled_on and Audio_Manager.has_method("get_music_volume"):
			previous_volume = Audio_Manager.music_volume
			print("Stored previous volume: " + str(previous_volume))
			
			# Update slider to show 0
			if volume_slider:
				volume_slider.value = 0
		else:  # Unmuting
			# Restore previous volume
			if Audio_Manager and Audio_Manager.has_method("set_music_volume"):
				Audio_Manager.set_music_volume(previous_volume)
				print("Restored volume to: " + str(previous_volume))
			
			# Update slider to show previous volume
			if volume_slider:
				volume_slider.value = previous_volume * 100
		
	# Save settings
	if Audio_Manager and Audio_Manager.has_method("_save_settings"):
		Audio_Manager.call("_save_settings")
		
	# Emit action signal
	_emit_action("set_mute", {"muted": toggled_on, "previous_volume": previous_volume})
		
	# Also update SettingsManager if it exists
	if SettingsManager:
		# Note: music_enabled is the opposite of muted
		var music_enabled = !toggled_on
		SettingsManager.set_setting("audio", "music_enabled", music_enabled)
		SettingsManager.apply_settings()
		SettingsManager.save_settings()

# Handle double-click on music list item
func _on_music_list_item_activated(index: int):
	print("Double-clicked item at index: " + str(index))
	
	# Select the item
	music_list.select(index)
	
	# Play the selected track
	_on_play_button_pressed()

func _on_close_button_pressed():
	# Play click sound and close the dialog
	if Audio_Manager:
		Audio_Manager.play_sfx("click")
	
	# Emit action signal
	_emit_action("close")
	
	# Close dialog
	_close()
	# Force hide to ensure it's closed
	hide()

# Handle repeat toggle changes
func _on_repeat_toggled(toggled_on: bool) -> void:
	# Update our local setting
	repeat_enabled = toggled_on
	
	# Play click sound
	if Audio_Manager:
		Audio_Manager.play_sfx("click")
		
	# If a track is currently playing, update its loop setting
	if current_track != "" and music_tracks.has(current_track):
		# Update the music config in AudioManager
		Audio_Manager.music_config[current_track] = repeat_enabled
		
		# If possible, update the current stream's loop property
		var track_path = music_tracks[current_track]
		var stream = load(track_path)
		if stream and (stream is AudioStreamMP3 or stream is AudioStreamOggVorbis):
			stream.loop = repeat_enabled
			print("[MusicSelectionPopup] Updated repeat setting for current track: " + str(repeat_enabled))
			
	# Emit action signal
	_emit_action("set_repeat", {"enabled": repeat_enabled})

# Apply custom styling for the music list
func _apply_theme() -> void:
	# Call the parent implementation first
	super._apply_theme()
	
	# Load the default theme
	var default_theme = load(DEFAULT_THEME_PATH)
	if not default_theme:
		push_error("[MusicSelectionPopup] Failed to load default theme")
		return
	
	# Apply custom styling to the music list
	if music_list:
		var paragraph_font = load("res://assets/fonts/Oswald/Oswald-VariableFont_wght.ttf")
		if paragraph_font:
			music_list.add_theme_font_override("font", paragraph_font)
		else:
			# Fallback to theme font
			var font = default_theme.get_font("font", "Label")
			music_list.add_theme_font_override("font", font)
		
		music_list.add_theme_font_size_override("font_size", 18)
		
		# Create a stylebox for the selected item using theme colors
		var selected_style = StyleBoxFlat.new()
		selected_style.bg_color = default_theme.get_color("tab_selected", "TabContainer")
		selected_style.border_width_left = 1
		selected_style.border_width_top = 1
		selected_style.border_width_right = 1
		selected_style.border_width_bottom = 1
		selected_style.border_color = default_theme.get_color("accent_color", "Button")
		selected_style.corner_radius_top_left = 4
		selected_style.corner_radius_top_right = 4
		selected_style.corner_radius_bottom_right = 4
		selected_style.corner_radius_bottom_left = 4
		
		music_list.add_theme_stylebox_override("selected", selected_style)
		music_list.add_theme_stylebox_override("selected_focus", selected_style)
