extends Node2D
class_name LevelBuilder

@export_file("*.json") var level_json_path: String = "res://assets/levels/landing_bay_alpha.json"
@export var auto_build: bool = true

signal level_built(level_data: Dictionary)

const TILESET_PATH := "res://assets/tiles/space_miner_tileset.tres"
const TILE_ATLAS_SIZE := 20
const LADDER_SCRIPT := preload("res://scripts/prototype/ladder.gd")

var _last_level_data: Dictionary = {}

func _ready() -> void:
	if auto_build:
		build_from_json(level_json_path)

func build_from_json(json_path: String) -> void:
	if json_path.is_empty():
		push_error("LevelBuilder: empty JSON path")
		return

	var text: String = FileAccess.get_file_as_string(json_path)
	if text.is_empty():
		push_error("LevelBuilder: could not read %s" % json_path)
		return

	var parsed: Variant = JSON.parse_string(text)
	if typeof(parsed) != TYPE_DICTIONARY:
		push_error("LevelBuilder: invalid JSON in %s" % json_path)
		return

	var data: Dictionary = parsed
	_last_level_data = data.duplicate(true)
	_clear_children()

	var room := get_parent()
	if room:
		var room_model = room.get("room_model")
		if room_model:
			room_model.room_name = str(data.get("room_name", data.get("name", room.name)))

	var tile_set: TileSet = load(TILESET_PATH)
	if not tile_set:
		push_error("LevelBuilder: missing tileset at %s" % TILESET_PATH)
		return

	_build_tile_layers(data, tile_set)
	_build_solids(data)
	_build_ladders(data)
	_build_npcs(data)
	_build_decor(data)
	_build_spawn_and_exit(data)
	_build_collectibles(data)
	_build_pickups(data)
	_build_hazards(data)
	_build_enemies(data)
	_build_mechanisms(data)
	_build_teleporters(data)
	_build_exits(data)
	level_built.emit(_last_level_data.duplicate(true))

func _clear_children() -> void:
	for child in get_children():
		child.queue_free()

func _build_tile_layers(data: Dictionary, tile_set: TileSet) -> void:
	var layers: Array = data.get("tile_layers", [])
	for layer_data_variant in layers:
		var layer_data: Dictionary = layer_data_variant
		var tml := TileMapLayer.new()
		tml.name = str(layer_data.get("name", "TileLayer"))
		tml.tile_set = tile_set
		tml.modulate = _as_color(layer_data.get("modulate", "#ffffff"))
		tml.z_index = int(layer_data.get("z_index", 0))
		tml.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
		add_child(tml)

		var cells: Array = layer_data.get("cells", [])
		for cell_variant in cells:
			var cell: Dictionary = cell_variant
			var x := int(cell.get("x", 0))
			var y := int(cell.get("y", 0))
			var tile_index := int(cell.get("tile", 0))
			tml.set_cell(Vector2i(x, y), 0, _idx_to_coords(tile_index))

func _build_solids(data: Dictionary) -> void:
	var solids: Array = data.get("solids", [])
	for solid_variant in solids:
		var solid: Dictionary = solid_variant
		var body := StaticBody2D.new()
		body.name = str(solid.get("name", "Solid"))
		var size := _as_vec2(solid.get("size", [32, 16]))
		# Generated rooms tag the kind on "kind"; older authored rooms use "type".
		var solid_type := str(solid.get("type", solid.get("kind", ""))).to_lower()
		var anchor := str(solid.get("anchor", ""))
		if anchor.is_empty():
			anchor = _default_solid_anchor(solid_type, body.name, size)
		body.position = _solid_body_position(_as_vec2(solid.get("position", [0, 0])), size, anchor)
		add_child(body)

		var shape_node := CollisionShape2D.new()
		var rect := RectangleShape2D.new()
		rect.size = size
		shape_node.shape = rect
		shape_node.position = Vector2.ZERO
		var solid_name := body.name.to_lower()
		# "platform" kind (or an explicit one_way flag) = one-way; everything else blocks all movement
		var is_platform := solid_type == "platform" or bool(solid.get("one_way", false))
		shape_node.one_way_collision = is_platform
		shape_node.one_way_collision_margin = 1.0
		# One-way platforms get their own collision layer (2) so the player can
		# drop through them on demand without falling through solid floor/walls (1).
		body.collision_layer = 2 if is_platform else 1
		body.add_child(shape_node)

func _build_ladders(data: Dictionary) -> void:
	for ladder_variant in data.get("ladders", []):
		var ladder_data: Dictionary = ladder_variant
		var size := _as_vec2(ladder_data.get("size", [14, 64]))
		var ladder := LADDER_SCRIPT.new()
		ladder.name = str(ladder_data.get("name", "Ladder"))
		ladder.width = size.x
		ladder.height = size.y
		ladder.position = _as_vec2(ladder_data.get("position", [0, 0]))
		add_child(ladder)

func _build_npcs(data: Dictionary) -> void:
	for npc_variant in data.get("npcs", []):
		var npc_data: Dictionary = npc_variant
		var scene := load(str(npc_data.get("scene", "")))
		if not scene:
			continue
		var inst: Node2D = scene.instantiate()
		inst.name = str(npc_data.get("name", "Npc"))
		inst.position = _as_vec2(npc_data.get("position", [0, 0]))
		var props: Dictionary = npc_data.get("props", {})
		for key in props.keys():
			inst.set(str(key), props[key])
		add_child(inst)

func _build_decor(data: Dictionary) -> void:
	var decor: Array = data.get("decor", [])
	for decor_variant in decor:
		var item: Dictionary = decor_variant
		var kind := str(item.get("type", "sprite"))
		if kind != "sprite":
			continue
		var sprite := Sprite2D.new()
		sprite.name = str(item.get("name", "Decor"))
		sprite.position = _as_vec2(item.get("position", [0, 0]))
		sprite.texture = load(str(item.get("texture", "")))
		sprite.scale = _as_vec2(item.get("scale", [1.0, 1.0]))
		add_child(sprite)

func _build_spawn_and_exit(data: Dictionary) -> void:
	var room := get_parent()
	if not room:
		return

	var exit_data: Dictionary = data.get("exit", {})
	var exit_zone: Area2D = room.get_node_or_null("ExitZone")
	if exit_zone:
		exit_zone.position = _as_vec2(exit_data.get("position", [272, -64]))
		exit_zone.collision_layer = 0
		exit_zone.collision_mask = 1
		exit_zone.monitorable = true
		_clear_node_children(exit_zone)

		var exit_shape := CollisionShape2D.new()
		var exit_rect := RectangleShape2D.new()
		exit_rect.size = _as_vec2(exit_data.get("size", [32, 64]))
		exit_shape.shape = exit_rect
		exit_zone.add_child(exit_shape)

		var exit_visual := AnimatedSprite2D.new()
		exit_visual.name = "Visual"
		exit_visual.sprite_frames = load(str(exit_data.get("sprite_frames", "res://assets/sprite/space_miner_teleporter_frames.tres")))
		exit_visual.animation = StringName(str(exit_data.get("animation", "cycle")))
		exit_visual.autoplay = StringName(str(exit_data.get("animation", "cycle")))
		exit_visual.scale = _as_vec2(exit_data.get("scale", [2.0, 2.0]))
		exit_zone.add_child(exit_visual)

	var respawn: Marker2D = room.get_node_or_null("RespawnPoint")
	if respawn:
		respawn.position = _as_vec2(data.get("spawn", [-208, 112])) + Vector2(0, -2)

	var player: Node2D = room.get_node_or_null("Player")
	if player:
		player.position = _as_vec2(data.get("spawn", [-208, 112])) + Vector2(0, -2)

func _build_collectibles(data: Dictionary) -> void:
	var room := get_parent()
	if not room:
		return
	var root: Node = room.get_node_or_null("Collectibles")
	if not root:
		return
	_clear_node_children(root)
	for item_variant in data.get("collectibles", []):
		var item: Dictionary = item_variant
		_add_instance(root, item)

func _build_pickups(data: Dictionary) -> void:
	var room := get_parent()
	if not room:
		return
	var root: Node = room.get_node_or_null("Pickups")
	if not root:
		return
	_clear_node_children(root)
	for item_variant in data.get("pickups", []):
		var item: Dictionary = item_variant
		_add_instance(root, item)

func _build_hazards(data: Dictionary) -> void:
	var room := get_parent()
	if not room:
		return
	var root: Node = room.get_node_or_null("Hazards")
	if not root:
		return
	_clear_node_children(root)
	# Hazards are stationary, so one landing on a door's approach can block
	# entry outright (worse than a roaming enemy near the same spot) — use a
	# wider exclusion margin than enemies get.
	var door_rects := _collect_door_rects(data, 32.0, 28.0)
	for item_variant in data.get("hazards", []):
		var item: Dictionary = item_variant
		if _item_overlaps_door(item, door_rects):
			continue
		_add_instance(root, item)

func _build_enemies(data: Dictionary) -> void:
	var room := get_parent()
	if not room:
		return
	var root: Node = room.get_node_or_null("Enemies")
	if not root:
		return
	_clear_node_children(root)
	var door_rects := _collect_door_rects(data)
	for item_variant in data.get("enemies", []):
		var item: Dictionary = item_variant
		if _item_overlaps_door(item, door_rects):
			continue
		var node := _add_instance(root, item)
		if node and item.has("props"):
			var props: Dictionary = item["props"]
			for key in props.keys():
				var value = props[key]
				if value is Array:
					node.set(key, PackedVector2Array(_array_to_vector2_array(value)))
				else:
					node.set(key, value)

func _build_mechanisms(data: Dictionary) -> void:
	var room := get_parent()
	if not room:
		return
	var root: Node = room.get_node_or_null("Mechanisms")
	if not root:
		root = Node.new()
		root.name = "Mechanisms"
		room.add_child(root)
	_clear_node_children(root)
	for item_variant in data.get("mechanisms", []):
		var item: Dictionary = item_variant
		_add_instance(root, item)

func _build_teleporters(data: Dictionary) -> void:
	var room := get_parent()
	if not room:
		return
	var root: Node = room.get_node_or_null("Teleporters")
	if not root:
		root = Node.new()
		root.name = "Teleporters"
		room.add_child(root)
	_clear_node_children(root)
	for item_variant in data.get("teleporters", []):
		var item: Dictionary = item_variant
		var node := _add_instance(root, item)
		if node and item.has("props"):
			var props: Dictionary = item["props"]
			for key in props.keys():
				node.set(str(key), props[key])

func _build_exits(data: Dictionary) -> void:
	var room := get_parent()
	if not room:
		return
	var root: Node = room.get_node_or_null("Doors")
	if not root:
		return
	_clear_node_children(root)

	var exit_entries: Array = []
	if data.has("exits"):
		exit_entries = data.get("exits", [])
	elif data.has("exit"):
		exit_entries = [data.get("exit", {})]

	for item_variant in exit_entries:
		var item: Dictionary = item_variant
		_add_instance(root, item)

func _collect_door_rects(data: Dictionary, margin_x: float = 16.0, margin_y: float = 12.0) -> Array:
	var door_rects: Array = []
	var entries: Array = []
	if data.has("exits"):
		entries = data.get("exits", [])
	elif data.has("exit"):
		entries = [data.get("exit", {})]
	for item_variant in entries:
		var item: Dictionary = item_variant
		var pos := _as_vec2(item.get("position", [0, 0]))
		var size := _door_size_for_exit(item)
		# Expand slightly so nothing sits directly in the door opening or on
		# the tiny landing just outside it.
		var rect := Rect2(pos - (size * 0.5) - Vector2(margin_x, margin_y), size + Vector2(margin_x, margin_y) * 2.0)
		door_rects.append(rect)
	return door_rects

func _item_overlaps_door(item: Dictionary, door_rects: Array) -> bool:
	if door_rects.is_empty():
		return false
	var pos := _as_vec2(item.get("position", [0, 0]))
	for rect_variant in door_rects:
		var rect: Rect2 = rect_variant
		if rect.has_point(pos):
			return true
	return false

func _add_instance(parent: Node, item: Dictionary) -> Node2D:
	var scene_path := str(item.get("scene", ""))
	if scene_path.is_empty():
		return null
	var scene := load(scene_path)
	if not scene:
		push_warning("LevelBuilder: missing scene %s" % scene_path)
		return null
	var instance: Node2D = scene.instantiate()
	instance.name = str(item.get("name", "Instance"))
	var item_position := _as_vec2(item.get("position", [0, 0]))
	var item_size := _as_vec2(item.get("size", [0, 0]))
	var anchor := str(item.get("anchor", ""))
	if not anchor.is_empty() and item_size != Vector2.ZERO:
		instance.position = _solid_body_position(item_position, item_size, anchor)
	else:
		instance.position = item_position
	if item.has("scale"):
		instance.scale = _as_vec2(item.get("scale", [1.0, 1.0]))
	parent.add_child(instance)
	if instance.has_method("configure"):
		instance.call("configure", item)
	elif item.has("props"):
		var props: Dictionary = item.get("props", {})
		for key in props.keys():
			instance.set(str(key), props[key])
	return instance

func _clear_node_children(node: Node) -> void:
	for child in node.get_children():
		child.queue_free()

func _solid_body_position(position: Vector2, size: Vector2, anchor: String) -> Vector2:
	match anchor:
		"top", "north", "ceiling":
			return position + Vector2(0, size.y * 0.5)
		"bottom", "south", "floor":
			return position - Vector2(0, size.y * 0.5)
		"left", "west":
			return position + Vector2(size.x * 0.5, 0)
		"right", "east":
			return position - Vector2(size.x * 0.5, 0)
		_:
			return position

func _default_solid_anchor(solid_type: String, solid_name: String, size: Vector2) -> String:
	if solid_type in ["floor", "platform", "step", "ledge"]:
		return "top"
	if solid_type == "ceiling":
		return "bottom"
	if solid_type == "wall":
		return "center"
	# Fall back to a simple shape heuristic for authored solids that did not
	# specify an anchor explicitly.
	if size.x >= size.y * 1.5:
		return "top"
	return "center"

func _is_one_way_solid(kind: String, solid_name: String, anchor: String) -> bool:
	if kind in ["floor", "platform", "ceiling", "step", "ledge"]:
		return true
	if anchor in ["top", "bottom"]:
		return true
	return solid_name.find("floor") >= 0 or solid_name.find("step") >= 0 or solid_name.find("platform") >= 0 or solid_name.find("ledge") >= 0

func _idx_to_coords(index: int) -> Vector2i:
	return Vector2i(index % TILE_ATLAS_SIZE, index / TILE_ATLAS_SIZE)

func _as_vec2(value) -> Vector2:
	if value is Vector2:
		return value
	if value is Array and value.size() >= 2:
		return Vector2(float(value[0]), float(value[1]))
	return Vector2.ZERO

func _as_color(value) -> Color:
	if value is Color:
		return value
	return Color(str(value))

func _door_size_for_exit(item: Dictionary) -> Vector2:
	var direction := str(item.get("direction", "")).to_lower()
	if direction in ["north", "south", "up", "down"]:
		return _as_vec2(item.get("size", Vector2(48, 16)))
	return _as_vec2(item.get("size", Vector2(16, 48)))

func _array_to_vector2_array(values: Array) -> Array[Vector2]:
	var result: Array[Vector2] = []
	for value in values:
		if value is Array and value.size() >= 2:
			result.append(Vector2(float(value[0]), float(value[1])))
	return result
