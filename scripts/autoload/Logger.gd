extends Node

enum LogLevel { DEBUG, INFO, WARN, ERROR, NONE }

# Global default log level, can be overridden per script
var default_log_level: LogLevel = LogLevel.INFO

# Per-script log levels, keyed by script resource path
var script_log_levels: Dictionary = {}

var input_debugging_enabled: bool = false

# --- Public API ---

func info(source: Object, message: String) -> void:
	_log(source, LogLevel.INFO, message)

func warn(source: Object, message: String) -> void:
	_log(source, LogLevel.WARN, message)

func error(source: Object, message: String) -> void:
	_log(source, LogLevel.ERROR, message)

func debug(source: Object, message: String) -> void:
	_log(source, LogLevel.DEBUG, message)

func set_level_for_script(script_path: String, level: LogLevel) -> void:
	script_log_levels[script_path] = level
	info(self, "Set log level for %s to %s" % [script_path, LogLevel.keys()[level]])

func enable_input_debugging(enable: bool) -> void:
	input_debugging_enabled = enable
	info(self, "Input debugging is now %s." % ("enabled" if enable else "disabled"))

# --- Internal Logic ---

func _log(source: Object, level: LogLevel, message: String) -> void:
	var script_path = ""
	var source_log_level = default_log_level

	if source and source.get_script():
		script_path = source.get_script().resource_path
		if script_log_levels.has(script_path):
			source_log_level = script_log_levels[script_path]
		else:
			# Use object's log_level if present; avoid using .has() on Object
			var maybe_level = source.get("log_level")
			if maybe_level != null:
				source_log_level = maybe_level

	if level < source_log_level:
		return

	var prefix = _get_prefix(level)
	var timestamp = _get_timestamp()
	var source_name = script_path.get_file().get_basename() if not script_path.is_empty() else "Global"

	var final_message = "%s %s [%s] %s" % [timestamp, prefix, source_name, message]

	if level == LogLevel.ERROR:
		push_error(final_message)
	elif level == LogLevel.WARN:
		push_warning(final_message)
	else:
		print(final_message)

func _get_prefix(level: LogLevel) -> String:
	match level:
		LogLevel.DEBUG: return "[DEBUG]"
		LogLevel.INFO:  return "[INFO]"
		LogLevel.WARN:  return "[WARN]"
		LogLevel.ERROR: return "[ERROR]"
	return "[LOG]"

func _get_timestamp() -> String:
	var now = Time.get_datetime_dict_from_system()
	return "%02d:%02d:%02d" % [now.hour, now.minute, now.second]

# --- Input Debugging (can be toggled) ---

func _find_topmost_control(node: Node, pos: Vector2) -> Control:
	var children = node.get_children()
	for i in range(children.size() - 1, -1, -1):
		var child = children[i]
		if child is Control and child.visible:
			var found = _find_topmost_control(child, pos)
			if found:
				return found
			if child.get_global_rect().has_point(pos) and child.mouse_filter != Control.MOUSE_FILTER_IGNORE:
				return child
	return null
