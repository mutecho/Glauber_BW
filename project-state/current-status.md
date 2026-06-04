# Current Status

## Snapshot

- Date: 2026-06-04
- Repository: `/Users/allenzhou/Research_software/Blast_wave`
- Durable baseline: the current documented runtime contract includes the default V1a path, the opt-in affine-effective closure path with `additive-rho` and `full-tensor` submodes, the opt-in V2 gradient-response path, optional differential `v2/v3{2}(pT)` analysis, event-level `v_n`-`epsilon_n` univariate and multivariate regression notebook analysis, the opt-in fixed/fluctuating `response-test-023` initial-geometry response-test paths with selectable source allocation, and the opt-in density-normal `shell-gradient-corrected` transverse-flow magnitude mode.
- Latest durable verification anchor: 2026-06-03 added opt-in per-event `response-test-023` template fluctuation for source count and `A2/A3/r2x/r2y/r3/sigma3`, extended ROOT `events` with `geo_r2x/geo_r2y`, and added `config/test_023_fluctuating.cfg`. Local build and full `ctest` passed with 10/10 tests; O2Physics ROOT executor generated and QA-validated 5000-event `/private/tmp/test_023_fluctuating.root` with `STATUS: PRIMARY_OK`; final ROOT metric extraction reported fluctuating `eps2 p95=0.362610`, `eps3 p95=0.297453`, both inside the planned broad-distribution window and wider than fixed 023 (`eps2 p95=0.242961`, `eps3 p95=0.148330`).
- Latest diagnostic update: 2026-06-03 added one-harmonic-open control configs `config/test_023_dense_eps2_only_fluct.cfg` and `config/test_023_dense_eps3_only_fluct.cfg`; 5000-event O2Physics generate+QA passed for both, but the `eps3_only` sample still showed `corr(eps2,eps3)=-0.195345` and `corr(eps2,geoA3)=-0.461106`, so the observed broad-fluctuation cross correlation did not disappear under the control test.
- Latest response-test allocation update: 2026-06-03 added opt-in `initial-geometry-source-allocation = independent-pools` and `config/test_023_dense_fluct_independent_pools.cfg`; local build, full 10/10 CTest, and three 5000-event O2Physics generate+QA samples passed, but the decorrelation target failed with full-fluctuation `corr(eps2,eps3)=-0.331139` and eps3-only `corr(eps2,eps3)=-0.316660`.
- Latest conditional-response analysis update: 2026-06-03 added `notebooks/vn_epsn_multivariate_regression.ipynb` and independent-pools configs for dense_mix, newrap, and ellipse; O2Physics generated and QA-validated dense/dense_mix/newrap/ellipse 5000-event ROOT outputs under `qa/`, and the executed notebook read 5000 selected events from each file. The shared geometry had `corr(eps2,eps3)=-0.331139`; raw cross slopes were largely reduced by the conditional fit, with dense `k23=-0.037593`, `k32=-0.029924`, dense_mix `k23=-0.029378`, `k32=-0.028774`, newrap `k23=-0.026321`, `k32=-0.026854`, and ellipse `k23=-0.015840`, `k32=0.031841`.
- Latest lab-frame V2 update: 2026-06-04 added fixed-coordinate `events.v2_lab_x/y` plus matching ROOT histograms, refreshed all 13 notebook input ROOT files from the `scripts/run_b8_v3.sh` config set, and QA-validated every refreshed file with `STATUS: PRIMARY_OK`. The refreshed univariate notebook now reads all nine configured inputs and produces a 36-row lab V2 table; the refreshed multivariate notebook reads all four independent-pools inputs and produces an eight-row lab V2 table. Neither notebook has lab-skip outputs after the refresh.
- Latest user-run-flow update: workspace cfg examples no longer carry inactive response-test, affine-effective, affine-evolution, gradient-response, or shell-gradient knobs outside their selected mode combinations; copied `scripts/run_*.sh` entrypoints now self-resolve their re-entry `script_path`; the maintained notebook entrypoint remains `notebooks/vn_epsn_regression.ipynb`.
- Latest task narrowed the notebook environment and analysis path to `uproot` only, deleting the PyROOT-only comparison notebook; the latest physics task is the density-normal gradient-fraction expansion-compensation fix recorded in DEC-022.

## Current Runtime Baseline

- default medium path:
  - `density-evolution = affine-gaussian`
- default transverse-flow source:
  - `flow-velocity-sampler = covariance-ellipse`
- legacy comparison path:
  - `density-evolution = none`
- opt-in affine-effective closure path:
  - `density-evolution = affine-gaussian`
  - `flow-velocity-sampler = affine-effective`
  - `affine-effective-mode = additive-rho` by default
  - `affine-effective-mode = full-tensor` as opt-in tensor closure
- opt-in V2 path:
  - `density-evolution = gradient-response`
  - `flow-velocity-sampler = gradient-response`
- opt-in response-test geometry path:
  - `initial-geometry = response-test-023`
  - recentered synthetic `0+2+3` transverse point cloud
  - participant records use `nucleus_id = -1`
  - default fixed mode remains the closure baseline
  - `initial-geometry-source-allocation = ratio-total` is the default fixed-total `1:A2:A3` allocation
  - `initial-geometry-source-allocation = independent-pools` uses `source-count` as background pool `N0`, then derives `N2/N3` from `A2/A3`; it is diagnostic and has not passed measured `eps2/eps3` decorrelation
  - optional `initial-geometry-fluctuate = true` samples event-local `source-count`, `A2/A3`, `r2x/r2y`, `r3`, and `sigma3` ranges for broad-distribution response tests
- optional differential-flow path:
  - configure `v2pt-bins`
  - configure `v3pt-bins`
  - choose `flowpt-output-mode = same-file | separate-file`
  - optionally post-process with `analyze_blastwave_vnpt`
- density-normal high-order transverse-flow path:
  - `flow-velocity-sampler = density-normal`
  - `flow-trans-direction-gradient-fraction = 0..1`
  - `flow-trans-direction-gradient-fraction = 0` is pure geometric expansion, `1` is pure density-gradient direction, and `0<f<1` requires the gradient direction to keep at least `1-f` outward projection
  - `flow-trans-radius = covariance | density-percentile:<p> | density-level:<fraction>`
  - `flow-trans-radius-resolution = balanced | precise | fast` for density-defined profile construction only
  - density-defined `R_density(phi)` is a geometry boundary; `xi_shell = r / R_density(phi)` is used for shell lookup
  - main density-defined flow strength uses `xi_flow = q * xi_shell`, with `q = sqrt(-2 log(1-p))` for percentile and `q = sqrt(-2 log(fraction))` for level
  - `flow-trans-magnitude-mode = radius-profile | shell-gradient-corrected`
  - `flow-trans-gradient-strength`, `flow-trans-gradient-density-floor-fraction`, and `flow-trans-gradient-max-factor-delta` for the opt-in shell-gradient correction only
  - `flow-trans-rho0` is the covariance-equivalent `xi_flow = 1` reference rapidity scale, and `flow-trans-profile-power` is the public radius-profile exponent

## Current Contract Highlights

- public entrypoints:
  - `generate_blastwave_events`
  - `qa_blastwave_output`
  - `analyze_blastwave_vnpt`
  - `notebooks/vn_epsn_regression.ipynb` for grouped labelled multi-file event-level `v_n`-`epsilon_n` and `v_n`-`epsilon_m` regression comparisons on ROOT result files through `uproot`
  - `notebooks/vn_epsn_multivariate_regression.ipynb` for independent-pools `v2/v3` conditional response fits against both `eps2` and `eps3`
  - both maintained notebooks also include optional lab-frame `v2_lab_x/y` diagnostics when the input ROOT files have the refreshed lab V2 schema
- canonical tracked example config:
  - `config/test_b8.cfg`
  - `config/test_b8_affine_effective.cfg`
  - `config/test_023_dense.cfg`
  - `config/test_023_dense_mix_glauber.cfg`
  - `config/test_023_dense_newrap_glauber.cfg`
  - `config/test_023_ellipse_glauber.cfg`
  - `config/test_023_fluctuating.cfg`
  - `config/test_023_dense_eps2_only_fluct.cfg`
  - `config/test_023_dense_eps3_only_fluct.cfg`
  - `config/test_023_dense_fluct_independent_pools.cfg`
  - `config/test_023_dense_mix_fluct_independent_pools.cfg`
  - `config/test_023_dense_newrap_fluct_independent_pools.cfg`
  - `config/test_023_ellipse_fluct_independent_pools.cfg`
  - `config/test_b8_flowpt.cfg`
  - `config/test_b8_density_normal_flow_trans.cfg`
  - `config/test_b8_density_normal_flow_trans_gradient.cfg`
- public transverse-flow naming:
  - `rho0` is rejected; use `flow-trans-rho0`
  - `flow-power` is rejected; use `flow-trans-profile-power`
  - `kappa2` is retained as the second-order response coefficient
- mandatory ROOT payload highlights:
  - `events`
  - `participants`
  - `particles`
  - `events.centrality`
  - `events.v2`
  - `events.eps3`
  - `events.psi3`
  - `events.v3`
  - `events.v2_wrt_psi2`
  - `events.v3_wrt_psi3`
  - `events.initial_geometry_mode`
  - `events.geo_r2x`
  - `events.geo_r2y`
  - `events.eps2_f`
  - `events.psi2_f`
  - `events.chi2`
  - `events.r2_0`
  - `events.r2_f`
  - `events.r2_ratio`
  - `particles.x0`
  - `particles.y0`
  - `particles.emission_weight`
  - `events.v2_lab_x`
  - `events.v2_lab_y`
- optional payload groups:
  - `initial_geometry_density_x-y` when `debug-initial-geometry = true`
  - flow-ellipse debug objects, including affine mode encoding, closure diagnostics, and additive-rho surface decomposition when `affine-effective` is selected
  - affine-effective density maps `affine_effective_density_initial_x-y` and `affine_effective_density_final_x-y`, captured as a first-valid-event pair with `LEGO1` default draw options
  - `density_normal_event_density_x-y`
  - V2 gradient debug histograms
  - `v2_2_pt_edges`
  - `v2_2_pt`
  - `v2_2_pt_canvas`
  - `v3_2_pt_edges`
  - `v3_2_pt`
  - `v3_2_pt_canvas`

## Current Verification Picture

- ROOT-free coverage currently includes:
  - `test_maxwell_juttner_sampler`
  - `test_progress_reporter`
  - `test_run_options`
  - `test_flow_field_model`
  - `test_emission_sampler`
  - `test_physics_utils`
  - `test_differential_flow_cumulant`
  - `test_output_path_utils`
  - `test_harmonic_geometry`
  - `test_blast_wave_generator_response`
- durable runtime evidence currently covers:
  - default V1a generation + QA
  - legacy `none` comparison
  - affine `density-normal` default and compensated cases
  - affine-effective `additive-rho` generation + QA with debug-flow-ellipse enabled
  - affine-effective `full-tensor` generation + QA with debug-flow-ellipse enabled
  - affine-effective before/after density map presence and QA validation
  - V2 gradient-response path
  - differential `v2/v3{2}(pT)` same-file and separate-file modes
  - standalone differential post-processing
  - flow-trans naming/parser contract with direct old-name rejection
  - density-normal direction mixing and covariance/percentile/level radius modes
  - density-normal flow-trans radius resolution parsing, cache-key separation, and balanced-vs-precise stability
  - density-normal density-defined radii with sigma-equivalent `xi_flow`, branch-local beta cap coverage, and shell-gradient `xi_shell` outer-shell lookup coverage
  - 1000-event response-test density-normal `newrap` acceptance with `meanPt` at O(1.0) and `psi2_proj` at O(0.02)
  - density-normal shell-gradient-corrected magnitude parsing, validation, ROOT-free behavior, and ROOT generate+QA smoke
  - `chi2` TH1 contract
  - ROOT-free third-harmonic helper and response-template generator coverage
  - default Glauber generation + QA with the expanded third-harmonic schema
  - tuned `config/test_023_dense_newrap_glauber.cfg` 5000-event generation + QA, plus ROOT metric comparison against `dense_mix_glauber` and `ellipse_glauber`
  - `flow-trans-direction-gradient-fraction = 0.8` Glauber generation + QA for ellipse/densemix/newrap, with `f=1.0` densemix/newrap controls showing the cone constraint does not suppress `phi` or `v2{2}(pT)` metrics
  - `response-test-023` generation + QA with optional `initial_geometry_density_x-y`
  - four-point `A3 = 0, 0.05, 0.10, 0.15` response-test scan showing increasing `mean(v3_wrt_psi3)`
  - `response-test-023` cross-harmonic control generation + QA for eps2-only and eps3-only template-weight fluctuations, showing the `eps3_only` geometry correlation persists through the fixed-total `1:A2:A3` source allocation and through the current `independent-pools` diagnostic allocation
  - lab-frame V2 schema smoke generation + QA, including `events.v2_lab_x/y` branches, matching histograms, and particle-level recomputation checks

Use `project-state/tests.md` for the summarized evidence trail.

## Active Caveats

- sandboxed `alienv` ROOT smoke output on this machine is not authoritative when PCM / module noise appears
- historical files under `qa/` span multiple schema generations and may not match the latest contract
- `flowpt-output-mode = separate-file` intentionally allows a metadata-only main result file that keeps enabled `v2_2_pt_edges` / `v3_2_pt_edges` but omits the full analysis payload
- `events.eps2` / `events.psi2` remain initial-state observables, while `eps2_f` / `psi2_f` / `chi2` remain freeze-out diagnostics
- `events.eps3` / `events.psi3` use the recentered harmonic convention and do not change the covariance `eps2/psi2` contract
- `response-test-023` is opt-in only; template weights `A2/A3` are not physical eccentricities
- `initial-geometry-fluctuate` is also opt-in and only valid for `response-test-023`; fixed 023 and fluctuating 023 have different roles, with fixed as closure baseline and fluctuating as broad-distribution response test
- default `response-test-023` source allocation uses one fixed total source pool with approximate fractions `1:A2:A3`; independently sampling `A2/A3` therefore does not guarantee independent measured `events.eps2/eps3`, and an `A3`-only fluctuation can still anti-correlate with `events.eps2`
- `initial-geometry-source-allocation = independent-pools` removes fixed-total source competition but still failed the measured decorrelation target because total `Npart` and shared radial-moment denominators vary with `A2/A3`
- response/cross-talk TH2 objects keep full storage ranges `epsilon = 0..1` and projected `v = -1..1`, but open with compact default display ranges `epsilon = 0..0.35` and projected `v = -0.15..0.15`
- event-level `v_n`-`epsilon_n` regression should use `events.v2_wrt_psi2` / `events.v3_wrt_psi3` against `events.eps2` / `events.eps3`; `events.v2_lab_x/y` are fixed-lab-coordinate sign/orientation diagnostics, not replacement response observables. `notebooks/vn_epsn_regression.ipynb` uses `uproot`, supports grouped `INPUT_FILE_GROUPS` overlays, keeps Glauber-direct, fixed manual third-order response-test, and 023 random-fluctuation response-test inputs in separate plot groups, keeps same-harmonic free-intercept plus through-origin response fits, uses free-intercept-only fits for cross-harmonic `v2/eps3` and `v3/eps2` mixing checks, conditionally adds lab V2 diagnostics when the branches exist, and no longer depends on PyROOT
- `shell_weight` and any `EmissionSite::emissionWeight` restructuring remain intentionally deferred; the current response-test rollout only adds geometry templates and observables
- older validation ledgers may cite pre-archive response-test config names; current tracked response-test examples use `config/test_023_dense*.cfg`

## Current Documentation Layout

- documentation index stays in `docs/README.md`
- detailed runtime explanation stays in `docs/项目说明.md`
- formula-heavy workflow explanation stays in `docs/数学物理公式流程说明.md`
- quick semantic reminders stay in `docs/手记文档.md`
- historical design and handoff plans stay under `docs/PLAN/` and are skipped by default during closeout
- current agent-facing workflow rules stay in `AGENTS.md`
- deterministic documentation routing stays in `project-state/doc-sync-map.yml`
- current coordination state stays in `project-state/guide.md`, this file, and `project-state/handoff.md`
- detailed raw command transcripts were intentionally removed from `project-state/tests.md`; only durable conclusions remain
