@tool
extends Resource
class_name CutsceneFrameResource

@export var title: String = "" ## Title displayed at the top of the frame (optional)
@export var show_title: bool = true ## Whether to show the title in the frame
@export_multiline var text: String = "" ## Main text content displayed in the frame
@export var text_color: Color = Color.WHITE ## Color of the text and title
@export var style: String = "default" ## Visual style for this frame (default, emphasis, etc.)
@export_enum("top", "bottom", "left", "right", "center") var text_anchor: String = "bottom" ## Position of the text container
@export_enum("center", "fill", "top", "bottom", "left", "right") var image_anchor: String = "center" ## Position and scaling of the frame image
@export_file("*.png", "*.jpg", "*.jpeg", "*.webp") var frame_image: String = "" ## Path to a foreground image that appears on top of the background
@export_file("*.mp3", "*.ogg", "*.wav") var voiceover_path: String = "" ## Path to voice audio that plays during this frame (MP3, WAV, or OGG)
@export_enum("auto_text", "auto_voiceover", "manual") var duration_mode: String = "auto_voiceover" ## How to determine frame duration: text length, voiceover length, or manual value
@export var manual_duration: float = 3.0 ## Duration in seconds if manual mode is selected (minimum 0.5)
@export var theme_override: String = "default" ## Override the cutscene's theme for this specific frame
@export var show_frame_box: bool = true ## Whether to show a decorative frame box around the content
