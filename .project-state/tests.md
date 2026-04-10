# Tests

## T-001 Baseline Build And Smoke Validation

- Status: planned
- Purpose: verify that the current checkout still configures, builds, generates events, and passes the independent QA reader.
- Execution shape:
  - configure with `cmake --preset default`
  - build with `cmake --build .../build`
  - generate a small ROOT sample with `generate_blastwave_events`
  - validate it with `qa_blastwave_output`
- Existing evidence:
  - human-written docs in `docs/agent_guide.md` and `docs/项目说明.md` describe the expected build and smoke commands
  - historical QA artifacts are present in `qa/`
- Current result:
  - not rerun in this task
- verification_status: `unverified`
