# Handoff

## Latest Durable Handoff

- Stage completed: documentation compaction aligned with the updated project-knowledge rules
- What changed:
  - `docs/agent_guide.md` now keeps agent-facing routing, semantic guardrails, and doc-sync rules only
  - `docs/手记文档.md` now keeps just the high-level chain and easy-to-misread concepts
  - `project-state/guide.md`, `project-state/current-status.md`, and `project-state/tests.md` were rewritten to act as concise current-view ledgers instead of duplicated manuals or raw command archives
- Current documentation ownership:
  - detailed runtime and physics explanation: `docs/项目说明.md`
  - concise semantic reminders: `docs/手记文档.md`
  - current coordination view: `project-state/guide.md`
  - current snapshot and caveats: `project-state/current-status.md`
  - summarized durable validation evidence: `project-state/tests.md`

## Current Contract Reminders

- default runtime path remains V1a `affine-gaussian + covariance-ellipse`
- V2 `gradient-response` remains opt-in and coupled across medium and flow selection
- differential `v2{2}(pT)` remains a separate analysis payload and does not replace `events.v2`
- `v2pt-output-mode = separate-file` may leave the main result file metadata-only
- authoritative ROOT validation on this machine still comes from the outside-sandbox O2Physics path

## Remaining Follow-Up

- if the active code worktree changes physics, config parsing, schema, QA behavior, or operator flow again, refresh `docs/项目说明.md` and the relevant `project-state/` files in the same patch
- if a future task needs to prove the current dirty tree end to end, rerun the authoritative build, `ctest`, and outside-sandbox ROOT smoke commands, then update `project-state/tests.md` only if the durable conclusion changes

## Verification Status

- carried forward durable baseline: `verified`
- this documentation-only task did not add a new validation run
