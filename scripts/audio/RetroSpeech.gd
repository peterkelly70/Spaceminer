# RetroSpeech.gd
# Public API for SpaceMiner speech.
#
# This is now a thin compatibility wrapper over the upstream SAM binary
# bridge in SamBridge.gd, so the project follows the real implementation
# path rather than the earlier approximation.
class_name RetroSpeech
extends RefCounted

const SAM_BRIDGE := preload("res://scripts/audio/SamBridge.gd")

# ---------------------------------------------------------------------------
# Built-in voice profiles
# ---------------------------------------------------------------------------

## Standard old-computer voice: crunchy, flat, 8-bit style.
static var VOICE_OLD_COMPUTER: SamVoice:
	get:
		var v := SamVoice.new()
		v.sample_rate = 8000
		v.pitch = 64
		v.speed = 1.0
		v.throat = 100
		v.mouth = 110
		v.crunch = 0.85
		v.crunch_bits = 4
		v.volume = 0.9
		return v

## Robot voice: lower pitch, slower pace, harder resonance.
static var VOICE_ROBOT: SamVoice:
	get:
		var v := SamVoice.new()
		v.sample_rate = 8000
		v.pitch = 90
		v.speed = 0.75
		v.throat = 160
		v.mouth = 100
		v.crunch = 0.5
		v.crunch_bits = 5
		v.volume = 0.9
		return v

## Bug voice: high pitch, quick, clipped.
static var VOICE_BUG: SamVoice:
	get:
		var v := SamVoice.new()
		v.sample_rate = 11025
		v.pitch = 30
		v.speed = 2.0
		v.throat = 80
		v.mouth = 200
		v.crunch = 0.6
		v.crunch_bits = 5
		v.volume = 0.85
		return v


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

## Speak text through an AudioStreamPlayer.
func speak(text: String, player: AudioStreamPlayer, voice: SamVoice = null) -> void:
	var wav := render_to_stream(text, voice)
	if wav == null:
		push_error("RetroSpeech.speak: render failed for text: %s" % text)
		return
	player.stream = wav
	player.play()


## Render text to an AudioStreamWAV using the upstream SAM binary.
func render_to_stream(text: String, voice: SamVoice = null) -> AudioStreamWAV:
	return SAM_BRIDGE.render_text_to_stream(text, _effective_voice(voice))


## Render text to raw unsigned 8-bit PCM.
func render_to_pcm(text: String, voice: SamVoice = null) -> PackedByteArray:
	return SAM_BRIDGE.render_text_to_pcm(text, _effective_voice(voice))


## Render a phoneme string directly to an AudioStreamWAV.
func render_phonemes_to_stream(phoneme_string: String, voice: SamVoice = null) -> AudioStreamWAV:
	return SAM_BRIDGE.render_phonemes_to_stream(phoneme_string, _effective_voice(voice))


## Render a phoneme string to raw unsigned 8-bit PCM.
func render_phonemes_to_pcm(phoneme_string: String, voice: SamVoice = null) -> PackedByteArray:
	return SAM_BRIDGE.render_phonemes_to_pcm(phoneme_string, _effective_voice(voice))


## Convert plain text to the phoneme string emitted by the SAM reciter.
func text_to_phoneme_string(text: String) -> String:
	return SAM_BRIDGE.text_to_phoneme_string(text)


# ---------------------------------------------------------------------------
# Private
# ---------------------------------------------------------------------------

func _effective_voice(voice: SamVoice) -> SamVoice:
	return voice if voice != null else VOICE_OLD_COMPUTER
