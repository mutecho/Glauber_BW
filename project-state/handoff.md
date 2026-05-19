# Handoff

## Latest Durable Handoff

- Stage completed: sigma-equivalent density-defined flow-trans radius fix from the historical plan now archived at `docs/PLAN/半径再次修复.md`
- What changed:
  - split density-defined radius handling into `xi_shell = r / R_density(phi)` and `xi_flow = q * xi_shell`
  - mapped `density-percentile:p` to `q = sqrt(-2 * log1p(-p))` and `density-level:fraction` to `q = sqrt(-2 * log(fraction))`
  - changed the main density-defined transverse rapidity profile to use `xi_flow`
  - kept invalid density-defined profiles on the existing covariance `rTilde` fallback path
  - kept `sampleFlowTransGradientCorrectionValue()` lookup clamped to the correction-table domain `0 <= xi_shell <= 1`; `xi_shell > 1` reuses the outer shell correction while the base `rhoRaw` uses `xi_flow`
  - tightened `density-level` parsing and generator validation to require `0 < fraction < 1`
  - updated `test_flow_field_model` to cover hard-coded q oracles, `DensityPercentile`, `DensityLevel`, branch-local beta cap, zero-strength shell-gradient equality, and positive-strength shell-gradient `xi_shell`/`xi_flow` separation
  - updated `test_run_options` to cover `density-level:1.0` / `1.2` rejection text
  - updated active docs, relevant design notes, and `project-state/`
  - kept the ROOT schema unchanged
  - did not add or change config keys or example cfg files
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
- Glauber mirror configs exist for the response-test comparison settings: `config/test_023_dense_mix_glauber.cfg`, `config/test_023_dense_newrap_glauber.cfg`, and `config/test_023_ellipse_glauber.cfg`
- workspace cfg examples are intentionally mode-local: do not re-add inactive response-test, affine-effective, affine-evolution, gradient-response, or shell-gradient knobs unless the selected mode combination uses them
- affine-effective remains opt-in and only valid for `affine-gaussian`
- V2 `gradient-response` remains opt-in and coupled across medium and flow selection
- `flow-trans-rho0` and `flow-trans-profile-power` are the current transverse rapidity baseline/profile names; `rho0` and `flow-power` are rejected
- `flow-trans-direction-gradient-fraction`, `flow-trans-radius`, and explicit `flow-trans-radius-resolution` are only valid when `flow-velocity-sampler = density-normal`
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
- event-level `v_n`-`epsilon_n` regression lives in `notebooks/vn_epsn_regression.ipynb`; it reads `events.eps2/eps3` and `events.v2_wrt_psi2/v3_wrt_psi3`, uses `uproot`, supports labelled `INPUT_FILES` multi-file overlays, and no longer depends on PyROOT
- `shell_weight` and `EmissionSite::emissionWeight` restructuring remain intentionally deferred

## Remaining Follow-Up

- if direct PyROOT analysis is needed again, add it as a separate opt-in environment/notebook path instead of putting ROOT/PyROOT back into the maintained `root_notebook` recipe
- for physics conclusions from shell-gradient scans, run larger-statistics response-test or Glauber scans and compare \(p_T\), `v2`, `v3`, and geometry-plane projections against radius-profile controls
- if future work needs to inspect correction internals, add an explicit ROOT-free or optional-debug payload design first; this packet intentionally kept ROOT schema unchanged
- if the active code worktree changes physics, config parsing, schema, QA behavior, or operator flow again, use `project-state/doc-sync-map.yml` to refresh the required current docs and relevant `project-state/` files in the same patch
- older validation records may cite pre-archive or pre-rename document/config paths; current tracked response-test examples use `config/test_023_dense*.cfg`, and historical plans live under `docs/PLAN/`

## Verification Status

- latest notebook packet: `checked after maintenance` on 2026-05-13 for the `uproot`-only `root_notebook` recipe and maintained `notebooks/vn_epsn_regression.ipynb`; the latest full notebook execution evidence remains the 2026-05-11 multi-file regression smoke over `response_023`, `dense_mix`, and `newrap`
- durable generator baseline remains `verified` on 2026-05-07
- full event-tree notebook execution was last verified in the existing `root_notebook` environment before the recipe was narrowed; recreate/update it from `notebooks/environment-notebook_with_root.yml` to use the current `uproot`-only stack
- local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed
- local full `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 9/9 tests
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
