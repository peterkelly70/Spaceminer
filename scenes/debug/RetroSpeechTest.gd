# RetroSpeechTest.gd
# Controller for the RetroSpeechTest debug scene.
# Attach to the root Control node of RetroSpeechTest.tscn.
extends Control

# ---------------------------------------------------------------------------
# Node references (set in _ready via get_node)
# ---------------------------------------------------------------------------
@onready var input_line: LineEdit          = $VBoxContainer/InputLine
@onready var voice_option: OptionButton    = $VBoxContainer/VoiceOption
@onready var pitch_spin: SpinBox           = $VBoxContainer/GridContainer/PitchSpin
@onready var speed_spin: SpinBox           = $VBoxContainer/GridContainer/SpeedSpin
@onready var throat_spin: SpinBox          = $VBoxContainer/GridContainer/ThroatSpin
@onready var mouth_spin: SpinBox           = $VBoxContainer/GridContainer/MouthSpin
@onready var speak_button: Button          = $VBoxContainer/SpeakButton
@onready var phoneme_label: Label          = $VBoxContainer/PhonemeLabel
@onready var audio_player: AudioStreamPlayer = $VBoxContainer/AudioStreamPlayer

# Lazily created RetroSpeech instance
var _speech: RetroSpeech = null

# ---------------------------------------------------------------------------
# Voice preset names matching RetroSpeech constants
# ---------------------------------------------------------------------------
const VOICE_NAMES: Array[String] = [
	"Old Computer",
	"Robot",
	"Bug",
]


func _ready() -> void:
	_speech = RetroSpeech.new()

	# Populate voice dropdown
	voice_option.clear()
	for name in VOICE_NAMES:
		voice_option.add_item(name)

	# Set defaults for spinboxes
	pitch_spin.value  = 64
	pitch_spin.min_value = 10
	pitch_spin.max_value = 120

	speed_spin.value  = 1.0
	speed_spin.min_value = 0.25
	speed_spin.max_value = 4.0
	speed_spin.step   = 0.05

	throat_spin.value = 128
	throat_spin.min_value = 0
	throat_spin.max_value = 255

	mouth_spin.value  = 128
	mouth_spin.min_value = 0
	mouth_spin.max_value = 255

	# Wire up signals
	speak_button.pressed.connect(_on_speak_pressed)
	voice_option.item_selected.connect(_on_voice_selected)

	# Load spinbox values from current preset
	_sync_spinboxes_to_preset()


func _on_speak_pressed() -> void:
	var text := input_line.text.strip_edges()
	if text.is_empty():
		return

	var voice := _build_voice()
	var phonemes := _speech.text_to_phoneme_string(text)
	phoneme_label.text = "Phonemes: " + phonemes

	var wav := _speech.render_to_stream(text, voice)
	if wav == null:
		phoneme_label.text += "\n[ERROR: render returned null]"
		return

	audio_player.stream = wav
	audio_player.play()


func _on_voice_selected(_index: int) -> void:
	_sync_spinboxes_to_preset()


## Sync spinbox values when a new preset is chosen.
func _sync_spinboxes_to_preset() -> void:
	var voice := _get_preset_voice()
	pitch_spin.value  = voice.pitch
	speed_spin.value  = voice.speed
	throat_spin.value = voice.throat
	mouth_spin.value  = voice.mouth


## Build a SamVoice from the current preset, then apply spinbox overrides.
func _build_voice() -> SamVoice:
	var voice := _get_preset_voice()
	voice.pitch  = int(pitch_spin.value)
	voice.speed  = speed_spin.value
	voice.throat = int(throat_spin.value)
	voice.mouth  = int(mouth_spin.value)
	return voice


## Return the base SamVoice for the currently selected preset.
func _get_preset_voice() -> SamVoice:
	match voice_option.selected:
		0: return RetroSpeech.VOICE_OLD_COMPUTER
		1: return RetroSpeech.VOICE_ROBOT
		2: return RetroSpeech.VOICE_BUG
		_: return RetroSpeech.VOICE_OLD_COMPUTER
