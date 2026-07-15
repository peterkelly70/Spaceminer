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
	# Godot expects signed PCM8 here. Convert our unsigned renderer output
	# (128 = silence) into signed byte values (-128..127) before assigning.
	var signed_pcm := PackedByteArray()
	signed_pcm.resize(pcm_bytes.size())
	for i in range(pcm_bytes.size()):
		signed_pcm[i] = (int(pcm_bytes[i]) - 128) & 0xFF
	wav.data = signed_pcm
	wav.format = AudioStreamWAV.FORMAT_8_BITS
	wav.mix_rate = mix_rate
	wav.stereo = false
	wav.loop_mode = AudioStreamWAV.LOOP_DISABLED
	return wav


## Build an AudioStreamWAV from unsigned 8-bit PCM, upconverting to signed 16-bit.
## This keeps the synth source simple while giving Godot a cleaner playback buffer.
static func build_wav_16bit_from_u8(pcm_bytes: PackedByteArray, mix_rate: int) -> AudioStreamWAV:
	var wav := AudioStreamWAV.new()
	var signed_pcm := PackedByteArray()
	signed_pcm.resize(pcm_bytes.size() * 2)
	var out_i := 0
	for i in range(pcm_bytes.size()):
		var sample := (int(pcm_bytes[i]) - 128) / 128.0
		var s16 := int(clampf(sample, -1.0, 1.0) * 32767.0)
		signed_pcm[out_i] = s16 & 0xFF
		signed_pcm[out_i + 1] = (s16 >> 8) & 0xFF
		out_i += 2
	wav.data = signed_pcm
	wav.format = AudioStreamWAV.FORMAT_16_BITS
	wav.mix_rate = mix_rate
	wav.stereo = false
	wav.loop_mode = AudioStreamWAV.LOOP_DISABLED
	return wav


## Build an AudioStreamWAV from 16-bit little-endian signed PCM.
static func build_wav_16bit(pcm_bytes: PackedByteArray, mix_rate: int) -> AudioStreamWAV:
	var wav := AudioStreamWAV.new()
	wav.data = pcm_bytes
	wav.format = AudioStreamWAV.FORMAT_16_BITS
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


## Parse a RIFF/WAV file into raw unsigned 8-bit PCM plus metadata.
## Returns a Dictionary with keys: pcm (PackedByteArray), sample_rate (int),
## channels (int), bits_per_sample (int). Returns an empty Dictionary on error.
static func extract_pcm_from_wav_bytes(wav_bytes: PackedByteArray) -> Dictionary:
	if wav_bytes.size() < 44:
		return {}
	if _ascii_from_bytes(wav_bytes, 0, 4) != "RIFF":
		return {}
	if _ascii_from_bytes(wav_bytes, 8, 4) != "WAVE":
		return {}

	var sample_rate := 22050
	var channels := 1
	var bits_per_sample := 8
	var pcm := PackedByteArray()

	var pos := 12
	while pos + 8 <= wav_bytes.size():
		var chunk_id := _ascii_from_bytes(wav_bytes, pos, 4)
		var chunk_size := _read_u32_le(wav_bytes, pos + 4)
		var chunk_data_pos := pos + 8
		if chunk_data_pos + chunk_size > wav_bytes.size():
			break

		if chunk_id == "fmt " and chunk_size >= 16:
			channels = _read_u16_le(wav_bytes, chunk_data_pos + 2)
			sample_rate = int(_read_u32_le(wav_bytes, chunk_data_pos + 4))
			bits_per_sample = _read_u16_le(wav_bytes, chunk_data_pos + 14)
		elif chunk_id == "data":
			pcm = wav_bytes.slice(chunk_data_pos, chunk_data_pos + chunk_size)

		pos = chunk_data_pos + chunk_size
		if (chunk_size & 1) == 1:
			pos += 1

	if pcm.is_empty():
		return {}

	if bits_per_sample == 16:
		pcm = _downconvert_s16le_to_u8(pcm, channels)
	elif bits_per_sample != 8:
		push_warning("RetroAudioUtil: unsupported WAV bits_per_sample=%s" % bits_per_sample)
		return {}

	return {
		"pcm": pcm,
		"sample_rate": sample_rate,
		"channels": channels,
		"bits_per_sample": bits_per_sample,
	}


static func extract_pcm_from_wav_file(path: String) -> Dictionary:
	var wav_bytes := FileAccess.get_file_as_bytes(path)
	if wav_bytes.is_empty():
		return {}
	return extract_pcm_from_wav_bytes(wav_bytes)


static func _ascii_from_bytes(bytes: PackedByteArray, start: int, length: int) -> String:
	var chars: Array[String] = []
	for i in range(length):
		var idx := start + i
		if idx < 0 or idx >= bytes.size():
			break
		chars.append(char(bytes[idx]))
	return "".join(chars)


static func _read_u16_le(bytes: PackedByteArray, offset: int) -> int:
	if offset < 0 or offset + 1 >= bytes.size():
		return 0
	return int(bytes[offset]) | (int(bytes[offset + 1]) << 8)


static func _read_u32_le(bytes: PackedByteArray, offset: int) -> int:
	if offset < 0 or offset + 3 >= bytes.size():
		return 0
	return int(bytes[offset]) \
		| (int(bytes[offset + 1]) << 8) \
		| (int(bytes[offset + 2]) << 16) \
		| (int(bytes[offset + 3]) << 24)


static func _downconvert_s16le_to_u8(pcm16: PackedByteArray, channels: int) -> PackedByteArray:
	var out := PackedByteArray()
	if channels <= 1:
		out.resize(pcm16.size() / 2)
		var out_i := 0
		for i in range(0, pcm16.size() - 1, 2):
			var sample := int(pcm16[i]) | (int(pcm16[i + 1]) << 8)
			if sample >= 32768:
				sample -= 65536
			out[out_i] = clamp_i(((sample + 32768) >> 8), 0, 255)
			out_i += 1
		return out

	var frame_bytes := channels * 2
	var frame_count := pcm16.size() / frame_bytes
	out.resize(frame_count)
	for frame in range(frame_count):
		var sum := 0
		for ch in range(channels):
			var base := frame * frame_bytes + ch * 2
			if base + 1 >= pcm16.size():
				continue
			var sample := int(pcm16[base]) | (int(pcm16[base + 1]) << 8)
			if sample >= 32768:
				sample -= 65536
			sum += sample
		var avg := int(round(float(sum) / float(channels)))
		out[frame] = clamp_i(((avg + 32768) >> 8), 0, 255)
	return out
