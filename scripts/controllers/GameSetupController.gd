extends StateAwareController
class_name GameSetupController

# UI References - Updated for new container structure
@onready var kingdom_name_edit: LineEdit = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/GameSettingsContainer/KingdomNameContainer/KingdomNameEdit
@onready var difficulty_option: OptionButton = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/GameSettingsContainer/DifficultyContainer/DifficultyOption
@onready var map_size_option: OptionButton = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/GameSettingsContainer/MapSizeContainer/MapSizeOption
@onready var resource_richness_option: OptionButton = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/GameSettingsContainer/ResourceRichContainer/ResourceRichnessOption
@onready var tutorial_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/GameSettingsContainer/TutorialContainer/TutorialCheck
@onready var ironman_check: CheckBox = $MainContainer/CenterContainer/MainPanel/MarginContainer/VBoxContainer/GameSettingsContainer/IronmanContainer/IronmanCheck
@onready var start_game_button: Button = $HBoxContainer2/HBoxContainer/StartButton
@onready var back_button: Button = $HBoxContainer2/HBoxContainer/BackButton

# Game setup data
var game_setup_data = {
	"kingdom_name": "IronHaven",
	"difficulty": 1,  # 0=Easy, 1=Normal, 2=Hard, 3=Ironhaven
	"map_size": 1,    # 0=Small, 1=Medium, 2=Large
	"resource_richness": 1,  # 0=Scarce, 1=Standard, 2=Abundant
	"tutorial_enabled": true,
	"ironman_mode": false
}

# Difficulty options
var difficulties = ["Easy", "Normal", "Hard", "Ironhaven"]
# Map size options
var map_sizes = ["Small", "Medium", "Large"]
# Resource richness options
var resource_richness = ["Scarce", "Standard", "Abundant"]

func _ready() -> void:
	Logger.info(self, "Initializing...")
	show_in_states = [GameState.GameState.GAMESETUP]
	super._ready()
	
	# Connect button signals
	if start_game_button:
		start_game_button.pressed.connect(_on_start_game_button_pressed)
	else:
		Logger.error(self, "StartGameButton node not found")
		
	if back_button:
		back_button.pressed.connect(_on_back_button_pressed)
	else:
		Logger.error(self, "BackButton node not found")
	
	# Load default settings
	_load_default_settings()
	
	Logger.info(self, "Ready")

func _load_default_settings() -> void:
	Logger.info(self, "Loading default settings...")
	
	# Set default kingdom name
	if kingdom_name_edit:
		kingdom_name_edit.text = game_setup_data.kingdom_name
	
	# Set default difficulty
	if difficulty_option:
		difficulty_option.select(game_setup_data.difficulty)
	
	# Set default map size
	if map_size_option:
		map_size_option.select(game_setup_data.map_size)
	
	# Set default resource richness
	if resource_richness_option:
		resource_richness_option.select(game_setup_data.resource_richness)
	
	# Set default tutorial setting
	if tutorial_check:
		tutorial_check.button_pressed = game_setup_data.tutorial_enabled
	
	# Set default ironman mode
	if ironman_check:
		ironman_check.button_pressed = game_setup_data.ironman_mode

func _save_game_settings() -> void:
	Logger.info(self, "Saving game settings...")
	
	# Save kingdom name
	if kingdom_name_edit:
		game_setup_data.kingdom_name = kingdom_name_edit.text
	
	# Save difficulty
	if difficulty_option:
		game_setup_data.difficulty = difficulty_option.get_selected_id()
	
	# Save map size
	if map_size_option:
		game_setup_data.map_size = map_size_option.get_selected_id()
	
	# Save resource richness
	if resource_richness_option:
		game_setup_data.resource_richness = resource_richness_option.get_selected_id()
	
	# Save tutorial setting
	if tutorial_check:
		game_setup_data.tutorial_enabled = tutorial_check.button_pressed
	
	# Save ironman mode
	if ironman_check:
		game_setup_data.ironman_mode = ironman_check.button_pressed
	
	# Save to global game state
	_save_to_global_state()

func _save_to_global_state() -> void:
	# Create a new game state based on the settings
	var game_state = {
		"kingdom_name": game_setup_data.kingdom_name,
		"difficulty": difficulties[game_setup_data.difficulty],
		"map_size": map_sizes[game_setup_data.map_size],
		"resource_richness": resource_richness[game_setup_data.resource_richness],
		"tutorial_enabled": game_setup_data.tutorial_enabled,
		"ironman_mode": game_setup_data.ironman_mode,
		"current_season": "Spring",
		"current_year": 1,
		"resources": {
			"ore": 100,
			"stone": 100,
			"wood": 100,
			"food": 100,
			"gold": 100,
			"gems": 0,
			"mana": 0
		},
		"population": {
			"total": 10,
			"workers": 8,
			"warriors": 2,
			"idle": 0
		}
	}
	
	# Store the game state in a global singleton or save to disk
	# For now, just print it
	Logger.info(self, "New game state created: %s" % JSON.stringify(game_state, "  "))
	
	# TODO: Store this in a global game state manager
	# GameState.initialize(game_state)

func _on_start_game_button_pressed() -> void:
	Logger.info(self, "Start Game button pressed")
	
	# Validate inputs
	if kingdom_name_edit and kingdom_name_edit.text.strip_edges().is_empty():
		var notification_manager = Notification_Manager
		if notification_manager:
			notification_manager.show_notification("Please enter a kingdom name!", 2.0)
		return
	
	# Save game settings
	_save_game_settings()
	
	# Start the game - transition to the main game state
	# For now, we'll just go to WILL_AND_WAY as the first game screen
	request_state(GameState.GameState.PLAYING)

func _on_back_button_pressed() -> void:
	Logger.info(self, "Back button pressed")
	request_state(GameState.GameState.MAIN_MENU)

func on_enter_state(_state: GameState.GameState) -> void:
	Logger.info(self, "Entering state")
	_load_default_settings()
	visible = true

func on_exit_state(_state: GameState.GameState) -> void:
	Logger.info(self, "Exiting state")
	visible = false
