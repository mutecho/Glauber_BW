# Handoff

## Latest Durable Handoff

- Stage completed: flow-trans radius resolution optimization packet
- What changed:
  - added density-normal-only `flow-trans-radius-resolution = balanced | precise | fast`
  - made `balanced = 240 x 256` the default density-defined boundary profile grid
  - kept `precise = 360 x 512` as the old-grid precision baseline and added `fast = 120 x 128`
  - changed `density-percentile` / `density-level` boundary construction to use scalar density queries instead of full density-gradient samples
  - reused percentile cumulative buffers across angular rays and removed per-ray radius-vector allocation
  - moved density-defined `radiusUpperBound` computation into profile cache construction so cache hits do not rescan support points
  - extended the cached profile key with resolution and radial sample count
  - updated parser, validation, help text, tests, complete Chinese configs, active docs, and `project-state/`
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
- `flow-trans-direction-gradient-fraction`, `flow-trans-radius`, and explicit `flow-trans-radius-resolution` are only valid when `flow-velocity-sampler = density-normal`
- `flow-trans-radius-resolution` only changes density-percentile/level profile sampling; it does not change `R(phi)` or `xi = r / R(phi)`, and has no effect on `flow-trans-radius = covariance`
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

- durable baseline after this task: `verified` on 2026-05-07
- local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed
- local full `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 9/9 tests
- `test_run_options` covers resolution parse/default/override/invalid values and density-normal-only validation
- `test_flow_field_model` covers resolution-aware cache rebuild and balanced-vs-precise stability
- O2Physics ROOT executor `PRIMARY_OK` generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg` with:
  - `flow-trans-radius = covariance`
  - `flow-trans-radius = density-percentile:0.95`, `flow-trans-radius-resolution = balanced`
  - `flow-trans-radius = density-percentile:0.95`, `flow-trans-radius-resolution = precise`
  - `flow-trans-radius = density-level:1.0e-3`, `flow-trans-radius-resolution = balanced`
