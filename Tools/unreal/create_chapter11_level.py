"""Generate Chapter 11's Echoes of the Fallen mission in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter11/L_CH11_EchoesOfTheFallen"


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


def build_chapter_11():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter11")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    boss_encounter_class = native_class("HunterBossEncounterActor")
    boss_class = native_class("HunterBossBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH11_Expedition_Memory_Sanctum")
    set_property(arena, "arena_half_width", 1900.0)
    set_property(arena, "arena_half_depth", 1900.0)
    set_property(arena, "wall_height", 720.0)
    spawn(unreal.PlayerStart, (0, -1500, 110), "CH11_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1050), "CH11_Sun",
                unreal.Rotator(-42.0, -30.0, 0.0))
    set_property(sun, "intensity", 4.4)
    sky = spawn(unreal.SkyLight, (0, 0, 780), "CH11_SkyLight")
    set_property(sky, "intensity", 0.78)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH11_Memory_Mist")
    set_property(fog, "fog_density", 0.023)

    memory_points = [(-900, 750, 100), (0, 1050, 100), (900, 750, 100)]
    for index, point in enumerate(memory_points, start=1):
        memory = spawn(trigger_class, point, "CH11_Expedition_Memory_%02d" % index)
        set_property(memory, "quest_event_tag", unreal.Name("ExpeditionMemoryRecovered"))
        set_property(memory, "event_amount", 1)
        set_property(memory, "trigger_once", True)
        set_property(memory, "destroy_after_trigger", False)
        set_property(memory, "requires_interaction", True)
        set_property(memory, "interaction_prompt_text", unreal.Text("E  RECOVER MEMORY ECHO"))
        set_property(memory, "completion_prompt_text", unreal.Text("MEMORY ECHO RECOVERED"))

    encounter = spawn(boss_encounter_class, (0, 250, 100), "CH11_FALLEN_CAPTAIN_ENCOUNTER")
    set_property(encounter, "boss_class", boss_class)
    set_property(encounter, "boss_defeat_quest_event_tag", unreal.Name("FallenCaptainDefeated"))
    set_property(encounter, "activation_radius", 580.0)
    set_property(encounter, "boss_spawn_offset",
                 unreal.Transform(location=unreal.Vector(620.0, 0.0, 0.0)))

    exit_actor = spawn(exit_class, (0, 1580, 100), "CH11_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 825)
    set_property(exit_actor, "completion_currency_reward", 410)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter12/L_CH12_ShadowAccord"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 270), "CH11_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(40.0, 40.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 11 with three memory echoes and Fallen Captain encounter.")
    unreal.log_warning("HUNTER: Build NavMesh and test memory interactions, boss phases, quest gating and campaign travel in Unreal Editor.")


build_chapter_11()
