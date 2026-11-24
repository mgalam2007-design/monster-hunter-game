"""Generate Chapter 10's Hollow District guardian mission in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter10/L_CH10_HollowDistrict"


def native_class(name):
    cls = unreal.load_class(None, "/Script/HunterGame." + name)
    if not cls:
        raise RuntimeError("Missing native class " + name + "; compile HunterGame first.")
    return cls


def spawn(cls, location, label, rotation=None):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        cls, unreal.Vector(*location), rotation or unreal.Rotator(0.0, 0.0, 0.0))
    if not actor:
        raise RuntimeError("Could not spawn " + label)
    actor.set_actor_label(label)
    return actor


def set_property(actor, name, value):
    try:
        actor.set_editor_property(name, value)
    except Exception as error:
        raise RuntimeError("Could not set " + name + " on " +
                           actor.get_name() + ": " + str(error))


def build_chapter_10():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter10")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    boss_encounter_class = native_class("HunterBossEncounterActor")
    boss_class = native_class("HunterBossBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH10_Hollow_District_Core")
    set_property(arena, "arena_half_width", 1950.0)
    set_property(arena, "arena_half_depth", 1950.0)
    set_property(arena, "wall_height", 760.0)
    spawn(unreal.PlayerStart, (0, -1550, 110), "CH10_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1100), "CH10_Sun",
                unreal.Rotator(-35.0, -25.0, 0.0))
    set_property(sun, "intensity", 4.2)
    sky = spawn(unreal.SkyLight, (0, 0, 800), "CH10_SkyLight")
    set_property(sky, "intensity", 0.7)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH10_Hollow_Shadow_Fog")
    set_property(fog, "fog_density", 0.028)

    core = spawn(trigger_class, (0, -350, 100), "CH10_Hollow_District_Core")
    set_property(core, "quest_event_tag", unreal.Name("HollowDistrictCoreReached"))
    set_property(core, "event_amount", 1)
    set_property(core, "trigger_once", True)
    set_property(core, "destroy_after_trigger", False)
    set_property(core, "requires_interaction", False)

    anchor_points = [(-1000, 800, 100), (0, 1120, 100), (1000, 800, 100)]
    for index, point in enumerate(anchor_points, start=1):
        anchor = spawn(trigger_class, point, "CH10_Shadow_Anchor_%02d" % index)
        set_property(anchor, "quest_event_tag", unreal.Name("ShadowAnchorDestroyed"))
        set_property(anchor, "event_amount", 1)
        set_property(anchor, "trigger_once", True)
        set_property(anchor, "destroy_after_trigger", False)
        set_property(anchor, "requires_interaction", True)
        set_property(anchor, "interaction_prompt_text", unreal.Text("E  DESTROY SHADOW ANCHOR"))
        set_property(anchor, "completion_prompt_text", unreal.Text("SHADOW ANCHOR DESTROYED"))

    encounter = spawn(boss_encounter_class, (0, 350, 100), "CH10_HOLLOW_WARDEN_ENCOUNTER")
    set_property(encounter, "boss_class", boss_class)
    set_property(encounter, "boss_defeat_quest_event_tag", unreal.Name("HollowWardenDefeated"))
    set_property(encounter, "activation_radius", 560.0)
    set_property(encounter, "boss_spawn_offset",
                 unreal.Transform(location=unreal.Vector(600.0, 0.0, 0.0)))

    exit_actor = spawn(exit_class, (0, 1600, 100), "CH10_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 750)
    set_property(exit_actor, "completion_currency_reward", 375)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 270), "CH10_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(41.0, 41.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 10 with core discovery, three shadow anchors and Hollow Warden encounter.")
    unreal.log_warning("HUNTER: Build NavMesh and test boss spawn, anchor interactions, quest gating and chapter exit in Unreal Editor.")


build_chapter_10()
