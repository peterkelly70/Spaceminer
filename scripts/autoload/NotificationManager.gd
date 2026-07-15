extends Node

# Kept as a compatibility autoload so older scripts can call Notification_Manager
# without creating any visual overlays during the platformer prototype.

enum Position { TOP_RIGHT, TOP_LEFT, BOTTOM_RIGHT, BOTTOM_LEFT, TOP_CENTER, BOTTOM_CENTER, CENTER }
enum AnimationStyle { FADE, SLIDE_UP, SLIDE_DOWN, SLIDE_LEFT, SLIDE_RIGHT, SCALE, BOUNCE }
enum NotificationType {
	DEFAULT,
	MESSAGE,
	WARNING,
	ERROR,
	SUCCESS,
	QUEST,
	ACHIEVEMENT,
	RESOURCE,
	MUSIC,
	TUTORIAL,
	SYSTEM,
}

func _ready() -> void:
	print("Notification_Manager disabled")

func register_theme(_theme_name: String, _theme_resource = null) -> void:
	pass

func register_theme_mapping(_notification_type: int, _theme_name: String) -> void:
	pass

func register_location_mapping(_notification_type: int, _position: int) -> void:
	pass

func register_notification_type_enum(
	_notification_type: int,
	_description: String,
	_position: int = Position.TOP_RIGHT,
	_animation: int = AnimationStyle.FADE,
	_theme_name: String = ""
) -> void:
	pass

func register_notification_type(
	_type_name: String,
	_description: String,
	_position: int = Position.TOP_RIGHT,
	_animation: int = AnimationStyle.FADE,
	_theme_name: String = ""
) -> void:
	pass

func show_startup_notifications() -> void:
	pass

func show_welcome_notification() -> void:
	pass

func show_music_notification(_track_path: String) -> void:
	pass

func show_notification(
	_message: String,
	_duration: float = -1.0,
	_force_show: bool = false,
	_notification_type: String = "default"
) -> void:
	pass

func show_notification_by_enum(
	_message: String,
	_notification_type_enum: int = NotificationType.DEFAULT,
	_duration: float = -1.0,
	_force_show: bool = false
) -> void:
	pass

func clear_all_notifications() -> void:
	pass
