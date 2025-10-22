@tool
extends VBoxContainer

signal property_changed(property, value)

# Settings manager
const SettingsManager = preload("res://addons/cutscene_editor/settings/settings_manager.gd")
var settings_manager = SettingsManager.new()

# UI References
@onready var title_edit = $TitleSection/TitleEdit
@onready var show_title_check = $TitleSection/ShowTitleCheck
@onready var text_edit = $TextSection/TextEdit
@onready var frame_image_path_edit = $FrameImageSection/PathHBox/FrameImagePathEdit
@onready var frame_image_browse_button = $FrameImageSection/PathHBox/FrameImageBrowseButton
@onready var sound_path_edit = $SoundSection/PathHBox/SoundPathEdit
@onready var sound_browse_button = $SoundSection/PathHBox/SoundBrowseButton
@onready var continue_text_edit = $ContinueSection/ContinueTextEdit
@onready var background_color_picker = $BackgroundSection/ColorPickerButton
@onready var show_frame_box_check = $FrameBoxSection/ShowFrameBoxCheck

# Current frame data
var current_frame = {}
var is_updating = false

func _ready():
	# Connect signals
	title_edit.text_changed.connect(_on_title_changed)
	if show_title_check:
		show_title_check.toggled.connect(_on_show_title_toggled)
	text_edit.text_changed.connect(_on_text_changed)
	
	# Connect frame image signals if the UI elements exist
	if frame_image_path_edit and frame_image_browse_button:
		frame_image_path_edit.text_changed.connect(_on_frame_image_path_changed)
		frame_image_browse_button.pressed.connect(_on_frame_image_browse_pressed)
	
	sound_path_edit.text_changed.connect(_on_sound_path_changed)
	sound_browse_button.pressed.connect(_on_sound_browse_pressed)
	continue_text_edit.text_changed.connect(_on_continue_text_changed)
	background_color_picker.color_changed.connect(_on_background_color_changed)
	
	# Connect frame box check if it exists
	if show_frame_box_check:
		show_frame_box_check.toggled.connect(_on_show_frame_box_toggled)
	
	# Initialize
	clear_properties()

func update_properties(frame):
	is_updating = true
	current_frame = frame
	
	# Update UI with frame properties
	title_edit.text = frame.get("title", "")
	if show_title_check:
		show_title_check.button_pressed = frame.get("show_title", true)
	text_edit.text = frame.get("text", "")
	sound_path_edit.text = frame.get("voiceover_path", "")
	continue_text_edit.text = frame.get("continue_text", "Continue")
	
	# Update frame image if the UI element exists
	if frame_image_path_edit:
		frame_image_path_edit.text = frame.get("frame_image", "")
	
	# Update frame box check if it exists
	if show_frame_box_check:
		show_frame_box_check.button_pressed = frame.get("show_frame_box", true)
	
	if frame.has("background_color"):
		background_color_picker.color = Color(frame.background_color)
	else:
		background_color_picker.color = Color("#000000cc")
	
	is_updating = false

func clear_properties():
	is_updating = true
	current_frame = {}
	
	# Clear UI
	title_edit.text = ""
	if show_title_check:
		show_title_check.button_pressed = true
	text_edit.text = ""
	sound_path_edit.text = ""
	continue_text_edit.text = "Continue"
	background_color_picker.color = Color("#000000cc")
	
	# Clear frame image if the UI element exists
	if frame_image_path_edit:
		frame_image_path_edit.text = ""
	
	# Reset frame box check if it exists
	if show_frame_box_check:
		show_frame_box_check.button_pressed = true
	
	is_updating = false

# Signal handlers
func _on_title_changed(new_text):
	if is_updating:
		return
	emit_signal("property_changed", "title", new_text)

func _on_show_title_toggled(button_pressed):
	if is_updating:
		return
	emit_signal("property_changed", "show_title", button_pressed)

func _on_text_changed():
	if is_updating:
		return
	emit_signal("property_changed", "text", text_edit.text)

func _on_sound_path_changed(new_text):
	if is_updating:
		return
	emit_signal("property_changed", "voiceover_path", new_text)

func _on_continue_text_changed(new_text):
	if is_updating:
		return
	emit_signal("property_changed", "continue_text", new_text)

func _on_background_color_changed(new_color):
	if is_updating:
		return
	emit_signal("property_changed", "background_color", new_color.to_html(true))

func _on_frame_image_path_changed(new_text):
	if is_updating:
		return
	emit_signal("property_changed", "frame_image", new_text)

func _on_show_frame_box_toggled(button_pressed):
	if is_updating:
		return
	emit_signal("property_changed", "show_frame_box", button_pressed)

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

func _on_sound_browse_pressed():
	# Get the voiceover directory from settings
	var settings = settings_manager.get_settings()
	var voiceover_dir = settings.voiceover_dir if settings else "res://assets/audio/voice"
	
	# Create and show the file browser
	var browser = FileBrowserDialog.new(
		"Select Voiceover Audio File",
		["mp3", "ogg", "wav"],
		voiceover_dir,
		func(path):
			sound_path_edit.text = path
			emit_signal("property_changed", "voiceover_path", path)
	)
	browser.show()

func _on_frame_image_browse_pressed():
	# Get the images directory from settings
	var settings = settings_manager.get_settings()
	var images_dir = settings.images_dir if settings else "res://assets/images/cutscenes"
	
	# Create and show the file browser
	var browser = FileBrowserDialog.new(
		"Select Frame Image",
		["png", "jpg", "jpeg", "webp"],
		images_dir,
		func(path):
			frame_image_path_edit.text = path
			emit_signal("property_changed", "frame_image", path)
	)
	browser.show()
