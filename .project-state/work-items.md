# Work Items

## WI-001 Baseline Validation On Current Checkout

- Status: done
- Goal: establish an authoritative end-to-end baseline for the current repository state.
- Why it exists: the repository has historical QA outputs, but this bootstrap task did not rerun build or validation commands.
- Suggested owner action:
  - configure and build with the documented O2Physics environment
  - run a bounded generator smoke test
  - run the independent QA reader on the generated ROOT file
  - update `.project-state/current-status.md`, `.project-state/tests.md`, and `.project-state/changelog.md` with the outcome
- Exit condition:
  - the current checkout has a recorded pass or a recorded failure mode tied to a concrete validation run

## WI-002 Regenerate Long-Lived Sample Outputs For The Participant Contract

- Status: open
- Goal: ensure commonly reused ROOT sample files expose the new participant tree and participant visual objects.
- Why it exists:
  - the generator and QA contract now require participant-level objects
  - older sample files remain useful historical artifacts but no longer match the newest file schema
- Suggested owner action:
  - regenerate representative files such as `qa/test_b8_5000.root`
  - rerun the QA reader on the regenerated files
  - update `tests.md` and `changelog.md` with the larger-sample evidence if that run is intended to be durable
- Exit condition:
  - the project's standard reference sample files have the new participant objects and matching QA outputs

## WI-003 Authoritative Validation For The Config-File CLI Path

- Status: open
- Goal: move the config-file CLI change from `partially verified` to `verified`.
- Why it exists:
  - parser behavior and generator-side execution were exercised successfully
  - the read-side QA step was not authoritative in the current Codex session because ROOT PCM/module loading failed before normal QA could complete
- Suggested owner action:
  - run `generate_blastwave_events qa/test_b8.cfg` in the user's known-good ROOT environment
  - run `qa_blastwave_output` on `qa/test_b8_from_config.root`
  - if successful, update `.project-state/current-status.md`, `.project-state/tests.md`, and `.project-state/changelog.md`
- Exit condition:
  - the shipped sample config has one durable end-to-end generate-and-validate result recorded in `.project-state/`
