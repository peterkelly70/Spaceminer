extends Resource
class_name MusicTrack

## A resource that defines a music track with metadata
## Used for organizing and managing music in the AudioManager

@export var track_name: String = ""
@export var display_name: String = ""
@export var artist: String = ""
@export var album: String = ""
@export var file_path: String = ""
@export var duration: float = 0.0
@export var loop: bool = true
@export var volume: float = 1.0
@export var category: String = "general"  # e.g., "menu", "gameplay", "battle", etc.

func _init(
	p_track_name: String = "",
	p_display_name: String = "",
	p_artist: String = "",
	p_album: String = "",
	p_file_path: String = "",
	p_duration: float = 0.0,
	p_loop: bool = true,
	p_volume: float = 1.0,
	p_category: String = "general"
):
	track_name = p_track_name
	display_name = p_display_name
	artist = p_artist
	album = p_album
	file_path = p_file_path
	duration = p_duration
	loop = p_loop
	volume = p_volume
	category = p_category

## Get a formatted display string for the track
func get_display_string() -> String:
	if artist.is_empty():
		return display_name
	return "%s - %s" % [artist, display_name]

## Check if the track file exists
func is_valid() -> bool:
	return not file_path.is_empty() and FileAccess.file_exists(file_path)
