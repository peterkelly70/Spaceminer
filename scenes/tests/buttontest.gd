extends Node2D

@onready var btn = $TextureRect/Button

func _ready() ->void:
	# Ensure background doesn't swallow input
	$TextureRect.mouse_filter = Control.MOUSE_FILTER_IGNORE
	btn.pressed.connect(_button_pressed)

func _button_pressed():
	# Fail fast: call autoloads directly
	Audio_Manager.play_sfx("notification")
	Notification_Manager.show_notification("Button pressed!", 1.5, true, "default")
