# Current Status

## Snapshot

- Date: 2026-05-13
- Repository: `/Users/allenzhou/Research_software/Blast_wave`
- Durable baseline: the current documented runtime contract includes the default V1a path, the opt-in affine-effective closure path with `additive-rho` and `full-tensor` submodes, the opt-in V2 gradient-response path, optional differential `v2/v3{2}(pT)` analysis, event-level `v_n`-`epsilon_n` regression notebook analysis, the opt-in `response-test-023` initial-geometry response-test path, and the opt-in density-normal `shell-gradient-corrected` transverse-flow magnitude mode.
- Latest durable verification anchor: 2026-05-13 notebook maintenance converted the `root_notebook` conda recipe to an `uproot`-only analysis stack, removed the PyROOT-only TH2/profile-fit notebook, and kept the existing 2026-05-11 multi-file event-tree regression smoke as the latest full notebook execution evidence; the latest comprehensive generator baseline remains the 2026-05-07 local build, full local `ctest` with 10/10 tests, focused `test_progress_reporter`, and prior O2Physics ROOT generate+QA smokes.
- Latest user-run-flow update: `notebooks/vn_epsn_regression.ipynb` is now the maintained notebook entrypoint for labelled multi-file event-tree regression comparisons using `uproot`; the generator progress heartbeat/ETA contract remains unchanged.
- Latest task narrowed the notebook environment and analysis path to `uproot` only, deleting the PyROOT-only comparison notebook; the latest physics task remains the sigma-equivalent density-defined radius fix from `docs/半径再次修复.md`.

## Current Runtime Baseline

- default medium path:
  - `density-evolution = affine-gaussian`
- default transverse-flow source:
  - `flow-velocity-sampler = covariance-ellipse`
- legacy comparison path:
  - `density-evolution = none`
- opt-in affine-effective closure path:
  - `density-evolution = affine-gaussian`
  - `flow-velocity-sampler = affine-effective`
  - `affine-effective-mode = additive-rho` by default
  - `affine-effective-mode = full-tensor` as opt-in tensor closure
- opt-in V2 path:
  - `density-evolution = gradient-response`
  - `flow-velocity-sampler = gradient-response`
- opt-in response-test geometry path:
  - `initial-geometry = response-test-023`
  - recentered synthetic `0+2+3` transverse point cloud
  - participant records use `nucleus_id = -1`
- optional differential-flow path:
  - configure `v2pt-bins`
  - configure `v3pt-bins`
  - choose `flowpt-output-mode = same-file | separate-file`
  - optionally post-process with `analyze_blastwave_vnpt`
- density-normal high-order transverse-flow path:
  - `flow-velocity-sampler = density-normal`
  - `flow-trans-direction-gradient-fraction = 0..1`
  - `flow-trans-radius = covariance | density-percentile:<p> | density-level:<fraction>`
  - `flow-trans-radius-resolution = balanced | precise | fast` for density-defined profile construction only
  - density-defined `R_density(phi)` is a geometry boundary; `xi_shell = r / R_density(phi)` is used for shell lookup
  - main density-defined flow strength uses `xi_flow = q * xi_shell`, with `q = sqrt(-2 log(1-p))` for percentile and `q = sqrt(-2 log(fraction))` for level
  - `flow-trans-magnitude-mode = radius-profile | shell-gradient-corrected`
  - `flow-trans-gradient-strength`, `flow-trans-gradient-density-floor-fraction`, and `flow-trans-gradient-max-factor-delta` for the opt-in shell-gradient correction only
  - `flow-trans-rho0` is the covariance-equivalent `xi_flow = 1` reference rapidity scale, and `flow-trans-profile-power` is the public radius-profile exponent

## Current Contract Highlights

- public entrypoints:
  - `generate_blastwave_events`
  - `qa_blastwave_output`
  - `analyze_blastwave_vnpt`
  - `notebooks/vn_epsn_regression.ipynb` for labelled multi-file event-level `v_n`-`epsilon_n` and `v_n`-`epsilon_m` regression comparisons on ROOT result files through `uproot`
- canonical tracked example config:
  - `config/test_b8.cfg`
  - `config/test_b8_affine_effective.cfg`
  - `config/test_023_dense.cfg`
  - `config/test_b8_flowpt.cfg`
  - `config/test_b8_density_normal_flow_trans.cfg`
  - `config/test_b8_density_normal_flow_trans_gradient.cfg`
- public transverse-flow naming:
  - `rho0` is rejected; use `flow-trans-rho0`
  - `flow-power` is rejected; use `flow-trans-profile-power`
  - `kappa2` is retained as the second-order response coefficient
- mandatory ROOT payload highlights:
  - `events`
  - `participants`
  - `particles`
  - `events.centrality`
  - `events.v2`
  - `events.eps3`
  - `events.psi3`
  - `events.v3`
  - `events.v2_wrt_psi2`
  - `events.v3_wrt_psi3`
  - `events.initial_geometry_mode`
  - `events.eps2_f`
  - `events.psi2_f`
  - `events.chi2`
  - `events.r2_0`
  - `events.r2_f`
  - `events.r2_ratio`
  - `particles.x0`
  - `particles.y0`
  - `particles.emission_weight`
- optional payload groups:
  - `initial_geometry_density_x-y` when `debug-initial-geometry = true`
  - flow-ellipse debug objects, including affine mode encoding, closure diagnostics, and additive-rho surface decomposition when `affine-effective` is selected
  - affine-effective density maps `affine_effective_density_initial_x-y` and `affine_effective_density_final_x-y`, captured as a first-valid-event pair with `LEGO1` default draw options
  - `density_normal_event_density_x-y`
  - V2 gradient debug histograms
  - `v2_2_pt_edges`
  - `v2_2_pt`
  - `v2_2_pt_canvas`
  - `v3_2_pt_edges`
  - `v3_2_pt`
  - `v3_2_pt_canvas`

## Current Verification Picture

- ROOT-free coverage currently includes:
  - `test_maxwell_juttner_sampler`
  - `test_progress_reporter`
  - `test_run_options`
  - `test_flow_field_model`
  - `test_emission_sampler`
  - `test_physics_utils`
  - `test_differential_flow_cumulant`
  - `test_output_path_utils`
  - `test_harmonic_geometry`
  - `test_blast_wave_generator_response`
- durable runtime evidence currently covers:
  - default V1a generation + QA
  - legacy `none` comparison
  - affine `density-normal` default and compensated cases
  - affine-effective `additive-rho` generation + QA with debug-flow-ellipse enabled
  - affine-effective `full-tensor` generation + QA with debug-flow-ellipse enabled
  - affine-effective before/after density map presence and QA validation
  - V2 gradient-response path
  - differential `v2/v3{2}(pT)` same-file and separate-file modes
  - standalone differential post-processing
  - flow-trans naming/parser contract with direct old-name rejection
  - density-normal direction mixing and covariance/percentile/level radius modes
  - density-normal flow-trans radius resolution parsing, cache-key separation, and balanced-vs-precise stability
  - density-normal density-defined radii with sigma-equivalent `xi_flow`, branch-local beta cap coverage, and shell-gradient `xi_shell` outer-shell lookup coverage
  - 1000-event response-test density-normal `newrap` acceptance with `meanPt` at O(1.0) and `psi2_proj` at O(0.02)
  - density-normal shell-gradient-corrected magnitude parsing, validation, ROOT-free behavior, and ROOT generate+QA smoke
  - `chi2` TH1 contract
  - ROOT-free third-harmonic helper and response-template generator coverage
  - default Glauber generation + QA with the expanded third-harmonic schema
  - `response-test-023` generation + QA with optional `initial_geometry_density_x-y`
  - four-point `A3 = 0, 0.05, 0.10, 0.15` response-test scan showing increasing `mean(v3_wrt_psi3)`

Use `project-state/tests.md` for the summarized evidence trail.

## Active Caveats

- sandboxed `alienv` ROOT smoke output on this machine is not authoritative when PCM / module noise appears
- historical files under `qa/` span multiple schema generations and may not match the latest contract
- `flowpt-output-mode = separate-file` intentionally allows a metadata-only main result file that keeps enabled `v2_2_pt_edges` / `v3_2_pt_edges` but omits the full analysis payload
- `events.eps2` / `events.psi2` remain initial-state observables, while `eps2_f` / `psi2_f` / `chi2` remain freeze-out diagnostics
- `events.eps3` / `events.psi3` use the recentered harmonic convention and do not change the covariance `eps2/psi2` contract
- `response-test-023` is opt-in only; template weights `A2/A3` are not physical eccentricities
- response/cross-talk TH2 objects keep full storage ranges `epsilon = 0..1` and projected `v = -1..1`, but open with compact default display ranges `epsilon = 0..0.35` and projected `v = -0.15..0.15`
- event-level `v_n`-`epsilon_n` regression should use `events.v2_wrt_psi2` / `events.v3_wrt_psi3` against `events.eps2` / `events.eps3`; `notebooks/vn_epsn_regression.ipynb` uses `uproot`, supports labelled `INPUT_FILES` multi-file overlays, and no longer depends on PyROOT
- `shell_weight` and any `EmissionSite::emissionWeight` restructuring remain intentionally deferred; the current response-test rollout only adds geometry templates and observables
- older validation ledgers may cite pre-archive response-test config names; current tracked response-test examples use `config/test_023_dense*.cfg`

## Current Documentation Layout

- documentation index stays in `docs/README.md`
- detailed runtime explanation stays in `docs/项目说明.md`
- formula-heavy workflow explanation stays in `docs/数学物理公式流程说明.md`
- quick semantic reminders stay in `docs/手记文档.md`
- historical design and handoff plans stay under `docs/PLAN/` and are skipped by default during closeout
- current agent-facing workflow rules stay in `AGENTS.md`
- deterministic documentation routing stays in `project-state/doc-sync-map.yml`
- current coordination state stays in `project-state/guide.md`, this file, and `project-state/handoff.md`
- detailed raw command transcripts were intentionally removed from `project-state/tests.md`; only durable conclusions remain
