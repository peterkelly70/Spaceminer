# SamBridge.gd
# Thin wrapper around the upstream SAM command-line implementation.
# This keeps SpaceMiner's speech path aligned with the original table-driven
# algorithm instead of the earlier clean-room approximation.
class_name SamBridge
extends RefCounted

const SAM_TOOL_DIR := "res://tools/sam"
const SAM_BIN_NAME := "sam"
const SAM_BUILD_SCRIPT := "build_sam.sh"
const SAM_CACHE_DIR := "user://.sam_cache"

static var _sam_bin_path: String = ""


static func text_to_phoneme_string(text: String) -> String:
	var args := PackedStringArray([
		"-debug",
		"-pitch", str(_default_pitch()),
		"-speed", str(_default_speed()),
		"-mouth", str(_default_mouth()),
		"-throat", str(_default_throat()),
	])
	args.append_array(_split_text_args(text))
	var exec_result := _run_sam(args)

	if exec_result["exit_code"] != 0:
		push_warning("SAM reciter failed for text: %s" % text)
		return ""

	return _extract_phoneme_line(exec_result["stdout"])


static func render_text_to_pcm(text: String, voice: SamVoice = null) -> PackedByteArray:
	var wav_info := _render_text_to_wav_data(text, voice)
	if wav_info.is_empty():
		return PackedByteArray()
	return wav_info["pcm"]


static func render_text_to_stream(text: String, voice: SamVoice = null) -> AudioStreamWAV:
	var wav_info := _render_text_to_wav_data(text, voice)
	if wav_info.is_empty():
		return null
	return RetroAudioUtil.build_wav_16bit_from_u8(wav_info["pcm"], wav_info["sample_rate"])


static func render_phonemes_to_pcm(phoneme_string: String, voice: SamVoice = null) -> PackedByteArray:
	var wav_info := _render_phonemes_to_wav_data(phoneme_string, voice)
	if wav_info.is_empty():
		return PackedByteArray()
	return wav_info["pcm"]


static func render_phonemes_to_stream(phoneme_string: String, voice: SamVoice = null) -> AudioStreamWAV:
	var wav_info := _render_phonemes_to_wav_data(phoneme_string, voice)
	if wav_info.is_empty():
		return null
	return RetroAudioUtil.build_wav_16bit_from_u8(wav_info["pcm"], wav_info["sample_rate"])


static func _render_text_to_wav_data(text: String, voice: SamVoice = null) -> Dictionary:
	var eff_voice := voice if voice != null else _default_voice()
	var temp_wav := _make_temp_wav_path(text, eff_voice, "text")
	var args := _sam_voice_args(eff_voice)
	args.append_array(["-wav", temp_wav])
	args.append_array(_split_text_args(text))

	var exec_result := _run_sam(args)
	if exec_result["exit_code"] != 0:
		push_warning("SAM render failed for text: %s" % text)
		return {}

	if not FileAccess.file_exists(temp_wav):
		push_warning("SAM render did not create wav file: %s" % temp_wav)
		return {}

	var wav_info := RetroAudioUtil.extract_pcm_from_wav_file(temp_wav)
	_safe_remove_file(temp_wav)
	return wav_info


static func _render_phonemes_to_wav_data(phoneme_string: String, voice: SamVoice = null) -> Dictionary:
	var eff_voice := voice if voice != null else _default_voice()
	var temp_wav := _make_temp_wav_path(phoneme_string, eff_voice, "phonemes")
	var args := _sam_voice_args(eff_voice)
	args.append_array(["-phonetic", "-wav", temp_wav, _to_sam_phonetic_string(phoneme_string)])

	var exec_result := _run_sam(args)
	if exec_result["exit_code"] != 0:
		push_warning("SAM phoneme render failed for phoneme string: %s" % phoneme_string)
		return {}

	if not FileAccess.file_exists(temp_wav):
		push_warning("SAM phoneme render did not create wav file: %s" % temp_wav)
		return {}

	var wav_info := RetroAudioUtil.extract_pcm_from_wav_file(temp_wav)
	_safe_remove_file(temp_wav)
	return wav_info


static func _run_sam(args: PackedStringArray) -> Dictionary:
	var bin_path := _ensure_sam_binary()
	if bin_path.is_empty():
		return {"exit_code": -1, "stdout": ""}

	var output: Array[String] = []
	var exit_code := OS.execute(bin_path, args, output, true)
	return {
		"exit_code": exit_code,
		"stdout": "\n".join(output),
		"lines": output,
	}


static func _ensure_sam_binary() -> String:
	if not _sam_bin_path.is_empty() and FileAccess.file_exists(_sam_bin_path):
		return _sam_bin_path

	var bin_path := ProjectSettings.globalize_path("%s/%s" % [SAM_TOOL_DIR, SAM_BIN_NAME])
	if FileAccess.file_exists(bin_path):
		_sam_bin_path = bin_path
		return _sam_bin_path

	var build_script := ProjectSettings.globalize_path("%s/%s" % [SAM_TOOL_DIR, SAM_BUILD_SCRIPT])
	if not FileAccess.file_exists(build_script):
		push_error("SAM bridge missing binary and build script: %s" % build_script)
		return ""

	var build_output: Array[String] = []
	var build_code := OS.execute("bash", [build_script], build_output, true)
	if build_code != 0:
		push_error("SAM build failed: %s" % "\n".join(build_output))
		return ""

	if FileAccess.file_exists(bin_path):
		_sam_bin_path = bin_path
		return _sam_bin_path

	push_error("SAM build completed but binary is still missing: %s" % bin_path)
	return ""


static func _sam_voice_args(voice: SamVoice) -> PackedStringArray:
	var args := PackedStringArray()
	args.append_array(["-pitch", str(clampi(voice.pitch, 1, 255))])
	args.append_array(["-speed", str(_speed_multiplier_to_sam_speed(voice.speed))])
	args.append_array(["-mouth", str(clampi(voice.mouth, 0, 255))])
	args.append_array(["-throat", str(clampi(voice.throat, 0, 255))])
	return args


static func _speed_multiplier_to_sam_speed(speed: float) -> int:
	var clamped := clampf(speed, 0.25, 4.0)
	return clampi(int(round(72.0 * clamped)), 1, 255)


static func _split_text_args(text: String) -> PackedStringArray:
	var args := PackedStringArray()
	for token in text.strip_edges().split(" ", false):
		if not token.is_empty():
			args.append(token)
	return args


static func _extract_phoneme_line(stdout_text: String) -> String:
	for line in stdout_text.split("\n", false):
		if line.begins_with("phonetic input:"):
			var phoneme_text := line.substr("phonetic input:".length()).strip_edges()
			return _strip_nonprintable(phoneme_text)
	return ""


static func _strip_nonprintable(text: String) -> String:
	var cleaned := ""
	for i in range(text.length()):
		var code := text.unicode_at(i)
		if code >= 32 and code <= 126:
			cleaned += text.substr(i, 1)
	return cleaned.strip_edges()


static func _to_sam_phonetic_string(phoneme_string: String) -> String:
	var out := ""
	var tokens := phoneme_string.strip_edges().split(" ", false)
	for token in tokens:
		var clean := token.strip_edges().to_upper()
		if clean.is_empty():
			continue
		if clean == "/" or clean == "." or clean == ",":
			out += "."
			continue
		var stress := ""
		if clean.length() > 0:
			var tail := clean.substr(clean.length() - 1, 1)
			if tail >= "0" and tail <= "9":
				stress = tail
				clean = clean.substr(0, clean.length() - 1)
		match clean:
			"NG":
				clean = "NX"
			"JH":
				clean = "J"
			"SH":
				clean = "SH"
			"ZH":
				clean = "ZH"
			"CH":
				clean = "CH"
			"PAUSE":
				out += "."
				continue
		out += clean + stress
	return out


static func _make_temp_wav_path(seed_text: String, voice: SamVoice, prefix: String) -> String:
	var cache_dir := ProjectSettings.globalize_path(SAM_CACHE_DIR)
	DirAccess.make_dir_recursive_absolute(cache_dir)
	var token := "%s_%s_%s_%s_%s" % [
		prefix,
		seed_text.hash(),
		voice.pitch,
		voice.mouth,
		Time.get_ticks_usec(),
	]
	return "%s/%s.wav" % [cache_dir, token]


static func _safe_remove_file(path: String) -> void:
	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(path)


static func _default_pitch() -> int:
	return 64


static func _default_speed() -> int:
	return 72


static func _default_mouth() -> int:
	return 128


static func _default_throat() -> int:
	return 128


static func _default_voice() -> SamVoice:
	var voice := SamVoice.new()
	voice.sample_rate = 22050
	voice.pitch = _default_pitch()
	voice.speed = 1.0
	voice.throat = _default_throat()
	voice.mouth = _default_mouth()
	voice.crunch = 0.0
	voice.crunch_bits = 8
	voice.volume = 1.0
	return voice
