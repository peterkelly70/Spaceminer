extends Area2D
class_name Teleporter

# Teleporters warp player to a linked teleporter or destination within the room.

signal teleported

@export var teleporter_id: String = "tp_01"     # unique identifier
@export var target_id: String = "tp_02"          # teleporter to link to
@export var color_tint: Color = Color.CYAN

var _recently_used := false
var _cooldown := 0.5

func _ready() -> void:
	add_to_group("teleporter")
	body_entered.connect(_on_body_entered)

func _process(delta: float) -> void:
	if _recently_used:
		_cooldown -= delta
		if _cooldown <= 0:
			_recently_used = false
			_cooldown = 0.5

func _on_body_entered(body: Node) -> void:
	if _recently_used:
		return
	if not body.is_in_group("player") and not body.is_in_group("prototype_player"):
		return

	# Find target teleporter in the scene
	var target := _find_target_teleporter()
	if not target:
		return

	# Teleport player
	body.global_position = target.global_position + Vector2(0, -20)
	_recently_used = true
	target._recently_used = true
	teleported.emit()

func _find_target_teleporter() -> Teleporter:
	var room := get_parent().get_parent()
	if not room:
		return null

	var teleporters = room.find_children("*", "Teleporter")
	for tp in teleporters:
		if tp.teleporter_id == target_id:
			return tp as Teleporter
	return null
