"""Generate Chapter 6's elite Trial of Steel encounter in Unreal Editor.

Compile HunterGame, then run this script from Tools > Execute Python Script.
Creates an elite boss encounter, an E-key approval seal and a gated campaign exit.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter06/L_CH06_TrialOfSteel"


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


def build_chapter_six():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter06")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    boss_encounter_class = native_class("HunterBossEncounterActor")
    boss_class = native_class("HunterBossBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH06_Guild_Trial_Of_Steel")
    set_property(arena, "arena_half_width", 1750.0)
    set_property(arena, "arena_half_depth", 1750.0)
    set_property(arena, "wall_height", 700.0)

    spawn(unreal.PlayerStart, (0, -1350, 110), "CH06_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1100), "CH06_Sun",
                unreal.Rotator(-50.0, -30.0, 0.0))
    set_property(sun, "intensity", 6.0)
    sky = spawn(unreal.SkyLight, (0, 0, 800), "CH06_SkyLight")
    set_property(sky, "intensity", 0.9)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH06_Trial_Haze")
    set_property(fog, "fog_density", 0.01)

    encounter = spawn(boss_encounter_class, (0, 300, 100),
                      "CH06_Steel_Trial_Elite_Encounter")
    set_property(encounter, "boss_class", boss_class)
    set_property(encounter, "boss_defeat_quest_event_tag",
                 unreal.Name("SteelTrialOpponentDefeated"))
    set_property(encounter, "activation_radius", 560.0)
    set_property(encounter, "boss_spawn_offset",
                 unreal.Transform(location=unreal.Vector(550.0, 0.0, 0.0)))

    seal = spawn(trigger_class, (0, 1120, 100), "CH06_Guild_Approval_Seal")
    set_property(seal, "quest_event_tag", unreal.Name("SteelTrialApproved"))
    set_property(seal, "event_amount", 1)
    set_property(seal, "trigger_once", True)
    set_property(seal, "destroy_after_trigger", False)
    set_property(seal, "requires_interaction", True)
    set_property(seal, "interaction_prompt_text",
                 unreal.Text("E  CLAIM TRIAL APPROVAL"))
    set_property(seal, "completion_prompt_text",
                 unreal.Text("TRIAL OF STEEL PASSED"))

    exit_actor = spawn(exit_class, (0, 1480, 100), "CH06_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 450)
    set_property(exit_actor, "completion_currency_reward", 220)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 260), "CH06_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(37.0, 37.0, 7.0))

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 6 Trial of Steel with elite encounter and approval seal.")
    unreal.log_warning("HUNTER: Build NavMesh and test boss spawning, phase changes, seal gating and chapter exit in Unreal Editor.")


build_chapter_six()
