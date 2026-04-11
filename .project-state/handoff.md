# Handoff

## Latest Durable Handoff

- Stage completed: config-file CLI support and shipped example config
- Artifact produced:
  - generator now supports config-file entrypoints in addition to explicit flags
  - config-file parsing now uses a shared option-application path with deterministic precedence
  - repository now ships `qa/test_b8.cfg` as a runnable sample config
  - human-facing and agent-facing docs now mention the shipped config and the config-file contract
- project_state_sync_status: `written`
- verification_status carried forward: `partially verified`

## Next Recommended Step

- Run one authoritative end-to-end validation of the config-file CLI in the user's known-good ROOT environment:
  - generate from `qa/test_b8.cfg`
  - validate the resulting ROOT file with `qa_blastwave_output`
  - if that succeeds, update `tests.md`, `current-status.md`, and `changelog.md` to move this task from `partially verified` to `verified`

## Suggested Validation Sequence

1. Configure with the default preset:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
2. Build the current checkout:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
3. Generate a bounded smoke sample:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 100 --b 8 --output /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke.root'"`
4. Run the independent QA reader:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke.root --output /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke_validation.root --expect-nevents 100'"`
5. Repeat the same read-side validation for the new config-file path:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/qa/test_b8.cfg'"`
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_from_config.root --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_from_config_validation.root --expect-nevents 100'"`

## What The Next Owner Should Not Redo

- Do not recreate the bootstrap ledger structure unless the project intentionally changes the `.project-state/` convention.
- Do not treat `.project-state/` as a higher-authority source than code, tests, or the human-written docs under `docs/`.
- Do not forget that output files created before the participant-contract change will lack the new participant objects unless regenerated.
