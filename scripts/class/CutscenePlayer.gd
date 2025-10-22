extends Control
class_name CutscenePlayer

# Set the log level for this script
var log_level: int = 1

signal cutscene_advanced(frame_index)
signal cutscene_completed

# Resource
var cutscene_path: String = ""
var cutscene_resource: CutsceneResource = null

# Frame tracking
var current_frame: int = -1
var total_frames: int = 0
var is_playing: bool = true
var auto_advance_timer_active: bool = false

# Typing effect
var is_typing: bool = false
var typing_speed: float = 0.05
var typing_timer: float = 0.0
var current_position: int = 0
var full_text: String = ""
var accumulated_text: String = ""

# Audio tracking
var audio_duration: float = 0.0
var was_persistent_music_playing: bool = false

# UI References
var background_panel: Control = null
var background: TextureRect = null
var frame_image: TextureRect = null
var title_label: Label = null
var story_text_label: RichTextLabel = null
var next_button: Button = null
var skip_button: Button = null
var continue_button: Button = null

# Settings
var clear_story_on_frame: bool = false

# External managers
var audio_manager = null

func _ready():
	Logger.info(self, "_ready called")
	
	# Get audio manager if available
	audio_manager = get_node_or_null("/root/AudioManager")
	if not audio_manager:
		Logger.warn(self, "AudioManager not found at /root/AudioManager")
		# Try alternative paths
		audio_manager = get_node_or_null("/root/Audio_Manager")
		if not audio_manager:
			Logger.warn(self, "AudioManager not found at /root/Audio_Manager either")
	
	Logger.debug(self, "AudioManager reference: %s" % audio_manager)
	
	# Get UI references
	background = $Background
	frame_image = $MainContainer/HBoxContainer/RightSideContainer/FrameImage
	title_label = $TitleLabel if has_node("TitleLabel") else null
	story_text_label = $MainContainer/HBoxContainer/FrameText/MarginContainer/StoryTextLabel
	
	Logger.debug(self, "UI references:")
	Logger.debug(self, "  - background: %s" % background)
	Logger.debug(self, "  - frame_image: %s" % frame_image)
	Logger.debug(self, "  - title_label: %s" % title_label)
	Logger.debug(self, "  - story_text_label: %s" % story_text_label)
	
	var scene_control = $MainContainer/HBoxContainer/RightSideContainer/SceneControlContainer/SceneControl
	
	if scene_control:
		for child in scene_control.get_children():
			match child.name:
				"NextButton":
					next_button = child
					if next_button and not next_button.pressed.is_connected(Callable(self, "_on_next_pressed")):
						next_button.pressed.connect(Callable(self, "_on_next_pressed"))
				"SkipButton":
					skip_button = child
					if skip_button and not skip_button.pressed.is_connected(Callable(self, "_on_skip_pressed")):
						skip_button.pressed.connect(Callable(self, "_on_skip_pressed"))
				"ContinueButton":
					continue_button = child
					if continue_button and not continue_button.pressed.is_connected(Callable(self, "_on_continue_pressed")):
						continue_button.pressed.connect(Callable(self, "_on_continue_pressed"))
		
		# If continue button doesn't exist, create it (should already exist in the scene now)
		if not continue_button:
			continue_button = Button.new()
			continue_button.text = "Continue"
			continue_button.custom_minimum_size = Vector2(120, 40)
			continue_button.visible = false
			continue_button.pressed.connect(Callable(self, "_on_continue_pressed"))
			scene_control.add_child(continue_button)
	
	Logger.debug(self, "Button references:")
	Logger.debug(self, "  - next_button: %s" % next_button)
	Logger.debug(self, "  - skip_button: %s" % skip_button)
	Logger.debug(self, "  - continue_button: %s" % continue_button)
	
	# Hide until a cutscene is loaded
	visible = false

func _process(delta):
	if is_typing:
		typing_timer += delta
		
		# Add characters at the specified typing speed
		while typing_timer >= typing_speed and current_position < full_text.length():
			typing_timer -= typing_speed
			current_position += 1
			
			# Update the text with the current number of characters
			var current_segment = full_text.substr(0, current_position)
			
			# Update the text display
			if story_text_label:
				var display_text = accumulated_text
				if not accumulated_text.is_empty() and not current_segment.is_empty():
					display_text += "\n\n"
				display_text += current_segment
				
				TextUtils.assign(story_text_label, display_text)
				
				# Ensure text scrolls to the bottom
				if story_text_label is RichTextLabel:
					story_text_label.scroll_to_line(story_text_label.get_line_count() - 1)
		
		# Check if typing is complete
		if current_position >= full_text.length():
			_finish_typing()

func _finish_typing():
	is_typing = false
	current_position = full_text.length()
	
	# Update the accumulated text with the full text
	if not accumulated_text.is_empty() and not full_text.is_empty():
		accumulated_text += "\n\n"
	accumulated_text += full_text
	
	# Update the text display
	if story_text_label:
		TextUtils.assign(story_text_label, accumulated_text)
		
		# Ensure text scrolls to the bottom
		if story_text_label is RichTextLabel:
			story_text_label.scroll_to_line(story_text_label.get_line_count() - 1)
	
	# Enable and show the next button if not in auto_next mode
	if next_button and cutscene_resource and not cutscene_resource.auto_next:
		next_button.disabled = false
		next_button.visible = true
	
	# If auto_next is enabled, set up auto-advance after a short delay
	if cutscene_resource and cutscene_resource.auto_next:
		var delay = 1.0  # Short delay after typing finishes
		_setup_auto_advance(current_frame, delay)

func load_cutscene(resource_path: String) -> bool:
	# Check if cutscenes are disabled in settings
	if SettingsManager.get_setting("gameplay", "hide_cutscenes", false):
		Logger.info(self, "Cutscenes are disabled in settings, skipping")
		# Emit the completed signal immediately
		emit_signal("cutscene_completed")
		return false
	
	Logger.info(self, "Loading cutscene from: %s" % resource_path)
	cutscene_path = resource_path
	cutscene_resource = load(resource_path)
	if not cutscene_resource:
		Logger.error(self, "Failed to load cutscene: %s" % resource_path)
		return false
	
	Logger.info(self, "Loaded cutscene: %s" % cutscene_resource.name)
	
	# Reset player state
	is_playing = true
	current_frame = -1
	accumulated_text = ""
	auto_advance_timer_active = false
	is_typing = false
	
	# Set up the cutscene
	visible = true
	total_frames = cutscene_resource.frames.size()
	typing_speed = cutscene_resource.typing_speed
	
	# Reset button state
	if next_button:
		next_button.visible = not cutscene_resource.auto_next
	if skip_button:
		skip_button.visible = true
	if continue_button:
		continue_button.visible = false
	
	# Start the cutscene
	_start_cutscene()
	return true

func _start_cutscene():
	if not cutscene_resource:
		Logger.error(self, "No cutscene resource set")
		return
		
	Logger.info(self, "Starting cutscene: %s" % cutscene_resource.name)
	
	# Set up the cutscene
	total_frames = cutscene_resource.frames.size()
	if total_frames == 0:
		Logger.error(self, "Cutscene has no frames")
		return
	
	# Apply theme if specified
	if cutscene_resource.theme and not cutscene_resource.theme.is_empty():
		_apply_theme(cutscene_resource.theme)
	
	# Set background color if specified
	if "background_color" in cutscene_resource and cutscene_resource.background_color:
		var bg_color = Color(cutscene_resource.background_color)
		if background_panel:
			background_panel.modulate = bg_color
	
	# Load background image if specified
	if cutscene_resource.background_image and not cutscene_resource.background_image.is_empty():
		_load_background_image(cutscene_resource.background_image)
	
	# Play music if specified
	if audio_manager and cutscene_resource.music_path and not cutscene_resource.music_path.is_empty():
		Logger.info(self, "Playing music: %s" % cutscene_resource.music_path)
		
		# Check if persistent music is playing and stop it
		if audio_manager.has_method("is_persistent_music_playing"):
			was_persistent_music_playing = audio_manager.is_persistent_music_playing()
			if was_persistent_music_playing and audio_manager.has_method("stop_music"):
				audio_manager.stop_music()
		
		# Set music volume if specified
		var music_volume = 1.0
		if "music_volume" in cutscene_resource:
			music_volume = cutscene_resource.music_volume
		
		# Play the music
		if audio_manager.has_method("play_music_file"):
			audio_manager.play_music_file(cutscene_resource.music_path, music_volume)
	
	# Show the first frame
	current_frame = -1
	_advance_frame()

func show_frame(frame_index: int):
	if not cutscene_resource or frame_index < 0 or frame_index >= cutscene_resource.frames.size():
		Logger.error(self, "Invalid frame index: %s" % frame_index)
		return
	
	Logger.info(self, "Showing frame %s" % frame_index)
	
	var frame = cutscene_resource.frames[frame_index]
	if not frame:
		Logger.error(self, "Frame is null")
		return
	
	# Optionally clear the story text
	var should_clear = false
	if "clear_story_on_frame" in frame:
		should_clear = frame.clear_story_on_frame
	elif "clear_story_on_frame" in cutscene_resource:
		should_clear = cutscene_resource.clear_story_on_frame
	else:
		should_clear = clear_story_on_frame
		
	if should_clear and story_text_label:
		accumulated_text = ""

	# Show frame title if specified
	if title_label:
		if "title" in frame and frame.title and not frame.title.is_empty():
			title_label.text = frame.title
			title_label.visible = true
			Logger.debug(self, "Showing title: %s" % frame.title)
		else:
			title_label.text = ""
			title_label.visible = false
			Logger.debug(self, "No title to show")
	
	# Show the frame image if specified
	if "frame_image" in frame and frame.frame_image and not frame.frame_image.is_empty():
		Logger.debug(self, "Loading frame image: %s" % frame.frame_image)
		_load_image(frame.frame_image)
	else:
		# Hide the image if no image is specified
		if frame_image:
			frame_image.visible = false
			Logger.debug(self, "No frame image to show")
	
	# Stop any playing voiceovers
	if audio_manager:
		Logger.debug(self, "Stopping any playing voiceovers")
		if audio_manager.has_method("voiceover_queue_clear"):
			audio_manager.voiceover_queue_clear()
		elif audio_manager.has_method("stop_voiceover"):
			audio_manager.stop_voiceover()
	
	# Play the frame's voiceover
	var _is_playing_audio = false
	audio_duration = 0.0
	
	if audio_manager:
		# Check if voiceover_path exists and is not empty
		if "voiceover_path" in frame and frame.voiceover_path and not frame.voiceover_path.is_empty():
			var vo_path = frame.voiceover_path
			Logger.info(self, "Playing voiceover: %s" % vo_path)
			
			# Set voiceover volume if specified
			var voiceover_volume = 1.0
			if "voiceover_volume" in cutscene_resource:
				voiceover_volume = cutscene_resource.voiceover_volume
			
			# Play the voiceover
			if audio_manager.has_method("play_voiceover"):
				audio_manager.play_voiceover(vo_path)
				_is_playing_audio = true
			
			# Get the duration for timing
			if audio_manager.has_method("get_sfx_duration"):
				audio_duration = audio_manager.get_sfx_duration(vo_path)
				if audio_duration <= 0:
					audio_duration = 3.0
			else:
				# Default duration if we can't determine it
				audio_duration = 3.0
			
			Logger.debug(self, "Voiceover duration: %s" % audio_duration)
	
	# Calculate frame duration based on duration_mode
	var frame_duration = 0.0
	var duration_mode = "auto_voiceover"
	if "duration_mode" in frame and frame.duration_mode != null:
		duration_mode = frame.duration_mode
	
	Logger.debug(self, "Duration mode: %s" % duration_mode)
	
	if duration_mode == "manual":
		frame_duration = 3.0
		if "manual_duration" in frame and frame.manual_duration != null:
			frame_duration = frame.manual_duration
	elif duration_mode == "auto_text":
		frame_duration = 3.0
		if "text" in frame and frame.text != null:
			frame_duration = frame.text.length() * typing_speed
	elif duration_mode == "auto_voiceover":
		frame_duration = audio_duration
	else:
		# Fallback: prefer voiceover, then text
		frame_duration = audio_duration
		if frame_duration <= 0 and "text" in frame and frame.text != null:
			frame_duration = frame.text.length() * typing_speed
	
	# Clamp to minimum 0.5s for safety
	frame_duration = max(0.5, frame_duration)
	Logger.debug(self, "Frame duration: %s" % frame_duration)
	
	# Show text with typing effect if needed
	if "text" in frame and frame.text != null and not frame.text.is_empty():
		full_text = frame.text
		Logger.debug(self, "Frame text: %s" % full_text)
		
		if _is_playing_audio and audio_duration > 0 and duration_mode == "auto_voiceover":
			# Sync typing speed to voiceover
			typing_speed = audio_duration / float(full_text.length()) if full_text.length() > 0 else 0.03
			Logger.debug(self, "Typing speed (synced to audio): %s" % typing_speed)
		else:
			# Use default typing speed
			typing_speed = cutscene_resource.typing_speed
			Logger.debug(self, "Typing speed (default): %s" % typing_speed)
		
		_start_typing_effect()
	else:
		# No text to display
		full_text = ""
		is_typing = false
		Logger.debug(self, "No text to display")
		
		# Enable the next button
		if next_button:
			next_button.disabled = false
	
	# Update button visibility based on auto_next setting
	if next_button:
		next_button.visible = not cutscene_resource.auto_next
		next_button.disabled = is_typing
	
	# Update current frame
	current_frame = frame_index
	emit_signal("cutscene_advanced", current_frame)
	
	# If auto_next is enabled, automatically advance to the next frame after correct duration
	if cutscene_resource and cutscene_resource.auto_next:
		_setup_auto_advance(frame_index, frame_duration)

func _load_image(image_path: String):
	if not frame_image:
		Logger.error(self, "Frame image node is null")
		return
		
	if image_path.is_empty():
		frame_image.visible = false
		return
		
	Logger.debug(self, "Loading image: %s" % image_path)
	
	var texture = _load_texture_from_path(image_path)
	if texture:
		frame_image.texture = texture
		frame_image.visible = true
		Logger.debug(self, "Successfully loaded image")
	else:
		frame_image.visible = false
		Logger.warn(self, "Failed to load image")

func _load_background_image(image_path: String):
	if not background:
		Logger.error(self, "Background node is null")
		return
		
	if image_path.is_empty():
		background.visible = false
		return
		
	Logger.debug(self, "Loading background image: %s" % image_path)
	
	var texture = _load_texture_from_path(image_path)
	if texture:
		background.texture = texture
		background.visible = true
		Logger.debug(self, "Successfully loaded background")
	else:
		background.visible = false
		Logger.warn(self, "Failed to load background")

func _load_texture_from_path(image_path: String) -> Texture2D:
	if image_path.is_empty():
		return null
		
	var texture = null
	
	# Handle different path formats
	if image_path.begins_with("uid://"):
		Logger.debug(self, "Loading texture from UID: %s" % image_path)
		texture = load(image_path)
	elif image_path.begins_with("res://"):
		Logger.debug(self, "Loading texture from res path: %s" % image_path)
		texture = load(image_path)
	else:
		Logger.debug(self, "Loading texture from relative path: %s" % image_path)
		texture = load("res://" + image_path)
		
	if not texture:
		Logger.warn(self, "Failed to load texture from path: %s" % image_path)
		
	return texture

func _on_next_pressed():
	if is_typing:
		_finish_typing()
	else:
		_advance_frame()

func _advance_frame():
	current_frame += 1
	if current_frame >= total_frames:
		_complete_cutscene()
	else:
		show_frame(current_frame)
		
		# Debug auto_next state
	if cutscene_resource:
		Logger.debug(self, "auto_next is %s" % cutscene_resource.auto_next)

func _setup_auto_advance(frame_index: int, delay: float):
	# Cancel any existing auto-advance timer
	auto_advance_timer_active = false
	
	# Create a new timer for auto-advancing
	if is_playing and delay > 0 and cutscene_resource and cutscene_resource.auto_next:
		Logger.debug(self, "Auto-advancing after %s seconds" % delay)
		auto_advance_timer_active = true
		await get_tree().create_timer(delay).timeout
		# Only call if still active (not canceled)
		if auto_advance_timer_active and is_playing:
			_on_auto_advance_timeout(frame_index)

func _on_auto_advance_timeout(frame_index: int):
	# Only advance if we're still on this frame and still playing
	if is_playing and current_frame == frame_index:
		_advance_frame()
	
	# Clear the timer reference
	auto_advance_timer_active = false

func _on_skip_pressed():
	# Immediately stop cutscene audio, but don't stop persistent music
	if audio_manager:
		# Only stop the cutscene-specific music, not persistent music
		if audio_manager.has_method("stop_cutscene_music"):
			audio_manager.stop_cutscene_music()
		elif audio_manager.has_method("stop_music"):
			# If we don't have a specific method, we'll have to restore it later
			if was_persistent_music_playing:
				Logger.info(self, "Will restore persistent music after skip")
			audio_manager.stop_music()
		
		# Stop other audio
		if audio_manager.has_method("stop_sfx"):
			audio_manager.stop_sfx()
		if audio_manager.has_method("stop_voiceover"):
			audio_manager.stop_voiceover()
	
	# Skip to the end
	_complete_cutscene()

func _complete_cutscene():
	Logger.info(self, "Completing cutscene")
	
	# Mark as not playing to prevent further frame advances
	is_playing = false
	
	# Cancel any auto-advance timer
	auto_advance_timer_active = false
	
	# Cancel any pending frames
	current_frame = total_frames - 1
	
	# Restore persistent music if it was playing before the cutscene
	if audio_manager and was_persistent_music_playing:
		Logger.info(self, "Restoring persistent music")
		if audio_manager.has_method("restore_persistent_music"):
			audio_manager.restore_persistent_music()
	
	# Emit the completed signal
	emit_signal("cutscene_completed")
	
	# Hide instead of removing from scene tree
	visible = false

func _on_continue_pressed():
	# Hide the cutscene player when continue is pressed
	visible = false
	if continue_button:
		continue_button.visible = false

func _start_typing_effect():
	# Reset typing effect variables
	current_position = 0
	typing_timer = 0
	is_typing = true
	
	# Disable the next button while typing
	if next_button and not cutscene_resource.auto_next:
		next_button.disabled = true
	
	# Ensure the text label is visible
	if story_text_label:
		story_text_label.visible = true

func _apply_theme(theme_name: String):
	var theme_path = "res://assets/themes/" + theme_name + "_theme.tres"
	var theme_resource = load(theme_path)
	if theme_resource:
		theme = theme_resource
