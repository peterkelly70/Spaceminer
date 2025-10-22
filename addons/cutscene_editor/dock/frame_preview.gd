@tool
extends PanelContainer

# UI References
@onready var background = $MarginContainer/VBoxContainer/PreviewBackground
@onready var image = $MarginContainer/VBoxContainer/PreviewBackground/ImageContainer/Image
@onready var title_label = $MarginContainer/VBoxContainer/PreviewBackground/TextContainer/TitleLabel
@onready var text_label = $MarginContainer/VBoxContainer/PreviewBackground/TextContainer/TextLabel
@onready var continue_button = $MarginContainer/VBoxContainer/PreviewBackground/ContinueButton

func _ready():
	# Initialize with empty preview
	clear_preview()

func update_preview(frame):
	# Set background color
	if frame.has("background_color"):
		background.color = Color(frame.background_color)
	else:
		background.color = Color("#000000cc")
	
	# Set title
	if frame.has("title") and frame.title != "":
		title_label.text = frame.title
		title_label.visible = true
	else:
		title_label.visible = false
	
	# Set text
	if frame.has("text"):
		text_label.text = frame.text
	else:
		text_label.text = ""
	
	# Set image
	if frame.has("image") and frame.image != "":
		var texture = load(frame.image) if ResourceLoader.exists(frame.image) else null
		if texture:
			image.texture = texture
			image.get_parent().visible = true
		else:
			image.get_parent().visible = false
	else:
		image.get_parent().visible = false
	
	# Set continue button text
	if frame.has("continue_text"):
		continue_button.text = frame.continue_text
	else:
		continue_button.text = "Continue"

func clear_preview():
	background.color = Color("#000000cc")
	title_label.text = ""
	title_label.visible = false
	text_label.text = ""
	image.texture = null
	image.get_parent().visible = false
	continue_button.text = "Continue"
