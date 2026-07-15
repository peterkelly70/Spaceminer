@tool
extends Control

signal cutscene_updated(cutscene_data)

var current_cutscene = null
var current_category = ""
var current_frame_index = 0
var frames = []

# Node references
@onready var name_edit = $VBoxContainer/ScrollContainer/PropertiesContainer/CutsceneNameSection/NameEdit
@onready var typing_speed_spinbox = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/TypingSpeedSpinBox
@onready var background_color_picker = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/BackgroundColorPickerButton
@onready var background_image_edit = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/BackgroundImageHBox/BackgroundImageEdit
@onready var browse_image_button = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/BackgroundImageHBox/BrowseImageButton
@onready var music_path_edit = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/MusicPathHBox/MusicPathEdit
@onready var browse_music_button = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/MusicPathHBox/BrowseButton
@onready var music_volume_slider = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/MusicVolumeSlider
@onready var voiceover_volume_slider = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/VoiceoverVolumeSlider
@onready var use_bbcode_checkbox = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/UseBBCodeCheckBox
@onready var theme_option_button = $VBoxContainer/ScrollContainer/PropertiesContainer/CutscenePropertiesSection/GridContainer/ThemeOptionButton
@onready var frame_list_container = $VBoxContainer/ScrollContainer/PropertiesContainer/FrameEditorSection/FrameListContainer
@onready var add_frame_button = $VBoxContainer/ScrollContainer/PropertiesContainer/FrameEditorSection/AddFrameButton
@onready var delete_frame_button = $VBoxContainer/ScrollContainer/PropertiesContainer/FrameEditorSection/DeleteFrameButton
@onready var save_button = $VBoxContainer/ButtonsContainer/SaveButton
@onready var preview_button = $VBoxContainer/ButtonsContainer/PreviewButton

# Frame editor
var frame_editor

# File dialogs
var music_file_dialog
var background_image_dialog

func _ready():
	# Connect signals
	name_edit.text_changed.connect(_on_name_changed)
	typing_speed_spinbox.value_changed.connect(_on_typing_speed_changed)
	background_color_picker.color_changed.connect(_on_background_color_changed)
	browse_music_button.pressed.connect(_on_browse_music_pressed)
	browse_image_button.pressed.connect(_on_browse_image_pressed)
	music_volume_slider.value_changed.connect(_on_music_volume_changed)
	voiceover_volume_slider.value_changed.connect(_on_voiceover_volume_changed)
	use_bbcode_checkbox.toggled.connect(_on_use_bbcode_toggled)
	theme_option_button.item_selected.connect(_on_theme_selected)
	add_frame_button.pressed.connect(_on_add_frame_pressed)
	delete_frame_button.pressed.connect(_on_delete_frame_pressed)
	save_button.pressed.connect(_on_save_pressed)
	preview_button.pressed.connect(_on_preview_pressed)
	
	# Create file dialogs
	_create_file_dialogs()
	
	# Create frame editor
	_create_frame_editor()
	
	# Populate theme options
	_populate_theme_options()
	
	# Disable editing until a cutscene is selected
	_set_editing_enabled(false)

func _create_file_dialogs():
	# Music file dialog
	music_file_dialog = FileDialog.new()
	music_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	music_file_dialog.access = FileDialog.ACCESS_RESOURCES
	music_file_dialog.filters = ["*.ogg, *.mp3, *.wav ; Audio Files"]
	music_file_dialog.size = Vector2(800, 600)
	music_file_dialog.file_selected.connect(_on_music_file_selected)
	add_child(music_file_dialog)
	
	# Background image file dialog
	background_image_dialog = FileDialog.new()
	background_image_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	background_image_dialog.access = FileDialog.ACCESS_RESOURCES
	background_image_dialog.filters = ["*.png, *.jpg, *.jpeg, *.webp ; Image Files"]
	background_image_dialog.size = Vector2(800, 600)
	background_image_dialog.file_selected.connect(_on_background_image_selected)
	add_child(background_image_dialog)

func _create_frame_editor():
	# Load frame editor scene
	var FrameEditorScene = load("res://addons/cutscene_editor/dock/frame_editor.tscn")
	if FrameEditorScene:
		frame_editor = FrameEditorScene.instantiate()
		frame_editor.frame_updated.connect(_on_frame_updated)
		$VBoxContainer/ScrollContainer/PropertiesContainer.add_child(frame_editor)
	else:
		push_error("Failed to load frame editor scene")

func _populate_theme_options():
	# Clear existing items
	theme_option_button.clear()
	
	# Add default theme
	theme_option_button.add_item("ironhaven")
	
	# Look for additional theme files in the themes directory
	var dir = DirAccess.open("res://themes")
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			if file_name.ends_with(".tres") and file_name != "ironhaven_theme.tres":
				var theme_name = file_name.replace(".tres", "")
				theme_option_button.add_item(theme_name)
			file_name = dir.get_next()
	else:
		print("Could not access themes directory")

func _on_cutscene_selected(cutscene_data, category_id):
	current_cutscene = cutscene_data
	current_category = category_id
	
	if current_cutscene:
		# Enable editing
		_set_editing_enabled(true)
		
		# Update UI with cutscene data
		name_edit.text = current_cutscene.name
		typing_speed_spinbox.value = current_cutscene.typing_speed
		
		# Set background color
		var bg_color = Color.BLACK
		bg_color.a = 0.8
		if current_cutscene.has_method("get") and current_cutscene.get("background_color") != null:
			bg_color = Color(current_cutscene.background_color)
		background_color_picker.color = bg_color
		
		# Set background image path
		background_image_edit.text = current_cutscene.background_image
		
		# Set music path
		music_path_edit.text = current_cutscene.music_path
		
		# Set volume sliders
		music_volume_slider.value = current_cutscene.music_volume if "music_volume" in current_cutscene else 1.0
		voiceover_volume_slider.value = current_cutscene.voiceover_volume if "voiceover_volume" in current_cutscene else 1.0
		
		# Set BBCode checkbox
		use_bbcode_checkbox.button_pressed = current_cutscene.use_bbcode
		
		# Select the theme
		for i in range(theme_option_button.get_item_count()):
			if theme_option_button.get_item_text(i) == current_cutscene.theme:
				theme_option_button.select(i)
				break
		
		# Load frames
		_load_frames()
		
		# Select first frame if available
		if frames.size() > 0:
			_on_frame_button_pressed(0)
	else:
		# Disable editing
		_set_editing_enabled(false)
		
		# Clear UI
		name_edit.text = ""
		typing_speed_spinbox.value = 0.05
		background_color_picker.color = Color(0, 0, 0, 0.8)
		background_image_edit.text = ""
		music_path_edit.text = ""
		use_bbcode_checkbox.button_pressed = false
		theme_option_button.select(0)  # Default to ironhaven theme
		
		# Clear frames
		for child in frame_list_container.get_children():
			child.queue_free()
		
		# Hide frame editor
		if frame_editor:
			frame_editor.visible = false

func _set_editing_enabled(enabled):
	name_edit.editable = enabled
	typing_speed_spinbox.editable = enabled
	background_color_picker.disabled = !enabled
	browse_image_button.disabled = !enabled
	browse_music_button.disabled = !enabled
	music_volume_slider.editable = enabled
	music_volume_slider.disabled = !enabled
	voiceover_volume_slider.editable = enabled
	voiceover_volume_slider.disabled = !enabled
	use_bbcode_checkbox.disabled = !enabled
	theme_option_button.disabled = !enabled
	add_frame_button.disabled = !enabled
	delete_frame_button.disabled = !enabled
	save_button.disabled = !enabled
	preview_button.disabled = !enabled

func _load_frames():
	# Clear existing frames
	for child in frame_list_container.get_children():
		child.queue_free()
	
	frames = current_cutscene.get("frames", [])
	
	# Create frame buttons
	for i in range(frames.size()):
		var frame = frames[i]
		var frame_container = _create_frame_container(i, frame)
		frame_list_container.add_child(frame_container)

func _create_frame_container(index, frame):
	# Create a container for the frame button and controls
	var container = HBoxContainer.new()
	container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	# Create up button
	var up_button = Button.new()
	up_button.text = "↑"
	up_button.tooltip_text = "Move Frame Up"
	up_button.disabled = (index == 0)  # Disable for first frame
	up_button.pressed.connect(_on_move_up_pressed.bind(index))
	container.add_child(up_button)
	
	# Create down button
	var down_button = Button.new()
	down_button.text = "↓"
	down_button.tooltip_text = "Move Frame Down"
	down_button.disabled = (index == frames.size() - 1)  # Disable for last frame
	down_button.pressed.connect(_on_move_down_pressed.bind(index))
	container.add_child(down_button)
	
	# Create frame button
	var frame_button = Button.new()
	
	# Set button text based on frame content
	var button_text = "Frame " + str(index+1)
	if frame.has("title") and frame.title.strip_edges() != "":
		button_text += ": " + frame.title
	
	frame_button.text = button_text
	frame_button.pressed.connect(_on_frame_button_pressed.bind(index))
	frame_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	# Add right-click menu
	frame_button.context_menu_enabled = true
	frame_button.gui_input.connect(_on_frame_button_gui_input.bind(index))
	
	container.add_child(frame_button)
	
	return container

func _on_frame_button_pressed(index):
	current_frame_index = index
	
	if current_frame_index >= 0 and current_frame_index < frames.size():
		if frame_editor:
			frame_editor.load_frame(frames[current_frame_index])
	else:
		if frame_editor:
			frame_editor.visible = false

func _on_frame_updated(frame_data):
	if current_frame_index >= 0 and current_frame_index < frames.size():
		frames[current_frame_index] = frame_data
		
		# Update frame button text
		if frame_list_container.get_child_count() > current_frame_index:
			var container = frame_list_container.get_child(current_frame_index)
			var frame_button = container.get_child(2) if container.get_child_count() > 2 else null
			
			if frame_button:
				var button_text = "Frame " + str(current_frame_index+1)
				if frame_data.has("title") and frame_data.title.strip_edges() != "":
					button_text += ": " + frame_data.title
				frame_button.text = button_text

func _on_name_changed(new_name):
	if current_cutscene:
		current_cutscene.name = new_name

func _on_typing_speed_changed(new_value):
	if current_cutscene:
		current_cutscene.typing_speed = new_value

func _on_background_color_changed(new_color):
	if current_cutscene:
		current_cutscene.background_color = new_color.to_html()

func _on_use_bbcode_toggled(button_pressed):
	if current_cutscene:
		current_cutscene.use_bbcode = button_pressed
		print("BBCode toggled: ", button_pressed)

func _on_theme_selected(index):
	if current_cutscene:
		current_cutscene.theme = theme_option_button.get_item_text(index)
		print("Theme selected: ", current_cutscene.theme)

func _on_browse_image_pressed():
	background_image_dialog.popup_centered()

func _on_background_image_selected(path):
	if current_cutscene:
		current_cutscene.background_image = path
		background_image_edit.text = path

func _on_browse_music_pressed():
	music_file_dialog.popup_centered()

func _on_music_file_selected(path):
	if current_cutscene:
		current_cutscene.music_path = path
		music_path_edit.text = path

func _on_add_frame_pressed():
	if current_cutscene:
		var new_frame = CutsceneFrameResource.new()
		new_frame.title = "New Frame"
		new_frame.text = "Enter text here..."
		new_frame.text_color = Color.WHITE
		new_frame.style = "default"
		new_frame.text_anchor = "bottom"
		new_frame.image_anchor = "center"
		new_frame.background_image = ""
		new_frame.voiceover_path = ""
		new_frame.theme_override = "default"
		
		frames.append(new_frame)
		
		# Update frame list
		_load_frames()
		
		# Select the new frame
		current_frame_index = frames.size() - 1
		if frame_editor:
			frame_editor.load_frame(frames[current_frame_index])

func _on_delete_frame_pressed():
	_delete_frame(current_frame_index)

func _on_save_pressed():
	if current_cutscene and current_category:
		# Update cutscene frames
		current_cutscene.frames = frames
		
		# Emit signal to update cutscene in registry
		emit_signal("cutscene_updated", current_cutscene)
		
		print("Saving cutscene: ", current_cutscene.name)

func _on_preview_pressed():
	if current_cutscene:
		# Save current cutscene frames
		current_cutscene.frames = frames
		
		# Create a preview window directly
		var preview_window = Window.new()
		preview_window.title = "Cutscene Preview"
		preview_window.size = Vector2(800, 600)
		preview_window.close_requested.connect(func(): preview_window.queue_free())
		
		# Create container for preview content
		var container = VBoxContainer.new()
		container.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT, Control.PRESET_MODE_MINSIZE, 10)
		preview_window.add_child(container)
		
		# Add title
		var title_label = Label.new()
		title_label.text = "Previewing: " + current_cutscene.name
		title_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		container.add_child(title_label)
		
		# Add frame info
		var info_label = Label.new()
		info_label.text = "Total frames: " + str(frames.size())
		info_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		container.add_child(info_label)
		
		# Add duration info
		var duration_label = Label.new()
		var total_duration = 0.0
		for frame in frames:
			var frame_duration = 3.0  # Default
			var duration_mode = frame.duration_mode if frame.duration_mode != null else "auto_voiceover"
			
			if duration_mode == "manual":
				frame_duration = frame.manual_duration if frame.manual_duration != null else 3.0
			elif duration_mode == "auto_text":
				frame_duration = (frame.text.length() if frame.text != null else 0) * current_cutscene.typing_speed
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
		frame_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		scroll.add_child(frame_list)
		
		# Add header row
		var header = HBoxContainer.new()
		header.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		frame_list.add_child(header)
		
		var h1 = Label.new()
		h1.text = "Frame"
		h1.custom_minimum_size.x = 80
		header.add_child(h1)
		
		var h2 = Label.new()
		h2.text = "Title"
		h2.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		header.add_child(h2)
		
		var h3 = Label.new()
		h3.text = "Duration Mode"
		h3.custom_minimum_size.x = 150
		header.add_child(h3)
		
		var h4 = Label.new()
		h4.text = "Duration"
		h4.custom_minimum_size.x = 100
		header.add_child(h4)
		
		# Add separator
		var separator = HSeparator.new()
		frame_list.add_child(separator)
		
		# Add frame rows
		for i in range(frames.size()):
			var frame = frames[i]
			var frame_container = HBoxContainer.new()
			frame_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
			frame_list.add_child(frame_container)
			
			var frame_number = Label.new()
			frame_number.text = str(i+1)
			frame_number.custom_minimum_size.x = 80
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
				var text_duration = (frame.text.length() if frame.text != null else 0) * current_cutscene.typing_speed
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
		close_button.pressed.connect(func(): preview_window.queue_free())
		button_container.add_child(close_button)
		
		# Show the window
		get_tree().root.add_child(preview_window)
		preview_window.popup_centered()

func _on_move_up_pressed(index):
	_move_frame_up(index)

func _on_move_down_pressed(index):
	_move_frame_down(index)

func _move_frame_up(index):
	if index <= 0 or index >= frames.size():
		return
		
	# Swap frames
	var temp = frames[index]
	frames[index] = frames[index - 1]
	frames[index - 1] = temp
	
	# Update UI
	_load_frames()
	
	# Update current frame index
	current_frame_index = index - 1
	
	# Update frame editor
	if frame_editor:
		frame_editor.load_frame(frames[current_frame_index])

func _move_frame_down(index):
	if index < 0 or index >= frames.size() - 1:
		return
		
	# Swap frames
	var temp = frames[index]
	frames[index] = frames[index + 1]
	frames[index + 1] = temp
	
	# Update UI
	_load_frames()
	
	# Update current frame index
	current_frame_index = index + 1
	
	# Update frame editor
	if frame_editor:
		frame_editor.load_frame(frames[current_frame_index])

func _duplicate_frame(index):
	if index < 0 or index >= frames.size():
		return
		
	# Create a deep copy of the frame
	var original_frame = frames[index]
	var new_frame = original_frame.duplicate(true)
	
	# Insert after the current frame
	frames.insert(index + 1, new_frame)
	
	# Update UI
	_load_frames()
	
	# Select the new frame
	current_frame_index = index + 1
	if frame_editor:
		frame_editor.load_frame(frames[current_frame_index])

func _delete_frame(index):
	if index < 0 or index >= frames.size():
		return
		
	frames.remove_at(index)
	
	# Update frame list
	_load_frames()
	
	# Reset current frame index if needed
	if current_frame_index >= frames.size():
		current_frame_index = max(0, frames.size() - 1)
	
	# Update frame editor if there are still frames
	if frames.size() > 0 and current_frame_index >= 0:
		if frame_editor:
			frame_editor.load_frame(frames[current_frame_index])
	else:
		if frame_editor:
			frame_editor.visible = false

func _on_frame_button_gui_input(event, index):
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_RIGHT and event.pressed:
		# Create popup menu
		var popup = PopupMenu.new()
		popup.add_item("Move Up", 0)
		popup.add_item("Move Down", 1)
		popup.add_separator()
		popup.add_item("Duplicate Frame", 2)
		popup.add_separator()
		popup.add_item("Delete Frame", 3)
		
		# Connect signal
		popup.id_pressed.connect(_on_frame_popup_menu_item_selected.bind(index))
		
		# Show popup
		add_child(popup)
		popup.position = get_global_mouse_position()
		popup.popup()

func _on_frame_popup_menu_item_selected(id, index):
	match id:
		0: # Move Up
			_move_frame_up(index)
		1: # Move Down
			_move_frame_down(index)
		2: # Duplicate Frame
			_duplicate_frame(index)
		3: # Delete Frame
			_delete_frame(index)

func _on_music_volume_changed(value):
	if current_cutscene:
		current_cutscene.music_volume = value
		print("Music volume changed: ", value)

func _on_voiceover_volume_changed(value):
	if current_cutscene:
		current_cutscene.voiceover_volume = value
		print("Voiceover volume changed: ", value)
