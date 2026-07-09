@tool
extends Resource
class_name CutsceneResource

@export var name: String = "New Cutscene" ## The name of the cutscene, used for identification
@export_file("*.mp3", "*.ogg", "*.wav") var music_path: String = "" ## Path to background music that will play during the entire cutscene
@export var music_volume: float = 1.0 ## Volume of the background music (0.0 to 1.0)
@export var voiceover_volume: float = 1.0 ## Volume of the voiceover audio (0.0 to 1.0)
@export_file("*.png", "*.jpg", "*.jpeg", "*.webp") var background_image: String = "" ## Optional global background image for the entire cutscene (PNG or JPG)
@export var typing_speed: float = 0.05 ## Speed of the typing effect (lower is faster)
@export var use_bbcode: bool = false ## Whether to parse BBCode in text (allows formatting)
@export var auto_next: bool = false ## Whether to automatically advance to the next frame when typing is complete
@export var theme: String = "ironhaven" ## Theme to apply to the cutscene (e.g., ironhaven, parchment, scroll)
@export var frames: Array[CutsceneFrameResource] = [] ## Individual frames/panels of the cutscene
