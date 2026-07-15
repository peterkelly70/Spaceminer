extends Control

class_name StateAwareController
var log_level = 1

signal request_state_change(new_state: int)

@export var show_in_states: Array[int] = []

var _last_state: int = -1

func _ready() -> void:
	await get_tree().process_frame # Ensure all singletons are ready
	print("Registering: %s" % name)

	State_Manager.register_state_aware_view(self)

	print("Registering as emitter: %s" % name)
	State_Manager.register_state_emitter(self)

	receive_state_ping(State_Manager.get_current_state())

func receive_state_ping(state: int) -> void:
	print("Received state ping: %s" % state)

	if _last_state == state:
		return

	var should_be_visible := state in show_in_states

	if should_be_visible:
		print("-> on_enter_state")
		on_enter_state(state)
	elif not should_be_visible and visible:
		print("-> on_exit_state")
		on_exit_state(state)

	visible = should_be_visible
	_last_state = state

func request_state(state: int) -> void:
	emit_signal("request_state_change", state)

func on_enter_state(_state: int) -> void:
	# Optional override in subclass
	print("Becoming Visible")
	visible = true

func on_exit_state(_state: int) -> void:
	# Optional override in subclass
	print("Becoming Invisible")
	visible = false
