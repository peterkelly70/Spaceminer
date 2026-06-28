extends StateAwareController
class_name MainGameController

const ROOM_JSON := {
	"Landing Bay Alpha": "res://assets/levels/landing_bay_alpha.json",
	"Control Hub":       "res://assets/levels/control_hub.json",
	"Ore Shaft":         "res://assets/levels/ore_shaft.json",
}
const GENERATED_ROOM_SCENE := preload("res://scenes/levels/GeneratedAsteroidRoom.tscn")
const GAME_OVER_POPUP_SCENE := preload("res://scenes/ui/popups/GameOverPopup.tscn")
const EQUIPMENT_CARD_SCENE := preload("res://scenes/ui/EquipmentStatusCard.tscn")
const SECURITY_CARDS := [
	{"item_id": "security_cyan", "icon_path": "res://assets/tiles/Transparent/tile_0400.png", "accent_color": Color("#00D7FF")},
	{"item_id": "security_blue", "icon_path": "res://assets/tiles/Transparent/tile_0400.png", "accent_color": Color("#1D4FFF")},
	{"item_id": "security_green", "icon_path": "res://assets/tiles/Transparent/tile_0400.png", "accent_color": Color("#00D66B")},
	{"item_id": "security_yellow", "icon_path": "res://assets/tiles/Transparent/tile_0400.png", "accent_color": Color("#FFD21A")},
	{"item_id": "security_orange", "icon_path": "res://assets/tiles/Transparent/tile_0400.png", "accent_color": Color("#FF8A00")},
	{"item_id": "security_red", "icon_path": "res://assets/tiles/Transparent/tile_0400.png", "accent_color": Color("#FF1E2D")},
]
const SUIT_UPGRADES := [
	{"item_id": "jetpack", "icon_path": "res://assets/tiles/Transparent/tile_0401.png", "accent_color": Color("#00D7FF")},
	{"item_id": "laser_pistol", "icon_path": "res://assets/tiles/Transparent/tile_0402.png", "accent_color": Color("#FF1E2D")},
	{"item_id": "grappling_hook", "icon_path": "res://assets/tiles/Transparent/tile_0403.png", "accent_color": Color("#FFD21A")},
	{"item_id": "magnetic_boots", "icon_path": "res://assets/tiles/Transparent/tile_0404.png", "accent_color": Color("#8CFF00")},
	{"item_id": "visibility_cloak", "icon_path": "res://assets/tiles/Transparent/tile_0407.png", "accent_color": Color("#FF00C8")},
	{"item_id": "shield", "icon_path": "res://assets/tiles/Transparent/tile_0408.png", "accent_color": Color("#1D4FFF")},
]

@onready var viewport: SubViewport = $Layout/ContentArea/ViewportContainer/GameplayViewport
@onready var viewport_container: SubViewportContainer = $Layout/ContentArea/ViewportContainer
@onready var overlay_label: Label = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/MissionLabel
@onready var lives_root: HBoxContainer = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Lives
@onready var ore_label: Label = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Ore/OreLabel
@onready var ore_icons_root: GridContainer = get_node_or_null("Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Ore/HBoxContainer") as GridContainer
@onready var air_label: Label = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Air/AirLabel
@onready var air_icon: TextureRect = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Air/AirIcon
@onready var air_bar: ProgressBar = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Air/ProgressBar
@onready var jetpack_root: HBoxContainer = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Jetpack
@onready var jetpack_icon: TextureRect = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Jetpack/JetIcon
@onready var jetpack_bar: ProgressBar = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Jetpack/ProgressBar
@onready var fuel_root: HBoxContainer = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Fuel
@onready var fuel_icon: TextureRect = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Fuel/FuelIcon
@onready var fuel_bar: ProgressBar = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Fuel/ProgressBar
@onready var battery_root: HBoxContainer = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Battery
@onready var battery_icon: TextureRect = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Battery/BatteryIcon
@onready var battery_bar: ProgressBar = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Consumables/Battery/ProgressBar
@onready var security_cards_grid: GridContainer = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/EquipmentStrip/SecurityCards
@onready var upgrade_cards_grid: GridContainer = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/EquipmentStrip/UpgradeCards
@onready var score_label: Label = $Layout/StatusArea/StatusPanel/StatusMargin/StatusRow/Status/Statusbox/ScoreValue

var room_instance: Node2D = null
var current_room_id: String = "room_000"
var _game_over_popup: Control = null
var _game_over_active: bool = false
var _room_transition_locked: bool = false
var _room_transition_request_id: int = 0

func _ready() -> void:
	print("MainGameController ready")
	show_in_states = [AppState.State.PLAYING, AppState.State.PAUSE]
	super._ready()
	_configure_viewport()
	_setup_consumable_icons()
	_setup_card_grids()
	if ore_icons_root:
		ore_icons_root.visible = false
	if jetpack_root:
		jetpack_root.visible = false
	if fuel_root:
		fuel_root.visible = false
	if battery_root:
		battery_root.visible = false
	if Audio_Manager and Audio_Manager.has_method("ensure_music_playing"):
		Audio_Manager.ensure_music_playing()
	_ensure_game_over_popup()

func _configure_viewport() -> void:
	if not viewport:
		push_error("Gameplay viewport missing")
		return
	if viewport_container:
		viewport_container.stretch = true
	viewport.disable_3d = true
	viewport.render_target_clear_mode = 0
	viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	viewport.transparent_bg = false
	if RenderingServer.has_method("set_default_clear_color"):
		RenderingServer.set_default_clear_color(Color.BLACK)

func _setup_consumable_icons() -> void:
	_set_texture_from_file(air_icon, "res://assets/tiles/Transparent/tile_0405.png")
	_set_texture_from_file(jetpack_icon, "res://assets/tiles/Transparent/tile_0401.png")
	_set_texture_from_file(fuel_icon, "res://assets/tiles/Transparent/tile_0410.png")
	_set_texture_from_file(battery_icon, "res://assets/tiles/Transparent/tile_0409.png")

func _set_texture_from_file(node: TextureRect, path: String) -> void:
	if not node:
		return
	var texture := load(path)
	if texture is Texture2D:
		node.texture = texture
		node.stretch_mode = TextureRect.STRETCH_KEEP_CENTERED
		node.expand_mode = TextureRect.EXPAND_FIT_WIDTH_PROPORTIONAL
		return
	var image := Image.new()
	var err := image.load(path)
	if err != OK:
		push_warning("Failed to load status icon: %s (%s)" % [path, err])
		return
	node.texture = ImageTexture.create_from_image(image)
	node.stretch_mode = TextureRect.STRETCH_KEEP_CENTERED
	node.expand_mode = TextureRect.EXPAND_FIT_WIDTH_PROPORTIONAL

func _ensure_room_loaded() -> void:
	if room_instance:
		return
	if RunManager and RunManager.has_method("start_new_run") and RunManager.active_run.is_empty():
		RunManager.start_new_run("")
	if RunManager and RunManager.has_method("ensure_campaign_for_active_seed"):
		RunManager.ensure_campaign_for_active_seed()
	if RunManager and RunManager.has_method("get_current_room_id"):
		current_room_id = RunManager.get_current_room_id()
	_load_room(current_room_id)
	_refresh_card_grids()

func _ensure_game_over_popup() -> void:
	if _game_over_popup and is_instance_valid(_game_over_popup):
		return
	_game_over_popup = GAME_OVER_POPUP_SCENE.instantiate()
	add_child(_game_over_popup)
	_game_over_popup.visible = false
	if _game_over_popup.has_signal("restart_requested") and not _game_over_popup.restart_requested.is_connected(_on_game_over_restart_requested):
		_game_over_popup.restart_requested.connect(_on_game_over_restart_requested)
	if _game_over_popup.has_signal("menu_requested") and not _game_over_popup.menu_requested.is_connected(_on_game_over_menu_requested):
		_game_over_popup.menu_requested.connect(_on_game_over_menu_requested)

func _load_room(room_id: String, entry_direction: String = "") -> void:
	var room_path := ""
	if RunManager and RunManager.has_method("get_room_path"):
		room_path = RunManager.get_room_path(room_id)
	if room_path.is_empty():
		room_path = ROOM_JSON.get(room_id, "")
	if room_path.is_empty():
		push_error("No room JSON for id: %s" % room_id)
		_room_transition_locked = false
		return
	_load_generated_room(room_id, room_path, entry_direction)

func _load_generated_room(room_id: String, room_path: String, entry_direction: String = "") -> void:
	_clear_room()
	current_room_id = room_id
	room_instance = GENERATED_ROOM_SCENE.instantiate()
	if room_instance.has_method("configure_room"):
		room_instance.call("configure_room", room_path, room_id, entry_direction)
	elif room_instance.has_method("set"):
		room_instance.set("room_json_path", room_path)
		room_instance.set("current_room_id", room_id)
	viewport.add_child(room_instance)
	overlay_label.text = room_id
	if RunManager and RunManager.has_method("set_current_room"):
		RunManager.set_current_room(room_id, false)
	if room_instance.has_signal("request_room_change"):
		room_instance.connect("request_room_change", Callable(self, "_on_room_change_requested"))
	if room_instance.has_signal("room_status_changed"):
		room_instance.connect("room_status_changed", Callable(self, "_on_room_status_changed"))
		if room_instance.has_method("_emit_room_status"):
			room_instance.call_deferred("_emit_room_status")
	print("Generated gameplay room instanced: %s from %s" % [room_id, room_path])
	if _room_transition_locked:
		_unlock_room_transition_after_spawn()

func on_enter_state(_state: int) -> void:
	visible = true
	_ensure_room_loaded()

func on_exit_state(_state: int) -> void:
	visible = false
	_clear_room()
	print("Gameplay room cleared on exit")

func _exit_tree() -> void:
	_clear_room()

func _clear_room() -> void:
	if room_instance and is_instance_valid(room_instance):
		room_instance.queue_free()
	room_instance = null

func _on_room_change_requested(next_room_id: String, entry_direction: String = "") -> void:
	if _room_transition_locked:
		print("Ignoring room transition while locked: %s" % next_room_id)
		return
	_room_transition_locked = true
	_room_transition_request_id += 1
	overlay_label.text = next_room_id
	print("Room requested transition to: %s" % next_room_id)
	if next_room_id == "campaign_complete":
		if RunManager and RunManager.has_method("set_current_room"):
			RunManager.set_current_room(next_room_id)
		_set_campaign_complete_state()
		_room_transition_locked = false
		return
	if RunManager and RunManager.has_method("set_current_room"):
		RunManager.set_current_room(next_room_id)
	call_deferred("_load_room", next_room_id, entry_direction)

func _unlock_room_transition_after_spawn() -> void:
	var request_id := _room_transition_request_id
	await get_tree().create_timer(0.45).timeout
	if request_id == _room_transition_request_id:
		_room_transition_locked = false

func _on_room_status_changed(status: Dictionary) -> void:
	if status.has("room_name"):
		overlay_label.text = str(status["room_name"])
	if status.has("ore_collected") and status.has("ore_total"):
		var ore_count := int(status.get("ore_count", status["ore_collected"]))
		ore_label.text = "Ore %d" % ore_count
	if status.has("oxygen_percent"):
		var oxygen_percent := int(status["oxygen_percent"])
		air_label.text = "Air"
		air_bar.value = oxygen_percent
	if status.has("jetpack_percent") or status.has("jetpack_unlocked") or status.has("has_jetpack"):
		if jetpack_root:
			jetpack_root.visible = true
		if status.has("jetpack_percent") and jetpack_bar:
			jetpack_bar.value = int(status["jetpack_percent"])
		elif status.has("has_jetpack") and jetpack_bar:
			jetpack_bar.value = 100 if bool(status["has_jetpack"]) else 0
		elif status.has("jetpack_unlocked") and jetpack_bar:
			jetpack_bar.value = 100 if bool(status["jetpack_unlocked"]) else 0
	elif jetpack_root:
		jetpack_root.visible = false
	if status.has("fuel_pct"):
		if fuel_root:
			fuel_root.visible = true
		if fuel_bar:
			if status.has("fuel_max_pct"):
				fuel_bar.max_value = float(status["fuel_max_pct"])
			fuel_bar.value = float(status["fuel_pct"])
	if status.has("battery_pct"):
		if battery_root:
			battery_root.visible = true
		if battery_bar:
			battery_bar.value = float(status["battery_pct"])
	if status.has("lives_remaining"):
		var lives_remaining := int(status["lives_remaining"])
		_update_life_icons(lives_remaining)
		if lives_remaining <= 0 and not _game_over_active:
			_open_game_over_popup(str(status.get("room_name", current_room_id)))
	if status.has("score") and score_label:
		score_label.text = "%d" % int(status["score"])
	_refresh_card_grids()

func _set_campaign_complete_state() -> void:
	room_model_tip("Exit reached. Returning to menu.")
	call_deferred("_return_to_menu_after_exit")

func _return_to_menu_after_exit() -> void:
	State_Manager.change_state(AppState.State.MAIN_MENU)

func _open_game_over_popup(room_name: String = "") -> void:
	_ensure_game_over_popup()
	if not _game_over_popup:
		return
	_game_over_active = true
	get_tree().paused = true
	_game_over_popup.visible = true
	if _game_over_popup.has_method("setup"):
		var seed_text := ""
		if RunManager and RunManager.has_method("get_active_seed_display"):
			seed_text = RunManager.get_active_seed_display()
		_game_over_popup.call("setup", seed_text, room_name)

func _close_game_over_popup() -> void:
	_game_over_active = false
	get_tree().paused = false
	if _game_over_popup:
		_game_over_popup.visible = false

func _on_game_over_restart_requested() -> void:
	var seed_text := ""
	if RunManager and RunManager.has_method("get_active_seed"):
		seed_text = RunManager.get_active_seed()
	if RunManager and RunManager.has_method("start_new_run"):
		RunManager.start_new_run(seed_text)
	_close_game_over_popup()
	_clear_room()
	current_room_id = RunManager.get_current_room_id() if RunManager and RunManager.has_method("get_current_room_id") else "room_000"
	_ensure_room_loaded()

func _on_game_over_menu_requested() -> void:
	_close_game_over_popup()
	State_Manager.change_state(AppState.State.MAIN_MENU)

func room_model_tip(message: String) -> void:
	if room_instance and room_instance.has_method("_on_tip_changed"):
		room_instance.call("_on_tip_changed", message)

func _update_life_icons(lives_remaining: int) -> void:
	var index := 0
	for child in lives_root.get_children():
		if child.has_method("set_alive"):
			child.call("set_alive", index < lives_remaining)
		index += 1

func _update_ore_icons(ore_collected: int) -> void:
	pass

func _setup_card_grids() -> void:
	_setup_card_grid(security_cards_grid, SECURITY_CARDS, true)
	_setup_card_grid(upgrade_cards_grid, SUIT_UPGRADES, false)

func _setup_card_grid(grid: GridContainer, entries: Array, is_security: bool) -> void:
	if not grid:
		return
	for child in grid.get_children():
		child.free()
	for card_info in entries:
		var card := EQUIPMENT_CARD_SCENE.instantiate()
		grid.add_child(card)
		if card.has_method("configure"):
			var item_id := str(card_info.get("item_id", ""))
			card.call(
				"configure",
				item_id,
				str(card_info.get("icon_path", "")),
				card_info.get("accent_color", Color.WHITE),
				_is_card_owned(item_id, is_security)
			)

func _refresh_card_grids() -> void:
	_refresh_card_grid(security_cards_grid, SECURITY_CARDS, true)
	_refresh_card_grid(upgrade_cards_grid, SUIT_UPGRADES, false)

func _refresh_card_grid(grid: GridContainer, entries: Array, is_security: bool) -> void:
	if not grid:
		return
	var index := 0
	for child in grid.get_children():
		if index >= entries.size():
			child.visible = false
			index += 1
			continue
		var card_info: Dictionary = entries[index]
		if child.has_method("set_owned"):
			child.call("set_owned", _is_card_owned(str(card_info.get("item_id", "")), is_security))
		index += 1

func _is_card_owned(item_id: String, is_security: bool) -> bool:
	if item_id.is_empty():
		return false
	if not RunManager:
		return false
	if is_security:
		if RunManager.has_method("has_security_card"):
			return RunManager.has_security_card(item_id)
		return false
	if RunManager.has_method("has_equipment"):
		return RunManager.has_equipment(item_id)
	return false
