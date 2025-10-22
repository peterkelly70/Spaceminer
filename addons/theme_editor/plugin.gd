@tool
extends EditorPlugin

const ThemeEditorWindow = preload("res://addons/theme_editor/scenes/theme_editor_window.tscn")
var theme_editor_window
var theme_editor_button

func _enter_tree():
	# Create toolbar button
	theme_editor_button = Button.new()
	theme_editor_button.text = "Theme Editor"
	theme_editor_button.tooltip_text = "Open Advanced Theme Editor"
	theme_editor_button.pressed.connect(_on_theme_editor_button_pressed)
	
	# Add button to editor toolbar
	add_control_to_container(CONTAINER_TOOLBAR, theme_editor_button)

	# Initialize the theme editor window
	theme_editor_window = ThemeEditorWindow.instantiate()
	get_editor_interface().get_base_control().add_child(theme_editor_window)
	
	# Connect to the editor interface for theme updates
	theme_editor_window.setup(get_editor_interface())
	
	# Initially hide the editor window
	theme_editor_window.hide()

func _exit_tree():
	# Clean up the theme editor when the plugin is disabled
	if theme_editor_window:
		theme_editor_window.queue_free()
	
	# Clean up the toolbar button
	if theme_editor_button:
		remove_control_from_container(CONTAINER_TOOLBAR, theme_editor_button)
		theme_editor_button.queue_free()

func _on_theme_editor_button_pressed():
	# Show the theme editor window
	theme_editor_window.show()
	theme_editor_window.grab_focus()
