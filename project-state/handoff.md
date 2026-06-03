# Handoff

## Latest Durable Handoff

- Stage completed: `response-test-023` one-harmonic fluctuation cross-check audit
- What changed:
  - added `config/test_023_dense_eps2_only_fluct.cfg` and `config/test_023_dense_eps3_only_fluct.cfg` as complete Chinese control configs
  - generated and QA-validated 5000-event ROOT controls for both configs through the O2Physics executor
  - extracted event-level cross statistics for the current broad fluctuating reference and both one-harmonic controls
  - found that the `eps3_only` control still has `corr(eps2,eps3)=-0.195345` and `corr(eps2,geoA3)=-0.461106`
  - concluded that the current cross correlation did not disappear under the control test, so the conditional branch for immediately implementing stratified-independent sampling was not taken
  - updated active docs and `project-state/` with the mechanism: current fixed-total `1:A2:A3` source allocation couples measured `events.eps2/eps3`
- Current documentation ownership:
  - documentation index and role map: `docs/README.md`
  - detailed runtime and physics explanation: `docs/项目说明.md`
  - formula walkthrough: `docs/数学物理公式流程说明.md`
  - historical design and old handoffs: `docs/PLAN/`
  - concise semantic reminders: `docs/手记文档.md`
  - agent workflow policy: `AGENTS.md`
  - documentation sync routing: `project-state/doc-sync-map.yml`
  - current coordination view: `project-state/guide.md`
  - current snapshot and caveats: `project-state/current-status.md`
  - summarized durable validation evidence: `project-state/tests.md`

## Current Contract Reminders

- default runtime path remains V1a `affine-gaussian + covariance-ellipse`
- default initial geometry remains `glauber`; `response-test-023` is opt-in only
- fixed `response-test-023` remains the closure baseline; fluctuating `response-test-023` is the broad-distribution response-test path
- `initial-geometry-fluctuate=true` requires all response-test range keys and is invalid under Glauber
- `events.geo_a2/geo_a3/geo_r2x/geo_r2y/geo_r3/geo_sigma3` are event-local template-parameter snapshots, not measured eccentricities
- `A2/A3` are mixed through one fixed total source pool with approximate fractions `1:A2:A3`; independent template-weight random numbers do not guarantee independent measured `events.eps2/eps3`
- use `config/test_023_dense_eps2_only_fluct.cfg` and `config/test_023_dense_eps3_only_fluct.cfg` for one-harmonic-open cross-correlation controls before interpreting broad fluctuating `v2/eps3` or `v3/eps2` slopes
- Glauber mirror configs exist for the response-test comparison settings: `config/test_023_dense_mix_glauber.cfg`, `config/test_023_dense_newrap_glauber.cfg`, and `config/test_023_ellipse_glauber.cfg`
- workspace cfg examples are intentionally mode-local: do not re-add inactive response-test, affine-effective, affine-evolution, gradient-response, or shell-gradient knobs unless the selected mode combination uses them
- affine-effective remains opt-in and only valid for `affine-gaussian`
- V2 `gradient-response` remains opt-in and coupled across medium and flow selection
- `flow-trans-rho0` and `flow-trans-profile-power` are the current transverse rapidity baseline/profile names; `rho0` and `flow-power` are rejected
- `flow-trans-direction-gradient-fraction`, `flow-trans-radius`, and explicit `flow-trans-radius-resolution` are only valid when `flow-velocity-sampler = density-normal`
- `flow-trans-direction-gradient-fraction = 0` is pure geometric expansion, `1` is pure local density-gradient direction, and `0<f<1` constrains the gradient direction to a global-expansion cone with minimum outward projection `1-f`
- `flow-trans-radius-resolution` only changes density-percentile/level profile sampling; it does not change `R_density(phi)`, `xi_shell`, `q`, or `xi_flow`, and has no effect on `flow-trans-radius = covariance`
- for density-defined radii, `R_density(phi)` is a geometry boundary and `flow-trans-rho0` is the covariance-equivalent `xi_flow = 1` reference rapidity scale, not a global maximum
- `flow-trans-magnitude-mode = radius-profile` is the default
- `flow-trans-magnitude-mode = shell-gradient-corrected` requires `density-normal + density-percentile/level`
- explicit `flow-trans-gradient-*` knobs require `shell-gradient-corrected`
- `flow-trans-gradient-max-factor-delta` limits the multiplicative factor offset `delta`, not an additive rapidity increment
- `shell-gradient-corrected` clamps only correction-table lookup to the outer shell for `xi_shell > 1`; the base `rhoRaw` uses `xi_flow`
- `shell-gradient-corrected` does not add ROOT debug maps or schema objects in this packet
- authoritative ROOT validation on this machine still comes from the O2Physics executor path
- copied `scripts/run_*.sh` entrypoints self-resolve `script_path` from `BASH_SOURCE[0]`, so new run-script copies do not need a manual re-entry path edit
- event-level `v_n`-`epsilon_n` regression lives in `notebooks/vn_epsn_regression.ipynb`; it reads `events.eps2/eps3` and `events.v2_wrt_psi2/v3_wrt_psi3`, uses `uproot`, supports grouped `INPUT_FILE_GROUPS` multi-file overlays, keeps Glauber-direct, fixed manual third-order response-test, and 023 random-fluctuation response-test inputs in separate plot groups, keeps through-origin fits only for same-harmonic response summaries, and no longer depends on PyROOT
- `shell_weight` and `EmissionSite::emissionWeight` restructuring remain intentionally deferred

## Remaining Follow-Up

- if the goal is to remove the induced `eps2/eps3` geometry correlation, add an explicit sampling mode with independent component source pools or post-sampled stratified matching, then require a generated control sample with `|corr(eps2,eps3)| < 0.08`
- if direct PyROOT analysis is needed again, add it as a separate opt-in environment/notebook path instead of putting ROOT/PyROOT back into the maintained `root_notebook` recipe
- for physics conclusions from shell-gradient scans, run larger-statistics response-test or Glauber scans and compare \(p_T\), `v2`, `v3`, and geometry-plane projections against radius-profile controls
- if future work needs to inspect correction internals, add an explicit ROOT-free or optional-debug payload design first; this packet intentionally kept ROOT schema unchanged
- if the active code worktree changes physics, config parsing, schema, QA behavior, or operator flow again, use `project-state/doc-sync-map.yml` to refresh the required current docs and relevant `project-state/` files in the same patch
- older validation records may cite pre-archive or pre-rename document/config paths; current tracked response-test examples use `config/test_023_dense*.cfg`, and historical plans live under `docs/PLAN/`

## Verification Status

- latest one-harmonic cross-check packet is `verified` diagnostically on 2026-06-03
- O2Physics ROOT executor generated and QA-validated `/private/tmp/blastwave_eps3_only_fluct.root` with 5000 events, `STATUS: PRIMARY_OK`; final event-stat extraction reported `corr(eps2,eps3)=-0.195345` and `corr(eps2,geoA3)=-0.461106`
- O2Physics ROOT executor generated and QA-validated `/private/tmp/blastwave_eps2_only_fluct.root` with 5000 events, `STATUS: PRIMARY_OK`; final event-stat extraction reported `corr(eps2,eps3)=-0.080169`
- latest fluctuating response-test packet is `verified` on 2026-06-03
- local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed after the fluctuating 023 rollout
- local full `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 10/10 tests
- O2Physics ROOT executor generated and QA-validated `/private/tmp/test_023_fluctuating.root` with 5000 events, `STATUS: PRIMARY_OK`
- final ROOT metric extraction reported fluctuating `eps2 p95=0.362610`, `eps3 p95=0.297453`, and fixed 023 comparison `eps2 p95=0.242961`, `eps3 p95=0.148330`
- latest notebook packet: full `root_notebook` execution on 2026-06-03 passed for grouped `glauber_direct`, `manual_third_order`, and `response_023_fluct` analyses, each with `dense_mix`, `newrap`, and `ellipse`
- latest density-normal gradient-fraction expansion-compensation fix is `verified` on 2026-05-19
- durable generator baseline remains `verified` on 2026-05-07
- O2Physics ROOT executor regenerated 5000-event `/private/tmp/test_023_ellipse_glauber.root`, `/private/tmp/test_023_dense_mix_glauber.root`, and `/private/tmp/test_023_newrap_glauber.root` from the three Glauber configs with `STATUS: PRIMARY_OK`
- O2Physics ROOT executor QA passed for all three regenerated files, each reporting `validation_passed events=5000`
- final ROOT metric comparison reported ellipse `phiA2=0.0576875`, `mean(v2)=0.0878417`, densemix `phiA2=0.0309274`, `mean(v2)=0.0761202`, and newrap `phiA2=0.0374482`, `mean(v2)=0.0877322`
- `v2_2_pt` RMSE to ellipse was `0.022872` for densemix and `0.0210139` for newrap
- explicit `f=1.0` controls for densemix/newrap matched the new `f=0.8` outputs on `phiA2`, `mean(v2)`, `mean(v2_wrt_psi2)`, and `v2_2_pt` RMSE, confirming the suppression came from the old direction blend
- full event-tree notebook execution was last verified in the existing `root_notebook` environment before the recipe was narrowed; recreate/update it from `notebooks/environment-notebook_with_root.yml` to use the current `uproot`-only stack
- local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed
- local full `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 10/10 tests
- focused `/Users/allenzhou/Research_software/Blast_wave/bin/test_flow_field_model` passed
- focused `/Users/allenzhou/Research_software/Blast_wave/bin/test_run_options` passed
- O2Physics ROOT executor generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-percentile:0.95 --nevents 20`, written to `/private/tmp/blastwave_radius_sigma_percentile.root`
- O2Physics ROOT executor generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-level:1.0e-3 --nevents 20`, written to `/private/tmp/blastwave_radius_sigma_level.root`
- O2Physics ROOT executor generate + QA passed for `config/test_b8_response_023_dense_mix.cfg --nevents 1000`, written to `/private/tmp/blastwave_response_023_dense_mix_sigma.root`; current tracked equivalent path is `config/test_023_dense_mix.cfg`
- O2Physics ROOT executor generate + QA passed for `config/test_b8_023_dense_newrap.cfg --nevents 1000`, written to `/private/tmp/blastwave_response_023_dense_newrap_sigma.root`; current tracked equivalent path is `config/test_023_dense_newrap.cfg`
- O2Physics ROOT executor generate + QA passed for `config/test_023_dense_mix_glauber.cfg --nevents 1000`, written to `/private/tmp/blastwave_test_023_dense_mix_glauber.root`
- O2Physics ROOT executor generate + QA passed for `config/test_023_dense_newrap_glauber.cfg --nevents 1000`, written to `/private/tmp/blastwave_test_023_dense_newrap_glauber.root`
- O2Physics ROOT executor generate + QA passed for `config/test_023_ellipse_glauber.cfg --nevents 1000`, written to `/private/tmp/blastwave_test_023_ellipse_glauber.root`
- O2Physics ROOT executor generate + QA passed for every workspace `config/*.cfg` file after config cleanup, using `--nevents 1000 --no-progress` and `/private/tmp/blastwave_cfg_cleanup_*.root`
- ROOT metric extraction reported `dense_mix meanPt=1.041669379 psi2_proj=0.022560737` and `newrap meanPt=1.067941944 psi2_proj=0.022885878`
