extends Control

@onready var bg: ColorRect = $BG
@onready var test_button: Button = $VBox/TestButton
@onready var quit_button: Button = $VBox/QuitButton

var color_a: Color = Color(0.10, 0.10, 0.10, 1.0)
var color_b: Color = Color(0.20, 0.05, 0.05, 1.0)
var toggled: bool = false

func _ready() -> void:
	assert(bg != null, "BG ColorRect is missing")
	assert(test_button != null, "TestButton is missing")
	assert(quit_button != null, "QuitButton is missing")
	
	bg.color = color_a
	test_button.pressed.connect(_on_test_pressed)
	quit_button.pressed.connect(_on_quit_pressed)
	print("[TestButton] Ready: hover to see default visuals, press to toggle background")

func _on_test_pressed() -> void:
	toggled = !toggled
	bg.color = (color_b if toggled else color_a)
	print("[TestButton] Toggled color -> %s" % ("B" if toggled else "A"))

func _on_quit_pressed() -> void:
	print("[TestButton] Quit pressed")
	get_tree().quit()
