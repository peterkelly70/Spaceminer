extends "res://scripts/ui/popups/BasePopup.gd"
class_name MapPopup

@onready var _title_label: Label = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/TitleContainer/TitleLabel"
@onready var _content_label: Label = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/ContentLabel"
@onready var _map_graph: Control = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/MapGraph"
@onready var _info_label: Label = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/MapInfoLabel"
@onready var _close_button: Button = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/ConfirmButton"
@onready var _cancel_button: Button = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/CancelButton"

func _ready() -> void:
	# Always process — the game is not paused while the map is open
	process_mode = Node.PROCESS_MODE_ALWAYS
	can_close_with_escape = false
	super._ready()
	var retro_theme: Theme = load("res://assets/themes/retro_theme.tres")
	if retro_theme:
		theme = retro_theme
	_setup_ui()

func _setup_ui() -> void:
	if _title_label:
		_title_label.text = "ASTEROID MAP"
	if _content_label:
		_content_label.visible = false
	if _cancel_button:
		_cancel_button.visible = false
		_cancel_button.disabled = true
	if _close_button:
		_close_button.text = "CLOSE"
		if not _close_button.pressed.is_connected(_on_close_pressed):
			_close_button.pressed.connect(_on_close_pressed)
		_close_button.grab_focus()
	if _info_label:
		_info_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		_info_label.add_theme_font_size_override("font_size", 16)
		_info_label.add_theme_color_override("font_color", Color("#F4F4F4"))

func setup(seed_text: String, room_name: String = "", campaign_name: String = "", room_count: int = 0, manifest: Dictionary = {}) -> void:
	if _title_label:
		_title_label.text = "ASTEROID MAP"
	var active_manifest := manifest
	if active_manifest.is_empty() and RunManager and RunManager.has_method("get_active_campaign_manifest"):
		active_manifest = RunManager.get_active_campaign_manifest()
	var current_seed := seed_text if not seed_text.strip_edges().is_empty() else "Random"
	var current_campaign := campaign_name
	if current_campaign.strip_edges().is_empty():
		current_campaign = str(active_manifest.get("campaign_name", "Asteroid 56"))
	var current_room := room_name
	if current_room.strip_edges().is_empty():
		current_room = str(active_manifest.get("start_room_id", "room_000"))
	var current_room_count := room_count
	if current_room_count <= 0:
		current_room_count = int(active_manifest.get("room_count", 0))
	if _info_label:
		var info_lines: Array[String] = []
		info_lines.append("Seed: %s" % current_seed)
		info_lines.append("Campaign: %s" % current_campaign)
		info_lines.append("Room: %s" % current_room)
		info_lines.append("Rooms: %d" % current_room_count)
		info_lines.append("WASD/Arrows pan | Z/X zoom | M or Jump closes | Esc pauses")
		_info_label.text = "\n".join(info_lines)
	if _map_graph:
		_map_graph.set_map_data(
			active_manifest.get("rooms", {}),
			active_manifest.get("map_links", []),
			current_room,
			current_seed,
			current_campaign,
			current_room_count
		)
	_setup_ui()

func _on_close_pressed() -> void:
	hide()
	get_tree().paused = false

func _input(event: InputEvent) -> void:
	if not visible:
		return
	if event is InputEventKey and event.pressed and not event.echo:
		if event.keycode == KEY_M:
			_on_close_pressed()
			get_viewport().set_input_as_handled()
			return
	# M or Jump closes the map
	if event.is_action_pressed("map_toggle") or event.is_action_pressed("jump"):
		_on_close_pressed()
		get_viewport().set_input_as_handled()
