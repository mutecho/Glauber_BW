# Issues

## ISSUE-001 Validation Freshness Is Unknown For The Current Checkout

- Type: verification gap
- Status: resolved on 2026-04-10
- Evidence:
  - a fresh configure/build/generate/validate sequence was executed on 2026-04-10
  - the updated QA reader reported `validation_passed events=10 particles=3668 mean_Npart=185.5 mean_eps2=0.237373 max_abs_eta_s=6.41806 max_E=222.582 max_mass_shell_deviation=9.16636e-12`
- Impact:
  - no current blocking impact remains for checkout freshness
- Recommended resolution:
  - preserve the passed status unless later code changes invalidate it

## ISSUE-002 Historical ROOT Outputs Span Older Output Contracts

- Type: migration note
- Status: open
- Evidence:
  - files generated before the participant-geometry update do not contain `participants`, `participant_x-y`, or `participant_x-y_canvas`
  - files generated before the 2026-04-13 centrality extension also do not contain `events.centrality` or `cent`
  - the `qa/` directory currently holds samples from multiple generation dates and therefore from multiple schema baselines
- Impact:
  - downstream inspection of older sample files may fail if newer QA checks are applied without regenerating those files
  - operators can misread an old sample as a current reference artifact when it only matches part of the contract
- Recommended resolution:
  - regenerate any long-lived sample files that should support the current participant and centrality checks

## ISSUE-003 Codex ROOT Read-Side Validation Is Not Currently Authoritative

- Type: environment gap
- Status: resolved on 2026-04-13
- Evidence:
  - an earlier session reported ROOT PCM/module loading errors when `qa_blastwave_output` read pre-existing files
  - a later 2026-04-13 smoke validation recorded in `project-state/tests.md` passed both generation and independent QA on `/tmp/blastwave_centrality_smoke.root`
- Impact:
  - no current blocking impact remains from this environment issue alone
- Recommended resolution:
  - keep this issue resolved unless new read-side failures appear in a later authoritative run

## ISSUE-004 Higher-Authority Docs Still Point To The Old Config Example Path

- Type: documentation drift
- Status: open
- Evidence:
  - the tracked sample config in the current checkout is `config/test_b8.cfg`
  - `config/run.sh` also invokes `config/test_b8.cfg`
  - higher-authority docs under `docs/` still refer to `qa/test_b8.cfg`
  - earlier `project-state/` entries also carried the stale `qa/test_b8.cfg` path before this sync task corrected them
- Impact:
  - users following the docs can try to run a non-canonical or nonexistent path
  - the config-file CLI change cannot yet be treated as fully documented and reproducibly validated from the checkout alone
- Recommended resolution:
  - choose `config/test_b8.cfg` as the canonical path unless the file is intentionally moved again
  - update the higher-authority docs to match that path
  - record one durable generate+QA validation run using the canonical tracked config path
