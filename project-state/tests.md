# Tests

## T-011 Kappa2 Public Flow Response Contract

- Status: passed
- Purpose: verify that the public second-order response input is now `kappa2`, that `rho2` fails fast with migration guidance, and that V1a uses the initial participant eccentricity vector for the second-order flow response.
- Execution shape:
  - rebuild the touched generator, QA, and ROOT-free regression targets
  - run all registered CTest tests
  - confirm `--rho2` is rejected with `rho2 -> kappa2`
  - run authoritative default V1a generate+QA smoke
  - run authoritative `density-evolution none` generate+QA smoke
  - run authoritative V1a + `density-normal` generate+QA smoke
- Fresh commands executed on 2026-04-28:
  - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build --target generate_blastwave_events qa_blastwave_output test_flow_field_model test_emission_sampler test_run_options test_maxwell_juttner_sampler -j4`
  - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build --target test_flow_field_model test_run_options generate_blastwave_events qa_blastwave_output -j4`
  - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
  - `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --rho2 0.06`
  - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --output /tmp/blastwave_kappa2_v1a_smoke.root && /Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_kappa2_v1a_smoke.root --output /tmp/blastwave_kappa2_v1a_smoke_validation.root --expect-nevents 20 && /Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --density-evolution none --output /tmp/blastwave_kappa2_none_smoke.root && /Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_kappa2_none_smoke.root --output /tmp/blastwave_kappa2_none_smoke_validation.root --expect-nevents 20 && /Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --output /tmp/blastwave_kappa2_density_normal_smoke.root && /Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_kappa2_density_normal_smoke.root --output /tmp/blastwave_kappa2_density_normal_smoke_validation.root --expect-nevents 20'"`
- Current result:
  - build: passed
  - `ctest`: passed, 6/6
  - deprecated `rho2`: `generate_blastwave_events failed: Invalid option/key 'rho2' from command line option '--rho2'. Migration: rho2 -> kappa2.`
  - default V1a covariance QA: `validation_passed events=20 particles=7077 mean_Npart=175.75 mean_eps2=0.276602 mean_v2=0.0714324 max_abs_eta_s=6.67501 max_E=958.06 max_mass_shell_deviation=3.02439e-11`
  - `density-evolution none` QA: `validation_passed events=20 particles=7077 mean_Npart=175.75 mean_eps2=0.276602 mean_v2=0.0924968 max_abs_eta_s=6.67501 max_E=984.787 max_mass_shell_deviation=3.58382e-11`
  - V1a `density-normal` QA: `validation_passed events=20 particles=7077 mean_Npart=175.75 mean_eps2=0.276602 mean_v2=0.0463026 max_abs_eta_s=6.67501 max_E=960.252 max_mass_shell_deviation=7.70089e-11`
- verification_status: `verified`

## T-010 V1a Affine Gaussian Density Response

- Status: passed
- Purpose: verify that the default medium mode now runs the V1a `s0 -> sf` affine Gaussian response, that the legacy `density-evolution none` path remains available, and that the new freeze-out geometry ROOT contract is validated by QA.
- Execution shape:
  - rebuild the checkout
  - run ROOT-free regression tests
  - run authoritative default V1a generate+QA smoke
  - run authoritative `density-evolution none` generate+QA smoke
  - run authoritative V1a + `density-normal` generate+QA smoke
- Fresh commands executed on 2026-04-28:
  - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build --target generate_blastwave_events qa_blastwave_output test_flow_field_model test_emission_sampler test_run_options test_physics_utils test_maxwell_juttner_sampler test_output_path_utils -j4`
  - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
  - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --output /tmp/blastwave_v1a_affine_smoke.root'"`
  - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_v1a_affine_smoke.root --output /tmp/blastwave_v1a_affine_smoke_validation.root --expect-nevents 20'"`
  - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --density-evolution none --output /tmp/blastwave_v1a_none_smoke.root'"`
  - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_v1a_none_smoke.root --output /tmp/blastwave_v1a_none_smoke_validation.root --expect-nevents 20'"`
  - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --output /tmp/blastwave_v1a_density_normal_smoke.root'"`
  - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_v1a_density_normal_smoke.root --output /tmp/blastwave_v1a_density_normal_smoke_validation.root --expect-nevents 20'"`
- Current result:
  - build: passed
  - `ctest`: passed, 6/6
  - default V1a covariance QA: `validation_passed events=20 particles=7077 mean_Npart=175.75 mean_eps2=0.276602 mean_v2=0.0694394 max_abs_eta_s=6.67501 max_E=951.03 max_mass_shell_deviation=4.83993e-11`
  - `density-evolution none` QA: `validation_passed events=20 particles=7077 mean_Npart=175.75 mean_eps2=0.276602 mean_v2=0.0924968 max_abs_eta_s=6.67501 max_E=984.787 max_mass_shell_deviation=3.58382e-11`
  - V1a `density-normal` QA: `validation_passed events=20 particles=7077 mean_Npart=175.75 mean_eps2=0.276602 mean_v2=0.0468845 max_abs_eta_s=6.67501 max_E=951.539 max_mass_shell_deviation=6.03567e-11`
  - implementation note: the first sandboxed ROOT attempt emitted the known PCM/module noise, so only the outside-sandbox reruns above are counted as authoritative
- verification_status: `verified`

## T-001 Baseline Build And Smoke Validation

- Status: passed
- Purpose: verify that the current checkout still configures, builds, generates events, and passes the independent QA reader.
- Execution shape:
  - configure with `cmake --preset default`
  - build with `cmake --build .../build`
  - generate a small ROOT sample with `generate_blastwave_events`
  - validate it with `qa_blastwave_output`
- Existing evidence:
  - human-written docs in `docs/agent_guide.md` and `docs/项目说明.md` describe the expected build and smoke commands
  - fresh commands executed on 2026-04-10:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 10 --output /Users/allenzhou/Research_software/Blast_wave/qa/test_psi2.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/test_psi2.root --output /Users/allenzhou/Research_software/Blast_wave/qa/test_psi2_validation.root --expect-nevents 10'"`
  - fresh commands executed on 2026-04-13 for the centrality-output extension:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 10 --output /tmp/blastwave_centrality_smoke.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_centrality_smoke.root --output /tmp/blastwave_centrality_smoke_validation.root --expect-nevents 10'"`
  - fresh commands executed on 2026-04-14 for the Maxwell-Juttner switch:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
  - fresh commands executed on 2026-04-14 after the source-structure cleanup:
    - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build`
    - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
- Current result:
  - configure: passed
  - build: passed
  - generator run: passed
  - QA run: passed
  - QA summary: `validation_passed events=10 particles=3668 mean_Npart=185.5 mean_eps2=0.237373 max_abs_eta_s=6.41806 max_E=222.582 max_mass_shell_deviation=9.16636e-12`
  - output contract includes `events.centrality` and `cent`
- verification_status: `verified`

## T-002 Participant Output Contract

- Status: passed
- Purpose: verify that the output file contains explicit participant geometry objects rather than only particle-level source metadata.
- Checks performed:
  - `participants` tree exists
  - `participant_x-y` histogram exists
  - `participant_x-y_canvas` canvas exists
  - `participant_x-y_canvas` contains a `TPaveStats` object so the saved participant visual opens with a visible stats box
  - `events.Npart` matches the number of rows in `participants` for each event
  - `participants.nucleus_id` stays within `{0, 1}`
- Current result:
  - passed via the updated independent QA reader on `qa/test_psi2.root`
  - direct ROOT inspection of `participant_x-y_canvas` reported:
    - `stats_object=TPaveStats`
    - `hist_stats_object=TPaveStats`

## T-003 Config-File CLI Contract

- Status: passed
- Purpose: verify that `generate_blastwave_events` accepts repository-shipped config files, preserves explicit CLI compatibility, and enforces the new parser contract with actionable errors.
- Execution shape:
  - rebuild the current checkout
  - inspect `--help`
  - generate from the tracked sample config path
  - exercise parser failure paths
  - validate the generated ROOT output with the independent QA reader
- Existing evidence:
  - build completed after the Maxwell-Juttner CLI/config change
  - `--help` reports:
    - `generate_blastwave_events --config <path> [options]`
    - `generate_blastwave_events <config-path> [options]`
    - `explicit CLI options > configuration file values > built-in defaults`
    - `--thermal-sampler <maxwell-juttner|gamma>`
    - `--mj-pmax <GeV>`
    - `--mj-grid-points <int>`
  - the current checkout tracks:
    - `config/test_b8.cfg`
    - `config/b8.cfg`
    - `scripts/run_example_config.sh`
  - the higher-authority docs now use the canonical tracked path `config/test_b8.cfg`
  - parser failure-path checks returned the expected errors for:
    - invalid thermal sampler mode
    - `mj-pmax <= 0`
    - `mj-grid-points < 2`
  - fresh authoritative generate+QA commands executed on 2026-04-14:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /tmp/blastwave_mj_default_smoke.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_mj_default_smoke.root --output /tmp/blastwave_mj_default_smoke_validation.root --expect-nevents 5000'"`
- Current result:
  - parser and entrypoint behavior: passed
  - tracked sample config presence: passed at `config/test_b8.cfg`
  - canonical documentation of the tracked path: passed
  - durable generate+QA evidence for the current tracked config path: passed
  - QA summary: `validation_passed events=5000 particles=1802202 mean_Npart=180.622 mean_eps2=0.263087 max_abs_eta_s=8.39197 max_E=3259.31 max_mass_shell_deviation=1.27587e-09`
  - note: the sandboxed `alienv` ROOT attempt on 2026-04-14 was not authoritative because it emitted PCM/module errors; the recorded commands above are the outside-sandbox reruns that passed
- verification_status: `verified`

## T-004 Thermal Sampler Switch Validation

- Status: passed
- Purpose: verify that the new default Maxwell-Juttner momentum sampler is numerically stable, that the new runtime knobs are wired through the build, and that legacy `gamma` mode remains compatible with the existing ROOT output contract.
- Execution shape:
  - rebuild the current checkout
  - run the ROOT-free core sampler regression test through `ctest`
  - exercise the new `--help` surface
  - run one explicit `gamma` generate+QA compatibility smoke
- Existing evidence:
  - fresh `ctest --output-on-failure` executed on 2026-04-14 reported:
    - `1/1 Test #1: test_maxwell_juttner_sampler ... Passed`
  - the core test covers:
    - two `(m, T)` points, including `(0.13957, 0.2)`
    - CDF monotonicity and finite-table checks
    - histogram-vs-theory shape checks for `p^2 exp(-sqrt(p^2 + m^2) / T)`
    - `4096/8 -> 8192/10` stability comparisons in `⟨p⟩` and `⟨E⟩`
    - `temperature <= 0` behavior for both `MaxwellJuttner` and `Gamma`
  - fresh authoritative gamma compatibility commands executed on 2026-04-14:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 100 --thermal-sampler gamma --output /tmp/blastwave_gamma_smoke.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_gamma_smoke.root --output /tmp/blastwave_gamma_smoke_validation.root --expect-nevents 100'"`
- Current result:
  - core regression test: passed
  - `--help` exposure of thermal-sampler knobs: passed
  - gamma compatibility smoke: passed
  - gamma QA summary: `validation_passed events=100 particles=36509 mean_Npart=181.99 mean_eps2=0.265514 max_abs_eta_s=7.21087 max_E=755.096 max_mass_shell_deviation=9.99424e-11`
- verification_status: `verified`

## T-005 Covariance-Ellipse Default Flow Replacement

- Status: passed
- Purpose: verify that the default flow field now uses the participant covariance ellipse, that the public flow knobs have migrated to `rho0/rho2/flow-power`, and that the optional debug payload writes and validates correctly.
- Historical note: this 2026-04-22 record predates DEC-008. In the current contract, the second-order public knob is `kappa2` and legacy `rho2` fails fast.
- Execution shape:
  - rebuild the checkout
  - run `ctest --output-on-failure`
  - inspect `generate_blastwave_events --help`
  - exercise deprecated flow-parameter failure paths
  - run one authoritative default generate+QA smoke
  - run one authoritative debug generate+QA smoke
- Existing evidence:
  - local build/test commands executed on 2026-04-22:
    - `cmake -S /Users/allenzhou/Research_software/Blast_wave -B /Users/allenzhou/Research_software/Blast_wave/build`
    - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build --target generate_blastwave_events qa_blastwave_output test_flow_field_model test_maxwell_juttner_sampler test_output_path_utils -j4`
    - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
    - `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --help`
    - `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --vmax 0.8`
    - `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --kappa2 0.5`
    - `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --r-ref 6.0`
  - authoritative outside-sandbox O2Physics commands executed on 2026-04-22:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /tmp/blastwave_covariance_default.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_covariance_default.root --output /tmp/blastwave_covariance_default_validation.root --expect-nevents 5000'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 100 --debug-flow-ellipse --output /tmp/blastwave_covariance_debug.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_covariance_debug.root --output /tmp/blastwave_covariance_debug_validation.root --expect-nevents 100'"`
- Current result:
  - ROOT-free tests: passed
  - `--help` flow surface: passed
  - deprecated flow-parameter failures: passed
  - default `config/test_b8.cfg` authoritative smoke: passed
  - default QA summary: `validation_passed events=5000 particles=1802202 mean_Npart=180.622 mean_eps2=0.263087 max_abs_eta_s=8.39197 max_E=7396.23 max_mass_shell_deviation=6.37788e-09`
  - debug authoritative smoke: passed
  - debug QA summary: `validation_passed events=100 particles=35786 mean_Npart=179.54 mean_eps2=0.258217 max_abs_eta_s=7.34076 max_E=1141.24 max_mass_shell_deviation=1.87173e-10`
  - implementation note: the first debug authoritative run exposed a `RootEventFileWriter` lifetime bug in the optional debug `TTree/TH2`; detaching and resetting those objects before writer teardown fixed the crash and the rerun passed
- verification_status: `verified`

## T-006 Event-Level `v2` Output Contract

- Status: passed
- Purpose: verify that the mandatory output contract now includes event-level final-state `v2` in the `events` tree plus a matching summary `v2` histogram, and that QA can recompute the observable from the particle tree.
- Execution shape:
  - rebuild the checkout after the schema change
  - run `ctest --output-on-failure`
  - run one authoritative generate+QA smoke in the O2Physics ROOT environment
- Existing evidence:
  - local build/test commands executed on 2026-04-22:
    - `cmake -S /Users/allenzhou/Research_software/Blast_wave -B /Users/allenzhou/Research_software/Blast_wave/build`
    - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build --target test_physics_utils test_flow_field_model test_maxwell_juttner_sampler test_output_path_utils generate_blastwave_events qa_blastwave_output -j4`
    - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
  - authoritative outside-sandbox O2Physics commands executed on 2026-04-22:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 20 --output /tmp/blastwave_event_v2_smoke.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_event_v2_smoke.root --output /tmp/blastwave_event_v2_smoke_validation.root --expect-nevents 20'"`
- Current result:
  - ROOT-free tests: passed
  - new `test_physics_utils` regression target: passed
  - event-level `v2` generate+QA smoke: passed
  - QA summary: `validation_passed events=20 particles=7051 mean_Npart=181.6 mean_eps2=0.228879 mean_v2=0.0645756 max_abs_eta_s=7.2615 max_E=568.599 max_mass_shell_deviation=6.21324e-11`
  - implementation note: the validation loop also exposed a pre-existing `computePseudorapidity` edge-case bug for exactly beam-aligned negative-z momenta; the helper now returns the documented finite fallback there as well
- verification_status: `verified`

## T-007 Example Launcher ROOT Alignment And Painter Startup Noise

- Status: passed
- Purpose: verify that the tracked example launcher no longer emits invalid ROOT noise when the active O2Physics ROOT revision changed since the last build, and that the saved participant canvas does not depend on delayed `HistPainter` autoload.
- Execution shape:
  - authoritatively reproduce the launcher noise under the active O2Physics ROOT environment
  - inspect the cached build ROOT prefix and generator `LC_RPATH`
  - repair the launcher so it refreshes stale ROOT-linked builds before running
  - link `ROOT::HistPainter` explicitly into `generate_blastwave_events`
  - rerun the tracked launcher outside the sandbox and inspect the output for the previous warning patterns
  - rerun `ctest --output-on-failure`
- Existing evidence:
  - authoritative diagnosis on 2026-04-23 showed:
    - `ROOTSYS=/Users/allenzhou/ALICE/sw/osx_arm64/ROOT/v6-36-10-alice1-local2`
    - `build/CMakeCache.txt` and the generator `LC_RPATH` still pointed at `ROOT/v6-36-10-alice1-local1`
    - the stale launcher run emitted duplicate `TClassTable::Add` warnings before the progress bar
    - the stale launcher run later emitted `Error in <TCling::LoadPCM>: ROOT PCM ... libHistPainter_rdict.pcm file does not exist`
  - an authoritative standalone local2 ROOT batch draw path did not reproduce the noise, isolating the problem to the stale generator binary rather than the active ROOT module alone
  - authoritative repair commands executed on 2026-04-23:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
    - `/Users/allenzhou/Research_software/Blast_wave/scripts/run_example_config.sh`
    - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
- Current result:
  - `generate_blastwave_events` now links `libHistPainter` and its `LC_RPATH` points at `ROOT/v6-36-10-alice1-local2/lib`
  - `scripts/run_example_config.sh` completed and wrote `/Users/allenzhou/Research_software/Blast_wave/qa/test_b8_5000.root`
  - the repaired launcher run emitted none of the earlier duplicate `TClassTable::Add` warnings
  - the repaired launcher run emitted none of the earlier `TCling::LoadPCM` `HistPainter` noise
  - `ctest --output-on-failure` passed all four registered tests
- verification_status: `verified`

## T-008 Fluid-Element Velocity Sampler Strategy Surface

- Status: passed
- Purpose: verify that the flow-selection contract is now expressed as a fluid-element velocity sampler surface, that the default covariance-ellipse path is preserved, and that the parallel `density-normal` sampler runs through generate+QA while only adding its sampler-specific density snapshot object.
- Execution shape:
  - reconfigure and rebuild the checkout
  - run `ctest --output-on-failure`
  - run one authoritative covariance-ellipse generate+QA smoke
  - run one authoritative `density-normal` generate+QA smoke
- Existing evidence:
  - local build/test commands executed on 2026-04-24:
    - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build --target generate_blastwave_events qa_blastwave_output test_flow_field_model test_run_options test_physics_utils -j4`
    - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
  - authoritative outside-sandbox O2Physics commands executed on 2026-04-24:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --output /tmp/blastwave_covariance_sampler_smoke.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_covariance_sampler_smoke.root --output /tmp/blastwave_covariance_sampler_smoke_validation.root --expect-nevents 20'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --output /tmp/blastwave_density_normal_sampler_smoke.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_density_normal_sampler_smoke.root --output /tmp/blastwave_density_normal_sampler_smoke_validation.root --expect-nevents 20'"`
    - `bash /Users/allenzhou/.codex/skills/cern_root/root-file-inspector/scripts/run_inspect_root_file.sh /tmp/blastwave_density_normal_sampler_smoke.root --max-keys 200`
    - `bash /Users/allenzhou/.codex/skills/cern_root/o2physics-root/scripts/run_root_command.sh --command 'root -l -b -q -e "TFile f(\"/tmp/blastwave_density_normal_sampler_smoke.root\", \"READ\"); auto* h = dynamic_cast<TH2*>(f.Get(\"density_normal_event_density_x-y\")); if (!h) { cout << \"STATUS: MISSING\" << endl; gSystem->Exit(2); } cout << \"STATUS: FOUND\" << endl; cout << \"DRAW_OPTION: \" << h->GetOption() << endl; cout << \"MAX_BIN: \" << h->GetMaximum() << endl; cout << \"MIN_BIN: \" << h->GetMinimum() << endl; gSystem->Exit(0);"'`
- Current result:
  - ROOT-free tests: passed
  - `test_run_options`: passed
  - `test_flow_field_model`: passed
  - default covariance-ellipse authoritative smoke: passed
  - covariance-ellipse QA summary: `validation_passed events=20 particles=7051 mean_Npart=181.6 mean_eps2=0.228879 mean_v2=0.0751083 max_abs_eta_s=7.2615 max_E=631.497 max_mass_shell_deviation=4.0778e-11`
  - `density-normal` authoritative smoke: passed
  - density-normal QA summary: `validation_passed events=20 particles=7051 mean_Npart=181.6 mean_eps2=0.228879 mean_v2=0.0619416 max_abs_eta_s=7.2615 max_E=647.385 max_mass_shell_deviation=7.59367e-11`
  - ROOT file inspection: `PRIMARY_OK`, `density_normal_event_density_x-y` key present in the output file
  - direct ROOT histogram query: `STATUS: FOUND`, `DRAW_OPTION: LEGO1`, `MAX_BIN: 5.5882`, `MIN_BIN: 0`
  - implementation note: the new surface keeps the mandatory ROOT payload unchanged while adding only runtime sampler selection, density-kernel width controls, and the optional sampler-specific `density_normal_event_density_x-y` `TH2`
- verification_status: `verified`

## T-009 EventMedium And Emission Interface Refactor

- Status: passed
- Purpose: verify that the internal `EventMedium` / `EmissionSite` refactor preserves the ROOT contract while removing the old `FlowFieldContext` interface.
- Execution shape:
  - rebuild the checkout
  - run ROOT-free regression tests
  - run one authoritative default covariance-ellipse generate+QA smoke
  - run one authoritative `density-normal` generate+QA smoke
  - inspect the density-normal ROOT file for the optional density snapshot object
- Existing evidence:
  - local commands executed on 2026-04-28:
    - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build --target generate_blastwave_events qa_blastwave_output test_flow_field_model test_emission_sampler test_maxwell_juttner_sampler test_output_path_utils test_run_options test_physics_utils -j4`
    - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
  - authoritative outside-sandbox O2Physics commands executed on 2026-04-28:
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --output /tmp/blastwave_eventmedium_covariance_smoke.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_eventmedium_covariance_smoke.root --output /tmp/blastwave_eventmedium_covariance_smoke_validation.root --expect-nevents 20'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --output /tmp/blastwave_eventmedium_density_normal_smoke.root'"`
    - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_eventmedium_density_normal_smoke.root --output /tmp/blastwave_eventmedium_density_normal_smoke_validation.root --expect-nevents 20'"`
    - `bash /Users/allenzhou/.codex/skills/cern_root/root-file-inspector/scripts/run_inspect_root_file.sh /tmp/blastwave_eventmedium_density_normal_smoke.root --max-keys 80`
- Current result:
  - ROOT-free tests: passed
  - registered tests now include `test_emission_sampler`
  - default covariance-ellipse authoritative smoke: passed
  - covariance-ellipse QA summary: `validation_passed events=20 particles=7077 mean_Npart=175.75 mean_eps2=0.276602 mean_v2=0.0924968 max_abs_eta_s=6.67501 max_E=984.787 max_mass_shell_deviation=3.58382e-11`
  - `density-normal` authoritative smoke: passed
  - density-normal QA summary: `validation_passed events=20 particles=7077 mean_Npart=175.75 mean_eps2=0.276602 mean_v2=0.0552974 max_abs_eta_s=6.67501 max_E=977.704 max_mass_shell_deviation=1.01809e-10`
  - ROOT file inspection: `RUNTIME_STATUS: PRIMARY_OK`, `STATUS: OK`, `density_normal_event_density_x-y [TH2F]` present with `entries=40000`
  - source search confirmed no tracked code occurrence of `FlowFieldContext`, `buildFlowFieldContext`, or `FlowDensitySample`
- verification_status: `verified`
