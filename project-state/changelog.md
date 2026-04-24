# Changelog

## 2026-04-24 Density-Normal Event Density Snapshot

- Added an optional sampler-specific ROOT object `density_normal_event_density_x-y`.
- The generator now captures the first event with participants when `flow-velocity-sampler = density-normal` and writes its smeared participant-density field as a `TH2`.
- Stored that histogram with the default ROOT draw option `LEGO1` so it opens as a 3D height-style visualization instead of a heatmap.
- Extended the independent QA reader so it validates that object when present:
  - it must be a `TH2`
  - its bin contents must be finite and non-negative
  - it must contain at least one positive density bin
  - it must advertise a 3D draw option such as `LEGO` or `SURF`
- Updated the human-facing docs and the project-state ledger for the new sampler-specific output contract.

## 2026-04-10 Project-State Bootstrap

- Bootstrapped the project coordination ledger on explicit user request.
- Synthesized the initial project snapshot from human-written docs in `docs/`, repository structure, git status, and the presence of historical QA artifacts under `qa/`.
- Recorded the baseline verification state as `unverified` because no authoritative build or smoke validation was executed in this task.

## 2026-04-10 Participant Geometry Output Update

- Added explicit participant output to the generator:
  - `participants` tree with `event_id`, `nucleus_id`, `x`, `y`, `z`
  - `participant_x-y` histogram
  - `participant_x-y_canvas` canvas with nucleus-A and nucleus-B circle outlines
- Enabled the ROOT stats box on the saved participant canvas.
- Confirmed by direct ROOT inspection that the saved `participant_x-y_canvas` now contains a `TPaveStats` primitive.
- Extended the QA reader so it validates the participant tree, the participant histogram/canvas objects, and the consistency of `Npart` with the participant tree entry count.
- Updated human-facing and agent-facing docs to describe the new participant contract.
- Rebuilt the project and reran a fresh 10-event smoke validation in the O2Physics ROOT environment.

## 2026-04-11 Config-File CLI And Example Config

- Added config-file support to `generate_blastwave_events` while keeping explicit CLI flags compatible:
  - `--config <path>`
  - positional `<config-path>`
- Adopted a lightweight `key = value` configuration contract with explicit precedence:
  - CLI overrides config file
  - config file overrides built-in defaults
  - relative `output` paths inside config files resolve relative to the config file directory
- Added a tracked example config for the repository workflow.
- Updated `docs/agent_guide.md` and `docs/项目说明.md` to document the new config-file interface and point to the example config.

## 2026-04-13 Centrality Output Contract

- Extended the event-level output contract with `events.centrality`.
- Added the `cent` histogram to the generated ROOT QA objects.
- Extended the independent QA reader so it checks:
  - `centrality` remains within `[0, 100]`
  - `centrality` matches the current fixed-`b` mapping
  - fixed-`b` runs keep one constant centrality value across all events
- Recorded a passed 10-event generate+QA smoke validation for the centrality-output extension in `project-state/tests.md`.

## 2026-04-13 Project-State Resync To Current Baseline

- Added the missing `project-state/guide.md` file to complete the minimum required bootstrap set.
- Synchronized the coordination ledger to the canonical `project-state/` path spelling.
- Corrected the tracked example-config location in the ledger from `qa/test_b8.cfg` to `config/test_b8.cfg`.
- Closed the old read-side ROOT environment blocker using the later passed smoke-QA evidence and shifted the remaining open gap to config-path documentation drift plus missing durable config-path QA evidence.

## 2026-04-14 Maxwell-Juttner Thermal Sampler Switch

- Added a ROOT-free `MaxwellJuttnerMomentumSampler` and switched the default thermal momentum mode to `maxwell-juttner`, while preserving `gamma` as an explicit compatibility path.
- Extended the public generator/config contract with:
  - `ThermalSamplerMode`
  - `thermal-sampler`
  - `mj-pmax`
  - `mj-grid-points`
- Added a ROOT-free core regression target `test_maxwell_juttner_sampler` and registered it in CTest.
- Updated the higher-authority docs under `docs/` and the tracked example config so they describe the new thermal sampler contract and consistently use `config/test_b8.cfg`.
- Closed the previously open config-file CLI documentation/validation gap with an authoritative default-MJ generate+QA record and an explicit gamma-mode compatibility smoke.
- Recorded the remaining environment caveat more precisely: sandboxed `alienv` ROOT smoke commands on this machine are not authoritative, but the same commands pass outside the sandbox.

## 2026-04-14 Repository Layout Cleanup

- Moved the historical reference ROOT macros from the top-level `qiye/` directory to `reference/legacy-root-macros/` so the directory name reflects responsibility instead of origin.
- Moved the tracked helper launcher from `config/run.sh` to `scripts/run_example_config.sh` so executable helpers no longer share a directory with static config files.
- Added a top-level `README.md` that documents the current repository structure and clarifies which directories are tracked sources versus ignored local artifacts.
- Updated higher-authority docs and the project-state ledger so they point to `scripts/run_example_config.sh` and `reference/legacy-root-macros/`.

## 2026-04-14 Source Responsibility Cleanup

- Split the oversized generator implementation into dedicated geometry, sampling, validation, and orchestration translation units.
- Split the generation app so CLI/config parsing, progress reporting, and ROOT output writing no longer live in the same source file as `main`.
- Added `include/blastwave/PhysicsUtils.h` plus `src/PhysicsUtils.cpp` so producer and QA code share derived-observable helpers instead of duplicating them.
- Tightened the repository-local `AGENTS.md` policy so future “整理代码/文件结构” requests explicitly include source-file responsibility cleanup rather than only top-level directory cleanup.

## 2026-04-22 Covariance-Ellipse Default Flow Replacement

- Replaced the default lab-radial flow model with a ROOT-free covariance-ellipse flow module:
  - added `include/blastwave/FlowFieldModel.h`
  - added `src/FlowFieldModel.cpp`
  - added `tests/FlowFieldModelTest.cpp`
- Migrated the public CLI/config flow surface from `vMax/referenceRadius/kappa2` to:
  - `rho0`
  - `rho2`
  - `flow-power`
  - `debug-flow-ellipse`
- The internal `BlastWaveConfig` field names now mirror that surface as:
  - `rho0`
  - `rho2`
  - `flowPower`
  - `debugFlowEllipse`
- Made legacy flow keys fail fast with migration guidance instead of silently mapping them.
- Extended the optional ROOT debug payload with:
  - `flow_ellipse_debug`
  - `flow_ellipse_participant_norm_x-y`
- Extended the QA reader so it validates those debug objects only when present.
- Fixed a real writer teardown crash in the optional debug path by detaching and resetting the debug `TTree/TH2` before `RootEventFileWriter` destruction.
- Updated higher-authority docs and the project-state ledger to reflect the new default flow model, public parameter contract, and debug/QA behavior.
- Recorded fresh authoritative outside-sandbox O2Physics validation for both:
  - the default `config/test_b8.cfg` path
  - a `--debug-flow-ellipse` smoke run

## 2026-04-22 Event-Level `v2` Output Contract

- Extended the mandatory event summary payload with `events.v2`.
- Added the `v2` summary histogram to the generated ROOT QA objects.
- Defined the event-level observable as the historical second-harmonic Q-vector magnitude:
  - `Q2x = sum cos(2 * phi)`
  - `Q2y = sum sin(2 * phi)`
  - `v2 = sqrt(Q2x^2 + Q2y^2) / Nch`
- Moved the azimuth and event-`v2` math into `PhysicsUtils` so the generator and QA reader share one implementation.
- Added a ROOT-free `test_physics_utils` regression target to lock the helper conventions.
- Extended the QA reader so it checks:
  - `events.v2` matches the particle-tree reconstruction
  - the `v2` histogram entry count and mean match the event payload
- Fixed a pre-existing `computePseudorapidity` edge case so exactly beam-aligned negative-z momenta now return the documented finite fallback instead of `-inf`.
- Recorded a fresh authoritative outside-sandbox O2Physics generate+QA smoke for the new contract.

## 2026-04-23 Example Launcher ROOT Alignment Repair

- Diagnosed the tracked example-launcher noise as a build/runtime ROOT mismatch:
  - the active O2Physics environment exposed `ROOT/v6-36-10-alice1-local2`
  - the cached build and generator binary still pointed at `ROOT/v6-36-10-alice1-local1`
- Hardened `scripts/run_example_config.sh` so it re-enters the canonical O2Physics runtime, checks the cached or binary ROOT prefix against `ROOTSYS`, and refreshes the generator build when they differ.
- Linked `ROOT::HistPainter` explicitly into `generate_blastwave_events` so the saved `participant_x-y_canvas` no longer depends on late painter autoload during `RootEventFileWriter::finish()`.
- Reconfigured, rebuilt, reran the tracked launcher outside the sandbox, and confirmed the earlier `TClassTable::Add` and `TCling::LoadPCM` noise no longer appears.

## 2026-04-23 Documentation Resync For The Current Runtime Contract

- Refreshed the lower-authority coordination guide in `project-state/guide.md` so it now matches the current covariance-ellipse flow model, public flow knobs, mandatory `v2`/`centrality` output, and optional flow-ellipse debug payload.
- Replaced the stale requirement-era `docs/blastwave_generator_agent_handoff.md` with a current implementation handoff focused on the shipped runtime contract.
- Updated `docs/agent_guide.md` so its repository layout, `EventInfo` summary, ROOT output contract, and QA-behavior sections now match the current code.
- Refreshed `project-state/handoff.md` so the durable handoff no longer points to the pre-flow-replacement structure-cleanup stage.
- This resync changed documentation only; it did not change generator or QA behavior.

## 2026-04-23 Fluid-Element Velocity Sampler Generalization

- Generalized flow selection into a fluid-element velocity sampler surface instead of introducing a narrowly named new flow model.
- Kept `covariance-ellipse` as the default sampler and added `density-normal` as a parallel sampler option.
- Split the ROOT-free flow implementation into:
  - `src/FlowFieldGeometry.cpp`
  - `src/FlowFieldDensity.cpp`
  - `src/FlowFieldModel.cpp`
- Extended the public runtime/config surface with:
  - `flow-velocity-sampler`
  - `flow-density-sigma`
- Kept `rho0`, `rho2`, and `flow-power`, with `rho2` intentionally ignored by `density-normal`.
- Added `test_run_options` and expanded `test_flow_field_model` to cover the new sampler contract.
- Recorded fresh authoritative outside-sandbox O2Physics generate+QA smokes for both:
  - the default covariance-ellipse sampler
  - the `density-normal` sampler
