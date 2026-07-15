@tool
extends RefCounted
class_name CutsceneRegistry

const REGISTRY_PATH = "res://assets/cutscenes/cutscene_directory.json"

var registry_data = {}
var categories = []
var asset_paths = {}

signal registry_loaded
signal registry_saved

func _init():
	load_registry()

func load_registry() -> bool:
	if not FileAccess.file_exists(REGISTRY_PATH):
		printerr("Cutscene registry file not found at: ", REGISTRY_PATH)
		_create_default_registry()
		return false
		
	var file = FileAccess.open(REGISTRY_PATH, FileAccess.READ)
	if not file:
		printerr("Failed to open cutscene registry file: ", REGISTRY_PATH)
		return false
		
	var json_text = file.get_as_text()
	file.close()
	
	var json = JSON.new()
	var error = json.parse(json_text)
	if error != OK:
		printerr("JSON Parse Error: ", json.get_error_message(), " at line ", json.get_error_line())
		return false
		
	registry_data = json.get_data()
	categories = registry_data.get("categories", [])
	asset_paths = registry_data.get("asset_paths", {})
	
	emit_signal("registry_loaded")
	return true

func save_registry() -> bool:
	# Update timestamp
	registry_data["last_updated"] = Time.get_datetime_string_from_system(false, true)
	
	var file = FileAccess.open(REGISTRY_PATH, FileAccess.WRITE)
	if not file:
		printerr("Failed to save cutscene registry file: ", REGISTRY_PATH)
		return false
		
	var json_text = JSON.stringify(registry_data, "  ")
	file.store_string(json_text)
	file.close()
	
	emit_signal("registry_saved")
	return true

func get_categories() -> Array:
	return categories

func get_category_by_id(category_id: String) -> Dictionary:
	for category in categories:
		if category.get("id") == category_id:
			return category
	return {}

func get_cutscenes_in_category(category_id: String) -> Array:
	var category = get_category_by_id(category_id)
	if category.has("cutscenes"):
		return category.cutscenes
	return []

func get_cutscene_path(category_id: String, cutscene_id: String) -> String:
	var category = get_category_by_id(category_id)
	if not category.has("path") or not category.has("cutscenes"):
		return ""
		
	for cutscene in category.cutscenes:
		if cutscene.get("id") == cutscene_id:
			return category.path + cutscene.get("file", "")
			
	return ""

func add_cutscene_to_category(category_id: String, cutscene_data: Dictionary) -> bool:
	var category = get_category_by_id(category_id)
	if not category.has("cutscenes"):
		return false
		
	# Check if cutscene already exists
	for i in range(category.cutscenes.size()):
		if category.cutscenes[i].get("id") == cutscene_data.get("id"):
			# Update existing cutscene
			category.cutscenes[i] = cutscene_data
			return save_registry()
	
	# Add new cutscene
	category.cutscenes.append(cutscene_data)
	return save_registry()

func remove_cutscene_from_category(category_id: String, cutscene_id: String) -> bool:
	var category = get_category_by_id(category_id)
	if not category.has("cutscenes"):
		return false
		
	for i in range(category.cutscenes.size()):
		if category.cutscenes[i].get("id") == cutscene_id:
			category.cutscenes.remove_at(i)
			return save_registry()
			
	return false

func add_category(category_data: Dictionary) -> bool:
	# Check if category already exists
	for i in range(categories.size()):
		if categories[i].get("id") == category_data.get("id"):
			categories[i] = category_data
			return save_registry()
	
	categories.append(category_data)
	return save_registry()

func remove_category(category_id: String) -> bool:
	for i in range(categories.size()):
		if categories[i].get("id") == category_id:
			categories.remove_at(i)
			return save_registry()
			
	return false

func get_asset_path(asset_type: String) -> String:
	return asset_paths.get(asset_type, "")

func _create_default_registry() -> void:
	registry_data = {
		"version": "1.0",
		"last_updated": Time.get_datetime_string_from_system(false, true),
		"categories": [
			{
				"id": "introduction",
				"name": "Introduction",
				"path": "res://assets/cutscenes/Introduction/",
				"description": "Introductory cutscenes that explain the game world and backstory",
				"cutscenes": []
			},
			{
				"id": "lost",
				"name": "Lost Kingdom",
				"path": "res://assets/cutscenes/Lost/",
				"description": "Cutscenes related to discovering the lost kingdom of Ironhaven",
				"cutscenes": []
			},
			{
				"id": "battle",
				"name": "Battle Sequences",
				"path": "res://assets/cutscenes/Battle/",
				"description": "Cutscenes for major battles and conflicts",
				"cutscenes": []
			}
		],
		"asset_paths": {
			"images": "res://assets/images/",
			"music": "res://assets/audio/music/",
			"sound_effects": "res://assets/audio/sfx/"
		}
	}
	
	categories = registry_data.categories
	asset_paths = registry_data.asset_paths
	
	save_registry()
