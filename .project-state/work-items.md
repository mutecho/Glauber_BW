# Work Items

## WI-001 Baseline Validation On Current Checkout

- Status: open
- Goal: establish an authoritative end-to-end baseline for the current repository state.
- Why it exists: the repository has historical QA outputs, but this bootstrap task did not rerun build or validation commands.
- Suggested owner action:
  - configure and build with the documented O2Physics environment
  - run a bounded generator smoke test
  - run the independent QA reader on the generated ROOT file
  - update `.project-state/current-status.md`, `.project-state/tests.md`, and `.project-state/changelog.md` with the outcome
- Exit condition:
  - the current checkout has a recorded pass or a recorded failure mode tied to a concrete validation run
