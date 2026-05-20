# Tests

## Durable Verification Baseline

- Status: verified
- Last comprehensive refresh: 2026-05-07
- Evidence shape:
  - local rebuild of touched binaries and regression targets passed
  - local `ctest` passed
  - authoritative outside-sandbox ROOT generate+QA smokes passed
  - standalone differential analysis passed where relevant
  - ROOT key inspection was used only when output placement semantics mattered

This file now keeps durable conclusions only.
Long command transcripts and repeated smoke-command variants were intentionally removed during documentation compaction.

## Command Families Still Used

- `cmake --build ...`
- `cd build && ctest --output-on-failure`
- authoritative outside-sandbox `generate_blastwave_events ...`
- authoritative outside-sandbox `qa_blastwave_output ...`
- authoritative outside-sandbox `analyze_blastwave_vnpt ...`
- ROOT key inspection through the shared inspector scripts when payload placement needs confirmation

## T-035 Density-Normal Gradient Fraction Expansion Compensation

- Status: passed on 2026-05-19
- Evidence:
  - local `cmake --build build -j4` passed after changing `src/FlowFieldModel.cpp`, `tests/FlowFieldModelTest.cpp`, and the stale progress reporter assertion
  - local `ctest --test-dir build --output-on-failure` passed with 10/10 tests
  - `test_flow_field_model` now covers the pure geometric `f=0` limit, preservation of density-gradient directions already inside the global-expansion cone, cone-boundary projection for hotspot-induced inward gradients with preserved tangential direction, and `shell-gradient-corrected` scaling only the gradient correction by the fraction
  - O2Physics ROOT executor first returned `STATUS: ESCALATION_REQUIRED` inside the sandbox; rerunning the same executor command outside the sandbox returned `STATUS: PRIMARY_OK`
  - O2Physics ROOT executor generated 5000-event `/private/tmp/test_023_ellipse_glauber.root`, `/private/tmp/test_023_dense_mix_glauber.root`, and `/private/tmp/test_023_newrap_glauber.root`
  - QA passed for all three regenerated files with `validation_passed events=5000`
  - ROOT metric comparison reported ellipse `phiA2=0.0576875`, `mean(v2)=0.0878417`, `mean(v2_wrt_psi2)=0.038566`
  - the same comparison reported densemix `f=0.8` `phiA2=0.0311146`, `mean(v2)=0.0764014`, `mean(v2_wrt_psi2)=0.0190202`, and `v2_2_pt` RMSE to ellipse `0.0225366`
  - the same comparison reported newrap `f=0.8` `phiA2=0.0378503`, `mean(v2)=0.0883567`, `mean(v2_wrt_psi2)=0.0229422`, and `v2_2_pt` RMSE to ellipse `0.0207931`
  - explicit densemix/newrap `--flow-trans-direction-gradient-fraction 1.0` controls reported densemix `phiA2=0.0309274`, `mean(v2)=0.0761202`, `mean(v2_wrt_psi2)=0.0189164`, RMSE `0.022872`; newrap `phiA2=0.0374482`, `mean(v2)=0.0877322`, `mean(v2_wrt_psi2)=0.0226999`, RMSE `0.0210139`
- Locked conclusions:
  - the old suppression came from treating `1-f` as a direction-vector blend with geometric expansion, which diluted density-gradient shape
  - the current contract uses density gradients for ellipse/triangle/hotspot shape and treats `1-f` as a minimum outward projection for global expansion, not a manual anisotropy coefficient
  - lowering densemix/newrap Glauber configs to `flow-trans-direction-gradient-fraction = 0.8` no longer suppresses their `radius-profile` phi/v2 metrics relative to the `1.0` controls

## T-034 Newrap Glauber Phi And `v2{2}(pT)` Retuning

- Status: passed on 2026-05-19
- Evidence:
  - compared current 5000-event `qa/test_023_newrap_glauber.root`, `qa/test_023_dense_mix_glauber.root`, and `qa/test_023_ellipse_glauber.root` with an O2Physics ROOT macro over `phi`, `events`, and `v2_2_pt`
  - baseline `newrap` before tuning had `phi` second-harmonic amplitude `0.012003353`, `mean(v2)=0.057891413`, `mean(v2_wrt_psi2)=0.0073743805`, and `v2_2_pt` RMSE to ellipse `0.05471767` over the common 9 bins
  - 1000-event candidate scans showed the best interpretable newrap direction was density-normal with covariance radius, pure density-gradient direction, `flow-trans-rho0 = 1.2`, `flow-trans-profile-power = 1.0`, `smear = 0.5`, and matched `v2/v3{2}(pT)` bins through 7 GeV
  - regenerated `qa/test_023_newrap_glauber.root` from `config/test_023_dense_newrap_glauber.cfg` with O2Physics ROOT executor `STATUS: PRIMARY_OK`, writing 5000 events
  - `qa_blastwave_output --input qa/test_023_newrap_glauber.root --expect-nevents 5000` returned `validation_passed events=5000`
  - final tuned `newrap` reported `phi` second-harmonic amplitude `0.037448246`, `mean(v2)=0.087732208`, `mean(v2_wrt_psi2)=0.02269988`, and 11-bin `v2_2_pt` RMSE to ellipse `0.02101393`
  - the same final comparison reported `densemix` 11-bin `v2_2_pt` RMSE to ellipse `0.02287196` and ellipse `phi` second-harmonic amplitude `0.057687497`
- Locked conclusions:
  - tuned `config/test_023_dense_newrap_glauber.cfg` moves both final-state `phi` and differential `v2{2}(pT)` toward the ellipse reference without changing generator code, public config keys, ROOT schema, or QA schema
  - the tuning keeps the model interpretable as density-normal flow, but uses the covariance radius scale for direct comparison with the ellipse reference and raises `flow-trans-rho0` because `density-normal + none` does not consume the explicit `kappa2` modulation
  - densemix remained unchanged because its existing behavior was already close to the ellipse reference

## T-033 Mode-Local Config Cleanup

- Status: passed on 2026-05-19
- Evidence:
  - removed active cfg entries that were unused by their selected mode combination, including response-test template knobs in Glauber configs, affine-evolution knobs outside `affine-gaussian`, affine-effective knobs outside `affine-effective`, gradient-response knobs outside paired `gradient-response`, shell-gradient knobs outside `shell-gradient-corrected`, and legacy/no-op `affine-kappa-aniso`
  - `git diff --check` passed after config cleanup
  - O2Physics ROOT executor generated 1000-event `/private/tmp/blastwave_cfg_cleanup_*.root` outputs for all 18 workspace `config/*.cfg` files; the first batch returned `STATUS: COMMAND_FAILED` only after `config/test_b8_gradient.cfg` exposed an inline-comment parse issue, and the fixed continuation returned `STATUS: PRIMARY_OK`
  - O2Physics ROOT executor QA passed for every generated `/private/tmp/blastwave_cfg_cleanup_*.root` output, reporting `validation_passed events=1000` for all 18 cfg files
- Locked conclusions:
  - workspace cfg examples are mode-local and executable after cleanup
  - no public config key, parser behavior, generator code, ROOT schema, QA schema, or physics algorithm changed

## T-032 Run Script Self-Reentry Path

- Status: passed on 2026-05-19
- Evidence:
  - `bash -n` passed for `scripts/run_example_config.sh`, `scripts/run_test.sh`, `scripts/run_newrep_test.sh`, and `scripts/run_b8_v3.sh`
  - a copied temporary probe script resolved `script_path` to its own copied filename through the same `BASH_SOURCE[0]` pattern
  - `git diff --check` passed after the script and project-state updates
- Locked conclusions:
  - copied `scripts/run_*.sh` entrypoints re-enter the O2Physics runtime through their own script path instead of a hard-coded source script
  - no generator code, public config key, ROOT schema, QA schema, or physics behavior changed

## T-031 Glauber Mirror Configs For Response-Test Comparisons

- Status: passed on 2026-05-19
- Evidence:
  - `git diff --check` passed after adding the mirror configs and project-state updates
  - new configs preserve their source config values except for the explanatory header, `initial-geometry = glauber`, and the `_glauber.root` output filename
  - O2Physics ROOT executor returned `STATUS: PRIMARY_OK` for `config/test_023_dense_mix_glauber.cfg --nevents 1000`, writing `/private/tmp/blastwave_test_023_dense_mix_glauber.root`
  - O2Physics ROOT executor QA passed for `/private/tmp/blastwave_test_023_dense_mix_glauber.root`, reporting `validation_passed events=1000`
  - O2Physics ROOT executor returned `STATUS: PRIMARY_OK` for `config/test_023_dense_newrap_glauber.cfg --nevents 1000`, writing `/private/tmp/blastwave_test_023_dense_newrap_glauber.root`
  - O2Physics ROOT executor QA passed for `/private/tmp/blastwave_test_023_dense_newrap_glauber.root`, reporting `validation_passed events=1000`
  - O2Physics ROOT executor returned `STATUS: PRIMARY_OK` for `config/test_023_ellipse_glauber.cfg --nevents 1000`, writing `/private/tmp/blastwave_test_023_ellipse_glauber.root`
  - O2Physics ROOT executor QA passed for `/private/tmp/blastwave_test_023_ellipse_glauber.root`, reporting `validation_passed events=1000`
- Locked conclusions:
  - the three Glauber mirror configs are executable comparison inputs for the corresponding response-test settings
  - low-stat 1-event and 20-event attempts are not useful validators for these configs because differential cumulant analysis can fail before enough events accumulate
  - no public config key, ROOT schema, QA schema, or generator code changed

## T-030 Retired Native PyROOT TH2 Response-Fit Notebook

- Status: retired on 2026-05-13
- Evidence:
  - `notebooks/vn_epsn_pyroot_th2_fit.ipynb` was removed when the maintained notebook environment was narrowed to an `uproot`-only stack
  - the prior 2026-05-11 PyROOT smoke is historical evidence only and is no longer a maintained validation target
- Locked conclusions:
  - maintained notebook analysis should use `notebooks/vn_epsn_regression.ipynb` and `uproot`
  - PyROOT-dependent notebook work must be reintroduced as an explicit separate path if needed later
  - no generator code, ROOT output schema, config contract, or QA schema changed by the retirement

## T-029 Event-Level `v_n`-`epsilon_n` Regression Notebook

- Status: passed for multi-file notebook smoke on 2026-05-11; maintained as `uproot`-only on 2026-05-13; source-checked after cross-harmonic fit-scope maintenance on 2026-05-20
- Evidence:
  - 2026-05-20 source-level check: `jq empty notebooks/vn_epsn_regression.ipynb` passed, and all 11 code cells parsed with Python `ast`
  - `notebooks/vn_epsn_regression.ipynb` JSON parsed successfully and all code cells parsed with Python `ast`
  - executing all code cells from the Blast_wave repo root with the `root_notebook` environment completed labelled multi-file comparison over `qa/test_b8_response_023.root`, `qa/test_b8_response_023_mix.root`, and `qa/test_b8_023_newrap.root`
  - each default input contributed 5000 selected events and produced same-harmonic `v_n/epsilon_n` plus cross-harmonic `v_n/epsilon_m` regression rows
  - the optional PyROOT inspection cells and PyROOT dependency were removed on 2026-05-13
  - O2Physics ROOT executor returned `STATUS: PRIMARY_OK` for an earlier input-contract check of `qa/test_b8_response_023.root`
- Locked conclusions:
  - the notebook targets current event-level ROOT schema and uses the participant-plane projected response branches for regression
  - regression is performed on raw `events` tree rows for each labelled input file; binned means in the plots are visual guides only
  - `uproot` is the notebook reader; PyROOT is no longer part of the maintained environment or notebook flow
  - no generator code, ROOT output schema, config contract, or QA schema changed

## T-028 Progress Heartbeat And ETA

- Status: passed on 2026-05-07
- Evidence:
  - local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed after the progress reporter, CMake, test, and docs updates
  - local `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 10/10 tests
  - focused `test_progress_reporter` passed with deterministic coverage for unknown initial ETA, finite average-event ETA, same-percent activity-frame changes, and final zero ETA
  - manual `generate_blastwave_events config/test_b8.cfg --progress --nevents 5 --output /private/tmp/blastwave_progress_smoke.root` smoke exited successfully and printed a progress line containing an activity frame plus ETA
- Locked conclusions:
  - enabled generator progress now has a one-second heartbeat independent of integer-percent changes
  - ETA is intentionally approximate and based only on average completed-event duration
  - no public config key, ROOT schema, or QA schema change was introduced

## T-027 Density-Defined Flow-Trans Sigma-Equivalent Radius

- Status: passed on 2026-05-07
- Evidence:
  - local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed after the flow-field, parser, test, docs, config-comment, and project-state updates
  - focused `/Users/allenzhou/Research_software/Blast_wave/bin/test_flow_field_model` passed with hard-coded q oracle coverage, percentile and level formula coverage, beta cap coverage, and shell-gradient `xi_shell` / `xi_flow` split coverage for both density selectors
  - focused `/Users/allenzhou/Research_software/Blast_wave/bin/test_run_options` passed with `density-level:1.0` / `1.2` rejection text coverage
  - local `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 9/9 tests
  - O2Physics ROOT executor generated and QA-validated `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-percentile:0.95 --nevents 20`, writing `/private/tmp/blastwave_radius_sigma_percentile.root`
  - O2Physics ROOT executor generated and QA-validated `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-level:1.0e-3 --nevents 20`, writing `/private/tmp/blastwave_radius_sigma_level.root`
  - O2Physics ROOT executor generated and QA-validated `config/test_b8_response_023_dense_mix.cfg --nevents 1000`, writing `/private/tmp/blastwave_response_023_dense_mix_sigma.root`; current tracked equivalent path is `config/test_023_dense_mix.cfg`
  - O2Physics ROOT executor generated and QA-validated `config/test_b8_023_dense_newrap.cfg --nevents 1000`, writing `/private/tmp/blastwave_response_023_dense_newrap_sigma.root`; current tracked equivalent path is `config/test_023_dense_newrap.cfg`
  - ROOT metric extraction reported `dense_mix meanPt=1.041669379 psi2_proj=0.022560737` and `newrap meanPt=1.067941944 psi2_proj=0.022885878`
- Locked conclusions:
  - density-defined `R_density(phi)` now defines angular shell geometry only
  - main flow strength uses `xi_flow = q * xi_shell`, with `xi_shell = r / R_density(phi)`
  - `density-percentile:p` uses `q = sqrt(-2 * log1p(-p))`
  - `density-level:fraction` uses `q = sqrt(-2 * log(fraction))` and rejects `fraction >= 1`
  - `flowTransRho0` is the covariance-equivalent `xi_flow = 1` rapidity scale for density-defined radii
  - `shell-gradient-corrected` correction lookup uses clamped `xi_shell`; the main radial profile uses `xi_flow`
  - the `newrap` 1000-event response acceptance is back at O(1.0) mean pT and O(0.02) `psi2_proj`, not the old suppressed `meanPt≈0.58` / `psi2_proj≈0.0057` regime
  - no public config key, ROOT schema, or QA schema change was introduced; existing example configs only had explanatory comments refreshed

## T-026 Density-Defined Flow-Trans Radius Unclamped Xi

- Status: superseded by T-027 on 2026-05-07
- Historical note:
  - this was the short-lived no-upper-clamp contract from `docs/半径算法修复.md`
  - current density-defined radius semantics and validation evidence live in T-027

## T-025 Shell-Gradient-Corrected Density-Normal Flow Magnitude

- Status: passed on 2026-05-07
- Evidence:
  - local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed after the parser, validation, flow-field cache, tests, config, docs, and project-state updates
  - local `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 9/9 tests
  - focused `./bin/test_run_options` passed with new `flow-trans-magnitude-mode` parse/default/override/invalid coverage, supported radius validation, covariance/non-density-normal rejection, explicit-gradient-with-radius-profile rejection, and floor/cap bound rejection
  - focused `./bin/test_flow_field_model` passed with `shell-gradient-corrected` zero-strength equality, circular symmetry, asymmetric same-shell difference, max-factor cap, and degenerate-profile finite fallback coverage
  - O2Physics ROOT executor returned `STATUS: PRIMARY_OK` for `config/test_b8_density_normal_flow_trans_gradient.cfg --nevents 20`, writing `qa/test_flow_trans_shell_gradient.root`, followed by `qa_blastwave_output --expect-nevents 20`
  - the same O2Physics command also generated and QA-validated a radius-profile control from `config/test_b8_density_normal_flow_trans.cfg --nevents 20`, writing `qa/test_flow_trans_radius_profile_control.root`
- Locked conclusions:
  - `flow-trans-magnitude-mode = radius-profile` remains the default behavior
  - `shell-gradient-corrected` is executable through the real generator and independent QA path
  - the correction is density-normal-only and requires `density-percentile` or `density-level` radius
  - explicit `flow-trans-gradient-*` knobs are rejected unless `shell-gradient-corrected` is selected
  - `flow-trans-gradient-max-factor-delta` bounds the multiplicative correction factor, not an additive rapidity term
  - no ROOT schema or QA schema change was introduced by this packet

## T-024 Flow-Trans Radius Resolution Presets

- Status: passed on 2026-05-07
- Evidence:
  - local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed after the resolution parser, cache, density-only query, docs, and config updates
  - local `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 9/9 tests
  - `test_run_options` covers default implicit `balanced`, config/CLI parsing for `balanced|precise|fast`, CLI-over-config precedence, invalid labels, empty value rejection, and explicit non-density-normal validation rejection
  - `test_flow_field_model` covers resolution-aware profile cache rebuilding and balanced-vs-precise stability for `density-percentile:0.95` and `density-level:1.0e-3`
  - O2Physics ROOT executor `PRIMARY_OK` generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius covariance --nevents 20`, written to `qa/test_flow_trans_covariance.root`
  - O2Physics ROOT executor `PRIMARY_OK` generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-percentile:0.95 --flow-trans-radius-resolution balanced --nevents 20`, written to `qa/test_flow_trans_density_percentile_balanced.root`
  - O2Physics ROOT executor `PRIMARY_OK` generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-percentile:0.95 --flow-trans-radius-resolution precise --nevents 20`, written to `qa/test_flow_trans_density_percentile_precise.root`
  - O2Physics ROOT executor `PRIMARY_OK` generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-level:1.0e-3 --flow-trans-radius-resolution balanced --nevents 20`, written to `qa/test_flow_trans_density_level_balanced.root`
- Locked conclusions:
  - `balanced` is the default density-defined flow-trans boundary profile resolution
  - `precise` preserves the old `360 x 512` grid for precision baselines and rollback comparisons
  - `fast` is available as a low-cost pre-scan preset
  - resolution changes do not alter `R_density(phi)`, `xi_shell`, `q`, or `xi_flow`, and do not affect `flow-trans-radius = covariance`
  - boundary profile construction now uses scalar density queries instead of full density-gradient samples

## T-023 Flow-Trans Naming And Density-Normal Radius Modes

- Status: passed on 2026-05-06
- Evidence:
  - local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed after the parser/config/flow-field/cache changes
  - local `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 9/9 tests
  - `test_run_options` covers new `flow-trans-*` parsing, CLI-over-config precedence, old `rho0` / `flow-power` rejection, radius value validation, and invalid non-density-normal direction/radius combinations
  - `test_flow_field_model` covered density-normal direction interpolation, covariance radius baseline, circular density-defined radii, triangular/hotspot radius variation, and the then-current outside-boundary behavior; T-026 supersedes the old clipped-radius expectation with unclamped `xi`
  - authoritative outside-sandbox O2Physics generate + QA passed for default `config/test_b8.cfg --nevents 20`, written to `qa/test_flow_trans_default.root`
  - authoritative outside-sandbox O2Physics generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius covariance --nevents 20`, written to `qa/test_flow_trans_covariance.root`
  - authoritative outside-sandbox O2Physics generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-percentile:0.95 --nevents 20`, written to `qa/test_flow_trans_density_percentile.root`
  - authoritative outside-sandbox O2Physics generate + QA passed for `config/test_b8_density_normal_flow_trans.cfg --flow-trans-radius density-level:1.0e-3 --nevents 20`, written to `qa/test_flow_trans_density_level.root`
- Locked conclusions:
  - `rho0` and `flow-power` are no longer compatible public names
  - `flow-trans-rho0` and `flow-trans-profile-power` are the public transverse rapidity baseline/profile controls
  - `kappa2` remains the second-order response coefficient
  - density-normal direction/radius high-order controls are executable and QA-valid for covariance, percentile, and level radius definitions
  - the sandboxed ROOT run hit known PCM/module noise and is not the authoritative result; the outside-sandbox run is the recorded evidence

## T-021 Response/Cross-Talk TH2 Display Window

- Status: passed on 2026-05-06
- Evidence:
  - local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed after the ROOT writer display-window change
  - local `ctest --output-on-failure` passed with 9/9 tests
  - O2Physics ROOT executor generated `/tmp/response_axis_range_smoke.root` from `config/test_b8_response_023.cfg --nevents 20`; current tracked equivalent path is `config/test_023_dense.cfg`
  - O2Physics ROOT executor QA passed for the smoke file with `--expect-nevents 20`
  - direct ROOT read of all four response/cross-talk TH2s reported `xstorage=0..1`, `xview=0..0.35`, `ystorage=-1..1`, `yview=-0.15..0.15`
- Locked conclusions:
  - the four response/cross-talk TH2s keep full storage ranges and only change their persisted default display window
  - current QA remains compatible with the display-window change

## T-022 Differential `v2/v3{2}(pT)` Flowpt Contract

- Status: passed on 2026-05-06
- Evidence:
  - local touched-target build passed for `generate_blastwave_events`, `analyze_blastwave_vnpt`, `qa_blastwave_output`, `test_run_options`, and `test_differential_flow_cumulant`
  - focused ROOT-free `test_differential_flow_cumulant` passed for harmonic `n=2` and `n=3`
  - focused `test_run_options` passed with `v3pt-bins`, `flowpt-output-mode`, `flowpt-output`, default `_flowpt.root`, and old-name rejection coverage
  - local full `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed
  - local full `ctest --test-dir /Users/allenzhou/Research_software/Blast_wave/build --output-on-failure` passed with 9/9 tests
  - O2Physics ROOT executor generated and QA-validated a same-file smoke with both `v2` and `v3` differential payloads
  - O2Physics ROOT executor generated and QA-validated a separate-file smoke where the main result kept only `v2_2_pt_edges` and `v3_2_pt_edges`
  - ROOT key inspection confirmed the shared separate flowpt file contains `v2_2_pt_edges`, `v2_2_pt`, `v2_2_pt_canvas`, `v3_2_pt_edges`, `v3_2_pt`, and `v3_2_pt_canvas`
  - standalone `analyze_blastwave_vnpt --output` wrote both enabled harmonics to a shared flowpt ROOT file
  - standalone `analyze_blastwave_vnpt --inplace` added both full payloads back to the main ROOT file, and QA passed afterward
- Locked conclusions:
  - public output controls are now shared `flowpt-output-mode` / `flowpt-output`
  - `v2pt-output-mode` and `v2pt-output` are rejected with migration guidance
  - `analyze_blastwave_vnpt` is the canonical standalone post-processing entrypoint
  - `flowpt-output-mode = separate-file` intentionally leaves full differential payloads in one shared flowpt file while keeping enabled edge metadata in the main result

## T-020 Response-Test 0+2+3 Initial Geometry And Third-Harmonic Contract

- Status: passed on 2026-05-06
- Evidence:
  - local `cmake --build /Users/allenzhou/Research_software/Blast_wave/build -j4` passed after adding the response-test generator, schema, QA, and tests
  - local `ctest --output-on-failure` passed with 9/9 tests, including `test_harmonic_geometry` and `test_blast_wave_generator_response`
  - authoritative outside-sandbox O2Physics generate + QA passed for default `config/test_b8.cfg --nevents 20`, written to `qa/response_test_glauber_smoke.root`
  - authoritative outside-sandbox O2Physics generate + QA passed for `config/test_b8_response_023.cfg --nevents 100`, written to `qa/response_test_023_smoke.root`; current tracked equivalent path is `config/test_023_dense.cfg`
  - response-test QA validated 100 events with `mean_Npart=600`, `mean_eps3=0.0732512`, and `mean_v3=0.0461744`
  - four-point outside-sandbox `A3` scan passed generate + QA with 50 events per point, then ROOT extraction from `events` reported:
    - `A3=0`: `mean_eps3=0.065015732`, `mean_v3_wrt_psi3=0.010711697`
    - `A3=0.05`: `mean_eps3=0.065350879`, `mean_v3_wrt_psi3=0.015722963`
    - `A3=0.10`: `mean_eps3=0.071685623`, `mean_v3_wrt_psi3=0.024233045`
    - `A3=0.15`: `mean_eps3=0.077883277`, `mean_v3_wrt_psi3=0.030142274`
- Locked conclusions:
  - default `initial-geometry = glauber` remains compatible with the expanded mandatory third-harmonic ROOT schema
  - `response-test-023` writes synthetic participants with `nucleus_id = -1`, and QA accepts that sentinel only for response-test events
  - `eps3/psi3`, `v3`, `v2_wrt_psi2`, and `v3_wrt_psi3` are now part of the current ROOT/QA contract
  - the small `A3` scan shows the expected monotonic rise in projected `v3_wrt_psi3`; physics conclusions still require larger statistics

## T-019 Affine-Effective Before/After Density Maps

- Status: passed on 2026-04-29
- Evidence:
  - O2Physics build passed after adding the two affine-effective density-map TH2 objects and QA checks
  - local `ctest --output-on-failure` passed
  - authoritative outside-sandbox O2Physics generate passed for `config/test_b8_affine_effective.cfg --nevents 100`, written to `qa/affine_effective_density_maps.root`
  - authoritative outside-sandbox O2Physics QA passed for `qa/affine_effective_density_maps.root`, validating 100 events
  - shared ROOT file inspector returned `RUNTIME_STATUS: PRIMARY_OK`, `STATUS: OK`, and listed both `affine_effective_density_initial_x-y` and `affine_effective_density_final_x-y`
- Locked conclusions:
  - affine-effective output now includes a paired first-valid-event density snapshot before and after `affine-gaussian` evolution
  - both maps are TH2 objects with ROOT 3D draw-style validation through QA
  - the maps are diagnostic snapshots, not event-averaged density observables

## T-018 Affine-Effective Additive-Rho And Full-Tensor Correction

- Status: passed on 2026-04-29
- Evidence:
  - local build passed after adding `affine-effective-mode`, updating affine-effective formulas, and extending ROOT debug schema/QA
  - local `ctest --output-on-failure` passed with updated `test_flow_field_model` and `test_run_options`
  - authoritative outside-sandbox O2Physics generate + QA passed for `config/test_b8.cfg` written to `qa/affine_effective_fix_default.root`
  - authoritative outside-sandbox O2Physics generate + QA passed for `config/test_b8_affine_effective.cfg --debug-flow-ellipse` written to `qa/affine_effective_fix_additive.root`
  - authoritative outside-sandbox O2Physics generate + QA passed for `config/test_b8_affine_effective.cfg --affine-effective-mode full-tensor --debug-flow-ellipse` written to `qa/affine_effective_fix_full_tensor.root`
- Locked conclusions:
  - `affine-effective-mode` is public and defaults to `additive-rho`
  - `additive-rho` preserves `rho0` as the baseline average-flow rapidity and uses density-normal direction with covariance-normal fallback
  - `full-tensor` is an opt-in principal-axis tensor velocity closure under the same `affine-effective` sampler
  - `affine-kappa-aniso` remains parse/finite-compatible but is legacy/no-op in both current affine-effective modes
  - `flow_ellipse_debug` now carries `affine_effective_mode` and additive-rho surface rapidity decomposition diagnostics, and QA validates the mode-specific formulas
- Note:
  - the tracked `config/test_b8.cfg` used in the baseline smoke currently selects `density-evolution = none`; the current built-in default remains `affine-gaussian`

## T-017 Opt-In Affine-Effective Closure Flow Sampler

- Status: passed on 2026-04-29; internal velocity formula superseded by T-018 / DEC-013
- Evidence:
  - local build passed after extending `EventMedium`, `FlowFieldModel`, generator validation, ROOT debug writing, and QA
  - local `ctest` passed with new affine-effective coverage in `test_flow_field_model` and `test_run_options`
  - authoritative default V1a generate + QA smoke passed
  - authoritative `config/test_b8_affine_effective.cfg` generate + QA smoke passed
- Locked conclusions:
  - `flow-velocity-sampler = affine-effective` is opt-in and only valid with `density-evolution = affine-gaussian`
  - `affine-delta-tau-ref`, `affine-kappa-flow`, `affine-kappa-aniso`, and `affine-u-max` were added as public validated knobs
  - `debug-flow-ellipse` may now carry affine closure diagnostics, and QA validates them when present
  - the first rollout's `rho0` and `affine-kappa-aniso` formula semantics are superseded by T-018; use DEC-013 for current behavior

## T-016 Differential `v2{2}(pT)` Joint And Standalone Contract

- Status: passed on 2026-04-29
- Evidence:
  - local `ctest` passed with the differential cumulant coverage in tree
  - authoritative same-file generate + QA passed
  - authoritative separate-file generate passed and QA accepted the metadata-only main file
  - standalone analysis passed in both separate-file and `--inplace` modes
  - ROOT key inspection confirmed the dedicated analysis file contains only `v2_2_pt_edges`, `v2_2_pt`, and `v2_2_pt_canvas`
- Locked conclusions:
  - configuring `v2pt-bins` always writes `v2_2_pt_edges`
  - `flowpt-output-mode = separate-file` may leave the main result file metadata-only
  - differential `v2{2}(pT)` is separate from `events.v2` and currently uses unit track weights only

## T-015 Freeze-Out Eccentricity Response TH1 Contract

- Status: passed on 2026-04-29
- Evidence:
  - authoritative generate + QA smoke passed
  - ROOT key inspection confirmed `chi2` is present and `chi2_canvas` is not required
- Locked conclusion:
  - the default contract keeps the `chi2` histogram only; there is no second mandatory canvas object for this diagnostic

## T-014 Public Affine V1a Evolution Knob Surface

- Status: passed on 2026-04-28
- Evidence:
  - `test_run_options` coverage passed
  - full local `ctest` passed after exposing the knobs publicly
- Locked conclusion:
  - `affine-lambda-in`, `affine-lambda-out`, and `affine-sigma-evo` are public validated knobs with unchanged defaults

## T-013 Affine Density-Normal Compensation Contract

- Status: passed on 2026-04-28
- Evidence:
  - parser and regression coverage passed
  - authoritative default-off, opt-in-on, and invalid-combination smokes passed
- Locked conclusion:
  - `density-normal-kappa-compensation` is opt-in and only valid for `affine-gaussian + density-normal`

## T-012 V2 Gradient-Response Medium And Flow

- Status: passed on 2026-04-28
- Evidence:
  - local build and `ctest` passed
  - authoritative default V1a control smoke passed
  - authoritative V2 gradient, V2 debug, and zero-event debug checks passed
- Locked conclusions:
  - `density-evolution = gradient-response` and `flow-velocity-sampler = gradient-response` must be selected together
  - `r2_0/r2_f/r2_ratio`, `x0/y0`, and `emission_weight` are part of the current contract

## T-011 `kappa2` Public Flow Response Contract

- Status: passed on 2026-04-28
- Evidence:
  - parser/test coverage passed
  - authoritative smoke validation passed after the public naming switch
- Locked conclusions:
  - `kappa2` replaced `rho2`
  - the V1a second-order response uses initial `eps2/psi2`, not freeze-out geometry diagnostics

## T-010 V1a Affine Gaussian Density Response

- Status: passed on 2026-04-28
- Evidence:
  - local build and `ctest` passed
  - authoritative default V1a smoke passed
  - authoritative `density-evolution = none` comparison smoke passed
  - authoritative V1a `density-normal` smoke passed
- Locked conclusions:
  - `affine-gaussian` is the default medium mode
  - `none` remains the legacy comparison path
  - freeze-out geometry diagnostics are mandatory in the current schema

## Earlier Checks Still Relied On

- T-001 Baseline build and smoke
  - configure, build, generate, and QA path established
- T-002 Participant output contract
  - `participants` tree plus participant visualization payload remain part of the current schema
- T-003 Config-file CLI contract
  - `--config` and positional config entry both remain public; relative outputs resolve against the config directory
- T-004 Thermal sampler switch
  - `maxwell-juttner` remains the default and `gamma` remains the explicit compatibility mode
- T-005 Covariance-ellipse default flow replacement
  - covariance-ellipse remains the default transverse-flow source
- T-006 Event-level `v2` output contract
  - `events.v2` and the `v2` summary histogram remain validated through shared observable helpers
- T-007 Example launcher ROOT alignment
  - launcher ROOT preflight and explicit `ROOT::HistPainter` link remain intentional
- T-008 Fluid-element velocity sampler surface
  - `flow-velocity-sampler` and `flow-density-sigma` remain the public abstraction
- T-009 `EventMedium` and `EmissionSite` interface refactor
  - the internal medium/emission split remains the intended extension boundary
- T-011 `kappa2` public flow response contract
  - covariance-style V1a flow still uses initial `eps2/psi2`

## Environment Note

- On this machine, sandboxed `alienv` ROOT runs are not authoritative when PCM / module noise appears.
- Durable ROOT evidence should continue to come from the outside-sandbox O2Physics path.
