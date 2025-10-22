@tool
extends Node

## Import helper: reads a folder of PNGs. Optional Aseprite JSON mapping.

static func find_layers(folder: String) -> Array:
	var d := DirAccess.open(folder)
	if d == null:
		push_error("Cannot open folder: %s" % folder)
		return []
	d.list_dir_begin()
	var files: Array = []
	while true:
		var f := d.get_next()
		if f == "":
			break
		if d.current_is_dir():
			continue
		if f.to_lower().ends_with(".png"):
			files.append(folder.plus_file(f))
	d.list_dir_end()
	return files

static func load_texture(path: String) -> Texture2D:
	return load(path)
