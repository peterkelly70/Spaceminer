@tool
extends Node2D
@class_name Puppet2D

## A simple segmented puppet: parts parented to Bone2D under a Skeleton2D.
## Naming convention maps files to bones: head, torso, left_arm, right_arm, left_leg, right_leg, weapon, cloak.

@export var pixel_snap := true
var skeleton: Skeleton2D
var anim_player: AnimationPlayer

func _ready() -> void:
	skeleton = Skeleton2D.new()
	add_child(skeleton)
	anim_player = AnimationPlayer.new()
	add_child(anim_player)
	if pixel_snap:
		# Prefer crisp rendering by disabling filtering on this CanvasItem
		texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST

func add_part(name: String, tex: Texture2D, pivot: Vector2 = Vector2.ZERO) -> Node2D:
	var bone := Bone2D.new()
	bone.name = name + "_bone"
	skeleton.add_child(bone)
	var part := load("res://addons/puppetlab/nodes/PuppetPart2D.gd").new()
	part.name = name
	part.texture = tex
	part.centered = true
	part.offset = -pivot
	bone.add_child(part)
	return part

func get_bone(name: String) -> Bone2D:
	for c in skeleton.get_children():
		if c is Bone2D and c.name == name + "_bone":
			return c
	return null

func create_basic_bones() -> void:
	var order = ["torso","head","left_arm","right_arm","left_leg","right_leg","weapon","cloak"]
	for n in order:
		var b := Bone2D.new()
		b.name = n+"_bone"
		skeleton.add_child(b)
	# Simple parenting so limbs follow torso
	get_bone("head").owner = self
	get_bone("torso").owner = self
	get_bone("left_arm").set_owner(self)
	get_bone("right_arm").set_owner(self)
	get_bone("left_leg").set_owner(self)
	get_bone("right_leg").set_owner(self)
	get_bone("weapon").set_owner(self)
	get_bone("cloak").set_owner(self)
	get_bone("head").reparent(get_bone("torso"))
	get_bone("left_arm").reparent(get_bone("torso"))
	get_bone("right_arm").reparent(get_bone("torso"))
	get_bone("cloak").reparent(get_bone("torso"))
	get_bone("left_leg").reparent(get_bone("torso"))
	get_bone("right_leg").reparent(get_bone("torso"))
	get_bone("weapon").reparent(get_bone("right_arm"))

func make_idle_animation() -> void:
	var anim := Animation.new()
	anim.length = 1.0
	anim.loop = true
	var tracks := {
		"torso": 2.0,
		"head": 2.0,
		"left_arm": 3.0,
		"right_arm": 3.0,
		"cloak": 4.0
	}
	for bone_name in tracks.keys():
		var bone := get_bone(bone_name)
		var p := anim.add_track(Animation.TYPE_VALUE)
		var prop_path := NodePath("%s:rotation" % bone.get_path())
		anim.track_set_path(p, prop_path)
		var amp := 3.0/180.0*PI
		var speed := tracks[bone_name]
		for i in range(0, 3):
			var t := float(i) * anim.length/2.0
			var val := sin(t*TAU*speed) * amp
			anim.track_insert_key(p, t, val)
	anim_player.add_animation("idle", anim)
	anim_player.play("idle")
