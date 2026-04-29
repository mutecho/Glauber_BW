# Current Status

## Snapshot

- Date: 2026-04-29
- Repository: `/Users/allenzhou/Research_software/Blast_wave`
- Durable baseline: the current documented runtime contract includes the default V1a path, the opt-in V2 gradient-response path, and optional differential `v2{2}(pT)` analysis.
- Latest durable verification anchor: 2026-04-29 outside-sandbox ROOT validation and local `ctest` evidence summarized in `project-state/tests.md`.
- This task updated documentation structure only. It did not change runtime behavior and did not add a new validation run.

## Current Runtime Baseline

- default medium path:
  - `density-evolution = affine-gaussian`
- default transverse-flow source:
  - `flow-velocity-sampler = covariance-ellipse`
- legacy comparison path:
  - `density-evolution = none`
- opt-in V2 path:
  - `density-evolution = gradient-response`
  - `flow-velocity-sampler = gradient-response`
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
- mandatory ROOT payload highlights:
  - `events`
  - `participants`
  - `particles`
  - `events.centrality`
  - `events.v2`
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
  - flow-ellipse debug objects
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
- durable runtime evidence currently covers:
  - default V1a generation + QA
  - legacy `none` comparison
  - affine `density-normal` default and compensated cases
  - V2 gradient-response path
  - differential `v2{2}(pT)` same-file and separate-file modes
  - standalone differential post-processing
  - `chi2` TH1 contract

Use `project-state/tests.md` for the summarized evidence trail.

## Active Caveats

- sandboxed `alienv` ROOT smoke output on this machine is not authoritative when PCM / module noise appears
- historical files under `qa/` span multiple schema generations and may not match the latest contract
- `v2pt-output-mode = separate-file` intentionally allows a metadata-only main result file that keeps `v2_2_pt_edges` but omits the full analysis payload
- `events.eps2` / `events.psi2` remain initial-state observables, while `eps2_f` / `psi2_f` / `chi2` remain freeze-out diagnostics

## Documentation Layout After Compaction

- detailed runtime explanation stays in `docs/项目说明.md`
- quick semantic reminders stay in `docs/手记文档.md`
- agent-facing sync rules stay in `docs/agent_guide.md`
- current coordination state stays in `project-state/guide.md`, this file, and `project-state/handoff.md`
- detailed raw command transcripts were intentionally removed from `project-state/tests.md`; only durable conclusions remain
