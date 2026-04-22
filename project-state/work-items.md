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
  - the newest schema also includes `events.v2` and `v2`
  - the default thermal momentum baseline is now `maxwell-juttner`, so older Gamma-era samples are no longer representative of the default runtime configuration
- older sample files remain useful historical artifacts but no longer match the newest file schema
- Suggested owner action:
  - regenerate representative files such as `qa/test_b8_5000.root`
  - rerun the QA reader on the regenerated files
  - update `tests.md` and `changelog.md` with the larger-sample evidence if that run is intended to be durable
- Exit condition:
  - the project's standard reference sample files have the current participant, centrality, and event-`v2` objects plus matching QA outputs

## WI-003 Authoritative Validation For The Config-File CLI Path

- Status: done on 2026-04-14
- Goal: move the config-file CLI change from `partially verified` to `verified`.
- Why it exists:
  - parser behavior and generator-side execution had already been exercised successfully
  - the tracked example config path had been normalized to `config/test_b8.cfg`
  - higher-authority docs and one durable generate+QA record were still missing at the time this work item was opened
- Suggested owner action:
  - completed on 2026-04-14 by updating `docs/项目说明.md` and `docs/agent_guide.md`
  - completed on 2026-04-14 by recording an authoritative default-MJ config-file generate+QA run in `project-state/tests.md`
- Exit condition:
  - satisfied: the canonical tracked sample config now has one durable end-to-end generate-and-validate result recorded in `project-state/`

## WI-004 Authoritative ROOT Smoke Path For Codex Sessions

- Status: open
- Goal: preserve one clear authoritative procedure for ROOT smoke and QA runs in Codex sessions on this machine.
- Why it exists:
  - sandboxed `alienv` ROOT runs on 2026-04-14 emitted PCM/module loading errors
  - the same commands passed immediately after rerunning outside the sandbox with escalation
- Suggested owner action:
  - either document the outside-sandbox invocation pattern wherever recurring validation is expected
  - or add a small wrapper/workflow that makes the authoritative execution path explicit
- Exit condition:
  - future Codex-run ROOT smoke validations have one documented authoritative execution path
