extends CanvasLayer
class_name HUD

@onready var shield_bar: ProgressBar = %ShieldBar
@onready var score_label: Label = %ScoreLabel

func _ready() -> void:
	visible = true

func set_shield(current: float, maximum: float) -> void:
	shield_bar.max_value = maximum
	shield_bar.value = clampf(current, 0.0, maximum)

func set_score(score: int) -> void:
	score_label.text = "Score: %d" % score
