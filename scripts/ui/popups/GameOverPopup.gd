extends "res://scripts/ui/popups/BasePopup.gd"
class_name GameOverPopup

signal restart_requested
signal menu_requested

@onready var _title_label: Label = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/TitleContainer/TitleLabel"
@onready var _content_label: Label = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/ContentLabel"
@onready var _restart_button: Button = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/ConfirmButton"
@onready var _menu_button: Button = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/CancelButton"

func _ready() -> void:
	process_mode = Node.PROCESS_MODE_WHEN_PAUSED
	can_close_with_escape = false
	super._ready()
	_setup_ui()

func _setup_ui() -> void:
	if _title_label:
		_title_label.text = "FOOT STOMPED"
	if _content_label:
		_content_label.text = "The foot got you.\nRestart the same seed or return to the menu?"
	if _restart_button:
		_restart_button.text = "RESTART SAME SEED"
		_restart_button.pressed.connect(_on_restart_pressed)
	if _menu_button:
		_menu_button.text = "MAIN MENU"
		_menu_button.pressed.connect(_on_menu_pressed)

func setup(seed_text: String, room_name: String = "") -> void:
	if _title_label:
		_title_label.text = "FOOT STOMPED"
	if _content_label:
		var lines: Array[String] = []
		lines.append("The foot got you.")
		if not room_name.strip_edges().is_empty():
			lines.append("Room: %s" % room_name)
		lines.append("Seed: %s" % (seed_text if not seed_text.strip_edges().is_empty() else "Random"))
		lines.append("Restart the same seed or return to the menu?")
		_content_label.text = "\n".join(lines)
	if _restart_button:
		_restart_button.grab_focus()

func _on_restart_pressed() -> void:
	if Audio_Manager and Audio_Manager.has_method("play_sfx"):
		Audio_Manager.play_sfx("click")
	restart_requested.emit()
	_emit_action("restart_requested")
	_close()

func _on_menu_pressed() -> void:
	if Audio_Manager and Audio_Manager.has_method("play_sfx"):
		Audio_Manager.play_sfx("click")
	menu_requested.emit()
	_emit_action("menu_requested")
	_close()
