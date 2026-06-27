# SamParser.gd
# Converts a space-separated phoneme string into an array of allophone frames.
# Each frame is a Dictionary:
#   { "phoneme": int, "name": String, "length": int, "stress": int }
#
# Phoneme input format: symbols separated by spaces, optional stress digit suffix,
# e.g. "HH EH3 L OW / W ER L D"
# "/" or "." inserts a pause.
class_name SamParser
extends RefCounted


# ---------------------------------------------------------------------------
# Phoneme registry
# index → { name, voiced, is_vowel, f1, f2, base_length }
# F1/F2 are rough formant centre frequencies (Hz).  They are used by
# SamRenderer to drive the simple bandpass resonators.
#
# Sources / derivation:
#   • SAM phoneme list (public-domain documentation from multiple retro-
#     computing sites and Don't Ask Software original docs)
#   • Formant values approximate Peterson & Barney (1952) averages
#     scaled to sound "robotic" at 8 kHz.
#   • Lengths are SAM-inspired frame counts (1 frame ≈ 1/22 kHz block).
# ---------------------------------------------------------------------------
const PHONEME_TABLE: Array = [
	# idx  name   voiced  vowel  f1   f2   len
	{ "name": "*",   "voiced": false, "vowel": false, "f1":   0, "f2":    0, "len":  0 }, #  0 silence
	{ "name": "IY",  "voiced": true,  "vowel": true,  "f1": 270, "f2": 2290, "len": 8  }, #  1
	{ "name": "IH",  "voiced": true,  "vowel": true,  "f1": 390, "f2": 1990, "len": 8  }, #  2
	{ "name": "EH",  "voiced": true,  "vowel": true,  "f1": 530, "f2": 1840, "len": 8  }, #  3
	{ "name": "AE",  "voiced": true,  "vowel": true,  "f1": 660, "f2": 1720, "len": 8  }, #  4
	{ "name": "AA",  "voiced": true,  "vowel": true,  "f1": 730, "f2": 1090, "len": 8  }, #  5
	{ "name": "AH",  "voiced": true,  "vowel": true,  "f1": 520, "f2": 1190, "len": 7  }, #  6
	{ "name": "AO",  "voiced": true,  "vowel": true,  "f1": 570, "f2":  840, "len": 8  }, #  7
	{ "name": "UH",  "voiced": true,  "vowel": true,  "f1": 440, "f2": 1020, "len": 7  }, #  8
	{ "name": "UW",  "voiced": true,  "vowel": true,  "f1": 300, "f2":  870, "len": 8  }, #  9
	{ "name": "ER",  "voiced": true,  "vowel": true,  "f1": 490, "f2": 1350, "len": 9  }, # 10
	{ "name": "AW",  "voiced": true,  "vowel": true,  "f1": 730, "f2": 1090, "len": 9  }, # 11
	{ "name": "OW",  "voiced": true,  "vowel": true,  "f1": 570, "f2":  840, "len": 9  }, # 12
	{ "name": "OY",  "voiced": true,  "vowel": true,  "f1": 570, "f2": 1840, "len": 10 }, # 13
	{ "name": "EY",  "voiced": true,  "vowel": true,  "f1": 530, "f2": 2290, "len": 9  }, # 14
	{ "name": "AY",  "voiced": true,  "vowel": true,  "f1": 660, "f2": 2290, "len": 9  }, # 15
	{ "name": "B",   "voiced": true,  "vowel": false, "f1":   0, "f2":    0, "len": 4  }, # 16
	{ "name": "D",   "voiced": true,  "vowel": false, "f1":   0, "f2":    0, "len": 4  }, # 17
	{ "name": "G",   "voiced": true,  "vowel": false, "f1":   0, "f2":    0, "len": 4  }, # 18
	{ "name": "P",   "voiced": false, "vowel": false, "f1":   0, "f2":    0, "len": 4  }, # 19
	{ "name": "T",   "voiced": false, "vowel": false, "f1":   0, "f2":    0, "len": 3  }, # 20
	{ "name": "K",   "voiced": false, "vowel": false, "f1":   0, "f2":    0, "len": 4  }, # 21
	{ "name": "CH",  "voiced": false, "vowel": false, "f1":   0, "f2": 2500, "len": 6  }, # 22
	{ "name": "JH",  "voiced": true,  "vowel": false, "f1":   0, "f2": 2500, "len": 6  }, # 23
	{ "name": "DH",  "voiced": true,  "vowel": false, "f1": 200, "f2":  900, "len": 5  }, # 24
	{ "name": "TH",  "voiced": false, "vowel": false, "f1":   0, "f2":    0, "len": 5  }, # 25
	{ "name": "F",   "voiced": false, "vowel": false, "f1":   0, "f2": 1000, "len": 6  }, # 26
	{ "name": "V",   "voiced": true,  "vowel": false, "f1": 100, "f2": 1000, "len": 6  }, # 27
	{ "name": "S",   "voiced": false, "vowel": false, "f1":   0, "f2": 3000, "len": 7  }, # 28
	{ "name": "Z",   "voiced": true,  "vowel": false, "f1": 100, "f2": 3000, "len": 7  }, # 29
	{ "name": "SH",  "voiced": false, "vowel": false, "f1":   0, "f2": 2300, "len": 7  }, # 30
	{ "name": "ZH",  "voiced": true,  "vowel": false, "f1": 100, "f2": 2300, "len": 7  }, # 31
	{ "name": "M",   "voiced": true,  "vowel": false, "f1": 200, "f2":  900, "len": 7  }, # 32
	{ "name": "N",   "voiced": true,  "vowel": false, "f1": 200, "f2": 1700, "len": 7  }, # 33
	{ "name": "NG",  "voiced": true,  "vowel": false, "f1": 200, "f2": 2300, "len": 5  }, # 34
	{ "name": "L",   "voiced": true,  "vowel": false, "f1": 350, "f2": 1100, "len": 6  }, # 35
	{ "name": "R",   "voiced": true,  "vowel": false, "f1": 490, "f2": 1350, "len": 5  }, # 36
	{ "name": "W",   "voiced": true,  "vowel": false, "f1": 300, "f2":  870, "len": 5  }, # 37
	{ "name": "Y",   "voiced": true,  "vowel": false, "f1": 270, "f2": 2290, "len": 4  }, # 38
	{ "name": "HH",  "voiced": false, "vowel": false, "f1":   0, "f2":    0, "len": 4  }, # 39
	{ "name": "NGK", "voiced": false, "vowel": false, "f1":   0, "f2":    0, "len": 6  }, # 40
	{ "name": "KS",  "voiced": false, "vowel": false, "f1":   0, "f2": 2000, "len": 6  }, # 41
	{ "name": "/",   "voiced": false, "vowel": false, "f1":   0, "f2":    0, "len": 14 }, # 42  pause
]

# Build name → index map at class load time
static var _name_to_index: Dictionary = {}
static var _index_map_built := false

# Pause phoneme index
const PAUSE_INDEX := 42


## Return a Dictionary mapping phoneme name → table index.
static func _get_name_map() -> Dictionary:
	if _index_map_built:
		return _name_to_index
	for i in range(PHONEME_TABLE.size()):
		_name_to_index[PHONEME_TABLE[i]["name"]] = i
	_index_map_built = true
	return _name_to_index


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

## Parse a space-separated phoneme string into an array of frame Dictionaries.
## Each dict: { "phoneme": int, "name": String, "length": int, "stress": int }
## Stress comes from a trailing digit (0-9) on the phoneme token.
func parse(phoneme_string: String) -> Array:
	var map := _get_name_map()
	var frames: Array = []
	var tokens := phoneme_string.strip_edges().split(" ", false)

	for token in tokens:
		if token == "":
			continue
		# Strip trailing stress digit
		var stress := 0
		var name := token.to_upper()
		if name.length() > 0:
			var last_char := name[name.length() - 1]
			if last_char >= "0" and last_char <= "9":
				stress = int(last_char)
				name = name.substr(0, name.length() - 1)

		# Normalise pause symbols
		if name == "." or name == "/" or name == ",":
			name = "/"

		if not map.has(name):
			# Unknown phoneme — skip with a warning
			push_warning("SamParser: unknown phoneme '%s' — skipping" % name)
			continue

		var idx: int = map[name]
		var entry: Dictionary = PHONEME_TABLE[idx]
		# Apply stress length bonus (each stress level adds ~1 frame)
		var base_len: int = entry["len"]
		var stressed_len: int = base_len + int(float(stress) * 0.5)

		frames.append({
			"phoneme": idx,
			"name":    name,
			"length":  stressed_len,
			"stress":  stress,
			"voiced":  entry["voiced"],
			"vowel":   entry["vowel"],
			"f1":      entry["f1"],
			"f2":      entry["f2"],
		})

	return frames


## Return the PHONEME_TABLE entry for a given index, or null.
static func get_phoneme_entry(index: int) -> Variant:
	if index < 0 or index >= PHONEME_TABLE.size():
		return null
	return PHONEME_TABLE[index]


## Return the index of a phoneme by name, or -1 if not found.
static func name_to_index(name: String) -> int:
	var map := _get_name_map()
	if map.has(name.to_upper()):
		return map[name.to_upper()]
	return -1
