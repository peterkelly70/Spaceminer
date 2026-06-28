extends Area2D
class_name ConveyorBelt

# A one-way platform that pushes the player horizontally at a constant speed.

@export var direction: int = 1            # 1 = right, -1 = left
@export var speed: float = 120.0          # pixels per second
@export var width: float = 80.0           # platform width
@export var visual_color: Color = Color(0.3, 0.8, 1.0, 0.8)

var _player_on_belt := false
var _player: Node2D = null

@onready var body: Sprite2D = $Body

func _ready() -> void:
	add_to_group("conveyor")
	body_entered.connect(_on_body_entered)
	body_exited.connect(_on_body_exited)
	if body:
		body.modulate = visual_color

func _physics_process(_delta: float) -> void:
	if _player_on_belt and is_instance_valid(_player):
		_player.velocity.x = direction * speed

func _on_body_entered(b: Node) -> void:
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		_player_on_belt = true
		_player = b as Node2D

func _on_body_exited(b: Node) -> void:
	if b.is_in_group("player") or b.is_in_group("prototype_player"):
		_player_on_belt = false
		_player = null
