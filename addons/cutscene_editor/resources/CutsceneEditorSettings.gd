@tool
extends Resource
class_name CutsceneEditorSettings

# Base directories for different asset types
@export var cutscenes_dir: String = "res://assets/cutscenes"
@export var images_dir: String = "res://assets/images/cutscenes"
@export var music_dir: String = "res://assets/audio/music"
@export var sfx_dir: String = "res://assets/audio/sfx"
@export var voiceover_dir: String = "res://assets/audio/voice"

# Default settings
@export var default_typing_speed: float = 0.05
@export var default_text_color: Color = Color.WHITE
@export var default_text_style: String = "default"
@export var default_text_anchor: String = "bottom"

# Get a properly formatted path for a specific asset type
func get_cutscene_path(filename: String) -> String:
    return _ensure_path_format(cutscenes_dir, filename, ".tres")
    
func get_image_path(filename: String) -> String:
    return _ensure_path_format(images_dir, filename, ".png")
    
func get_music_path(filename: String) -> String:
    return _ensure_path_format(music_dir, filename, ".ogg")
    
func get_sfx_path(filename: String) -> String:
    return _ensure_path_format(sfx_dir, filename, ".ogg")
    
func get_voiceover_path(filename: String) -> String:
    return _ensure_path_format(voiceover_dir, filename, ".ogg")
    
# Helper function to ensure proper path formatting
func _ensure_path_format(base_dir: String, filename: String, extension: String) -> String:
    # Remove any existing extension
    var name_only = filename.get_basename()
    
    # Format the filename (lowercase, replace spaces with underscores)
    name_only = name_only.to_lower().replace(" ", "_")
    
    # Make sure the extension is included
    if not extension.begins_with("."):
        extension = "." + extension
        
    # Make sure the base directory ends with a slash
    if not base_dir.ends_with("/"):
        base_dir += "/"
        
    return base_dir + name_only + extension
