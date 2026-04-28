# Blast-Wave Generator Agent Handoff

## Current Baseline

- The repository no longer needs a requirement-analysis handoff for the original generator plan; the generator is implemented and the main handoff concern is preserving the current runtime contract.
- The codebase is split into:
  - a ROOT-free physics core in `include/` and `src/`
  - a ROOT-writing generation app in `apps/generate_blastwave_events.cpp` plus `apps/generate_blastwave/`
  - an independent ROOT-reading QA app in `apps/qa_blastwave_output.cpp`
- The default medium model is now V1a `AffineGaussianResponse`: participant density `s0` is transformed into freeze-out density `sf` through fixed affine expansion and smoothing before emission.
- V2 `GradientResponse` is now available as an opt-in coupled medium/flow mode:
  - `density-evolution = gradient-response`
  - `flow-velocity-sampler = gradient-response`
  It builds separate `s_em` marker and `s_dyn` dynamics densities and uses `-grad ln s_dyn` for both transverse displacement and transverse velocity.
- The default transverse flow direction remains the covariance-ellipse normal sampler implemented in:
  - `include/blastwave/FlowFieldModel.h`
  - `src/FlowFieldModel.cpp`
- The internal event-medium and emission extension points are implemented in:
  - `include/blastwave/DensityFieldModel.h`
  - `include/blastwave/EventMedium.h`
  - `include/blastwave/EmissionSampler.h`
  - `src/EventMedium.cpp`
  - `src/EmissionSampler.cpp`
- The current public flow surface is:
  - `rho0`
  - `kappa2`
  - `flow-power`
  - `flow-velocity-sampler`
  - `density-evolution`
  - `flow-density-sigma`
  - `debug-flow-ellipse`
  - V2 `gradient-*` parameters
  - `debug-gradient-response`
  - `cooper-frye-weight`
- `density-evolution` and `flow-velocity-sampler` are orthogonal:
  - `affine-gaussian|none|gradient-response` controls the medium/emission stage
  - `covariance-ellipse|density-normal|gradient-response` controls the transverse flow source
  - the two `gradient-response` values must be selected together
- Legacy flow knobs `vmax`, `rho2`, and `r-ref` are no longer accepted; they fail fast with migration guidance.
- Event summaries now carry both:
  - initial-state geometry observables `eps2` and `psi2`
  - freeze-out geometry observables `eps2_f`, `psi2_f`, and `chi2`
  - centered source-size diagnostics `r2_0`, `r2_f`, and `r2_ratio`
  - final-state event observable `v2`
  - fixed-`b` mapping observable `centrality`
- `GeneratedEvent` carries `EventMedium`; `participantGeometry` defines event-summary `eps2/psi2`, while `emissionDensity` and `emissionGeometry` define the V1a freeze-out source and diagnostics.

## Key Source Evidence

- Public data contracts:
  - `include/blastwave/BlastWaveGenerator.h`
  - `include/blastwave/DensityFieldModel.h`
  - `include/blastwave/EventMedium.h`
  - `include/blastwave/EmissionSampler.h`
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
  - `tests/EmissionSamplerTest.cpp`
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
  - `--kappa2`
  - `--flow-power`
  - `--flow-velocity-sampler <covariance-ellipse|density-normal|gradient-response>`
  - `--density-evolution <affine-gaussian|none|gradient-response>`
  - `--debug-flow-ellipse`
  - `--no-debug-flow-ellipse`
  - `--gradient-sigma-em`
  - `--gradient-sigma-dyn`
  - `--gradient-density-floor-fraction`
  - `--gradient-density-cutoff-fraction`
  - `--gradient-displacement-max`
  - `--gradient-displacement-kappa`
  - `--gradient-diffusion-sigma`
  - `--gradient-vmax`
  - `--gradient-velocity-kappa`
  - `--debug-gradient-response`
  - `--no-debug-gradient-response`
  - `--cooper-frye-weight <none|mt-cosh>`
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
  - `eps2_f`
  - `psi2`
  - `psi2_f`
  - `chi2`
  - `r2_0`
  - `r2_f`
  - `r2_ratio`
  - `v2`
  - `centrality`
  - `Nch`
- Mandatory `particles` branches include the legacy kinematics/source payload plus:
  - `x0`
  - `y0`
  - `emission_weight`
- Mandatory QA objects:
  - `Npart`
  - `eps2`
  - `eps2_f`
  - `psi2`
  - `psi2_f`
  - `chi2`
  - `r2_0`
  - `r2_f`
  - `r2_ratio`
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
- Optional sampler-specific payload, emitted only when `flow-velocity-sampler = density-normal`:
  - `density_normal_event_density_x-y`
- Optional V2 debug payload, emitted when `debug-gradient-response` is enabled and at least one event has participants and particles:
  - `gradient_s0_x-y`
  - `gradient_s_em_x-y`
  - `gradient_s_dyn_x-y`
  - `gradient_s_f_x-y`

### QA Behavior

- The QA reader always validates:
  - tree presence and event ID continuity
  - participant and particle multiplicity consistency
  - `events.eps2_f`, `events.psi2_f`, and `events.chi2`
  - `chi2` consistency with `eps2_f / eps2`
  - `events.r2_0`, `events.r2_f`, and `events.r2_ratio` against centered particle-tree moments
  - `events.v2` against the particle-level second-harmonic Q-vector
  - `particles.emission_weight` finite and non-negative
  - `v2` histogram entry count and mean against the `events.v2` payload
  - `centrality` range and consistency with the fixed-`b` mapping
  - finite tree content and mass-shell stability
- The QA reader conditionally validates flow-ellipse debug objects:
  - absent: ignored
  - present: checked for entry counts, finite covariance content, eigenvalue/radius consistency, orthonormal axes, and normalized-participant fill counts
- The QA reader conditionally validates `density_normal_event_density_x-y`:
  - absent: ignored
  - present: checked as a finite non-negative `TH2` with positive support and a 3D draw option
- The QA reader conditionally validates the V2 gradient-response TH2 payload:
  - absent: ignored
  - present: requires all four histograms, finite non-negative bin contents, and positive support

## Verification Snapshot

- The ROOT-free regression suite currently includes:
  - `test_maxwell_juttner_sampler`
  - `test_output_path_utils`
  - `test_flow_field_model`
  - `test_emission_sampler`
  - `test_run_options`
  - `test_physics_utils`
- The current durable verification baseline is recorded in `project-state/current-status.md` and `project-state/tests.md`.
- The authoritative runtime path on this machine is still the outside-sandbox O2Physics `alienv` path; sandboxed ROOT smoke output is not the source of truth when PCM/module noise appears.
- The 2026-04-28 V2 baseline passed build, full CTest, default V1a smoke, `density-evolution none` smoke, V2 gradient smoke, V2 identity `r2_f == r2_0` smoke, V2 debug smoke, and zero-event debug QA.

## Remaining Follow-Up

- Regenerate long-lived sample files in `qa/` when one fully current reference artifact set is needed for reuse.
- Preserve the explicit `ROOT::HistPainter` link and the launcher ROOT-alignment preflight while `participant_x-y_canvas` remains part of the mandatory output contract.
- Future density evolution variants should extend `buildEventMedium()` without changing the `participantGeometry` event-summary contract.
- Future emission backends should continue to live behind `sampleEmissionSites()` and return `EmissionSite`.
- Future V2-like modes should keep medium response and velocity source coupled explicitly if their physics assumes a shared gradient field.
- When future changes touch output schema or public knobs, update `docs/agent_guide.md`, `docs/项目说明.md`, and `project-state/guide.md` in the same patch so coordination docs do not drift again.
