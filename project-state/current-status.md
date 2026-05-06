# Current Status

## Snapshot

- Date: 2026-05-06
- Repository: `/Users/allenzhou/Research_software/Blast_wave`
- Durable baseline: the current documented runtime contract includes the default V1a path, the opt-in affine-effective closure path with `additive-rho` and `full-tensor` submodes, the opt-in V2 gradient-response path, optional differential `v2{2}(pT)` analysis, and the opt-in `response-test-023` initial-geometry response-test path.
- Latest durable verification anchor: 2026-05-06 local build, full local `ctest`, outside-sandbox ROOT default/response-test smokes, and a four-point response-test `A3` scan.
- This task added a synthetic `0+2+3` initial geometry template plus mandatory `eps3/psi3/v3` response observables and QA recomputation.

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
  - choose `v2pt-output-mode = same-file | separate-file`
  - optionally post-process with `analyze_blastwave_v2pt`

## Current Contract Highlights

- public entrypoints:
  - `generate_blastwave_events`
  - `qa_blastwave_output`
  - `analyze_blastwave_v2pt`
- canonical tracked example config:
  - `config/test_b8.cfg`
  - `config/test_b8_affine_effective.cfg`
  - `config/test_b8_response_023.cfg`
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

## Current Verification Picture

- ROOT-free coverage currently includes:
  - `test_maxwell_juttner_sampler`
  - `test_run_options`
  - `test_flow_field_model`
  - `test_emission_sampler`
  - `test_physics_utils`
  - `test_v2_pt_cumulant`
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
  - differential `v2{2}(pT)` same-file and separate-file modes
  - standalone differential post-processing
  - `chi2` TH1 contract
  - ROOT-free third-harmonic helper and response-template generator coverage
  - default Glauber generation + QA with the expanded third-harmonic schema
  - `response-test-023` generation + QA with optional `initial_geometry_density_x-y`
  - four-point `A3 = 0, 0.05, 0.10, 0.15` response-test scan showing increasing `mean(v3_wrt_psi3)`

Use `project-state/tests.md` for the summarized evidence trail.

## Active Caveats

- sandboxed `alienv` ROOT smoke output on this machine is not authoritative when PCM / module noise appears
- historical files under `qa/` span multiple schema generations and may not match the latest contract
- `v2pt-output-mode = separate-file` intentionally allows a metadata-only main result file that keeps `v2_2_pt_edges` but omits the full analysis payload
- `events.eps2` / `events.psi2` remain initial-state observables, while `eps2_f` / `psi2_f` / `chi2` remain freeze-out diagnostics
- `events.eps3` / `events.psi3` use the recentered harmonic convention and do not change the covariance `eps2/psi2` contract
- `response-test-023` is opt-in only; template weights `A2/A3` are not physical eccentricities
- `shell_weight` and any `EmissionSite::emissionWeight` restructuring remain intentionally deferred; the current response-test rollout only adds geometry templates and observables

## Documentation Layout After Compaction

- detailed runtime explanation stays in `docs/项目说明.md`
- formula-heavy workflow explanation stays in `docs/数学物理公式流程说明.md`
- quick semantic reminders stay in `docs/手记文档.md`
- agent-facing sync rules stay in `docs/agent_guide.md`
- current coordination state stays in `project-state/guide.md`, this file, and `project-state/handoff.md`
- detailed raw command transcripts were intentionally removed from `project-state/tests.md`; only durable conclusions remain
