@tool
extends EditorPlugin

var dock: Control

func _enter_tree() -> void:
	# Register custom types
	add_custom_type("Puppet2D", "Node2D", load("res://addons/puppetlab/nodes/Puppet2D.gd"), load("res://addons/puppetlab/icons/puppetlab.svg"))
	add_custom_type("PuppetPart2D", "Sprite2D", load("res://addons/puppetlab/nodes/PuppetPart2D.gd"), load("res://addons/puppetlab/icons/puppetlab.svg"))
	# Dock UI
	dock = load("res://addons/puppetlab/dock/dock.tscn").instantiate()
	# Provide EditorInterface to the dock for selection control
	if dock.has_method("set_editor_interface"):
		dock.set_editor_interface(get_editor_interface())
	add_control_to_dock(DOCK_SLOT_RIGHT_UL, dock)

func _exit_tree() -> void:
	if dock:
		remove_control_from_docks(dock)
		dock.free()
	remove_custom_type("PuppetPart2D")
	remove_custom_type("Puppet2D")
