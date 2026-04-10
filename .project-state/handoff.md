# Handoff

## Latest Durable Handoff

- Stage completed: participant-geometry output enhancement plus fresh end-to-end validation
- Artifact produced:
  - generator now writes a `participants` tree
  - generator now writes `participant_x-y`
  - generator now writes `participant_x-y_canvas` with nucleus-A and nucleus-B circle outlines
  - participant canvas keeps the ROOT stats box enabled
  - QA now checks the new participant tree and participant visual objects
- project_state_sync_status: `written`
- verification_status carried forward: `passed`

## Next Recommended Step

- Regenerate any long-lived sample outputs that should expose the new participant contract, especially `qa/test_b8_5000.root`, if downstream inspection will use those files.
- If a larger validation sample is generated, update:
  - `tests.md`
  - `changelog.md`
  - `current-status.md`
- If any downstream tool assumes the old two-tree format, document that migration point in:
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
- Do not forget that output files created before the participant-contract change will lack the new participant objects unless regenerated.
