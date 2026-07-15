extends Control
class_name RoomEditor

const DEFAULT_LEVEL_JSON := "res://assets/levels/landing_bay_alpha.json"
const DEFAULT_ROOM_SCENE := "res://scenes/levels/GeneratedAsteroidRoom.tscn"
const TILESET_TEXTURE_PATH := "res://assets/tiles/monochrome_tilemap_transparent_packed.png"
const TILE_PX := 16          # pixels per tile in the source image
const TILE_COLS := 20        # columns in the tileset
const TILE_SCALE := 3        # display scale for tile picker
const LevelSolvabilityValidator := preload("res://scripts/tools/LevelSolvabilityValidator.gd")
const RoomNameGenerator := preload("res://scripts/tools/RoomNameGenerator.gd")

# ---- Tool modes ----
enum Tool { TILE, SOLID, HAZARD, ORE, RESUPPLY, DOOR_LEFT, DOOR_RIGHT, ERASE }

var _tool: Tool = Tool.TILE
var _selected_tile: int = 175   # default floor tile
var _level_json_path := DEFAULT_LEVEL_JSON
var _level_data: Dictionary = {}
var _dirty := false

# UI refs
var _json_label: Label
var _room_name_edit: LineEdit
var _status_label: Label
var _tile_picker: Control
var _tool_label: Label
var _is_resupply_check: CheckBox
var _is_junction_check: CheckBox
var _preview_viewport: SubViewport
var _preview_container: SubViewportContainer
var _preview_root: Node2D
var _preview_camera: Camera2D
var _tile_texture: Texture2D

func _ready() -> void:
	_tile_texture = load(TILESET_TEXTURE_PATH)
	_build_ui()
	_load_from_disk(_level_json_path)

# ===== UI BUILD =====

func _build_ui() -> void:
	set_anchors_preset(Control.PRESET_FULL_RECT)
	mouse_filter = Control.MOUSE_FILTER_STOP

	var root := VBoxContainer.new()
	root.set_anchors_preset(Control.PRESET_FULL_RECT)
	root.add_theme_constant_override("separation", 4)
	add_child(root)

	root.add_child(_build_topbar())
	root.add_child(_build_main_area())

func _build_topbar() -> HBoxContainer:
	var bar := HBoxContainer.new()
	bar.custom_minimum_size = Vector2(0, 40)
	bar.add_theme_constant_override("separation", 6)

	_json_label = Label.new()
	_json_label.add_theme_font_size_override("font_size", 14)
	_json_label.text = _level_json_path
	_json_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_json_label.clip_text = true
	bar.add_child(_json_label)

	var room_label := Label.new()
	room_label.text = "Name:"
	room_label.add_theme_font_size_override("font_size", 14)
	bar.add_child(room_label)

	_room_name_edit = LineEdit.new()
	_room_name_edit.custom_minimum_size = Vector2(200, 0)
	_room_name_edit.add_theme_font_size_override("font_size", 14)
	bar.add_child(_room_name_edit)

	for label_text in ["Random", "Save", "Save As…", "Reload"]:
		var btn := _btn(label_text)
		match label_text:
			"Random": btn.pressed.connect(_on_random_name)
			"Save":   btn.pressed.connect(_on_save)
			"Save As…": btn.pressed.connect(_on_save_as)
			"Reload": btn.pressed.connect(func(): _load_from_disk(_level_json_path))
		bar.add_child(btn)

	var sep := VSeparator.new()
	bar.add_child(sep)

	for label_text in ["Standard", "Resupply", "Junction"]:
		var btn := _btn("+ " + label_text)
		var t := label_text.to_lower()
		btn.pressed.connect(func(): _new_room(t))
		bar.add_child(btn)

	return bar

func _build_main_area() -> HSplitContainer:
	var split := HSplitContainer.new()
	split.size_flags_vertical = Control.SIZE_EXPAND_FILL
	split.split_offset = -420  # sidebar on right, preview takes most space

	# ---- Left: preview ----
	var prev_panel := PanelContainer.new()
	prev_panel.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	prev_panel.size_flags_vertical = Control.SIZE_EXPAND_FILL
	split.add_child(prev_panel)

	var prev_vbox := VBoxContainer.new()
	prev_panel.add_child(prev_vbox)

	_tool_label = Label.new()
	_tool_label.add_theme_font_size_override("font_size", 14)
	_tool_label.text = "Tool: Tile (tile 175)  |  Left-click = place  Right-click = erase"
	prev_vbox.add_child(_tool_label)

	_preview_viewport = SubViewport.new()
	_preview_viewport.disable_3d = true
	_preview_viewport.transparent_bg = true
	_preview_viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_preview_viewport.size = Vector2i(1280, 720)

	_preview_root = Node2D.new()
	_preview_root.name = "PreviewRoot"
	_preview_viewport.add_child(_preview_root)

	_preview_camera = Camera2D.new()
	_preview_camera.zoom = Vector2(2, 2)
	_preview_root.add_child(_preview_camera)
	_preview_camera.make_current()

	_preview_container = SubViewportContainer.new()
	_preview_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_preview_container.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_preview_container.stretch = true
	_preview_container.add_child(_preview_viewport)
	_preview_container.gui_input.connect(_on_preview_input)
	_preview_container.mouse_filter = Control.MOUSE_FILTER_STOP
	prev_vbox.add_child(_preview_container)

	_status_label = Label.new()
	_status_label.add_theme_font_size_override("font_size", 13)
	_status_label.text = "Ready"
	prev_vbox.add_child(_status_label)

	# ---- Right: sidebar ----
	var sidebar := _build_sidebar()
	split.add_child(sidebar)

	return split

func _build_sidebar() -> VBoxContainer:
	var sb := VBoxContainer.new()
	sb.custom_minimum_size = Vector2(420, 0)
	sb.size_flags_vertical = Control.SIZE_EXPAND_FILL
	sb.add_theme_constant_override("separation", 6)

	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	sb.add_child(scroll)

	var inner := VBoxContainer.new()
	inner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	inner.add_theme_constant_override("separation", 8)
	scroll.add_child(inner)

	# -- Object tools --
	inner.add_child(_section_label("PLACE OBJECTS"))
	inner.add_child(_build_object_tools())

	# -- Room flags --
	inner.add_child(_section_label("ROOM TYPE"))
	_is_resupply_check = CheckBox.new()
	_is_resupply_check.text = "Resupply Room"
	_is_resupply_check.add_theme_font_size_override("font_size", 14)
	_is_resupply_check.toggled.connect(func(on): _level_data["is_resupply"] = on; _dirty = true)
	inner.add_child(_is_resupply_check)

	_is_junction_check = CheckBox.new()
	_is_junction_check.text = "Junction Room"
	_is_junction_check.add_theme_font_size_override("font_size", 14)
	_is_junction_check.toggled.connect(func(on): _level_data["is_junction"] = on; _dirty = true)
	inner.add_child(_is_junction_check)

	# -- Tile picker --
	inner.add_child(_section_label("TILE PICKER  (click to select, then paint in preview)"))
	_tile_picker = _build_tile_picker()
	inner.add_child(_tile_picker)

	return sb

func _build_object_tools() -> GridContainer:
	var grid := GridContainer.new()
	grid.columns = 3
	grid.add_theme_constant_override("h_separation", 6)
	grid.add_theme_constant_override("v_separation", 6)

	var tools := [
		[Tool.TILE,       "🟩 Paint Tile",    Color(0.2, 0.8, 0.2)],
		[Tool.SOLID,      "🟫 Solid Floor",   Color(0.6, 0.4, 0.2)],
		[Tool.HAZARD,     "🔴 Hazard",        Color(0.9, 0.2, 0.2)],
		[Tool.ORE,        "🟡 Ore Pickup",    Color(0.9, 0.8, 0.1)],
		[Tool.RESUPPLY,   "🟢 Resupply",      Color(0.1, 0.8, 0.4)],
		[Tool.DOOR_LEFT,  "⬅ Door Left",     Color(0.3, 0.5, 0.9)],
		[Tool.DOOR_RIGHT, "➡ Door Right",    Color(0.3, 0.5, 0.9)],
		[Tool.ERASE,      "❌ Erase",         Color(0.5, 0.5, 0.5)],
	]

	for entry in tools:
		var t: Tool = entry[0]
		var lbl: String = entry[1]
		var btn := Button.new()
		btn.text = lbl
		btn.add_theme_font_size_override("font_size", 15)
		btn.custom_minimum_size = Vector2(130, 36)
		btn.pressed.connect(func(): _set_tool(t))
		grid.add_child(btn)

	return grid

func _build_tile_picker() -> Control:
	if not _tile_texture:
		var lbl := Label.new()
		lbl.text = "Tileset not found"
		return lbl

	var picker := Control.new()
	var tex_w := TILE_COLS * TILE_PX * TILE_SCALE
	var tex_h := TILE_COLS * TILE_PX * TILE_SCALE  # 20 rows too
	picker.custom_minimum_size = Vector2(tex_w, tex_h)
	picker.mouse_filter = Control.MOUSE_FILTER_STOP
	picker.draw.connect(func(): _draw_tile_picker(picker))
	picker.gui_input.connect(func(e): _on_tile_picker_input(e, picker))
	return picker

func _draw_tile_picker(picker: Control) -> void:
	if not _tile_texture:
		return
	var tw := TILE_PX * TILE_SCALE
	# Draw tileset scaled up
	picker.draw_texture_rect(_tile_texture, Rect2(Vector2.ZERO, picker.custom_minimum_size), false)
	# Draw selection highlight
	var sel_x := (_selected_tile % TILE_COLS) * tw
	var sel_y := (_selected_tile / TILE_COLS) * tw
	picker.draw_rect(Rect2(sel_x, sel_y, tw, tw), Color(1, 1, 0, 0.7), false, 2)
	picker.draw_rect(Rect2(sel_x, sel_y, tw, tw), Color(1, 1, 0, 0.2))

func _on_tile_picker_input(event: InputEvent, picker: Control) -> void:
	if not (event is InputEventMouseButton):
		return
	var mb := event as InputEventMouseButton
	if not mb.pressed or mb.button_index != MOUSE_BUTTON_LEFT:
		return
	var tw := TILE_PX * TILE_SCALE
	var col := int(mb.position.x / tw)
	var row := int(mb.position.y / tw)
	if col < 0 or col >= TILE_COLS or row < 0 or row >= TILE_COLS:
		return
	_selected_tile = row * TILE_COLS + col
	_set_tool(Tool.TILE)
	picker.queue_redraw()

func _section_label(text: String) -> Label:
	var lbl := Label.new()
	lbl.text = text
	lbl.add_theme_font_size_override("font_size", 13)
	lbl.modulate = Color(0.7, 0.9, 1.0)
	return lbl

func _set_tool(t: Tool) -> void:
	_tool = t
	var names := {
		Tool.TILE:        "Paint Tile (tile %d)" % _selected_tile,
		Tool.SOLID:       "Solid Floor",
		Tool.HAZARD:      "Hazard Zone",
		Tool.ORE:         "Ore Pickup",
		Tool.RESUPPLY:    "Resupply Station",
		Tool.DOOR_LEFT:   "Door (Left)",
		Tool.DOOR_RIGHT:  "Door (Right)",
		Tool.ERASE:       "Erase",
	}
	_tool_label.text = "Tool: %s  |  Left-click = place  Right-click = erase tile" % names.get(t, "?")

# ===== PREVIEW INTERACTION =====

func _on_preview_input(event: InputEvent) -> void:
	if not (event is InputEventMouseButton):
		return
	var mb := event as InputEventMouseButton
	if not mb.pressed:
		return

	var world_pos := _screen_to_world(mb.position)
	var cell := Vector2i(floori(world_pos.x / 20.0), floori(world_pos.y / 20.0))

	if mb.button_index == MOUSE_BUTTON_LEFT:
		match _tool:
			Tool.TILE:     _place_tile(cell.x, cell.y, _selected_tile)
			Tool.SOLID:    _place_solid(world_pos)
			Tool.HAZARD:   _place_object("hazards", "Hazard", "res://scenes/prototype/HazardZone.tscn", world_pos)
			Tool.ORE:      _place_object("collectibles", "Ore", "res://scenes/prototype/OreFragment.tscn", world_pos)
			Tool.RESUPPLY: _place_object("collectibles", "Resupply", "res://scenes/prototype/ResupplyStation.tscn", world_pos)
			Tool.DOOR_LEFT:  _place_door("left", world_pos)
			Tool.DOOR_RIGHT: _place_door("right", world_pos)
			Tool.ERASE:    _erase_at(cell.x, cell.y, world_pos)
	elif mb.button_index == MOUSE_BUTTON_RIGHT:
		_erase_at(cell.x, cell.y, world_pos)

func _screen_to_world(screen_pos: Vector2) -> Vector2:
	if _preview_container.size.x <= 0 or _preview_container.size.y <= 0:
		return Vector2.ZERO
	var vp_size := Vector2(_preview_viewport.size)
	var scale_v := vp_size / _preview_container.size
	var vp_pos := screen_pos * scale_v
	var cam_pos := _preview_camera.global_position
	var zoom := _preview_camera.zoom
	return cam_pos + (vp_pos - vp_size * 0.5) / zoom

# ===== EDIT OPERATIONS =====

func _place_tile(x: int, y: int, tile: int) -> void:
	var layers: Array = _level_data.get("tile_layers", [])
	if layers.is_empty():
		layers = [{"name": "PlatformTiles", "modulate": "#8cff00", "z_index": 1, "cells": []}]
		_level_data["tile_layers"] = layers
	var layer: Dictionary = layers[0]
	var cells: Array = layer.get("cells", [])
	var found := false
	for i in range(cells.size()):
		var c: Dictionary = cells[i]
		if int(c.get("x", 0)) == x and int(c.get("y", 0)) == y:
			cells[i] = {"x": x, "y": y, "tile": tile}
			found = true
			break
	if not found:
		cells.append({"x": x, "y": y, "tile": tile})
	layer["cells"] = cells
	layers[0] = layer
	_level_data["tile_layers"] = layers
	_dirty = true
	_rebuild_preview()
	_set_status("Tile %d at (%d, %d)" % [tile, x, y])

func _place_solid(world_pos: Vector2) -> void:
	var solids: Array = _level_data.get("solids", [])
	var snapped := Vector2(snappedf(world_pos.x, 20.0), snappedf(world_pos.y, 20.0))
	var name_str := "Solid_%d_%d" % [int(snapped.x), int(snapped.y)]
	solids.append({
		"name": name_str,
		"position": [int(snapped.x), int(snapped.y)],
		"size": [60, 16]
	})
	_level_data["solids"] = solids
	_dirty = true
	_rebuild_preview()
	_set_status("Solid floor at (%.0f, %.0f) — resize in JSON if needed" % [snapped.x, snapped.y])

func _place_object(list_key: String, prefix: String, scene: String, world_pos: Vector2) -> void:
	var arr: Array = _level_data.get(list_key, [])
	var snapped := Vector2(snappedf(world_pos.x, 20.0), snappedf(world_pos.y, 20.0))
	var name_str := "%s_%d_%d" % [prefix, int(snapped.x), int(snapped.y)]
	arr.append({
		"name": name_str,
		"scene": scene,
		"position": [int(snapped.x), int(snapped.y)]
	})
	_level_data[list_key] = arr
	_dirty = true
	_rebuild_preview()
	_set_status("Placed %s at (%.0f, %.0f)" % [prefix, snapped.x, snapped.y])

func _place_door(side: String, world_pos: Vector2) -> void:
	var doors: Array = _level_data.get("doors", [])
	var snapped := Vector2(snappedf(world_pos.x, 20.0), snappedf(world_pos.y, 20.0))
	var name_str := "Door_%s_%d_%d" % [side, int(snapped.x), int(snapped.y)]
	doors.append({
		"name": name_str,
		"position": [int(snapped.x), int(snapped.y)],
		"size": [16, 64],
		"next_room": ""
	})
	_level_data["doors"] = doors
	_dirty = true
	_rebuild_preview()
	_set_status("Door (%s) at (%.0f, %.0f) — set next_room in JSON" % [side, snapped.x, snapped.y])

func _erase_at(x: int, y: int, world_pos: Vector2) -> void:
	# Erase tile first
	var layers: Array = _level_data.get("tile_layers", [])
	for li in range(layers.size()):
		var layer: Dictionary = layers[li]
		var cells: Array = layer.get("cells", [])
		for i in range(cells.size() - 1, -1, -1):
			var c: Dictionary = cells[i]
			if int(c.get("x", 0)) == x and int(c.get("y", 0)) == y:
				cells.remove_at(i)
				_dirty = true
		layer["cells"] = cells
		layers[li] = layer
	_level_data["tile_layers"] = layers
	if _dirty:
		_rebuild_preview()
		_set_status("Erased tile (%d, %d)" % [x, y])

# ===== LOAD / SAVE =====

func _load_from_disk(path: String) -> void:
	var raw := FileAccess.get_file_as_string(path)
	if raw.is_empty():
		_set_status("Cannot read %s" % path)
		return
	var parsed: Variant = JSON.parse_string(raw)
	if typeof(parsed) != TYPE_DICTIONARY:
		_set_status("Invalid JSON: %s" % path)
		return
	_level_data = parsed
	_level_json_path = path
	_dirty = false
	_refresh_controls()
	_rebuild_preview()
	_set_status("Loaded %s" % path.get_file())

func _refresh_controls() -> void:
	_json_label.text = _level_json_path
	_room_name_edit.text = str(_level_data.get("room_name", _level_data.get("name", "")))
	_is_resupply_check.button_pressed = bool(_level_data.get("is_resupply", false))
	_is_junction_check.button_pressed = bool(_level_data.get("is_junction", false))

func _rebuild_preview() -> void:
	for child in _preview_root.get_children():
		child.queue_free()

	# Magenta border + black playfield
	var bg := Polygon2D.new()
	bg.color = Color("#330044")
	bg.polygon = PackedVector2Array([-360, -220, 360, -220, 360, 220, -360, 220])
	_preview_root.add_child(bg)

	# Re-add camera
	_preview_camera = Camera2D.new()
	_preview_camera.zoom = Vector2(2, 2)
	_preview_root.add_child(_preview_camera)
	_preview_camera.make_current()

	# Build level
	if not _level_json_path.is_empty():
		var abs_path := ProjectSettings.globalize_path(_level_json_path)
		if FileAccess.file_exists(abs_path) or FileAccess.file_exists(_level_json_path):
			var lb_script: Script = load("res://scripts/controllers/rooms/LevelBuilder.gd")
			if lb_script:
				var lb := Node2D.new()
				lb.name = "Level"
				lb.set_script(lb_script)
				lb.level_json_path = _level_json_path
				_preview_root.add_child(lb)

	# Draw object overlays (coloured dots/icons so you can see them)
	_draw_object_overlays()

	# Update status bar
	var cells := 0
	for l: Dictionary in _level_data.get("tile_layers", []):
		cells += int(l.get("cells", []).size())
	var rtype := "standard"
	if _level_data.get("is_resupply", false): rtype = "resupply"
	elif _level_data.get("is_junction", false): rtype = "junction"
	var solids: int = _level_data.get("solids", []).size()
	var hazards: int = _level_data.get("hazards", []).size()
	var collectibles: int = _level_data.get("collectibles", []).size()
	var doors: int = _level_data.get("doors", []).size()
	_status_label.text = "%s | %s | tiles:%d  solids:%d  hazards:%d  pickups:%d  doors:%d%s" % [
		_level_json_path.get_file(), rtype, cells, solids, hazards, collectibles, doors,
		"  *UNSAVED*" if _dirty else ""
	]

func _draw_object_overlays() -> void:
	var overlay := Node2D.new()
	overlay.name = "Overlays"
	_preview_root.add_child(overlay)

	# Spawn point
	var spawn: Array = _level_data.get("spawn", [0, 0])
	_add_dot(overlay, Vector2(spawn[0], spawn[1]), Color.CYAN, "SPAWN")

	# Exit
	var exit_pos: Array = _level_data.get("exit", {}).get("position", [])
	if exit_pos.size() >= 2:
		_add_dot(overlay, Vector2(exit_pos[0], exit_pos[1]), Color.GREEN, "EXIT")

	# Hazards
	for h: Dictionary in _level_data.get("hazards", []):
		var p: Array = h.get("position", [0, 0])
		_add_dot(overlay, Vector2(p[0], p[1]), Color.RED, "⚡")

	# Collectibles
	for c: Dictionary in _level_data.get("collectibles", []):
		var p: Array = c.get("position", [0, 0])
		var scene: String = str(c.get("scene", ""))
		var color := Color.YELLOW
		var icon := "ORE"
		if "resupply" in scene: color = Color.LIME_GREEN; icon = "RS"
		_add_dot(overlay, Vector2(p[0], p[1]), color, icon)

	# Doors
	for d: Dictionary in _level_data.get("doors", []):
		var p: Array = d.get("position", [0, 0])
		_add_dot(overlay, Vector2(p[0], p[1]), Color.CORNFLOWER_BLUE, "🚪")

func _add_dot(parent: Node2D, pos: Vector2, color: Color, label: String) -> void:
	var dot := Node2D.new()
	dot.position = pos
	parent.add_child(dot)

	var circle := Line2D.new()
	var pts: Array[Vector2] = []
	for i in range(9):
		var angle := i * TAU / 8.0
		pts.append(Vector2(cos(angle), sin(angle)) * 6.0)
	circle.points = PackedVector2Array(pts)
	circle.default_color = color
	circle.width = 2.0
	dot.add_child(circle)

	var lbl := Label.new()
	lbl.text = label
	lbl.position = Vector2(-12, -18)
	lbl.add_theme_font_size_override("font_size", 8)
	lbl.add_theme_color_override("font_color", color)
	dot.add_child(lbl)

# ===== NEW ROOM TEMPLATES =====

func _new_room(type: String) -> void:
	var room_name := RoomNameGenerator.get_random_name()
	_level_data = {
		"name": room_name,
		"room_name": room_name,
		"spawn": [-200, 100],
		"exit": {
			"position": [200, -80],
			"size": [40, 72],
			"sprite_frames": "res://assets/sprite/space_miner_teleporter_frames.tres",
			"animation": "cycle",
			"scale": [2.0, 2.0]
		},
		"tile_layers": [
			{"name": "PlatformTiles", "modulate": "#8cff00", "z_index": 1, "cells": _floor_cells()}
		],
		"solids": [
			{"name": "Floor",     "position": [0, 128],    "size": [480, 16]},
			{"name": "WallLeft",  "position": [-240, 0],   "size": [16, 320]},
			{"name": "WallRight", "position": [240, 0],    "size": [16, 320]},
		],
		"collectibles": [],
		"hazards": [],
		"enemies": [],
		"doors": [],
		"is_resupply": type == "resupply",
		"is_junction": type == "junction",
	}
	match type:
		"resupply":
			_level_data["collectibles"] = [
				{"name": "ResupplyStation", "scene": "res://scenes/prototype/ResupplyStation.tscn", "position": [0, 96]}
			]
		"junction":
			_level_data["doors"] = [
				{"name": "DoorLeft",  "position": [-220, 64],  "size": [16, 64], "next_room": ""},
				{"name": "DoorRight", "position": [204, 64],   "size": [16, 64], "next_room": ""},
				{"name": "DoorTop",   "position": [0, -160],   "size": [64, 16], "next_room": ""},
			]

	var slug := room_name.to_lower().replace(" ", "_").replace("'", "").replace(":", "")
	slug = ''.join(slug.split("").filter(func(c): return c.is_valid_identifier() or c == "_"))
	_level_json_path = "res://assets/levels/room_%s.json" % slug
	_dirty = true
	_refresh_controls()
	_rebuild_preview()
	_set_status("New %s room: %s — press Save As to write to disk" % [type, room_name])

func _floor_cells() -> Array:
	var cells := []
	for x in range(-12, 13):
		cells.append({"x": x, "y": 7, "tile": 175 + (x + 12) % 5})
	return cells

# ===== RANDOM NAME =====

func _on_random_name() -> void:
	var n := RoomNameGenerator.get_random_name()
	_room_name_edit.text = n
	_level_data["room_name"] = n
	_level_data["name"] = n
	_dirty = true

# ===== SAVE =====

func _on_save() -> void:
	_apply_name()
	var abs_path := ProjectSettings.globalize_path(_level_json_path)
	_write_json(abs_path)
	_dirty = false
	_rebuild_preview()

func _on_save_as() -> void:
	_apply_name()
	var rname := str(_level_data.get("room_name", "room")).to_lower()\
		.replace(" ", "_").replace("'", "").replace(":", "")\
		.replace("[", "").replace("]", "").strip_edges()
	var fname := "room_%s.json" % rname
	var abs_path := ProjectSettings.globalize_path("res://assets/levels/") + fname
	_write_json(abs_path)
	_level_json_path = "res://assets/levels/" + fname
	_dirty = false
	_refresh_controls()
	_rebuild_preview()

func _write_json(abs_path: String) -> void:
	var json := JSON.stringify(_level_data, "\t", false)
	var file := FileAccess.open(abs_path, FileAccess.WRITE)
	if file == null:
		_set_status("Could not write: %s" % abs_path)
		return
	file.store_string(json)
	file.close()
	_set_status("Saved: %s" % abs_path.get_file())

func _apply_name() -> void:
	var n := _room_name_edit.text.strip_edges()
	if not n.is_empty():
		_level_data["room_name"] = n
		_level_data["name"] = n

# ===== HELPERS =====

func _btn(text: String) -> Button:
	var b := Button.new()
	b.text = text
	b.add_theme_font_size_override("font_size", 14)
	return b

func _set_status(text: String) -> void:
	if _status_label:
		_status_label.text = text
