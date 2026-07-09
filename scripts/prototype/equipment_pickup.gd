extends Area2D
class_name EquipmentPickup

signal picked_up(item_id: String)

@export var item_id: String = "jetpack"
@export var display_name: String = "Pickup"
@export var neon_color: Color = Color("#00D7FF")

@onready var collision_shape: CollisionShape2D = $CollisionShape2D
@onready var sprite: Sprite2D = $Sprite2D
@onready var label: Label = $Label

var _phase: float = 0.0

func _ready() -> void:
	body_entered.connect(_on_body_entered)
	_refresh_visual()

func configure(data: Dictionary) -> void:
	item_id = str(data.get("item_id", item_id))
	display_name = str(data.get("display_name", display_name))
	neon_color = _color_for_item(item_id)
	var radius := float(data.get("radius", 10.0))
	if collision_shape and collision_shape.shape is CircleShape2D:
		(collision_shape.shape as CircleShape2D).radius = radius
	_refresh_visual()

func _process(delta: float) -> void:
	_phase += delta * 8.0
	var pulse := 0.75 + (sin(_phase) * 0.25)
	if sprite:
		sprite.modulate = neon_color.lightened(0.15 * pulse)

func _refresh_visual() -> void:
	if sprite:
		sprite.texture = _texture_for_item(item_id)
		sprite.modulate = neon_color
	if label:
		label.text = display_name.to_upper()

func _on_body_entered(body: Node) -> void:
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return
	picked_up.emit(item_id)
	_play_pickup_sfx()
	queue_free()

func _play_pickup_sfx() -> void:
	var audio_manager := get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("collect")

func _color_for_item(name: String) -> Color:
	match name.to_lower():
		"jetpack":
			return Color("#00D7FF")
		"magnetic_boots":
			return Color("#8CFF00")
		"grappling_hook":
			return Color("#FFD21A")
		"cargo_key":
			return Color("#FF8A00")
		"visibility_cloak":
			return Color("#FF00C8")
		"laser_pistol":
			return Color("#FF1E2D")
		"shield":
			return Color("#1D4FFF")
		"o2_tank":
			return Color("#00D7FF")
		"oxygen_tank":
			return Color("#00D7FF")
		"battery":
			return Color("#5A2DFF")
		"fuel":
			return Color("#FF8A00")
		"barrel":
			return Color("#8C8C8C")
		"barrell":
			return Color("#8C8C8C")
		"tile_0410":
			return Color("#FF8A00")
		"tile_0411":
			return Color("#8C8C8C")
		"fuel_tank":
			return Color("#FF8A00")
		"fuel_cell":
			return Color("#FF8A00")
		"battery_pack":
			return Color("#5A2DFF")
		"dead_astronaut":
			return Color("#F4F4F4")
		"security_cyan":
			return Color("#00D7FF")
		"security_blue":
			return Color("#1D4FFF")
		"security_green":
			return Color("#00D66B")
		"security_yellow":
			return Color("#FFD21A")
		"security_orange":
			return Color("#FF8A00")
		"security_red":
			return Color("#FF1E2D")
		"door_key":
			return Color("#F4F4F4")
		_:
			return Color("#F4F4F4")

func _texture_for_item(name: String) -> Texture2D:
	var path := "res://assets/images/willyinspace/coin_hollow.png"
	match name.to_lower():
		"jetpack":
			path = "res://assets/tiles/Transparent/tile_0401.png"
		"laser_pistol":
			path = "res://assets/tiles/Transparent/tile_0402.png"
		"grappling_hook":
			path = "res://assets/tiles/Transparent/tile_0403.png"
		"magnetic_boots":
			path = "res://assets/tiles/Transparent/tile_0404.png"
		"o2_tank", "oxygen_tank":
			path = "res://assets/tiles/Transparent/tile_0405.png"
		"dead_astronaut":
			path = "res://assets/tiles/Transparent/tile_0406.png"
		"visibility_cloak":
			path = "res://assets/tiles/Transparent/tile_0407.png"
		"shield":
			path = "res://assets/tiles/Transparent/tile_0408.png"
		"battery", "battery_pack":
			path = "res://assets/tiles/Transparent/tile_0409.png"
		"fuel", "fuel_cell", "fuel_tank":
			path = "res://assets/tiles/Transparent/tile_0410.png"
		"barrel", "barrell", "tile_0411":
			path = "res://assets/tiles/Transparent/tile_0411.png"
	var texture := load(path)
	if texture is Texture2D:
		return texture
	var image := Image.new()
	if image.load(path) == OK:
		return ImageTexture.create_from_image(image)
	return null
