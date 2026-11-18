"""Generate Chapter 2's survivor-district greybox in Unreal Editor.

Run after compiling HunterGame via Tools > Execute Python Script. This creates
a real level asset with the authored CH02 quest events and gated chapter exit.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter02/L_CH02_MissingSurvivors"


def native_class(name):
    actor_class = unreal.load_class(None, "/Script/HunterGame." + name)
    if not actor_class:
        raise RuntimeError("Missing native class " + name + "; compile HunterGame first.")
    return actor_class


def spawn(actor_class, location, label, rotation=None):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        actor_class, unreal.Vector(*location),
        rotation or unreal.Rotator(0.0, 0.0, 0.0))
    if not actor:
        raise RuntimeError("Could not spawn " + label)
    actor.set_actor_label(label)
    return actor


def set_property(actor, name, value):
    try:
        actor.set_editor_property(name, value)
    except Exception as error:
        raise RuntimeError("Could not configure " + name + " on " +
                           actor.get_name() + ": " + str(error))


def add_quest_trigger(trigger_class, location, label, event_tag, scale=(1.0, 1.0, 1.0)):
    trigger = spawn(trigger_class, location, label)
    set_property(trigger, "quest_event_tag", unreal.Name(event_tag))
    set_property(trigger, "event_amount", 1)
    set_property(trigger, "trigger_once", True)
    set_property(trigger, "destroy_after_trigger", False)
    set_property(trigger, "requires_interaction", event_tag == "SurvivorRescued")
    if event_tag == "SurvivorRescued":
        set_property(trigger, "interaction_prompt_text",
                     unreal.Text("E  RESCUE CIVILIAN"))
    trigger.set_actor_scale3d(unreal.Vector(*scale))
    return trigger


def add_civilian_blockout(point, index):
    """Place a simple visible civilian silhouette beside each rescue marker."""
    cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    sphere_mesh = unreal.load_asset("/Engine/BasicShapes/Sphere.Sphere")
    if not cube_mesh or not sphere_mesh:
        raise RuntimeError("Unreal basic cube/sphere meshes are unavailable.")

    x, y, z = point
    body = spawn(unreal.StaticMeshActor, (x, y + 170, z + 95),
                 "CH02_Civilian_%02d_Body" % index)
    body.static_mesh_component.set_static_mesh(cube_mesh)
    body.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    body.set_actor_scale3d(unreal.Vector(0.38, 0.28, 0.72))

    head = spawn(unreal.StaticMeshActor, (x, y + 170, z + 205),
                 "CH02_Civilian_%02d_Head" % index)
    head.static_mesh_component.set_static_mesh(sphere_mesh)
    head.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    head.set_actor_scale3d(unreal.Vector(0.25, 0.25, 0.25))

    for side, offset in (("Left", -32), ("Right", 32)):
        arm = spawn(unreal.StaticMeshActor, (x + offset, y + 170, z + 115),
                    "CH02_Civilian_%02d_%sArm" % (index, side))
        arm.static_mesh_component.set_static_mesh(cube_mesh)
        arm.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        arm.set_actor_scale3d(unreal.Vector(0.12, 0.14, 0.48))

    for side, offset in (("Left", -16), ("Right", 16)):
        leg = spawn(unreal.StaticMeshActor, (x + offset, y + 170, z + 25),
                    "CH02_Civilian_%02d_%sLeg" % (index, side))
        leg.static_mesh_component.set_static_mesh(cube_mesh)
        leg.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        leg.set_actor_scale3d(unreal.Vector(0.14, 0.16, 0.42))


def build_chapter_two():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter02")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    native_class("ArinValeCharacter")  # Spawned by AHunterGameMode at PlayerStart.
    trigger_class = native_class("HunterQuestTriggerActor")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH02_SurvivorDistrict_Greybox")
    set_property(arena, "arena_half_width", 1800.0)
    set_property(arena, "arena_half_depth", 1800.0)
    set_property(arena, "wall_height", 600.0)

    spawn(unreal.PlayerStart, (0, -1450, 110), "CH02_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 900), "CH02_Sun",
                unreal.Rotator(-42.0, -32.0, 0.0))
    set_property(sun, "intensity", 8.0)
    sky = spawn(unreal.SkyLight, (0, 0, 700), "CH02_SkyLight")
    set_property(sky, "intensity", 1.2)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH02_DistrictFog")
    set_property(fog, "fog_density", 0.018)

    # Chapter 2 requires one district-location event and three separate rescues.
    add_quest_trigger(trigger_class, (0, -700, 100),
                      "CH02_Locate_Survivor_District",
                      "SurvivorDistrictLocated", (1.8, 1.8, 1.2))
    rescue_points = [
        (-900, 250, 100),
        (0, 850, 100),
        (900, 250, 100),
    ]
    for index, point in enumerate(rescue_points, start=1):
        add_civilian_blockout(point, index)
        add_quest_trigger(trigger_class, point,
                          "CH02_Rescue_Civilian_%02d" % index,
                          "SurvivorRescued", (1.2, 1.2, 1.2))

    chapter_exit = spawn(exit_class, (0, 1500, 100),
                         "CH02_Chapter_Exit")
    set_property(chapter_exit, "completion_experience_reward", 150)
    set_property(chapter_exit, "completion_currency_reward", 75)
    set_property(chapter_exit, "save_slot_name", "HunterCampaign")

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 250),
                "CH02_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(38.0, 38.0, 7.0))

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 2 survivor-district greybox with four CH02 quest-event triggers.")
    unreal.log_warning("HUNTER: Build NavMesh and play-test all rescue volumes and the gated exit. This generator has not been executed in Unreal Editor in this environment.")


build_chapter_two()
