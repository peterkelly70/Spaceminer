# RetroSpeech — SAM Runtime Speech Bridge

Runtime 1980s-style speech synthesis for Godot 4.6. `RetroSpeech` is a thin
compatibility layer over the upstream SAM source code bundled in
`tools/sam/`, so the project follows the real table-driven SAM implementation
path instead of a clean-room approximation.

---

## Quick Start

```gdscript
# Attach RetroSpeech as a member variable or create it inline
var speech := RetroSpeech.new()

# One-liner: text → audio
speech.speak("BEWARE THE FOREST", $AudioStreamPlayer)

# Or get the AudioStreamWAV and assign it yourself
var wav := speech.render_to_stream("INSERT COIN")
$AudioStreamPlayer.stream = wav
$AudioStreamPlayer.play()

# Use a specific voice profile
var wav := speech.render_to_stream("DANGER", RetroSpeech.VOICE_ROBOT)

# Feed raw phonemes directly (SAM-style notation)
var wav := speech.render_phonemes_to_stream("D EY N JH ER /")
```

---

## Voice Profiles

Three built-in voice constants on `RetroSpeech`:

| Constant              | Character                                      |
|-----------------------|------------------------------------------------|
| `VOICE_OLD_COMPUTER`  | 8 kHz, heavy crunch, flat — classic C64 feel  |
| `VOICE_ROBOT`         | Low pitch, slow, hard resonances               |
| `VOICE_BUG`           | High pitch, fast, bright, clipped              |

All are instances of `SamVoice`. You can tweak any field:

```gdscript
var v := RetroSpeech.VOICE_ROBOT
v.pitch = 100      # higher number = lower pitch
v.speed = 0.5      # slower
v.mouth = 200      # brighter vowels
v.throat = 80      # thinner resonance
v.crunch = 0.9     # more 8-bit crunch
v.crunch_bits = 4  # 4-bit quantisation
```

---

## SamVoice Parameters

The bridge maps `pitch`, `speed`, `mouth`, and `throat` directly into the SAM
binary call path. The remaining fields stay on the voice resource for
compatibility with the existing UI and preset code.

| Parameter     | Range       | Effect                                          |
|---------------|-------------|-------------------------------------------------|
| `sample_rate` | Hz          | Output rate. 8000 = crunchiest, 22050 = cleanest|
| `pitch`       | 10–120      | Glottal period in samples. Higher → lower pitch |
| `speed`       | 0.25–4.0    | Frame count multiplier. >1 = faster             |
| `throat`      | 0–255       | F1 (low formant) bias. Lower = thinner          |
| `mouth`       | 0–255       | F2 (high formant) bias. Higher = brighter       |
| `crunch`      | 0.0–1.0     | Bit-depth reduction intensity                   |
| `crunch_bits` | 2–8         | Quantisation depth in bits                      |
| `volume`      | 0.0–1.0     | Output level                                    |

---

## Phoneme Input Format

You can bypass the reciter and supply phonemes directly. Use space-separated
symbols. A trailing digit (1–9) on a vowel adds stress.

```
HH EH3 L OW / W ER L D
P L EY3 ER / W AH N / R EH D IY
```

Supported phonemes:

**Vowels:** `IY IH EH AE AA AH AO UH UW ER AW OW OY EY AY`  
**Consonants:** `B D G P T K CH JH DH TH F V S Z SH ZH M N NG L R W Y HH`  
**Clusters:** `NGK KS`  
**Pause:** `/`

---

## Architecture

```
Text / phonemes → RetroSpeech
                      ↓
                SamBridge (resolves voices, builds CLI args)
                      ↓
        tools/sam/sam (upstream SAM binary, built locally)
                      ↓
              WAV output → RetroAudioUtil → AudioStreamWAV
```

### What is SAM vs project glue

- **SAM:** `tools/sam/src/*` is the upstream C implementation and the `sam`
  binary built from it.
- **Project glue:** `RetroSpeech`, `SamBridge`, and `RetroAudioUtil` adapt
  that binary for Godot, preserve the existing `RetroSpeech` API, and turn the
  resulting WAV into an `AudioStreamWAV`.
- **Compatibility fields:** The `SamVoice` resource still carries the project
  voice knobs used by the UI. The bridge maps the core SAM parameters
  (`pitch`, `speed`, `mouth`, `throat`) directly into the binary call path.

### AudioStreamWAV Notes

`AudioStreamWAV.data` must contain **raw PCM bytes only** — no RIFF/WAV
header. Godot handles the header internally. The `FORMAT_8_BIT` mode uses
unsigned bytes where `128 = silence`, `0 = full negative`, `255 = full positive`.

---

## Test Scene

Open `scenes/debug/RetroSpeechTest.tscn` in Godot and run it standalone
(Scene → Run Current Scene). Type any phrase, pick a voice, adjust controls,
press **Speak**. The generated phoneme string is shown below the controls.

---

## Known Pronunciation Issues

- The reciter uses simple left-to-right rules; it handles regular English
  patterns well but will mispronounce irregular words.
- Schwa reduction (unstressed vowels collapsing to AH) is approximate.
- The reciter's "silent E" detection is heuristic and will miss edge cases.
- For critical game phrases, supply phonemes directly via
  `render_phonemes_to_stream()`.

---

## Licensing

The original SAM software is a commercial product by Don't Ask Software (now
Nicola Cimmino / Eric Giguere). This project vendors the upstream SAM source
under `tools/sam/src/` and builds it locally. If you reuse the bundle outside
this repo, review the upstream license and source provenance before shipping.
