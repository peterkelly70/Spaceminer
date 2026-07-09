extends Area2D
class_name HazardZone

signal player_hit

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _on_body_entered(body: Node) -> void:
	if not body.is_in_group("prototype_player") and not body.is_in_group("player"):
		return
	player_hit.emit()
