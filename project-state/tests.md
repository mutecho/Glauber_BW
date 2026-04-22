# Tests

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
