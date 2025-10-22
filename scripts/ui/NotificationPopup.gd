extends PanelContainer

var log_level: int = 1

# Signal emitted when the notification is closed
signal closed

# Notification position and animation types (must match the ones in NotificationManager.gd)
enum Position { TOP_RIGHT, TOP_LEFT, BOTTOM_RIGHT, BOTTOM_LEFT, TOP_CENTER, BOTTOM_CENTER, CENTER }
enum AnimationStyle { FADE, SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN }

# Notification properties
var notification_id: String = ""
var position_type: int = Position.TOP_RIGHT
var animation_style: int = AnimationStyle.FADE
var duration: float = 5.0
var auto_close: bool = true

# UI elements (resolved at runtime; robust to owner/unique-name context)
var header: HBoxContainer
var icon: TextureRect
var title_label: Label
var message_label: RichTextLabel
var close_timer: Timer

# Animation properties
var tween: Tween
var start_position: Vector2
var target_position: Vector2
var is_closing: bool = false

func _gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		Logger.info(self, "Notification clicked; closing")
		close()

# Debug: dump subtree for diagnostics
func _dump_subtree(node: Node, indent: String = "") -> void:
	Logger.debug(self, "%s- %s [%s]" % [indent, node.name, node.get_class()])
	for child in node.get_children():
		_dump_subtree(child, indent + "  ")

func _ready() -> void:
	# Set initial state
	modulate.a = 0
	mouse_filter = Control.MOUSE_FILTER_STOP
	
	# Default size (smaller, top-right toast)
	custom_minimum_size = Vector2(300, 120)

	# Resolve nodes by name, recursively, to avoid owner/unique-name pitfalls
	var _h = find_child("Header", true, false)
	if _h is HBoxContainer:
		header = _h
	else:
		header = null

	var _i = find_child("Icon", true, false)
	icon = _i as TextureRect

	var _tl = find_child("TitleLabel", true, false)
	title_label = _tl as Label

	message_label = find_child("MessageLabel", true, false) as RichTextLabel
	close_timer = find_child("CloseTimer", true, false) as Timer

	# If not found by name, try robust matching by type and structure
	if header == null:
		for candidate in get_children():
			if candidate is HBoxContainer:
				header = candidate
				break
		if header == null:
			var candidates = get_tree().get_nodes_in_group("") # no-op to keep style
			for n in get_children():
				if n is Container:
					var hb = n.find_child("", true, false)
					if hb is HBoxContainer:
						header = hb
						break
	if icon == null:
		icon = find_child("", true, false) as TextureRect
		if icon == null:
			# Walk under header if exists
			if header:
				for c in header.get_children():
					if c is TextureRect:
						icon = c
						break
	if title_label == null:
		if header:
			for c in header.get_children():
				if c is Label:
					title_label = c
					break
		if title_label == null:
			title_label = find_child("", true, false) as Label
	if message_label == null:
		message_label = find_child("", true, false) as RichTextLabel
	if close_timer == null:
		close_timer = find_child("", true, false) as Timer

	# Ensure child controls don't swallow clicks so root handler receives them
	for control_child in get_tree().get_nodes_in_group(""):
		pass # no-op placeholder to preserve style
	for child in get_children():
		if child is Control:
			child.mouse_filter = Control.MOUSE_FILTER_IGNORE
			_child_bind_close_on_click(child)

	# If any still missing, dump subtree and build minimal structure as fallback
	if header == null or icon == null or message_label == null or close_timer == null:
		Logger.warn(self, "Expected children not found. Dumping subtree and building minimal fallback UI.")
		_dump_subtree(self)

		# Build minimal UI if empty or mismatched
		if header == null:
			# Root: PanelContainer (self)
			var margin := MarginContainer.new()
			margin.name = "MarginContainer"
			add_child(margin)

			var vbox := VBoxContainer.new()
			vbox.name = "VBoxContainer"
			margin.add_child(vbox)

			header = HBoxContainer.new()
			header.name = "Header"
			vbox.add_child(header)

			icon = TextureRect.new()
			icon.name = "Icon"
			icon.custom_minimum_size = Vector2(32, 32)
			icon.expand_mode = TextureRect.EXPAND_FIT_WIDTH
			icon.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
			header.add_child(icon)

			title_label = Label.new()
			title_label.name = "TitleLabel"
			title_label.text = "Notification"
			header.add_child(title_label)

			var sep := HSeparator.new()
			vbox.add_child(sep)

			message_label = RichTextLabel.new()
			message_label.name = "MessageLabel"
			message_label.bbcode_enabled = true
			message_label.fit_content = true
			message_label.scroll_active = false
			message_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
			vbox.add_child(message_label)

		if close_timer == null:
			close_timer = Timer.new()
			close_timer.name = "CloseTimer"
			close_timer.one_shot = true
			add_child(close_timer)
			close_timer.timeout.connect(_on_close_timer_timeout)

		# Re-resolve to ensure references are valid
		if header == null:
			header = find_child("Header", true, false)
		if icon == null:
			icon = find_child("Icon", true, false) as TextureRect
		if title_label == null:
			title_label = find_child("TitleLabel", true, false) as Label
		if message_label == null:
			message_label = find_child("MessageLabel", true, false) as RichTextLabel

	# Final fail-fast checks
	assert(header != null, "NotificationPopup: Header not available after fallback build")
	assert(icon != null, "NotificationPopup: Icon not available after fallback build")
	assert(message_label != null, "NotificationPopup: MessageLabel not available after fallback build")
	assert(close_timer != null, "NotificationPopup: CloseTimer not available after fallback build")
	
	# Set default theme if not set
	if not theme:
		theme = Theme.new()
		var stylebox = StyleBoxFlat.new()
		stylebox.bg_color = Color(0.1, 0.1, 0.1, 0.9)
		stylebox.border_width_bottom = 2
		stylebox.border_width_left = 2
		stylebox.border_width_right = 2
		stylebox.border_width_top = 2
		stylebox.border_color = Color(0.8, 0.6, 0.2, 1)
		theme.set_stylebox("panel", "PanelContainer", stylebox)
	
	# Update icon if provided in theme
	var notification_icon = null
	if has_theme_icon("Notification", "EditorIcons"):
		notification_icon = get_theme_icon("Notification", "EditorIcons")
	
	icon.texture = notification_icon if notification_icon else preload("res://assets/icons/icon.png")
	
	# Initialize with default values
	position_type = Position.TOP_RIGHT
	animation_style = AnimationStyle.FADE
	
	# Make sure we're visible
	visible = true

func _child_bind_close_on_click(node: Node) -> void:
	if node is Control:
		node.mouse_filter = Control.MOUSE_FILTER_IGNORE
		node.gui_input.connect(_on_child_gui_input)
	for grandchild in node.get_children():
		_child_bind_close_on_click(grandchild)

func _on_child_gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		close()

# Initialize the notification with parameters
func init_notification(params: Dictionary) -> void:
	# Set notification properties
	if params.has("id"):
		notification_id = params.id
	
	# Set title and show/hide header based on title presence
	if params.has("title") and params.title:
		title_label.text = params.title
	else:
		header.hide()
		
	# Set icon if provided
	if params.has("icon") and params.icon is Texture2D:
		icon.texture = params.icon
		icon.show()
	else:
		icon.hide()
		
	# Adjust minimum size based on content
	await get_tree().process_frame
	var content_height = message_label.size.y + 60  # Padding + header
	if header.visible:
		content_height += header.size.y
	custom_minimum_size.y = max(120, content_height)
	
	if params.has("message"):
		message_label.text = params.message
	else:
		message_label.visible = false
	
	if params.has("position"):
		position_type = params.position
	
	if params.has("animation"):
		animation_style = params.animation
	
	if params.has("duration"):
		duration = params.duration
		if duration <= 0:
			auto_close = false
	
	if params.has("auto_close"):
		auto_close = params.auto_close
	
	# Set up timer if auto-close is enabled
	if auto_close and duration > 0:
		close_timer.wait_time = duration
		close_timer.start()
	
	# Set up initial position and animate in
	setup_position()
	animate_in()


# Backwards-compatible API expected by NotificationManager
func show_notification(message: String, notify_duration: float, notify_position: int, notify_animation: int, theme_name: String) -> void:
	var params := {
		"message": message,
		"duration": notify_duration,
		"position": notify_position,
		"animation": notify_animation,
		"theme_name": theme_name,
	}
	init_notification(params)

# Apply a theme to the notification
func apply_theme(theme_name: String) -> void:
	var notification_manager = get_node_or_null("/root/Notification_Manager")
	if notification_manager:
		var theme_res = notification_manager.get_theme(theme_name)
		if theme_res:
			add_theme_stylebox_override("panel", theme_res.get_stylebox("notification_panel", "PanelContainer"))
			title_label.add_theme_color_override("font_color", theme_res.get_color("notification_title", "Label"))
			message_label.add_theme_color_override("default_color", theme_res.get_color("notification_text", "RichTextLabel"))

# Set up the notification's position and animation properties
func setup_position() -> void:
	# Get the viewport size
	var viewport_size = get_viewport_rect().size
	
	# Calculate the notification position based on its type
	match position_type:
		Position.TOP_RIGHT:
			start_position = Vector2(viewport_size.x, 20)
			target_position = Vector2(viewport_size.x - size.x - 20, 20)
		Position.TOP_LEFT:
			start_position = Vector2(-size.x, 20)
			target_position = Vector2(20, 20)
		Position.BOTTOM_RIGHT:
			start_position = Vector2(viewport_size.x, viewport_size.y - size.y - 20)
			target_position = Vector2(viewport_size.x - size.x - 20, viewport_size.y - size.y - 20)
		Position.BOTTOM_LEFT:
			start_position = Vector2(-size.x, viewport_size.y - size.y - 20)
			target_position = Vector2(20, viewport_size.y - size.y - 20)
		Position.TOP_CENTER:
			start_position = Vector2((viewport_size.x - size.x) / 2, -size.y)
			target_position = Vector2((viewport_size.x - size.x) / 2, 20)
		Position.BOTTOM_CENTER:
			start_position = Vector2((viewport_size.x - size.x) / 2, viewport_size.y)
			target_position = Vector2((viewport_size.x - size.x) / 2, viewport_size.y - size.y - 20)
		Position.CENTER:
			start_position = Vector2((viewport_size.x - size.x) / 2, viewport_size.y)
			target_position = Vector2((viewport_size.x - size.x) / 2, (viewport_size.y - size.y) / 2)
	
	# Set the initial position
	position = start_position
	position = start_position

# Animate the notification in
func animate_in() -> void:
	# Cancel any existing animation
	if tween:
		tween.kill()
	
	# Create a new tween
	tween = create_tween()
	tween.set_ease(Tween.EASE_OUT)
	tween.set_trans(Tween.TRANS_BACK)
	
	# Animate based on the style
	match animation_style:
		AnimationStyle.FADE:
			position = target_position
			tween.tween_property(self, "modulate:a", 1.0, 0.5)
		AnimationStyle.SLIDE_LEFT, AnimationStyle.SLIDE_RIGHT:
			tween.tween_property(self, "position", target_position, 0.5)
			tween.parallel().tween_property(self, "modulate:a", 1.0, 0.5)
		AnimationStyle.SLIDE_UP, AnimationStyle.SLIDE_DOWN:
			tween.tween_property(self, "position", target_position, 0.5)
			tween.parallel().tween_property(self, "modulate:a", 1.0, 0.5)

# Animate the notification out
func animate_out() -> void:
	if is_closing:
		return
		
	is_closing = true
	
	# Cancel any existing animation
	if tween:
		tween.kill()
	
	# Create a new tween
	tween = create_tween()
	tween.set_ease(Tween.EASE_IN)
	tween.set_trans(Tween.TRANS_BACK)
	
	# Animate based on the style
	match animation_style:
		AnimationStyle.FADE:
			tween.tween_property(self, "modulate:a", 0.0, 0.3)
		AnimationStyle.SLIDE_LEFT:
			tween.tween_property(self, "position", Vector2(-size.x, position.y), 0.3)
			tween.parallel().tween_property(self, "modulate:a", 0.0, 0.3)
		AnimationStyle.SLIDE_RIGHT:
			tween.tween_property(self, "position", Vector2(get_viewport_rect().size.x + size.x, position.y), 0.3)
			tween.parallel().tween_property(self, "modulate:a", 0.0, 0.3)
		AnimationStyle.SLIDE_UP:
			tween.tween_property(self, "position", Vector2(position.x, -size.y), 0.3)
			tween.parallel().tween_property(self, "modulate:a", 0.0, 0.3)
		AnimationStyle.SLIDE_DOWN:
			tween.tween_property(self, "position", Vector2(position.x, get_viewport_rect().size.y + size.y), 0.3)
			tween.parallel().tween_property(self, "modulate:a", 0.0, 0.3)
	
	# Queue free after animation
	tween.tween_callback(func(): 
		emit_signal("closed")
		queue_free()
	)

# Close the notification
func close() -> void:
	animate_out()

# Timer timeout handler
func _on_close_timer_timeout() -> void:
	close()
