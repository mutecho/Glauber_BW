# Changelog

## 2026-04-10 Project-State Bootstrap

- Bootstrapped the project coordination ledger on explicit user request.
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

## 2026-04-11 Config-File CLI And Example Config

- Added config-file support to `generate_blastwave_events` while keeping explicit CLI flags compatible:
  - `--config <path>`
  - positional `<config-path>`
- Adopted a lightweight `key = value` configuration contract with explicit precedence:
  - CLI overrides config file
  - config file overrides built-in defaults
  - relative `output` paths inside config files resolve relative to the config file directory
- Added a tracked example config for the repository workflow.
- Updated `docs/agent_guide.md` and `docs/项目说明.md` to document the new config-file interface and point to the example config.

## 2026-04-13 Centrality Output Contract

- Extended the event-level output contract with `events.centrality`.
- Added the `cent` histogram to the generated ROOT QA objects.
- Extended the independent QA reader so it checks:
  - `centrality` remains within `[0, 100]`
  - `centrality` matches the current fixed-`b` mapping
  - fixed-`b` runs keep one constant centrality value across all events
- Recorded a passed 10-event generate+QA smoke validation for the centrality-output extension in `project-state/tests.md`.

## 2026-04-13 Project-State Resync To Current Baseline

- Added the missing `project-state/guide.md` file to complete the minimum required bootstrap set.
- Synchronized the coordination ledger to the canonical `project-state/` path spelling.
- Corrected the tracked example-config location in the ledger from `qa/test_b8.cfg` to `config/test_b8.cfg`.
- Closed the old read-side ROOT environment blocker using the later passed smoke-QA evidence and shifted the remaining open gap to config-path documentation drift plus missing durable config-path QA evidence.
