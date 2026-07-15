@tool
extends EditorPlugin

var cutscene_dock: Control
var settings_dialog: AcceptDialog = null
var inspector_plugin = null

func _enter_tree():
	# Load the dock scene
	var dock_scene = preload("res://addons/cutscene_editor/dock/CutsceneEditorDock.tscn")
	cutscene_dock = dock_scene.instantiate()
	
	# Set editor interface
	cutscene_dock.set_editor_interface(get_editor_interface())
	
	# Set a custom name for the dock tab
	cutscene_dock.set_name("Cutscenes")
	
	# Add the dock to the editor
	add_control_to_dock(DOCK_SLOT_LEFT_UL, cutscene_dock)

	# Connect to resource selection signal
	if cutscene_dock.has_signal("resource_selected"):
		cutscene_dock.resource_selected.connect(_on_resource_selected)
		
	# Connect to settings requested signal
	if cutscene_dock.has_signal("settings_requested"):
		cutscene_dock.settings_requested.connect(_on_settings_requested)
		
	# Register inspector plugin
	inspector_plugin = preload("res://addons/cutscene_editor/resources/CutsceneEditorInspectorPlugin.gd").new()
	add_inspector_plugin(inspector_plugin)

func _exit_tree():
	# Remove inspector plugin
	if inspector_plugin:
		remove_inspector_plugin(inspector_plugin)
		inspector_plugin = null
		
	# Remove the dock
	remove_control_from_docks(cutscene_dock)
	cutscene_dock.queue_free()
	
	# Close settings dialog if open
	if settings_dialog and is_instance_valid(settings_dialog):
		settings_dialog.queue_free()

func _on_resource_selected(res: Resource) -> void:
	# Show the resource in the inspector
	get_editor_interface().inspect_object(res)

func _on_settings_requested() -> void:
	# Create settings dialog if it doesn't exist
	if not settings_dialog or not is_instance_valid(settings_dialog):
		var settings_scene = preload("res://addons/cutscene_editor/dock/settings_panel.tscn")
		settings_dialog = settings_scene.instantiate() as AcceptDialog
		
		# Connect to the settings_saved signal if it exists
		if settings_dialog.has_signal("settings_saved"):
			settings_dialog.settings_saved.connect(_on_settings_saved)
		
		# Add to editor
		get_editor_interface().get_base_control().add_child(settings_dialog)
	
	# Show the settings dialog
	settings_dialog.popup_centered()

func _on_settings_saved() -> void:
	# Reload the dock to apply new settings
	if cutscene_dock and is_instance_valid(cutscene_dock):
		cutscene_dock._ready() # This will reload settings
