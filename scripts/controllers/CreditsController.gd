extends StateAwareController
class_name CreditsController

@onready var back_button: Button = $MainContainer/MarginContainer/Footer/BackButton


func _ready() -> void:
	show_in_states = [AppState.State.CREDITS]
	super._ready()
	
	# Connect signals
	if back_button:
		back_button.pressed.connect(_on_back_button_pressed)
		back_button.mouse_entered.connect(_on_button_hover)
	else:
		push_error("BackButton not found!")

func _on_back_button_pressed() -> void:
	print("Back button pressed")
	request_state(AppState.State.MAIN_MENU)

# Hover sound feedback similar to MenuController
func _on_button_hover() -> void:
	var audio_manager = get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("button_hover")
		return

	var hover_sound_paths = [
		"res://assets/audio/sfx/ui/button_hover.mp3",
		"res://assets/audio/sfx/ui/hover.mp3"
	]

	for path in hover_sound_paths:
		if FileAccess.file_exists(path):
			var stream = load(path)
			if stream and stream is AudioStream:
				var hover_sound = AudioStreamPlayer.new()
				hover_sound.stream = stream
				hover_sound.volume_db = -10.0
				add_child(hover_sound)
				hover_sound.play()
				
				# Auto-cleanup
				hover_sound.finished.connect(func(): hover_sound.queue_free())
				return
