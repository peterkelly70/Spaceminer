extends Node
class_name MusicManager

var log_level: int = 1

const MUSIC_ROOT := "res://assets/audio/music"
const AUDIO_EXTENSIONS := ["mp3", "ogg", "wav"]

## Music Manager handles music collection loading and playback coordination
## This separates music logic from the AudioManager for cleaner architecture

signal track_changed(track: MusicTrack)
signal collection_loaded(collection: MusicCollection)

var current_collection: MusicCollection
var current_track: MusicTrack
var audio_manager: Node

func _ready() -> void:
	# Get reference to AudioManager
	audio_manager = get_node_or_null("/root/Audio_Manager")
	if not audio_manager:
		push_error("AudioManager not found!")
		return
	add_to_group("music_manager")
	
	print("Initialized")

## Load a music collection from a resource file or create it programmatically
func load_music_collection(collection_path: String) -> bool:
	print("Loading music collection: %s" % collection_path)
	
	# Try to load from resource file first
	var collection = load(collection_path) as MusicCollection
	if not collection:
		push_warning("Resource file failed to load, creating Karl Casey collection programmatically")
		collection = _create_karl_casey_collection()
		if not collection:
			push_error("Failed to create Karl Casey collection")
			return false
	return _load_collection(collection)

func load_music_collection_from_directory(preferred_directory: String = "", root_path: String = MUSIC_ROOT) -> bool:
	print("Scanning music root: %s" % root_path)
	var directories := DirAccess.get_directories_at(root_path)
	directories.sort()
	if directories.is_empty():
		push_error("No music directories found under: %s" % root_path)
		return false
	var target_directory := preferred_directory
	if target_directory.is_empty() or not directories.has(target_directory):
		target_directory = directories[0]
	print("Selected music directory: %s" % target_directory)
	var collection := _create_collection_from_directory(root_path, target_directory)
	if collection.tracks.is_empty():
		push_error("Music directory is empty: %s" % target_directory)
		if target_directory != directories[0]:
			push_warning("Falling back to alphabetical first directory: %s" % directories[0])
			var fallback_collection := _create_collection_from_directory(root_path, directories[0])
			if fallback_collection.tracks.is_empty():
				push_error("Fallback directory also empty: %s" % directories[0])
				return false
			return _load_collection(fallback_collection)
		return false
	return _load_collection(collection)

func _create_collection_from_directory(root_path: String, directory_name: String) -> MusicCollection:
	var absolute_directory := root_path.path_join(directory_name)
	print("Creating collection from directory: %s" % absolute_directory)
	var file_paths := DirAccess.get_files_at(absolute_directory)
	file_paths.sort()
	var collection := MusicCollection.new()
	collection.collection_name = directory_name
	collection.artist = directory_name
	collection.album = directory_name
	for file_name in file_paths:
		var extension := file_name.get_extension().to_lower()
		if not AUDIO_EXTENSIONS.has(extension):
			continue
		var track := MusicTrack.new()
		var base_name := file_name.get_basename()
		track.track_name = base_name
		track.display_name = _humanize_track_name(base_name)
		track.artist = directory_name
		track.album = directory_name
		track.file_path = absolute_directory.path_join(file_name)
		collection.add_track(track)
	
	var should_loop := collection.tracks.size() <= 1
	for track in collection.tracks:
		track.loop = should_loop
	return collection

func _humanize_track_name(value: String) -> String:
	var normalized := value.replace("_", " ").replace("-", " ").strip_edges()
	if normalized.is_empty():
		return value
	return normalized.capitalize() if normalized == normalized.to_lower() else normalized

func _load_collection(collection: MusicCollection) -> bool:
	current_collection = collection
	var loaded_count := 0
	for track in collection.get_valid_tracks():
		if not audio_manager.has_method("load_music"):
			push_error("AudioManager missing load_music method")
			return false
		audio_manager.load_music(track.track_name, track.file_path)
		if audio_manager.has_method("set_music_loop"):
			audio_manager.set_music_loop(track.track_name, track.loop)
		loaded_count += 1
		print("Loaded track: %s" % track.display_name)
	if loaded_count == 0:
		push_error("No playable audio files found in collection: %s" % collection.collection_name)
		current_collection = null
		return false
	print("Loaded %s tracks from collection: %s" % [loaded_count, collection.collection_name])
	collection_loaded.emit(collection)
	return true

## Create the Karl Casey collection programmatically
func _create_karl_casey_collection() -> MusicCollection:
	print("Creating Karl Casey collection programmatically")
	
	var collection = MusicCollection.new()
	collection.collection_name = "Karl Casey - Dark Synthwave Collection Vol. 1"
	collection.artist = "Karl Casey"
	collection.album = "Dark Synthwave Collection Vol. 1"
	collection.description = "A collection of dark synthwave tracks by Karl Casey"
	
	# Define all Karl Casey tracks
	var track_data = [
		{"name": "rise_of_machines", "display": "Rise of the Machines", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 01 Rise of the Machines.mp3"},
		{"name": "sin_city", "display": "Sin City", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 02 Sin City.mp3"},
		{"name": "mutants_technology", "display": "Mutants of Technology", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 03 Mutants of Technology.mp3"},
		{"name": "assault", "display": "Assault", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 04 Assault.mp3"},
		{"name": "knee_deep_dead", "display": "Knee Deep in the Dead", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 05 Knee Deep in the Dead.mp3"},
		{"name": "dredd", "display": "Dredd", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 06 Dredd.mp3"},
		{"name": "neuromancer", "display": "Neuromancer", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 07 Neuromancer.mp3"},
		{"name": "outrun_reaper", "display": "Outrun the Reaper", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 08 Outrun the Reaper.mp3"},
		{"name": "black_orb", "display": "Answer to the Black Orb", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 09 Answer to the Black Orb.mp3"},
		{"name": "subterranean_terror", "display": "Subterranean Terror", "file": "Karl Casey - Dark Synthwave Collection Vol. 1 - 10 Subterranean Terror.mp3"}
	]
	
	# Create MusicTrack resources for each track
	for track_info in track_data:
		var track = MusicTrack.new()
		track.track_name = track_info.name
		track.display_name = track_info.display
		track.artist = "Karl Casey"
		track.album = "Dark Synthwave Collection Vol. 1"
		track.file_path = "res://assets/audio/music/Karl Casey - Dark Synthwave Collection Vol. 1/" + track_info.file
		track.duration = 0.0  # Will be set by AudioManager when loaded
		track.loop = false
		track.volume = 1.0
		track.category = "general"
		
		collection.tracks.append(track)
		print("Created track: %s" % track.display_name)
	
	print("Created collection with %s tracks" % collection.tracks.size())
	return collection

## Play a random track from the current collection
func play_random_track(category: String = "") -> bool:
	if not current_collection:
		push_error("No music collection loaded")
		return false
	
	var available_tracks: Array[MusicTrack]
	
	if category.is_empty():
		available_tracks = current_collection.get_valid_tracks()
	else:
		available_tracks = current_collection.get_tracks_by_category(category)
	
	if available_tracks.is_empty():
		push_warning("No tracks available for category: %s" % category)
		return false
	
	var selected_track = available_tracks[randi() % available_tracks.size()]
	return play_track(selected_track)

## Play a specific track
func play_track(track: MusicTrack) -> bool:
	if not track or not track.is_valid():
		push_error("Invalid track provided")
		return false
	
	if not audio_manager or not audio_manager.has_method("play_music"):
		push_error("AudioManager not available or missing play_music method")
		return false
	
	print("Playing track: %s" % track.get_display_string())
	
	# Play the track through AudioManager
	audio_manager.play_music(track.track_name)
	current_track = track
	
	# Emit signal with debugging
	print("Emitting track_changed signal for: %s" % track.display_name)
	track_changed.emit(track)
	print("Signal emitted successfully")
	
	return true

## Play a track by name
func play_track_by_name(track_name: String) -> bool:
	if not current_collection:
		push_error("No music collection loaded")
		return false
	
	var track = current_collection.get_track_by_name(track_name)
	if not track:
		push_error("Track not found: %s" % track_name)
		return false
	
	return play_track(track)

## Get the current track
func get_current_track() -> MusicTrack:
	return current_track

## Get the current collection
func get_current_collection() -> MusicCollection:
	return current_collection

## Get all tracks in a category
func get_tracks_by_category(category: String) -> Array[MusicTrack]:
	if not current_collection:
		return []
	return current_collection.get_tracks_by_category(category)

## Get a random track without playing it
func get_random_track(category: String = "") -> MusicTrack:
	if not current_collection:
		return null
	
	if category.is_empty():
		return current_collection.get_random_track()
	
	var category_tracks = current_collection.get_tracks_by_category(category)
	if category_tracks.is_empty():
		return null
	
	return category_tracks[randi() % category_tracks.size()]

## Setup automatic track switching when tracks finish
func setup_auto_switching(enabled: bool = true) -> void:
	if not audio_manager:
		return
	
	# Connect to AudioManager's track finished signal if it exists
	if audio_manager.has_signal("track_finished"):
		if enabled and not audio_manager.track_finished.is_connected(_on_track_finished):
			audio_manager.track_finished.connect(_on_track_finished)
		elif not enabled and audio_manager.track_finished.is_connected(_on_track_finished):
			audio_manager.track_finished.disconnect(_on_track_finished)

## Called when a track finishes playing
func _on_track_finished(_track_name: String = "") -> void:
	print("Track finished, switching to next random track")
	play_random_track()
