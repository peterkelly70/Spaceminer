extends Resource
class_name MusicCollection

## A resource that defines a collection of music tracks
## Used for organizing music by album, artist, or theme

@export var collection_name: String = ""
@export var artist: String = ""
@export var album: String = ""
@export var description: String = ""
@export var tracks: Array[MusicTrack] = []

func _init(
	p_collection_name: String = "",
	p_artist: String = "",
	p_album: String = "",
	p_description: String = ""
):
	collection_name = p_collection_name
	artist = p_artist
	album = p_album
	description = p_description

## Add a track to the collection
func add_track(track: MusicTrack) -> void:
	if track and track not in tracks:
		tracks.append(track)

## Get a random track from the collection
func get_random_track() -> MusicTrack:
	if tracks.is_empty():
		return null
	return tracks[randi() % tracks.size()]

## Get tracks by category
func get_tracks_by_category(category: String) -> Array[MusicTrack]:
	var filtered_tracks: Array[MusicTrack] = []
	for track in tracks:
		if track.category == category:
			filtered_tracks.append(track)
	return filtered_tracks

## Get all valid tracks (files that exist)
func get_valid_tracks() -> Array[MusicTrack]:
	var valid_tracks: Array[MusicTrack] = []
	for track in tracks:
		if track.is_valid():
			valid_tracks.append(track)
	return valid_tracks

## Get track by name
func get_track_by_name(track_name: String) -> MusicTrack:
	for track in tracks:
		if track.track_name == track_name:
			return track
	return null
