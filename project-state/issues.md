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
  - files generated before the 2026-04-28 V1a density-evolution update also do not contain `events.eps2_f`, `events.psi2_f`, `events.chi2`, or their QA histograms
  - the `qa/` directory currently holds samples from multiple generation dates and therefore from multiple schema baselines
- Impact:
  - downstream inspection of older sample files may fail if newer QA checks are applied without regenerating those files
  - operators can misread an old sample as a current reference artifact when it only matches part of the contract
- Recommended resolution:
  - regenerate any long-lived sample files that should support the current participant, centrality, event-`v2`, and freeze-out geometry checks

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
- Status: resolved on 2026-04-14
- Evidence:
  - the tracked sample config in the current checkout is `config/test_b8.cfg`
  - `scripts/run_example_config.sh` also invokes `config/test_b8.cfg`
  - higher-authority docs under `docs/` now also refer to `config/test_b8.cfg`
  - a durable config-file generate+QA record was added on 2026-04-14 using that canonical path
- Impact:
  - no current blocking impact remains from the earlier path drift
- Recommended resolution:
  - preserve `config/test_b8.cfg` as the canonical tracked example path unless the file is intentionally moved again

## ISSUE-005 Sandboxed `alienv` ROOT Runs Are Not Authoritative In Codex

- Type: environment gap
- Status: open
- Evidence:
  - sandboxed 2026-04-14 `alienv` ROOT smoke commands emitted PCM/module loading errors and produced a non-authoritative QA result
  - the same generate and QA commands passed immediately after rerunning outside the sandbox with escalation
- Impact:
  - Codex-hosted ROOT smoke and QA results can be misleading when run inside the sandbox on this machine
  - authoritative ROOT validation currently requires an outside-sandbox rerun
- Recommended resolution:
  - keep using outside-sandbox `alienv` ROOT commands for authoritative validation in Codex sessions until the sandbox/PCM interaction is fixed

## ISSUE-006 Example Launcher Reused A Stale ROOT-Linked Generator

- Type: runtime/build mismatch
- Status: resolved on 2026-04-23
- Evidence:
  - authoritative diagnosis showed `scripts/run_example_config.sh` entering `ROOT/v6-36-10-alice1-local2` while the cached build and generator `LC_RPATH` still pointed at `ROOT/v6-36-10-alice1-local1`
  - that mismatch produced duplicate `TClassTable::Add` warnings before the progress bar and delayed `TCling::LoadPCM` noise when the writer saved `participant_x-y_canvas`
  - the launcher now refreshes the generator build when the cached or binary ROOT prefix differs from `ROOTSYS`
  - `generate_blastwave_events` now links `ROOT::HistPainter` explicitly so the saved canvas does not depend on late painter autoload
- Impact:
  - operators could get large amounts of misleading ROOT noise from the tracked example launcher even though event generation still finished
- Recommended resolution:
  - preserve the launcher ROOT preflight and the explicit `HistPainter` link while the participant canvas remains part of the output contract
