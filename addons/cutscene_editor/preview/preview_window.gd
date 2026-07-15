@tool
extends Window

signal preview_completed

func _ready():
	size = Vector2(1024, 768)
	title = "Testing Cutscene"
	close_requested.connect(func(): queue_free())

func show_cutscene(cutscene_resource):
	if not cutscene_resource:
		_show_error("No cutscene resource provided")
		return
		
	# Create a simple preview display
	var container = VBoxContainer.new()
	container.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT, Control.PRESET_MODE_MINSIZE, 10)
	add_child(container)
	
	# Add title
	var title_label = Label.new()
	title_label.text = "Previewing: " + cutscene_resource.name
	title_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	container.add_child(title_label)
	
	# Add frame info
	var info_label = Label.new()
	info_label.text = "Total frames: " + str(cutscene_resource.frames.size())
	info_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	container.add_child(info_label)
	
	# Add duration info
	var duration_label = Label.new()
	var total_duration = 0.0
	for frame in cutscene_resource.frames:
		var frame_duration = 3.0  # Default
		var duration_mode = frame.duration_mode if frame.duration_mode != null else "auto_voiceover"
		
		if duration_mode == "manual":
			frame_duration = frame.manual_duration if frame.manual_duration != null else 3.0
		elif duration_mode == "auto_text":
			frame_duration = (frame.text.length() if frame.text != null else 0) * cutscene_resource.typing_speed
			frame_duration = max(0.5, frame_duration)
		
		total_duration += frame_duration
	
	duration_label.text = "Estimated total duration: " + str(total_duration) + " seconds"
	duration_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	container.add_child(duration_label)
	
	# Add frame list
	var scroll = ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	container.add_child(scroll)
	
	var frame_list = VBoxContainer.new()
	scroll.add_child(frame_list)
	
	for i in range(cutscene_resource.frames.size()):
		var frame = cutscene_resource.frames[i]
		var frame_container = HBoxContainer.new()
		frame_list.add_child(frame_container)
		
		var frame_number = Label.new()
		frame_number.text = "Frame " + str(i+1) + ":"
		frame_number.custom_minimum_size.x = 100
		frame_container.add_child(frame_number)
		
		var frame_title = Label.new()
		frame_title.text = frame.title if frame.title else "[No Title]"
		frame_title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		frame_container.add_child(frame_title)
		
		var duration_mode_label = Label.new()
		duration_mode_label.text = frame.duration_mode if frame.duration_mode != null else "auto_voiceover"
		duration_mode_label.custom_minimum_size.x = 150
		frame_container.add_child(duration_mode_label)
		
		var duration_value = Label.new()
		if frame.duration_mode == "manual":
			duration_value.text = str(frame.manual_duration) + " sec"
		elif frame.duration_mode == "auto_text":
			var text_duration = (frame.text.length() if frame.text != null else 0) * cutscene_resource.typing_speed
			duration_value.text = str(text_duration) + " sec"
		else:
			duration_value.text = "3.0 sec (est.)"
		duration_value.custom_minimum_size.x = 100
		frame_container.add_child(duration_value)
	
	# Add close button
	var button_container = HBoxContainer.new()
	button_container.alignment = BoxContainer.ALIGNMENT_CENTER
	container.add_child(button_container)
	
	var close_button = Button.new()
	close_button.text = "Close Preview"
	close_button.pressed.connect(func(): queue_free())
	button_container.add_child(close_button)
	
	# Show the window
	popup_centered()

func _show_error(message):
	var error_label = Label.new()
	error_label.text = "Error: " + message
	error_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	error_label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	add_child(error_label)
	popup_centered()
