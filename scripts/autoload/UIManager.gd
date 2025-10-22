extends Node

# Dictionary of themes that can be accessed throughout the game
var themes = {
	"DEFAULT": "res://assets/themes/groot_theme.tres",
	"GROOT": "res://assets/themes/groot_theme.tres",
	"IRONHAVEN": "res://assets/themes/ironhaven_theme.tres",
	"PARCHMENT": "res://assets/themes/parchment_theme.tres",
	# Add more themes as needed
}

# Cached theme instances
var _loaded_themes = {}

func _ready():
	# Preload all themes for faster access
	for theme_key in themes.keys():
		_loaded_themes[theme_key] = load(themes[theme_key])

# Get a theme by its key
func get_theme(theme_key: String):
	if _loaded_themes.has(theme_key):
		return _loaded_themes[theme_key]
	elif themes.has(theme_key):
		# Load on demand if not preloaded
		var theme = load(themes[theme_key])
		_loaded_themes[theme_key] = theme
		return theme
	return null

# Register a new theme
func register_theme(theme_key: String, theme_path: String):
	themes[theme_key] = theme_path
	_loaded_themes[theme_key] = load(theme_path)
