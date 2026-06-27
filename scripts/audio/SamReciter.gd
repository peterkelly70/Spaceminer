# SamReciter.gd
# Rule-based English text → phoneme string converter.
# Inspired by SAM's reciter but written from scratch.
# Output uses the phoneme symbols understood by SamParser.
#
# The rules are deliberately imperfect — the goal is recognisable
# retro-style speech, not IPA-accurate synthesis.
class_name SamReciter
extends RefCounted

# ---------------------------------------------------------------------------
# Digraph / trigraph rules applied before single-letter rules.
# Each entry: [pattern_upper, replacement_phonemes]
# Patterns are matched left-to-right; first match wins.
# ---------------------------------------------------------------------------
const MULTI_RULES: Array = [
	# Three-letter clusters
	["TCH", "CH"],
	# Two-letter clusters — consonant digraphs
	["SH",  "SH"],
	["CH",  "CH"],
	["TH",  "DH"],   # default voiced; context will refine later
	["PH",  "F"],
	["WH",  "W"],
	["NG",  "NG"],
	["NK",  "NGK"],
	["GH",  ""],      # silent in most positions (NIGHT → NAYT)
	["CK",  "K"],
	["QU",  "KW"],
	["ZZ",  "Z"],
	["SS",  "S"],
	["LL",  "L"],
	# Two-letter vowel digraphs
	["EA",  "IY"],
	["EE",  "IY"],
	["OO",  "UW"],
	["OA",  "OW"],
	["OU",  "AW"],
	["OW",  "OW"],
	["OI",  "OY"],
	["OY",  "OY"],
	["AY",  "EY"],
	["AI",  "EY"],
	["AU",  "AO"],
	["AW",  "AO"],
	["EW",  "UW"],
	["EI",  "EY"],
	["IE",  "IY"],
	["UE",  "UW"],
	["UI",  "IH"],
]

# ---------------------------------------------------------------------------
# Single-letter vowel rules.
# Applied only when no digraph matched.
# ---------------------------------------------------------------------------
const VOWEL_RULES: Dictionary = {
	"A": "AE",   # default short-a
	"E": "EH",   # short-e
	"I": "IH",   # short-i
	"O": "AO",   # short-o
	"U": "AH",   # short-u (schwa-ish)
	"Y": "IY",   # y as vowel (end of word handled below)
}

# ---------------------------------------------------------------------------
# Single-letter consonant map
# ---------------------------------------------------------------------------
const CONSONANT_MAP: Dictionary = {
	"B": "B",
	"C": "K",   # refined below for CE/CI/CY
	"D": "D",
	"F": "F",
	"G": "G",
	"H": "HH",
	"J": "JH",
	"K": "K",
	"L": "L",
	"M": "M",
	"N": "N",
	"P": "P",
	"Q": "K",
	"R": "R",
	"S": "S",
	"T": "T",
	"V": "V",
	"W": "W",
	"X": "KS",
	"Y": "Y",   # y as consonant at start of word
	"Z": "Z",
}

# Words that get special one-shot treatment before letter rules run.
const WORD_EXCEPTIONS: Dictionary = {
	"THE":   "DH AH",
	"A":     "AH",
	"OF":    "AH V",
	"TO":    "T UW",
	"FOR":   "F AO R",
	"AND":   "AE N D",
	"ONE":   "W AH N",
	"TWO":   "T UW",
	"ARE":   "AA R",
	"IS":    "IH Z",
	"IN":    "IH N",
	"BE":    "B IY",
	"HE":    "HH IY",
	"SHE":   "SH IY",
	"WE":    "W IY",
	"ME":    "M IY",
	"HI":    "HH AY",
	"YOU":   "Y UW",
	"YOUR":  "Y AO R",
	"WHAT":  "W AH T",
	"BEWARE": "B IH W EH R",
	"FOREST": "F AO R IH S T",
	"INSERT": "IH N S ER T",
	"COIN":  "K OY N",
	"PLAYER": "P L EY ER",
	"READY": "R EH D IY",
	"DANGER": "D EY N JH ER",
	"KILLAPEDE": "K IH L AH P IY D",
}


## Convert plain English text to a space-separated phoneme string.
## Returns something like "HH EH L OW / W ER L D"
func text_to_phonemes(text: String) -> String:
	var upper := text.to_upper().strip_edges()
	# Split on whitespace/punctuation into words
	var words := _split_words(upper)
	var phoneme_parts: Array[String] = []

	for word in words:
		if word == "":
			continue
		if word == "/" or word == "." or word == "," or word == "!":
			phoneme_parts.append("/")
			continue
		var ph := _word_to_phonemes(word)
		if ph != "":
			phoneme_parts.append(ph)

	# Insert a short pause between words
	var joined := " / ".join(phoneme_parts)
	# Collapse multiple pauses
	while joined.contains("/ /"):
		joined = joined.replace("/ /", "/")
	return joined.strip_edges()


## Split text into words, treating punctuation as separate tokens.
func _split_words(text: String) -> Array:
	var result: Array = []
	var current := ""
	for ch in text:
		if ch == " " or ch == "\t" or ch == "\n":
			if current != "":
				result.append(current)
				current = ""
		elif ch == "." or ch == "!" or ch == "?" or ch == ",":
			if current != "":
				result.append(current)
				current = ""
			result.append("/")
		elif ch == "-" or ch == "_":
			if current != "":
				result.append(current)
				current = ""
		else:
			current += ch
	if current != "":
		result.append(current)
	return result


## Convert a single UPPERCASE word to a phoneme string.
func _word_to_phonemes(word: String) -> String:
	# Check exception dictionary first
	if WORD_EXCEPTIONS.has(word):
		return WORD_EXCEPTIONS[word]

	var phonemes: Array[String] = []
	var i := 0
	var length := word.length()
	var is_vowel_context := false  # tracks whether previous char was a vowel

	while i < length:
		var remaining := word.substr(i)
		var ch := word[i]
		var matched := false

		# Try three-letter digraph
		if remaining.length() >= 3:
			var tri := remaining.substr(0, 3)
			for rule in MULTI_RULES:
				if rule[0].length() == 3 and tri == rule[0]:
					if rule[1] != "":
						phonemes.append(rule[1])
					i += 3
					matched = true
					break

		# Try two-letter digraph
		if not matched and remaining.length() >= 2:
			var di := remaining.substr(0, 2)
			for rule in MULTI_RULES:
				if rule[0].length() == 2 and di == rule[0]:
					if rule[1] != "":
						phonemes.append(rule[1])
					i += 2
					matched = true
					break

		if matched:
			continue

		# Context-sensitive single-letter rules
		var next_ch := word[i + 1] if i + 1 < length else ""
		var prev_ch := word[i - 1] if i > 0 else ""

		match ch:
			"A":
				# Silent E rule: if next vowel is E at word end, use long-a
				if _is_long_a_context(word, i):
					phonemes.append("EY")
				else:
					phonemes.append("AE")
			"E":
				# Silent E at end of word
				if i == length - 1 and length > 1:
					pass  # silent
				else:
					phonemes.append("EH")
			"I":
				# Long-i context (e.g. LIKE, MINE)
				if _is_long_i_context(word, i):
					phonemes.append("AY")
				else:
					phonemes.append("IH")
			"O":
				if _is_long_o_context(word, i):
					phonemes.append("OW")
				else:
					phonemes.append("AO")
			"U":
				if _is_long_u_context(word, i):
					phonemes.append("UW")
				else:
					phonemes.append("AH")
			"C":
				# Soft C before E, I, Y
				if next_ch == "E" or next_ch == "I" or next_ch == "Y":
					phonemes.append("S")
				else:
					phonemes.append("K")
			"G":
				# Soft G before E, I, Y
				if next_ch == "E" or next_ch == "I" or next_ch == "Y":
					phonemes.append("JH")
				else:
					phonemes.append("G")
			"S":
				# S between vowels or at end after vowel = Z
				if prev_ch != "" and _is_vowel(prev_ch) and (next_ch == "" or _is_vowel(next_ch)):
					phonemes.append("Z")
				else:
					phonemes.append("S")
			"Y":
				# Y as vowel in middle/end, consonant at start
				if i == 0:
					phonemes.append("Y")
				else:
					phonemes.append("IY")
			"R":
				phonemes.append("R")
			_:
				if CONSONANT_MAP.has(ch):
					phonemes.append(CONSONANT_MAP[ch])
				# Unknown characters are skipped

		i += 1

	return " ".join(phonemes)


## True if the 'A' at position i is in a long-a (EY) context.
func _is_long_a_context(word: String, i: int) -> bool:
	# Pattern: a consonant cluster followed by E at end = long-a (BAKE, MAKE)
	var rest := word.substr(i + 1)
	if rest.length() >= 2:
		var last := rest[rest.length() - 1]
		if last == "E":
			# Check there's at least one consonant between A and E
			var between := rest.substr(0, rest.length() - 1)
			for ch in between:
				if not _is_vowel(ch):
					return true
	return false


## True if 'I' at position i is a long-i (AY).
func _is_long_i_context(word: String, i: int) -> bool:
	var rest := word.substr(i + 1)
	if rest.length() >= 2 and rest[rest.length() - 1] == "E":
		return true
	# IGH pattern (NIGHT, LIGHT)
	if rest.begins_with("GH"):
		return true
	return false


## True if 'O' at position i is a long-o (OW).
func _is_long_o_context(word: String, i: int) -> bool:
	var rest := word.substr(i + 1)
	if rest.length() >= 2 and rest[rest.length() - 1] == "E":
		var between := rest.substr(0, rest.length() - 1)
		for ch in between:
			if not _is_vowel(ch):
				return true
	return false


## True if 'U' at position i is a long-u (UW).
func _is_long_u_context(word: String, i: int) -> bool:
	var rest := word.substr(i + 1)
	if rest.length() >= 2 and rest[rest.length() - 1] == "E":
		return true
	return false


## True if ch is an English vowel letter.
func _is_vowel(ch: String) -> bool:
	return ch == "A" or ch == "E" or ch == "I" or ch == "O" or ch == "U" or ch == "Y"
