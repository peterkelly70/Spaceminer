extends RefCounted
class_name LevelSolvabilityValidator

const MAX_HORIZONTAL_GAP := 96.0
const MAX_VERTICAL_GAP := 64.0
const POINT_SUPPORT_X_PADDING := 12.0
const POINT_SUPPORT_Y_ABOVE := 32.0
const POINT_SUPPORT_Y_BELOW := 96.0

func validate_level(level_data: Dictionary, options: Dictionary = {}) -> Dictionary:
	var validate_reachability := bool(options.get("validate_reachability", true))
	var surfaces := _extract_surfaces(level_data)
	var errors: Array[String] = []
	var collectible_targets: Array = []
	var exit_targets: Array = []

	if surfaces.is_empty():
		errors.append("No walkable surfaces were found")
		return {
			"ok": false,
			"errors": errors,
			"reachable_surface_ids": []
		}

	var spawn_point := _as_vec2(level_data.get("spawn", [0, 0]))
	var spawn_surface := _find_support_surface(spawn_point, surfaces)
	if spawn_surface.is_empty():
		errors.append("Spawn is not grounded on a walkable surface")

	var exit_entries: Array = []
	if level_data.has("exits"):
		exit_entries = level_data.get("exits", [])
	else:
		exit_entries = [level_data.get("exit", {})]

	for index in range(exit_entries.size()):
		var exit_variant = exit_entries[index]
		if not (exit_variant is Dictionary):
			continue
		var exit_data: Dictionary = exit_variant
		var exit_name := str(exit_data.get("name", "Exit %d" % index))
		# South doors are dropped through (a hole in the floor), not stood on.
		if str(exit_data.get("direction", "")) == "south":
			continue
		var exit_point := _as_vec2(exit_data.get("position", [0, 0]))
		var exit_surface := _find_support_surface(exit_point, surfaces)
		if exit_surface.is_empty():
			errors.append("%s is not grounded on a walkable surface" % exit_name)
		else:
			exit_targets.append({
				"name": exit_name,
				"surface_id": int(exit_surface["id"])
			})

	for item_variant in level_data.get("collectibles", []):
		var item: Dictionary = item_variant
		var item_name := str(item.get("name", "Collectible"))
		var item_point := _as_vec2(item.get("position", [0, 0]))
		var support_surface := _find_support_surface(item_point, surfaces)
		if support_surface.is_empty():
			errors.append("%s has no walkable support beneath it" % item_name)
		else:
			collectible_targets.append({
				"name": item_name,
				"surface_id": int(support_surface["id"])
			})

	for item_variant in level_data.get("pickups", []):
		var item: Dictionary = item_variant
		var item_name := str(item.get("name", item.get("display_name", "Pickup")))
		var item_point := _as_vec2(item.get("position", [0, 0]))
		var support_surface := _find_support_surface(item_point, surfaces)
		if support_surface.is_empty():
			errors.append("%s has no walkable support beneath it" % item_name)
		else:
			collectible_targets.append({
				"name": item_name,
				"surface_id": int(support_surface["id"])
			})

	if spawn_surface.is_empty():
		return {
			"ok": false,
			"errors": errors,
			"reachable_surface_ids": []
		}

	var adjacency := _build_adjacency(surfaces)
	var reachable := _flood_fill(adjacency, int(spawn_surface["id"]))

	if validate_reachability:
		for target in exit_targets:
			if not reachable.has(int(target["surface_id"])):
				errors.append("%s is unreachable from spawn" % str(target["name"]))

		for target in collectible_targets:
			if not reachable.has(int(target["surface_id"])):
				errors.append("%s is unreachable from spawn" % str(target["name"]))

	return {
		"ok": errors.is_empty(),
		"errors": errors,
		"reachable_surface_ids": reachable.keys()
	}

func _extract_surfaces(level_data: Dictionary) -> Array:
	var surfaces: Array = []
	var solids: Array = level_data.get("solids", [])
	for i in range(solids.size()):
		var solid: Dictionary = solids[i]
		var solid_name := str(solid.get("name", "Solid")).to_lower()
		if solid_name.find("wall") >= 0:
			continue

		var size := _as_vec2(solid.get("size", [0, 0]))
		if size.x <= 0.0 or size.y <= 0.0:
			continue

		var walkable_hint := solid_name.find("floor") >= 0 or solid_name.find("bridge") >= 0 or solid_name.find("step") >= 0 or solid_name.find("ledge") >= 0 or solid_name.find("base") >= 0 or solid_name.find("run") >= 0 or solid_name.find("exit") >= 0
		if size.x < size.y and not walkable_hint:
			continue

		var position := _as_vec2(solid.get("position", [0, 0]))
		var anchor := str(solid.get("anchor", ""))
		var top_y := _surface_top_y(position, size, anchor, solid_name)
		var rect := Rect2(Vector2(position.x - (size.x * 0.5), top_y), size)
		surfaces.append({
			"id": i,
			"name": solid_name,
			"rect": rect,
			"top_y": top_y
		})
	return surfaces

func _build_adjacency(surfaces: Array) -> Dictionary:
	var adjacency: Dictionary = {}
	for surface in surfaces:
		adjacency[int(surface["id"])] = PackedInt32Array()

	for i in range(surfaces.size()):
		for j in range(i + 1, surfaces.size()):
			var a: Dictionary = surfaces[i]
			var b: Dictionary = surfaces[j]
			if _can_traverse(a, b):
				adjacency[int(a["id"])].append(int(b["id"]))
				adjacency[int(b["id"])].append(int(a["id"]))
	return adjacency

func _can_traverse(a: Dictionary, b: Dictionary) -> bool:
	var rect_a: Rect2 = a["rect"]
	var rect_b: Rect2 = b["rect"]
	var horizontal_gap := _rect_horizontal_gap(rect_a, rect_b)
	var vertical_gap := absf(float(a["top_y"]) - float(b["top_y"]))
	return horizontal_gap <= MAX_HORIZONTAL_GAP and vertical_gap <= MAX_VERTICAL_GAP

func _rect_horizontal_gap(a: Rect2, b: Rect2) -> float:
	if a.end.x < b.position.x:
		return b.position.x - a.end.x
	if b.end.x < a.position.x:
		return a.position.x - b.end.x
	return 0.0

func _find_support_surface(point: Vector2, surfaces: Array) -> Dictionary:
	var best_surface: Dictionary = {}
	var best_surface_y := -INF
	for surface in surfaces:
		var rect: Rect2 = surface["rect"]
		var top_y := float(surface["top_y"])
		if point.x < rect.position.x - POINT_SUPPORT_X_PADDING:
			continue
		if point.x > rect.end.x + POINT_SUPPORT_X_PADDING:
			continue
		if point.y < top_y - POINT_SUPPORT_Y_ABOVE:
			continue
		if point.y > top_y + POINT_SUPPORT_Y_BELOW:
			continue
		if top_y > best_surface_y:
			best_surface = surface
			best_surface_y = top_y
	return best_surface

func _flood_fill(adjacency: Dictionary, start_id: int) -> Dictionary:
	var visited: Dictionary = {}
	var queue: Array[int] = [start_id]
	while not queue.is_empty():
		var current: int = int(queue.pop_front())
		if visited.has(current):
			continue
		visited[current] = true
		for neighbor in adjacency.get(current, PackedInt32Array()):
			var neighbor_id := int(neighbor)
			if not visited.has(neighbor_id):
				queue.append(neighbor_id)
	return visited

func _as_vec2(value) -> Vector2:
	if value is Vector2:
		return value
	if value is Array and value.size() >= 2:
		return Vector2(float(value[0]), float(value[1]))
	return Vector2.ZERO

func _surface_top_y(position: Vector2, size: Vector2, anchor: String, solid_name: String) -> float:
	match anchor:
		"top", "north", "ceiling":
			return position.y
		"bottom", "south", "floor":
			return position.y - size.y
		"left", "west", "right", "east":
			return position.y - (size.y * 0.5)
		_:
			if size.y > size.x:
				return position.y - (size.y * 0.5)
			if solid_name.find("floor") >= 0 or solid_name.find("step") >= 0 or solid_name.find("ledge") >= 0 or solid_name.find("platform") >= 0:
				return position.y - size.y
			return position.y - size.y
