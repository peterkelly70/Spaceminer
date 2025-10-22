extends RefCounted


# Basic log level enum for compatibility with existing code.
enum LogLevel { DEBUG, INFO, WARN, ERROR }
static var level: int = LogLevel.DEBUG

static func info(_who, msg: String) -> void:
	print("[INFO] %s" % msg)

static func warn(_who, msg: String) -> void:
	push_warning("[WARN] %s" % msg)

static func error(_who, msg: String) -> void:
	push_error("[ERROR] %s" % msg)

static func debug(_who, msg: String) -> void:
	print("[DEBUG] %s" % msg)
