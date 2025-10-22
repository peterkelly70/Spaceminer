@tool
extends Resource
class_name CutsceneEditorPropertyHints

# Text anchor options - Expanded to match image anchors (3x3 grid)
const TEXT_ANCHOR_OPTIONS = ["top_left", "top_center", "top_right", 
                             "center_left", "center", "center_right", 
                             "bottom_left", "bottom_center", "bottom_right"]

# Image anchor options
const IMAGE_ANCHOR_OPTIONS = ["top_left", "top_center", "top_right", 
                             "center_left", "center", "center_right", 
                             "bottom_left", "bottom_center", "bottom_right"]

# Text style options
const TEXT_STYLE_OPTIONS = ["default", "italic", "bold", "bold-italic"]

# Theme options - Added for consistent styling
const THEME_OPTIONS = ["default", "dark", "light", "fantasy", "sci-fi"]

# Get file dialog for path selection
static func get_file_dialog(title: String, mode: int, filters: PackedStringArray, callback: Callable) -> FileDialog:
    var dialog = FileDialog.new()
    dialog.title = title
    dialog.file_mode = mode
    dialog.access = FileDialog.ACCESS_RESOURCES
    dialog.filters = filters
    dialog.file_selected.connect(callback)
    dialog.dir_selected.connect(callback)
    dialog.canceled.connect(func(): dialog.queue_free())
    
    # Set size and make it modal
    dialog.min_size = Vector2(600, 400)
    dialog.exclusive = true
    
    return dialog

# Get option button for dropdown selection
static func get_option_button(options: Array, current_value: String, callback: Callable) -> OptionButton:
    var option_button = OptionButton.new()
    
    # Add options
    for i in range(options.size()):
        option_button.add_item(options[i], i)
        if options[i] == current_value:
            option_button.select(i)
    
    # Connect signal
    option_button.item_selected.connect(func(index): callback.call(options[index]))
    
    return option_button

# Get color picker for color selection
static func get_color_picker(current_color: Color, callback: Callable) -> ColorPickerButton:
    var color_picker = ColorPickerButton.new()
    color_picker.color = current_color
    color_picker.custom_minimum_size = Vector2(40, 30)
    color_picker.color_changed.connect(callback)
    
    return color_picker

# Get BBCode preview dialog
static func get_bbcode_preview_dialog(text: String, use_bbcode: bool, color: Color) -> AcceptDialog:
    var dialog = AcceptDialog.new()
    dialog.title = "BBCode Preview"
    dialog.min_size = Vector2(500, 300)
    
    var vbox = VBoxContainer.new()
    dialog.add_child(vbox)
    
    var rich_text = RichTextLabel.new()
    rich_text.size_flags_vertical = Control.SIZE_EXPAND_FILL
    rich_text.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    rich_text.custom_minimum_size = Vector2(480, 250)
    rich_text.bbcode_enabled = true
    
    if use_bbcode:
        rich_text.text = text
    else:
        rich_text.text = text
    
    rich_text.add_theme_color_override("default_color", color)
    vbox.add_child(rich_text)
    
    return dialog
