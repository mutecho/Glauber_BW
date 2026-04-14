# Handoff

## Latest Durable Handoff

- Stage completed: source-file responsibility split plus local policy tightening
- Artifact produced:
  - split the generator core into geometry, sampling, validation, and orchestration translation units
  - split the generation app into entrypoint, run-option parsing/progress, and ROOT output writer modules
  - introduced `PhysicsUtils` so producer and QA code reuse the same centrality and pseudorapidity helpers
  - updated repository-local `AGENTS.md` so future structure-cleanup requests explicitly cover oversized source files rather than only top-level directories
- Remaining durable follow-up:
  - `apps/qa_blastwave_output.cpp` is still a single read-and-validate executable and can be split further if you want the QA side to mirror the producer-side layering
  - keep using outside-sandbox ROOT smoke commands in Codex sessions until the sandbox/`alienv` PCM issue is fixed
- project_state_sync_status: `written`
- verification_status carried forward: `verified`

## Next Recommended Step

- If you want stricter symmetry between producer and validator, split `apps/qa_blastwave_output.cpp` into argument parsing, contract checks, and histogram-writing helpers.
- If future Codex-driven ROOT validation is expected, preserve the outside-sandbox invocation pattern because the sandboxed `alienv` ROOT runtime is still not authoritative on this machine.

## Suggested Validation Sequence

1. Check structure hygiene:
   - `git diff --check`
2. Rebuild the refactored checkout:
   - `cmake --build /Users/allenzhou/Research_software/Blast_wave/build`
3. Re-run the ROOT-free regression:
   - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
4. If you want an end-to-end runtime smoke after the split, reuse the established authoritative ROOT sequence:
   - generate with `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /tmp/blastwave_mj_default_smoke.root`
   - validate with `/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_mj_default_smoke.root --output /tmp/blastwave_mj_default_smoke_validation.root --expect-nevents 5000`

## What The Next Owner Should Not Redo

- Do not merge CLI parsing, ROOT writing, and generator orchestration back into one large `generate_blastwave_events.cpp`.
- Do not merge geometry, thermal/flow sampling, and validation back into one large `BlastWaveGenerator.cpp`.
- Do not treat sandboxed `alienv` ROOT smoke output as authoritative when it emits PCM/module errors; rerun outside the sandbox instead.
- Do not treat historical `qa/` ROOT files as interchangeable proof of the latest physics baseline without checking which schema generation they came from.
