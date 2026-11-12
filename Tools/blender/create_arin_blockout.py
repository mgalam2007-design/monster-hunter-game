"""
HUNTER 2.0 — Arin Vale stylized blockout generator
Run with Blender 4.x in background or from the Scripting workspace:
    blender --background --python Tools/blender/create_arin_blockout.py

Creates a proportioned, modular placeholder character and exports an FBX.
This is a modelling BLOCKOUT, not a finished sculpt, textured hero asset,
facial rig, cloth simulation, or production animation.
"""
import bpy
import math
from mathutils import Vector

# Reset scene
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
for datablocks in (bpy.data.materials,):
    for datablock in list(datablocks):
        if datablock.users == 0:
            datablocks.remove(datablock)

def material(name, color, metallic=0.0, roughness=0.55, emission=0.0):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1.0)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Metallic"].default_value = metallic
    bsdf.inputs["Roughness"].default_value = roughness
    if emission:
        bsdf.inputs["Emission Color"].default_value = (*color, 1.0)
        bsdf.inputs["Emission Strength"].default_value = emission
    return mat

MAT_CLOTH = material("Arin | Charcoal fabric", (0.018, 0.025, 0.045), roughness=0.82)
MAT_ARMOR = material("Arin | Gunmetal", (0.075, 0.095, 0.13), metallic=0.72, roughness=0.3)
MAT_SKIN = material("Arin | Skin blockout", (0.55, 0.34, 0.25), roughness=0.65)
MAT_HAIR = material("Arin | Midnight hair", (0.012, 0.018, 0.03), roughness=0.42)
MAT_BLUE = material("Arin | Electric blue accent", (0.015, 0.22, 1.0), metallic=0.25, emission=3.0)
MAT_STEEL = material("Arin | Blade steel", (0.32, 0.42, 0.55), metallic=0.88, roughness=0.22)

def smooth(obj, mat, bevel=0.0):
    obj.data.materials.append(mat)
    if obj.type == 'MESH':
        for p in obj.data.polygons:
            p.use_smooth = True
        if bevel:
            mod = obj.modifiers.new("Soft manufactured edges", 'BEVEL')
            mod.width = bevel
            mod.segments = 2
    return obj

def uv(name, loc, scale, mat, segments=24, rings=16):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    return smooth(obj, mat)

def cube(name, loc, scale, mat, bevel=0.06):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    return smooth(obj, mat, bevel)

def capsule_between(name, a, b, radius, mat):
    mid = (Vector(a) + Vector(b)) * 0.5
    direction = Vector(b) - Vector(a)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=20, ring_count=12, location=mid)
    obj = bpy.context.object
    obj.name = name
    obj.scale = (radius, radius, direction.length * 0.58)
    obj.rotation_mode = 'QUATERNION'
    obj.rotation_quaternion = direction.to_track_quat('Z', 'Y')
    return smooth(obj, mat)

# Character stands facing -Y; height approximately 1.87 m.
# Torso and coat silhouette
uv("Arin_Body_Torso", (0, 0, 1.23), (0.29, 0.17, 0.39), MAT_CLOTH)
cube("Arin_Coat_LeftPanel", (-0.16, 0.035, 0.68), (0.19, 0.10, 0.72), MAT_CLOTH)
cube("Arin_Coat_RightPanel", (0.16, 0.035, 0.68), (0.19, 0.10, 0.72), MAT_CLOTH)
uv("Arin_Neck", (0, 0, 1.63), (0.09, 0.09, 0.13), MAT_SKIN)
uv("Arin_Head", (0, -0.015, 1.77), (0.145, 0.135, 0.19), MAT_SKIN)
uv("Arin_Hair_Cap", (0, 0.005, 1.89), (0.16, 0.145, 0.105), MAT_HAIR)
# Hair fringe tufts
for i, x in enumerate((-0.095, -0.035, 0.035, 0.095)):
    uv("Arin_Hair_Fringe_%02d" % i, (x, -0.105, 1.82 + (0.015 if i % 2 else 0)), (0.045, 0.07, 0.11), MAT_HAIR, 16, 12)
# Chest armor and blue chest inlay
uv("Arin_ChestArmor", (0, -0.145, 1.34), (0.25, 0.055, 0.22), MAT_ARMOR)
cube("Arin_ChestBlueInlay", (0, -0.202, 1.36), (0.025, 0.012, 0.16), MAT_BLUE, 0.008)
# Legs, boots, arms and gauntlets
for side, label in ((-1, "L"), (1, "R")):
    x = side * 0.14
    capsule_between("Arin_Thigh_" + label, (x, 0, 0.65), (x, 0, 0.98), 0.105, MAT_CLOTH)
    capsule_between("Arin_Shin_" + label, (x, 0, 0.18), (x, 0, 0.61), 0.075, MAT_ARMOR)
    cube("Arin_Boot_" + label, (x, -0.055, 0.10), (0.16, 0.29, 0.18), MAT_CLOTH)
    shoulder = (side * 0.34, 0, 1.48)
    elbow = (side * 0.43, -0.005, 1.18)
    wrist = (side * 0.43, -0.045, 0.96)
    capsule_between("Arin_UpperArm_" + label, shoulder, elbow, 0.085, MAT_CLOTH)
    capsule_between("Arin_Forearm_" + label, elbow, wrist, 0.065, MAT_ARMOR)
    uv("Arin_Glove_" + label, (side * 0.43, -0.05, 0.91), (0.07, 0.07, 0.09), MAT_CLOTH)
    uv("Arin_ShoulderGuard_" + label, shoulder, (0.13, 0.14, 0.10), MAT_ARMOR)

# Signature single-edged sword held at character's right side
cube("Arin_Sword_Grip", (0.53, -0.07, 0.90), (0.055, 0.055, 0.22), MAT_CLOTH)
cube("Arin_Sword_Guard", (0.53, -0.07, 1.02), (0.23, 0.055, 0.045), MAT_STEEL)
blade = cube("Arin_Sword_Blade_Blockout", (0.53, -0.07, 1.34), (0.095, 0.035, 0.60), MAT_STEEL, 0.015)
blade.rotation_euler[1] = math.radians(-8)
# Small emissive sword fuller
cube("Arin_Sword_BlueFuller", (0.53, -0.091, 1.34), (0.012, 0.008, 0.48), MAT_BLUE, 0.003)

# Organize generated objects
root = bpy.data.collections.new("ARIN_VALE_BLOCKOUT")
bpy.context.scene.collection.children.link(root)
for obj in list(bpy.context.scene.objects):
    for col in list(obj.users_collection):
        col.objects.unlink(obj)
    root.objects.link(obj)

# Set a useful origin and save/export beside the script's working directory.
bpy.ops.object.select_all(action='SELECT')
bpy.context.view_layer.objects.active = bpy.context.selected_objects[0]
bpy.ops.wm.save_as_mainfile(filepath="//ArinVale_Blockout.blend")
bpy.ops.export_scene.fbx(
    filepath="//ArinVale_Blockout.fbx",
    use_selection=True,
    apply_unit_scale=True,
    bake_space_transform=False,
    object_types={'MESH'},
    mesh_smooth_type='FACE',
    add_leaf_bones=False
)
print("HUNTER 2.0: Arin Vale blockout generated. This is a static modelling blockout; rigging and animation are separate production steps.")
