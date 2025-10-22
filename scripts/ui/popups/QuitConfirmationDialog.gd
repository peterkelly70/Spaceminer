extends "res://scripts/ui/popups/BasePopup.gd"
class_name QuitConfirmationPopup2

# Signals (in addition to BasePopup signals)
signal quit_confirmed
signal quit_cancelled

func _ready():
	super._ready()
	# Connect button signals
	var confirm_button = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/ConfirmButton
	var cancel_button = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/CancelButton
	
	if confirm_button:
		confirm_button.text = "YES"
		confirm_button.pressed.connect(_on_confirm_button_pressed)
	
	if cancel_button:
		cancel_button.text = "NO"
		cancel_button.pressed.connect(_on_cancel_button_pressed)
	
	# Set default title and message
	title = "QUIT GAME"
	var title_label = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/TitleContainer/TitleLabel
	if title_label:
		title_label.text = title
	
	var content_label = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/ContentLabel
	if content_label:
		content_label.text = "Are you sure you want to quit the game?\nAny unsaved progress will be lost."

func _init_custom(params: Dictionary) -> void:
	# Custom initialization from params if needed
	if params.has("message"):
		var content_label = $CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/ContentLabel
		if content_label:
			content_label.text = params.message

# Override to emit our specific signals
func _on_confirm_button_pressed() -> void:
	# Play click sound
	if Audio_Manager:
		Audio_Manager.play_sfx("click")
	
	# Emit signals
	emit_signal("quit_confirmed")
	_emit_action("quit_confirmed")
	
	# Close dialog
	_close()

# Override to emit our specific signals
func _on_cancel_button_pressed() -> void:
	# Play click sound
	if Audio_Manager:
		Audio_Manager.play_sfx("click")
	
	# Emit signals
	emit_signal("quit_cancelled")
	_emit_action("quit_cancelled")
	
	# Close dialog
	_close()

# Use the base popup theming
func _apply_theme() -> void:
	# Call the parent class implementation
	super._apply_theme()
