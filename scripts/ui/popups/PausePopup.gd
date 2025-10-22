extends Control
class_name PausePopup

# Pause popup: resume, exit to main menu, music + sfx volume controls

@onready var _music_slider: HSlider = %MusicSlider
@onready var _track_select: OptionButton = %TrackSelect
@onready var _sfx_slider: HSlider = %SfxSlider
@onready var _resume_button: Button = %ResumeButton
@onready var _exit_button: Button = %ExitButton
@onready var _mute_check: CheckButton = %MuteButton

func _ready() -> void:
	process_mode = Node.PROCESS_MODE_WHEN_PAUSED
	# Ensure this popup root captures input clicks
	mouse_filter = Control.MOUSE_FILTER_STOP
	
	# Debug: confirm popup is ready while paused
	if Engine.is_editor_hint() == false:
		print("[PausePopup] ready (paused=", get_tree().paused, ")")
	
	# Initialize UI from Audio_Manager
	var am = get_node("/root/Audio_Manager")
	_music_slider.value = am.music_volume * 100.0
	_sfx_slider.value = am.sfx_volume * 100.0
	_mute_check.button_pressed = am.is_muted()

	# Populate tracks from MusicManager
	var mm = get_tree().get_first_node_in_group("music_manager")
	if mm and mm.get_current_collection():
		_track_select.clear()
		var tracks = mm.get_current_collection().get_valid_tracks()
		for i in tracks.size():
			var t = tracks[i]
			_track_select.add_item(t.display_name, i)
			_track_select.set_item_metadata(i, t.track_name)
		# Select current track if any
		var cur = mm.get_current_track()
		if cur:
			for i in _track_select.item_count:
				if _track_select.get_item_metadata(i) == cur.track_name:
					_track_select.select(i)
					break
	# React to track changes
	if mm and not mm.track_changed.is_connected(_on_track_changed):
		mm.track_changed.connect(_on_track_changed)
	
	# Connect UI
	_music_slider.value_changed.connect(_on_music_changed)
	_sfx_slider.value_changed.connect(_on_sfx_changed)
	_mute_check.toggled.connect(_on_mute_toggled)
	_resume_button.pressed.connect(_on_resume)
	_exit_button.pressed.connect(_on_exit)
	_track_select.item_selected.connect(_on_track_selected)

	# Ensure focus/input is allowed on buttons while paused
	_resume_button.focus_mode = Control.FOCUS_ALL
	_exit_button.focus_mode = Control.FOCUS_ALL
	_resume_button.disabled = false
	_exit_button.disabled = false
	focus_default()

func _on_music_changed(v: float) -> void:
	var am = get_node("/root/Audio_Manager")
	am.set_music_volume(v / 100.0)

func _on_sfx_changed(v: float) -> void:
	var am = get_node("/root/Audio_Manager")
	am.set_sfx_volume(v / 100.0)

func _on_mute_toggled(on: bool) -> void:
	var am = get_node("/root/Audio_Manager")
	am.set_mute(on)

func _on_resume() -> void:
	if Engine.is_editor_hint() == false:
		print("[PausePopup] Resume pressed")
	get_tree().paused = false
	hide()

func _on_exit() -> void:
	if Engine.is_editor_hint() == false:
		print("[PausePopup] Exit pressed")
	# Exit to main menu action
	State_Manager.change_state(GameState.GameState.MAIN_MENU)

func focus_default() -> void:
	if is_instance_valid(_resume_button):
		_resume_button.grab_focus()

func _on_track_selected(_idx: int) -> void:
	var mm = get_tree().get_first_node_in_group("music_manager")
	if not mm:
		return
	var name = _track_select.get_selected_metadata()
	if typeof(name) == TYPE_NIL:
		return
	mm.play_track_by_name(name)

func _on_track_changed(track) -> void:
	# Reflect external changes in dropdown
	for i in _track_select.item_count:
		if _track_select.get_item_metadata(i) == track.track_name:
			_track_select.select(i)
			break
