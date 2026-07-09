extends Control
class_name MapGraph

var _rooms: Dictionary = {}
var _links: Array = []
var _current_room_id: String = ""
var _seed_text: String = ""
var _campaign_name: String = ""
var _room_count: int = 0
var _pan_pixels: Vector2 = Vector2.ZERO
var _zoom: float = 1.0
var _base_scale: float = 18.0

const BACKGROUND := Color("#06080d")
const FRAME_A := Color("#FFD21A")
const FRAME_B := Color("#00D7FF")
const FRAME_C := Color("#FF00C8")
const ROOM_MAIN := Color("#8CFF00")
const ROOM_BRANCH := Color("#00D7FF")
const ROOM_LOCKED := Color("#FF8A00")
const ROOM_SPECIAL := Color("#F4F4F4")
const ROOM_CURRENT := Color("#FFFFFF")
const MIN_ZOOM := 0.35
const MAX_ZOOM := 2.75
const ZOOM_STEP := 0.15
const PAN_STEP := 48.0

func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS
	set_process_input(true)
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	clip_contents = true   # keep the map (and zoom overflow) inside the panel

func _input(event: InputEvent) -> void:
	if not visible:
		return
	if event is InputEventKey and event.pressed and not event.echo:
		match event.keycode:
			KEY_Z:
				_nudge_zoom(-ZOOM_STEP)
				get_viewport().set_input_as_handled()
			KEY_X:
				_nudge_zoom(ZOOM_STEP)
				get_viewport().set_input_as_handled()
			KEY_LEFT, KEY_A:
				_nudge_pan(Vector2(PAN_STEP, 0.0))
				get_viewport().set_input_as_handled()
			KEY_RIGHT, KEY_D:
				_nudge_pan(Vector2(-PAN_STEP, 0.0))
				get_viewport().set_input_as_handled()
			KEY_UP, KEY_W:
				_nudge_pan(Vector2(0.0, PAN_STEP))
				get_viewport().set_input_as_handled()
			KEY_DOWN, KEY_S:
				_nudge_pan(Vector2(0.0, -PAN_STEP))
				get_viewport().set_input_as_handled()

func set_map_data(rooms: Dictionary, links: Array, current_room_id: String, seed_text: String, campaign_name: String, room_count: int) -> void:
	_rooms = rooms.duplicate(true)
	_links = links.duplicate(true)
	_current_room_id = current_room_id
	_seed_text = seed_text
	_campaign_name = campaign_name
	_room_count = room_count
	# Deliberately does NOT reset _pan_pixels/_zoom — the MapPopup node is
	# created once and reused for the whole session (see main.gd), so leaving
	# these alone lets the player's zoom/pan position persist across opens.
	queue_redraw()

func _draw() -> void:
	var rect := Rect2(Vector2.ZERO, size)
	draw_rect(rect, BACKGROUND, true)
	_draw_frame(rect)
	if _rooms.is_empty():
		return

	var layout := _build_layout(rect)
	_draw_links(layout)
	_draw_rooms(layout)

func _draw_frame(rect: Rect2) -> void:
	draw_rect(rect.grow(-1.0), Color(0, 0, 0, 0), false, 2.0)
	var inset := rect.grow(-4.0)
	draw_rect(inset, Color(0, 0, 0, 0), false, 1.0)
	draw_line(inset.position, inset.position + Vector2(inset.size.x, 0), FRAME_A, 2.0)
	draw_line(inset.position + Vector2(0, inset.size.y), inset.position + inset.size, FRAME_B, 2.0)
	draw_line(inset.position, inset.position + Vector2(0, inset.size.y), FRAME_C, 2.0)
	draw_line(inset.position + Vector2(inset.size.x, 0), inset.position + inset.size, FRAME_A, 2.0)

func _build_layout(rect: Rect2) -> Dictionary:
	var min_pos := Vector2(INF, INF)
	var max_pos := Vector2(-INF, -INF)
	var points: Dictionary = {}

	for room_id in _rooms.keys():
		var room: Dictionary = _rooms[room_id]
		var pos := _room_map_position(room)
		points[room_id] = pos
		min_pos.x = minf(min_pos.x, pos.x)
		min_pos.y = minf(min_pos.y, pos.y)
		max_pos.x = maxf(max_pos.x, pos.x)
		max_pos.y = maxf(max_pos.y, pos.y)

	var span := Vector2(maxf(max_pos.x - min_pos.x, 1.0), maxf(max_pos.y - min_pos.y, 1.0))
	var padding := Vector2(28.0, 22.0)
	var usable := rect.size - (padding * 2.0)
	_base_scale = minf(usable.x / span.x, usable.y / span.y)
	_base_scale = clampf(_base_scale, 12.0, 22.0)
	var focus: Vector2 = points.get(_current_room_id, (min_pos + max_pos) * 0.5)
	return {
		"points": points,
		"focus": focus,
		"center": rect.position + (rect.size * 0.5)
	}

func _room_map_position(room: Dictionary) -> Vector2:
	if room.has("map_pos"):
		return _as_vec2(room.get("map_pos"))
	var role := str(room.get("role", "critical"))
	var index := int(room.get("depth", room.get("index", 0)))
	if role == "branch":
		var branch_step := int(room.get("branch_step", 0))
		var branch_direction := str(room.get("branch_direction", "north"))
		var branch_y := -(branch_step + 1) if branch_direction == "north" else (branch_step + 1)
		return Vector2(float(index), float(branch_y))
	return Vector2(float(index), 0.0)

func _draw_links(layout: Dictionary) -> void:
	var placements: Dictionary = layout.get("points", {})
	for link_variant in _links:
		var link: Dictionary = link_variant
		var from_id := str(link.get("from", ""))
		var to_id := str(link.get("to", ""))
		if not placements.has(from_id) or not placements.has(to_id):
			continue
		var from_pos: Vector2 = _map_to_screen(placements[from_id], layout)
		var to_pos: Vector2 = _map_to_screen(placements[to_id], layout)
		var requires: Array = link.get("requires", [])
		var color := ROOM_BRANCH
		if not requires.is_empty():
			color = ROOM_LOCKED
		elif from_id == "room_000" or to_id == "room_000":
			color = FRAME_A
		if from_id == _current_room_id or to_id == _current_room_id:
			color = ROOM_CURRENT
		draw_line(from_pos, to_pos, color, 2.0, true)
		if not requires.is_empty():
			var mid := from_pos.lerp(to_pos, 0.5)
			draw_rect(Rect2(mid - Vector2(3, 3), Vector2(6, 6)), color, true)

func _draw_rooms(layout: Dictionary) -> void:
	var placements: Dictionary = layout.get("points", {})
	# Draw current room highlight box first (so it appears behind other rooms)
	for room_id in _rooms.keys():
		if room_id == _current_room_id:
			var pos: Vector2 = _map_to_screen(placements.get(room_id, Vector2.ZERO), layout)
			draw_rect(Rect2(pos - Vector2(18, 18), Vector2(36, 36)), ROOM_CURRENT, false, 2.5)
			break

	for room_id in _rooms.keys():
		var room: Dictionary = _rooms[room_id]
		var pos: Vector2 = _map_to_screen(placements.get(room_id, Vector2.ZERO), layout)
		var role := str(room.get("role", "critical"))
		var is_resupply := bool(room.get("is_resupply", false))
		var is_current: bool = room_id == _current_room_id
		var fill := ROOM_MAIN
		if role == "branch":
			fill = ROOM_BRANCH
		if is_resupply:
			fill = ROOM_SPECIAL
		if room_id == "room_019":
			fill = ROOM_LOCKED
		if is_current:
			fill = ROOM_CURRENT
		var outline := FRAME_A if role == "critical" else FRAME_B
		if is_resupply:
			outline = FRAME_C
		if is_current:
			outline = ROOM_CURRENT
		var size := Vector2(7, 7)
		if is_current:
			size = Vector2(10, 10)
		draw_rect(Rect2(pos - (size * 0.5), size), fill, true)
		draw_rect(Rect2(pos - (size * 0.5), size), outline, false, 1.0)
		if room_id == "room_000":
			draw_rect(Rect2(pos - Vector2(5, 5), Vector2(10, 10)), FRAME_A, false, 1.0)
		if is_resupply:
			draw_circle(pos, 2.5, FRAME_C)

func _map_to_screen(map_pos: Vector2, layout: Dictionary) -> Vector2:
	var focus: Vector2 = layout.get("focus", Vector2.ZERO)
	var center: Vector2 = layout.get("center", Vector2.ZERO)
	var scale := _base_scale * _zoom
	return center + ((map_pos - focus) * scale) + _pan_pixels

func _nudge_pan(delta_pixels: Vector2) -> void:
	_pan_pixels += delta_pixels
	queue_redraw()

func _nudge_zoom(amount: float) -> void:
	_zoom = clampf(_zoom + amount, MIN_ZOOM, MAX_ZOOM)
	queue_redraw()

func _as_vec2(value: Variant) -> Vector2:
	if value is Vector2:
		return value
	if value is Array and value.size() >= 2:
		return Vector2(float(value[0]), float(value[1]))
	return Vector2.ZERO
