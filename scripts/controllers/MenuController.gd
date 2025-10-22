extends StateAwareController	

@onready var startBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/StartButton
@onready var loadBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/LoadButton
@onready var settingsBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/SettingsButton
@onready var creditsBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/CreditsButton
@onready var quitBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/QuitButton


func _ready() -> void:
	Logger.info(self, "Initializing Menu")
	show_in_states = [GameState.GameState.MAIN_MENU]
	super._ready()
	# Connect signals for all buttons
	if startBtn:
		startBtn.pressed.connect(_on_start_button_pressed)
		startBtn.mouse_entered.connect(_on_button_hover)
	else:
		Logger.error(self, "StartButton node not found")

	if loadBtn:
		loadBtn.pressed.connect(_on_load_button_pressed)
		loadBtn.mouse_entered.connect(_on_button_hover)
	else:
		Logger.error(self, "LoadButton node not found")

	if settingsBtn:
		settingsBtn.pressed.connect(_on_settings_button_pressed)
		settingsBtn.mouse_entered.connect(_on_button_hover)
	else:
		Logger.error(self, "SettingsButton node not found")

	if creditsBtn:
		creditsBtn.pressed.connect(_on_credits_button_pressed)
		creditsBtn.mouse_entered.connect(_on_button_hover)
	else:
		Logger.error(self, "CreditsButton node not found")

	if quitBtn:
		quitBtn.pressed.connect(_on_quit_button_pressed)
		quitBtn.mouse_entered.connect(_on_button_hover)
	else:
		Logger.error(self, "QuitButton node not found")


func on_enter_state(_state: int) -> void:
	Logger.info(self, "Entering state")
	SettingsManager.load_settings()
	visible = true

func on_exit_state(_state: int) -> void:
	Logger.info(self, "Exiting state")
	visible = false

# Button handlers
func _on_start_button_pressed() -> void:
	Logger.info(self, "Start button pressed")
	# Transition to game setup flow
	request_state(GameState.GameState.GAMESETUP)
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_load_button_pressed() -> void:
	Logger.info(self, "Load button pressed")
	# TODO: Implement load game flow; for now, route to GAMESETUP or show a load UI when available
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_settings_button_pressed() -> void:
	Logger.info(self, "Settings button pressed")
	request_state(GameState.GameState.SETTINGS)
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_credits_button_pressed() -> void:
	Logger.info(self, "Credits button pressed")
	request_state(GameState.GameState.CREDITS)
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_quit_button_pressed() -> void:
	Logger.info(self, "Quit button pressed")
	if Audio_Manager:
		Audio_Manager.play_sfx("click")
	get_tree().quit()

# Hover sound feedback
func _on_button_hover() -> void:
	var audio_manager = get_node_or_null("/root/Audio_Manager")
	if audio_manager and audio_manager.has_method("play_sfx"):
		audio_manager.play_sfx("button_hover")
		return

	var hover_sound_paths = [
		"res://assets/audio/sfx/ui/button_hover.mp3",
		"res://assets/audio/sfx/ui/hover.mp3"
	]

	for path in hover_sound_paths:
		if FileAccess.file_exists(path):
			var stream = load(path)
			if stream and stream is AudioStream:
				var hover_sound = AudioStreamPlayer.new()
				hover_sound.stream = stream
				hover_sound.volume_db = -10.0
				add_child(hover_sound)
				hover_sound.play()
				# Auto-cleanup
				hover_sound.finished.connect(func(): hover_sound.queue_free())
				return
