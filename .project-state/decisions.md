# Decisions

## DEC-001 Adopt `.project-state/` As A Coordination Ledger

- Status: accepted
- Date: 2026-04-10
- Context:
  - the user explicitly bootstrapped `/Users/allenzhou/Research_software/Blast_wave/.project-state/`
  - future engineering work benefits from a durable, low-authority coordination snapshot
- Decision:
  - use `.project-state/` as the repository's engineering coordination ledger
  - keep it lower authority than code and tests for behavior, and lower authority than human-written docs in `docs/` for design intent and requirements
  - normally update `current-status.md`, `handoff.md`, and `changelog.md`, and touch the remaining files only when a task materially changes them
- Consequences:
  - later tasks should preserve canonical verification labels
  - future updates should be incremental rather than full-file rewrites
