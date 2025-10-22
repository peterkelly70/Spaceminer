@tool
extends Control

signal cutscene_selected(cutscene_data, category_id)

const CutsceneRegistry = preload("res://addons/cutscene_editor/dock/cutscene_registry.gd")

var registry = CutsceneRegistry.new()
var current_category = ""
var current_cutscene = null

# Node references
@onready var category_list = $VBoxContainer/HSplitContainer/LeftPanel/VBoxContainer/CategoryList
@onready var cutscene_list = $VBoxContainer/HSplitContainer/LeftPanel/VBoxContainer/CutsceneList
@onready var add_category_button = $VBoxContainer/HSplitContainer/LeftPanel/VBoxContainer/ButtonsContainer/AddCategoryButton
@onready var edit_category_button = $VBoxContainer/HSplitContainer/LeftPanel/VBoxContainer/ButtonsContainer/EditCategoryButton
@onready var delete_category_button = $VBoxContainer/HSplitContainer/LeftPanel/VBoxContainer/ButtonsContainer/DeleteCategoryButton
@onready var add_cutscene_button = $VBoxContainer/HSplitContainer/LeftPanel/VBoxContainer/ButtonsContainer2/AddCutsceneButton
@onready var delete_cutscene_button = $VBoxContainer/HSplitContainer/LeftPanel/VBoxContainer/ButtonsContainer2/DeleteCutsceneButton

# Dialogs
var category_dialog
var cutscene_dialog
var confirm_dialog

func _ready():
	# Connect signals
	category_list.item_selected.connect(_on_category_selected)
	cutscene_list.item_selected.connect(_on_cutscene_selected)
	add_category_button.pressed.connect(_on_add_category_pressed)
	edit_category_button.pressed.connect(_on_edit_category_pressed)
	delete_category_button.pressed.connect(_on_delete_category_pressed)
	add_cutscene_button.pressed.connect(_on_add_cutscene_pressed)
	delete_cutscene_button.pressed.connect(_on_delete_cutscene_pressed)
	
	# Create dialogs
	_create_dialogs()
	
	# Load registry
	_load_registry()

func _create_dialogs():
	# Category dialog
	category_dialog = ConfirmationDialog.new()
	category_dialog.title = "Category"
	category_dialog.size = Vector2(400, 150)
	
	var category_vbox = VBoxContainer.new()
	category_vbox.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT, Control.PRESET_MODE_MINSIZE, 10)
	
	var category_label = Label.new()
	category_label.text = "Category Name:"
	category_vbox.add_child(category_label)
	
	var category_edit = LineEdit.new()
	category_edit.name = "CategoryEdit"
	category_edit.placeholder_text = "Enter category name"
	category_vbox.add_child(category_edit)
	
	category_dialog.add_child(category_vbox)
	category_dialog.confirmed.connect(_on_category_dialog_confirmed)
	add_child(category_dialog)
	
	# Cutscene dialog
	cutscene_dialog = ConfirmationDialog.new()
	cutscene_dialog.title = "Cutscene"
	cutscene_dialog.size = Vector2(400, 150)
	
	var cutscene_vbox = VBoxContainer.new()
	cutscene_vbox.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT, Control.PRESET_MODE_MINSIZE, 10)
	
	var cutscene_label = Label.new()
	cutscene_label.text = "Cutscene Name:"
	cutscene_vbox.add_child(cutscene_label)
	
	var cutscene_edit = LineEdit.new()
	cutscene_edit.name = "CutsceneEdit"
	cutscene_edit.placeholder_text = "Enter cutscene name"
	cutscene_vbox.add_child(cutscene_edit)
	
	cutscene_dialog.add_child(cutscene_vbox)
	cutscene_dialog.confirmed.connect(_on_cutscene_dialog_confirmed)
	add_child(cutscene_dialog)
	
	# Confirm dialog
	confirm_dialog = ConfirmationDialog.new()
	confirm_dialog.title = "Confirm"
	confirm_dialog.size = Vector2(400, 100)
	add_child(confirm_dialog)

func _load_registry():
	registry.load_registry()
	
	# Update category list
	_update_category_list()

func _update_category_list():
	category_list.clear()
	
	for category in registry.get_categories():
		category_list.add_item(category.name, null, true)
		category_list.set_item_metadata(category_list.get_item_count() - 1, category.id)
	
	# Disable buttons if no categories
	edit_category_button.disabled = category_list.get_item_count() == 0
	delete_category_button.disabled = category_list.get_item_count() == 0
	add_cutscene_button.disabled = category_list.get_item_count() == 0

func _update_cutscene_list():
	cutscene_list.clear()
	
	if current_category == "":
		return
	
	var cutscenes = registry.get_cutscenes_in_category(current_category)
	
	for cutscene in cutscenes:
		cutscene_list.add_item(cutscene.name, null, true)
		cutscene_list.set_item_metadata(cutscene_list.get_item_count() - 1, cutscene)
	
	# Disable delete button if no cutscenes
	delete_cutscene_button.disabled = cutscene_list.get_item_count() == 0

func _on_category_selected(index):
	var category_id = category_list.get_item_metadata(index)
	current_category = category_id
	
	# Update cutscene list
	_update_cutscene_list()
	
	# Clear current cutscene selection when changing categories
	current_cutscene = null
	emit_signal("cutscene_selected", null, current_category)

func _on_cutscene_selected(index):
	current_cutscene = cutscene_list.get_item_metadata(index)
	
	# Emit signal to update inspector
	emit_signal("cutscene_selected", current_cutscene, current_category)

func _on_add_category_pressed():
	category_dialog.title = "Add Category"
	category_dialog.get_node("CategoryEdit").text = ""
	category_dialog.popup_centered()

func _on_edit_category_pressed():
	if category_list.get_selected_items().size() == 0:
		return
	
	var index = category_list.get_selected_items()[0]
	var category_id = category_list.get_item_metadata(index)
	var category = registry.get_category_by_id(category_id)
	
	category_dialog.title = "Edit Category"
	category_dialog.get_node("CategoryEdit").text = category.name
	category_dialog.popup_centered()

func _on_delete_category_pressed():
	if category_list.get_selected_items().size() == 0:
		return
	
	var index = category_list.get_selected_items()[0]
	var category_id = category_list.get_item_metadata(index)
	var category = registry.get_category_by_id(category_id)
	
	confirm_dialog.title = "Delete Category"
	confirm_dialog.dialog_text = "Are you sure you want to delete the category '" + category.name + "'? This will delete all cutscenes in this category."
	
	# Disconnect previous connections to avoid multiple connections
	if confirm_dialog.confirmed.is_connected(_on_delete_category_confirmed):
		confirm_dialog.confirmed.disconnect(_on_delete_category_confirmed)
	
	confirm_dialog.confirmed.connect(_on_delete_category_confirmed.bind(category_id))
	confirm_dialog.popup_centered()

func _on_add_cutscene_pressed():
	if current_category == "":
		return
	
	cutscene_dialog.title = "Add Cutscene"
	cutscene_dialog.get_node("CutsceneEdit").text = ""
	cutscene_dialog.popup_centered()

func _on_delete_cutscene_pressed():
	if cutscene_list.get_selected_items().size() == 0:
		return
	
	var index = cutscene_list.get_selected_items()[0]
	var cutscene = cutscene_list.get_item_metadata(index)
	
	confirm_dialog.title = "Delete Cutscene"
	confirm_dialog.dialog_text = "Are you sure you want to delete the cutscene '" + cutscene.name + "'?"
	
	# Disconnect previous connections to avoid multiple connections
	if confirm_dialog.confirmed.is_connected(_on_delete_cutscene_confirmed):
		confirm_dialog.confirmed.disconnect(_on_delete_cutscene_confirmed)
	
	confirm_dialog.confirmed.connect(_on_delete_cutscene_confirmed.bind(cutscene.id))
	confirm_dialog.popup_centered()

func _on_category_dialog_confirmed():
	var category_name = category_dialog.get_node("CategoryEdit").text
	
	if category_name.strip_edges() == "":
		return
	
	if category_dialog.title == "Add Category":
		# Generate a unique ID for the new category
		var category_id = category_name.to_lower().replace(" ", "_")
		var category_data = {
			"id": category_id,
			"name": category_name,
			"path": "res://assets/cutscenes/" + category_name + "/",
			"description": "Cutscenes for " + category_name,
			"cutscenes": []
		}
		registry.add_category(category_data)
	else:
		var index = category_list.get_selected_items()[0]
		var category_id = category_list.get_item_metadata(index)
		var category = registry.get_category_by_id(category_id)
		category.name = category_name
		registry.add_category(category) # This updates the existing category
	
	# Update category list
	_update_category_list()

func _on_cutscene_dialog_confirmed():
	var cutscene_name = cutscene_dialog.get_node("CutsceneEdit").text
	
	if cutscene_name.strip_edges() == "":
		return
	
	if cutscene_dialog.title == "Add Cutscene":
		# Generate a unique ID for the new cutscene
		var cutscene_id = cutscene_name.to_lower().replace(" ", "_")
		var cutscene_data = {
			"id": cutscene_id,
			"name": cutscene_name,
			"file": cutscene_id + ".json",
			"typing_speed": 0.03,
			"background_color": "#000000cc",
			"frames": []
		}
		
		var category = registry.get_category_by_id(current_category)
		category.cutscenes.append(cutscene_data)
		registry.add_category(category) # This updates the existing category
	
	# Update cutscene list
	_update_cutscene_list()

func _on_delete_category_confirmed(category_id):
	registry.remove_category(category_id)
	
	# Reset current category
	current_category = ""
	
	# Update category list
	_update_category_list()
	
	# Clear cutscene list
	cutscene_list.clear()
	
	# Clear current cutscene selection
	current_cutscene = null
	emit_signal("cutscene_selected", null, "")
	
	# Disconnect signal
	confirm_dialog.confirmed.disconnect(_on_delete_category_confirmed)

func _on_delete_cutscene_confirmed(cutscene_id):
	registry.remove_cutscene_from_category(current_category, cutscene_id)
	
	# Update cutscene list
	_update_cutscene_list()
	
	# Clear current cutscene selection
	current_cutscene = null
	emit_signal("cutscene_selected", null, current_category)
	
	# Disconnect signal
	confirm_dialog.confirmed.disconnect(_on_delete_cutscene_confirmed)

# This function can be called from the inspector panel to update the registry
func update_cutscene(cutscene_data, category_id):
	if category_id == current_category:
		registry.add_cutscene_to_category(category_id, cutscene_data)
		_update_cutscene_list()
