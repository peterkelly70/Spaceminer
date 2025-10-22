extends Control

var log_level: int = 1
 
func _ready():
	printPhase05()
	
func printPhase05():
	Logger.info(self, "printPhase05() called!")
