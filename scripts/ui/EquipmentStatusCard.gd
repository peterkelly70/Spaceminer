extends PanelContainer
class_name EquipmentStatusCard

const ICON_SCALE := 3
const ICON_SIZE := Vector2i(8, 8)
const BLANK_ICON_PATH := "res://assets/tiles/Transparent/tile_0000.png"
const SECURITY_CARD_ICON_PATH := "res://assets/tiles/Transparent/tile_0400.png"

const CARD_PATTERNS := {
	"jetpack": [
		"...AA...",
		"..AAAA..",
		".AAWWAA.",
		".AAWWAA.",
		".AAWWAA.",
		"..A..A..",
		".A....A.",
		"AA....AA",
	],
	"magnetic_boots": [
		"AA....AA",
		"AA....AA",
		"AAAAAA..",
		"..AAAA..",
		"..AAAA..",
		".AA..AA.",
		".AA..AA.",
		"AA....AA",
	],
	"grappling_hook": [
		".....AA.",
		"....A..A",
		"...A.A..",
		"..A.A...",
		".AA.....",
		"..A.....",
		"..AA....",
		".A.A....",
	],
	"visibility_cloak": [
		"..AAAA..",
		".A....A.",
		".AWWWWA.",
		".AWWWWA.",
		".A....A.",
		".AA..AA.",
		"..A..A..",
		"..A..A..",
	],
	"laser_pistol": [
		"AAAAAA..",
		"AA..AA..",
		"AAAAAA..",
		"..AA....",
		"..AA....",
		"...A....",
		"...A....",
		"..AA....",
	],
	"shield": [
		"..AAAA..",
		".AWWWWA.",
		"AWWWWWWA",
		"AWWWWWWA",
		"AWWWWWWA",
		".AWWWWA.",
		"..AAAA..",
		"..AA....",
	],
}

@export var item_id: String = ""
@export var icon_path: String = ""
@export var accent_color: Color = Color("#00D7FF")
@export var slot_number: int = 0

@onready var icon: TextureRect = $MarginContainer/Icon
@onready var slot_number_label: Label = $SlotNumber

var _owned := false
var _security_icon_texture: Texture2D = null
var _blank_icon_texture: Texture2D = null

func _ready() -> void:
	_apply_panel_style()
	_refresh_visual()

# slot_number > 0 shows a hotkey hint ("1".."4") over the empty slot, which
# hides once the slot is filled so the icon reads clearly as "picked up".
func configure(config_item_id: String, config_icon_path: String = "", config_accent_color: Color = Color("#00D7FF"), owned: bool = false, config_slot_number: int = 0) -> void:
	item_id = config_item_id
	icon_path = config_icon_path
	accent_color = config_accent_color
	_owned = owned
	slot_number = config_slot_number
	_refresh_visual()

func set_owned(value: bool) -> void:
	if _owned == value:
		return
	_owned = value
	_refresh_visual()

func is_owned() -> bool:
	return _owned

func _apply_panel_style() -> void:
	var style := StyleBoxFlat.new()
	style.bg_color = Color.BLACK
	style.border_width_left = 2
	style.border_width_top = 2
	style.border_width_right = 2
	style.border_width_bottom = 2
	style.border_color = accent_color
	style.corner_radius_top_left = 2
	style.corner_radius_top_right = 2
	style.corner_radius_bottom_left = 2
	style.corner_radius_bottom_right = 2
	add_theme_stylebox_override("panel", style)

func _refresh_visual() -> void:
	if not is_inside_tree():
		return
	if item_id.is_empty():
		item_id = "security_card"
	visible = true
	if icon:
		icon.texture = _build_icon_texture() if _owned else _get_blank_texture()
		icon.modulate = accent_color if _owned else Color.BLACK
	if slot_number_label:
		slot_number_label.visible = slot_number > 0 and not _owned
		slot_number_label.text = str(slot_number)
		slot_number_label.add_theme_color_override("font_color", accent_color)

func _build_icon_texture() -> Texture2D:
	if not icon_path.is_empty():
		if _security_icon_texture and icon_path == SECURITY_CARD_ICON_PATH:
			return _security_icon_texture
		var loaded := load(icon_path)
		if loaded is Texture2D:
			if icon_path == SECURITY_CARD_ICON_PATH:
				_security_icon_texture = loaded
			return loaded
		var image := Image.new()
		image.load_from_file(ProjectSettings.globalize_path(icon_path))
		if not image.is_empty():
			var texture := ImageTexture.create_from_image(image)
			if icon_path == SECURITY_CARD_ICON_PATH:
				_security_icon_texture = texture
			return texture
	return _get_blank_texture()

func _get_blank_texture() -> Texture2D:
	if _blank_icon_texture:
		return _blank_icon_texture
	var loaded := load(BLANK_ICON_PATH)
	if loaded is Texture2D:
		_blank_icon_texture = loaded
		return _blank_icon_texture
	var image := Image.create(ICON_SIZE.x * ICON_SCALE, ICON_SIZE.y * ICON_SCALE, false, Image.FORMAT_RGBA8)
	image.fill(Color(0, 0, 0, 0))
	_blank_icon_texture = ImageTexture.create_from_image(image)
	return _blank_icon_texture

func _build_pattern_texture() -> Texture2D:
	var image := Image.create(ICON_SIZE.x * ICON_SCALE, ICON_SIZE.y * ICON_SCALE, false, Image.FORMAT_RGBA8)
	image.fill(Color(0, 0, 0, 0))
	var pattern: Array = CARD_PATTERNS.get(item_id, CARD_PATTERNS["jetpack"])
	for y in range(min(pattern.size(), ICON_SIZE.y)):
		var row := str(pattern[y])
		for x in range(min(row.length(), ICON_SIZE.x)):
			var ch := row.substr(x, 1)
			var color := Color(0, 0, 0, 0)
			match ch:
				"A":
					color = accent_color
				"W":
					color = Color("#F4F4F4")
			if color.a <= 0.0:
				continue
			for oy in range(ICON_SCALE):
				for ox in range(ICON_SCALE):
					image.set_pixel(x * ICON_SCALE + ox, y * ICON_SCALE + oy, color)
	return ImageTexture.create_from_image(image)
