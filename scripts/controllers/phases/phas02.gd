extends Control

var log_level: int = 1

func _ready():
	printPhase02()

func printPhase02():
	Logger.info(self, "printPhase02() called!")
