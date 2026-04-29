# Handoff

## Latest Durable Handoff

- Stage completed: opt-in affine-effective closure flow sampler implementation and runtime-contract resync
- What changed:
  - `EventMedium` now carries a dedicated `affineEffectiveClosure` block instead of mixing affine closure diagnostics into `FlowEllipseInfo`
  - `flow-velocity-sampler = affine-effective` is now implemented as an opt-in flow source for `affine-gaussian`
  - public knobs `affine-delta-tau-ref`, `affine-kappa-flow`, `affine-kappa-aniso`, and `affine-u-max` are now parsed and validated
  - `debug-flow-ellipse` now writes affine closure diagnostics when the affine-effective sampler is active, and QA validates those extra fields when present
  - docs plus `project-state/` were refreshed to describe the new sampler and its validation evidence
- Current documentation ownership:
  - detailed runtime and physics explanation: `docs/项目说明.md`
  - concise semantic reminders: `docs/手记文档.md`
  - current coordination view: `project-state/guide.md`
  - current snapshot and caveats: `project-state/current-status.md`
  - summarized durable validation evidence: `project-state/tests.md`

## Current Contract Reminders

- default runtime path remains V1a `affine-gaussian + covariance-ellipse`
- affine-effective remains opt-in and only valid for `affine-gaussian`
- V2 `gradient-response` remains opt-in and coupled across medium and flow selection
- differential `v2{2}(pT)` remains a separate analysis payload and does not replace `events.v2`
- `v2pt-output-mode = separate-file` may leave the main result file metadata-only
- authoritative ROOT validation on this machine still comes from the outside-sandbox O2Physics path
- `shell_weight` and `EmissionSite::emissionWeight` restructuring were intentionally not part of this rollout

## Remaining Follow-Up

- if the active code worktree changes physics, config parsing, schema, QA behavior, or operator flow again, refresh `docs/项目说明.md`, `docs/agent_guide.md`, and the relevant `project-state/` files in the same patch
- if the next affine step changes weighting or surface sampling, keep it separate from the current closure-only implementation and record the contract shift explicitly
- if a future task needs to prove the current dirty tree end to end, rerun the authoritative build, `ctest`, and outside-sandbox ROOT smoke commands, then update `project-state/tests.md` only if the durable conclusion changes

## Verification Status

- durable baseline after this task: `verified`
- this task added fresh local build, `ctest`, default ROOT generate+QA, and affine-effective ROOT generate+QA evidence
