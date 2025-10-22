extends Node

var log_level: int = 1

# Remove the class_name to avoid hiding the autoload singleton
# class_name Game_Manager

# Define enums that were previously in State class
enum GamePhase {
	ASSIGN_WORKERS,
	BUILD_UPGRADE,
	BUY_LAND,
	EQUIP_SOLDIERS,
	PRODUCTION,
	EVENTS,
	END_TURN
}

enum Season {
	SPRING,
	SUMMER,
	FALL,
	WINTER
}

signal turn_started(season: String, year: int)
signal turn_ended(season: String, year: int)
signal phase_changed(phase: GamePhase)
signal resources_updated(resources: Dictionary)
signal workers_updated(workers: Dictionary)
signal buildings_updated(buildings: Dictionary)
signal building_constructed(building_type: String)
signal building_upgraded(building_type: String, new_level: int)
signal random_event_triggered(event: Dictionary)
signal land_updated(land: Dictionary)
signal military_updated(military: Dictionary)
signal game_loaded
signal game_reset
signal game_saved
signal update_sidebar

# Game state
var current_phase: GamePhase = GamePhase.ASSIGN_WORKERS
var current_season: Season = Season.SPRING
var current_month: int = 1  # 1-3 for each season (3 months per season)
var current_year: int = 1
var current_turn: int = 1  # Track turn number
var game_started: bool = false

# Resources
var resources: Dictionary = {
	"ore": 20,
	"wood": 30,
	"food": 50,
	"gold": 10,
	"gems": 0,
	"mana": 0,
	"crown": 0,
	"lore": 0,
	"runes": 0,
	"warrior": 0
}

# Resource generation rates
var resource_generation: Dictionary = {
	"ore": 0,
	"wood": 0,
	"food": 0,
	"gold": 0,
	"gems": 0,
	"mana": 0,
	"crown": 0,
	"lore": 0,
	"runes": 0,
	"warrior": 0
}

# Land and military
var land: Dictionary = {
	"owned": 100,  # Starting land in acres
	"farmland": 50,  # Land used for farming
	"forest": 30,   # Land with forests for lumber
	"mines": 20,    # Land with mines
	"price_per_acre": 5  # Gold cost per acre of land
}

var military: Dictionary = {
	"soldiers": 0,  # Number of soldiers
	"equipment_level": 0,  # Quality of equipment (0-5)
	"morale": 50,  # Soldier morale (0-100)
	"cost_per_soldier": 10,  # Gold cost per soldier
	"upkeep_per_soldier": 2  # Food cost per soldier per turn
}

# Population
var total_population: int = 10
var available_workers: int = 5
var assigned_workers: Dictionary = {
	"mine": 0,
	"lumber_mill": 0,
	"granary": 0
}
var max_workers: int = 10

# Specialized workers
var specialized_workers: Dictionary = {
	"logger": {"count": 2, "tier": 0},  # Tier: 0=Novice, 1=Journeyman, 2=Master
	"farmer": {"count": 2, "tier": 0},
	"miner": {"count": 2, "tier": 0},
	"mason": {"count": 2, "tier": 0},
	"smith": {"count": 0, "tier": 0},
	"weaver": {"count": 0, "tier": 0},
	"herbalist": {"count": 0, "tier": 0},
	"glassblower": {"count": 0, "tier": 0}
}

# Building worker assignments
var building_workers: Dictionary = {
	"mines": {"assigned": 1, "max": 3, "worker_type": "miner"},
	"farms": {"assigned": 1, "max": 3, "worker_type": "farmer"},
	"lumber": {"assigned": 1, "max": 3, "worker_type": "logger"},
	"quarry": {"assigned": 1, "max": 3, "worker_type": "mason"},
	"smithy": {"assigned": 0, "max": 2, "worker_type": "smith"},
	"carpenter": {"assigned": 0, "max": 2, "worker_type": "logger"},
	"stonemason": {"assigned": 0, "max": 2, "worker_type": "mason"},
	"barracks": {"assigned": 0, "max": 2, "worker_type": "smith"},
	"brewery": {"assigned": 0, "max": 2, "worker_type": "farmer"},
	"alchemist": {"assigned": 0, "max": 1, "worker_type": "herbalist"},
	"runeforge": {"assigned": 0, "max": 1, "worker_type": "smith"},
	"ancestorshrine": {"assigned": 0, "max": 1, "worker_type": "mason"},
	"warhost": {"assigned": 0, "max": 1, "worker_type": "smith"},
	"builbrakt": {"assigned": 0, "max": 1, "worker_type": "glassblower"}
}

# Upgrade slots
var upgrade_slots: Array[Dictionary] = [
	{"occupied": false, "worker_type": "", "tier": 0},
	{"occupied": false, "worker_type": "", "tier": 0},
	{"occupied": false, "worker_type": "", "tier": 0},
	{"occupied": false, "worker_type": "", "tier": 0},
	{"occupied": false, "worker_type": "", "tier": 0},
	{"occupied": false, "worker_type": "", "tier": 0},
	{"occupied": false, "worker_type": "", "tier": 0},
	{"occupied": false, "worker_type": "", "tier": 0}
]

# --- Static Game Data (Loaded from JSON) ---
var buildings: Dictionary = {} # Instance state for each building
var building_data: Dictionary = {} # Static building definitions
var worker_data: Dictionary = {} # Static worker definitions
var season_data: Dictionary = {} # Static season definitions

# --- Data Loading on Startup ---

# --- Accessors for Static Data ---
func get_building_static_data(building_type: String) -> Dictionary:
	if building_data.has(building_type):
		return building_data[building_type]
	return {}

func get_worker_static_data(worker_type: String) -> Dictionary:
	if worker_data.has(worker_type):
		return worker_data[worker_type]
	return {}

func get_season_static_data(season_key: String) -> Dictionary:
	if season_data.has(season_key):
		return season_data[season_key]
	return {}

# --- Resource List/Tooltip Provider ---
var resource_list: Array = []

func get_resource_list() -> Array:
	return resource_list

# --- Worker Counts Provider ---
func get_all_workers() -> Dictionary:
	# Create a dictionary with all worker types from worker_data
	var workers_count = {}
	
	# Initialize all worker types with 0 count
	for worker_type in worker_data.keys():
		workers_count[worker_type] = 0
	
	# Count specialized workers
	for worker_type in specialized_workers.keys():
		var count = specialized_workers[worker_type]["count"]
		
		# Map old worker types to new ones if needed
		var mapped_type = worker_type
		if worker_type == "mason":
			mapped_type = "quarryworker"
		
		# Only add if the mapped worker type is in our dictionary
		if workers_count.has(mapped_type):
			workers_count[mapped_type] += count
	
	# Count workers assigned to buildings
	for building_name in building_workers.keys():
		var building = building_workers[building_name]
		var worker_type = building["worker_type"]
		var assigned = building["assigned"]
		
		# Map old worker types to new ones if needed
		var mapped_type = worker_type
		if worker_type == "mason":
			mapped_type = "quarryworker"
		if worker_type == "herbalist":
			mapped_type = "alchemist"
		
		# Only count if the mapped worker type is in our dictionary
		if workers_count.has(mapped_type):
			workers_count[mapped_type] += assigned
	
	# Count specialized workers in upgrade slots
	for slot in upgrade_slots:
		if slot["occupied"] and slot["worker_type"] != "":
			var worker_type = slot["worker_type"]
			
			# Map old worker types to new ones if needed
			var mapped_type = worker_type
			if worker_type == "mason":
				mapped_type = "quarryworker"
			if worker_type == "herbalist":
				mapped_type = "alchemist"
			
			if workers_count.has(mapped_type):
				workers_count[mapped_type] += 1
	
	# Emit signal to notify of worker update
	workers_updated.emit(workers_count)
	
	return workers_count

# Random events


# Helper function to convert Season enum to string
func _get_string_for_season_enum(season_enum: Season) -> String:
	match season_enum:
		Season.SPRING: return "SPRING"
		Season.SUMMER: return "SUMMER"
		Season.FALL: return "FALL"
		Season.WINTER: return "WINTER"
		_: 
			Logger.error(self, "Unknown season enum value: %s" % season_enum)
			return "UNKNOWN_SEASON"

# Provides a BBCode description for a given season name (string)
var season_defs: Dictionary = {}

func _ready() -> void:
	Logger.info(self, "Initializing...")
	
	# [Fail Fast] Load all JSON data files
	var load_success = true
	
	# Don't automatically show the turn popup - let the MainGameController handle this
	# after the game is fully initialized
	
	# --- Buildings ---
	var building_file = FileAccess.open("res://assets/database/buildings.json", FileAccess.READ)
	if building_file:
		var content = building_file.get_as_text()
		building_file.close()
		var parsed = JSON.parse_string(content)
		if parsed and parsed.has("buildings"):
			building_data = parsed["buildings"]
			Logger.info(self, "Loaded %d building definitions" % building_data.size())
		else:
			Logger.error(self, "[Fail Fast] buildings.json missing 'buildings' key or failed to parse.")
			load_success = false
	else:
		Logger.error(self, "[Fail Fast] Could not open buildings.json!")
		load_success = false
	
	# --- Workers ---
	var worker_file = FileAccess.open("res://assets/database/workers.json", FileAccess.READ)
	if worker_file:
		var content = worker_file.get_as_text()
		worker_file.close()
		var parsed = JSON.parse_string(content)
		if parsed and parsed.has("workers"):
			worker_data = parsed["workers"]
			Logger.info(self, "Loaded %d worker definitions" % worker_data.size())
		else:
			Logger.error(self, "[Fail Fast] workers.json missing 'workers' key or failed to parse.")
			load_success = false
	else:
		Logger.error(self, "[Fail Fast] Could not open workers.json!")
		load_success = false
	
	# --- Seasons ---
	var season_file = FileAccess.open("res://assets/database/seasons.json", FileAccess.READ)
	if season_file:
		var content = season_file.get_as_text()
		season_file.close()
		var parsed = JSON.parse_string(content)
		if parsed and typeof(parsed) == TYPE_DICTIONARY:
			season_data = parsed
			season_defs = parsed # Also set season_defs to avoid duplicate loading
			Logger.info(self, "Loaded %d season definitions" % season_data.size())
		else:
			Logger.error(self, "[Fail Fast] seasons.json failed to parse.")
			load_success = false
	else:
		Logger.error(self, "[Fail Fast] Could not open seasons.json!")
		load_success = false
	
	# --- Resources ---
	var resource_file = FileAccess.open("res://assets/database/resources.json", FileAccess.READ)
	if resource_file:
		var content = resource_file.get_as_text()
		resource_file.close()
		var parsed = JSON.parse_string(content)
		if typeof(parsed) == TYPE_ARRAY:
			resource_list = parsed
			Logger.info(self, "Loaded %d resource definitions" % resource_list.size())
		else:
			Logger.error(self, "[Fail Fast] resources.json is not a valid array!")
			load_success = false
	else:
		Logger.error(self, "[Fail Fast] Could not open resources.json!")
		load_success = false
	
	# [Fail Loud] Report loading status
	if load_success:
		Logger.info(self, "All game data loaded successfully!")
	else:
		Logger.error(self, "[Fail Hard] One or more data files failed to load. Game may not function correctly!")
	
	Logger.info(self, "Initialization complete")

func get_season_description(season_name_str: String) -> String:
	var key = season_name_str.to_upper()
	if season_defs.has(key):
		return season_defs[key].get("bbcode_description", "[color=red]No description found for %s[/color]" % key)
	return "[color=red]No description found for %s[/color]" % key

# Reset resources to their initial values
func _reset_resources() -> void:
	# [Fail Fast] Reset all resources to starting values
	resources = {
		"ore": 20,
		"wood": 30,
		"food": 50,
		"gold": 10,
		"gems": 0,
		"mana": 0,
		"crown": 0,
		"lore": 0,
		"runes": 0,
		"warrior": 0
	}
	
	# Reset resource generation rates
	resource_generation = {
		"ore": 0,
		"wood": 0,
		"food": 0,
		"gold": 0,
		"gems": 0,
		"mana": 0,
		"crown": 0,
		"lore": 0,
		"runes": 0,
		"warrior": 0
	}
	
	Logger.info(self, "Resources reset to initial values")

# Start a new game
func start_new_game() -> void:
	Logger.info(self, "Starting new game")
	
	# [Fail Fast] Reset game state
	current_phase = GamePhase.ASSIGN_WORKERS
	current_season = Season.SPRING
	current_month = 1
	current_year = 1
	current_turn = 1
	
	# Initialize resources to starting values
	_reset_resources()
	
	# Don't set game_started to true yet - this will be done when showing the turn popup
	# to prevent premature turn initialization
	game_started = false
	
	# Emit signals to update UI
	emit_signal("resources_updated", resources)
	emit_signal("game_reset")
	
	Logger.info(self, "Game state initialized")
	

# Advance to the next phase
func next_phase() -> void:
	if not game_started:
		Logger.warn(self, "Game not started, cannot advance phase.")
		return
		
	var old_phase = current_phase
	var default_phase_count = GamePhase.values().size()
	current_phase = GamePhase.values()[((current_phase as int) + 1) % default_phase_count]
	
	# If we've gone through all phases, advance to the next season
	if current_phase == GamePhase.ASSIGN_WORKERS and old_phase != GamePhase.ASSIGN_WORKERS:
		# End of turn
		emit_signal("turn_ended", get_current_season_name(), current_year)
		# Advance to the next season/year
		_advance_season()
	
	Logger.info(self, "Phase changed: %s -> %s" % [GamePhase.keys()[old_phase], GamePhase.keys()[current_phase]])
	
	# Emit phase changed signal
	emit_signal("phase_changed", current_phase)

# Advance to the next season
# Advance to the next month, which may trigger season and year changes
func _advance_month() -> void:
	# Store old month value for logging
	var old_month = current_month
	
	# Advance to the next month
	current_month += 1
	
	# If we've completed 3 months in a season, move to the next season
	if current_month > 3:
		current_month = 1
		_advance_season()
	
	# Log the month change
	Logger.info(self, "Month changed: %d -> %d in %s" % [old_month, current_month, get_current_season_name()])

# Advance to the next season
func _advance_season() -> void:
	# Store old values for comparison
	var old_season = current_season
	var old_year = current_year
	
	# Get the next season
	var season_count = Season.values().size()
	current_season = Season.values()[((current_season as int) + 1) % season_count]
	
	# If we've gone from WINTER to SPRING, advance the year
	if old_season == Season.WINTER and current_season == Season.SPRING:
		current_year += 1
	
	# Emit the turn started signal
	emit_signal("turn_started", get_current_season_name(), current_year)
	
	Logger.info(self, "Season changed: %s -> %s" % [Season.keys()[old_season], Season.keys()[current_season]])
	if current_year != old_year:
		Logger.info(self, "Year changed: %d -> %d" % [old_year, current_year])

# Calculate resource generation based on worker assignments and tiers
func _calculate_resource_generation() -> void:
	# Reset generation rates
	resource_generation = {
		"ore": 0,
		"wood": 0,
		"food": 0,
		"gold": 0,
		"gems": 0,
		"mana": 0,
		"crown": 0,
		"lore": 0,
		"runes": 0,
		"warrior": 0
	}
	
	# Calculate production from specialized workers assigned to buildings
	for building_name in building_workers.keys():
		var building = building_workers[building_name]
		var assigned = building.assigned
		var worker_type = building.worker_type
		
		if assigned <= 0:
			continue
		
		# Get worker tier bonus
		var tier_bonus = 1.0
		if specialized_workers.has(worker_type):
			tier_bonus = 1.0 + (specialized_workers[worker_type].tier * 0.5)
		
		# Calculate resource production based on building type
		match building_name:
			"mines":
				resource_generation["ore"] += int(2 * assigned * tier_bonus)
			"farms":
				resource_generation["food"] += int(3 * assigned * tier_bonus)
			"lumber":
				resource_generation["wood"] += int(2 * assigned * tier_bonus)
			"quarry":
				resource_generation["ore"] += int(1 * assigned * tier_bonus)
				resource_generation["gems"] += int(0.5 * assigned * tier_bonus)
			"smithy":
				resource_generation["gold"] += int(2 * assigned * tier_bonus)
			"carpenter":
				resource_generation["gold"] += int(1 * assigned * tier_bonus)
				resource_generation["wood"] += int(1 * assigned * tier_bonus)
			"stonemason":
				resource_generation["gold"] += int(1 * assigned * tier_bonus)
				resource_generation["ore"] += int(1 * assigned * tier_bonus)
			"barracks":
				resource_generation["warrior"] += int(1 * assigned * tier_bonus)
			"brewery":
				resource_generation["gold"] += int(3 * assigned * tier_bonus)
			"alchemist":
				resource_generation["mana"] += int(2 * assigned * tier_bonus)
			"runeforge":
				resource_generation["runes"] += int(1 * assigned * tier_bonus)
			"ancestorshrine":
				resource_generation["lore"] += int(2 * assigned * tier_bonus)
			"warhost":
				resource_generation["warrior"] += int(2 * assigned * tier_bonus)
			"builbrakt":
				resource_generation["crown"] += int(1 * assigned * tier_bonus)
	
	# Apply seasonal modifiers
	match current_season:
		Season.SPRING:
			resource_generation["food"] = int(resource_generation["food"] * 1.2)
			resource_generation["wood"] = int(resource_generation["wood"] * 1.1)
		Season.SUMMER:
			resource_generation["food"] = int(resource_generation["food"] * 1.5)
			resource_generation["ore"] = int(resource_generation["ore"] * 1.2)
		Season.FALL:
			resource_generation["food"] = int(resource_generation["food"] * 0.8)
			resource_generation["gold"] = int(resource_generation["gold"] * 1.3)
		Season.WINTER:
			resource_generation["food"] = int(resource_generation["food"] * 0.5)
			resource_generation["wood"] = int(resource_generation["wood"] * 0.7)
			resource_generation["ore"] = int(resource_generation["ore"] * 0.8)
	
	# Update resources with generation
	for resource in resource_generation.keys():
		resources[resource] += resource_generation[resource]
	
	# Emit signal for updated resources
	emit_signal("resources_updated", resources)

# Assign workers to a building
func assign_workers(building_type: String, count: int) -> bool:
	if not game_started or current_phase != GamePhase.ASSIGN_WORKERS:
		return false
	
	# Check if building exists
	if not buildings.has(building_type):
		return false
	
	# Check if we have enough available workers
	if available_workers < count:
		return false
	
	# Check if building can accommodate more workers
	var building = buildings[building_type]
	var current_workers = assigned_workers.get(building_type, 0)
	var max_workers_for_building = building.get("max_workers", 0)
	
	if current_workers + count > max_workers_for_building:
		return false
	
	# Assign workers
	available_workers -= count
	assigned_workers[building_type] = current_workers + count
	
	# Update resource generation
	_calculate_resource_generation()
	
	# Emit signals
	if assigned_workers[building_type] < count:
		return false
	
	# Unassign workers
	assigned_workers[building_type] -= count
	available_workers += count
	
	emit_signal("workers_updated", {"total": total_population, "available": available_workers, "assigned": assigned_workers})
	return true

# Build a new building
func build_building(building_type: String) -> bool:
	if not game_started or current_phase != GamePhase.BUILD_UPGRADE:
		return false
	
	# Check if we have enough resources
	var cost = buildings[building_type]["build_cost"]
	for resource in cost.keys():
		if resources[resource] < cost[resource]:
			return false
	
	# Deduct resources
	for resource in cost.keys():
		resources[resource] -= cost[resource]
	
	# Add building
	buildings[building_type]["count"] += 1
	
	emit_signal("resources_updated", resources)
	emit_signal("buildings_updated", buildings)
	return true

# Upgrade a building
func upgrade_building(building_type: String) -> bool:
	if not game_started or current_phase != GamePhase.BUILD_UPGRADE:
		return false
	
	# Check if the building exists
	if buildings[building_type]["count"] <= 0:
		return false
	
	# Check if we have enough resources
	var cost = buildings[building_type]["upgrade_cost"]
	for resource in cost.keys():
		if resources[resource] < cost[resource]:
			return false
	
	# Deduct resources
	for resource in cost.keys():
		resources[resource] -= cost[resource]
	
	# Upgrade building
	buildings[building_type]["level"] += 1
	var new_level = buildings[building_type]["level"]
	
	# Increase costs for next upgrade
	for resource in buildings[building_type]["upgrade_cost"].keys():
		buildings[building_type]["upgrade_cost"][resource] = int(buildings[building_type]["upgrade_cost"][resource] * 1.5)
	
	emit_signal("resources_updated", resources)
	emit_signal("buildings_updated", buildings)
	emit_signal("building_upgraded", building_type, new_level)
	return true

# Buy new land
func buy_land(acres: int) -> bool:
	if not game_started or current_phase != GamePhase.BUY_LAND:
		return false
	
	# Check if we have enough gold
	var total_cost = acres * land["price_per_acre"]
	if resources["gold"] < total_cost:
		return false
	
	# Deduct gold
	resources["gold"] -= total_cost
	
	# Add land
	land["owned"] += acres
	
	# Increase land price slightly (simulate market forces)
	land["price_per_acre"] = int(land["price_per_acre"] * (1 + (acres * 0.01)))
	
	emit_signal("resources_updated", resources)
	emit_signal("land_updated", land)
	return true

# Allocate land usage
func allocate_land(land_type: String, acres: int) -> bool:
	if not game_started or current_phase != GamePhase.BUY_LAND:
		return false
	
	# Calculate total allocated land
	var total_allocated = land["farmland"] + land["forest"] + land["mines"]
	
	# Check if we're trying to allocate more than we own
	if total_allocated + acres > land["owned"]:
		return false
	
	# Check if we're trying to reduce below zero
	if land[land_type] + acres < 0:
		return false
	
	# Allocate land
	land[land_type] += acres
	
	emit_signal("land_updated", land)
	return true

# Recruit soldiers
func recruit_soldiers(count: int) -> bool:
	if not game_started or current_phase != GamePhase.EQUIP_SOLDIERS:
		return false
	
	# Check if we have enough gold
	var total_cost = count * military["cost_per_soldier"]
	if resources["gold"] < total_cost:
		return false
	
	# Deduct gold
	resources["gold"] -= total_cost
	
	# Add soldiers
	military["soldiers"] += count
	
	emit_signal("resources_updated", resources)
	emit_signal("military_updated", military)
	return true

# Upgrade military equipment
func upgrade_military_equipment() -> bool:
	if not game_started or current_phase != GamePhase.EQUIP_SOLDIERS:
		return false
	
	# Check if we're already at max level
	if military["equipment_level"] >= 5:
		return false
	
	# Cost increases with each level
	var upgrade_cost = 20 * (military["equipment_level"] + 1)
	
	# Check if we have enough gold
	if resources["gold"] < upgrade_cost:
		return false
	
	# Deduct gold
	resources["gold"] -= upgrade_cost
	
	# Upgrade equipment level
	military["equipment_level"] += 1
	
	# Boost morale
	military["morale"] = min(100, military["morale"] + 10)
	
	emit_signal("resources_updated", resources)
	emit_signal("military_updated", military)
	return true

# Get the current season name
func get_current_season_name() -> String:
	return Season.keys()[current_season]

# Get the current season
func get_current_season() -> int:
	return current_season

# Get the current month (1-3 within each season)
func get_current_month() -> int:
	return current_month

# Get the current month name
func get_current_month_name() -> String:
	# Get dwarf month names based on season and month number
	var season_name = get_current_season_name().to_lower()
	var month_index = current_month - 1
	
	# Try to get dwarf month names from season data first
	var season_info = get_season_static_data(season_name)
	if season_info.has("dwarf_months") and season_info["dwarf_months"].size() > month_index:
		return season_info["dwarf_months"][month_index]
	
	# Fall back to regular month names if dwarf names not available
	if season_info.has("months") and season_info["months"].size() > month_index:
		return season_info["months"][month_index]
	
	# Default dwarf month names if not found in season data
	var default_month_names = {
		"spring": ["Thawfrost", "Seedsow", "Sproutgrow"],
		"summer": ["Fireforge", "Stonedry", "Goldgrain"],
		"fall": ["Amberhew", "Mistfall", "Stormshroud"],
		"winter": ["Icevault", "Deepdark", "Frosthold"]
	}
	
	# Fall back to default dwarf month names
	if default_month_names.has(season_name) and default_month_names[season_name].size() > month_index:
		return default_month_names[season_name][month_index]
	
	# Last resort fallback
	return "Month %d of %s" % [current_month, get_current_season_name()]

# Get the current year
func get_current_year() -> int:
	return current_year

# Get the current phase name
func get_current_phase_name() -> String:
	return GamePhase.keys()[current_phase]

# Get the current phase
func get_current_phase() -> GamePhase:
	return current_phase

# Get building data
func get_building_data(building_type: String) -> Dictionary:
	if building_data.has(building_type):
		return building_data[building_type]
	return {}

# Get building position
func get_building_position(building_type: String) -> Vector2:
	if building_data.has(building_type) and building_data[building_type].has("position"):
		var pos = building_data[building_type]["position"]
		return Vector2(pos.x, pos.y)
	return Vector2.ZERO

# Get building level
func get_building_level(building_type: String) -> int:
	if buildings.has(building_type):
		return buildings[building_type]["level"]
	return 0

# Get all resources with their current values
func get_all_resources() -> Dictionary:
	return resources.duplicate()

# Get all resource generation rates
func get_all_resource_generation() -> Dictionary:
	return resource_generation.duplicate()

# Check if building is built
func is_building_built(building_type: String) -> bool:
	if buildings.has(building_type):
		return buildings[building_type]["count"] > 0 and buildings[building_type]["level"] > 0
	return false

# Construct a new building (for TownMapController)
func construct_building(building_type: String) -> bool:
	# Check if building exists in data
	if not building_data.has(building_type):
		return false
	
	# Check if building exists in game state
	if not buildings.has(building_type):
		# Add new building to game state
		buildings[building_type] = {
			"level": 0,
			"count": 0,
			"production_per_worker": 1,
			"max_workers": 1,
			"build_cost": {},
			"upgrade_cost": {}
		}
	
	# Set building as constructed
	buildings[building_type]["count"] = 1
	buildings[building_type]["level"] = 1
	
	emit_signal("buildings_updated", buildings)
	emit_signal("building_constructed", building_type)
	return true

"""
Shows the save game dialog for the current game state.
You can implement this to open the appropriate dialog or trigger save logic.
"""
func save_game_dialog() -> void:
	# TODO: Implement save dialog logic for the current game state
	Logger.info(self, "save_game_dialog() called - implement UI or logic here.")
	
	# Emit the game_saved signal
	emit_signal("game_saved")

"""
Shows the load game dialog for the current game state.
You can implement this to open the appropriate dialog or trigger load logic.
"""
func load_game_dialog() -> void:
	# TODO: Implement load dialog logic for the current game state
	Logger.info(self, "load_game_dialog() called - implement UI or logic here.")
	
	# For now, just emit the signal to test the connection
	# In a real implementation, this would be called after successfully loading game data
	emit_signal("game_loaded")
	
	# Notify all UI elements about the updated resources
	emit_signal("resources_updated", resources)

# Advances to the next turn, updating month, season and year as appropriate
func next_turn() -> void:
	# Signal that the current turn is ending
	emit_signal("turn_ended", get_current_season_name(), current_year)
	
	# Increment turn number
	current_turn += 1
	
	# Advance the month
	_advance_month()
	
	# Calculate resource generation for the new turn
	_calculate_resource_generation()
	
	# Apply any seasonal effects
	_apply_seasonal_effects()
	
	# Get current season and month information
	var season_name = get_current_season_name()
	var month_name = get_current_month_name()
	
	# Create combined season string
	var combined_season = "%s - %s" % [season_name, month_name]
	
	# Note: Sidebar updates removed - sidebar only exists in MainGameView, not MenuView
	# The sidebar will be updated via signals when the MainGameView is active
	
	# Emit signals for UI updates
	Logger.debug(self, "Emitting turn_started with combined season: %s" % combined_season)
	emit_signal("turn_started", combined_season, current_year)
	emit_signal("resources_updated", resources)
	emit_signal("workers_updated")
	emit_signal("update_sidebar")
	
	Logger.info(self, "Advanced to next turn: %d - %s (Month %d), Year %d" % [current_turn, get_current_season_name(), current_month, current_year])
	
	# Show the turn start popup
	_show_turn_start_popup()

# Apply effects specific to the current season
func _apply_seasonal_effects() -> void:
	# Get the current season data
	var season_name = get_current_season_name()
	var season_info = get_season_static_data(season_name.to_lower())
	
	# Apply food consumption in winter
	if current_season == Season.WINTER:
		# Increased food consumption in winter
		var food_consumed = int(total_population * 0.5) # Base consumption
		resources["food"] = max(0, resources["food"] - food_consumed)
		
		Logger.info(self, "Winter food consumption: %d food consumed" % food_consumed)
		
		# If food runs out, reduce morale
		if resources["food"] <= 0:
			military["morale"] = max(0, military["morale"] - 20)
			Logger.warn(self, "Food shortage! Morale reduced to %d" % military["morale"])
	
	# Apply any seasonal effects from the season data
	if season_info.has("effects"):
		for effect in season_info["effects"]:
			# Process effects based on their type
			if effect.has("type"):
				match effect["type"]:
					"resource_modifier":
						if effect.has("resource") and effect.has("multiplier") and resources.has(effect["resource"]):
							var resource_name = effect["resource"]
							var multiplier = effect["multiplier"]
							
							# Apply to resource generation rather than direct resources
							if resource_generation.has(resource_name):
								var old_value = resource_generation[resource_name]
								resource_generation[resource_name] = int(old_value * multiplier)
								
								Logger.info(self, "%s seasonal effect: %s generation %d -> %d (x%.1f)" % 
									[season_name, resource_name, old_value, resource_generation[resource_name], multiplier])
	
	# Update UI
	emit_signal("resources_updated", resources)
	emit_signal("military_updated", military)

# Emit signals to update the UI with the current game state
func _update_game_ui() -> void:
	# Get current season and month information
	var season_name = get_current_season_name()
	var month_name = get_current_month_name()
	
	# Emit signals for UI updates - include month name in debug output
	emit_signal("turn_started", season_name, current_year)
	emit_signal("resources_updated", resources)
	emit_signal("workers_updated")
	
	Logger.debug(self, "Emitted signals to update UI with new turn information: Season %s, Month %s, Year %d" % 
		[season_name, month_name, current_year])

# Show the initial turn popup when the game first loads
func _show_initial_turn_popup() -> void:
	# [Fail Fast] Only proceed if the game is properly initialized
	if not game_started:
		game_started = true
		Logger.info(self, "Setting game_started to true")
	
	# Wait a short time to ensure everything is fully initialized
	await get_tree().create_timer(0.5).timeout
	
	# Show the turn start popup
	Logger.info(self, "Showing initial turn popup")
	_show_turn_start_popup()

# Show the turn start popup with current turn information
func _show_turn_start_popup() -> void:
	# Get current season data
	var season_name = get_current_season_name()
	var season_info = get_season_static_data(season_name.to_lower())
	var month_name = get_current_month_name()
	
	# Create parameters for the popup
	var popup_params = {
		"turn_number": current_turn,
		"season": season_name,
		"month_name": month_name,
		"year": current_year,
		"resource_changes": {}
	}
	
	# Add season description if available
	if season_info.has("bbcode_description"):
		popup_params["season_description"] = season_info["bbcode_description"]
	
	# Add resource modifiers if available
	if season_info.has("effects"):
		for effect in season_info["effects"]:
			if effect.has("type") and effect["type"] == "resource_modifier":
				if effect.has("resource") and effect.has("multiplier"):
					var resource_name = effect["resource"]
					var multiplier = effect["multiplier"]
					# Calculate percentage change (multiplier - 1.0)
					popup_params["resource_changes"][resource_name] = multiplier - 1.0
	
	# Show the popup using the PopupManager singleton
	var popup_manager = get_node("/root/PopupManager")
	if popup_manager:
		popup_manager.show_popup("TurnStartPopup", popup_params)
	else:
		push_error("[Game_Manager] PopupManager singleton not found")
	
