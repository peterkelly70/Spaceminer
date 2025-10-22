@tool
extends AcceptDialog
class_name CutsceneEditorSettingsDialog

signal settings_saved

# Line edit references
@onready var cutscenes_dir_edit: LineEdit = $VBoxContainer/ScrollContainer/SettingsContainer/CutscenesDir/LineEdit
@onready var images_dir_edit: LineEdit = $VBoxContainer/ScrollContainer/SettingsContainer/ImagesDir/LineEdit
@onready var music_dir_edit: LineEdit = $VBoxContainer/ScrollContainer/SettingsContainer/MusicDir/LineEdit
@onready var sfx_dir_edit: LineEdit = $VBoxContainer/ScrollContainer/SettingsContainer/SfxDir/LineEdit
@onready var voiceover_dir_edit: LineEdit = $VBoxContainer/ScrollContainer/SettingsContainer/VoiceoverDir/LineEdit

# Browse button references
@onready var cutscenes_dir_browse: Button = $VBoxContainer/ScrollContainer/SettingsContainer/CutscenesDir/BrowseButton
@onready var images_dir_browse: Button = $VBoxContainer/ScrollContainer/SettingsContainer/ImagesDir/BrowseButton
@onready var music_dir_browse: Button = $VBoxContainer/ScrollContainer/SettingsContainer/MusicDir/BrowseButton
@onready var sfx_dir_browse: Button = $VBoxContainer/ScrollContainer/SettingsContainer/SfxDir/BrowseButton
@onready var voiceover_dir_browse: Button = $VBoxContainer/ScrollContainer/SettingsContainer/VoiceoverDir/BrowseButton

@onready var save_button: Button = $VBoxContainer/SaveButton

var settings_manager = null
var settings = null

func _ready():
    # Initialize settings manager
    var manager_script = load("res://addons/cutscene_editor/settings/settings_manager.gd")
    settings_manager = manager_script.new()
    settings = settings_manager.get_settings()
    
    # Connect signals
    save_button.pressed.connect(_on_save_pressed)
    confirmed.connect(_on_dialog_closed)
    
    # Connect browse buttons
    cutscenes_dir_browse.pressed.connect(func(): _browse_directory(cutscenes_dir_edit))
    images_dir_browse.pressed.connect(func(): _browse_directory(images_dir_edit))
    music_dir_browse.pressed.connect(func(): _browse_directory(music_dir_edit))
    sfx_dir_browse.pressed.connect(func(): _browse_directory(sfx_dir_edit))
    voiceover_dir_browse.pressed.connect(func(): _browse_directory(voiceover_dir_edit))
    
    # Load current values
    _load_settings_to_ui()

func _load_settings_to_ui():
    if settings:
        cutscenes_dir_edit.text = settings.cutscenes_dir
        images_dir_edit.text = settings.images_dir
        music_dir_edit.text = settings.music_dir
        sfx_dir_edit.text = settings.sfx_dir
        voiceover_dir_edit.text = settings.voiceover_dir

func _on_save_pressed():
    if settings:
        # Update settings from UI
        settings.cutscenes_dir = cutscenes_dir_edit.text
        settings.images_dir = images_dir_edit.text
        settings.music_dir = music_dir_edit.text
        settings.sfx_dir = sfx_dir_edit.text
        settings.voiceover_dir = voiceover_dir_edit.text
        
        # Save settings
        settings_manager.save_settings()
        
        # Ensure directories exist
        settings_manager.ensure_directories()
        
        # Emit signal
        emit_signal("settings_saved")
        
        print("Cutscene Editor settings saved successfully!")

func _on_dialog_closed():
    # Clean up
    if settings_manager:
        settings_manager.queue_free()
        settings_manager = null

# Custom directory browser dialog
class DirBrowserDialog:
    var window: Window
    var tree: Tree
    var path_label: Label
    var current_path: String
    var callback_function: Callable
    var root_item: TreeItem
    
    func _init(title: String, start_path: String, callback: Callable):
        current_path = start_path if DirAccess.dir_exists_absolute(start_path) else "res://"
        callback_function = callback
        
        # Create window with larger size
        window = Window.new()
        window.title = title
        window.size = Vector2(800, 600)  # Increased size
        window.exclusive = true
        window.unresizable = false
        
        # Create main container
        var main_container = VBoxContainer.new()
        main_container.anchors_preset = Control.PRESET_FULL_RECT
        main_container.size_flags_vertical = Control.SIZE_EXPAND_FILL
        window.add_child(main_container)
        
        # Create path display with better styling
        var path_container = HBoxContainer.new()
        path_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        main_container.add_child(path_container)
        
        var path_label_prefix = Label.new()
        path_label_prefix.text = "Current path: "
        path_container.add_child(path_label_prefix)
        
        path_label = Label.new()
        path_label.text = current_path
        path_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        path_container.add_child(path_label)
        
        # Add separator
        var separator = HSeparator.new()
        main_container.add_child(separator)
        
        # Create tree with proper expansion
        tree = Tree.new()
        tree.size_flags_vertical = Control.SIZE_EXPAND_FILL
        tree.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        tree.custom_minimum_size = Vector2(0, 400)  # Ensure minimum height
        tree.allow_rmb_select = false
        tree.hide_root = true
        main_container.add_child(tree)
        
        # Add another separator
        var separator2 = HSeparator.new()
        main_container.add_child(separator2)
        
        # Create buttons
        var button_container = HBoxContainer.new()
        button_container.alignment = BoxContainer.ALIGNMENT_END
        button_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        main_container.add_child(button_container)
        
        var cancel_button = Button.new()
        cancel_button.text = "Cancel"
        cancel_button.pressed.connect(func(): window.queue_free())
        button_container.add_child(cancel_button)
        
        # Add some spacing
        var spacer = Control.new()
        spacer.custom_minimum_size.x = 10
        button_container.add_child(spacer)
        
        var select_button = Button.new()
        select_button.text = "Select Directory"
        select_button.pressed.connect(func():
            callback_function.call(current_path)
            window.queue_free()
        )
        button_container.add_child(select_button)
        
        # Connect tree signals
        tree.item_activated.connect(func():
            var selected = tree.get_selected()
            if selected:
                var meta = selected.get_metadata(0)
                if meta is String:
                    if DirAccess.dir_exists_absolute(meta):
                        current_path = meta
                        _refresh_tree()
        )
        
        # Initial refresh
        _refresh_tree()
    
    func _refresh_tree():
        tree.clear()
        root_item = tree.create_item()
        path_label.text = current_path
        
        # Add parent directory option
        if current_path != "res://":
            var parent_dir = current_path.get_base_dir()
            var parent_item = tree.create_item(root_item)
            parent_item.set_text(0, "..")
            parent_item.set_icon(0, _get_folder_icon())
            parent_item.set_metadata(0, parent_dir)
        
        # List directories
        var dir = DirAccess.open(current_path)
        if dir:
            dir.list_dir_begin()
            var file_name = dir.get_next()
            
            # Add directories
            while file_name != "":
                if dir.current_is_dir() and not file_name.begins_with("."):
                    var dir_path = current_path.path_join(file_name)
                    var item = tree.create_item(root_item)
                    item.set_text(0, file_name)
                    item.set_icon(0, _get_folder_icon())
                    item.set_metadata(0, dir_path)
                file_name = dir.get_next()
    
    func _get_folder_icon():
        var theme = ThemeDB.get_default_theme()
        return theme.get_icon("Folder", "EditorIcons")
    
    func show():
        var parent = Engine.get_main_loop().root
        parent.add_child(window)
        window.popup_centered()

func _browse_directory(line_edit: LineEdit):
    # Get the current directory from the line edit
    var current_dir = line_edit.text
    if not DirAccess.dir_exists_absolute(current_dir):
        current_dir = "res://"
    
    # Create and show the directory browser
    var browser = DirBrowserDialog.new(
        "Select Directory",
        current_dir,
        func(path):
            line_edit.text = path
    )
    browser.show()
