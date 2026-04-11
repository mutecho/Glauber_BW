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

## ISSUE-002 Historical ROOT Outputs Use The Old Output Contract

- Type: migration note
- Status: open
- Evidence:
  - files generated before the participant-geometry update do not contain `participants`, `participant_x-y`, or `participant_x-y_canvas`
- Impact:
  - downstream inspection of older sample files may fail if the new QA reader or new visual checks are applied without regenerating those files
- Recommended resolution:
  - regenerate any long-lived sample files that should support the new participant checks and visuals

## ISSUE-003 Codex ROOT Read-Side Validation Is Not Currently Authoritative

- Type: environment gap
- Status: open
- Evidence:
  - in this Codex session, `qa_blastwave_output` failed with ROOT PCM/module loading errors
  - the same failure appeared even when the QA reader was pointed at pre-existing sample files such as `qa/test_b8.root`
  - generator-side runs still completed and wrote output files, so the observed gap is specific to the read-side validation environment in this session
- Impact:
  - config-file CLI changes cannot yet be marked `verified` from this session alone
  - the repository still needs one authoritative read-side validation in the user's known-good ROOT environment for the new config-file path
- Recommended resolution:
  - rerun generation from `qa/test_b8.cfg` and validate the resulting ROOT file with `qa_blastwave_output` in the user's known-good environment
