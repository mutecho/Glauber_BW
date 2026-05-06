# Handoff

## Latest Durable Handoff

- Stage completed: differential `v2/v3{2}(pT)` flowpt output contract
- What changed:
  - generalized the old v2-only differential cumulant implementation into a harmonic-aware core for `n=2` and `n=3`
  - kept existing v2 object names and added `v3_2_pt_edges`, `v3_2_pt`, and `v3_2_pt_canvas`
  - added public `v3pt-bins`
  - replaced `v2pt-output-mode` / `v2pt-output` with shared `flowpt-output-mode` / `flowpt-output`
  - old v2-specific output option names now fail in config and CLI with explicit migration guidance
  - generator writes enabled edge metadata into the main file and writes full enabled payloads according to the shared output mode
  - standalone post-processing entrypoint is now `analyze_blastwave_vnpt`, computing all harmonics whose edge metadata exists
  - QA recomputes and validates optional v2 and v3 differential payload groups through the shared cumulant core
  - docs, complete Chinese flowpt config, and `project-state/` were refreshed
- Current documentation ownership:
  - detailed runtime and physics explanation: `docs/项目说明.md`
  - formula walkthrough: `docs/数学物理公式流程说明.md`
  - concise semantic reminders: `docs/手记文档.md`
  - current coordination view: `project-state/guide.md`
  - current snapshot and caveats: `project-state/current-status.md`
  - summarized durable validation evidence: `project-state/tests.md`

## Current Contract Reminders

- default runtime path remains V1a `affine-gaussian + covariance-ellipse`
- default initial geometry remains `glauber`; `response-test-023` is opt-in only
- `initial-geometry-a2/a3` are template mixture weights, not `eps2/eps3`
- `events.eps2/psi2` keep covariance semantics; `events.eps3/psi3` use recentered harmonic geometry
- `events.v3`, `v2_wrt_psi2`, and `v3_wrt_psi3` are mandatory ROOT/QA contract fields
- response/cross-talk TH2 storage is still `epsilon = 0..1` and projected `v = -1..1`; the default displayed window is `epsilon = 0..0.35` and projected `v = -0.15..0.15`
- affine-effective remains opt-in and only valid for `affine-gaussian`
- affine-effective defaults to `additive-rho`; use `--affine-effective-mode full-tensor` for the tensor closure path
- V2 `gradient-response` remains opt-in and coupled across medium and flow selection
- differential `v2{2}(pT)` / `v3{2}(pT)` remain separate analysis payloads and do not replace `events.v2/events.v3`
- `flowpt-output-mode = separate-file` may leave the main result file metadata-only
- authoritative ROOT validation on this machine still comes from the outside-sandbox O2Physics path
- `shell_weight` and `EmissionSite::emissionWeight` restructuring were intentionally not part of this rollout

## Remaining Follow-Up

- for physics conclusions from the response test, run a larger controlled `A3` scan and analyze mean `eps3` versus `v3_wrt_psi3`; this rollout provides the closure-test surface
- if the active code worktree changes physics, config parsing, schema, QA behavior, or operator flow again, refresh `docs/项目说明.md`, `docs/agent_guide.md`, and the relevant `project-state/` files in the same patch
- if a future task needs to prove the current dirty tree end to end, rerun the authoritative build, `ctest`, and outside-sandbox ROOT smoke commands, then update `project-state/tests.md` only if the durable conclusion changes

## Verification Status

- durable baseline after this task: `verified` on 2026-05-06
- local full build and full local `ctest` passed after integrating the differential flowpt implementation
- focused ROOT-free `test_differential_flow_cumulant` and `test_run_options` passed
- authoritative outside-sandbox O2Physics same-file generate + QA passed with both v2 and v3 differential payloads in the main result
- authoritative outside-sandbox O2Physics separate-file generate + QA passed with edge metadata in the main result and both full payloads in one flowpt file
- standalone `analyze_blastwave_vnpt --output` and `--inplace` passed under O2Physics ROOT; the in-place result QA passed afterward
- ROOT key inspection confirmed the expected v2/v3 edges, histograms, and canvases in same-file, separate-file, standalone-output, and in-place states
