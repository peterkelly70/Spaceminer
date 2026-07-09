extends StateAwareController	

@onready var startBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/StartButton
@onready var loadBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/LoadButton2
@onready var settingsBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/SettingsButton
@onready var creditsBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/CreditsButton
@onready var quitBtn = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer/QuitButton
@onready var mainMenuPage = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/VBoxContainer
@onready var newGamePanel = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/NewGamePanel
@onready var loadGamePanel = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/LoadGamePanel
@onready var seedInput: LineEdit = %SeedLineEdit
@onready var seedPreviewLabel: Label = %SeedPreviewLabel
@onready var randomSeedBtn: Button = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/NewGamePanel/MarginContainer/VBoxContainer/ButtonRow/RandomSeedButton
@onready var startNewBtn: Button = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/NewGamePanel/MarginContainer/VBoxContainer/ButtonRow/StartNewButton
@onready var newBackBtn: Button = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/NewGamePanel/MarginContainer/VBoxContainer/ButtonRow/BackButton
@onready var saveList: ItemList = %SaveList
@onready var loadDetailLabel: Label = %DetailLabel
@onready var loadSelectedBtn: Button = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/LoadGamePanel/MarginContainer/VBoxContainer/ButtonRow/LoadSelectedButton
@onready var refreshLoadBtn: Button = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/LoadGamePanel/MarginContainer/VBoxContainer/ButtonRow/RefreshButton
@onready var deleteLoadBtn: Button = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/LoadGamePanel/MarginContainer/VBoxContainer/ButtonRow/DeleteButton
@onready var loadBackBtn: Button = $MainContainer/MainPanelContainer/MainPanel/MarginContainer/LoadGamePanel/MarginContainer/VBoxContainer/ButtonRow/BackButton

var _selected_save_path: String = ""


func _ready() -> void:
	print("Initializing Menu")
	show_in_states = [AppState.State.MAIN_MENU]
	super._ready()
	_set_keyboard_focus_modes()
	_setup_menu_pages()
	_connect_menu_buttons()
	_refresh_save_list()
	# Connect signals for all buttons
	if startBtn:
		startBtn.pressed.connect(_on_start_button_pressed)
		startBtn.mouse_entered.connect(_on_button_hover)
	else:
		push_error("StartButton node not found")

	if loadBtn:
		loadBtn.pressed.connect(_on_load_button_pressed)
		loadBtn.mouse_entered.connect(_on_button_hover)
	else:
		push_error("LoadButton node not found")

	if settingsBtn:
		settingsBtn.pressed.connect(_on_settings_button_pressed)
		settingsBtn.mouse_entered.connect(_on_button_hover)
	else:
		push_error("SettingsButton node not found")

	if creditsBtn:
		creditsBtn.pressed.connect(_on_credits_button_pressed)
		creditsBtn.mouse_entered.connect(_on_button_hover)
	else:
		push_error("CreditsButton node not found")

	if quitBtn:
		quitBtn.pressed.connect(_on_quit_button_pressed)
		quitBtn.mouse_entered.connect(_on_button_hover)
	else:
		push_error("QuitButton node not found")

	if seedInput and not seedInput.text_changed.is_connected(_on_seed_text_changed):
		seedInput.text_changed.connect(_on_seed_text_changed)
	if randomSeedBtn and not randomSeedBtn.pressed.is_connected(_on_random_seed_pressed):
		randomSeedBtn.pressed.connect(_on_random_seed_pressed)
	if startNewBtn and not startNewBtn.pressed.is_connected(_on_start_new_run_pressed):
		startNewBtn.pressed.connect(_on_start_new_run_pressed)
	if newBackBtn and not newBackBtn.pressed.is_connected(_on_back_to_main_pressed):
		newBackBtn.pressed.connect(_on_back_to_main_pressed)
	if saveList and not saveList.item_selected.is_connected(_on_save_list_selected):
		saveList.item_selected.connect(_on_save_list_selected)
	if loadSelectedBtn and not loadSelectedBtn.pressed.is_connected(_on_load_selected_pressed):
		loadSelectedBtn.pressed.connect(_on_load_selected_pressed)
	if refreshLoadBtn and not refreshLoadBtn.pressed.is_connected(_on_refresh_load_pressed):
		refreshLoadBtn.pressed.connect(_on_refresh_load_pressed)
	if deleteLoadBtn and not deleteLoadBtn.pressed.is_connected(_on_delete_load_pressed):
		deleteLoadBtn.pressed.connect(_on_delete_load_pressed)
	if loadBackBtn and not loadBackBtn.pressed.is_connected(_on_back_to_main_pressed):
		loadBackBtn.pressed.connect(_on_back_to_main_pressed)

func _unhandled_input(event: InputEvent) -> void:
	if not visible:
		return
	if State_Manager.get_current_state() != AppState.State.MAIN_MENU:
		return

	if event.is_action_pressed("ui_cancel"):
		if _is_new_game_page_visible() or _is_load_page_visible():
			_show_main_menu()
		else:
			get_tree().quit()
		get_viewport().set_input_as_handled()
		return

	if event.is_action_pressed("ui_accept"):
		_activate_focused_menu_item()
		get_viewport().set_input_as_handled()
		return

	if _is_tab_focus_next(event):
		_cycle_menu_focus(1)
		get_viewport().set_input_as_handled()
		return

	if _is_tab_focus_prev(event):
		_cycle_menu_focus(-1)
		get_viewport().set_input_as_handled()
		return

	if _should_use_menu_focus_arrows() and (event.is_action_pressed("ui_up") or event.is_action_pressed("ui_down") or event.is_action_pressed("ui_left") or event.is_action_pressed("ui_right")):
		_move_menu_focus(event)
		get_viewport().set_input_as_handled()
		return


func on_enter_state(_state: int) -> void:
	print("Entering state")
	SettingsManager.load_settings()
	if Audio_Manager and Audio_Manager.has_method("ensure_music_playing"):
		Audio_Manager.ensure_music_playing()
	_show_main_menu()
	_refresh_save_list()
	visible = true

func on_exit_state(_state: int) -> void:
	print("Exiting state")
	visible = false

# Button handlers
func _on_start_button_pressed() -> void:
	print("New Game button pressed")
	_show_new_game_page()
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_load_button_pressed() -> void:
	print("Load button pressed")
	_show_load_page()
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_settings_button_pressed() -> void:
	print("Settings button pressed")
	request_state(AppState.State.SETTINGS)
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_credits_button_pressed() -> void:
	print("Credits button pressed")
	request_state(AppState.State.CREDITS)
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_quit_button_pressed() -> void:
	print("Quit button pressed")
	if Audio_Manager:
		Audio_Manager.play_sfx("click")
	get_tree().quit()

func _on_seed_text_changed(new_text: String) -> void:
	if seedPreviewLabel:
		var preview := new_text.strip_edges()
		if preview.is_empty():
			seedPreviewLabel.text = "Expedition: (random)"
		else:
			seedPreviewLabel.text = "%s  —  Seed %s" % [_expedition_name_for(preview), preview]

func _on_random_seed_pressed() -> void:
	if seedInput:
		seedInput.text = RunManager.generate_seed() if RunManager and RunManager.has_method("generate_seed") else str(Time.get_unix_time_from_system())
		seedInput.caret_column = seedInput.text.length()
		_on_seed_text_changed(seedInput.text)
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_start_new_run_pressed() -> void:
	var seed_text := ""
	if seedInput:
		seed_text = seedInput.text.strip_edges()
	var run_data: Dictionary = RunManager.start_new_run(seed_text)
	print("Starting new run with seed: %s" % str(run_data.get("seed", "")))
	State_Manager.change_state(AppState.State.PLAYING)
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_save_list_selected(index: int) -> void:
	if not saveList:
		return
	var data: Variant = saveList.get_item_metadata(index)
	if typeof(data) != TYPE_DICTIONARY:
		return
	_selected_save_path = str(data.get("file_path", ""))
	if loadDetailLabel:
		loadDetailLabel.text = _format_save_details(data)

func _on_load_selected_pressed() -> void:
	if _selected_save_path.is_empty():
		_set_load_detail_message("Select a save file first.")
		return
	var run_data := RunManager.load_run(_selected_save_path)
	if run_data.is_empty():
		_set_load_detail_message("Failed to load save file.")
		return
	print("Loaded run seed: %s" % str(run_data.get("seed", "")))
	State_Manager.change_state(AppState.State.PLAYING)
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_refresh_load_pressed() -> void:
	_refresh_save_list()
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _on_delete_load_pressed() -> void:
	if _selected_save_path.is_empty():
		_set_load_detail_message("Select a save file first.")
		return
	if RunManager and RunManager.has_method("delete_save") and RunManager.delete_save(_selected_save_path):
		_selected_save_path = ""
		_refresh_save_list()
		_set_load_detail_message("Save deleted.")
		if Audio_Manager:
			Audio_Manager.play_sfx("click")
		return
	_set_load_detail_message("Failed to delete save file.")

func _on_back_to_main_pressed() -> void:
	_show_main_menu()
	if Audio_Manager:
		Audio_Manager.play_sfx("click")

func _setup_menu_pages() -> void:
	if mainMenuPage:
		mainMenuPage.visible = true
	if newGamePanel:
		newGamePanel.visible = false
	if loadGamePanel:
		loadGamePanel.visible = false
	_on_seed_text_changed(seedInput.text if seedInput else "")
	call_deferred("_focus_main_menu_default")

func _connect_menu_buttons() -> void:
	pass

func _show_main_menu() -> void:
	if mainMenuPage:
		mainMenuPage.visible = true
	if newGamePanel:
		newGamePanel.visible = false
	if loadGamePanel:
		loadGamePanel.visible = false
	_selected_save_path = ""
	_refresh_save_list()
	_configure_focus_chain(_current_focus_chain())
	call_deferred("_focus_main_menu_default")

func _show_new_game_page() -> void:
	if mainMenuPage:
		mainMenuPage.visible = false
	if newGamePanel:
		newGamePanel.visible = true
	if loadGamePanel:
		loadGamePanel.visible = false
	# Pre-fill a random seed so the expedition always has a name to show
	if seedInput and seedInput.text.strip_edges().is_empty():
		seedInput.text = RunManager.generate_seed() if (RunManager and RunManager.has_method("generate_seed")) else str(Time.get_unix_time_from_system())
	if seedInput:
		seedInput.grab_focus()
		seedInput.caret_column = seedInput.text.length()
	_on_seed_text_changed(seedInput.text if seedInput else "")
	_configure_focus_chain(_current_focus_chain())
	call_deferred("_focus_new_game_default")

func _expedition_name_for(seed_text: String) -> String:
	if RunManager and RunManager.has_method("_generate_run_name"):
		return RunManager._generate_run_name(seed_text)
	return seed_text

func _show_load_page() -> void:
	if mainMenuPage:
		mainMenuPage.visible = false
	if newGamePanel:
		newGamePanel.visible = false
	if loadGamePanel:
		loadGamePanel.visible = true
	_refresh_save_list()
	if saveList and saveList.item_count > 0:
		saveList.select(0)
		_on_save_list_selected(0)
		saveList.grab_focus()
	else:
		_set_load_detail_message("No save files found. Start a new run to create one.")
	_configure_focus_chain(_current_focus_chain())
	call_deferred("_focus_load_page_default")

func _set_keyboard_focus_modes() -> void:
	var focusable_buttons: Array[Button] = [
		startBtn, loadBtn, settingsBtn, creditsBtn, quitBtn,
		randomSeedBtn, startNewBtn, newBackBtn,
		loadSelectedBtn, refreshLoadBtn, deleteLoadBtn, loadBackBtn
	]
	for button in focusable_buttons:
		if button:
			button.focus_mode = Control.FOCUS_ALL
	if seedInput:
		seedInput.focus_mode = Control.FOCUS_ALL
	if saveList:
		saveList.focus_mode = Control.FOCUS_ALL

func _focus_main_menu_default() -> void:
	_configure_focus_chain(_current_focus_chain())
	if startBtn and startBtn.visible:
		startBtn.grab_focus()

func _focus_new_game_default() -> void:
	_configure_focus_chain(_current_focus_chain())
	if seedInput and seedInput.visible:
		seedInput.grab_focus()
	elif startNewBtn and startNewBtn.visible:
		startNewBtn.grab_focus()

func _focus_load_page_default() -> void:
	_configure_focus_chain(_current_focus_chain())
	if saveList and saveList.visible and saveList.item_count > 0:
		saveList.grab_focus()
	elif loadSelectedBtn and loadSelectedBtn.visible:
		loadSelectedBtn.grab_focus()

func _move_menu_focus(event: InputEvent) -> void:
	var controls := _current_focus_chain()
	if controls.is_empty():
		return
	var current := get_viewport().gui_get_focus_owner()
	var index := controls.find(current)
	if index < 0:
		controls[0].grab_focus()
		return
	var delta := 0
	if event.is_action_pressed("ui_up") or event.is_action_pressed("ui_left"):
		delta = -1
	elif event.is_action_pressed("ui_down") or event.is_action_pressed("ui_right"):
		delta = 1
	if delta == 0:
		return
	index = (index + delta + controls.size()) % controls.size()
	controls[index].grab_focus()

func _configure_focus_chain(chain: Array[Control]) -> void:
	if chain.is_empty():
		return
	var chain_size := chain.size()
	for i in range(chain_size):
		var current := chain[i]
		if not current:
			continue
		var next_control := chain[(i + 1) % chain_size]
		var prev_control := chain[(i - 1 + chain_size) % chain_size]
		if next_control:
			var next_path := current.get_path_to(next_control)
			current.focus_next = next_path
		if prev_control:
			var prev_path := current.get_path_to(prev_control)
			current.focus_previous = prev_path

func _cycle_menu_focus(step: int) -> void:
	var controls := _current_focus_chain()
	if controls.is_empty():
		return
	var current := get_viewport().gui_get_focus_owner()
	var index := controls.find(current)
	if index < 0:
		if step < 0:
			controls[controls.size() - 1].grab_focus()
		else:
			controls[0].grab_focus()
		return
	index = (index + step + controls.size()) % controls.size()
	controls[index].grab_focus()

func _is_tab_focus_next(event: InputEvent) -> bool:
	if event.is_action_pressed("ui_focus_next"):
		return true
	if event is InputEventKey and event.pressed and not event.echo and event.keycode == KEY_TAB and not event.shift_pressed:
		return true
	return false

func _is_tab_focus_prev(event: InputEvent) -> bool:
	if event.is_action_pressed("ui_focus_prev"):
		return true
	if event is InputEventKey and event.pressed and not event.echo and event.keycode == KEY_TAB and event.shift_pressed:
		return true
	return false

func _should_use_menu_focus_arrows() -> bool:
	var focus := get_viewport().gui_get_focus_owner()
	if focus == null:
		return true
	if focus is LineEdit:
		return false
	if focus is ItemList:
		return false
	return true

func _activate_focused_menu_item() -> void:
	var focus := get_viewport().gui_get_focus_owner()
	if focus == startBtn:
		_on_start_button_pressed()
	elif focus == loadBtn:
		_on_load_button_pressed()
	elif focus == settingsBtn:
		_on_settings_button_pressed()
	elif focus == creditsBtn:
		_on_credits_button_pressed()
	elif focus == quitBtn:
		_on_quit_button_pressed()
	elif focus == randomSeedBtn:
		_on_random_seed_pressed()
	elif focus == startNewBtn:
		_on_start_new_run_pressed()
	elif focus == newBackBtn:
		_on_back_to_main_pressed()
	elif focus == loadSelectedBtn:
		_on_load_selected_pressed()
	elif focus == refreshLoadBtn:
		_on_refresh_load_pressed()
	elif focus == deleteLoadBtn:
		_on_delete_load_pressed()
	elif focus == loadBackBtn:
		_on_back_to_main_pressed()
	elif focus == seedInput:
		_on_start_new_run_pressed()
	elif focus == saveList and saveList.item_count > 0:
		_on_load_selected_pressed()

func _current_focus_chain() -> Array[Control]:
	if _is_new_game_page_visible():
		return _controls_from_candidates([seedInput, randomSeedBtn, startNewBtn, newBackBtn])
	if _is_load_page_visible():
		return _controls_from_candidates([saveList, loadSelectedBtn, refreshLoadBtn, deleteLoadBtn, loadBackBtn])
	return _controls_from_candidates([startBtn, loadBtn, settingsBtn, creditsBtn, quitBtn])

func _controls_from_candidates(candidates: Array) -> Array[Control]:
	var controls: Array[Control] = []
	for candidate in candidates:
		if candidate and candidate is Control and candidate.visible and candidate.focus_mode != Control.FOCUS_NONE:
			controls.append(candidate)
	return controls

func _is_new_game_page_visible() -> bool:
	return newGamePanel != null and newGamePanel.visible

func _is_load_page_visible() -> bool:
	return loadGamePanel != null and loadGamePanel.visible

func _refresh_save_list() -> void:
	if not saveList:
		return
	saveList.clear()
	_selected_save_path = ""
	var saves: Array = []
	if RunManager:
		saves = RunManager.list_saved_runs()
	if saves.is_empty():
		_set_load_detail_message("No save files found. Start a new run to create one.")
		return

	for save_data in saves:
		var display_name := str(save_data.get("display_name", "Save"))
		var seed := str(save_data.get("seed", ""))
		var room_id := str(save_data.get("current_room_id", "room_000"))
		var label := "%s | Seed %s | %s" % [display_name, seed, room_id]
		saveList.add_item(label)
		saveList.set_item_metadata(saveList.item_count - 1, save_data)
	if saveList.item_count > 0:
		saveList.select(0)
		_on_save_list_selected(0)

func _format_save_details(save_data: Dictionary) -> String:
	var lines: Array[String] = []
	lines.append("Save: %s" % str(save_data.get("display_name", save_data.get("file_name", "Save"))))
	lines.append("Seed: %s" % str(save_data.get("seed", "Random")))
	lines.append("Campaign: %s" % str(save_data.get("campaign", "asteroid_56_room")))
	lines.append("Room: %s" % str(save_data.get("current_room_id", "room_000")))
	lines.append("Rooms: %s" % str(save_data.get("room_count", 56)))
	lines.append("Updated: %s" % _format_unix_time(int(save_data.get("updated_at", 0))))
	lines.append("File: %s" % str(save_data.get("file_name", "")))
	return "\n".join(lines)

func _set_load_detail_message(message: String) -> void:
	if loadDetailLabel:
		loadDetailLabel.text = message

func _format_unix_time(unix_time: int) -> String:
	if unix_time <= 0:
		return "Unknown"
	var dt := Time.get_datetime_dict_from_unix_time(unix_time)
	return "%04d-%02d-%02d %02d:%02d" % [
		int(dt.get("year", 0)),
		int(dt.get("month", 0)),
		int(dt.get("day", 0)),
		int(dt.get("hour", 0)),
		int(dt.get("minute", 0))
	]

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
