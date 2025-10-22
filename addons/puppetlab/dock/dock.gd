@tool
extends VBoxContainer

var editor_if

@onready var path_edit = $Path
@onready var info = $Info

func set_editor_interface(ei) -> void:
	editor_if = ei

func _ready() -> void:
	$Pick.pressed.connect(_on_pick)
	$BuildBtn.pressed.connect(_on_build)

func _on_pick() -> void:
	var d = EditorFileDialog.new()
	d.file_mode = EditorFileDialog.FILE_MODE_OPEN_DIR
	d.access = EditorFileDialog.ACCESS_RESOURCES
	d.title = "Select source folder"
	add_child(d)
	d.dir_selected.connect(_on_dir_selected)
	d.popup_centered()

func _on_dir_selected(p) -> void:
	path_edit.text = p

func _on_build() -> void:
	var src = path_edit.text.strip_edges()
	if src == "":
		info.text = "Pick a folder with layered PNGs, or an Aseprite JSON+PNG export."
		return
	var builder = load("res://addons/puppetlab/builders/puppet_builder.gd").new()
	var opts = {
		"gen_idle": $Options/GenIdle.button_pressed,
		"gen_walk": $Options/GenWalk.button_pressed,
		"pixel_snap": $Options/PixelSnap.button_pressed
	}
	var puppet: Node2D = builder.build_from_folder(src, opts)
	if puppet:
		var root = get_tree().edited_scene_root
		root.add_child(puppet)
		info.text = "Puppet built: %s" % puppet.name
		if editor_if:
			var sel = editor_if.get_selection()
			sel.clear()
			sel.add_node(puppet)
	else:
		info.text = "Build failed. Check folder structure and names."
