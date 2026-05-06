# Handoff

## Latest Durable Handoff

- Stage completed: response-test `0+2+3` initial geometry and third-harmonic output contract
- What changed:
  - added `initial-geometry = glauber | response-test-023`, defaulting to `glauber`
  - `response-test-023` creates a recentered synthetic `0+2+3` transverse point cloud and writes participant records with `nucleus_id = -1`
  - added template knobs `initial-geometry-source-count`, `r0`, `a2`, `r2x/r2y/phi2`, `a3`, `r3/sigma3/phi3`, plus `debug-initial-geometry`
  - added mandatory event branches and histograms for `eps3/psi3`, `v3`, `v2_wrt_psi2`, and `v3_wrt_psi3`
  - added response/cross-talk TH2 objects and optional `initial_geometry_density_x-y`
  - QA now accepts `nucleus_id=-1` only for response-test events and recomputes weighted Q2/Q3 observables from `particles`
  - docs, complete Chinese response-test config, and `project-state/` were refreshed
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
- affine-effective remains opt-in and only valid for `affine-gaussian`
- affine-effective defaults to `additive-rho`; use `--affine-effective-mode full-tensor` for the tensor closure path
- V2 `gradient-response` remains opt-in and coupled across medium and flow selection
- differential `v2{2}(pT)` remains a separate analysis payload and does not replace `events.v2`
- `v2pt-output-mode = separate-file` may leave the main result file metadata-only
- authoritative ROOT validation on this machine still comes from the outside-sandbox O2Physics path
- `shell_weight` and `EmissionSite::emissionWeight` restructuring were intentionally not part of this rollout

## Remaining Follow-Up

- for physics conclusions from the response test, run a larger controlled `A3` scan and analyze mean `eps3` versus `v3_wrt_psi3`; this rollout provides the closure-test surface
- if the active code worktree changes physics, config parsing, schema, QA behavior, or operator flow again, refresh `docs/项目说明.md`, `docs/agent_guide.md`, and the relevant `project-state/` files in the same patch
- if a future task needs to prove the current dirty tree end to end, rerun the authoritative build, `ctest`, and outside-sandbox ROOT smoke commands, then update `project-state/tests.md` only if the durable conclusion changes

## Verification Status

- durable baseline after this task: `verified` on 2026-05-06
- local build and full local `ctest` passed after integrating the response-test implementation
- authoritative outside-sandbox O2Physics default Glauber generate + QA passed with the expanded third-harmonic schema
- authoritative outside-sandbox O2Physics `response-test-023` generate + QA passed, including optional initial-geometry density output
- four-point `A3 = 0, 0.05, 0.10, 0.15` scan passed generate + QA and showed increasing `mean(v3_wrt_psi3)`
