# Handoff

## Latest Durable Handoff

- Stage completed: flow-trans naming and density-normal radius/direction first packet
- What changed:
  - replaced public `rho0` / `flow-power` with `flow-trans-rho0` / `flow-trans-profile-power`
  - old `rho0` and `flow-power` now fail in config and CLI with explicit migration guidance
  - kept `kappa2` as the public second-order response coefficient
  - added density-normal-only `flow-trans-direction-gradient-fraction`
  - added density-normal-only `flow-trans-radius = covariance | density-percentile:<p> | density-level:<fraction>`
  - added `EventMedium::emissionDensityScale` plus an event-level angular boundary profile cache for density-defined radii
  - added `config/test_b8_density_normal_flow_trans.cfg` as the complete Chinese explained flow-trans example
  - migrated tracked configs and refreshed active docs plus `project-state/`
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
- `flow-trans-rho0` and `flow-trans-profile-power` are the current transverse rapidity baseline/profile names; `rho0` and `flow-power` are rejected
- `flow-trans-direction-gradient-fraction` and `flow-trans-radius` are only valid when `flow-velocity-sampler = density-normal`
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
- local full build and full local `ctest` passed after integrating the flow-trans naming/radius implementation
- `test_run_options` and `test_flow_field_model` carry the focused ROOT-free coverage for this packet
- authoritative outside-sandbox O2Physics default generate + QA passed for `config/test_b8.cfg --nevents 20`
- authoritative outside-sandbox O2Physics generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg` with `flow-trans-radius = covariance`, `density-percentile:0.95`, and `density-level:1.0e-3`
- sandboxed ROOT run hit known PCM/module noise; do not use it as the evidence source
