extends TextureButton

signal dial_turned(index)

@export var label_text: String = "Dial"
@export var textures: Array[Texture2D] = []

var current_index: int = 0

func _ready() -> void:
	# Set the label text
	$Label.text = label_text
	
	# Ensure we have at least one texture
	if textures.size() > 0:
		texture_normal = textures[0]
	
	# Connect the pressed signal
	pressed.connect(_on_dial_pressed)

func _on_dial_pressed() -> void:
	# Cycle to the next texture
	current_index = (current_index + 1) % textures.size()
	
	# Update the texture
	if textures.size() > 0:
		texture_normal = textures[current_index]
	
	# Emit the signal with the current index
	dial_turned.emit(current_index)

func set_index(index: int) -> void:
	if textures.size() > 0:
		current_index = index % textures.size()
		texture_normal = textures[current_index]
