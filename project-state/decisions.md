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

## DEC-002 Adopt A Lightweight Config-File Contract For `generate_blastwave_events`

- Status: accepted
- Date: 2026-04-11
- Context:
  - the generator needed a file-driven input path without bringing in a new parser dependency
  - the existing explicit CLI flag surface had to remain compatible
  - the repository benefits from a shipped example config that can be referenced from docs and smoke commands
- Decision:
  - support both `--config <path>` and positional `<config-path>` entrypoints
  - use plain-text `key = value` parsing with blank lines and full-line `#` comments ignored
  - keep the config-file key surface aligned with the current public CLI flags rather than exposing internal-only `BlastWaveConfig` fields
  - enforce deterministic precedence: explicit CLI overrides config file, config file overrides built-in defaults
  - resolve relative `output` paths in config files relative to the config file directory
  - ship `qa/test_b8.cfg` as the repository example config
- Consequences:
  - docs and help text must stay synchronized with the config-file key list
  - future CLI/config additions should update both explicit flag parsing and the shared config-file mapping
  - the config-file behavior is now part of the public CLI contract and should be validated when parser logic changes
