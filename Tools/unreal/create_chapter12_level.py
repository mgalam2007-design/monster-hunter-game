"""Generate Chapter 12's Shadow Accord negotiation encounter in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter12/L_CH12_ShadowAccord"


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


def build_chapter_12():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter12")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    monster_class = native_class("HunterMonsterBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH12_Shadow_Accord_Conclave")
    set_property(arena, "arena_half_width", 1900.0)
    set_property(arena, "arena_half_depth", 1900.0)
    set_property(arena, "wall_height", 720.0)
    spawn(unreal.PlayerStart, (0, -1500, 110), "CH12_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1050), "CH12_Sun",
                unreal.Rotator(-38.0, -28.0, 0.0))
    set_property(sun, "intensity", 4.0)
    sky = spawn(unreal.SkyLight, (0, 0, 780), "CH12_SkyLight")
    set_property(sky, "intensity", 0.72)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH12_Shadow_Accord_Mist")
    set_property(fog, "fog_density", 0.025)

    contact = spawn(trigger_class, (0, -300, 100), "CH12_Shadow_Faction_Contact")
    set_property(contact, "quest_event_tag", unreal.Name("ShadowContactReached"))
    set_property(contact, "event_amount", 1)
    set_property(contact, "trigger_once", True)
    set_property(contact, "destroy_after_trigger", False)
    set_property(contact, "requires_interaction", False)

    sentinel_points = [(-850, 350, 110), (0, 650, 110), (850, 350, 110)]
    for index, point in enumerate(sentinel_points, start=1):
        sentinel = spawn(monster_class, point, "CH12_Shadow_Sentinel_%02d" % index)
        set_property(sentinel, "defeat_quest_event_tag", unreal.Name("ShadowSentinelDefeated"))

    pact = spawn(trigger_class, (0, 1200, 100), "CH12_Shadow_Pact_Altar")
    set_property(pact, "quest_event_tag", unreal.Name("ShadowPactResolved"))
    set_property(pact, "event_amount", 1)
    set_property(pact, "trigger_once", True)
    set_property(pact, "destroy_after_trigger", False)
    set_property(pact, "requires_interaction", True)
    set_property(pact, "interaction_prompt_text", unreal.Text("E  RESOLVE SHADOW PACT"))
    set_property(pact, "completion_prompt_text", unreal.Text("SHADOW ACCORD SEALED"))

    exit_actor = spawn(exit_class, (0, 1580, 100), "CH12_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 900)
    set_property(exit_actor, "completion_currency_reward", 450)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter13/L_CH13_SiegeOfTheNorth"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 270), "CH12_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(40.0, 40.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 12 with faction contact, three sentinels and pact altar.")
    unreal.log_warning("HUNTER: Build NavMesh and test enemy AI, pact interaction, quest gating and campaign travel in Unreal Editor.")


build_chapter_12()
