extends CharacterBody2D
@export var speed := 200.0
func _physics_process(delta):
    move_and_slide()
