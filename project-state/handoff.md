# Handoff

## Latest Durable Handoff

- Stage completed: `project-state/` ledger sync to the current post-centrality repository baseline
- Artifact produced:
  - added the missing `project-state/guide.md` bootstrap file
  - synchronized the ledger to the current canonical path `project-state/` instead of the historical `.project-state/` spelling
  - carried forward the latest code baseline: participant output, config-file CLI support, and the `events.centrality` / `cent` contract
  - corrected the tracked example config location in the ledger from `qa/test_b8.cfg` to `config/test_b8.cfg`
  - retired the earlier “Codex ROOT read-side environment is not authoritative” blocker as the primary explanation for partial verification
- Remaining durable follow-up:
  - higher-authority docs in `docs/` still point at the old config path
  - the current tracked config-file path still lacks one durable generate+QA validation record in `project-state/tests.md`
- project_state_sync_status: `written`
- verification_status carried forward: `partially verified`

## Next Recommended Step

- Make `config/test_b8.cfg` the single canonical documented sample-config path across the higher-authority docs:
  - update `docs/项目说明.md`
  - update `docs/agent_guide.md`
- Then record one authoritative end-to-end validation for the current tracked sample config:
  - generate from `/Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg`
  - preferably override the output into `qa/`, because the config file's relative `output` otherwise resolves under `config/`
  - validate the generated ROOT file with `qa_blastwave_output`
  - if that succeeds, update `project-state/tests.md`, `project-state/current-status.md`, and `project-state/changelog.md` to promote the config-file path from `partially verified` to `verified`

## Suggested Validation Sequence

1. Configure with the default preset:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
2. Build the current checkout:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
3. Generate an explicit-CLI smoke file if a fresh baseline is desired:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 100 --b 8 --output /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke.root'"`
4. Validate that smoke file with the independent QA reader:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke.root --output /Users/allenzhou/Research_software/Blast_wave/qa/project_state_smoke_validation.root --expect-nevents 100'"`
5. Run the tracked config-file path with an explicit QA-friendly output override:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke.root'"`
6. Validate the config-driven output:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke.root --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke_validation.root --expect-nevents 5000'"`

## What The Next Owner Should Not Redo

- Do not recreate the ledger bootstrap; it is now complete at the minimum required level.
- Do not reintroduce `.project-state/` path references; use `project-state/`.
- Do not treat historical `qa/` ROOT files as interchangeable proof of the current full schema.
- Do not reopen the old ROOT read-side environment blocker unless fresh failing evidence appears after the 2026-04-13 passed smoke QA.
