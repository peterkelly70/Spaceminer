extends Control
class_name PausePopup

@onready var _music_slider: HSlider = %MusicSlider
@onready var _music_value_label: Label = %MusicValueLabel
@onready var _sfx_slider: HSlider = %SfxSlider
@onready var _sfx_value_label: Label = %SfxValueLabel
@onready var _resume_button: Button = %ResumeButton
@onready var _exit_button: Button = %ExitButton

func _ready() -> void:
	process_mode = Node.PROCESS_MODE_WHEN_PAUSED
	mouse_filter = Control.MOUSE_FILTER_STOP

	var am = get_node_or_null("/root/Audio_Manager")
	if am:
		_music_slider.value = am.music_volume * 100.0
		_sfx_slider.value = am.sfx_volume * 100.0
	_update_music_label(_music_slider.value)
	_update_sfx_label(_sfx_slider.value)

	_music_slider.value_changed.connect(_on_music_changed)
	_sfx_slider.value_changed.connect(_on_sfx_changed)
	_resume_button.pressed.connect(_on_resume)
	_exit_button.pressed.connect(_on_exit)

	_resume_button.focus_mode = Control.FOCUS_ALL
	_exit_button.focus_mode = Control.FOCUS_ALL
	_resume_button.disabled = false
	_exit_button.disabled = false
	focus_default()

func _input(event: InputEvent) -> void:
	if not visible:
		return
	if event.is_action_pressed("ui_cancel"):
		_on_resume()
		get_viewport().set_input_as_handled()

func _update_music_label(v: float) -> void:
	_music_value_label.text = "%d%%" % int(v)

func _update_sfx_label(v: float) -> void:
	_sfx_value_label.text = "%d%%" % int(v)

func _on_music_changed(v: float) -> void:
	_update_music_label(v)
	var am = get_node_or_null("/root/Audio_Manager")
	if am:
		am.set_music_volume(v / 100.0)

func _on_sfx_changed(v: float) -> void:
	_update_sfx_label(v)
	var am = get_node_or_null("/root/Audio_Manager")
	if am:
		am.set_sfx_volume(v / 100.0)

func _on_resume() -> void:
	get_tree().paused = false
	State_Manager.change_state(AppState.State.PLAYING)
	hide()

func _on_exit() -> void:
	get_tree().paused = false
	_save_game()
	State_Manager.change_state(AppState.State.MAIN_MENU)
	hide()

func _save_game() -> void:
	if has_node("/root/Save_Manager"):
		var sm = get_node("/root/Save_Manager")
		if sm.has_method("save_game"):
			sm.save_game()
			return
	# Persist audio settings via SettingsManager so they survive relaunch
	var am = get_node_or_null("/root/Audio_Manager")
	if am:
		SettingsManager.set_setting("audio", "music_volume", am.music_volume)
		SettingsManager.set_setting("audio", "sfx_volume", am.sfx_volume)

func focus_default() -> void:
	if is_instance_valid(_resume_button):
		_resume_button.grab_focus()
