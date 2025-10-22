@tool
extends Control

signal frame_updated(frame_data)

# Anchor presets - matching the CutscenePlayer enum
enum ImageAnchor {
	FULL_RECT = 0,
	TOP_LEFT = 1,
	TOP_CENTER = 2,
	TOP_RIGHT = 3,
	CENTER_LEFT = 4,
	CENTER = 5,
	CENTER_RIGHT = 6,
	BOTTOM_LEFT = 7,
	BOTTOM_CENTER = 8,
	BOTTOM_RIGHT = 9
}

# Style presets - matching the CutscenePlayer enum
enum TextStyle {
	NONE = 0,
	THEME_DEFAULT = 1,
	CUSTOM = 2
}

var current_frame = null
var current_image_index = 0

# Node references
@onready var title_edit = $VBoxContainer/TitleSection/TitleEdit
@onready var background_edit = $VBoxContainer/BackgroundSection/BackgroundHBox/BackgroundEdit
@onready var browse_background_btn = $VBoxContainer/BackgroundSection/BackgroundHBox/BrowseBackgroundBtn
@onready var clear_background_btn = $VBoxContainer/BackgroundSection/BackgroundHBox/ClearBackgroundBtn
@onready var text_edit = $VBoxContainer/TextSection/TextEdit
@onready var text_style_option = $VBoxContainer/TextStyleSection/StyleHBox/StyleOption
@onready var text_border_check = $VBoxContainer/TextStyleSection/StyleHBox/BorderCheck
@onready var text_bg_color = $VBoxContainer/TextStyleSection/BgColorHBox/ColorPicker
@onready var text_anchor = $VBoxContainer/TextStyleSection/AnchorHBox/AnchorOption
@onready var voiceover_edit = $VBoxContainer/VoiceoverSection/VoiceoverHBox/VoiceoverEdit
@onready var browse_voiceover_btn = $VBoxContainer/VoiceoverSection/VoiceoverHBox/BrowseVoiceoverBtn
@onready var clear_voiceover_btn = $VBoxContainer/VoiceoverSection/VoiceoverHBox/ClearVoiceoverBtn

@onready var image_list = $VBoxContainer/ImagesSection/ImageList
@onready var add_image_btn = $VBoxContainer/ImagesSection/ButtonsHBox/AddImageBtn
@onready var delete_image_btn = $VBoxContainer/ImagesSection/ButtonsHBox/DeleteImageBtn
@onready var image_path_edit = $VBoxContainer/ImagePropsSection/PathHBox/PathEdit
@onready var image_browse_btn = $VBoxContainer/ImagePropsSection/PathHBox/BrowseBtn
@onready var image_anchor = $VBoxContainer/ImagePropsSection/AnchorHBox/AnchorOption
@onready var fade_in_spin = $VBoxContainer/ImagePropsSection/FadeHBox/FadeInSpin
@onready var fade_out_spin = $VBoxContainer/ImagePropsSection/FadeHBox/FadeOutSpin

@onready var duration_mode_option = $VBoxContainer/DurationSection/DurationModeOption
@onready var manual_duration_spin = $VBoxContainer/DurationSection/ManualDurationSpin

# File dialogs
var image_file_dialog
var background_file_dialog
var voiceover_file_dialog

func _ready():
	# Connect signals
	title_edit.text_changed.connect(_on_title_changed)
	browse_background_btn.pressed.connect(_on_browse_background_pressed)
	clear_background_btn.pressed.connect(_on_clear_background_pressed)
	text_edit.text_changed.connect(_on_text_changed)
	text_style_option.item_selected.connect(_on_text_style_selected)
	text_border_check.toggled.connect(_on_text_border_toggled)
	text_bg_color.color_changed.connect(_on_text_bg_color_changed)
	text_anchor.item_selected.connect(_on_text_anchor_selected)
	browse_voiceover_btn.pressed.connect(_on_browse_voiceover_pressed)
	clear_voiceover_btn.pressed.connect(_on_clear_voiceover_pressed)
	
	add_image_btn.pressed.connect(_on_add_image_pressed)
	delete_image_btn.pressed.connect(_on_delete_image_pressed)
	image_browse_btn.pressed.connect(_on_image_browse_pressed)
	image_anchor.item_selected.connect(_on_image_anchor_selected)
	fade_in_spin.value_changed.connect(_on_fade_in_changed)
	fade_out_spin.value_changed.connect(_on_fade_out_changed)
	
	duration_mode_option.item_selected.connect(_on_duration_mode_selected)
	manual_duration_spin.value_changed.connect(_on_manual_duration_changed)
	
	# Create file dialogs
	_create_file_dialogs()
	
	# Initialize UI
	_init_ui()
	
	# Disable until a frame is loaded
	visible = false

func _create_file_dialogs():
	# Image file dialog
	image_file_dialog = FileDialog.new()
	image_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	image_file_dialog.access = FileDialog.ACCESS_RESOURCES
	image_file_dialog.filters = ["*.png, *.jpg, *.jpeg, *.webp ; Image Files"]
	image_file_dialog.size = Vector2(800, 600)
	image_file_dialog.file_selected.connect(_on_image_file_selected)
	add_child(image_file_dialog)
	
	# Background image file dialog
	background_file_dialog = FileDialog.new()
	background_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	background_file_dialog.access = FileDialog.ACCESS_RESOURCES
	background_file_dialog.filters = ["*.png, *.jpg, *.jpeg, *.webp ; Image Files"]
	background_file_dialog.size = Vector2(800, 600)
	background_file_dialog.file_selected.connect(_on_background_file_selected)
	add_child(background_file_dialog)
	
	# Voiceover file dialog
	voiceover_file_dialog = FileDialog.new()
	voiceover_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	voiceover_file_dialog.access = FileDialog.ACCESS_RESOURCES
	voiceover_file_dialog.filters = ["*.mp3, *.ogg, *.wav ; Audio Files"]
	voiceover_file_dialog.size = Vector2(800, 600)
	voiceover_file_dialog.file_selected.connect(_on_voiceover_file_selected)
	add_child(voiceover_file_dialog)

func _init_ui():
	# Initialize text style options
	text_style_option.clear()
	text_style_option.add_item("None", TextStyle.NONE)
	text_style_option.add_item("Theme Default", TextStyle.THEME_DEFAULT)
	text_style_option.add_item("Custom", TextStyle.CUSTOM)
	
	# Initialize anchor options for text
	text_anchor.clear()
	text_anchor.add_item("Full Screen", ImageAnchor.FULL_RECT)
	text_anchor.add_item("Top Left", ImageAnchor.TOP_LEFT)
	text_anchor.add_item("Top Center", ImageAnchor.TOP_CENTER)
	text_anchor.add_item("Top Right", ImageAnchor.TOP_RIGHT)
	text_anchor.add_item("Center Left", ImageAnchor.CENTER_LEFT)
	text_anchor.add_item("Center", ImageAnchor.CENTER)
	text_anchor.add_item("Center Right", ImageAnchor.CENTER_RIGHT)
	text_anchor.add_item("Bottom Left", ImageAnchor.BOTTOM_LEFT)
	text_anchor.add_item("Bottom Center", ImageAnchor.BOTTOM_CENTER)
	text_anchor.add_item("Bottom Right", ImageAnchor.BOTTOM_RIGHT)
	
	# Initialize anchor options for images
	image_anchor.clear()
	image_anchor.add_item("Full Screen", ImageAnchor.FULL_RECT)
	image_anchor.add_item("Top Left", ImageAnchor.TOP_LEFT)
	image_anchor.add_item("Top Center", ImageAnchor.TOP_CENTER)
	image_anchor.add_item("Top Right", ImageAnchor.TOP_RIGHT)
	image_anchor.add_item("Center Left", ImageAnchor.CENTER_LEFT)
	image_anchor.add_item("Center", ImageAnchor.CENTER)
	image_anchor.add_item("Center Right", ImageAnchor.CENTER_RIGHT)
	image_anchor.add_item("Bottom Left", ImageAnchor.BOTTOM_LEFT)
	image_anchor.add_item("Bottom Center", ImageAnchor.BOTTOM_CENTER)
	image_anchor.add_item("Bottom Right", ImageAnchor.BOTTOM_RIGHT)
	
	# Initialize duration mode options
	duration_mode_option.clear()
	duration_mode_option.add_item("Auto Text", 0)
	duration_mode_option.add_item("Auto Voiceover", 1)
	duration_mode_option.add_item("Manual", 2)

func load_frame(frame_data):
	current_frame = frame_data
	if not current_frame:
		visible = false
		return
	
	visible = true
	
	# Load frame data
	title_edit.text = current_frame.title if current_frame.title != null else ""
	
	# Load background image
	background_edit.text = current_frame.background_image if current_frame.background_image != null else ""
	
	# Load text
	text_edit.text = current_frame.text if current_frame.text != null else ""
	
	# Load text style
	var style = current_frame.style if current_frame.style != null else "default"
	var style_index = 0
	if style == "default":
		style_index = TextStyle.THEME_DEFAULT
	elif style == "custom":
		style_index = TextStyle.CUSTOM
	text_style_option.select(style_index)
	
	# Load text border
	text_border_check.button_pressed = current_frame.text_border if current_frame.text_border != null else false
	
	# Load text background color
	var bg_color = current_frame.text_bg_color if current_frame.text_bg_color != null else "#00000080"
	text_bg_color.color = Color.html(bg_color)
	
	# Load text anchor
	var text_anchor_value = _get_anchor_index_from_string(current_frame.text_anchor if current_frame.text_anchor != null else "bottom")
	text_anchor.select(text_anchor_value)
	
	# Load voiceover
	voiceover_edit.text = current_frame.voiceover_path if current_frame.voiceover_path != null else ""
	
	# Load duration mode
	var mode = current_frame.duration_mode if current_frame.duration_mode != null else "auto_voiceover"
	if mode == "manual":
		duration_mode_option.select(2)
		manual_duration_spin.editable = true
	elif mode == "auto_text":
		duration_mode_option.select(0)
		manual_duration_spin.editable = false
	else:
		duration_mode_option.select(1)
		manual_duration_spin.editable = false
	manual_duration_spin.value = current_frame.manual_duration if current_frame.manual_duration != null else 3.0
	
	# Load images
	_load_images()

func _get_anchor_index_from_string(anchor_string):
	match anchor_string:
		"fill", "full":
			return ImageAnchor.FULL_RECT
		"top_left", "topleft":
			return ImageAnchor.TOP_LEFT
		"top", "top_center", "topcenter":
			return ImageAnchor.TOP_CENTER
		"top_right", "topright":
			return ImageAnchor.TOP_RIGHT
		"left", "center_left", "centerleft":
			return ImageAnchor.CENTER_LEFT
		"center":
			return ImageAnchor.CENTER
		"right", "center_right", "centerright":
			return ImageAnchor.CENTER_RIGHT
		"bottom_left", "bottomleft":
			return ImageAnchor.BOTTOM_LEFT
		"bottom", "bottom_center", "bottomcenter":
			return ImageAnchor.BOTTOM_CENTER
		"bottom_right", "bottomright":
			return ImageAnchor.BOTTOM_RIGHT
		_:
			return ImageAnchor.BOTTOM_CENTER

func _get_anchor_string_from_index(anchor_index):
	match anchor_index:
		ImageAnchor.FULL_RECT:
			return "fill"
		ImageAnchor.TOP_LEFT:
			return "top_left"
		ImageAnchor.TOP_CENTER:
			return "top"
		ImageAnchor.TOP_RIGHT:
			return "top_right"
		ImageAnchor.CENTER_LEFT:
			return "left"
		ImageAnchor.CENTER:
			return "center"
		ImageAnchor.CENTER_RIGHT:
			return "right"
		ImageAnchor.BOTTOM_LEFT:
			return "bottom_left"
		ImageAnchor.BOTTOM_CENTER:
			return "bottom"
		ImageAnchor.BOTTOM_RIGHT:
			return "bottom_right"
		_:
			return "bottom"

func _load_images():
	# Clear image list
	image_list.clear()
	
	# Get images from frame
	var images = current_frame.images if current_frame.images != null else []
	
	# Add images to list
	for i in range(images.size()):
		var image = images[i]
		var image_name = "Image " + str(i+1)
		if image.path != null and image.path != "":
			image_name += ": " + image.path.get_file()
		image_list.add_item(image_name)
	
	# Clear image properties
	_clear_image_properties()
	
	# Connect image selection signal
	if not image_list.item_selected.is_connected(_on_image_selected):
		image_list.item_selected.connect(_on_image_selected)

func _update_image_properties(image):
	image_path_edit.text = image.path if image.path != null else ""
	image_anchor.select(image.anchor if image.anchor != null else ImageAnchor.FULL_RECT)
	fade_in_spin.value = image.fade_in if image.fade_in != null else 0.5
	fade_out_spin.value = image.fade_out if image.fade_out != null else 0.5

func _clear_image_properties():
	image_path_edit.text = ""
	image_anchor.select(ImageAnchor.FULL_RECT)
	fade_in_spin.value = 0.5
	fade_out_spin.value = 0.5

func _on_title_changed(new_text):
	if current_frame:
		current_frame.title = new_text
		emit_signal("frame_updated", current_frame)

func _on_browse_background_pressed():
	background_file_dialog.popup_centered()

func _on_background_file_selected(path):
	if current_frame:
		current_frame.background_image = path
		background_edit.text = path
		emit_signal("frame_updated", current_frame)

func _on_clear_background_pressed():
	if current_frame:
		current_frame.background_image = ""
		background_edit.text = ""
		emit_signal("frame_updated", current_frame)

func _on_text_changed():
	if current_frame:
		current_frame.text = text_edit.text
		emit_signal("frame_updated", current_frame)

func _on_text_style_selected(index):
	if current_frame:
		match index:
			TextStyle.NONE:
				current_frame.style = "none"
			TextStyle.THEME_DEFAULT:
				current_frame.style = "default"
			TextStyle.CUSTOM:
				current_frame.style = "custom"
		emit_signal("frame_updated", current_frame)

func _on_text_border_toggled(toggled):
	if current_frame:
		current_frame.text_border = toggled
		emit_signal("frame_updated", current_frame)

func _on_text_bg_color_changed(color):
	if current_frame:
		current_frame.text_bg_color = color.to_html()
		emit_signal("frame_updated", current_frame)

func _on_text_anchor_selected(index):
	if current_frame:
		current_frame.text_anchor = _get_anchor_string_from_index(index)
		emit_signal("frame_updated", current_frame)

func _on_browse_voiceover_pressed():
	voiceover_file_dialog.popup_centered()

func _on_voiceover_file_selected(path):
	if current_frame:
		current_frame.voiceover_path = path
		voiceover_edit.text = path
		emit_signal("frame_updated", current_frame)

func _on_clear_voiceover_pressed():
	if current_frame:
		current_frame.voiceover_path = ""
		voiceover_edit.text = ""
		emit_signal("frame_updated", current_frame)

func _on_image_selected(index):
	if current_frame and current_frame.images != null:
		current_image_index = index
		if index >= 0 and index < current_frame.images.size():
			_update_image_properties(current_frame.images[index])

func _on_add_image_pressed():
	if current_frame:
		if current_frame.images == null:
			current_frame.images = []
		
		var new_image = {
			"path": "",
			"anchor": ImageAnchor.FULL_RECT,
			"fade_in": 0.5,
			"fade_out": 0.5
		}
		
		current_frame.images.append(new_image)
		
		# Update image list
		_load_images()
		
		# Select the new image
		current_image_index = current_frame.images.size() - 1
		image_list.select(current_image_index)
		_update_image_properties(new_image)
		
		emit_signal("frame_updated", current_frame)

func _on_delete_image_pressed():
	if current_frame and current_frame.images != null and current_image_index >= 0 and current_image_index < current_frame.images.size():
		current_frame.images.remove_at(current_image_index)
		
		# Update image list
		_load_images()
		
		emit_signal("frame_updated", current_frame)

func _on_image_browse_pressed():
	image_file_dialog.popup_centered()

func _on_image_file_selected(path):
	if current_frame and current_frame.images != null and current_image_index >= 0 and current_image_index < current_frame.images.size():
		current_frame.images[current_image_index].path = path
		
		# Update image path
		image_path_edit.text = path
		
		# Update image list item
		var image_name = path.get_file()
		var item_text = "Image " + str(current_image_index+1) + ": " + image_name
		image_list.set_item_text(current_image_index, item_text)
		
		emit_signal("frame_updated", current_frame)

func _on_image_anchor_selected(index):
	if current_frame and current_frame.images != null and current_image_index >= 0 and current_image_index < current_frame.images.size():
		current_frame.images[current_image_index].anchor = index
		emit_signal("frame_updated", current_frame)

func _on_fade_in_changed(value):
	if current_frame and current_frame.images != null and current_image_index >= 0 and current_image_index < current_frame.images.size():
		current_frame.images[current_image_index].fade_in = value
		emit_signal("frame_updated", current_frame)

func _on_fade_out_changed(value):
	if current_frame and current_frame.images != null and current_image_index >= 0 and current_image_index < current_frame.images.size():
		current_frame.images[current_image_index].fade_out = value
		emit_signal("frame_updated", current_frame)

func _on_duration_mode_selected(index):
	if current_frame:
		match index:
			0:
				current_frame.duration_mode = "auto_text"
				manual_duration_spin.editable = false
			1:
				current_frame.duration_mode = "auto_voiceover"
				manual_duration_spin.editable = false
			2:
				current_frame.duration_mode = "manual"
				manual_duration_spin.editable = true
		emit_signal("frame_updated", current_frame)

func _on_manual_duration_changed(value):
	if current_frame:
		current_frame.manual_duration = value
		emit_signal("frame_updated", current_frame)
