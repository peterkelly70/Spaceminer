# PuppetLab 0.1.0 — Godot 4.4

## Install
1. Copy `addons/puppetlab` into your project.
2. In Project Settings → Plugins, enable **PuppetLab**.

## Prepare art
- Export a folder of transparent PNGs. File names should include part keys:
  - `head.png`, `torso.png`, `left_arm.png`, `right_arm.png`, `left_leg.png`, `right_leg.png`, `weapon.png`, `cloak.png`.
- Any extra PNGs are added as accessories under the torso bone.

## Build a puppet
- Open the **PuppetLab** dock on the right, pick your source folder, hit **Build Puppet**.
- A `Puppet2D` node is created in your current scene with a `Skeleton2D`, `Bone2D` limbs, and `Sprite2D` parts.
- Toggle **Pixel Snap** for crisp retro visuals.

## Animate
- PuppetLab auto-generates an `idle` loop with subtle per-bone rotations.
- Use the built-in `AnimationPlayer` on the puppet to create new clips or adjust curves.

## Tips
- Keep silhouettes chunky and readable at game scale.
- For mesh-deform cloth later, replace a `Sprite2D` with a `Polygon2D`, add bones in the same hierarchy, and paint weights in the editor.

## Roadmap
- Aseprite JSON import with automatic pivot capture
- On-canvas bone gizmos and part anchors
- Walk/attack preset generators
- Batch importer and re-linker
