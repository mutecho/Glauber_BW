# Handoff

## Latest Durable Handoff

- Stage completed: shell-gradient-corrected density-normal flow-magnitude packet from `docs/高阶半径补充.md`
- What changed:
  - added `flow-trans-magnitude-mode = radius-profile | shell-gradient-corrected`
  - kept `radius-profile` as the default behavior
  - added `flow-trans-gradient-strength`, `flow-trans-gradient-density-floor-fraction`, and `flow-trans-gradient-max-factor-delta`
  - implemented an event-level shell-gradient correction cache that reuses density-defined `R(phi)`, builds the cumulative gradient proxy, applies event RMS normalization, subtracts same-shell means, and clamps the multiplicative `delta`
  - restricted corrected mode to `flow-velocity-sampler = density-normal` with `flow-trans-radius = density-percentile:<p>` or `density-level:<fraction>`
  - rejected explicit `flow-trans-gradient-*` knobs unless corrected mode is selected
  - added `config/test_b8_density_normal_flow_trans_gradient.cfg`
  - kept the ROOT schema unchanged
  - updated parser, validation, help text, tests, active docs, and `project-state/`
- Current documentation ownership:
  - detailed runtime and physics explanation: `docs/项目说明.md`
  - formula walkthrough: `docs/数学物理公式流程说明.md`
  - high-order design/current contract: `docs/高阶半径与梯度混合整合方案.md`
  - concise semantic reminders: `docs/手记文档.md`
  - current coordination view: `project-state/guide.md`
  - current snapshot and caveats: `project-state/current-status.md`
  - summarized durable validation evidence: `project-state/tests.md`

## Current Contract Reminders

- default runtime path remains V1a `affine-gaussian + covariance-ellipse`
- default initial geometry remains `glauber`; `response-test-023` is opt-in only
- affine-effective remains opt-in and only valid for `affine-gaussian`
- V2 `gradient-response` remains opt-in and coupled across medium and flow selection
- `flow-trans-rho0` and `flow-trans-profile-power` are the current transverse rapidity baseline/profile names; `rho0` and `flow-power` are rejected
- `flow-trans-direction-gradient-fraction`, `flow-trans-radius`, and explicit `flow-trans-radius-resolution` are only valid when `flow-velocity-sampler = density-normal`
- `flow-trans-radius-resolution` only changes density-percentile/level profile sampling; it does not change `R(phi)` or `xi = r / R(phi)`, and has no effect on `flow-trans-radius = covariance`
- `flow-trans-magnitude-mode = radius-profile` is the default
- `flow-trans-magnitude-mode = shell-gradient-corrected` requires `density-normal + density-percentile/level`
- explicit `flow-trans-gradient-*` knobs require `shell-gradient-corrected`
- `flow-trans-gradient-max-factor-delta` limits the multiplicative factor offset `delta`, not an additive rapidity increment
- `shell-gradient-corrected` does not add ROOT debug maps or schema objects in this packet
- authoritative ROOT validation on this machine still comes from the O2Physics executor path
- `shell_weight` and `EmissionSite::emissionWeight` restructuring remain intentionally deferred

## Remaining Follow-Up

- for physics conclusions from shell-gradient scans, run larger-statistics response-test or Glauber scans and compare \(p_T\), `v2`, `v3`, and geometry-plane projections against radius-profile controls
- if future work needs to inspect correction internals, add an explicit ROOT-free or optional-debug payload design first; this packet intentionally kept ROOT schema unchanged
- if the active code worktree changes physics, config parsing, schema, QA behavior, or operator flow again, refresh `docs/项目说明.md`, `docs/agent_guide.md`, and the relevant `project-state/` files in the same patch

## Verification Status

- durable baseline after this task: `verified` on 2026-05-07
- local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed
- local full `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 9/9 tests
- focused `./bin/test_run_options` passed
- focused `./bin/test_flow_field_model` passed
- O2Physics ROOT executor `PRIMARY_OK` generate + QA passed for `config/test_b8_density_normal_flow_trans_gradient.cfg --nevents 20`, written to `qa/test_flow_trans_shell_gradient.root`
- O2Physics ROOT executor `PRIMARY_OK` generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --nevents 20`, written to `qa/test_flow_trans_radius_profile_control.root`
