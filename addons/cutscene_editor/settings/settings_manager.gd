@tool
extends Node
class_name CutsceneEditorSettingsManager

const SETTINGS_PATH = "res://addons/cutscene_editor/settings/editor_settings.tres"
const SettingsClass = preload("res://addons/cutscene_editor/resources/CutsceneEditorSettings.gd")

var _settings = null

# Get the settings, loading them if necessary
func get_settings():
    if _settings == null:
        _settings = _load_settings()
    return _settings

# Save the current settings
func save_settings() -> void:
    if _settings != null:
        ResourceSaver.save(_settings, SETTINGS_PATH)
        print("Cutscene Editor settings saved to: ", SETTINGS_PATH)

# Load settings from disk or create new ones
func _load_settings():
    var settings
    
    if ResourceLoader.exists(SETTINGS_PATH):
        var loaded = ResourceLoader.load(SETTINGS_PATH)
        if loaded and loaded.get_script() == SettingsClass:
            settings = loaded
            print("Cutscene Editor settings loaded from: ", SETTINGS_PATH)
        else:
            settings = SettingsClass.new()
            push_warning("Failed to load Cutscene Editor settings, creating new settings.")
    else:
        settings = SettingsClass.new()
        print("Creating new Cutscene Editor settings.")
        
    # Ensure the settings are saved
    ResourceSaver.save(settings, SETTINGS_PATH)
    
    return settings

# Ensure directories exist
func ensure_directories() -> void:
    var settings = get_settings()
    var directories = [
        settings.cutscenes_dir,
        settings.images_dir,
        settings.music_dir,
        settings.sfx_dir,
        settings.voiceover_dir
    ]
    
    for dir in directories:
        # Convert res:// path to a project path
        var project_path = dir.replace("res://", "")
        
        # Create directory if it doesn't exist
        var dir_obj = DirAccess.open("res://")
        if dir_obj:
            if !dir_obj.dir_exists(project_path):
                var err = dir_obj.make_dir_recursive(project_path)
                if err == OK:
                    print("Created directory: ", dir)
                else:
                    push_warning("Failed to create directory: " + dir + ", error: " + str(err))
