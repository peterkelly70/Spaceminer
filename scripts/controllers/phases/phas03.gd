extends Control

var log_level: int = 1
 
func _ready():
	printPhase03()
	
func printPhase03():
	Logger.info(self, "printPhase03() called!")
