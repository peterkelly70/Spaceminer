extends Node

var log_level: int = 1

# Script to set up audio buses
# Run this once to configure the audio system

func _ready():
	print("Starting audio bus configuration...")
	setup_audio_buses()
	
func setup_audio_buses():
	# Get the AudioServer
	var audio_server = AudioServer
	
	# Check if buses already exist
	var music_bus_idx = audio_server.get_bus_index("Music")
	var sfx_bus_idx = audio_server.get_bus_index("SFX")
	
	if music_bus_idx != -1 and sfx_bus_idx != -1:
		print("Audio buses already exist")
		return
		
	# Create Music bus if it doesn't exist
	if music_bus_idx == -1:
		music_bus_idx = audio_server.bus_count
		audio_server.add_bus()
		audio_server.set_bus_name(music_bus_idx, "Music")
		audio_server.set_bus_send(music_bus_idx, "Master")
		print("Created Music bus")
	
	# Create SFX bus if it doesn't exist
	if sfx_bus_idx == -1:
		sfx_bus_idx = audio_server.bus_count
		audio_server.add_bus()
		audio_server.set_bus_name(sfx_bus_idx, "SFX")
		audio_server.set_bus_send(sfx_bus_idx, "Master")
		print("Created SFX bus")
	
	print("Audio buses configuration complete!")
	print("Current bus structure:")
	for i in range(audio_server.bus_count):
		print(" - Bus %d: %s" % [i, audio_server.get_bus_name(i)])
