# RetroAudioUtil.gd
# Low-level helpers for the SAM-inspired retro speech synthesiser.
# All functions are static so callers never need to instantiate this class.
class_name RetroAudioUtil
extends RefCounted


# ---------------------------------------------------------------------------
# Byte / integer helpers
# ---------------------------------------------------------------------------

## Wrap an integer to the range [0, 255] (unsigned 8-bit).
static func wrap_u8(v: int) -> int:
	return v & 0xFF


## Reinterpret an unsigned 8-bit value as a signed 8-bit value.
static func to_s8(v: int) -> int:
	v = v & 0xFF
	if v >= 128:
		return v - 256
	return v


## Clamp an integer to [lo, hi].
static func clamp_i(v: int, lo: int, hi: int) -> int:
	if v < lo:
		return lo
	if v > hi:
		return hi
	return v


## Safely index an Array. Returns default_val when the index is out of range,
## avoiding crashes on malformed phoneme data.
static func safe_table_get(table: Array, index: int, default_val: int) -> int:
	if index < 0 or index >= table.size():
		return default_val
	return int(table[index])


## Safely index a PackedByteArray. Returns default_val when out of range.
static func safe_byte_get(table: PackedByteArray, index: int, default_val: int) -> int:
	if index < 0 or index >= table.size():
		return default_val
	return int(table[index])


# ---------------------------------------------------------------------------
# AudioStreamWAV builders
# ---------------------------------------------------------------------------

## Build an AudioStreamWAV from unsigned 8-bit PCM (128 = silence).
static func build_wav_8bit(pcm_bytes: PackedByteArray, mix_rate: int) -> AudioStreamWAV:
	var wav := AudioStreamWAV.new()
	wav.data = pcm_bytes
	wav.format = AudioStreamWAV.FORMAT_8_BIT
	wav.mix_rate = mix_rate
	wav.stereo = false
	wav.loop_mode = AudioStreamWAV.LOOP_DISABLED
	return wav


## Build an AudioStreamWAV from 16-bit little-endian signed PCM.
static func build_wav_16bit(pcm_bytes: PackedByteArray, mix_rate: int) -> AudioStreamWAV:
	var wav := AudioStreamWAV.new()
	wav.data = pcm_bytes
	wav.format = AudioStreamWAV.FORMAT_16_BIT
	wav.mix_rate = mix_rate
	wav.stereo = false
	wav.loop_mode = AudioStreamWAV.LOOP_DISABLED
	return wav


## Pack a float in [-1.0, 1.0] as an unsigned 8-bit byte (0-255).
## 0.0 → 128.
static func float_to_u8(f: float) -> int:
	var v := int(f * 127.0) + 128
	return clamp_i(v, 0, 255)


## Pack a float in [-1.0, 1.0] as two little-endian bytes (signed 16-bit).
static func float_to_s16le(f: float) -> PackedByteArray:
	var v := int(clampf(f, -1.0, 1.0) * 32767.0)
	var b := PackedByteArray()
	b.resize(2)
	b[0] = v & 0xFF
	b[1] = (v >> 8) & 0xFF
	return b


## Append all bytes of src onto dst in-place.
static func append_bytes(dst: PackedByteArray, src: PackedByteArray) -> void:
	for byte in src:
		dst.append(byte)
