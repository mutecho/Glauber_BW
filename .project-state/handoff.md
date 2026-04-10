# Handoff

## Latest Durable Handoff

- Stage completed: project-state bootstrap using `analyze-requirement` plus `sync-project-knowledge` in `both` mode with `writeback_target: project-state`
- Artifact produced: initial `.project-state/` coordination ledger with seven files
- project_state_sync_status: `written`
- verification_status carried forward: `unverified`

## Next Recommended Step

- Run an authoritative baseline validation on the current checkout inside the O2Physics ROOT environment.
- If build and smoke validation pass, update:
  - `current-status.md`
  - `tests.md`
  - `changelog.md`
- If validation fails, record the failure mode in:
  - `tests.md`
  - `issues.md`
  - `handoff.md`

## Suggested Validation Sequence

1. Configure with the default preset:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
2. Build the current checkout:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
3. Generate a bounded smoke sample:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 100 --b 8 --output /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke.root'"`
4. Run the independent QA reader:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke.root --output /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke_validation.root --expect-nevents 100'"`

## What The Next Owner Should Not Redo

- Do not recreate the bootstrap ledger structure unless the project intentionally changes the `.project-state/` convention.
- Do not treat `.project-state/` as a higher-authority source than code, tests, or the human-written docs under `docs/`.
