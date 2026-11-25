# HUNTER 2.0 — Standalone Machine-Learning Architecture

## Product requirement

All game intelligence runs independently inside HUNTER 2.0. No GPT, OpenAI API, hosted LLM, external inference endpoint, or internet connection is required. The AI systems consume game events and locally stored model data.

The project will implement distinct, explainable techniques as separate native Unreal C++ modules. They must not be presented as a recreation of GPT: GPT is a large pretrained transformer language model, and reproducing its scale, training corpus, and compute requirements is outside this game's scope.

## Separate in-game learning methods

| Module | Technique | Game responsibility |
|---|---|---|
| Adaptive combat | Reinforcement learning (tabular Q-learning first) | Enemies evaluate tactics using state, action, and reward feedback |
| Player profiling | Unsupervised learning (k-means clustering) | Classify tendencies such as aggressive, defensive, evasive, and ability-heavy play |
| Behaviour prediction | Supervised learning (small classifier trained on labelled/local observations) | Estimate likely player response from recent combat features |
| Neural decision model | Small feed-forward neural network | Combine combat features into tactic scores; train offline or incrementally with bounded updates |
| Equipment advisor | Recommendation/ranking model | Rank weapons, skills, and defensive options using player profile and encounter context |
| Lore retrieval | Local text embeddings and vector similarity | Retrieve relevant authored lore, codex entries, and NPC memory without a cloud service |
| Dialogue intelligence | Local authored dialogue + intent classification + retrieval | NPCs select context-aware dialogue from game-authored content; no hosted language model |
| Procedural content | Rule-based generation informed by learned player profile | Vary encounter composition, optional objectives, and challenge pacing |

## Transformer and language-model boundary

Transformer attention is a model architecture, not a single feature that can simply be switched on. A GPT-like model needs a tokenizer, a large training corpus, substantial training compute, model weights, inference memory, and extensive evaluation. HUNTER 2.0 will not claim to train or contain GPT.

A future optional local language model may be evaluated only if a compact, legally usable model and an inference runtime fit the game's target hardware. It must remain offline and optional. The first NPC dialogue release will use local intent classification, authored response templates, and semantic retrieval so it can ship without that dependency.

## Runtime and hardware rules

- Native C++ implementation; no network requests and no API keys.
- Learning updates are bounded and run at controlled intervals, never every frame.
- Invalid, non-finite, or out-of-range observations are rejected.
- Save learned profile/model state locally with a version number; corrupted or missing data falls back to safe defaults.
- Gameplay remains deterministic enough to debug: expose debug values and allow learning to be disabled/reset.
- AI may adjust tactics and presentation, but must not secretly change player input, bypass quest gates, or make unavoidable attacks.
- Train/evaluate models with offline tools where practical; runtime learning is incremental and conservative.
- The target PC is modest, so avoid requiring a GPU or a large model for core gameplay.

## Delivery stages

1. **Learning foundation:** local profile data, bounded observation collection, model versioning, save/load and debug controls.
2. **Adaptive enemies:** Q-learning policy for a small, readable set of combat tactics; reward signals for hits, misses, dodges, and player counterattacks.
3. **Player understanding:** clustering and a lightweight supervised classifier, with confidence thresholds and a neutral fallback.
4. **Recommendations:** equipment/skill ranking driven by the profile and chapter context.
5. **Local NPC intelligence:** intent classifier, lore embeddings, retrieval and authored dialogue variation.
6. **Evaluation:** repeatable combat scenarios, baseline-vs-learning comparisons, performance budgets, save compatibility and abuse/exploit checks.

Each stage needs its own native code, validation, and documented test evidence. Static CI validation does not prove Unreal compilation or runtime learning; those require Unreal Editor builds and gameplay tests.

## Acceptance criteria

- The game runs with networking disabled and no AI credentials configured.
- Every technique has a separate implementation boundary and a clear input/output contract.
- Learning state survives a save/load cycle once persistence is implemented.
- A baseline policy always exists if model data is unavailable.
- Automated tests cover invalid data, model reset, bounded updates, and reproducible evaluation scenarios.
- No claims of GPT-equivalent generation or training are made.
