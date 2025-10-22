@tool
extends EditorInspectorPlugin

# This plugin adds custom property editors for CutsceneResource and CutsceneFrameResource
# to ensure all properties are properly labeled in the inspector

const CutsceneResource = preload("res://addons/cutscene_editor/resources/CutsceneResource.gd")
const CutsceneFrameResource = preload("res://addons/cutscene_editor/resources/CutsceneFrameResource.gd")
const SettingsManager = preload("res://addons/cutscene_editor/settings/settings_manager.gd")

var settings_manager = SettingsManager.new()

func _can_handle(object):
	return object is CutsceneResource or object is CutsceneFrameResource

func _parse_property(object, type, name, hint_type, hint_string, usage_flags, wide):
	# Add custom labels and editors for specific properties
	
	# Handle frame_image property
	if name == "frame_image":
		var container = VBoxContainer.new()
		
		# Add a label
		var label = Label.new()
		label.text = "Frame Image"
		label.tooltip_text = "Path to the frame image (PNG or JPG)"
		container.add_child(label)
		
		# Add the file path edit
		var hbox = HBoxContainer.new()
		hbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		container.add_child(hbox)
		
		var edit = LineEdit.new()
		edit.text = object.get(name)
		edit.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		edit.text_changed.connect(_on_text_changed.bind(object, name))
		hbox.add_child(edit)
		
		var browse_btn = Button.new()
		browse_btn.text = "Browse"
		browse_btn.pressed.connect(_on_custom_browse_pressed.bind(
			edit, 
			object, 
			name, 
			["png", "jpg", "jpeg", "webp"],
			"Select Frame Image"
		))
		hbox.add_child(browse_btn)
		
		add_custom_control(container)
		return true
	
	# Handle background_image property
	elif name == "background_image":
		var container = VBoxContainer.new()
		
		# Add a label
		var label = Label.new()
		label.text = "Background Image"
		label.tooltip_text = "Path to the background image (PNG or JPG)"
		container.add_child(label)
		
		# Add the file path edit
		var hbox = HBoxContainer.new()
		hbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		container.add_child(hbox)
		
		var edit = LineEdit.new()
		edit.text = object.get(name)
		edit.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		edit.text_changed.connect(_on_text_changed.bind(object, name))
		hbox.add_child(edit)
		
		var browse_btn = Button.new()
		browse_btn.text = "Browse"
		browse_btn.pressed.connect(_on_custom_browse_pressed.bind(
			edit, 
			object, 
			name, 
			["png", "jpg", "jpeg", "webp"],
			"Select Background Image"
		))
		hbox.add_child(browse_btn)
		
		add_custom_control(container)
		return true
	
	# Handle music_path property
	elif name == "music_path":
		var container = VBoxContainer.new()
		
		# Add a label
		var label = Label.new()
		label.text = "Background Music"
		label.tooltip_text = "Path to background music (MP3, OGG, or WAV)"
		container.add_child(label)
		
		# Add the file path edit
		var hbox = HBoxContainer.new()
		hbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		container.add_child(hbox)
		
		var edit = LineEdit.new()
		edit.text = object.get(name)
		edit.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		edit.text_changed.connect(_on_text_changed.bind(object, name))
		hbox.add_child(edit)
		
		var browse_btn = Button.new()
		browse_btn.text = "Browse"
		browse_btn.pressed.connect(_on_custom_browse_pressed.bind(
			edit, 
			object, 
			name, 
			["mp3", "ogg", "wav"],
			"Select Background Music"
		))
		hbox.add_child(browse_btn)
		
		add_custom_control(container)
		return true
	
	# Handle voiceover_path property
	elif name == "voiceover_path":
		var container = VBoxContainer.new()
		
		# Add a label
		var label = Label.new()
		label.text = "Voiceover Audio"
		label.tooltip_text = "Path to voice audio (MP3, OGG, or WAV)"
		container.add_child(label)
		
		# Add the file path edit
		var hbox = HBoxContainer.new()
		hbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		container.add_child(hbox)
		
		var edit = LineEdit.new()
		edit.text = object.get(name)
		edit.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		edit.text_changed.connect(_on_text_changed.bind(object, name))
		hbox.add_child(edit)
		
		var browse_btn = Button.new()
		browse_btn.text = "Browse"
		browse_btn.pressed.connect(_on_custom_browse_pressed.bind(
			edit, 
			object, 
			name, 
			["mp3", "ogg", "wav"],
			"Select Voiceover Audio"
		))
		hbox.add_child(browse_btn)
		
		add_custom_control(container)
		return true
	
	# Let the default inspector handle other properties
	return false

func _on_text_changed(new_text, object, property_name):
	# Update the property in the object
	object.set(property_name, new_text)

func _on_custom_browse_pressed(edit, object, property_name, extensions, dialog_title):
	# Get appropriate directory from settings based on property type
	var settings = settings_manager.get_settings()
	var start_path = "res://"
	
	if property_name == "frame_image" or property_name == "background_image":
		start_path = settings.images_dir if settings else "res://assets/images/cutscenes"
	elif property_name == "music_path":
		start_path = settings.music_dir if settings else "res://assets/audio/music"
	elif property_name == "voiceover_path":
		start_path = settings.voiceover_dir if settings else "res://assets/audio/voice"
	
	# Create and show our custom file browser
	var browser = FileBrowserDialog.new(
		dialog_title,
		extensions,
		start_path,
		func(path):
			edit.text = path
			object.set(property_name, path)
	)
	browser.show()

# Custom file browser dialog that doesn't use resource loading
class FileBrowserDialog:
	var window: Window
	var tree: Tree
	var path_label: Label
	var filter_extensions: Array
	var current_path: String
	var callback_function: Callable
	var root_item: TreeItem
	
	func _init(title: String, extensions: Array, start_path: String, callback: Callable):
		filter_extensions = extensions
		current_path = start_path
		callback_function = callback
		
		# Create window with larger size
		window = Window.new()
		window.title = title
		window.size = Vector2(800, 600)
		window.exclusive = true
		window.unresizable = false
		
		# Create main container
		var main_container = VBoxContainer.new()
		main_container.anchors_preset = Control.PRESET_FULL_RECT
		main_container.size_flags_vertical = Control.SIZE_EXPAND_FILL
		window.add_child(main_container)
		
		# Create path display with better styling
		var path_container = HBoxContainer.new()
		path_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		main_container.add_child(path_container)
		
		var path_label_prefix = Label.new()
		path_label_prefix.text = "Current path: "
		path_container.add_child(path_label_prefix)
		
		path_label = Label.new()
		path_label.text = current_path
		path_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		path_container.add_child(path_label)
		
		# Add separator
		var separator = HSeparator.new()
		main_container.add_child(separator)
		
		# Create tree with proper expansion
		tree = Tree.new()
		tree.size_flags_vertical = Control.SIZE_EXPAND_FILL
		tree.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		tree.custom_minimum_size = Vector2(0, 400)
		tree.allow_rmb_select = false
		tree.hide_root = true
		main_container.add_child(tree)
		
		# Add another separator
		var separator2 = HSeparator.new()
		main_container.add_child(separator2)
		
		# Create buttons
		var button_container = HBoxContainer.new()
		button_container.alignment = BoxContainer.ALIGNMENT_END
		button_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		main_container.add_child(button_container)
		
		var cancel_button = Button.new()
		cancel_button.text = "Cancel"
		cancel_button.pressed.connect(func(): window.queue_free())
		button_container.add_child(cancel_button)
		
		# Add some spacing
		var spacer = Control.new()
		spacer.custom_minimum_size.x = 10
		button_container.add_child(spacer)
		
		var select_button = Button.new()
		select_button.text = "Select"
		select_button.pressed.connect(func():
			var selected = tree.get_selected()
			if selected and selected.get_metadata(0) is String:
				var path = selected.get_metadata(0)
				callback_function.call(path)
			window.queue_free()
		)
		button_container.add_child(select_button)
		
		# Connect tree signals
		tree.item_selected.connect(func():
			var selected = tree.get_selected()
			if selected:
				var meta = selected.get_metadata(0)
				if meta is String and _is_file_with_extension(meta, filter_extensions):
					select_button.disabled = false
				else:
					select_button.disabled = true
		)
		
		tree.item_activated.connect(func():
			var selected = tree.get_selected()
			if selected:
				var meta = selected.get_metadata(0)
				if meta is String:
					if _is_file_with_extension(meta, filter_extensions):
						callback_function.call(meta)
						window.queue_free()
					elif DirAccess.dir_exists_absolute(meta):
						current_path = meta
						_refresh_tree()
		)
		
		# Initial refresh
		_refresh_tree()
	
	func _is_file_with_extension(path: String, extensions: Array) -> bool:
		if extensions.is_empty():
			return true
		
		for ext in extensions:
			if path.ends_with("." + ext):
				return true
		
		return false
	
	func _refresh_tree():
		tree.clear()
		root_item = tree.create_item()
		path_label.text = current_path
		
		# Add parent directory option
		if current_path != "res://":
			var parent_dir = current_path.get_base_dir()
			var parent_item = tree.create_item(root_item)
			parent_item.set_text(0, "..")
			parent_item.set_icon(0, _get_folder_icon())
			parent_item.set_metadata(0, parent_dir)
		
		# List directories first
		var dir = DirAccess.open(current_path)
		if dir:
			dir.list_dir_begin()
			var file_name = dir.get_next()
			
			# Add directories
			while file_name != "":
				if dir.current_is_dir() and not file_name.begins_with("."):
					var dir_path = current_path.path_join(file_name)
					var item = tree.create_item(root_item)
					item.set_text(0, file_name)
					item.set_icon(0, _get_folder_icon())
					item.set_metadata(0, dir_path)
				file_name = dir.get_next()
			
			# Reset and add files
			dir.list_dir_begin()
			file_name = dir.get_next()
			while file_name != "":
				if not dir.current_is_dir():
					var is_valid = false
					for ext in filter_extensions:
						if file_name.ends_with("." + ext):
							is_valid = true
							break
					
					if is_valid:
						var file_path = current_path.path_join(file_name)
						var item = tree.create_item(root_item)
						item.set_text(0, file_name)
						item.set_icon(0, _get_file_icon())
						item.set_metadata(0, file_path)
				file_name = dir.get_next()
	
	func _get_folder_icon():
		var theme = ThemeDB.get_default_theme()
		return theme.get_icon("Folder", "EditorIcons")
	
	func _get_file_icon():
		var theme = ThemeDB.get_default_theme()
		return theme.get_icon("File", "EditorIcons")
	
	func show():
		var parent = Engine.get_main_loop().root
		parent.add_child(window)
		window.popup_centered()
