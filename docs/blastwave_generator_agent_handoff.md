# Blast-Wave Generator Agent Handoff

## Current Baseline

- The repository no longer needs a requirement-analysis handoff for the original generator plan; the generator is implemented and the main handoff concern is preserving the current runtime contract.
- The codebase is split into:
  - a ROOT-free physics core in `include/` and `src/`
  - a ROOT-writing generation app in `apps/generate_blastwave_events.cpp` plus `apps/generate_blastwave/`
  - an independent ROOT-reading QA app in `apps/qa_blastwave_output.cpp`
- The default transverse flow model is the participant covariance-ellipse normal field implemented in:
  - `include/blastwave/FlowFieldModel.h`
  - `src/FlowFieldModel.cpp`
- The current public flow surface is:
  - `rho0`
  - `rho2`
  - `flow-power`
  - `debug-flow-ellipse`
- Legacy flow knobs `vmax`, `kappa2`, and `r-ref` are no longer accepted; they fail fast with migration guidance.
- Event summaries now carry both:
  - initial-state geometry observables `eps2` and `psi2`
  - final-state event observable `v2`
  - fixed-`b` mapping observable `centrality`
- `GeneratedEvent` now carries `FlowEllipseInfo` so the ROOT writer and QA code do not need to duplicate covariance math.

## Key Source Evidence

- Public data contracts:
  - `include/blastwave/BlastWaveGenerator.h`
  - `include/blastwave/FlowFieldModel.h`
  - `include/blastwave/io/RootOutputSchema.h`
- Runtime surface:
  - `apps/generate_blastwave/RunOptions.cpp`
  - `apps/generate_blastwave/RootEventFileWriter.cpp`
  - `apps/qa_blastwave_output.cpp`
- Shared observable helpers:
  - `include/blastwave/PhysicsUtils.h`
  - `src/PhysicsUtils.cpp`
- Regression coverage:
  - `tests/FlowFieldModelTest.cpp`
  - `tests/PhysicsUtilsTest.cpp`
  - `tests/MaxwellJuttnerMomentumSamplerTest.cpp`

## Public Runtime Contract

### CLI And Config Surface

- Generator entry styles:
  - `generate_blastwave_events [options]`
  - `generate_blastwave_events --config <path> [options]`
  - `generate_blastwave_events <config-path> [options]`
- Config-file precedence:
  - explicit CLI overrides config-file values
  - config-file values override built-in defaults
  - relative `output` paths resolve relative to the config file directory
- Flow-facing public knobs:
  - `--rho0`
  - `--rho2`
  - `--flow-power`
  - `--debug-flow-ellipse`
  - `--no-debug-flow-ellipse`
- Thermal-facing public knobs:
  - `--thermal-sampler <maxwell-juttner|gamma>`
  - `--mj-pmax`
  - `--mj-grid-points`
- The tracked example configs are:
  - `config/test_b8.cfg`
  - `config/b8.cfg`

### ROOT Output Contract

- Mandatory trees:
  - `events`
  - `participants`
  - `particles`
- Mandatory `events` branches:
  - `event_id`
  - `b`
  - `Npart`
  - `eps2`
  - `psi2`
  - `v2`
  - `centrality`
  - `Nch`
- Mandatory QA objects:
  - `Npart`
  - `eps2`
  - `psi2`
  - `v2`
  - `cent`
  - `participant_x-y`
  - `participant_x-y_canvas`
  - `x-y`
  - `px-py`
  - `pT`
  - `eta`
  - `phi`
- Optional debug payload, emitted only when `debug-flow-ellipse` is enabled:
  - `flow_ellipse_debug`
  - `flow_ellipse_participant_norm_x-y`

### QA Behavior

- The QA reader always validates:
  - tree presence and event ID continuity
  - participant and particle multiplicity consistency
  - `events.v2` against the particle-level second-harmonic Q-vector
  - `v2` histogram entry count and mean against the `events.v2` payload
  - `centrality` range and consistency with the fixed-`b` mapping
  - finite tree content and mass-shell stability
- The QA reader conditionally validates flow-ellipse debug objects:
  - absent: ignored
  - present: checked for entry counts, finite covariance content, eigenvalue/radius consistency, orthonormal axes, and normalized-participant fill counts

## Verification Snapshot

- The ROOT-free regression suite currently includes:
  - `test_maxwell_juttner_sampler`
  - `test_output_path_utils`
  - `test_flow_field_model`
  - `test_physics_utils`
- The current durable verification baseline is recorded in `project-state/current-status.md` and `project-state/tests.md`.
- The authoritative runtime path on this machine is still the outside-sandbox O2Physics `alienv` path; sandboxed ROOT smoke output is not the source of truth when PCM/module noise appears.

## Remaining Follow-Up

- Regenerate long-lived sample files in `qa/` when one fully current reference artifact set is needed for reuse.
- Preserve the explicit `ROOT::HistPainter` link and the launcher ROOT-alignment preflight while `participant_x-y_canvas` remains part of the mandatory output contract.
- When future changes touch output schema or public knobs, update `docs/agent_guide.md`, `docs/项目说明.md`, and `project-state/guide.md` in the same patch so coordination docs do not drift again.
