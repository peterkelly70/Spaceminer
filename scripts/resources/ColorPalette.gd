extends Resource
class_name SpaceMinerPalette

@export var names: PackedStringArray = PackedStringArray()
@export var colors: PackedColorArray = PackedColorArray()

func get_palette_name(index: int, fallback: String = "") -> String:
	if index < 0 or index >= names.size():
		return fallback
	return names[index]

func get_color(index: int, fallback: Color = Color.WHITE) -> Color:
	if index < 0 or index >= colors.size():
		return fallback
	return colors[index]

func size() -> int:
	return colors.size()
