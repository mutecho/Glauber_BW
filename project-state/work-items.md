# Work Items

## WI-001 Baseline Validation On Current Checkout

- Status: done
- Goal: establish an authoritative end-to-end baseline for the current repository state.
- Why it exists: the repository has historical QA outputs, but this bootstrap task did not rerun build or validation commands.
- Suggested owner action:
  - configure and build with the documented O2Physics environment
  - run a bounded generator smoke test
  - run the independent QA reader on the generated ROOT file
  - update `project-state/current-status.md`, `project-state/tests.md`, and `project-state/changelog.md` with the outcome
- Exit condition:
  - the current checkout has a recorded pass or a recorded failure mode tied to a concrete validation run

## WI-002 Regenerate Long-Lived Sample Outputs For The Current Output Contract

- Status: open
- Goal: ensure commonly reused ROOT sample files expose the current participant and centrality objects.
- Why it exists:
  - the generator and QA contract now require participant-level objects
  - the newest schema also includes `events.centrality` and `cent`
  - older sample files remain useful historical artifacts but no longer match the newest file schema
- Suggested owner action:
  - regenerate representative files such as `qa/test_b8_5000.root`
  - rerun the QA reader on the regenerated files
  - update `tests.md` and `changelog.md` with the larger-sample evidence if that run is intended to be durable
- Exit condition:
  - the project's standard reference sample files have the current participant and centrality objects plus matching QA outputs

## WI-003 Authoritative Validation For The Config-File CLI Path

- Status: open
- Goal: move the config-file CLI change from `partially verified` to `verified`.
- Why it exists:
  - parser behavior and generator-side execution were exercised successfully
  - the tracked example config path is now `config/test_b8.cfg`
  - higher-authority docs still point to the old path `qa/test_b8.cfg`
  - the ledger does not yet contain one durable generate+QA record tied to the current tracked config path
- Suggested owner action:
  - update `docs/项目说明.md` and `docs/agent_guide.md` to use `config/test_b8.cfg`
  - run `generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke.root`
  - run `qa_blastwave_output` on `qa/test_b8_cfg_smoke.root`
  - if successful, update `project-state/current-status.md`, `project-state/tests.md`, and `project-state/changelog.md`
- Exit condition:
  - the canonical tracked sample config has one durable end-to-end generate-and-validate result recorded in `project-state/`
