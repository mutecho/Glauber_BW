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
- Current result:
  - configure: passed
  - build: passed
  - generator run: passed
  - QA run: passed
  - QA summary: `validation_passed events=10 particles=3668 mean_Npart=185.5 mean_eps2=0.237373 max_abs_eta_s=6.41806 max_E=222.582 max_mass_shell_deviation=9.16636e-12`
  - output contract includes `events.centrality` and `cent`
- verification_status: `passed`

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

- Status: partially verified
- Purpose: verify that `generate_blastwave_events` accepts repository-shipped config files, preserves explicit CLI compatibility, and enforces the new parser contract with actionable errors.
- Execution shape:
  - rebuild the current checkout
  - inspect `--help`
  - generate from the shipped sample config `qa/test_b8.cfg`
  - exercise parser failure paths without depending on ROOT read-side validation
- Existing evidence:
  - build completed after the config-file CLI change
  - `--help` reports:
    - `generate_blastwave_events --config <path> [options]`
    - `generate_blastwave_events <config-path> [options]`
    - `explicit CLI options > configuration file values > built-in defaults`
  - `./bin/generate_blastwave_events qa/test_b8.cfg` completed and reported:
    - `Wrote 100 events to qa/test_b8_from_config.root`
  - parser failure-path checks returned the expected errors for:
    - missing config file
    - invalid config line syntax
    - unknown key
    - duplicate key
    - invalid numeric value
    - simultaneous positional config path and `--config`
- Current result:
  - parser and entrypoint behavior: passed
  - shipped sample config generation: passed
  - independent QA reader validation for this change set: not yet reproduced in the current Codex ROOT read environment because the reader hit ROOT PCM/module loading errors even on pre-existing sample files
- verification_status: `partially verified`
