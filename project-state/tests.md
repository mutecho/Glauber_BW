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
    - `config/run.sh`
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
