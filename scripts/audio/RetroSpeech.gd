# RetroSpeech.gd
# Public API for the SAM-inspired retro speech synthesiser.
#
# Usage (from any Node):
#
#   # Simple one-shot speak using default voice
#   RetroSpeech.new().speak("BEWARE THE FOREST", $AudioStreamPlayer)
#
#   # Render to AudioStreamWAV and assign manually
#   var wav := RetroSpeech.render_to_stream("DANGER")
#   $AudioStreamPlayer.stream = wav
#   $AudioStreamPlayer.play()
#
#   # Use a named voice profile
#   var wav := RetroSpeech.render_to_stream("KILLAPEDE", RetroSpeech.VOICE_BUG)
#
class_name RetroSpeech
extends RefCounted

# ---------------------------------------------------------------------------
# Built-in voice profiles
# ---------------------------------------------------------------------------

## Standard old-computer voice: 8 kHz, heavy crunch, flat delivery.
static var VOICE_OLD_COMPUTER: SamVoice:
	get:
		var v := SamVoice.new()
		v.sample_rate = 8000
		v.pitch       = 64
		v.speed       = 1.0
		v.throat      = 100
		v.mouth       = 110
		v.crunch      = 0.85
		v.crunch_bits = 4
		v.volume      = 0.9
		return v

## Robot voice: low pitch, deliberate pace, hard resonances.
static var VOICE_ROBOT: SamVoice:
	get:
		var v := SamVoice.new()
		v.sample_rate = 8000
		v.pitch       = 90
		v.speed       = 0.75
		v.throat      = 160
		v.mouth       = 100
		v.crunch      = 0.5
		v.crunch_bits = 5
		v.volume      = 0.9
		return v

## Bug voice: high pitch, rapid-fire, clipped.
static var VOICE_BUG: SamVoice:
	get:
		var v := SamVoice.new()
		v.sample_rate = 11025
		v.pitch       = 30
		v.speed       = 2.0
		v.throat      = 80
		v.mouth       = 200
		v.crunch      = 0.6
		v.crunch_bits = 5
		v.volume      = 0.85
		return v

# ---------------------------------------------------------------------------
# Internal helpers
# ---------------------------------------------------------------------------

var _reciter  := SamReciter.new()
var _parser   := SamParser.new()
var _renderer := SamRenderer.new()


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

## Speak text through an AudioStreamPlayer.
## Uses the default voice if none is provided.
func speak(text: String, player: AudioStreamPlayer, voice: SamVoice = null) -> void:
	var wav := render_to_stream(text, voice)
	if wav == null:
		push_error("RetroSpeech.speak: render failed for text: %s" % text)
		return
	player.stream = wav
	player.play()


## Render text to an AudioStreamWAV.
## Returns null on error.
func render_to_stream(text: String, voice: SamVoice = null) -> AudioStreamWAV:
	var pcm := render_to_pcm(text, voice)
	if pcm.is_empty():
		return null
	var eff_voice := voice if voice != null else _default_voice()
	return RetroAudioUtil.build_wav_8bit(pcm, eff_voice.sample_rate)


## Render text to a raw 8-bit PCM PackedByteArray (128 = silence).
func render_to_pcm(text: String, voice: SamVoice = null) -> PackedByteArray:
	var eff_voice := voice if voice != null else _default_voice()
	var phoneme_string := _reciter.text_to_phonemes(text)
	return render_phonemes_to_pcm(phoneme_string, eff_voice)


## Render a pre-built phoneme string directly to an AudioStreamWAV.
## Useful for testing specific pronunciations.
func render_phonemes_to_stream(phoneme_string: String, voice: SamVoice = null) -> AudioStreamWAV:
	var pcm := render_phonemes_to_pcm(phoneme_string, voice)
	if pcm.is_empty():
		return null
	var eff_voice := voice if voice != null else _default_voice()
	return RetroAudioUtil.build_wav_8bit(pcm, eff_voice.sample_rate)


## Render a phoneme string to raw 8-bit PCM.
func render_phonemes_to_pcm(phoneme_string: String, voice: SamVoice = null) -> PackedByteArray:
	var eff_voice := voice if voice != null else _default_voice()
	var frames := _parser.parse(phoneme_string)
	if frames.is_empty():
		push_warning("RetroSpeech: no frames produced from phoneme string: %s" % phoneme_string)
		return PackedByteArray()
	return _renderer.render(frames, eff_voice)


## Convert plain text to its phoneme string without rendering.
## Useful for the debug label in the test scene.
func text_to_phoneme_string(text: String) -> String:
	return _reciter.text_to_phonemes(text)


# ---------------------------------------------------------------------------
# Private
# ---------------------------------------------------------------------------

func _default_voice() -> SamVoice:
	return VOICE_OLD_COMPUTER
