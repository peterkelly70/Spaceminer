extends TextureRect
class_name AnimatedLifeIcon

const DEFAULT_LIVE_FRAMES: Array[Texture2D] = [
	preload("res://assets/images/willyinspace/SpaceMiner1.png"),
	preload("res://assets/images/willyinspace/SpaceMiner2.png"),
	preload("res://assets/images/willyinspace/SpaceMiner3.png"),
	preload("res://assets/images/willyinspace/SpaceMiner4.png"),
]
const DEFAULT_EMPTY_TEXTURE := preload("res://assets/images/blank_life.png")

@export var live_frames: Array[Texture2D] = DEFAULT_LIVE_FRAMES
@export var empty_texture: Texture2D = DEFAULT_EMPTY_TEXTURE
@export var frame_time: float = 0.12

var _alive := true
var _frame_index := 0
var _elapsed := 0.0

func _ready() -> void:
	custom_minimum_size = Vector2(64, 64)
	expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	_update_texture()

func _process(delta: float) -> void:
	if not _alive or live_frames.size() <= 1:
		return
	_elapsed += delta
	if _elapsed < frame_time:
		return
	_elapsed = 0.0
	_frame_index = (_frame_index + 1) % live_frames.size()
	_update_texture()

func set_alive(value: bool) -> void:
	if _alive == value:
		return
	_alive = value
	_frame_index = 0
	_elapsed = 0.0
	_update_texture()

func _update_texture() -> void:
	if _alive and not live_frames.is_empty():
		texture = live_frames[_frame_index]
	else:
		texture = empty_texture
