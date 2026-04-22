# Changelog

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
- Migrated the public flow surface from `vMax/referenceRadius/kappa2` to:
  - `rho0`
  - `rho2`
  - `flowPower`
  - `debugFlowEllipse`
- Switched CLI/config parsing and repository-shipped example configs to:
  - `rho0`
  - `rho2`
  - `flow-power`
  - `debug-flow-ellipse`
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
