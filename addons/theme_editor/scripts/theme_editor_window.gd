@tool
extends Window

# Constants
const DEFAULT_THEME_PATH = "res://assets/themes/groot_theme.tres"
const CONFIG_SECTION = "theme_editor"
const CONFIG_RECENT_THEMES = "recent_themes"

# Node references
@onready var theme_path_edit: LineEdit = $MainContainer/MenuBar/ThemePathEdit
@onready var load_button: Button = $MainContainer/MenuBar/LoadButton
@onready var save_button: Button = $MainContainer/MenuBar/SaveButton
@onready var create_button: Button = $MainContainer/MenuBar/CreateButton
@onready var recent_themes_option: OptionButton = $MainContainer/MenuBar/RecentThemesOption
@onready var tab_container: TabContainer = $MainContainer/HSplitContainer/LeftPanel/VBoxContainer/TabContainer
@onready var preview_container: Control = $MainContainer/HSplitContainer/RightPanel/VBoxContainer/ScrollContainer/PreviewContainer
@onready var status_label: Label = $MainContainer/StatusBar/StatusLabel
@onready var close_button: Button = $MainContainer/StatusBar/CloseButton

# Editor tabs
@onready var fonts_tab: Control = $MainContainer/HSplitContainer/LeftPanel/VBoxContainer/TabContainer/Fonts
@onready var colors_tab: Control = $MainContainer/HSplitContainer/LeftPanel/VBoxContainer/TabContainer/Colors
@onready var styleboxes_tab: Control = $MainContainer/HSplitContainer/LeftPanel/VBoxContainer/TabContainer/Styleboxes
@onready var constants_tab: Control = $MainContainer/HSplitContainer/LeftPanel/VBoxContainer/TabContainer/Constants
@onready var icons_tab: Control = $MainContainer/HSplitContainer/LeftPanel/VBoxContainer/TabContainer/Icons

# Private variables
var _editor_interface: EditorInterface
var _current_theme: Theme
var _file_dialog: FileDialog
var _recent_themes: Array = []
var _preview_controls: Dictionary = {}
var _modified: bool = false

# Signal for theme modifications
signal theme_modified

func _ready() -> void:
	# Initialize UI
	_setup_ui()
	
	# Connect signals
	_connect_signals()
	
	# Load configuration
	_load_config()
	
	# Create file dialog
	_create_file_dialog()
	
	# Set up preview panel
	_setup_preview_panel()
	
	# Auto-load default theme if it exists
	if ResourceLoader.exists(DEFAULT_THEME_PATH):
		_load_theme(DEFAULT_THEME_PATH)

func setup(editor_interface: EditorInterface) -> void:
	_editor_interface = editor_interface

func _setup_ui() -> void:
	# Set initial state
	save_button.disabled = true
	
	# Set up tabs
	tab_container.current_tab = 0
	
	# Connect close requested signal
	close_requested.connect(_on_close_requested)

func _connect_signals() -> void:
	# Connect UI signals
	load_button.pressed.connect(_on_load_button_pressed)
	save_button.pressed.connect(_on_save_button_pressed)
	create_button.pressed.connect(_on_create_button_pressed)
	recent_themes_option.item_selected.connect(_on_recent_theme_selected)
	close_button.pressed.connect(_on_close_button_pressed)
	
	# Connect theme modified signal
	theme_modified.connect(_on_theme_modified)
	
	# Connect tab change signal
	tab_container.tab_changed.connect(_on_tab_changed)


func _create_file_dialog() -> void:
	_file_dialog = FileDialog.new()
	add_child(_file_dialog)
	
	# Configure file dialog
	_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	_file_dialog.add_filter("*.tres")
	
	# Connect file dialog signals
	_file_dialog.file_selected.connect(_on_theme_file_selected)

func _load_config() -> void:
	# Load recent themes
	if ProjectSettings.has_setting(CONFIG_SECTION + "/" + CONFIG_RECENT_THEMES):
		_recent_themes = ProjectSettings.get_setting(CONFIG_SECTION + "/" + CONFIG_RECENT_THEMES)
		_update_recent_themes_dropdown()

func _save_config() -> void:
	# Save recent themes
	ProjectSettings.set_setting(CONFIG_SECTION + "/" + CONFIG_RECENT_THEMES, _recent_themes)
	ProjectSettings.save()

func _update_recent_themes_dropdown() -> void:
	recent_themes_option.clear()
	recent_themes_option.add_item("Recent Themes")
	
	for theme_path in _recent_themes:
		recent_themes_option.add_item(theme_path.get_file())

func _add_to_recent_themes(path: String) -> void:
	# Remove if already exists
	if _recent_themes.has(path):
		_recent_themes.erase(path)
	
	# Add to beginning of list
	_recent_themes.push_front(path)
	
	# Limit to 10 recent themes
	if _recent_themes.size() > 10:
		_recent_themes.resize(10)
	
	# Update dropdown and save config
	_update_recent_themes_dropdown()
	_save_config()

func _load_theme(path: String) -> void:
	if ResourceLoader.exists(path):
		var theme_resource = ResourceLoader.load(path)
		
		if theme_resource is Theme:
			_current_theme = theme_resource
			theme_path_edit.text = path
			_add_to_recent_themes(path)
			
			# Update UI
			save_button.disabled = false
			status_label.text = "Loaded theme: " + path.get_file()
			
			# Populate tabs with theme data
			_populate_tabs()
			
			# Apply theme to preview
			_update_preview()
		else:
			status_label.text = "Error: Selected file is not a Theme resource"
	else:
		status_label.text = "Error: Theme file does not exist: " + path

func _create_new_theme() -> void:
	_current_theme = Theme.new()
	theme_path_edit.text = ""
	
	# Update UI
	save_button.disabled = false
	status_label.text = "Created new theme"
	
	# Populate tabs with default data
	_populate_tabs()
	
	# Apply theme to preview
	_update_preview()

func _populate_tabs() -> void:
	# Clear existing tab content
	_clear_tabs()
	
	# Populate each tab with current theme data
	_populate_fonts_tab()
	_populate_colors_tab()
	_populate_styleboxes_tab()
	_populate_constants_tab()
	_populate_icons_tab()

func _clear_tabs() -> void:
	# Clear each tab's content
	_clear_fonts_tab()
	_clear_colors_tab()
	_clear_styleboxes_tab()
	_clear_constants_tab()
	_clear_icons_tab()

# Tab population methods
func _populate_fonts_tab() -> void:
	var font_container = fonts_tab.get_node("ScrollContainer/VBoxContainer")
	
	# Get all font types from the theme
	var font_types = _current_theme.get_font_type_list()
	
	for type in font_types:
		var font_names = _current_theme.get_font_list(type)
		
		for font_name in font_names:
			var font = _current_theme.get_font(font_name, type)
			
			# Create UI for this font
			var font_editor = preload("res://addons/theme_editor/scenes/font_editor_item.tscn").instantiate()
			font_container.add_child(font_editor)
			
			# Configure font editor
			font_editor.setup(type, font_name, font)
			font_editor.font_changed.connect(_on_font_changed.bind(type, font_name))

func _populate_colors_tab() -> void:
	var color_container = colors_tab.get_node("ScrollContainer/VBoxContainer")
	
	# Get all color types from the theme
	var color_types = _current_theme.get_color_type_list()
	
	for type in color_types:
		var color_names = _current_theme.get_color_list(type)
		
		for color_name in color_names:
			var color = _current_theme.get_color(color_name, type)
			
			# Create UI for this color
			var color_editor = preload("res://addons/theme_editor/scenes/color_editor_item.tscn").instantiate()
			color_container.add_child(color_editor)
			
			# Configure color editor
			color_editor.setup(type, color_name, color)
			color_editor.color_changed.connect(_on_color_changed.bind(type, color_name))

func _populate_styleboxes_tab() -> void:
	var stylebox_container = styleboxes_tab.get_node("ScrollContainer/VBoxContainer")
	
	# Get all stylebox types from the theme
	var stylebox_types = _current_theme.get_stylebox_type_list()
	
	for type in stylebox_types:
		var stylebox_names = _current_theme.get_stylebox_list(type)
		
		for stylebox_name in stylebox_names:
			var stylebox = _current_theme.get_stylebox(stylebox_name, type)
			
			# Create UI for this stylebox
			var stylebox_editor = preload("res://addons/theme_editor/scenes/stylebox_editor_item.tscn").instantiate()
			stylebox_container.add_child(stylebox_editor)
			
			# Configure stylebox editor
			stylebox_editor.setup(type, stylebox_name, stylebox)
			stylebox_editor.stylebox_changed.connect(_on_stylebox_changed.bind(type, stylebox_name))

func _populate_constants_tab() -> void:
	var constant_container = constants_tab.get_node("ScrollContainer/VBoxContainer")
	
	# Get all constant types from the theme
	var constant_types = _current_theme.get_constant_type_list()
	
	for type in constant_types:
		var constant_names = _current_theme.get_constant_list(type)
		
		for constant_name in constant_names:
			var constant = _current_theme.get_constant(constant_name, type)
			
			# Create UI for this constant
			var constant_editor = preload("res://addons/theme_editor/scenes/constant_editor_item.tscn").instantiate()
			constant_container.add_child(constant_editor)
			
			# Configure constant editor
			constant_editor.setup(type, constant_name, constant)
			constant_editor.constant_changed.connect(_on_constant_changed.bind(type, constant_name))

func _populate_icons_tab() -> void:
	var icon_container = icons_tab.get_node("ScrollContainer/VBoxContainer")
	
	# Get all icon types from the theme
	var icon_types = _current_theme.get_icon_type_list()
	
	for type in icon_types:
		var icon_names = _current_theme.get_icon_list(type)
		
		for icon_name in icon_names:
			var icon = _current_theme.get_icon(icon_name, type)
			
			# Create UI for this icon
			var icon_editor = preload("res://addons/theme_editor/scenes/icon_editor_item.tscn").instantiate()
			icon_container.add_child(icon_editor)
			
			# Configure icon editor
			icon_editor.setup(type, icon_name, icon)
			icon_editor.icon_changed.connect(_on_icon_changed.bind(type, icon_name))

# Tab clearing methods
func _clear_fonts_tab() -> void:
	var font_container = fonts_tab.get_node("ScrollContainer/VBoxContainer")
	for child in font_container.get_children():
		child.queue_free()

func _clear_colors_tab() -> void:
	var color_container = colors_tab.get_node("ScrollContainer/VBoxContainer")
	for child in color_container.get_children():
		child.queue_free()

func _clear_styleboxes_tab() -> void:
	var stylebox_container = styleboxes_tab.get_node("ScrollContainer/VBoxContainer")
	for child in stylebox_container.get_children():
		child.queue_free()

func _clear_constants_tab() -> void:
	var constant_container = constants_tab.get_node("ScrollContainer/VBoxContainer")
	for child in constant_container.get_children():
		child.queue_free()

func _clear_icons_tab() -> void:
	var icon_container = icons_tab.get_node("ScrollContainer/VBoxContainer")
	for child in icon_container.get_children():
		child.queue_free()

# Preview panel methods
func _setup_preview_panel() -> void:
	# Create common UI controls for preview
	_create_preview_controls()
	
	# Apply initial theme if available
	if _current_theme:
		_update_preview()
		
	# Connect preview signals
	_connect_preview_signals()

func _create_preview_controls() -> void:
	# Clear existing controls
	for child in preview_container.get_children():
		child.queue_free()
	
	_preview_controls.clear()
	
	# Create a container for the preview
	var preview_vbox = VBoxContainer.new()
	preview_vbox.name = "PreviewVBox"
	preview_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	preview_vbox.size_flags_vertical = Control.SIZE_EXPAND_FILL
	preview_container.add_child(preview_vbox)
	
	# Add section title
	var title = Label.new()
	title.text = "Theme Preview (Click elements to edit)"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	preview_vbox.add_child(title)
	_preview_controls["title"] = title
	
	# Add separator
	var separator = HSeparator.new()
	preview_vbox.add_child(separator)
	_preview_controls["separator"] = separator
	
	# Add buttons section
	var buttons_section = _create_section("Buttons")
	preview_vbox.add_child(buttons_section)
	
	# Normal button
	var button = Button.new()
	button.text = "Normal Button"
	buttons_section.add_child(button)
	_preview_controls["button"] = button
	
	# Disabled button
	var disabled_button = Button.new()
	disabled_button.text = "Disabled Button"
	disabled_button.disabled = true
	buttons_section.add_child(disabled_button)
	_preview_controls["disabled_button"] = disabled_button
	
	# Add input section
	var input_section = _create_section("Input Controls")
	preview_vbox.add_child(input_section)
	
	# LineEdit
	var line_edit = LineEdit.new()
	line_edit.placeholder_text = "Enter text here..."
	input_section.add_child(line_edit)
	_preview_controls["line_edit"] = line_edit
	
	# TextEdit
	var text_edit = TextEdit.new()
	text_edit.placeholder_text = "Multiline text editor..."
	text_edit.custom_minimum_size = Vector2(0, 100)
	input_section.add_child(text_edit)
	_preview_controls["text_edit"] = text_edit
	
	# Add option controls section
	var option_section = _create_section("Option Controls")
	preview_vbox.add_child(option_section)
	
	# CheckBox
	var check_box = CheckBox.new()
	check_box.text = "Check Box"
	option_section.add_child(check_box)
	_preview_controls["check_box"] = check_box
	
	# OptionButton
	var option_button = OptionButton.new()
	option_button.add_item("Option 1")
	option_button.add_item("Option 2")
	option_button.add_item("Option 3")
	option_section.add_child(option_button)
	_preview_controls["option_button"] = option_button
	
	# Add container section
	var container_section = _create_section("Containers")
	preview_vbox.add_child(container_section)
	
	# Panel
	var panel = Panel.new()
	panel.custom_minimum_size = Vector2(0, 80)
	container_section.add_child(panel)
	_preview_controls["panel"] = panel
	
	# TabContainer
	var tab_container = TabContainer.new()
	tab_container.custom_minimum_size = Vector2(0, 150)
	
	var tab1 = Control.new()
	tab1.name = "Tab 1"
	tab_container.add_child(tab1)
	
	var tab2 = Control.new()
	tab2.name = "Tab 2"
	tab_container.add_child(tab2)
	
	var tab3 = Control.new()
	tab3.name = "Tab 3"
	tab_container.add_child(tab3)
	
	container_section.add_child(tab_container)
	_preview_controls["tab_container"] = tab_container
	
	# Add progress section
	var progress_section = _create_section("Progress Controls")
	preview_vbox.add_child(progress_section)
	
	# ProgressBar
	var progress_bar = ProgressBar.new()
	progress_bar.value = 50
	progress_section.add_child(progress_bar)
	_preview_controls["progress_bar"] = progress_bar
	
	# Add tree section
	var tree_section = _create_section("Tree View")
	preview_vbox.add_child(tree_section)
	
	# Tree
	var tree = Tree.new()
	tree.custom_minimum_size = Vector2(0, 150)
	
	var root = tree.create_item()
	root.set_text(0, "Root")
	
	var child1 = tree.create_item(root)
	child1.set_text(0, "Child 1")
	
	var child2 = tree.create_item(root)
	child2.set_text(0, "Child 2")
	
	var subchild = tree.create_item(child1)
	subchild.set_text(0, "Subchild")
	
	tree_section.add_child(tree)
	_preview_controls["tree"] = tree

func _create_section(title: String) -> VBoxContainer:
	var section = VBoxContainer.new()
	section.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	# Create a header with collapse button
	var header = HBoxContainer.new()
	header.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	section.add_child(header)
	
	# Collapse button
	var collapse_button = Button.new()
	collapse_button.text = "-"
	collapse_button.tooltip_text = "Collapse/Expand Section"
	collapse_button.custom_minimum_size = Vector2(24, 24)
	header.add_child(collapse_button)
	
	# Section title
	var section_title = Label.new()
	section_title.text = title
	section_title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	header.add_child(section_title)
	
	# Content container (can be collapsed)
	var content = VBoxContainer.new()
	content.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	section.add_child(content)
	
	# Connect collapse button
	collapse_button.pressed.connect(_on_section_collapse_toggled.bind(collapse_button, content))
	
	# Add separator
	var section_separator = HSeparator.new()
	section.add_child(section_separator)
	
	# Store references to make sure they get proper theme updates
	_preview_controls["section_" + title + "_header"] = header
	_preview_controls["section_" + title + "_button"] = collapse_button
	_preview_controls["section_" + title + "_title"] = section_title
	_preview_controls["section_" + title + "_separator"] = section_separator
	
	return content

func _update_preview() -> void:
	# Apply current theme to all preview controls
	if _current_theme:
		for control in _preview_controls.values():
			control.theme = _current_theme

# Connect interactive preview elements
func _connect_preview_signals() -> void:
	# Connect all controls to be clickable for editing
	for key in _preview_controls.keys():
		var control = _preview_controls[key]
		
		# Skip non-interactive controls or section controls
		if key in ["title", "separator"] or key.begins_with("section_"):
			continue
		
		# Make control interactive
		control.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
		control.gui_input.connect(_on_preview_control_clicked.bind(key, control))

# Handle section collapse/expand
func _on_section_collapse_toggled(button: Button, content: Control) -> void:
	content.visible = !content.visible
	button.text = "+" if !content.visible else "-"

# Handle preview control clicked
func _on_preview_control_clicked(event: InputEvent, control_key: String, control: Control) -> void:
	# Only respond to left mouse button press
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		# Highlight the corresponding property in the editor
		_highlight_control_properties(control_key, control)
		
		# Show a tooltip with control info
		status_label.text = "Editing properties for: " + control_key

# Find and highlight properties for the clicked control
func _highlight_control_properties(control_key: String, control: Control) -> void:
	# Determine which tab to show based on control type
	var tab_index = 0 # Default to fonts tab
	var control_type = ""
	var property_name = ""
	
	# Select appropriate tab based on control type
	match control_key:
		"button":
			tab_index = 2 # Styleboxes tab
			control_type = "Button"
			property_name = "normal"
		"disabled_button":
			tab_index = 2 # Styleboxes tab
			control_type = "Button"
			property_name = "disabled"
		"line_edit":
			tab_index = 2 # Styleboxes tab
			control_type = "LineEdit"
			property_name = "normal"
		"text_edit":
			tab_index = 2 # Styleboxes tab
			control_type = "TextEdit"
			property_name = "normal"
		"check_box":
			tab_index = 4 # Icons tab
			control_type = "CheckBox"
			property_name = "checked"
		"option_button":
			tab_index = 2 # Styleboxes tab
			control_type = "OptionButton"
			property_name = "normal"
		"panel":
			tab_index = 2 # Styleboxes tab
			control_type = "Panel"
			property_name = "panel"
		"tab_container":
			tab_index = 2 # Styleboxes tab
			control_type = "TabContainer"
			property_name = "tab_selected"
		"progress_bar":
			tab_index = 2 # Styleboxes tab
			control_type = "ProgressBar"
			property_name = "fill"
		"tree":
			tab_index = 2 # Styleboxes tab
			control_type = "Tree"
			property_name = "panel"
	
	# Switch to the appropriate tab
	tab_container.current_tab = tab_index
	
	# Highlight the appropriate item
	if tab_index == 2: # Styleboxes tab
		_highlight_stylebox_item(control_type, property_name)
	elif tab_index == 4: # Icons tab
		_highlight_icon_item(control_type, property_name)

# Highlight a specific stylebox item in the styleboxes tab
func _highlight_stylebox_item(type: String, name: String) -> void:
	var stylebox_container = styleboxes_tab.get_node("ScrollContainer/VBoxContainer")
	
	# Find the matching stylebox editor
	for child in stylebox_container.get_children():
		if child.has_method("get_type") and child.has_method("get_name"):
			if child.get_type() == type and child.get_name() == name:
				# Scroll to this item
				var scroll = styleboxes_tab.get_node("ScrollContainer")
				scroll.ensure_control_visible(child)
				
				# Highlight the item
				child.highlight()
				return

# Highlight a specific icon item in the icons tab
func _highlight_icon_item(type: String, name: String) -> void:
	var icon_container = icons_tab.get_node("ScrollContainer/VBoxContainer")
	
	# Find the matching icon editor
	for child in icon_container.get_children():
		if child.has_method("get_type") and child.has_method("get_name"):
			if child.get_type() == type and child.get_name() == name:
				# Scroll to this item
				var scroll = icons_tab.get_node("ScrollContainer")
				scroll.ensure_control_visible(child)
				
				# Highlight the item
				child.highlight()
				return

# Signal handlers
func _on_load_button_pressed() -> void:
	_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	_file_dialog.popup_centered_ratio(0.7)

func _on_save_button_pressed() -> void:
	if _current_theme:
		if theme_path_edit.text.is_empty():
			# No path specified, show save dialog
			_file_dialog.file_mode = FileDialog.FILE_MODE_SAVE_FILE
			_file_dialog.popup_centered_ratio(0.7)
		else:
			# Save to existing path
			_save_theme(theme_path_edit.text)

func _on_create_button_pressed() -> void:
	_create_new_theme()

func _on_recent_theme_selected(index: int) -> void:
	if index > 0 and index <= _recent_themes.size():
		_load_theme(_recent_themes[index - 1])


func _on_theme_file_selected(path: String) -> void:
	if _file_dialog.file_mode == FileDialog.FILE_MODE_OPEN_FILE:
		_load_theme(path)
	elif _file_dialog.file_mode == FileDialog.FILE_MODE_SAVE_FILE:
		_save_theme(path)

func _save_theme(path: String) -> void:
	if _current_theme:
		# Save the theme resource
		var err = ResourceSaver.save(_current_theme, path)
		
		if err == OK:
			theme_path_edit.text = path
			_add_to_recent_themes(path)
			
			# Mark as unmodified
			_modified = false
			
			status_label.text = "Theme saved successfully to: " + path.get_file()
		else:
			status_label.text = "Error saving theme: " + str(err)

func _on_theme_modified() -> void:
	_modified = true
	status_label.text = "Theme modified (unsaved)"
	
	# Update preview
	_update_preview()

func _on_tab_changed(tab_index: int) -> void:
	# Update preview based on current tab
	match tab_index:
		0: # Fonts tab
			pass
		1: # Colors tab
			pass
		2: # Styleboxes tab
			pass
		3: # Constants tab
			pass
		4: # Icons tab
			pass

# Theme property change handlers
func _on_font_changed(font: Font, type: String, name: String) -> void:
	if _current_theme:
		_current_theme.set_font(name, type, font)
		theme_modified.emit()

func _on_color_changed(color: Color, type: String, name: String) -> void:
	if _current_theme:
		_current_theme.set_color(name, type, color)
		theme_modified.emit()

func _on_stylebox_changed(stylebox: StyleBox, type: String, name: String) -> void:
	if _current_theme:
		_current_theme.set_stylebox(name, type, stylebox)
		theme_modified.emit()

func _on_constant_changed(value: int, type: String, name: String) -> void:
	if _current_theme:
		_current_theme.set_constant(name, type, value)
		theme_modified.emit()

func _on_icon_changed(icon: Texture2D, type: String, name: String) -> void:
	if _current_theme:
		_current_theme.set_icon(name, type, icon)
		theme_modified.emit()

func _on_close_button_pressed() -> void:
	if _modified:
		# TODO: Show confirmation dialog
		hide()
	else:
		hide()

func _on_close_requested() -> void:
	if _modified:
		# TODO: Show confirmation dialog
		hide()
	else:
		hide()
