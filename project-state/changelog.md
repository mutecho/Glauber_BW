# Changelog

## 2026-05-20 Cross-Harmonic Notebook Fit Scope

- Updated `notebooks/vn_epsn_regression.ipynb` so cross-harmonic `v2_wrt_psi2` versus `eps3` and `v3_wrt_psi3` versus `eps2` mixing checks use only free-intercept linear fits.
- Kept same-harmonic `v_n/epsilon_n` response summaries on the existing free-intercept plus through-origin fit contract.
- No generator code, public config keys, ROOT output schema, or QA schema changed.

## 2026-05-19 Density-Normal Gradient-Fraction Expansion Compensation

- Changed explicit density-normal flow-trans semantics so `flow-trans-direction-gradient-fraction` is no longer a uniform vector blend; `0<f<1` now constrains the local density-gradient direction to a global-expansion cone with minimum outward projection `1-f`.
- Scaled only the `shell-gradient-corrected` multiplicative gradient correction by `flow-trans-direction-gradient-fraction`; default `radius-profile` baseline flow strength remains unchanged.
- Set densemix/newrap Glauber comparison configs to `flow-trans-direction-gradient-fraction = 0.8` and updated active docs, generated site pages, tests, and `project-state/`.
- Validated local build, full 10/10 CTest, O2Physics ROOT 5000-event generate+QA for ellipse/densemix/newrap, and `f=1.0` densemix/newrap controls showing the new `0.8` configs are not phi/v2 suppressed.

## 2026-05-19 Newrap Glauber Response Retuning

- Retuned `config/test_023_dense_newrap_glauber.cfg` for the current three-way Glauber comparison so newrap uses density-normal flow with covariance radius, pure density-gradient direction, `flow-trans-rho0 = 1.2`, `flow-trans-profile-power = 1.0`, matched `smear = 0.5`, and `v2/v3{2}(pT)` bins through 7 GeV.
- Kept densemix unchanged because its current `phi` and `v2{2}(pT)` behavior was already close to the ellipse reference.
- Regenerated `qa/test_023_newrap_glauber.root` at 5000 events and recorded QA plus ROOT metric evidence showing improved `phi` and `v2_2_pt` agreement with `qa/test_023_ellipse_glauber.root`.
- Kept generator code, public config keys, ROOT schema, and QA schema unchanged.

## 2026-05-19 Mode-Local Config Cleanup

- Removed inactive mode-specific entries from workspace `config/*.cfg` files so examples no longer carry unused response-test, affine-effective, affine-evolution, gradient-response, or shell-gradient knobs outside their selected mode combinations.
- Kept public config keys, parser behavior, generator code, ROOT schema, QA schema, and physics algorithms unchanged.
- Validated all 18 workspace cfg files with O2Physics ROOT executor 1000-event generate+QA smokes.

## 2026-05-19 Run Script Self-Reentry Path

- Updated run scripts under `scripts/` except `cmake.sh` so their alienv re-entry `script_path` is derived from `BASH_SOURCE[0]`.
- Fixed copied run scripts such as `scripts/run_b8_v3.sh` so they re-enter themselves instead of a hard-coded source script.
- Kept generator code, config keys, ROOT schema, QA schema, and physics behavior unchanged.

## 2026-05-13 Uproot-Only Notebook Environment

- Narrowed the `root_notebook` conda recipe to the `uproot`-based Python analysis stack and removed the `root`/PyROOT dependency.
- Removed the PyROOT-only `notebooks/vn_epsn_pyroot_th2_fit.ipynb` path.
- Kept `notebooks/vn_epsn_regression.ipynb` as the maintained multi-file event-tree regression notebook.

## 2026-05-11 Event-Level Multi-File Regression Notebook

- Updated `notebooks/vn_epsn_regression.ipynb` so `INPUT_FILES` accepts labelled ROOT result files and runs the same event-level regression/cuts for each file.
- Added grouped comparison plots and tables for same-harmonic `v_n/epsilon_n` response and cross-harmonic `v_n/epsilon_m` mixing, with regression still performed on raw `events` tree rows.
- Validated the notebook in the `root_notebook` environment over the default `response_023`, `dense_mix`, and `newrap` ROOT files.

## 2026-05-11 Native PyROOT TH2 Response-Fit Notebook

- Added `notebooks/vn_epsn_pyroot_th2_fit.ipynb` as a notebook-first analysis path that reads response/cross-talk `TH2` objects directly with PyROOT.
- The notebook profiles each `TH2` with `TH2::ProfileX`, performs ROOT `TF1` fits with free-intercept and through-origin forms, and compares multiple ROOT files in grouped `v_n/epsilon_n` and `v_n/epsilon_m` canvases.
- No generator code, ROOT output schema, config contract, or QA schema changed.

## 2026-05-09 Documentation Routing And Historical Plan Archive

- Added `project-state/doc-sync-map.yml` as the local `sync-project-knowledge` routing manifest.
- Added `docs/README.md` and `docs/PLAN/README.md` to separate current docs, coordination ledgers, and historical plans.
- Updated active entrypoint and `project-state` docs to treat `AGENTS.md` as the current agent policy and `docs/PLAN/` as historical reference.

## 2026-05-07 Progress Heartbeat And ETA

- Added a one-second heartbeat to `generate_blastwave_events` progress output so enabled progress visibly updates even when the integer percent is unchanged.
- Added rough ETA rendering based on average completed-event time, showing unknown ETA before the first completed event and zero ETA at completion.
- Kept the existing `progress`, `--progress`, `--no-progress`, auto-TTY behavior, config keys, ROOT schema, and QA schema unchanged.
- Added deterministic `test_progress_reporter` coverage and updated the user-facing progress description in `docs/项目说明.md`.

## 2026-05-07 Density-Defined Flow-Trans Sigma-Equivalent Radius

- Changed density-percentile and density-level transverse-flow radii so `R_density(phi)` controls angular shell geometry while main strength uses `xi_flow = q * xi_shell`.
- Defined `q = sqrt(-2 * log1p(-p))` for `density-percentile:p` and `q = sqrt(-2 * log(fraction))` for `density-level:fraction`.
- Tightened `density-level` validation to `0 < fraction < 1`.
- Kept shell-gradient correction table lookup bounded to `0 <= xi_shell <= 1`, reusing the outer shell for `xi_shell > 1` while the base `rhoRaw` uses `xi_flow`.
- Kept CLI/config names, example cfg files, ROOT schema, and QA schema unchanged.
- Updated ROOT-free tests, active docs, config comments, and `project-state/`; recorded local build, full `ctest`, O2Physics percentile/level smokes, and 1000-event response acceptance metrics.

## 2026-05-07 Shell-Gradient-Corrected Density-Normal Flow Magnitude

- Added `flow-trans-magnitude-mode = radius-profile | shell-gradient-corrected`, keeping `radius-profile` as the default behavior.
- Added `flow-trans-gradient-strength`, `flow-trans-gradient-density-floor-fraction`, and `flow-trans-gradient-max-factor-delta`.
- Implemented an event-level `FlowTransGradientCorrectionProfile` cache that reuses density-defined `R(phi)`, integrates the outward density-gradient proxy, normalizes it event-wide, subtracts same-shell means, and applies a clamped multiplicative correction.
- Restricted `shell-gradient-corrected` to `density-normal + density-percentile/level`; covariance radius and non-density-normal samplers now fail validation for that mode.
- Added `config/test_b8_density_normal_flow_trans_gradient.cfg` as the complete Chinese example config for the opt-in corrected path.
- Updated parser, validation, help text, ROOT-free tests, active docs, and `project-state/`; recorded local build, full `ctest`, and O2Physics `PRIMARY_OK` ROOT generate+QA smokes for shell-gradient and radius-profile control runs.

## 2026-05-07 Flow-Trans Radius Resolution Presets

- Added density-normal-only `flow-trans-radius-resolution = balanced | precise | fast`, with `balanced` as the new default profile grid and `precise` preserving the old `360 x 512` grid.
- Changed density-defined radius profile construction to use scalar density queries and reuse percentile cumulative buffers across angular rays.
- Extended the flow-trans radius profile cache key with resolution and radial sample count so fast/precise profiles cannot be reused across presets.
- Updated parser, validation, help text, tests, example configs, docs, and `project-state/`.
- Recorded local build, full `ctest`, and O2Physics ROOT generate+QA smokes for covariance, percentile balanced, percentile precise, and level balanced radius runs.

## 2026-05-06 Flow-Trans Naming And Density-Normal Radius Modes

- Replaced public `rho0` / `flow-power` with `flow-trans-rho0` / `flow-trans-profile-power`; old names now fail in CLI and config with migration guidance.
- Kept `kappa2` as the public second-order response coefficient.
- Added density-normal-only `flow-trans-direction-gradient-fraction` and `flow-trans-radius = covariance | density-percentile:<p> | density-level:<fraction>`.
- Added `EventMedium::emissionDensityScale` and an event-level angular radius profile cache for density-defined transverse-flow radii.
- Added `config/test_b8_density_normal_flow_trans.cfg` as a Chinese explained example; optional flowpt bins stay commented so low-stat radius smokes are not coupled to differential cumulants.
- Updated parser, validation, flow-field tests, active docs, and `project-state/`; recorded local build, full `ctest`, and outside-sandbox O2Physics generate+QA smokes for default plus three density-normal radius modes.

## 2026-05-06 High-Order Transverse Radius Design Draft

- Added `docs/高阶半径与梯度混合整合方案.md` as the reviewed design entry for future high-order transverse-flow direction, density-defined radius, and optional gradient-correction work.
- Made config key cleanup the required first step before implementation, with the proposed new transverse-flow layer using `flow-trans-*`.
- Recorded WI-005 for the required config audit and migration plan before code/config changes.

## 2026-05-06 Differential `v2/v3{2}(pT)` Flowpt Output

- Generalized the previous v2-only differential cumulant path into a harmonic-aware `v2/v3{2}(pT)` analysis core.
- Added `v3pt-bins`, `v3_2_pt_edges`, `v3_2_pt`, and `v3_2_pt_canvas`.
- Replaced `v2pt-output-mode` / `v2pt-output` with shared `flowpt-output-mode` / `flowpt-output`; old output option names now fail with migration guidance.
- Replaced standalone `analyze_blastwave_v2pt` with `analyze_blastwave_vnpt`, which computes all enabled harmonics found in ROOT metadata.
- Added `config/test_b8_flowpt.cfg` as a complete Chinese explained flowpt example config.

## 2026-05-06 Response/Cross-Talk TH2 Display Window

- Kept the four response/cross-talk TH2 objects filled over their full storage ranges `epsilon = 0..1` and projected `v = -1..1`.
- Set their persisted default display window to `epsilon = 0..0.35` and projected `v = -0.15..0.15` so response-test output opens on the populated region.
- Updated docs and `project-state/` to distinguish display range from stored histogram bins.

## 2026-05-06 Response-Test 0+2+3 Initial Geometry

- Added public `initial-geometry = glauber | response-test-023`, defaulting to `glauber`.
- Implemented `response-test-023` as a recentered synthetic `0+2+3` transverse point cloud with configurable source count, circular background, elliptical Gaussian component, and triangular hotspot component.
- Added `eps3/psi3`, `v3`, `v2_wrt_psi2`, `v3_wrt_psi3`, response/cross-talk TH2 objects, and optional `initial_geometry_density_x-y` debug output.
- Extended independent QA to accept `nucleus_id=-1` only for response-test events and to recompute weighted Q2/Q3 observables from the particle tree.
- Added ROOT-free harmonic geometry and response-template generator tests plus a complete Chinese response-test config `config/test_b8_response_023.cfg`.
- Updated user docs and `project-state/` for the config, algorithm, output schema, QA, and operator-flow changes.

## 2026-05-02 Formula-Oriented Workflow Documentation

- Added `docs/数学物理公式流程说明.md` as a math-heavy walkthrough of the current generator chain from Woods-Saxon participants through medium evolution, emission sampling, thermal momentum, flow boost, event observables, and optional `v2{2}(pT)` analysis.
- Expanded the formula walkthrough with per-section code locations for the relevant headers, source files, entrypoints, writer paths, QA paths, and core functions.
- Added lightweight pointers from `README.md`, `docs/agent_guide.md`, `project-state/guide.md`, and `project-state/current-status.md`.
- Documentation-only update; no runtime behavior, config contract, ROOT schema, QA logic, or validation baseline changed.

## 2026-04-29 Affine-Effective Density Map Output

- Added `affine_effective_density_initial_x-y` and `affine_effective_density_final_x-y` to affine-effective ROOT output as first-valid-event before/after density snapshots.
- Stored both maps as `TH2F` objects with default ROOT draw option `LEGO1`.
- Extended independent QA so the pair is validated as an all-or-none optional payload with finite non-negative bins, positive support, and 3D draw style.
- Recorded fresh O2Physics build, `ctest`, authoritative affine-effective generate+QA, and ROOT file inspection evidence.

## 2026-04-29 Affine-Effective Additive-Rho / Full-Tensor Correction

- Added `affine-effective-mode = additive-rho | full-tensor`, defaulting to `additive-rho`.
- Replaced the first affine-effective internal formula with:
  - `additive-rho`: density-normal direction, `rho0` baseline rapidity, and affine geometry correction
  - `full-tensor`: opt-in principal-axis tensor velocity closure
- Changed `H_in_eff/H_out_eff` diagnostics to use `lambdaIn/lambdaOut / affineDeltaTauRef`.
- Kept `affine-kappa-aniso` parsed and finite-validated as a legacy/no-op compatibility key for current affine-effective modes.
- Extended `flow_ellipse_debug`, writer schema, and QA with `affine_effective_mode` and additive-rho surface rapidity decomposition diagnostics.
- Updated parser tests, flow-model tests, the affine-effective example config, user docs, and `project-state/`.
- Recorded fresh local build, full `ctest`, baseline ROOT generate+QA, additive-rho ROOT generate+QA, and full-tensor ROOT generate+QA evidence.

## 2026-04-29 Affine-Effective Closure Flow Sampler

- Added a dedicated `EventMedium::affineEffectiveClosure` block so affine closure diagnostics stay separate from covariance-ellipse geometry.
- Implemented opt-in `flow-velocity-sampler = affine-effective` for `density-evolution = affine-gaussian`.
- Added public CLI/config knobs:
  - `affine-delta-tau-ref`
  - `affine-kappa-flow`
  - `affine-kappa-aniso`
  - `affine-u-max`
- Extended `debug-flow-ellipse` and the independent QA reader so affine closure diagnostics are serialized and validated when present.
- Added ROOT-free parser/validation and flow-model coverage, plus a tracked opt-in example config `config/test_b8_affine_effective.cfg`.
- Recorded fresh local build, `ctest`, authoritative default ROOT generate+QA, and authoritative affine-effective ROOT generate+QA evidence.
- Intentionally did not change `shell_weight` semantics or restructure `EmissionSite::emissionWeight` in this rollout.

## 2026-04-29 Documentation Compaction For Project Knowledge Rules

- Reduced `docs/agent_guide.md` to an actual agent-facing guide and removed the duplicated full runtime walk-through.
- Reduced `docs/手记文档.md` to a short current-chain and confusion-note document.
- Rewrote `project-state/guide.md`, `project-state/current-status.md`, `project-state/handoff.md`, and `project-state/tests.md` so they now keep current coordination state, durable conclusions, and the latest actionable handoff instead of repeated parameter tables or raw command archives.
- This was a documentation-only cleanup; it did not change runtime behavior or add a new validation run.

## 2026-04-29 Differential `v2{2}(pT)` Analysis Contract

- Implemented the optional differential `v2{2}(pT)` workflow described in `docs/v2计算.md` with explicitly configured `pT` bin edges, a shared ROOT-free cumulant core, and a standalone post-processing app.
- Added public app-layer controls `v2pt-bins`, `v2pt-output-mode = same-file | separate-file`, and `v2pt-output`; relative separate-file output paths now resolve against the config-file directory.
- Extended the ROOT analysis contract with `v2_2_pt_edges` metadata plus optional `v2_2_pt` and `v2_2_pt_canvas`; in `separate-file` mode, the main result file is allowed to remain metadata-only while the dedicated analysis file carries the full trio.
- Extended generator wiring, QA recomputation, regression coverage, user docs, and `project-state/`, then recorded fresh local build, CTest, authoritative outside-sandbox O2Physics smoke runs, standalone analysis runs, and ROOT key inspections.

## 2026-04-29 Freeze-Out Eccentricity Response TH1 Clarification

- Kept the existing `chi2` histogram as the default TH1 output for `eps2_f / eps2` instead of adding a second mandatory canvas object.
- Clarified the `chi2` histogram title so the ratio meaning is explicit when the ROOT file is opened.
- Rebuilt the touched app binaries, ran an authoritative outside-sandbox generate+QA smoke, and synchronized docs plus `project-state/`.

## 2026-04-28 Public Affine V1a Evolution Knobs

- Exposed `affine-lambda-in`, `affine-lambda-out`, and `affine-sigma-evo` on the public CLI/config surface.
- Moved the default V1a affine response values out of generator hardcoding and into `BlastWaveConfig`, while preserving the previous defaults `1.20`, `1.05`, and `0.5 fm`.
- Added generator validation for positive affine expansion factors and non-negative affine evolution smoothing.
- Expanded `test_run_options`, updated sample configs, refreshed high-authority docs, and synchronized `project-state/`.

## 2026-04-28 Affine Density-Normal Kappa Compensation Switch

- Added public CLI/config switch `density-normal-kappa-compensation`, defaulting to `false`.
- Changed `affine-gaussian + density-normal` so the default strength is `rho0 * pow(rTilde, flowPower)` with gradient-derived direction only.
- Kept the previous `kappa2` exponential multiplier as an opt-in compatibility path behind that switch.
- Made invalid mode combinations fail fast in generator validation instead of silently ignoring the switch.
- Updated flow-field regression tests, RunOptions regression tests, sample configs, user docs, and `project-state/`.
- Recorded fresh build, CTest, covariance baseline, density-normal default-off, density-normal compensated, and invalid-combination validation evidence in `project-state/tests.md`.

## 2026-04-28 V2 Gradient-Response Medium And Flow

- Added opt-in `density-evolution = gradient-response` and `flow-velocity-sampler = gradient-response`, with generator validation requiring the two modes to be selected together.
- Built separate V2 densities:
  - `s_em` for marker initial positions using `gradient-sigma-em`
  - `s_dyn` for gradient response using `gradient-sigma-dyn`
- Implemented gradient-driven marker displacement, optional diffusion, and site transverse velocity in `EmissionSampler`.
- Extended `EmissionSite`, `ParticleRecord`, and the ROOT schema with `x0`, `y0`, and `emission_weight`.
- Added event-level centered radius diagnostics `r2_0`, `r2_f`, and `r2_ratio`, plus matching QA histograms.
- Added optional `cooper-frye-weight = none|mt-cosh`; default `none` preserves unit weights, while `mt-cosh` stores `m_T cosh(y - eta_s)` and feeds weighted event-`v2`.
- Added optional V2 debug histograms:
  - `gradient_s0_x-y`
  - `gradient_s_em_x-y`
  - `gradient_s_dyn_x-y`
  - `gradient_s_f_x-y`
- Extended QA to validate the new branches, histograms, centered `r2` values, particle weights, V2 debug TH2 payload, and zero-event debug behavior.
- Updated the V2 sample config comments, docs, and `project-state/` ledger.
- Recorded fresh build, CTest, default V1a, legacy `none`, V2 gradient, V2 identity, V2 debug, and zero-event debug validation evidence in `project-state/tests.md`.

## 2026-04-28 Kappa2 Flow Response Contract

- Replaced public `rho2` with `kappa2` in CLI/config parsing, help text, sample configs, tests, and current docs.
- Defined the event-wise V1a second-order rapidity modulation amplitude as `kappa2 * eps2_initial`.
- Switched the V1a response plane from freeze-out `psi2_f` to initial participant `psi2`; freeze-out `eps2_f`, `psi2_f`, and `chi2` remain diagnostics.
- Made legacy `rho2` fail fast with migration guidance to `kappa2`.
- Kept the ROOT output schema unchanged.

## 2026-04-28 V1a Affine Gaussian Density Evolution

- Added `density-evolution = affine-gaussian | none` as the public medium-mode switch and made `affine-gaussian` the default.
- Implemented the fixed-parameter V1a `s0 -> sf` response with `lambda_in = 1.20`, `lambda_out = 1.05`, and `sigma_evo = 0.5 fm`.
- Kept `density-evolution` orthogonal to `flow-velocity-sampler = covariance-ellipse | density-normal` so the medium response and velocity-direction sampler can be selected independently.
- Preserved the previous identity-medium and participant-hotspot emission behavior behind `density-evolution = none`.
- Extended the mandatory event summary and QA contract with freeze-out geometry diagnostics:
  - `events.eps2_f`
  - `events.psi2_f`
  - `events.chi2`
  - matching `eps2_f`, `psi2_f`, and `chi2` histograms
- Extended the QA reader so it validates the new branches, histograms, finite ranges, and `chi2 = eps2_f / eps2` consistency.
- Updated the human-facing docs and project-state ledger to document the new switch surface, default mode, ROOT schema, and validation evidence.
- Recorded fresh build, CTest, default V1a, legacy `none`, and `density-normal` authoritative validation evidence in `project-state/tests.md`.

## 2026-04-28 EventMedium And Emission Interface Refactor

- Replaced the old flow-context language with an `EventMedium` state that separates participant geometry, initial density, emission-stage density, and emission-stage geometry.
- Added a dedicated `DensityFieldModel` and routed `density-normal` flow plus the optional density snapshot through the same generator-owned emission density.
- Added an `EmissionSampler` boundary with `EmissionSite` so the generator loop no longer directly owns participant-hotspot smearing.
- Kept the public CLI/config surface and ROOT output schema unchanged.
- Added the `test_emission_sampler` regression target and expanded flow/density coverage in `test_flow_field_model`.
- Added `docs/EventMedium与发射接口设计.md` and refreshed the human-facing docs and project-state ledger for the new extension points.

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
