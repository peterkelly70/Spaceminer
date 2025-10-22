extends Node

# Set the log level for this script
var log_level: int = 1

# Notification popup scene - load dynamically to avoid errors if missing
var NotificationScene = null

func _load_notification_scene():
	# Prefer ironhaven-style popup if present
	if ResourceLoader.exists("res://scenes/ui/NotificationPopupIron.tscn"):
		NotificationScene = load("res://scenes/ui/NotificationPopupIron.tscn")
		return true
	# Fallback to existing Groot popup
	if ResourceLoader.exists("res://scenes/ui/NotificationPopup.tscn"):
		NotificationScene = load("res://scenes/ui/NotificationPopup.tscn")
		return true
	Logger.warn(self, "[NotificationManager] NotificationPopup scene not found (checked NotificationPopupIron.tscn, NotificationPopup.tscn)!")
	return false

# UI Canvas layer for notifications
var notification_layer: CanvasLayer

# Notification properties
var active_notifications = []
var notification_spacing = 10  # Space between notifications (vertical gap)
var max_notifications = 5     # Maximum number of notifications visible at once
var notification_margin = 40  # Margin from the edge of the screen
var top_screen_gap = 60      # Gap to maintain at the top of the screen

# Track shown notifications to avoid duplicates
var shown_notifications = {}  # Dictionary to track notifications by ID

# Flag to track if we've shown the startup notifications
var _startup_notifications_shown = false

# Version and music attribution data
const GAME_VERSION := "v0.4.4"
var music_attributions := {}

func _ready() -> void:
	Logger.info(self, "_ready called")
	
	# Create a canvas layer for notifications to ensure they appear on top
	notification_layer = CanvasLayer.new()
	notification_layer.layer = 100  # High layer number to be on top
	add_child(notification_layer)
		
	# Register default themes - will be loaded from UIRoot later
	# We'll use fallback themes until UIRoot registers the real ones
	register_theme("ironhaven", null)
	register_theme("parchment", null)
	
	# Initialize default notification types
	initialize_default_notification_types()
	
	# Load music attributions from JSON file
	load_music_attributions()
	
	# Print all available notification types
	_log_notification_types()
	
	# Schedule startup notifications with a longer delay
	await get_tree().create_timer(1.5).timeout
	show_startup_notifications()

# Show the welcome and music notifications with proper timing
func show_startup_notifications() -> void:
	# Prevent multiple calls
	if _startup_notifications_shown:
		return
	
	_startup_notifications_shown = true
	
	# Show welcome notification
	var allow_welcome := true
	if Engine.has_singleton("SettingsManager"):
		var settings = Engine.get_singleton("SettingsManager")
		if settings.has_method("get_setting"):
			allow_welcome = settings.get_setting("gameplay", "show_welcome_notification", true)

	if not allow_welcome:
		Logger.info(self, "Startup welcome notification disabled by settings")
	else:
		show_welcome_notification()
	
	# Wait longer before showing music notification
	await get_tree().create_timer(5.0).timeout
	
	# Show music notification if allowed and available
	var allow_music := true
	if Engine.has_singleton("SettingsManager"):
		var settings = Engine.get_singleton("SettingsManager")
		if settings.has_method("get_setting"):
			allow_music = settings.get_setting("gameplay", "show_music_notifications", true)

	if not allow_music:
		Logger.info(self, "Startup music notification disabled by settings")
		return

	Logger.info(self, "Checking for music to show notification...")
	var audio_manager = get_node_or_null("/root/Audio_Manager")
	if audio_manager:
		Logger.debug(self, "AudioManager found")
		Logger.debug(self, "Current track: %s" % audio_manager.current_track)
		# Note: Music notifications are now handled by main.gd _on_music_track_changed
		# This provides detailed track information (track number, album, artist)
		Logger.info(self, "Skipping automatic music notification - handled by main.gd")
	else:
		Logger.warn(self, "AudioManager not found")

# Generate a unique ID for a notification message
func _generate_notification_id(message: String) -> String:
	# Simple hash function for the message
	return str(message.hash())

# Notification position and animation types (must match the ones in NotificationPopup.gd)
enum Position { TOP_RIGHT, TOP_LEFT, BOTTOM_RIGHT, BOTTOM_LEFT, TOP_CENTER, BOTTOM_CENTER, CENTER }

# Notification types enum
enum NotificationType {
	DEFAULT,   # Default notification type
	MESSAGE,   # General message
	WARNING,   # Warning message
	ERROR,     # Error message
	SUCCESS,   # Success message
	QUEST,     # Quest notification
	ACHIEVEMENT, # Achievement notification
	RESOURCE,  # Resource gained/lost
	MUSIC,     # Music change notification
	TUTORIAL,  # Tutorial tip
	SYSTEM     # System message
}

# Animation styles enum
enum AnimationStyle { FADE, SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN }

# Default notification settings for different types
var notification_settings = {}

# Mapping from NotificationType enum to string type names
var notification_enum_mappings = {}

# Available themes for notifications
var available_themes = {}

# Register a new notification type with settings
func register_notification_type(type_name: String, description: String, pos: int, anim: int, theme_name: String) -> void:
	notification_settings[type_name] = {
		"position": pos,
		"animation": anim,
		"theme": theme_name,
		"description": description
	}
	Logger.info(self, "Registered notification type: %s - %s" % [type_name, description])

# Register a notification type using enum
func register_notification_type_enum(enum_type: int, description: String, pos: int, anim: int, theme_name: String) -> void:
	# Create a string key for this enum type
	var type_key = "enum_" + str(enum_type)
	
	# Register the notification type
	notification_settings[type_key] = {
		"position": pos,
		"animation": anim,
		"theme": theme_name,
		"description": description
	}
	
	# Register the enum mapping
	notification_enum_mappings[enum_type] = type_key
	Logger.info(self, "Registered enum notification type: %s - %s" % [enum_type, description])

# Register a new theme for notifications
func register_theme(theme_name: String, theme_resource) -> void:
	available_themes[theme_name] = theme_resource
	Logger.info(self, "Registered theme: %s (%s)" % [theme_name, ("resource loaded" if theme_resource else "placeholder")])

	# Register notification type mappings
	register_theme_mapping(NotificationType.MUSIC, "DEFAULT")

# Register a theme mapping for a notification type
func register_theme_mapping(notification_type, theme_name: String) -> void:
	# Create a string key for this enum type
	var type_key = "enum_" + str(notification_type)
	
	# Update the theme in the notification settings
	if notification_settings.has(type_key):
		notification_settings[type_key]["theme"] = theme_name
		Logger.info(self, "Updated theme mapping for type %s to %s" % [notification_type, theme_name])
	
# Register a location mapping for a notification type
func register_location_mapping(notification_type, location) -> void:
	# Create a string key for this enum type
	var type_key = "enum_" + str(notification_type)
	
	# Update the position in the notification settings
	if notification_settings.has(type_key):
		notification_settings[type_key]["position"] = location
		Logger.info(self, "Updated location mapping for type %s to %s" % [notification_type, location])

# Get a theme by name
func get_theme(theme_name: String):
	# First check our local themes
	if available_themes.has(theme_name) and available_themes[theme_name] != null:
		return available_themes[theme_name]
	
	# If not found or null, try to get from UIRoot
	var ui_root = get_node_or_null("/root/MainView")
	if ui_root and ui_root.has_method("get_ui_theme"):
		var theme = ui_root.get_ui_theme(theme_name)
		if theme:
			# Cache the theme for future use
			available_themes[theme_name] = theme
			return theme
	
	# Fallback to default theme
	return null

# Initialize default notification types
func initialize_default_notification_types() -> void:
	# Register default notification types with their settings
	register_notification_type_enum(NotificationType.DEFAULT, "Default notifications", Position.TOP_RIGHT, AnimationStyle.FADE, "ironhaven")
	register_notification_type_enum(NotificationType.MESSAGE, "Message notifications", Position.TOP_RIGHT, AnimationStyle.FADE, "ironhaven")
	register_notification_type_enum(NotificationType.WARNING, "Warning notifications", Position.TOP_CENTER, AnimationStyle.SLIDE_UP, "ironhaven")
	register_notification_type_enum(NotificationType.ERROR, "Error notifications", Position.TOP_CENTER, AnimationStyle.SLIDE_UP, "ironhaven")
	register_notification_type_enum(NotificationType.SUCCESS, "Success notifications", Position.BOTTOM_CENTER, AnimationStyle.SLIDE_UP, "ironhaven")
	register_notification_type_enum(NotificationType.QUEST, "Quest notifications", Position.BOTTOM_RIGHT, AnimationStyle.SLIDE_UP, "ironhaven")
	register_notification_type_enum(NotificationType.ACHIEVEMENT, "Achievement notifications", Position.TOP_RIGHT, AnimationStyle.SLIDE_LEFT, "ironhaven")
	register_notification_type_enum(NotificationType.RESOURCE, "Resource notifications", Position.TOP_RIGHT, AnimationStyle.SLIDE_LEFT, "ironhaven")
	register_notification_type_enum(NotificationType.MUSIC, "Music notifications", Position.BOTTOM_LEFT, AnimationStyle.SLIDE_LEFT, "parchment")
	register_notification_type_enum(NotificationType.TUTORIAL, "Tutorial notifications", Position.CENTER, AnimationStyle.FADE, "ironhaven")
	register_notification_type_enum(NotificationType.SYSTEM, "System notifications", Position.BOTTOM_RIGHT, AnimationStyle.SLIDE_RIGHT, "ironhaven")
	
	# Register legacy string-based notification types for backward compatibility
	register_notification_type("default", "System notifications", Position.TOP_RIGHT, AnimationStyle.FADE, "ironhaven")
	register_notification_type("resource", "Resource change notifications", Position.TOP_RIGHT, AnimationStyle.SLIDE_LEFT, "ironhaven")
	register_notification_type("event", "Event notifications", Position.BOTTOM_LEFT, AnimationStyle.SLIDE_UP, "ironhaven")
	register_notification_type("welcome", "Welcome message", Position.TOP_RIGHT, AnimationStyle.SLIDE_DOWN, "ironhaven")
	register_notification_type("music", "Music attribution", Position.BOTTOM_LEFT, AnimationStyle.SLIDE_LEFT, "parchment")
	register_notification_type("quest", "Quest notifications", Position.BOTTOM_RIGHT, AnimationStyle.SLIDE_UP, "ironhaven")
	register_notification_type("achievement", "Achievement notifications", Position.TOP_CENTER, AnimationStyle.SLIDE_DOWN, "ironhaven")
	register_notification_type("error", "Error messages", Position.TOP_CENTER, AnimationStyle.FADE, "ironhaven")

# Get all registered notification types
func get_notification_types() -> Array:
	return notification_settings.keys()

# Get settings for a specific notification type
func get_notification_settings(notification_type: String) -> Dictionary:
	if notification_settings.has(notification_type):
		return notification_settings[notification_type]
	# Return default settings if type not found
	return notification_settings.get("default", {
		"position": Position.TOP_RIGHT,
		"animation": AnimationStyle.FADE,
		"theme": "ironhaven",
		"description": "Default notification"
	})

# Log all notification types at startup
func _log_notification_types() -> void:
	Logger.info(self, "Available notification types:")
	for type in get_notification_types():
		var settings = notification_settings[type]
		var pos_name = Position.keys()[settings.position]
		var anim_name = AnimationStyle.keys()[settings.animation]
		Logger.info(self, "  - %s: %s" % [type, settings.description])
		Logger.info(self, "    Position: %s, Animation: %s, Theme: %s" % [pos_name, anim_name, settings.theme])

# Show a notification with the given message using enum type
func show_notification_by_enum(message: String, notification_type_enum: int = NotificationType.DEFAULT, duration: float = -1.0, force_show: bool = false) -> void:
	# Convert enum type to string type
	var type_name = "default"
	if notification_enum_mappings.has(notification_type_enum):
		type_name = notification_enum_mappings[notification_type_enum]
	else:
		# If no mapping exists, use the DEFAULT type
		type_name = notification_enum_mappings.get(NotificationType.DEFAULT, "default")
	
	# Call the string-based show_notification method
	show_notification(message, duration, force_show, type_name)

# Show a notification with the given message
func show_notification(message: String, duration: float = -1.0, force_show: bool = false, notification_type: String = "default") -> void:
	# Make sure notification scene is loaded
	if NotificationScene == null and not _load_notification_scene():
		Logger.error(self, "Cannot show notification: NotificationScene not loaded")
		return
	# Generate a unique ID for this notification
	var notification_id = _generate_notification_id(message)
	
	# Check if we've already shown this notification
	if shown_notifications.has(notification_id) and not force_show:
		Logger.info(self, "Skipping duplicate notification: %s" % message)
		return
	
	# Mark this notification as shown
	shown_notifications[notification_id] = true
	
	Logger.info(self, "Showing notification: %s" % message)
	
	# Create a new notification instance
	var notif_instance = NotificationScene.instantiate()
	
	Logger.debug(self, "Instantiating notification scene")
	
	# Add it to the notification layer instead of the root
	Logger.debug(self, "Adding notification to scene tree")
	notification_layer.add_child(notif_instance)
	
	# Get settings for this notification type
	var settings = get_notification_settings(notification_type)
	
	Logger.debug(self, "Showing notification with message: %s" % message)
	Logger.debug(self, "Position: %s, Animation: %s, Theme: %s" % [settings.position, settings.animation, settings.theme])
	
	# Initialize the notification using parameter dictionary (compatible API)
	notif_instance.init_notification({
		"message": message,
		"duration": duration,
		"position": settings.position,
		"animation": settings.animation,
		"theme_name": settings.theme,
	})
	
	# Add to active notifications
	active_notifications.append(notif_instance)
	
	# Connect to the notification's tree_exited signal to remove it from active notifications
	notif_instance.tree_exited.connect(func(): _on_notification_removed(notif_instance.get_instance_id()))
	
	# If we have too many notifications, remove the oldest one
	if active_notifications.size() > max_notifications:
		var oldest = active_notifications[0]
		if is_instance_valid(oldest):
			oldest.queue_free()

# Position a new notification based on existing ones and move existing ones down
func _position_notification(notif: Control) -> void:
	# Get screen dimensions
	var screen_size = get_viewport().get_visible_rect().size
	
	# Get the notification's position type
	var position_type = notif.position_type if notif.has_method("get_position_type") else Position.TOP_RIGHT
	
	# Group notifications by position type
	var notifications_by_position = {}
	
	# Collect existing notifications by position type
	for existing in active_notifications:
		if is_instance_valid(existing) and existing != notif:
			var existing_pos_type = existing.position_type if existing.has_method("get_position_type") else Position.TOP_RIGHT
			
			if not notifications_by_position.has(existing_pos_type):
				notifications_by_position[existing_pos_type] = []
			
			notifications_by_position[existing_pos_type].append(existing)
	
	# Initialize position for this notification
	var pos_x = 0
	var pos_y = 0
	
	# Calculate position based on position type
	match position_type:
		Position.TOP_RIGHT:
			pos_x = screen_size.x - notif.size.x - notification_margin
			pos_y = top_screen_gap
		Position.TOP_LEFT:
			pos_x = notification_margin
			pos_y = top_screen_gap
		Position.BOTTOM_RIGHT:
			pos_x = screen_size.x - notif.size.x - notification_margin
			pos_y = screen_size.y - notif.size.y - notification_margin
		Position.BOTTOM_LEFT:
			pos_x = notification_margin
			pos_y = screen_size.y - notif.size.y - notification_margin
		Position.TOP_CENTER:
			pos_x = (screen_size.x - notif.size.x) / 2
			pos_y = top_screen_gap
		Position.BOTTOM_CENTER:
			pos_x = (screen_size.x - notif.size.x) / 2
			pos_y = screen_size.y - notif.size.y - notification_margin
		Position.CENTER:
			pos_x = (screen_size.x - notif.size.x) / 2
			pos_y = (screen_size.y - notif.size.y) / 2
	
	# Set the position of the new notification at the fixed position
	notif.position = Vector2(pos_x, pos_y)
	
	# Move existing notifications of the same type
	if notifications_by_position.has(position_type):
		var same_position_notifications = notifications_by_position[position_type]
		
		if position_type <= Position.TOP_CENTER:  # Top positions
			# For top positions, push existing notifications down
			for existing in same_position_notifications:
				if is_instance_valid(existing):
					var new_y = existing.position.y + notif.size.y + notification_spacing
					
					# Create a tween to animate the movement
					var tween = create_tween()
					tween.tween_property(existing, "position:y", new_y, 0.3).set_ease(Tween.EASE_OUT)
		else:  # Bottom positions
			# For bottom positions, push existing notifications up
			for existing in same_position_notifications:
				if is_instance_valid(existing):
					var new_y = existing.position.y - notif.size.y - notification_spacing
					
					# Create a tween to animate the movement
					var tween = create_tween()
					tween.tween_property(existing, "position:y", new_y, 0.3).set_ease(Tween.EASE_OUT)
	
	# Position already set above

# Handle notification removal
func _on_notification_removed(notification_id: int) -> void:
	# Find and remove the notification with the matching ID
	for i in range(active_notifications.size() - 1, -1, -1):
		var notif = active_notifications[i]
		if not is_instance_valid(notif) or notif.get_instance_id() == notification_id:
			active_notifications.remove_at(i)

# Show a notification for a resource change
func show_resource_notification(resource_name: String, amount: int) -> void:
	var message = ""
	if amount > 0:
		message = resource_name.capitalize() + " increased by " + str(amount)
	else:
		message = resource_name.capitalize() + " decreased by " + str(abs(amount))
	
	# Create a specific ID for this resource notification
	var notification_id = "resource_" + resource_name.to_lower()
	
	# Check if this resource has already had a notification
	if shown_notifications.has(notification_id):
		Logger.info(self, "Skipping duplicate resource notification for: %s" % resource_name)
		return
	
	# Mark this resource as having had a notification
	shown_notifications[notification_id] = true
	
	show_notification(message, -1.0, true, "resource")

# Show a notification for an event
func show_event_notification(event_name: String) -> void:
	# Create a specific ID for this event notification
	var notification_id = "event_" + event_name.to_lower().replace(" ", "_")
	
	# Check if this event has already had a notification
	if shown_notifications.has(notification_id):
		Logger.info(self, "Skipping duplicate event notification for: %s" % event_name)
		return
	
	# Mark this event as having had a notification
	shown_notifications[notification_id] = true
	
	show_notification("Event: " + event_name, -1.0, true, "event")

# Show a welcome notification with version info
func show_welcome_notification() -> void:
	# Create a specific ID for the welcome notification that includes the version
	var notification_id = "welcome_" + GAME_VERSION
	
	# Check if welcome notification has already been shown for this version
	if shown_notifications.has(notification_id):
		Logger.info(self, "Skipping duplicate welcome notification")
		return
	
	# Mark welcome notification as shown for this version
	shown_notifications[notification_id] = true
	
	# Read settings
	var show_welcome := true
	var duration := 10.0
	if Engine.has_singleton("SettingsManager"):
		var settings = Engine.get_singleton("SettingsManager")
		if settings.has_method("get_setting"):
			show_welcome = settings.get_setting("gameplay", "show_welcome_notification", true)
			duration = settings.get_setting("gameplay", "notification_duration", 10.0)
	
	if not show_welcome:
		Logger.info(self, "Welcome notification disabled by settings")
		return
	
	var message = "Welcome to Groot!"
	show_notification(message, duration, true, "welcome")  # Use welcome notification type

# Clear all active notifications
func clear_all_notifications() -> void:
	# [Fail Fast] Log the action
	Logger.info(self, "Clearing all active notifications")
	
	# Create a copy of the array to avoid modification during iteration
	var notifications_to_clear = active_notifications.duplicate()
	
	# Remove each notification
	for notification in notifications_to_clear:
		if is_instance_valid(notification):
			notification.queue_free()
	
	# Clear the active notifications array
	active_notifications.clear()
	
	# Log completion
	Logger.info(self, "All notifications cleared")

# Load music attributions from JSON file
func load_music_attributions() -> void:
	var file_path = "res://assets/licences/music.json"
	var file = FileAccess.open(file_path, FileAccess.READ)
	
	if file:
		var json_text = file.get_as_text()
		file.close()
		
		var json = JSON.new()
		var error = json.parse(json_text)
		
		if error == OK:
			music_attributions = json.get_data()
			Logger.info(self, "Loaded music attributions from JSON")
		else:
			Logger.error(self, "JSON Parse Error: %s at line %s" % [json.get_error_message(), json.get_error_line()])
	else:
		Logger.error(self, "Could not open music attributions file: %s" % file_path)

# Show a notification for the currently playing music track
func show_music_notification(track_path: String) -> void:
	# Respect settings
	var allow_music := true
	var duration := 10.0
	if Engine.has_singleton("SettingsManager"):
		var settings = Engine.get_singleton("SettingsManager")
		if settings.has_method("get_setting"):
			allow_music = settings.get_setting("gameplay", "show_music_notifications", true)
			duration = settings.get_setting("gameplay", "notification_duration", 10.0)
	if not allow_music:
		Logger.info(self, "Music notifications disabled by settings")
		return
	# Extract the filename from the path
	var filename = track_path.get_file()
	
	# Remove the extension to get the track name
	var track_name = filename.get_basename()
	
	# Create a specific ID for this music notification
	var notification_id = "music_" + track_name.to_lower().replace(" ", "_")
	
	# Check if this music track has already had a notification
	if shown_notifications.has(notification_id):
		Logger.info(self, "Skipping duplicate music notification for: %s" % track_name)
		return
	
	# Mark this music track as having had a notification
	shown_notifications[notification_id] = true
	
	# Format the notification message
	var message = ""
	
	# Try to find the track in the music_attributions dictionary
	var track_info = null
	
	# First try with the exact track name
	var track_key = track_name.to_lower().replace(" ", "_")
	if music_attributions.has(track_key):
		track_info = music_attributions[track_key]
	else:
		# Try some variations of the filename
		var variations = [
			track_name,  # As is
			track_name.to_lower(),  # Lowercase
			track_name.to_lower().replace("_", " "),  # Lowercase with spaces
			track_name.to_lower().replace(".", "_")  # Replace dots with underscores
		]
		
		# Try each variation
		for variant in variations:
			if music_attributions.has(variant):
				track_info = music_attributions[variant]
				break
		
		# If still not found, try to match by title
		if track_info == null:
			for key in music_attributions.keys():
				if music_attributions[key].has("title") and \
				   (music_attributions[key]["title"].to_lower() == track_name.to_lower() or \
					music_attributions[key]["title"].to_lower() == track_name.to_lower().replace("_", " ")):
					track_info = music_attributions[key]
					break
	
	# Format the message based on available information
	if track_info != null and track_info.has("title") and track_info.has("artist") and track_info.has("license"):
		# Full attribution with title, artist, and license
		message = "Now Playing: \"" + track_info["title"] + "\" by " + track_info["artist"] + "\n" + track_info["license"]
		
		# Add license URL if available
		if track_info.has("license_url"):
			message += " - " + track_info["license_url"]
	else:
		# Fallback if no attribution data is found
		message = "Now Playing: " + track_name.replace("_", " ").capitalize()
		Logger.warn(self, "No attribution data found for track: %s (from %s)" % [track_name, filename])
	
	# Show the notification with the MUSIC type using enum
	show_notification_by_enum(message, NotificationType.MUSIC, 10.0, true)

# Test function to demonstrate the notification system
func test_notifications() -> void:
	# Schedule a series of test notifications
	await get_tree().create_timer(0.5).timeout
	show_notification("Welcome to Groot!")
	
	await get_tree().create_timer(1.0).timeout
	show_resource_notification("gold", 10)
	
	await get_tree().create_timer(1.0).timeout
	show_resource_notification("food", -5)
	
	await get_tree().create_timer(1.0).timeout
	show_event_notification("Dragon Sighting")
	
	await get_tree().create_timer(1.0).timeout
	show_notification("Multiple notifications can appear at once")
	
	Logger.info(self, "Test notifications complete")
