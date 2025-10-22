extends Node

var log_level: int = 1

# Utility for formatting tooltips with a bold title and description below
func get_tooltip_bbcode(title: String, description: String) -> String:
	# Returns BBCode-formatted string: bold title, newline, then description
	return "[b]%s[/b]\n%s" % [title, description]

## Assigns or appends text to a Label or RichTextLabel, handling BBCode when enabled.
static func assign(node: Control, text: String, append: bool = false) -> void:
	if node == null:
		Logger.warn(null, "assign(): Node is null")
		return

	if node is RichTextLabel:
		var r = node as RichTextLabel
		if r.bbcode_enabled:
			if not append:
				r.clear()
			r.append_text(text)
		else:
			r.text = r.text + text if append else text
	elif node is Label:
		var l = node as Label
		l.text = l.text + text if append else text
	else:
		Logger.warn(null, "assign(): Node '%s' does not support text assignment" % node.name)

## Appends story-style text with optional newlines.
static func append_story(node: Control, text: String, add_newlines: bool = true) -> void:
	if node == null:
		Logger.warn(null, "append_story(): Node is null")
		return

	if node is RichTextLabel:
		var r = node as RichTextLabel
		var current_text = r.text
		var separator = "\n\n" if add_newlines else ""

		if r.bbcode_enabled:
			r.clear()
			if not current_text.is_empty():
				r.append_text(current_text + separator)
			r.append_text(text)
		else:
			var prefix = current_text + separator if not current_text.is_empty() else ""
			r.text = prefix + text
	elif node is Label:
		var l = node as Label
		var current_text = l.text
		var separator = "\n\n" if add_newlines else ""
		var prefix = current_text + separator if not current_text.is_empty() else ""
		l.text = prefix + text
	else:
		Logger.warn(null, "append_story(): Node '%s' does not support text assignment" % node.name)

## Auto-scales font size to fit within node bounds.
static func font_scale(text: String, node: Control, min_size: int = 8, max_size: int = 64) -> void:
	if node == null:
		Logger.warn(null, "font_scale(): Node is null")
		return

	var label := node as Label
	if not label:
		Logger.warn(null, "font_scale(): Node '%s' is not a Label or RichTextLabel" % node.name)
		return

	var theme_font := label.get_theme_font("font")
	if not theme_font:
		Logger.warn(null, "font_scale(): No font found for node '%s'" % node.name)
		return

	var container_size := node.size
	var font_size := max_size

	while font_size >= min_size:
		var size := theme_font.get_string_size(text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size)
		if size.x <= container_size.x and size.y <= container_size.y:
			break
		font_size -= 1

	label.add_theme_font_size_override("font_size", font_size)

## Applies a custom font.
static func apply_font(custom_font: Font, node: Control) -> void:
	if node == null:
		Logger.warn(null, "apply_font(): Node is null")
		return

	if node is Label or node is RichTextLabel:
		node.add_theme_font_override("font", custom_font)
	else:
		Logger.warn(null, "apply_font(): Node '%s' does not support font override" % node.name)

## Clears the text of a Label or RichTextLabel.
static func clear(node: Node) -> void:
	if node == null:
		Logger.warn(null, "clear(): Node is null")
		return

	if node is RichTextLabel:
		node.clear()
	elif node is Label:
		node.text = ""
	else:
		Logger.warn(null, "clear(): Node '%s' does not support text clearing" % node.name)

## Formats a resource display string.
static func format_resource_display(current_amount: int, generation_amount: int) -> String:
	var amount_str = "%03d" % current_amount
	var gen_abs = abs(generation_amount)
	var gen_abs_str = "%03d" % gen_abs
	var gen_sign = "+" if generation_amount >= 0 else "-"
	# Using direct hex codes as GlobalConstants for colors were not found
	var gen_color_hex = "#44CC44" if generation_amount >= 0 else "#CC4444" # Green for positive/zero, Red for negative

	# BBCode: e.g., [b]000[/b]/[color=#44CC44]+001[/color]
	var bbcode_string = "[b]%s[/b]/[color=%s]%s%s[/color]" % [amount_str, gen_color_hex, gen_sign, gen_abs_str]
	return bbcode_string
