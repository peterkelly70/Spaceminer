extends Area2D
class_name DwarfMiner

# A friendly dwarf miner NPC. When the player is near, an interact prompt shows;
# pressing interact opens a retro-styled dialogue box. For now he just says hi.

const RETRO_THEME_PATH := "res://assets/themes/retro_theme.tres"

@export_multiline var dialogue: String = "Arr! Another spaceman in me tunnels. Mind the spikes an' dig deep, lad!"
@export var speaker_name: String = "DWARF MINER"
@export var quest_item_tile: int = 96          # tile index for quest item sprite
@export var quest_reward: int = 250            # ore given when quest item collected
@export var quest_enabled: bool = true         # whether to spawn a quest item

var _player_near := false
var _dialog_layer: CanvasLayer = null
var _quest_item_spawned := false

@onready var prompt: Label = $Prompt if has_node("Prompt") else null

func _ready() -> void:
	add_to_group("npc")
	body_entered.connect(_on_body_entered)
	body_exited.connect(_on_body_exited)
	if prompt:
		prompt.visible = false

func _on_body_entered(body: Node) -> void:
	if body.is_in_group("player") or body.is_in_group("prototype_player"):
		_player_near = true
		if prompt and not _is_open():
			prompt.visible = true

func _on_body_exited(body: Node) -> void:
	if body.is_in_group("player") or body.is_in_group("prototype_player"):
		_player_near = false
		if prompt:
			prompt.visible = false
		_close()

func _unhandled_input(event: InputEvent) -> void:
	if not _player_near:
		return
	if not event.is_pressed() or event.is_echo():
		return
	if event.is_action_pressed("interact"):
		if _is_open():
			_close()
		else:
			_open()
		get_viewport().set_input_as_handled()

func _is_open() -> bool:
	return _dialog_layer != null and is_instance_valid(_dialog_layer)

func _open() -> void:
	if _is_open():
		return
	if prompt:
		prompt.visible = false
	var theme: Theme = load(RETRO_THEME_PATH)

	_dialog_layer = CanvasLayer.new()
	_dialog_layer.layer = 60

	var dim := ColorRect.new()
	dim.color = Color(0, 0, 0, 0.55)
	dim.set_anchors_preset(Control.PRESET_FULL_RECT)
	dim.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_dialog_layer.add_child(dim)

	var center := CenterContainer.new()
	center.set_anchors_preset(Control.PRESET_FULL_RECT)
	_dialog_layer.add_child(center)

	var border_rect := TextureRect.new()
	border_rect.texture = load("res://assets/images/ui/border_01.png")
	border_rect.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	border_rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	border_rect.stretch_mode = TextureRect.STRETCH_SCALE
	border_rect.custom_minimum_size = Vector2(768, 280)
	border_rect.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	border_rect.size_flags_vertical = Control.SIZE_SHRINK_CENTER
	center.add_child(border_rect)

	var panel := PanelContainer.new()
	panel.custom_minimum_size = Vector2(640, 200)
	if theme:
		panel.theme = theme
	border_rect.add_child(panel)
	panel.anchor_left = 0.5
	panel.anchor_top = 0.5
	panel.offset_left = -320
	panel.offset_top = -100

	var margin := MarginContainer.new()
	for side in ["left", "top", "right", "bottom"]:
		margin.add_theme_constant_override("margin_%s" % side, 24)
	panel.add_child(margin)

	var row := HBoxContainer.new()
	row.add_theme_constant_override("separation", 20)
	margin.add_child(row)

	var portrait := TextureRect.new()
	portrait.texture = load("res://assets/images/dwarf.png")
	portrait.custom_minimum_size = Vector2(120, 120)
	portrait.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	portrait.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	portrait.size_flags_vertical = Control.SIZE_SHRINK_CENTER
	row.add_child(portrait)

	var col := VBoxContainer.new()
	col.add_theme_constant_override("separation", 12)
	col.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	row.add_child(col)

	var name_lbl := Label.new()
	name_lbl.text = speaker_name
	name_lbl.add_theme_font_size_override("font_size", 40)
	name_lbl.add_theme_color_override("font_color", Color(1, 0.85, 0.3))
	col.add_child(name_lbl)

	var text_lbl := Label.new()
	text_lbl.text = dialogue
	text_lbl.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	text_lbl.add_theme_font_size_override("font_size", 32)
	text_lbl.size_flags_vertical = Control.SIZE_EXPAND_FILL
	col.add_child(text_lbl)

	var hint := Label.new()
	hint.text = "Hold interact to close"
	hint.add_theme_font_size_override("font_size", 24)
	hint.add_theme_color_override("font_color", Color(0.6, 0.8, 1.0))
	hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	col.add_child(hint)

	add_child(_dialog_layer)
	_play_talk_sfx()
	_spawn_quest_item()

func _close() -> void:
	if _is_open():
		_dialog_layer.queue_free()
	_dialog_layer = null
	if prompt and _player_near:
		prompt.visible = true

func _play_talk_sfx() -> void:
	var am := get_node_or_null("/root/Audio_Manager")
	if am and am.has_method("play_sfx"):
		am.play_sfx("collect")

func _spawn_quest_item() -> void:
	if not quest_enabled or _quest_item_spawned:
		return
	_quest_item_spawned = true

	var item := Area2D.new()
	item.name = "QuestItem_%s" % speaker_name
	item.position = global_position + Vector2(0, 40)
	item.collision_layer = 0
	item.collision_mask = 1
	item.monitorable = true
	add_sibling(item)

	var shape := CollisionShape2D.new()
	var circle := CircleShape2D.new()
	circle.radius = 8.0
	shape.shape = circle
	item.add_child(shape)

	var sprite := Sprite2D.new()
	sprite.texture = load("res://assets/tiles/Transparent/tile_%04d.png" % quest_item_tile)
	sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	sprite.scale = Vector2(1.5, 1.5)
	sprite.modulate = Color(1, 0.85, 0.3)
	item.add_child(sprite)

	# Quest item acts like a special collectible that gives ore
	item.area_entered.connect(func(area: Area2D) -> void:
		if area.is_in_group("prototype_player") or area.is_in_group("player"):
			if RunManager.has_method("add_score"):
				RunManager.add_score(quest_reward)
			item.queue_free()
	)
