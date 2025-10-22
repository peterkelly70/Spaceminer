@tool
extends Control
class_name CutsceneEditorDock

# Import required classes
const SettingsManager = preload("res://addons/cutscene_editor/settings/settings_manager.gd")
const CutsceneEditorSettings = preload("res://addons/cutscene_editor/resources/CutsceneEditorSettings.gd")
const CutsceneResource = preload("res://addons/cutscene_editor/resources/CutsceneResource.gd")
const CutsceneFrameResource = preload("res://addons/cutscene_editor/resources/CutsceneFrameResource.gd")
const PropertyHints = preload("res://addons/cutscene_editor/resources/CutsceneEditorPropertyHints.gd")

signal resource_selected(resource: Resource)
signal settings_requested

@onready var tree: Tree = $VBoxContainer/CutsceneTree
@onready var menu_button: MenuButton = $VBoxContainer/HeaderButtons/MenuButton
@onready var context_menu: PopupMenu = $ContextMenu

var current_cutscene: CutsceneResource = null
var tree_root
var settings: CutsceneEditorSettings
var selected_item = null
var rename_timer: Timer = null
var editor_interface = null

# Menu IDs
enum MenuAction {
	NEW_CUTSCENE = 0,
	LOAD_CUTSCENE = 1,
	ADD_FRAME = 2,
	SAVE = 3,
	SETTINGS = 4
}

# Context Menu IDs
enum ContextAction {
	RENAME = 0,
	DELETE = 1,
	DUPLICATE = 2,
	ADD_FRAME = 3,
	TEST_CUTSCENE = 4
}

func _ready():
	# Initialize UI
	tree.clear()
	tree_root = tree.create_item()
	
	# Load settings
	var settings_script = load("res://addons/cutscene_editor/settings/settings_manager.gd")
	settings = settings_script.new().get_settings()
	
	# Setup menu
	var popup = menu_button.get_popup()
	popup.clear()
	popup.add_item("New Cutscene", MenuAction.NEW_CUTSCENE)
	popup.add_item("Load Cutscene", MenuAction.LOAD_CUTSCENE)
	popup.add_separator()
	popup.add_item("Add Frame", MenuAction.ADD_FRAME)
	popup.add_separator()
	popup.add_item("Save", MenuAction.SAVE)
	popup.add_separator()
	popup.add_item("Settings", MenuAction.SETTINGS)
	
	# Setup context menu
	context_menu.clear()
	context_menu.add_item("Rename", ContextAction.RENAME)
	context_menu.add_item("Delete", ContextAction.DELETE)
	context_menu.add_item("Duplicate", ContextAction.DUPLICATE)
	context_menu.add_separator()
	context_menu.add_item("Add Frame", ContextAction.ADD_FRAME)
	context_menu.add_separator()
	context_menu.add_item("Test Cutscene", ContextAction.TEST_CUTSCENE)
	
	# Connect signals - check if already connected to avoid errors
	if !popup.id_pressed.is_connected(_on_menu_id_pressed):
		popup.id_pressed.connect(_on_menu_id_pressed)
	
	if !context_menu.id_pressed.is_connected(_on_context_menu_id_pressed):
		context_menu.id_pressed.connect(_on_context_menu_id_pressed)
	
	if !tree.item_selected.is_connected(_on_tree_item_selected):
		tree.item_selected.connect(_on_tree_item_selected)
	
	if !tree.item_mouse_selected.is_connected(_on_tree_item_mouse_selected):
		tree.item_mouse_selected.connect(_on_tree_item_mouse_selected)
	
	if !tree.item_edited.is_connected(_on_item_edited):
		tree.item_edited.connect(_on_item_edited)
	
	# Setup rename timer (for double-click detection)
	rename_timer = Timer.new()
	rename_timer.one_shot = true
	rename_timer.wait_time = 0.5
	add_child(rename_timer)
	
	# Register custom property editors
	_register_custom_property_editors()

func set_editor_interface(interface):
	editor_interface = interface

func _register_custom_property_editors():
	# We'll register our custom property editors with the inspector
	# This will be called from the plugin script
	pass

func _input(event):
	# Handle F2 key for renaming (like scene editor)
	if event is InputEventKey and event.keycode == KEY_F2 and event.pressed and not event.echo:
		if selected_item:
			_start_rename(selected_item)
			get_viewport().set_input_as_handled()

func _on_menu_id_pressed(id: int) -> void:
	match id:
		MenuAction.NEW_CUTSCENE:
			_on_new_cutscene()
		MenuAction.LOAD_CUTSCENE:
			_on_load_cutscene()
		MenuAction.ADD_FRAME:
			_on_add_frame()
		MenuAction.SAVE:
			_on_save_cutscene()
		MenuAction.SETTINGS:
			emit_signal("settings_requested")

func _on_context_menu_id_pressed(id: int) -> void:
	if not selected_item:
		return
		
	var meta = selected_item.get_metadata(0)
	if not meta:
		return
		
	match id:
		ContextAction.RENAME:
			_start_rename(selected_item)
		ContextAction.DELETE:
			_delete_item(selected_item)
		ContextAction.DUPLICATE:
			_duplicate_item(selected_item)
		ContextAction.ADD_FRAME:
			if meta is CutsceneResource:
				_on_add_frame()
		ContextAction.TEST_CUTSCENE:
			if meta is CutsceneResource:
				_test_cutscene(meta)
			elif meta is CutsceneFrameResource:
				# If a frame is selected, find its parent cutscene
				var parent_item = selected_item.get_parent()
				if parent_item:
					var parent_meta = parent_item.get_metadata(0)
					if parent_meta is CutsceneResource:
						_test_cutscene(parent_meta)

func _on_tree_item_mouse_selected(position: Vector2, mouse_button_index: int) -> void:
	if mouse_button_index == MOUSE_BUTTON_RIGHT:
		selected_item = tree.get_selected()
		if selected_item:
			# Show context menu at mouse position
			context_menu.position = get_global_mouse_position()
			context_menu.popup()
	elif mouse_button_index == MOUSE_BUTTON_LEFT:
		# Handle double-click for rename (like scene editor)
		if rename_timer and not rename_timer.is_stopped():
			selected_item = tree.get_selected()
			if selected_item:
				_start_rename(selected_item)
		else:
			if rename_timer:
				rename_timer.start()

func _on_new_cutscene():
	current_cutscene = CutsceneResource.new()
	current_cutscene.name = "New Cutscene"
	
	# Apply default settings
	current_cutscene.typing_speed = settings.default_typing_speed
	
	_refresh_tree()

func _on_load_cutscene():
	# Close any existing dialogs to prevent exclusive window errors
	for child in get_children():
		if child is AcceptDialog and child.visible:
			child.hide()
			child.queue_free()
			
	# Create a file dialog for loading cutscenes
	var dialog = FileDialog.new()
	dialog.title = "Load Cutscene"
	dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	dialog.access = FileDialog.ACCESS_RESOURCES
	dialog.filters = PackedStringArray(["*.tres ; Cutscene Resources"])
	dialog.exclusive = false  # Make non-exclusive to avoid conflicts
	
	# Set initial directory to the cutscenes directory from settings
	if settings and settings.cutscenes_dir:
		dialog.current_dir = settings.cutscenes_dir
	
	# Connect signals
	dialog.file_selected.connect(func(path):
		var resource = ResourceLoader.load(path)
		if resource is CutsceneResource:
			current_cutscene = resource
			_refresh_tree()
			print("Loaded cutscene from: " + path)
		else:
			push_error("Selected file is not a valid CutsceneResource: " + path)
		dialog.queue_free()
	)
	dialog.canceled.connect(func(): dialog.queue_free())
	
	# Show dialog
	add_child(dialog)
	dialog.popup_centered(Vector2(800, 600))

func _on_add_frame():
	if current_cutscene == null:
		return
		
	var frame := CutsceneFrameResource.new()
	
	# Apply default settings
	frame.title = "New Frame " + str(current_cutscene.frames.size() + 1)
	frame.text_color = settings.default_text_color
	frame.style = settings.default_text_style
	frame.text_anchor = settings.default_text_anchor
	
	current_cutscene.frames.append(frame)
	_refresh_tree()

func _on_save_cutscene():
	if current_cutscene == null:
		return
		
	# Use settings to get the proper path
	var filename = current_cutscene.name
	var path = settings.cutscenes_dir.path_join(filename + ".tres")
	
	# Create directory if it doesn't exist
	var dir = DirAccess.open("res://")
	if dir:
		var dir_path = settings.cutscenes_dir.replace("res://", "")
		if !dir.dir_exists(dir_path):
			dir.make_dir_recursive(dir_path)
	
	# Save the resource
	var error = ResourceSaver.save(current_cutscene, path)
	if error == OK:
		print("Saved cutscene to:" + path)
		
		# Close any existing notifications to prevent exclusive window errors
		for child in get_children():
			if child is AcceptDialog and child.visible:
				child.hide()
				child.queue_free()
		
		# Show a notification to the user
		var notification = AcceptDialog.new()
		notification.title = "Cutscene Saved"
		notification.dialog_text = "Cutscene saved to:\n" + path
		notification.exclusive = false  # Make non-exclusive to avoid conflicts
		add_child(notification)
		notification.popup_centered()
		
		# Auto-close after 3 seconds
		var timer = Timer.new()
		timer.wait_time = 3.0
		timer.one_shot = true
		notification.add_child(timer)
		timer.timeout.connect(func(): notification.hide(); notification.queue_free())
		timer.start()
	else:
		push_error("Failed to save cutscene. Error code: " + str(error))

func _refresh_tree():
	tree.clear()
	tree_root = tree.create_item()
	if current_cutscene:
		var cutscene_item = tree.create_item(tree_root)
		cutscene_item.set_text(0, current_cutscene.name)
		cutscene_item.set_metadata(0, current_cutscene)

		for i in current_cutscene.frames.size():
			var frame = current_cutscene.frames[i]
			var frame_item = tree.create_item(cutscene_item)
			frame_item.set_text(0, frame.title)
			frame_item.set_metadata(0, frame)

		# In Godot 4, Tree no longer has an update() method

func _on_tree_item_selected():
	var item = tree.get_selected()
	if item:
		selected_item = item
		var meta = item.get_metadata(0)
		if meta and meta is Resource:
			emit_signal("resource_selected", meta)

func _start_rename(item):
	if not item:
		return
	
	# Make the item editable and start editing
	item.set_editable(0, true)
	tree.edit_selected()

func _on_item_edited():
	var item = tree.get_edited()
	if not item:
		return
	
	# Get the new name
	var new_name = item.get_text(0)
	if new_name.is_empty():
		# Reset to original name if empty
		var meta = item.get_metadata(0)
		if meta is CutsceneResource:
			item.set_text(0, meta.name)
		elif meta is CutsceneFrameResource:
			item.set_text(0, meta.title)
		return
	
	# Update the resource name
	var meta = item.get_metadata(0)
	if meta is CutsceneResource:
		meta.name = new_name
	elif meta is CutsceneFrameResource:
		meta.title = new_name
	
	# Make item non-editable again
	item.set_editable(0, false)
	
	# Save changes
	_on_save_cutscene()

func _delete_item(item):
	var meta = item.get_metadata(0)
	if not meta:
		return
	
	# Create confirmation dialog
	var dialog = ConfirmationDialog.new()
	dialog.title = "Delete Confirmation"
	
	if meta is CutsceneResource:
		# For cutscenes, offer to delete the file too
		var cutscene_path = settings.cutscenes_dir.path_join(meta.name + ".tres")
		
		# Create a custom container for better layout
		var content = VBoxContainer.new()
		content.add_theme_constant_override("separation", 10)
		
		# Add message label
		var message_label = Label.new()
		message_label.text = "Delete cutscene '" + meta.name + "'?"
		message_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		content.add_child(message_label)
		
		# Add spacer
		var spacer = Control.new()
		spacer.custom_minimum_size.y = 10
		content.add_child(spacer)
		
		# Add checkbox with proper styling
		var delete_file_check = CheckBox.new()
		delete_file_check.text = "Also delete cutscene file from disk"
		delete_file_check.button_pressed = true
		delete_file_check.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		delete_file_check.alignment = HORIZONTAL_ALIGNMENT_CENTER
		content.add_child(delete_file_check)
		
		# Add file path info
		var path_label = Label.new()
		path_label.text = "File: " + cutscene_path
		path_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		path_label.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
		path_label.add_theme_font_size_override("font_size", 12)
		content.add_child(path_label)
		
		# Add content to dialog
		dialog.add_child(content)
		
		# Set dialog size
		dialog.min_size = Vector2(400, 180)
		
		# Connect to confirmed signal with the checkbox state
		dialog.confirmed.connect(func():
			var should_delete_file = delete_file_check.button_pressed
			_on_delete_confirmed(item, should_delete_file)
			dialog.queue_free()
		)
	else:
		# For frames, use the standard confirmation
		dialog.dialog_text = "Delete selected item?"
		
		# Connect to confirmed signal
		dialog.confirmed.connect(func():
			_on_delete_confirmed(item, false)
			dialog.queue_free()
		)
	
	# Connect to canceled signal
	dialog.canceled.connect(func():
		dialog.queue_free()
	)
	
	# Show dialog
	add_child(dialog)
	dialog.popup_centered()

func _on_delete_confirmed(item, delete_file = false):
	var meta = item.get_metadata(0)
	if not meta:
		return
		
	if meta is CutsceneResource:
		# If requested, delete the file from disk
		if delete_file:
			var cutscene_path = settings.cutscenes_dir.path_join(meta.name + ".tres")
			if FileAccess.file_exists(cutscene_path):
				var dir = DirAccess.open("res://")
				if dir:
					var err = dir.remove(cutscene_path.replace("res://", ""))
					if err == OK:
						print("Deleted cutscene file: ", cutscene_path)
					else:
						push_warning("Failed to delete cutscene file: " + cutscene_path + ", error: " + str(err))
		
		# Clear the current cutscene
		current_cutscene = null
	elif meta is CutsceneFrameResource:
		# Find and remove the frame from the cutscene
		if current_cutscene:
			var frames = current_cutscene.frames
			var frame_index = -1
			
			# Find the frame index
			for i in range(frames.size()):
				if frames[i] == meta:
					frame_index = i
					break
					
			if frame_index >= 0:
				frames.remove_at(frame_index)
	
	# Remove the item from the tree
	item.free()
	
	# Save changes
	_on_save_cutscene()

func _duplicate_item(item):
	var meta = item.get_metadata(0)
	if not meta:
		return
		
	if meta is CutsceneResource:
		# Duplicate the cutscene
		var new_cutscene = meta.duplicate(true)
		new_cutscene.name = meta.name + " (Copy)"
		current_cutscene = new_cutscene
	elif meta is CutsceneFrameResource:
		# Duplicate the frame
		var parent_item = item.get_parent()
		var parent_meta = parent_item.get_metadata(0)
		
		if parent_meta is CutsceneResource:
			var new_frame = meta.duplicate(true)
			new_frame.title = meta.title + " (Copy)"
			parent_meta.frames.append(new_frame)
	
	# Refresh the tree
	_refresh_tree()
	
	# Save changes
	_on_save_cutscene()

# Custom property editing
func show_file_dialog_for_property(resource, property_name, title, filters):
	var dialog = PropertyHints.get_file_dialog(
		title,
		FileDialog.FILE_MODE_OPEN_FILE,
		filters,
		func(path): 
			resource.set(property_name, path)
			_on_save_cutscene()
	)
	add_child(dialog)
	dialog.popup_centered()

func show_dir_dialog_for_property(resource, property_name, title):
	var dialog = PropertyHints.get_file_dialog(
		title,
		FileDialog.FILE_MODE_OPEN_DIR,
		PackedStringArray(),
		func(path): 
			resource.set(property_name, path)
			_on_save_cutscene()
	)
	add_child(dialog)
	dialog.popup_centered()

func show_option_dialog_for_property(resource, property_name, options, title):
	# Create dialog
	var dialog = AcceptDialog.new()
	dialog.title = title
	
	# Create option button
	var vbox = VBoxContainer.new()
	dialog.add_child(vbox)
	
	var option_button = PropertyHints.get_option_button(
		options,
		resource.get(property_name),
		func(value): 
			resource.set(property_name, value)
			_on_save_cutscene()
			dialog.hide()
	)
	vbox.add_child(option_button)
	
	# Show dialog
	add_child(dialog)
	dialog.popup_centered()

func show_color_picker_for_property(resource, property_name, title):
	# Create dialog
	var dialog = AcceptDialog.new()
	dialog.title = title
	
	# Create color picker
	var vbox = VBoxContainer.new()
	dialog.add_child(vbox)
	
	var color_picker = ColorPicker.new()
	color_picker.color = resource.get(property_name)
	color_picker.color_changed.connect(func(color): 
		resource.set(property_name, color)
	)
	vbox.add_child(color_picker)
	
	# Connect to confirmed signal
	dialog.connect("confirmed", Callable(self, "_on_save_cutscene"))
	
	# Show dialog
	add_child(dialog)
	dialog.popup_centered()

func show_bbcode_preview(resource, parent_cutscene = null):
	# Create a preview dialog
	var dialog = AcceptDialog.new()
	dialog.title = "Text Preview"
	dialog.min_size = Vector2(500, 300)
	
	var vbox = VBoxContainer.new()
	dialog.add_child(vbox)
	
	var rich_text = RichTextLabel.new()
	rich_text.size_flags_vertical = Control.SIZE_EXPAND_FILL
	rich_text.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	rich_text.custom_minimum_size = Vector2(480, 250)
	
	# For frame resources, we need to check the parent cutscene for BBCode setting
	var use_bbcode = false
	
	if resource is CutsceneResource:
		use_bbcode = resource.use_bbcode
	elif resource is CutsceneFrameResource and parent_cutscene:
		use_bbcode = parent_cutscene.use_bbcode
	
	rich_text.bbcode_enabled = use_bbcode
	
	if resource is CutsceneFrameResource:
		if use_bbcode:
			rich_text.text = resource.text
		else:
			rich_text.text = resource.text
		
		rich_text.add_theme_color_override("default_color", resource.text_color)
	
	vbox.add_child(rich_text)
	
	# Show dialog
	add_child(dialog)
	dialog.popup_centered()

func toggle_bbcode(resource, value):
	if resource is CutsceneResource:
		resource.use_bbcode = value
		_on_save_cutscene()

func show_expanded_text_editor(resource):
	# Create a dialog with a text editor
	var dialog = AcceptDialog.new()
	dialog.title = "Expanded Text Editor"
	dialog.min_size = Vector2(500, 300)
	
	var vbox = VBoxContainer.new()
	dialog.add_child(vbox)
	
	var text_editor = TextEdit.new()
	text_editor.size_flags_vertical = Control.SIZE_EXPAND_FILL
	text_editor.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	text_editor.custom_minimum_size = Vector2(480, 250)
	
	if resource is CutsceneFrameResource:
		text_editor.text = resource.text
	
	text_editor.connect("text_changed", func(): 
		if resource is CutsceneFrameResource:
			resource.text = text_editor.text
			_on_save_cutscene()
	)
	
	vbox.add_child(text_editor)
	
	# Show dialog
	add_child(dialog)
	dialog.popup_centered()

# Custom inspector integration
class CutsceneEditorInspectorPlugin extends EditorInspectorPlugin:
	var dock: CutsceneEditorDock
	
	func _init(p_dock: CutsceneEditorDock):
		dock = p_dock
	
	func _can_handle(object):
		return object is CutsceneResource or object is CutsceneFrameResource
	
	func _parse_property(object, type, name, hint_type, hint_string, usage_flags, wide):
		# Hide the frames array in the inspector
		if object is CutsceneResource and name == "frames":
			return true  # Skip this property
			
		# Handle CutsceneResource properties
		if object is CutsceneResource:
			if name == "music_path":
				# Create a file picker property
				var editor = EditorResourcePicker.new()
				editor.base_type = "AudioStream"
				editor.edited_resource = load(object.music_path) if object.music_path and ResourceLoader.exists(object.music_path) else null
				editor.resource_changed.connect(func(res):
					if res:
						object.music_path = res.resource_path
					else:
						object.music_path = ""
					dock._on_save_cutscene()
				)
				add_property_editor(name, editor)
				return true
			elif name == "use_bbcode":
				# Use a checkbox for BBCode toggle - use default editor
				return false
		
		# Handle CutsceneFrameResource properties
		if object is CutsceneFrameResource:
			if name == "background_image":
				# Create a file picker property
				var editor = EditorResourcePicker.new()
				editor.base_type = "Texture2D"
				editor.edited_resource = load(object.background_image) if object.background_image and ResourceLoader.exists(object.background_image) else null
				editor.resource_changed.connect(func(res):
					if res:
						object.background_image = res.resource_path
					else:
						object.background_image = ""
					dock._on_save_cutscene()
				)
				add_property_editor(name, editor)
				return true
			elif name == "voiceover_path":
				# Create a file picker property
				var editor = EditorResourcePicker.new()
				editor.base_type = "AudioStream"
				editor.edited_resource = load(object.voiceover_path) if object.voiceover_path and ResourceLoader.exists(object.voiceover_path) else null
				editor.resource_changed.connect(func(res):
					if res:
						object.voiceover_path = res.resource_path
					else:
						object.voiceover_path = ""
					dock._on_save_cutscene()
				)
				add_property_editor(name, editor)
				return true
			elif name == "text_anchor":
				# Create a dropdown property using OptionButton
				var container = HBoxContainer.new()
				container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				var option_button = OptionButton.new()
				option_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				# Add options
				for i in range(PropertyHints.TEXT_ANCHOR_OPTIONS.size()):
					option_button.add_item(PropertyHints.TEXT_ANCHOR_OPTIONS[i], i)
					if PropertyHints.TEXT_ANCHOR_OPTIONS[i] == object.text_anchor:
						option_button.select(i)
				
				# Connect signal
				option_button.item_selected.connect(func(index): 
					object.text_anchor = PropertyHints.TEXT_ANCHOR_OPTIONS[index]
					dock._on_save_cutscene()
				)
				
				container.add_child(option_button)
				add_custom_control(container)
				return true
			elif name == "image_anchor":
				# Create a dropdown property using OptionButton
				var container = HBoxContainer.new()
				container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				var option_button = OptionButton.new()
				option_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				# Add options
				for i in range(PropertyHints.IMAGE_ANCHOR_OPTIONS.size()):
					option_button.add_item(PropertyHints.IMAGE_ANCHOR_OPTIONS[i], i)
					if PropertyHints.IMAGE_ANCHOR_OPTIONS[i] == object.image_anchor:
						option_button.select(i)
				
				# Connect signal
				option_button.item_selected.connect(func(index): 
					object.image_anchor = PropertyHints.IMAGE_ANCHOR_OPTIONS[index]
					dock._on_save_cutscene()
				)
				
				container.add_child(option_button)
				add_custom_control(container)
				return true
			elif name == "style":
				# Create a dropdown property using OptionButton
				var container = HBoxContainer.new()
				container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				var option_button = OptionButton.new()
				option_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				# Add options
				for i in range(PropertyHints.TEXT_STYLE_OPTIONS.size()):
					option_button.add_item(PropertyHints.TEXT_STYLE_OPTIONS[i], i)
					if PropertyHints.TEXT_STYLE_OPTIONS[i] == object.style:
						option_button.select(i)
				
				# Connect signal
				option_button.item_selected.connect(func(index): 
					object.style = PropertyHints.TEXT_STYLE_OPTIONS[index]
					dock._on_save_cutscene()
				)
				
				container.add_child(option_button)
				add_custom_control(container)
				return true
			elif name == "theme_override":
				# Create a dropdown property using OptionButton
				var container = HBoxContainer.new()
				container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				var option_button = OptionButton.new()
				option_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				# Add options
				var theme_options = ["default", "dark", "light", "fantasy", "sci-fi"]
				for i in range(theme_options.size()):
					option_button.add_item(theme_options[i], i)
					if theme_options[i] == object.theme_override:
						option_button.select(i)
				
				# Connect signal
				option_button.item_selected.connect(func(index): 
					object.theme_override = theme_options[index]
					dock._on_save_cutscene()
				)
				
				container.add_child(option_button)
				add_custom_control(container)
				return true
			elif name == "text_color":
				# Use the default color picker
				return false
			elif name == "text":
				# Add a preview button for text
				var hbox = HBoxContainer.new()
				
				var preview_button = Button.new()
				preview_button.text = "Preview"
				preview_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
				
				# Find the parent cutscene
				var parent_cutscene = null
				var selected_item = dock.tree.get_selected()
				if selected_item and selected_item.get_parent():
					var parent_item = selected_item.get_parent()
					var meta = parent_item.get_metadata(0)
					if meta is CutsceneResource:
						parent_cutscene = meta
				
				preview_button.pressed.connect(func(): dock.show_bbcode_preview(object, parent_cutscene))
				hbox.add_child(preview_button)
				
				# Add an expand button for the text editor
				var expand_button = Button.new()
				expand_button.text = "Expand"
				expand_button.pressed.connect(func(): dock.show_expanded_text_editor(object))
				hbox.add_child(expand_button)
				
				add_custom_control(hbox)
				
				# Don't return true here so the default text editor is still shown
				return false
		
		return false

func _test_cutscene(cutscene: CutsceneResource):
	# First save the cutscene to ensure latest changes are included
	_on_save_cutscene()
	
	# Create a test dialog
	var dialog = AcceptDialog.new()
	dialog.title = "Testing Cutscene"
	dialog.dialog_text = "Testing cutscene: " + cutscene.name
	dialog.min_size = Vector2(800, 600)
	dialog.exclusive = false
	
	# Create a container for the cutscene player
	var container = VBoxContainer.new()
	dialog.add_child(container)
	
	# Create the cutscene player
	var player_scene = load("res://addons/cutscene_editor/cutscene_player.tscn")
	if player_scene:
		var player = player_scene.instantiate()
		player.cutscene_resource = cutscene
		container.add_child(player)
		
		# Add controls
		var controls = HBoxContainer.new()
		controls.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		controls.alignment = BoxContainer.ALIGNMENT_CENTER
		
		var play_button = Button.new()
		play_button.text = "Play"
		play_button.pressed.connect(func(): player.play())
		controls.add_child(play_button)
		
		var pause_button = Button.new()
		pause_button.text = "Pause"
		pause_button.pressed.connect(func(): player.pause())
		controls.add_child(pause_button)
		
		var stop_button = Button.new()
		stop_button.text = "Stop"
		stop_button.pressed.connect(func(): player.stop())
		controls.add_child(stop_button)
		
		var next_button = Button.new()
		next_button.text = "Next Frame"
		next_button.pressed.connect(func(): player.next_frame())
		controls.add_child(next_button)
		
		container.add_child(controls)
		
		# Auto-play the cutscene
		player.play()
	else:
		dialog.dialog_text = "Error: Could not load cutscene player scene."
	
	# Show dialog
	add_child(dialog)
	dialog.popup_centered()
	
	# Connect to close signal
	dialog.close_requested.connect(func(): 
		if player_scene:
			var player = container.get_node_or_null("CutscenePlayer")
			if player:
				player.stop()
		dialog.queue_free()
	)
