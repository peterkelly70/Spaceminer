@tool
extends Node

static func path_key(p):
    var file_name = p.get_file()
    file_name = file_name.get_basename()
    var f = String(file_name).to_lower()
    # Normalize common synonyms
    f = f.replace(" ", "_")
    if f.begins_with("larm") or f.contains("left_arm"):
        return "left_arm"
    if f.begins_with("rarm") or f.contains("right_arm"):
        return "right_arm"
    if f.contains("left_leg") or f.begins_with("lleg"):
        return "left_leg"
    if f.contains("right_leg") or f.begins_with("rleg"):
        return "right_leg"
    if f.contains("weapon") or f.contains("sword") or f.contains("dagger"):
        return "weapon"
    if f.contains("cloak") or f.contains("cape"):
        return "cloak"
    if f.contains("head") or f.contains("helm"):
        return "head"
    if f.contains("torso") or f.contains("chest") or f.contains("body"):
        return "torso"
    return f

static func folder_to_puppet_name(folder):
    var s = String(folder).strip_edges()
    if s.begins_with("res://"):
        s = s.substr(6)
    var parts = s.split("/")
    if parts.size() > 0:
        return parts[parts.size() - 1]
    return "Puppet"
