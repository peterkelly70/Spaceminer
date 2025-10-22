extends Control
class_name GamePhaseController

var log_level: int = 1

# UI References
@onready var phase_content = $ContentArea/PhaseContent

# Phase scene paths - using strings instead of preload to avoid compile-time errors
var phase_scene_paths = {
	GameState.GamePhase.ASSIGN_WORKERS: "res://scenes/game/phases/AssignWorkersPhase.tscn",
	GameState.GamePhase.COLLECT_RESOURCES: "res://scenes/game/phases/CollectResourcesPhase.tscn",
	GameState.GamePhase.BUILD_UPGRADE: "res://scenes/game/phases/BuildUpgradePhase.tscn",
	GameState.GamePhase.BUY_LAND: "res://scenes/game/phases/BuyLandPhase.tscn",
	GameState.GamePhase.EQUIP_SOLDIERS: "res://scenes/game/phases/EquipSoldiersPhase.tscn",
	GameState.GamePhase.RANDOM_EVENT: "res://scenes/game/phases/RandomEventPhase.tscn",
	GameState.GamePhase.POPULATION_ADJUST: "res://scenes/game/phases/PopulationPhase.tscn",
	GameState.GamePhase.TURN_SUMMARY: "res://scenes/game/phases/TurnSummaryPhase.tscn",
	GameState.GamePhase.TOWN_VIEW: "res://scenes/game/town/TownMapView.tscn"
}

# Current phase scene instance
var current_phase_scene = null

func _ready():
	Logger.info(self, "Initializing...")
	# Connect signals - use deferred to ensure UI is fully loaded
	call_deferred("_connect_signals")
	# Connect to Game_Manager signals
	Game_Manager.phase_changed.connect(Callable(self, "_on_Game_Manager_phase_changed"))
	# Initialize with current phase
	_on_Game_Manager_phase_changed(Game_Manager.get_current_phase())
	
	Logger.info(self, "Ready")
	

func _connect_signals():
	# Find next phase button if not already set
	
	# Connect to Game_Manager signals
	Game_Manager.phase_changed.connect(Callable(self, "_on_phase_changed"))
	Game_Manager.turn_started.connect(Callable(self, "_on_turn_started"))
	
	# Start a new game
	Game_Manager.start_new_game()
	
	# Force initial phase display
	_on_phase_changed(Game_Manager.current_phase)

func _on_next_phase_button_pressed():
    Game_Manager.next_phase()
    
    # Play click sound
    Audio_Manager.play_sfx("click")

func _on_phase_changed(phase: int):
	Logger.info(self, "Phase changed to: %s" % GameState.GamePhase.keys()[phase])
	
	# Clear current phase scene
	if current_phase_scene:
		current_phase_scene.queue_free()
		current_phase_scene = null
	
	# Ensure the content area fills the available space (reserved for layout tuning)
	var content_area = $MainContainer/ContentArea
	content_area = content_area # no-op to keep fail-fast semantics if path is wrong

	# Load new phase scene
	if phase_scene_paths.has(phase):
		# Directly pass the phase enum to the load_phase_scene function
		load_phase_scene(phase)
	else:
		Logger.error(self, "No scene path found for phase: %s" % GameState.GamePhase.keys()[phase])


func _on_Game_Manager_phase_changed(phase: int) -> void:
	Logger.info(self, "Game_Manager phase changed to: %s" % GameState.GamePhase.keys()[phase])
	# Update the UI with the new phase
	_on_phase_changed(phase)
	# No need to call load_phase_scene again as _on_phase_changed already does that

func _on_turn_started(season, year):
	Logger.info(self, "Turn started: %s - Year %s" % [season, year])


func load_phase_scene(phase) -> void:
	# Load and display the correct phase scene based on the game state.
	# This function does NOT modify UI layout or phase scene internals.
	# Fail fast and loud on error conditions.
	
	# Handle both string and enum inputs
	var scene_path = ""
	if phase is String:
		if not phase_scene_paths.has(phase):
			Logger.error(self, "No scene path found for phase: %s" % str(phase))
			return
		scene_path = phase_scene_paths[phase]
	else: # Assume it's an enum
		if not phase_scene_paths.has(phase):
			Logger.error(self, "No scene path found for phase enum: %s" % str(phase))
			return
		scene_path = phase_scene_paths[phase]

	# Remove the previous phase scene if it exists
	if current_phase_scene:
		current_phase_scene.queue_free()
		current_phase_scene = null

	# Load and instantiate the scene
	var scene_resource = load(scene_path)
	if not scene_resource:
		Logger.error(self, "Failed to load scene: %s" % str(scene_path))
		return
	
	# Instantiate the scene
	current_phase_scene = scene_resource.instantiate()
	if not current_phase_scene:
		Logger.error(self, "Failed to instantiate scene: %s" % str(scene_path))
		return

	# Add the phase scene to the content area and ensure proper layout
	phase_content.add_child(current_phase_scene)
	
	# Ensure the phase scene fills the available space
	current_phase_scene.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	current_phase_scene.size_flags_vertical = Control.SIZE_EXPAND_FILL
	current_phase_scene.anchors_preset = Control.PRESET_FULL_RECT

	# Connect signals on the phase scene (fail-fast if absent)
	current_phase_scene.connect_signals(self)


	Logger.info(self, "Loaded phase scene: %s" % str(phase))
