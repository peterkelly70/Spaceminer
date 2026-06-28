extends Node

const ASTEROID_CAMPAIGN_GENERATOR_SCRIPT := preload("res://scripts/tools/AsteroidCampaignGenerator.gd")
const BASE_FUEL_CAPACITY := 160.0
const FUEL_CAPACITY_PER_JETPACK := 40.0
const STARTING_LIVES := 10

signal run_started(run_data: Dictionary)
signal run_loaded(run_data: Dictionary)
signal run_saved(save_path: String, run_data: Dictionary)

const SAVE_DIR := "user://saves/spaceminer/"
const SAVE_EXTENSION := ".json"
const DEFAULT_CAMPAIGN := "asteroid_56_room"

var active_seed: String = ""
var active_save_path: String = ""
var active_run: Dictionary = {}
var active_campaign_manifest: Dictionary = {}
var active_campaign_manifest_path: String = ""

func _ready() -> void:
	_ensure_save_dir()

func _ensure_save_dir() -> void:
	var absolute_path := ProjectSettings.globalize_path(SAVE_DIR)
	DirAccess.make_dir_recursive_absolute(absolute_path)

func start_new_run(seed_text: String = "", run_name: String = "") -> Dictionary:
	var seed := seed_text.strip_edges()
	if seed.is_empty():
		seed = generate_seed()

	active_seed = seed
	active_save_path = ""
	_generate_or_load_campaign(seed)
	active_run = {
		"run_name": run_name if not run_name.strip_edges().is_empty() else _generate_run_name(seed),
		"seed": seed,
		"campaign": str(active_campaign_manifest.get("campaign_name", DEFAULT_CAMPAIGN)),
		"campaign_manifest_path": active_campaign_manifest_path,
		"campaign_dir": str(active_campaign_manifest.get("campaign_dir", "")),
		"created_at": Time.get_unix_time_from_system(),
		"updated_at": Time.get_unix_time_from_system(),
		"current_room_id": str(active_campaign_manifest.get("start_room_id", "room_000")),
		"room_count": int(active_campaign_manifest.get("room_count", 56)),
		"critical_path_index": 0,
		"collected_ore": 0,
		"oxygen_pct": 100.0,
		"security_cards": [],
		"unlocked_tools": [],
		"equipment": [],
		"branches": [],
		"checkpoint": "start",
		"checkpoint_room_id": str(active_campaign_manifest.get("start_room_id", "room_000")),
		"fuel_pct": BASE_FUEL_CAPACITY,
		"fuel_capacity_pct": BASE_FUEL_CAPACITY,
		"jetpack_upgrades": 0,
		"battery_pct": 100.0,
		"ore_count": 0,
		"lives_remaining": STARTING_LIVES,
		"max_lives": STARTING_LIVES,
	}

	save_current_run(active_run["run_name"])
	run_started.emit(active_run.duplicate(true))
	return active_run.duplicate(true)

func save_current_run(save_name: String = "", extra_data: Dictionary = {}) -> String:
	if active_run.is_empty():
		start_new_run()

	var run_data := active_run.duplicate(true)
	for key in extra_data.keys():
		run_data[key] = extra_data[key]

	var now := Time.get_unix_time_from_system()
	run_data["updated_at"] = now
	if not run_data.has("created_at"):
		run_data["created_at"] = now
	if save_name.strip_edges().is_empty():
		save_name = str(run_data.get("run_name", "Seed %s" % active_seed))
	run_data["save_name"] = save_name

	# One stable save file per run (keyed by seed); repeated saves overwrite it,
	# so the Load screen shows one entry per game instead of every snapshot.
	var file_name := "run_%s.json" % _sanitize_file_component(str(active_seed))
	var save_path := SAVE_DIR.path_join(file_name)
	var file := FileAccess.open(save_path, FileAccess.WRITE)
	if not file:
		push_error("RunManager: failed to open save file for writing: %s" % save_path)
		return ""

	file.store_string(JSON.stringify(run_data, "\t"))
	file.close()
	active_save_path = save_path
	active_run = run_data.duplicate(true)
	run_saved.emit(save_path, active_run.duplicate(true))
	return save_path

func load_run(save_path: String) -> Dictionary:
	if save_path.is_empty():
		push_error("RunManager: empty save path")
		return {}
	if not FileAccess.file_exists(save_path):
		push_error("RunManager: missing save file %s" % save_path)
		return {}

	var file := FileAccess.open(save_path, FileAccess.READ)
	if not file:
		push_error("RunManager: failed to open save file %s" % save_path)
		return {}

	var parsed: Variant = JSON.parse_string(file.get_as_text())
	file.close()
	if typeof(parsed) != TYPE_DICTIONARY:
		push_error("RunManager: invalid save JSON in %s" % save_path)
		return {}

	active_run = parsed
	active_seed = str(active_run.get("seed", ""))
	active_save_path = save_path
	_generate_or_load_campaign(active_seed)
	active_run["campaign"] = str(active_run.get("campaign", active_campaign_manifest.get("campaign_name", DEFAULT_CAMPAIGN)))
	active_run["campaign_manifest_path"] = active_campaign_manifest_path
	active_run["campaign_dir"] = str(active_campaign_manifest.get("campaign_dir", ""))
	if not active_run.has("current_room_id") or str(active_run.get("current_room_id", "")).is_empty():
		active_run["current_room_id"] = str(active_campaign_manifest.get("start_room_id", "room_000"))
	active_run["room_count"] = int(active_campaign_manifest.get("room_count", int(active_run.get("room_count", 56))))
	if not active_run.has("max_lives"):
		active_run["max_lives"] = STARTING_LIVES
	if not active_run.has("lives_remaining"):
		active_run["lives_remaining"] = int(active_run.get("max_lives", STARTING_LIVES))
	if not active_run.has("equipment") or not (active_run["equipment"] is Array):
		active_run["equipment"] = []
	if not active_run.has("security_cards") or not (active_run["security_cards"] is Array):
		active_run["security_cards"] = []
	if not active_run.has("oxygen_pct"):
		active_run["oxygen_pct"] = 100.0
	if not active_run.has("ore_count"):
		active_run["ore_count"] = int(active_run.get("collected_ore", 0))
	if not active_run.has("jetpack_upgrades"):
		active_run["jetpack_upgrades"] = 1 if has_equipment("jetpack") else 0
	if not active_run.has("fuel_capacity_pct"):
		active_run["fuel_capacity_pct"] = BASE_FUEL_CAPACITY if int(active_run.get("jetpack_upgrades", 0)) > 0 else BASE_FUEL_CAPACITY
	active_run["fuel_capacity_pct"] = maxf(float(active_run.get("fuel_capacity_pct", BASE_FUEL_CAPACITY)), BASE_FUEL_CAPACITY)
	active_run["fuel_pct"] = clampf(float(active_run.get("fuel_pct", active_run["fuel_capacity_pct"])), 0.0, float(active_run["fuel_capacity_pct"]))
	active_run["collected_ore"] = int(active_run.get("ore_count", 0))
	run_loaded.emit(active_run.duplicate(true))
	return active_run.duplicate(true)

func list_saved_runs() -> Array[Dictionary]:
	_ensure_save_dir()
	var results: Array[Dictionary] = []
	var dir := DirAccess.open(SAVE_DIR)
	if not dir:
		return results

	# Keep only the newest save per seed (collapses legacy timestamped snapshots
	# into one entry per game).
	var newest_by_seed: Dictionary = {}
	dir.list_dir_begin()
	var file_name := dir.get_next()
	while file_name != "":
		if not dir.current_is_dir() and file_name.get_extension().to_lower() == "json":
			var save_path := SAVE_DIR.path_join(file_name)
			var data := _load_save_metadata(save_path)
			if not data.is_empty():
				var key := str(data.get("seed", save_path))
				if not newest_by_seed.has(key) or int(data.get("updated_at", 0)) > int((newest_by_seed[key] as Dictionary).get("updated_at", 0)):
					newest_by_seed[key] = data
		file_name = dir.get_next()
	dir.list_dir_end()

	for entry in newest_by_seed.values():
		results.append(entry)
	results.sort_custom(func(a: Dictionary, b: Dictionary) -> bool:
		return int(a.get("updated_at", 0)) > int(b.get("updated_at", 0))
	)
	return results

func delete_save(save_path: String) -> bool:
	if save_path.is_empty():
		return false
	if not FileAccess.file_exists(save_path):
		return false
	var abs_path := ProjectSettings.globalize_path(save_path)
	var err := DirAccess.remove_absolute(abs_path)
	if err != OK:
		push_error("RunManager: failed to delete save file %s (%s)" % [save_path, err])
		return false
	if active_save_path == save_path:
		active_save_path = ""
	return true

func get_active_seed() -> String:
	return active_seed

func get_active_seed_display() -> String:
	if active_seed.strip_edges().is_empty():
		return "Random"
	return active_seed

func get_active_run_summary() -> String:
	if active_run.is_empty():
		return "No active run"
	var lines: Array[String] = []
	lines.append("Seed: %s" % get_active_seed_display())
	lines.append("Run: %s" % str(active_run.get("run_name", "Unnamed")))
	lines.append("Campaign: %s" % str(active_run.get("campaign", DEFAULT_CAMPAIGN)))
	lines.append("Room: %s" % str(active_run.get("current_room_id", "room_000")))
	lines.append("Rooms: %s" % str(active_run.get("room_count", 56)))
	return "\n".join(lines)

func get_latest_save_path() -> String:
	return active_save_path

func get_active_campaign_manifest() -> Dictionary:
	return active_campaign_manifest.duplicate(true)

func get_campaign_manifest_path() -> String:
	return active_campaign_manifest_path

func get_current_room_id() -> String:
	if active_run.is_empty():
		return "room_000"
	return str(active_run.get("current_room_id", "room_000"))

func get_room_path(room_id: String) -> String:
	if active_campaign_manifest.is_empty():
		return ""
	var room_files: Dictionary = active_campaign_manifest.get("room_files", {})
	return str(room_files.get(room_id, ""))

func set_current_room(room_id: String, should_save: bool = true) -> void:
	if room_id.is_empty():
		return
	active_run["current_room_id"] = room_id
	active_run["updated_at"] = Time.get_unix_time_from_system()
	if should_save and not active_run.is_empty():
		save_current_run(str(active_run.get("run_name", "Run")))

func grant_equipment(item_id: String, should_save: bool = true) -> void:
	if item_id.is_empty():
		return
	if not active_run.has("equipment") or not (active_run["equipment"] is Array):
		active_run["equipment"] = []
	var equipment: Array = active_run["equipment"]
	if item_id == "jetpack":
		if not active_run.has("jetpack_upgrades"):
			active_run["jetpack_upgrades"] = 0
		active_run["jetpack_upgrades"] = int(active_run.get("jetpack_upgrades", 0)) + 1
		var capacity := BASE_FUEL_CAPACITY + maxf(float(active_run.get("jetpack_upgrades", 1)) - 1.0, 0.0) * FUEL_CAPACITY_PER_JETPACK
		active_run["fuel_capacity_pct"] = capacity
		active_run["fuel_pct"] = capacity
	if not equipment.has(item_id):
		equipment.append(item_id)
	active_run["equipment"] = equipment
	active_run["updated_at"] = Time.get_unix_time_from_system()
	if should_save and not active_run.is_empty():
		save_current_run(str(active_run.get("run_name", "Run")))

func has_equipment(item_id: String) -> bool:
	if active_run.is_empty():
		return false
	var equipment: Array = active_run.get("equipment", [])
	return equipment.has(item_id)

func grant_security_card(card_id: String, should_save: bool = true) -> void:
	if card_id.is_empty():
		return
	if not active_run.has("security_cards") or not (active_run["security_cards"] is Array):
		active_run["security_cards"] = []
	var security_cards: Array = active_run["security_cards"]
	if not security_cards.has(card_id):
		security_cards.append(card_id)
	active_run["security_cards"] = security_cards
	active_run["updated_at"] = Time.get_unix_time_from_system()
	if should_save and not active_run.is_empty():
		save_current_run(str(active_run.get("run_name", "Run")))

func has_security_card(card_id: String) -> bool:
	if active_run.is_empty():
		return false
	var security_cards: Array = active_run.get("security_cards", [])
	return security_cards.has(card_id)

# ── Lives (run-level, persist across rooms and saves) ─────────────────────────

func get_lives() -> int:
	return int(active_run.get("lives_remaining", STARTING_LIVES))

func get_max_lives() -> int:
	return int(active_run.get("max_lives", STARTING_LIVES))

# Decrement one life on death; persists immediately. Returns lives left.
func lose_life() -> int:
	if active_run.is_empty():
		return 0
	var left := maxi(get_lives() - 1, 0)
	active_run["lives_remaining"] = left
	save_current_run(str(active_run.get("run_name", "Run")))
	return left

func add_life(amount: int = 1) -> void:
	if active_run.is_empty():
		return
	active_run["lives_remaining"] = mini(get_lives() + amount, get_max_lives())
	save_current_run(str(active_run.get("run_name", "Run")))

# ── Resupply & resource tracking ──────────────────────────────────────────────

func save_checkpoint(room_id: String) -> void:
	if active_run.is_empty():
		return
	active_run["checkpoint_room_id"] = room_id
	active_run["checkpoint"] = room_id
	active_run["fuel_pct"] = float(active_run.get("fuel_capacity_pct", BASE_FUEL_CAPACITY))
	active_run["battery_pct"] = 100.0
	active_run["updated_at"] = Time.get_unix_time_from_system()
	save_current_run(str(active_run.get("run_name", "Run")))

func get_checkpoint_room_id() -> String:
	return str(active_run.get("checkpoint_room_id", str(active_campaign_manifest.get("start_room_id", "room_000"))))

func update_fuel(pct: float) -> void:
	if active_run.is_empty():
		return
	var capacity := float(active_run.get("fuel_capacity_pct", BASE_FUEL_CAPACITY))
	active_run["fuel_pct"] = clampf(pct, 0.0, capacity)

func update_battery(pct: float) -> void:
	if active_run.is_empty():
		return
	active_run["battery_pct"] = clampf(pct, 0.0, 100.0)

func update_oxygen(pct: float) -> void:
	if active_run.is_empty():
		return
	active_run["oxygen_pct"] = clampf(pct, 0.0, 100.0)

func update_ore(amount: int) -> void:
	if active_run.is_empty():
		return
	active_run["ore_count"] = maxi(int(amount), 0)
	active_run["collected_ore"] = active_run["ore_count"]

func add_ore(amount: int) -> void:
	if amount == 0:
		return
	update_ore(get_ore_count() + amount)

func get_fuel_pct() -> float:
	return float(active_run.get("fuel_pct", 100.0))

func get_fuel_capacity_pct() -> float:
	return float(active_run.get("fuel_capacity_pct", BASE_FUEL_CAPACITY))

func get_battery_pct() -> float:
	return float(active_run.get("battery_pct", 100.0))

func get_oxygen_pct() -> float:
	return float(active_run.get("oxygen_pct", 100.0))

func get_ore_count() -> int:
	return int(active_run.get("ore_count", active_run.get("collected_ore", 0)))

func ensure_campaign_for_active_seed() -> Dictionary:
	if active_seed.strip_edges().is_empty():
		return {}
	if active_campaign_manifest.is_empty() or active_campaign_manifest_path.is_empty():
		_generate_or_load_campaign(active_seed)
	return active_campaign_manifest.duplicate(true)

func _load_save_metadata(save_path: String) -> Dictionary:
	var file := FileAccess.open(save_path, FileAccess.READ)
	if not file:
		return {}
	var parsed: Variant = JSON.parse_string(file.get_as_text())
	file.close()
	if typeof(parsed) != TYPE_DICTIONARY:
		return {}

	var data: Dictionary = parsed
	data["file_path"] = save_path
	data["file_name"] = save_path.get_file()
	data["display_name"] = str(data.get("save_name", data.get("run_name", save_path.get_file())))
	data["seed"] = str(data.get("seed", ""))
	data["updated_at"] = int(data.get("updated_at", FileAccess.get_modified_time(save_path)))
	data["created_at"] = int(data.get("created_at", data["updated_at"]))
	data["room_count"] = int(data.get("room_count", 56))
	data["current_room_id"] = str(data.get("current_room_id", "room_000"))
	return data

func _sanitize_file_component(value: String) -> String:
	var result := ""
	for character in value.to_lower():
		if character.is_valid_int() or (character >= "a" and character <= "z") or character == "_" or character == "-":
			result += character
		else:
			result += "_"
	return result.strip_edges().replace(" ", "_")

func generate_seed() -> String:
	var rng := RandomNumberGenerator.new()
	rng.randomize()
	return "%08d" % int(rng.randi() % 100000000)

const RUN_ADJECTIVES := [
	"Rusty", "Iron", "Silent", "Crimson", "Hollow", "Frozen", "Drifting",
	"Broken", "Lost", "Ashen", "Cobalt", "Derelict", "Forgotten", "Distant",
	"Shattered", "Pale", "Vacant", "Buried", "Wandering", "Endless",
]
const RUN_NOUNS := [
	"Vesper", "Halcyon", "Expanse", "Drift", "Reach", "Vault", "Relay",
	"Beacon", "Quarry", "Furnace", "Spire", "Verge", "Cradle", "Anchorage",
	"Trench", "Hollow", "Bastion", "Threshold", "Lode", "Wake",
]

# Deterministic codename from the seed: same seed → same name.
func _generate_run_name(seed: String) -> String:
	var rng := RandomNumberGenerator.new()
	rng.seed = abs(str(seed).hash())
	var adj: String = RUN_ADJECTIVES[rng.randi() % RUN_ADJECTIVES.size()]
	var noun: String = RUN_NOUNS[rng.randi() % RUN_NOUNS.size()]
	return "%s %s" % [adj, noun]

func _generate_or_load_campaign(seed: String) -> void:
	if seed.strip_edges().is_empty():
		active_campaign_manifest = {}
		active_campaign_manifest_path = ""
		return
	var generator := ASTEROID_CAMPAIGN_GENERATOR_SCRIPT.new()
	active_campaign_manifest = generator.generate_campaign(seed, 56)
	active_campaign_manifest_path = str(active_campaign_manifest.get("manifest_path", ""))
