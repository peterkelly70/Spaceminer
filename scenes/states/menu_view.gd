extends Control

func _init():
	print("*** MENUCONTROLLER _init() CALLED ***")

func _ready() -> void:
	print("*** MENUCONTROLLER _ready() CALLED ***")
	
	# Get buttons directly
	var start_btn = get_node("MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/StartButton")
	var settings_btn = get_node("MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/SettingsButton")
	var credits_btn = get_node("MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/CreditsButton")
	var quit_btn = get_node("MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/QuitButton")
	
	if start_btn:
		print("Connecting StartButton")
		start_btn.pressed.connect(_on_start_pressed)
	
	if settings_btn:
		print("Connecting SettingsButton") 
		settings_btn.pressed.connect(_on_settings_pressed)
		
	if credits_btn:
		print("Connecting CreditsButton")
		credits_btn.pressed.connect(_on_credits_pressed)
		
	if quit_btn:
		print("Connecting QuitButton")
		quit_btn.pressed.connect(_on_quit_pressed)

func _on_start_pressed():
	print("*** START BUTTON CLICKED ***")
	State_Manager.change_state(GameState.GameState.GAMESETUP)

func _on_settings_pressed():
	print("*** SETTINGS BUTTON CLICKED ***")
	State_Manager.change_state(GameState.GameState.SETTINGS)

func _on_credits_pressed():
	print("*** CREDITS BUTTON CLICKED ***")
	State_Manager.change_state(GameState.GameState.CREDITS)

func _on_quit_pressed():
	print("*** QUIT BUTTON CLICKED ***")
	State_Manager.change_state(GameState.GameState.QUIT)
