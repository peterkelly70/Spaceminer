extends Node

# ── Action catalogue ──────────────────────────────────────────────────────────
const GAME_ACTIONS := [
	"move_left", "move_right", "move_up", "move_down", "jump", "thrust",
	"grapple", "laser_fire", "boots_toggle",
	"interact",
	"equipment_1", "equipment_2", "equipment_3",
	"equipment_4", "equipment_5", "equipment_6",
	"quest_slot_1", "quest_slot_2", "quest_slot_3", "quest_slot_4",
	"map_toggle", "pause_game",
]

# Friendly labels shown in the remapper UI
const ACTION_LABELS := {
	"move_left":    "Move Left",
	"move_right":   "Move Right",
	"move_up":      "Climb Up",
	"move_down":    "Climb Down",
	"jump":         "Jump / Jetpack",
	"thrust":       "Jetpack Thrust",
	"grapple":      "Grappling Hook (Hold F)",
	"laser_fire":   "Laser Pistol",
	"boots_toggle": "Magnetic Boots",
	"interact":     "Interact / Refuel (Hold)",
	"equipment_1":  "Equipment Slot 1",
	"equipment_2":  "Equipment Slot 2",
	"equipment_3":  "Equipment Slot 3",
	"equipment_4":  "Equipment Slot 4",
	"equipment_5":  "Equipment Slot 5",
	"equipment_6":  "Equipment Slot 6",
	"quest_slot_1": "Quest Item Slot 1",
	"quest_slot_2": "Quest Item Slot 2",
	"quest_slot_3": "Quest Item Slot 3",
	"quest_slot_4": "Quest Item Slot 4",
	"map_toggle":   "Map Overlay",
	"pause_game":   "Pause",
}

# ── Presets ───────────────────────────────────────────────────────────────────
# KEY_* constants embedded as integers so preset dicts are compile-time safe.
# Joypad buttons appended as InputEventJoypadButton entries after key setup.
const PRESET_RIGHT_HANDED := {
	"move_left":    [KEY_A, KEY_LEFT],
	"move_right":   [KEY_D, KEY_RIGHT],
	"move_up":      [KEY_W, KEY_UP],
	"move_down":    [KEY_S, KEY_DOWN],
	"jump":         [KEY_SPACE],
	"thrust":       [KEY_W, KEY_UP],
	"grapple":      [KEY_F],
	"laser_fire":   [KEY_G],
	"boots_toggle": [KEY_TAB],
	"interact":     [KEY_E],
	"equipment_1":  [KEY_INSERT],
	"equipment_2":  [KEY_HOME],
	"equipment_3":  [KEY_PAGEUP],
	"equipment_4":  [KEY_DELETE],
	"equipment_5":  [KEY_END],
	"equipment_6":  [KEY_PAGEDOWN],
	"quest_slot_1": [KEY_1],
	"quest_slot_2": [KEY_2],
	"quest_slot_3": [KEY_3],
	"quest_slot_4": [KEY_4],
	"map_toggle":   [KEY_M],
	"pause_game":   [KEY_P, KEY_ESCAPE],
}

const PRESET_LEFT_HANDED := {
	"move_left":    [KEY_LEFT, KEY_A],
	"move_right":   [KEY_RIGHT, KEY_D],
	"move_up":      [KEY_UP, KEY_W],
	"move_down":    [KEY_DOWN, KEY_S],
	"jump":         [KEY_SPACE],
	"thrust":       [KEY_W, KEY_UP],
	"grapple":      [KEY_F],
	"laser_fire":   [KEY_T],
	"boots_toggle": [KEY_I],
	"interact":     [KEY_CTRL],
	"equipment_1":  [KEY_INSERT],
	"equipment_2":  [KEY_HOME],
	"equipment_3":  [KEY_PAGEUP],
	"equipment_4":  [KEY_DELETE],
	"equipment_5":  [KEY_END],
	"equipment_6":  [KEY_PAGEDOWN],
	"quest_slot_1": [KEY_1],
	"quest_slot_2": [KEY_2],
	"quest_slot_3": [KEY_3],
	"quest_slot_4": [KEY_4],
	"map_toggle":   [KEY_M],
	"pause_game":   [KEY_P, KEY_ESCAPE],
}

# Joypad mappings (added to every preset — Xbox layout button indices)
const JOYPAD_BINDINGS := {
	"move_left":    [JOY_AXIS_LEFT_X, -1],
	"move_right":   [JOY_AXIS_LEFT_X,  1],
	"move_up":      [JOY_AXIS_LEFT_Y, -1],
	"move_down":    [JOY_AXIS_LEFT_Y,  1],
	"jump":         [JOY_BUTTON_A],
	"thrust":       [JOY_BUTTON_DPAD_UP],
	"grapple":      [JOY_BUTTON_X],
	"laser_fire":   [JOY_BUTTON_RIGHT_SHOULDER],
	"boots_toggle": [JOY_BUTTON_LEFT_SHOULDER],
	"equipment_1":  [JOY_BUTTON_DPAD_LEFT],
	"equipment_2":  [JOY_BUTTON_DPAD_UP],
	"equipment_3":  [JOY_BUTTON_DPAD_RIGHT],
	"equipment_4":  [JOY_BUTTON_Y],
	"equipment_5":  [JOY_BUTTON_B],
	"equipment_6":  [JOY_BUTTON_BACK],
	"map_toggle":   [JOY_BUTTON_START],
	"pause_game":   [JOY_BUTTON_START],
}

# ── Public API ────────────────────────────────────────────────────────────────

signal bindings_changed

var current_preset: String = "right_handed"

func _ready() -> void:
	_ensure_actions_exist()
	var saved := _load_preset_setting()
	apply_preset(saved, false)   # already calls _apply_joypad_bindings internally

# Apply a named preset ("right_handed" or "left_handed").
# Pass save=true to persist the choice.
func apply_preset(preset_name: String, save: bool = true) -> void:
	var preset: Dictionary
	match preset_name:
		"left_handed":
			preset = PRESET_LEFT_HANDED
			current_preset = "left_handed"
		_:
			preset = PRESET_RIGHT_HANDED
			current_preset = "right_handed"

	for action in preset:
		if not InputMap.has_action(action):
			InputMap.add_action(action)
		InputMap.action_erase_events(action)
		for key_code in preset[action]:
			var ev := InputEventKey.new()
			ev.keycode = key_code
			InputMap.action_add_event(action, ev)

	_apply_joypad_bindings()

	if save:
		_save_preset_setting(current_preset)
		_save_custom_bindings({})   # clear any per-action overrides
	bindings_changed.emit()

# Remap a single action to a new InputEvent.
# Preserves the first event; replaces from index 1 onward with the new one.
func remap_action(action: String, new_event: InputEvent) -> void:
	if not InputMap.has_action(action):
		return
	var events := InputMap.action_get_events(action)
	# Keep joypad events, replace keyboard ones
	var kept: Array[InputEvent] = []
	for ev in events:
		if ev is InputEventJoypadButton or ev is InputEventJoypadMotion:
			kept.append(ev)
	kept.append(new_event)
	InputMap.action_erase_events(action)
	for ev in kept:
		InputMap.action_add_event(action, ev)

	_save_action_binding(action, new_event)
	bindings_changed.emit()

# Returns the key label at a specific slot index (0 = primary, 1 = secondary).
func get_key_label(action: String, slot: int = 0) -> String:
	if not InputMap.has_action(action):
		return "—"
	var found := 0
	for ev in InputMap.action_get_events(action):
		if ev is InputEventKey:
			if found == slot:
				return OS.get_keycode_string(ev.keycode)
			found += 1
	return "—"

# Remap a specific keyboard slot (0=primary, 1=secondary) for an action.
func remap_action_slot(action: String, slot: int, new_event: InputEvent) -> void:
	if not InputMap.has_action(action):
		return
	var events := InputMap.action_get_events(action)
	var keys: Array[InputEvent] = []
	var joys: Array[InputEvent] = []
	for ev in events:
		if ev is InputEventKey:
			keys.append(ev)
		elif ev is InputEventJoypadButton or ev is InputEventJoypadMotion:
			joys.append(ev)
	# Resize keys array to fit slot
	while keys.size() <= slot:
		keys.append(null)
	keys[slot] = new_event
	InputMap.action_erase_events(action)
	for ev in keys:
		if ev != null:
			InputMap.action_add_event(action, ev)
	for ev in joys:
		InputMap.action_add_event(action, ev)
	_save_action_binding(action, new_event)
	bindings_changed.emit()

func get_action_display_name(action: String) -> String:
	return ACTION_LABELS.get(action, action)

# ── Private helpers ───────────────────────────────────────────────────────────

func _ensure_actions_exist() -> void:
	for action in GAME_ACTIONS:
		if not InputMap.has_action(action):
			InputMap.add_action(action)

func _apply_joypad_bindings() -> void:
	for action in JOYPAD_BINDINGS:
		if not InputMap.has_action(action):
			continue
		var binding = JOYPAD_BINDINGS[action]
		# Check if it's an axis binding ([axis, sign]) or button binding ([button_index])
		if binding.size() == 2 and binding[1] is int and (binding[1] == -1 or binding[1] == 1):
			var ev := InputEventJoypadMotion.new()
			ev.axis = binding[0]
			ev.axis_value = float(binding[1])
			# Only add if not already present
			var already := false
			for existing in InputMap.action_get_events(action):
				if existing is InputEventJoypadMotion and existing.axis == ev.axis:
					already = true
					break
			if not already:
				InputMap.action_add_event(action, ev)
		else:
			var ev := InputEventJoypadButton.new()
			ev.button_index = binding[0]
			var already := false
			for existing in InputMap.action_get_events(action):
				if existing is InputEventJoypadButton and existing.button_index == ev.button_index:
					already = true
					break
			if not already:
				InputMap.action_add_event(action, ev)

func _load_preset_setting() -> String:
	var sm := get_node_or_null("/root/SettingsManager")
	if sm and sm.has_method("get_setting"):
		var v: String = sm.get_setting("controls", "preset", "right_handed")
		return v if v in ["right_handed", "left_handed"] else "right_handed"
	return "right_handed"

func _save_preset_setting(preset: String) -> void:
	var sm := get_node_or_null("/root/SettingsManager")
	if sm and sm.has_method("set_setting"):
		sm.set_setting("controls", "preset", preset)

func _save_custom_bindings(overrides: Dictionary) -> void:
	var sm := get_node_or_null("/root/SettingsManager")
	if sm and sm.has_method("set_setting"):
		sm.set_setting("controls", "custom_bindings", var_to_str(overrides))

func _save_action_binding(action: String, ev: InputEvent) -> void:
	var sm := get_node_or_null("/root/SettingsManager")
	if not sm or not sm.has_method("get_setting"):
		return
	var raw: String = sm.get_setting("controls", "custom_bindings", "{}")
	var overrides: Dictionary = {}
	var parsed = str_to_var(raw)
	if typeof(parsed) == TYPE_DICTIONARY:
		overrides = parsed
	if ev is InputEventKey:
		overrides[action] = int(ev.keycode)
	sm.set_setting("controls", "custom_bindings", var_to_str(overrides))
