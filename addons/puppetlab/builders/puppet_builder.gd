@tool
extends Node

const LayerImport = preload("res://addons/puppetlab/builders/layer_import.gd")
const NameUtils = preload("res://addons/puppetlab/utils/name_utils.gd")

func build_from_folder(folder: String, opts := {}) -> Node2D:
	var puppet := load("res://addons/puppetlab/nodes/Puppet2D.gd").new()
	puppet.name = NameUtils.folder_to_puppet_name(folder)
	puppet.pixel_snap = opts.get("pixel_snap", true)
	puppet.create_basic_bones()
	var files := LayerImport.find_layers(folder)
	if files.is_empty():
		return null
	for path in files:
		var key := NameUtils.path_key(path)
		var tex := LayerImport.load_texture(path)
		match key:
			"head","torso","left_arm","right_arm","left_leg","right_leg","weapon","cloak":
				puppet.add_part(key, tex)
			_:
				puppet.add_part(key, tex)
	if opts.get("gen_idle", true):
		puppet.make_idle_animation()
	return puppet
