# Changelog

## 2026-04-10 Project-State Bootstrap

- Bootstrapped the `.project-state/` coordination ledger on explicit user request.
- Synthesized the initial project snapshot from human-written docs in `docs/`, repository structure, git status, and the presence of historical QA artifacts under `qa/`.
- Recorded the baseline verification state as `unverified` because no authoritative build or smoke validation was executed in this task.

## 2026-04-10 Participant Geometry Output Update

- Added explicit participant output to the generator:
  - `participants` tree with `event_id`, `nucleus_id`, `x`, `y`, `z`
  - `participant_x-y` histogram
  - `participant_x-y_canvas` canvas with nucleus-A and nucleus-B circle outlines
- Enabled the ROOT stats box on the saved participant canvas.
- Confirmed by direct ROOT inspection that the saved `participant_x-y_canvas` now contains a `TPaveStats` primitive.
- Extended the QA reader so it validates the participant tree, the participant histogram/canvas objects, and the consistency of `Npart` with the participant tree entry count.
- Updated human-facing and agent-facing docs to describe the new participant contract.
- Rebuilt the project and reran a fresh 10-event smoke validation in the O2Physics ROOT environment.
