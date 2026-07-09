extends Resource
class_name EnemyStates

## Shared enemy state buckets for simple AI behaviors.
enum State {
	IDLE,
	PATROL,
	CHASE,
	ATTACK,
	STUNNED,
	DISABLED
}

static func is_hostile(state: State) -> bool:
	return state in [State.CHASE, State.ATTACK]
