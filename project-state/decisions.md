# Decisions

## DEC-001 Adopt `project-state/` As A Coordination Ledger

- Status: accepted
- Date: 2026-04-10
- Context:
  - the user explicitly bootstrapped a repository-local coordination ledger for `/Users/allenzhou/Research_software/Blast_wave`
  - the directory was later normalized to the canonical path `project-state/`
  - future engineering work benefits from a durable, low-authority coordination snapshot
- Decision:
  - use `project-state/` as the repository's engineering coordination ledger
  - keep it lower authority than code and tests for behavior, and lower authority than human-written docs in `docs/` for design intent and requirements
  - normally update `current-status.md`, `handoff.md`, and `changelog.md`, and touch the remaining files only when a task materially changes them
- Consequences:
  - later tasks should preserve canonical verification labels
  - future updates should be incremental rather than full-file rewrites
  - future references should use `project-state/`, not `.project-state/`

## DEC-002 Adopt A Lightweight Config-File Contract For `generate_blastwave_events`

- Status: accepted
- Date: 2026-04-11
- Context:
  - the generator needed a file-driven input path without bringing in a new parser dependency
  - the existing explicit CLI flag surface had to remain compatible
  - the repository benefits from a tracked example config that can be referenced from docs and smoke commands
- Decision:
  - support both `--config <path>` and positional `<config-path>` entrypoints
  - use plain-text `key = value` parsing with blank lines and full-line `#` comments ignored
  - keep the config-file key surface aligned with the current public CLI flags rather than exposing internal-only `BlastWaveConfig` fields
  - enforce deterministic precedence: explicit CLI overrides config file, config file overrides built-in defaults
  - resolve relative `output` paths in config files relative to the config file directory
  - treat `config/test_b8.cfg` as the current canonical repository example config path
- Consequences:
  - docs and help text must stay synchronized with the config-file key list and the example-config path
  - future CLI/config additions should update both explicit flag parsing and the shared config-file mapping
  - the config-file behavior is now part of the public CLI contract and should be validated when parser logic changes

## DEC-003 Replace The Default Flow Model With The Covariance-Ellipse Normal Flow

- Status: accepted
- Date: 2026-04-22
- Context:
  - the previous default flow model used a lab-origin radial direction with `vMax`, `referenceRadius`, and `kappa2`
  - that model did not match the documented covariance-ellipse normal-flow intent
  - the project needed the default implementation, public config surface, optional debug payload, and QA strategy to align on one consistent flow-field contract
- Decision:
  - make the participant covariance ellipse the only default flow model
  - expose its public tuning surface as `rho0`, `rho2`, and `flowPower`
  - keep `events.eps2` and `events.psi2` on the existing summary convention
  - keep extra flow-ellipse diagnostics optional behind `debugFlowEllipse`
  - reject legacy `vmax`, `kappa2`, and `r-ref` inputs with explicit migration guidance instead of silently mapping them
- Consequences:
  - future flow-field work should extend `FlowFieldModel` rather than reimplementing covariance math inside the generator or QA code
  - docs, help text, and sample configs must all use the `rho*` / `flow-power` vocabulary
  - default ROOT consumers keep the same mandatory contract, while debug-aware workflows may opt into the extra tree and normalized-participant histogram
