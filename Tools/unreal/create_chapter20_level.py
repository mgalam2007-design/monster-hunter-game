"""Generate Chapter 20's The Final Hunter endgame boss mission in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter20/L_CH20_TheFinalHunter"


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


def build_chapter_20():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter20")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    boss_encounter_class = native_class("HunterBossEncounterActor")
    boss_class = native_class("HunterFinalBossBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH20_Origin_Heart_Final_Arena")
    set_property(arena, "arena_half_width", 2350.0)
    set_property(arena, "arena_half_depth", 2350.0)
    set_property(arena, "wall_height", 1050.0)
    spawn(unreal.PlayerStart, (0, -1850, 110), "CH20_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1350), "CH20_Final_Battle_Light",
                unreal.Rotator(-58.0, -15.0, 0.0))
    set_property(sun, "intensity", 4.6)
    sky = spawn(unreal.SkyLight, (0, 0, 920), "CH20_Final_Battle_Sky")
    set_property(sky, "intensity", 0.65)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH20_Origin_Heart_Haze")
    set_property(fog, "fog_density", 0.034)

    heart = spawn(trigger_class, (0, 650, 100), "CH20_Origin_Heart_Final_Seal")
    set_property(heart, "quest_event_tag", unreal.Name("OriginHeartExposed"))
    set_property(heart, "event_amount", 1)
    set_property(heart, "trigger_once", True)
    set_property(heart, "destroy_after_trigger", False)
    set_property(heart, "requires_interaction", True)
    set_property(heart, "interaction_prompt_text", unreal.Text("E  DISABLE FINAL SEAL"))
    set_property(heart, "completion_prompt_text", unreal.Text("ORIGIN HEART EXPOSED"))

    encounter = spawn(boss_encounter_class, (0, 1250, 100), "CH20_THE_FINAL_HUNTER_ENCOUNTER")
    set_property(encounter, "boss_class", boss_class)
    set_property(encounter, "boss_defeat_quest_event_tag", unreal.Name("FinalHunterDefeated"))
    set_property(encounter, "activation_radius", 720.0)
    set_property(encounter, "boss_spawn_offset",
                 unreal.Transform(location=unreal.Vector(700.0, 0.0, 0.0)))

    exit_actor = spawn(exit_class, (0, 1900, 100), "CH20_Campaign_Finale_Exit")
    set_property(exit_actor, "completion_experience_reward", 2000)
    set_property(exit_actor, "completion_currency_reward", 1000)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    # No next map: this is the final chapter. The exit completes the campaign in place.

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 330), "CH20_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(47.0, 47.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 20 final arena with Origin Heart seal and Final Hunter boss encounter.")
    unreal.log_warning("HUNTER: Build NavMesh and test final boss phases, campaign completion, save flow and ending sequence in Unreal Editor.")


build_chapter_20()
