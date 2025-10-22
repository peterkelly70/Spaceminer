extends Control

var log_level: int = 1
 
func _ready():
	printPhase01()
	
func printPhase01():
	Logger.info(self, "printPhase01() called!")
