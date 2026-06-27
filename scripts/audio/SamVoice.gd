# SamVoice.gd
# Voice profile for the SAM-inspired retro speech synthesiser.
# Tweak these values to change the character of the voice.
class_name SamVoice
extends Resource

# ---------------------------------------------------------------------------
# Core tuning parameters
# ---------------------------------------------------------------------------

## Output sample rate in Hz. Lower values = crunchier, more retro.
## Typical: 8000, 11025, 22050.
@export var sample_rate: int = 8000

## Base pitch period in samples at the chosen sample_rate.
## Higher number = lower pitch. Range roughly 20–120.
@export var pitch: int = 64

## Playback speed multiplier. 1.0 = normal, 0.5 = half speed, 2.0 = double.
@export var speed: float = 1.0

## Throat parameter (0–255). Controls the spectral tilt / resonance character.
## Lower values = thinner, higher = fuller/darker.
@export var throat: int = 128

## Mouth parameter (0–255). Controls mouth opening / F2 bias.
## Higher = more open / brighter vowels.
@export var mouth: int = 128

## Crunch factor (0.0–1.0). Adds hard clipping / bit-depth reduction artefacts.
## 0.0 = clean, 1.0 = heavily quantised.
@export var crunch: float = 0.5

## Bit depth for quantisation during crunch. 4 = very 8-bit, 8 = standard.
@export var crunch_bits: int = 6

## Volume scale applied to final output (0.0–1.0).
@export var volume: float = 0.85

# ---------------------------------------------------------------------------
# Constructor helpers
# ---------------------------------------------------------------------------

func _init() -> void:
	pass


## Return a deep copy so callers can tweak without touching the original.
func duplicate_profile() -> SamVoice:
	var v := SamVoice.new()
	v.sample_rate  = sample_rate
	v.pitch        = pitch
	v.speed        = speed
	v.throat       = throat
	v.mouth        = mouth
	v.crunch       = crunch
	v.crunch_bits  = crunch_bits
	v.volume       = volume
	return v
