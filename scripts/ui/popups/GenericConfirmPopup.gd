extends Control

var log_level: int = 1

# Track the result of the popup
var confirmed_result: bool = false

@onready var message_label: Label = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ContentContainer/MessageLabel"
# Use the autoloaded TextUtils singleton
@onready var cancel_button: Button = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/CancelButton"
@onready var confirm_button: Button = $"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/ButtonContainer/ConfirmButton"

##
# Sets up the popup title, message, and mode.
# If confirm_only is true, only the confirm button is shown (informational popup).
# Usage:
#   popup.setup("Info", "Something happened", true)
func setup(dialog_title: String, message: String, confirm_only: bool = false) -> void:
	$"CenterContainer/PanelContainer/MarginContainer/VBoxContainer/TitleContainer/TitleLabel".text = dialog_title
	print("Setting title to: %s" % dialog_title)
	
	print("Setting message to: %s" % message)
	print("Message label found: %s" % is_instance_valid(message_label))
	if message_label:
		# Force direct text assignment to override any inherited default text
		TextUtils.assign(message_label,message,false)
		print("Message set directly to override defaults")
	else:
		push_error("Message label not found!")
	
	print("Setting up popup with confirm_only = %s" % confirm_only)
	
	if confirm_only:
		# For informational popups, hide cancel button and change confirm text
		cancel_button.visible = false
		cancel_button.disabled = true
		confirm_button.text = "OK"
		print("Setting up as informational popup (OK only)")
	else:
		# For confirmation popups, show both buttons
		cancel_button.visible = true
		cancel_button.disabled = false
		cancel_button.text = "NO"
		confirm_button.text = "YES"
		print("Setting up as confirmation popup (YES/NO)")

func _ready():
	var default_theme = load("res://assets/themes/default_theme.tres")
	self.theme = default_theme
	cancel_button.theme = default_theme
	confirm_button.theme = default_theme
	cancel_button.connect("pressed", Callable(self, "_on_cancel_pressed"))
	confirm_button.connect("pressed", Callable(self, "_on_confirm_pressed"))
	
	# Make sure the buttons are properly initialized
	cancel_button.visible = true
	cancel_button.disabled = false
	print("_ready() called, theme applied")

func _on_cancel_pressed():
	confirmed_result = false
	hide()

func _on_confirm_pressed():
	confirmed_result = true
	hide()
