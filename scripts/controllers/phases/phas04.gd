extends Control

var log_level: int = 1
 
func _ready():
	printPhase04()
	
func printPhase04():
	Logger.info(self, "printPhase04() called!")
