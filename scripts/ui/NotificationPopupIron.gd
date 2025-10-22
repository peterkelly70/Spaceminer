extends Control

# References to child nodes
@onready var label: RichTextLabel = $Panel/MarginContainer/Label
@onready var panel: Panel = $Panel
@onready var shadow: Control = $Shadow

# Animation properties
var fade_in_duration := 0.5
var fade_out_duration := 1.5
var slide_distance := 100

# Timer for controlling notification duration
var duration_timer: Timer

# Position and animation settings
enum Position { TOP_RIGHT, TOP_LEFT, BOTTOM_RIGHT, BOTTOM_LEFT, TOP_CENTER, BOTTOM_CENTER }
enum AnimationStyle { FADE, SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN }

var position_type := Position.BOTTOM_LEFT
var animation_style := AnimationStyle.SLIDE_LEFT
var theme_type := "ironhaven"  # Default theme key, Notification_Manager may map this
var margin := 40

# Load themes (mapped to Groot theme assets)
var groot_theme := load("res://assets/themes/groot_theme.tres")
var parchment_theme := load("res://assets/themes/groot_theme.tres") # fallback to same if parchment not present

func _ready() -> void:
	# Create the duration timer
	duration_timer = Timer.new()
	duration_timer.one_shot = true
	add_child(duration_timer)
	duration_timer.timeout.connect(_on_duration_timer_timeout)
	
	# Hide shadow by default (only used for parchment-like theme)
	shadow.visible = false
	
	# Set initial position
	_set_initial_position()
	
	# Start invisible
	modulate.a = 0
	
	# Input setup
	gui_input.connect(_on_gui_input)
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND

# External API (ironhaven signature)
func show_notification(message: String, duration: float = -1.0, pos_type = null, anim_style = null, theme_name = null) -> void:
	if pos_type != null:
		position_type = pos_type
	if anim_style != null:
		animation_style = anim_style
	if theme_name != null:
		theme_type = theme_name
	
	# Resolve theme via manager if present
	var theme_resource: Resource = null
	if Engine.has_singleton("Notification_Manager"):
		var nm = Engine.get_singleton("Notification_Manager")
		if nm.has_method("get_theme"):
			theme_resource = nm.get_theme(theme_type)
	
	if theme_resource:
		theme = theme_resource
		panel.theme = theme_resource
		label.theme = theme_resource
		shadow.visible = (theme_type == "parchment")
		panel.visible = true
	else:
		# Fallback to Groot theme mapping
		if theme_type == "parchment":
			theme = parchment_theme
			panel.theme = parchment_theme
			label.theme = parchment_theme
			shadow.visible = true
		else:
			theme = groot_theme
			panel.theme = groot_theme
			label.theme = groot_theme
			shadow.visible = false
		panel.visible = true
		panel.visible = true
	
	# Set initial position based on current settings
	_set_initial_position()
	
	# Duration default
	if duration < 0:
		duration = get_notification_duration()
	
	# Set message (BBCode centered bold)
	label.text = "[center][b]" + message + "[/b][/center]"
	
	# Reset opacity
	modulate.a = 0
	
	# Play SFX if available
	if Engine.has_singleton("Audio_Manager"):
		var am = Engine.get_singleton("Audio_Manager")
		if am.has_method("play_sfx"):
			am.play_sfx("notification")
	
	# Animate in
	var show_tween = create_tween()
	show_tween.parallel().tween_property(self, "modulate:a", 1.0, fade_in_duration).set_ease(Tween.EASE_OUT)
	var target_pos = global_position
	match animation_style:
		AnimationStyle.SLIDE_LEFT:
			target_pos.x += slide_distance
			show_tween.parallel().tween_property(self, "global_position:x", target_pos.x, fade_in_duration).set_ease(Tween.EASE_OUT)
		AnimationStyle.SLIDE_RIGHT:
			target_pos.x -= slide_distance
			show_tween.parallel().tween_property(self, "global_position:x", target_pos.x, fade_in_duration).set_ease(Tween.EASE_OUT)
		AnimationStyle.SLIDE_UP:
			target_pos.y -= slide_distance
			show_tween.parallel().tween_property(self, "global_position:y", target_pos.y, fade_in_duration).set_ease(Tween.EASE_OUT)
		AnimationStyle.SLIDE_DOWN:
			target_pos.y += slide_distance
			show_tween.parallel().tween_property(self, "global_position:y", target_pos.y, fade_in_duration).set_ease(Tween.EASE_OUT)
	
	# Start duration timer
	duration_timer.wait_time = duration
	duration_timer.start()

# Adapter for Groot NotificationManager: accept params dict and forward to ironhaven API
func init_notification(params: Dictionary) -> void:
	var msg: String = str(params.get("message", ""))
	var dur: float = float(params.get("duration", -1.0))
	var pos: Variant = params.get("position", null)
	var anim: Variant = params.get("animation", null)
	var theme_name: Variant = params.get("theme_name", null)
	show_notification(msg, dur, pos, anim, theme_name)

# Back-compat wrapper for Groot call style
func show_notification_groot(message: String, notify_duration: float, notify_position: int, notify_animation: int, theme_name: String) -> void:
	show_notification(message, notify_duration, notify_position, notify_animation, theme_name)

# Dismiss with animation
func dismiss_notification() -> void:
	if duration_timer and duration_timer.time_left > 0:
		duration_timer.stop()
	var exit_tween = create_tween()
	exit_tween.parallel().tween_property(self, "modulate:a", 0.0, fade_out_duration).set_ease(Tween.EASE_IN)
	# Slide out opposite direction
	var end_pos = global_position
	match animation_style:
		AnimationStyle.SLIDE_LEFT:
			end_pos.x -= slide_distance
			exit_tween.parallel().tween_property(self, "global_position:x", end_pos.x, fade_out_duration).set_ease(Tween.EASE_IN)
		AnimationStyle.SLIDE_RIGHT:
			end_pos.x += slide_distance
			exit_tween.parallel().tween_property(self, "global_position:x", end_pos.x, fade_out_duration).set_ease(Tween.EASE_IN)
		AnimationStyle.SLIDE_UP:
			end_pos.y += slide_distance
			exit_tween.parallel().tween_property(self, "global_position:y", end_pos.y, fade_out_duration).set_ease(Tween.EASE_IN)
		AnimationStyle.SLIDE_DOWN:
			end_pos.y -= slide_distance
			exit_tween.parallel().tween_property(self, "global_position:y", end_pos.y, fade_out_duration).set_ease(Tween.EASE_IN)
	exit_tween.tween_callback(queue_free)

# Timers / Input
func _on_duration_timer_timeout() -> void:
	dismiss_notification()

func _on_gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		var local_pos = get_global_transform().affine_inverse() * event.position
		if get_rect().has_point(local_pos):
			dismiss_notification()

# Utilities
func _set_initial_position() -> void:
	var screen_size = get_viewport().get_visible_rect().size
	var pos_x := 0.0
	var pos_y := 0.0
	match position_type:
		Position.TOP_RIGHT:
			pos_x = screen_size.x - size.x - margin
			pos_y = margin
		Position.TOP_LEFT:
			pos_x = margin
			pos_y = margin
		Position.BOTTOM_RIGHT:
			pos_x = screen_size.x - size.x - margin
			pos_y = screen_size.y - size.y - margin
		Position.BOTTOM_LEFT:
			pos_x = margin
			pos_y = screen_size.y - size.y - margin
		Position.TOP_CENTER:
			pos_x = (screen_size.x - size.x) / 2.0
			pos_y = margin
		Position.BOTTOM_CENTER:
			pos_x = (screen_size.x - size.x) / 2.0
			pos_y = screen_size.y - size.y - margin
	global_position = Vector2(pos_x, pos_y)
	match animation_style:
		AnimationStyle.SLIDE_LEFT:
			global_position.x -= slide_distance
		AnimationStyle.SLIDE_RIGHT:
			global_position.x += slide_distance
		AnimationStyle.SLIDE_UP:
			global_position.y += slide_distance
		AnimationStyle.SLIDE_DOWN:
			global_position.y -= slide_distance

func get_notification_duration() -> float:
	# Try multiple singleton names for compatibility
	if Engine.has_singleton("Settings_Manager"):
		var s = Engine.get_singleton("Settings_Manager")
		if s.has_method("get_setting"):
			return s.get_setting("gameplay", "notification_duration", 30.0)
	if Engine.has_singleton("SettingsManager"):
		var s2 = Engine.get_singleton("SettingsManager")
		if s2.has_method("get_setting"):
			return s2.get_setting("gameplay", "notification_duration", 30.0)
	return 30.0
