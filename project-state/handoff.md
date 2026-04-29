# Handoff

## Latest Durable Handoff

- Stage completed: affine-effective before/after density-map output and QA resync
- What changed:
  - affine-effective generation now writes `affine_effective_density_initial_x-y` and `affine_effective_density_final_x-y`
  - both density maps come from the same first valid event and compare the `affine-gaussian` pre-evolution `s0` field against the freeze-out `sf` field
  - both maps default to ROOT `LEGO1` drawing
  - QA now treats the two maps as an all-or-none optional payload and validates TH2 type, finite non-negative bins, positive support, and 3D draw style
  - docs plus `project-state/` were refreshed to describe the output-schema change
- Current documentation ownership:
  - detailed runtime and physics explanation: `docs/项目说明.md`
  - concise semantic reminders: `docs/手记文档.md`
  - current coordination view: `project-state/guide.md`
  - current snapshot and caveats: `project-state/current-status.md`
  - summarized durable validation evidence: `project-state/tests.md`

## Current Contract Reminders

- default runtime path remains V1a `affine-gaussian + covariance-ellipse`
- affine-effective remains opt-in and only valid for `affine-gaussian`
- affine-effective defaults to `additive-rho`; use `--affine-effective-mode full-tensor` for the tensor closure path
- `rho0` affects `additive-rho` but not `full-tensor`; `kappa2` and `density-normal-kappa-compensation` do not affect affine-effective
- affine-effective output includes first-valid-event before/after density maps; they are diagnostic snapshots, not event-averaged density observables
- V2 `gradient-response` remains opt-in and coupled across medium and flow selection
- differential `v2{2}(pT)` remains a separate analysis payload and does not replace `events.v2`
- `v2pt-output-mode = separate-file` may leave the main result file metadata-only
- authoritative ROOT validation on this machine still comes from the outside-sandbox O2Physics path
- `shell_weight` and `EmissionSite::emissionWeight` restructuring were intentionally not part of this rollout

## Remaining Follow-Up

- if the active code worktree changes physics, config parsing, schema, QA behavior, or operator flow again, refresh `docs/项目说明.md`, `docs/agent_guide.md`, and the relevant `project-state/` files in the same patch
- if the next affine step removes `affine-kappa-aniso` compatibility parsing or changes weighting/surface sampling, keep it separate from the current closure-only implementation and record the contract shift explicitly
- if a future task needs to prove the current dirty tree end to end, rerun the authoritative build, `ctest`, and outside-sandbox ROOT smoke commands, then update `project-state/tests.md` only if the durable conclusion changes

## Verification Status

- durable baseline after this task: `verified`
- this task added fresh O2Physics build, local `ctest`, authoritative affine-effective generate+QA, and ROOT file inspection evidence for the two new density maps
