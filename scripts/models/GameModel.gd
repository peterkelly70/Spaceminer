extends Node
class_name GameModel

var log_level: int = 1

# Game Metadata
var game_name: String = "Ironhaven Chronicle"
var game_start_timestamp: int = 0 # Initialized by GameManager or reset
var last_save_timestamp: int = 0

# Progression
var turn_number: int = 0 # Initialized by GameManager or reset
var current_year: int = 0 # Initialized by GameManager or reset
var current_season: String = "" # Initialized by GameManager or reset

# Signals
signal resource_updated(resource_key, new_amount, new_generation)
signal turn_advanced(new_turn, new_year, new_season)
signal game_loaded
signal game_saved
signal game_reset

# Default state constants
const DEFAULT_TURN_NUMBER = 1
const DEFAULT_YEAR = 1
const DEFAULT_SEASON = "Spring"
const DEFAULT_RESOURCES = {
	"ore": {"current": 100, "generation": 5},
	"stone": {"current": 80, "generation": 4},
	"wood": {"current": 120, "generation": 6},
	"food": {"current": 50, "generation": 10},
	"gold": {"current": 20, "generation": 1},
	"gems": {"current": 5, "generation": 0},
	"mana": {"current": 10, "generation": 1},
	"crown": {"current": 1, "generation": 0},
	"lore": {"current": 0, "generation": 0},
	"runes": {"current": 0, "generation": 0},
	"tools": {"current": 10, "generation": 0},
	"weapons": {"current": 5, "generation": 0}
}

# --- Resources --- 
var resources: Dictionary = {}

# --- Building and Worker Data ---
var buildings: Dictionary = {
	"mine": {"count": 1, "level": 1, "production_per_worker": 1.5, "max_workers": 5, "assigned_workers": 0},
	"lumber_mill": {"count": 1, "level": 1, "production_per_worker": 1.0, "max_workers": 5, "assigned_workers": 0},
	"granary": {"count": 1, "level": 1, "production_per_worker": 1.0, "max_workers": 5, "assigned_workers": 0}
}

var total_workers: int = 10
var available_workers: int = 10 # This will be calculated based on total_workers - sum of assigned_workers

# --- Town Map Data (Placeholder) ---
var current_town_map_data: Dictionary = {}

func _init():
	# Initialize with empty or default structures if necessary
	# reset_to_default_state() will typically be called by GameManager after instantiation
	pass

func reset_to_default_state() -> void:
	game_name = "Ironhaven Chronicle"
	game_start_timestamp = Time.get_unix_time_from_system()
	last_save_timestamp = 0
	
	turn_number = DEFAULT_TURN_NUMBER
	current_year = DEFAULT_YEAR
	current_season = DEFAULT_SEASON
	
	resources = {}
	for key in DEFAULT_RESOURCES:
		resources[key] = DEFAULT_RESOURCES[key].duplicate(true)
	
	# Reset buildings to default state
	buildings = {
		"mine": {"count": 1, "level": 1, "production_per_worker": 1.5, "max_workers": 5, "assigned_workers": 0},
		"lumber_mill": {"count": 1, "level": 1, "production_per_worker": 1.0, "max_workers": 5, "assigned_workers": 0},
		"granary": {"count": 1, "level": 1, "production_per_worker": 1.0, "max_workers": 5, "assigned_workers": 0}
	}
	
	# Reset worker counts
	total_workers = 10
	available_workers = 10
	
	current_town_map_data = {}
	
	print("State reset to defaults.")
	emit_signal("game_reset")
	# Emit signals to update UI after reset
	for key in resources:
		emit_signal("resource_updated", key, get_resource_amount(key), get_resource_generation(key))
	emit_signal("turn_advanced", turn_number, current_year, current_season)


# --- Resource Management --- 
func get_resource_amount(resource_key: String) -> int:
	if resources.has(resource_key) and resources[resource_key].has("current"):
		return resources[resource_key]["current"]
	push_error("Tried to get amount for uninitialized/unknown resource '%s'" % resource_key)
	return 0

func get_resource_generation(resource_key: String) -> int:
	if resources.has(resource_key) and resources[resource_key].has("generation"):
		return resources[resource_key]["generation"]
	push_error("Tried to get generation for uninitialized/unknown resource '%s'" % resource_key)
	return 0

func set_resource_amount(resource_key: String, new_amount: int) -> void:
	if not resources.has(resource_key):
		push_error("Tried to set amount for unknown resource '%s'" % resource_key)
		return
	
	var old_amount = resources[resource_key].get("current", 0)
	if old_amount != new_amount:
		resources[resource_key]["current"] = new_amount
		emit_signal("resource_updated", resource_key, new_amount, get_resource_generation(resource_key))

func set_resource_generation(resource_key: String, new_generation: int) -> void:
	if not resources.has(resource_key):
		push_error("Tried to set generation for unknown resource '%s'" % resource_key)
		return

	var old_generation = resources[resource_key].get("generation", 0)
	if old_generation != new_generation:
		resources[resource_key]["generation"] = new_generation
		emit_signal("resource_updated", resource_key, get_resource_amount(resource_key), new_generation)

func add_to_resource(resource_key: String, amount_to_add: int) -> void:
	if not resources.has(resource_key):
		push_error("Tried to add to unknown resource '%s'" % resource_key)
		return
	
	var current_val = get_resource_amount(resource_key)
	set_resource_amount(resource_key, current_val + amount_to_add)

func get_all_resources_current() -> Dictionary:
	var current_res = {}
	for key in resources:
		current_res[key] = resources[key]["current"]
	return current_res

func get_all_resources_generation() -> Dictionary:
	var generation_res = {}
	for key in resources:
		generation_res[key] = resources[key]["generation"]
	return generation_res

# --- Worker Management ---

func get_available_workers() -> int:
	return available_workers

func get_max_workers() -> int:
	return total_workers

func get_assigned_workers(building_type: String) -> int:
	if not buildings.has(building_type):
		push_error("Tried to get assigned workers for unknown building '%s'" % building_type)
		return 0
	
	return buildings[building_type]["assigned_workers"]

func adjust_worker_assignment(building_type: String, amount: int) -> void:
	if not buildings.has(building_type):
		push_error("Tried to adjust workers for unknown building '%s'" % building_type)
		return
	
	var current_assigned = buildings[building_type]["assigned_workers"]
	var max_workers_for_building = buildings[building_type]["max_workers"]
	
	# Adding workers
	if amount > 0:
		# Check if we have enough available workers
		if available_workers < amount:
			push_error("Not enough available workers to assign %s workers" % amount)
			return
		
		# Check if the building can hold more workers
		if current_assigned + amount > max_workers_for_building:
			push_error("Building '%s' cannot hold more than %s workers" % [building_type, max_workers_for_building])
			return
		
		# Assign workers
		buildings[building_type]["assigned_workers"] += amount
		available_workers -= amount
	
	# Removing workers
	elif amount < 0:
		# Check if there are enough assigned workers to remove
		if current_assigned < abs(amount):
			push_error("Not enough assigned workers to remove %s workers from '%s'" % [abs(amount), building_type])
			return
		
		# Remove workers
		buildings[building_type]["assigned_workers"] += amount # amount is negative
		available_workers -= amount # amount is negative, so this adds to available_workers
	
	# Update resource generation based on worker assignment
	_update_resource_generation()

# Update resource generation rates based on worker assignments
func _update_resource_generation() -> void:
	# Reset all resource generation to base values (if any)
	for resource_key in resources:
		resources[resource_key]["generation"] = 0
	
	# Mine produces ore and stone
	if buildings.has("mine"):
		var mine_workers = buildings["mine"]["assigned_workers"]
		var mine_production = buildings["mine"]["production_per_worker"]
		var mine_level = buildings["mine"]["level"]
		var ore_production = int(mine_workers * mine_production * mine_level)
		var stone_production = int(mine_workers * (mine_production * 0.5) * mine_level)
		
		set_resource_generation("ore", ore_production)
		set_resource_generation("stone", stone_production)
	
	# Lumber mill produces wood
	if buildings.has("lumber_mill"):
		var lumber_workers = buildings["lumber_mill"]["assigned_workers"]
		var lumber_production = buildings["lumber_mill"]["production_per_worker"]
		var lumber_level = buildings["lumber_mill"]["level"]
		var wood_production = int(lumber_workers * lumber_production * lumber_level)
		
		set_resource_generation("wood", wood_production)
	
	# Granary produces food
	if buildings.has("granary"):
		var granary_workers = buildings["granary"]["assigned_workers"]
		var granary_production = buildings["granary"]["production_per_worker"]
		var granary_level = buildings["granary"]["level"]
		var food_production = int(granary_workers * granary_production * granary_level)
		
		set_resource_generation("food", food_production)

# --- Building Management ---

func get_building_data(building_type: String) -> Dictionary:
	if not buildings.has(building_type):
		push_error("Tried to get data for unknown building '%s'" % building_type)
		return {"count": 0, "level": 1, "production_per_worker": 1.0, "max_workers": 0, "assigned_workers": 0}
	
	return buildings[building_type]

func get_season_description(season_name: String) -> String:
	match season_name:
		"Spring": return "The season of growth and renewal."
		"Summer": return "The season of warmth and abundance."
		"Autumn": return "The season of harvest and preparation."
		"Winter": return "The season of cold and scarcity."
		_: return "Unknown season."

# --- Game Progression --- 
func advance_turn() -> void:
	for resource_key in resources:
		add_to_resource(resource_key, get_resource_generation(resource_key))
	
	turn_number += 1
	if current_season == "Spring": current_season = "Summer"
	elif current_season == "Summer": current_season = "Autumn"
	elif current_season == "Autumn": current_season = "Winter"
	elif current_season == "Winter":
		current_season = "Spring"
		current_year += 1
	
	emit_signal("turn_advanced", turn_number, current_year, current_season)
	print("Turn Advanced: %s, Year: %s, Season: %s" % [turn_number, current_year, current_season])

# --- Save/Load --- 
func get_save_data() -> Dictionary:
	return {
		"game_name": game_name,
		"turn_number": turn_number,
		"current_year": current_year,
		"current_season": current_season,
		"resources": resources,
		"current_town_map_data": current_town_map_data,
		"game_start_timestamp": game_start_timestamp,
		"last_save_timestamp": Time.get_unix_time_from_system()
	}

func load_from_data(data: Dictionary) -> bool:
	game_name = data.get("game_name", "Ironhaven Chronicle")
	turn_number = data.get("turn_number", DEFAULT_TURN_NUMBER)
	current_year = data.get("current_year", DEFAULT_YEAR)
	current_season = data.get("current_season", DEFAULT_SEASON)
	# Ensure resources are loaded correctly, potentially deep copying
	var loaded_resources = data.get("resources", DEFAULT_RESOURCES.duplicate(true))
	resources = {}
	for key in loaded_resources:
		if loaded_resources[key] is Dictionary:
			resources[key] = loaded_resources[key].duplicate(true)
		else: # Fallback for older save format if necessary, or error
			push_error("Malformed resource data for key '%s' during load." % key)
			# Ensure a valid structure even if data is bad
			if not DEFAULT_RESOURCES.has(key):
				push_error("Critical error - Default resource '%s' not found during faulty load recovery." % key)
				resources[key] = {"current":0, "generation":0}
			else:
				resources[key] = DEFAULT_RESOURCES[key].duplicate(true)
	
	current_town_map_data = data.get("current_town_map_data", {})
	game_start_timestamp = data.get("game_start_timestamp", Time.get_unix_time_from_system())
	last_save_timestamp = data.get("last_save_timestamp", 0)
	
	print("Data loaded.")
	emit_signal("game_loaded")
	# Manually emit resource_updated for all resources to refresh UI
	for key in resources:
		emit_signal("resource_updated", key, get_resource_amount(key), get_resource_generation(key))
	emit_signal("turn_advanced", turn_number, current_year, current_season)
	return true

func save_game_to_file(file_path: String = "user://ironhaven_save.dat") -> void:
	var save_data = get_save_data()
	# Update last_save_timestamp right before saving
	save_data.last_save_timestamp = Time.get_unix_time_from_system()
	
	var file = FileAccess.open(file_path, FileAccess.WRITE)
	if file:
		var json_string = JSON.stringify(save_data, "\t")
		file.store_string(json_string)
		file.close()
		self.last_save_timestamp = save_data.last_save_timestamp # Update model's timestamp
		emit_signal("game_saved")
		print("Game saved to: %s" % file_path)
	else:
		push_error("Error saving game to %s" % file_path)

func load_game_from_file(file_path: String = "user://ironhaven_save.dat") -> bool:
	if not FileAccess.file_exists(file_path):
		push_error("Save file not found at %s" % file_path)
		return false
	
	var file = FileAccess.open(file_path, FileAccess.READ)
	if file:
		var json_string = file.get_as_text()
		file.close()
		
		var parse_result = JSON.parse_string(json_string)
		if parse_result == null:
			push_error("Error parsing save file JSON from %s" % file_path)
			return false

		return load_from_data(parse_result)
	else:
		push_error("Error loading game from %s" % file_path)
		return false
