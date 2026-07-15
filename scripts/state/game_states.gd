extends Resource
class_name GameStates

## Encapsulates high-level in-game states that are separate from the global AppState.
## These map directly to the design doc's area groups so progression logic can
## reason about them without overloading the menu/application state machine.
enum State {
	LANDING_BAY,
	CONTROL_HUB,
	REFINERY,
	OXYGEN_GARDEN,
	CORE_ACCESS,
	FUSION_CHAMBER
}

static func get_label(state: State) -> String:
	match state:
		State.LANDING_BAY:
			return "Landing Bay Alpha"
		State.CONTROL_HUB:
			return "Control Hub"
		State.REFINERY:
			return "Refinery"
		State.OXYGEN_GARDEN:
			return "Oxygen Garden"
		State.CORE_ACCESS:
			return "Core Access"
		State.FUSION_CHAMBER:
			return "Fusion Chamber"
		_:
			return "Unknown"

static func ordered_states() -> Array[State]:
	return [State.LANDING_BAY, State.CONTROL_HUB, State.REFINERY, State.OXYGEN_GARDEN, State.CORE_ACCESS, State.FUSION_CHAMBER]
