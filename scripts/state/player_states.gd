extends Resource
class_name PlayerStates

## Lightweight state chart for the player so gameplay code can evolve separately
## from the application state machine.
enum State {
	IDLE,
	RUNNING,
	JUMPING,
	CLINGING,
	STUNNED,
	DEFEATED
}

static func is_airborne(state: State) -> bool:
	return state in [State.JUMPING, State.CLINGING]

static func is_mobile(state: State) -> bool:
	return state in [State.RUNNING, State.JUMPING, State.CLINGING]
