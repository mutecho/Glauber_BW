# Handoff

## Latest Durable Handoff

- Stage completed: repository layout cleanup plus docs/`project-state/` sync
- Artifact produced:
  - moved the historical reference ROOT macros into `reference/legacy-root-macros/`
  - moved the tracked helper launcher into `scripts/run_example_config.sh`
  - clarified the top-level repository contract in `README.md`
  - updated higher-authority docs and project-state records to the new structure
- Remaining durable follow-up:
  - decide whether `qa/` and `res/` should remain purely ignored local artifact directories or later be formalized under a more explicit output policy
  - keep using outside-sandbox ROOT smoke commands in Codex sessions until the sandbox/`alienv` PCM issue is fixed
- project_state_sync_status: `written`
- verification_status carried forward: `verified`

## Next Recommended Step

- If you want the output side to be as explicit as the source side, decide whether `qa/` and `res/` should keep their current informal local-artifact roles or be renamed/documented as distinct output classes.
- If future Codex-driven ROOT validation is expected, preserve the outside-sandbox invocation pattern because the sandboxed `alienv` ROOT runtime is still not authoritative on this machine.

## Suggested Validation Sequence

1. Check structure hygiene:
   - `git diff --check`
   - `bash -n /Users/allenzhou/Research_software/Blast_wave/scripts/run_example_config.sh`
2. If you want an end-to-end runtime smoke after the layout cleanup, reuse the established authoritative sequence:
   - configure with `cmake --preset default`
   - build with `cmake --build /Users/allenzhou/Research_software/Blast_wave/build`
   - generate with `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /tmp/blastwave_mj_default_smoke.root`
   - validate with `/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_mj_default_smoke.root --output /tmp/blastwave_mj_default_smoke_validation.root --expect-nevents 5000`

## What The Next Owner Should Not Redo

- Do not move helper shell launchers back into `config/`; keep static config and executable helpers separate.
- Do not reintroduce a personal-name top-level directory for historical reference macros; keep them under `reference/legacy-root-macros/`.
- Do not treat sandboxed `alienv` ROOT smoke output as authoritative when it emits PCM/module errors; rerun outside the sandbox instead.
- Do not treat historical `qa/` ROOT files as interchangeable proof of the latest physics baseline without checking which schema generation they came from.
