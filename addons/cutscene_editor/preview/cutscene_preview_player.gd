@tool
extends Control
class_name CutscenePreviewPlayer

signal cutscene_completed
signal cutscene_advanced

@onready var background: TextureRect = $Background
@onready var frame_text_panel: Control = $FrameText
@onready var story_text_label: RichTextLabel = $FrameText/StoryTextLabel
@onready var title_label: Label = $FrameText/TitleLabel
@onready var frame_image: TextureRect = $FrameImage
@onready var frame_box: Control = $FrameBox

var cutscene_resource: Resource
var current_frame: int = 0
var total_frames: int = 0
var cutscene_path: String = ""

var typing_speed: float = 0.03
var is_typing: bool = false
var full_text: String = ""
var current_position: int = 0
var typing_timer: float = 0.0
var accumulated_text: String = ""

var next_button: Button
var skip_button: Button

# Audio duration for the current frame
var audio_duration: float = 0.0

func _enter_tree():
	visible = false

func _ready():
	print("PreviewPlayer: _ready called")
	_setup_buttons()
	
func _setup_buttons():
	print("PreviewPlayer: _setup_buttons called")
	var scene_control = $SceneControl
	print("PreviewPlayer: scene_control = ", scene_control)
	
	if not scene_control:
		push_error("SceneControl not found")
		return

	next_button = null
	skip_button = null
	
	for child in scene_control.get_children():
		print("PreviewPlayer: child = ", child, " name = ", child.name)
		match child.name:
			"NextButton":
				print("PreviewPlayer: Found NextButton")
				next_button = child
				if next_button and not next_button.pressed.is_connected(_on_next_pressed):
					next_button.pressed.connect(_on_next_pressed)
			"SkipButton":
				print("PreviewPlayer: Found SkipButton")
				skip_button = child
				if skip_button and not skip_button.pressed.is_connected(_on_skip_pressed):
					skip_button.pressed.connect(_on_skip_pressed)
	
	print("PreviewPlayer: After setup - next_button = ", next_button, " skip_button = ", skip_button)
	
	if not next_button:
		push_warning("NextButton not found in SceneControl")
	if not skip_button:
		push_warning("SkipButton not found in SceneControl")

func _process(delta):
	if is_typing:
		typing_timer += delta
		
		# Add characters at the specified typing speed
		while typing_timer >= typing_speed and current_position < full_text.length():
			typing_timer -= typing_speed
			current_position += 1
			
			# Update the text with the current number of characters
			var current_segment = full_text.substr(0, current_position)
			story_text_label.text = current_segment
		
		# Check if typing is complete
		if current_position >= full_text.length():
			_finish_typing()
	
	# Allow skipping the typing effect with a click
	if Input.is_action_just_pressed("ui_accept") and is_typing:
		_finish_typing()

func _finish_typing():
	is_typing = false
	current_position = full_text.length()
	story_text_label.text = full_text
	accumulated_text = story_text_label.text
	
	# If auto_next is enabled, automatically advance to the next frame after a delay
	if cutscene_resource and cutscene_resource.auto_next:
		var wait_time = 2.0  # Default wait time for preview
		await get_tree().create_timer(wait_time).timeout
		_advance_frame()
	else:
		# Otherwise, enable the next button
		if next_button:
			next_button.disabled = false

func load_cutscene(resource_path: String) -> bool:
	cutscene_path = resource_path
	cutscene_resource = load(resource_path)
	if not cutscene_resource:
		push_error("Failed to load cutscene: " + resource_path)
		return false

	print("PreviewPlayer: Loaded cutscene: ", cutscene_resource.name)
	
	# Set up the cutscene
	visible = true
	total_frames = cutscene_resource.frames.size()
	typing_speed = cutscene_resource.typing_speed
	
	# Set background color
	if cutscene_resource.background_color:
		var color = Color(cutscene_resource.background_color)
		background.modulate = color
		background.visible = true
	
	current_frame = -1
	_advance_frame()
	return true

func reload_cutscene() -> bool:
	if cutscene_path.is_empty():
		return false
	
	# Execute the task without await since we're not using the result
	load_cutscene(cutscene_path)
	return true

func show_frame(frame_index: int):
	if frame_index < 0 or frame_index >= total_frames:
		push_error("Invalid frame index")
		return

	var frame = cutscene_resource.frames[frame_index]
	
	# Set title
	if frame.title and not frame.title.is_empty() and frame.show_title:
		title_label.text = frame.title
		title_label.visible = true
	else:
		title_label.text = ""
		title_label.visible = false
	
	# Set background image if specified
	if frame.background_image and not frame.background_image.is_empty():
		var texture = await _load_texture_from_path(frame.background_image)
		if texture:
			background.texture = texture
			background.visible = true
	else:
		# Use a black background if no image
		var black_texture = ImageTexture.create_from_image(Image.create(1, 1, false, Image.FORMAT_RGBA8))
		background.texture = black_texture
		background.visible = true
	
	# Handle frame image (separate from background)
	if frame_image:
		frame_image.visible = false
		
		# Check if frame has a frame-specific image
		if frame.frame_image and not frame.frame_image.is_empty():
			var texture = await _load_texture_from_path(frame.frame_image)
			if texture:
				frame_image.texture = texture
				frame_image.visible = true
				
				# Set image anchor
				match frame.image_anchor:
					"center":
						frame_image.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
					"fill":
						frame_image.stretch_mode = TextureRect.STRETCH_SCALE
					"top":
						frame_image.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
						frame_image.anchor_top = 0
						frame_image.anchor_bottom = 0.5
					"bottom":
						frame_image.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
						frame_image.anchor_top = 0.5
						frame_image.anchor_bottom = 1.0
					"left":
						frame_image.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
						frame_image.anchor_left = 0
						frame_image.anchor_right = 0.5
					"right":
						frame_image.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
						frame_image.anchor_left = 0.5
						frame_image.anchor_right = 1.0
	
	# Calculate frame duration based on duration_mode
	var frame_duration = 0.0
	var duration_mode = frame.duration_mode if frame.duration_mode != null else "auto_voiceover"
	if duration_mode == "manual":
		frame_duration = frame.manual_duration if frame.manual_duration != null else 3.0
	elif duration_mode == "auto_text":
		frame_duration = (frame.text.length() if frame.text != null else 0) * typing_speed
	else:
		# For preview, we'll use a fixed duration for auto_voiceover
		frame_duration = 3.0
	# Clamp to minimum 0.5s for safety
	frame_duration = max(0.5, frame_duration)

	# Show text with typing effect if needed
	if frame.text != null and not frame.text.is_empty():
		full_text = frame.text
		# Use default typing speed
		typing_speed = cutscene_resource.typing_speed
		_start_typing_effect()
	else:
		# Show text immediately without typing effect
		story_text_label.text = ""
		accumulated_text = ""
		is_typing = false

	# If auto_next is enabled, automatically advance to the next frame after correct duration
	if cutscene_resource.auto_next:
		print("PreviewPlayer: Auto-advancing after ", frame_duration, " seconds")
		await get_tree().create_timer(frame_duration).timeout
		# Only advance if we're still on this frame (user hasn't manually advanced)
		if current_frame == frame_index:
			_advance_frame()
	else:
		# Otherwise, enable the next button
		if next_button:
			next_button.disabled = false

	current_frame = frame_index
	emit_signal("cutscene_advanced", current_frame)

func _load_texture_from_path(image_path: String) -> Texture2D:
	if image_path.is_empty():
		push_warning("Empty image path provided")
		return null
		
	var texture = null
	
	# Handle different path formats
	if image_path.begins_with("uid://"):
		# Handle potential invalid UIDs by catching errors
		print("PreviewPlayer: Loading texture from UID: ", image_path)
		
		# Try to load with error handling
		var err = ResourceLoader.load_threaded_request(image_path, "", true)
		if err == OK:
			# Wait for loading to complete
			var status = ResourceLoader.load_threaded_get_status(image_path)
			while status == ResourceLoader.THREAD_LOAD_IN_PROGRESS:
				await get_tree().process_frame
				status = ResourceLoader.load_threaded_get_status(image_path)
				
			if status == ResourceLoader.THREAD_LOAD_LOADED:
				texture = ResourceLoader.load_threaded_get(image_path)
			else:
				push_warning("Failed to load texture from UID: " + image_path)
		else:
			push_warning("Invalid UID or resource not found: " + image_path)
	elif image_path.begins_with("res://"):
		print("PreviewPlayer: Loading texture from res path: ", image_path)
		texture = load(image_path)
	else:
		print("PreviewPlayer: Loading texture from relative path: ", image_path)
		texture = load("res://" + image_path)
		
	if not texture:
		push_warning("Failed to load texture from path: " + image_path)
		
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

func _on_skip_pressed():
	_complete_cutscene()

func _complete_cutscene():
	visible = false
	accumulated_text = ""
	story_text_label.text = ""
	emit_signal("cutscene_completed")

func _start_typing_effect():
	# Reset typing effect variables
	current_position = 0
	typing_timer = 0
	is_typing = true
	if next_button:
		next_button.disabled = true
