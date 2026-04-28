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
  - expose its public CLI/config tuning surface as `rho0`, `rho2`, and `flow-power`
  - keep `events.eps2` and `events.psi2` on the existing summary convention
  - keep extra flow-ellipse diagnostics optional behind the public `debug-flow-ellipse` switch
  - reject legacy `vmax`, `kappa2`, and `r-ref` inputs with explicit migration guidance instead of silently mapping them
- Consequences:
  - future flow-field work should extend `FlowFieldModel` rather than reimplementing covariance math inside the generator or QA code
  - docs, help text, and sample configs must all use the `rho*` / `flow-power` vocabulary
  - default ROOT consumers keep the same mandatory contract, while debug-aware workflows may opt into the extra tree and normalized-participant histogram

## DEC-004 Add Event-Level Final-State `v2` To The Mandatory ROOT Contract

- Status: accepted
- Date: 2026-04-22
- Context:
  - the project already exposed initial-state geometry observables such as `eps2` and `psi2`, but it still lacked one compact event-level final-state anisotropy observable in the default output
  - the user explicitly requested an event `v2` plus a corresponding summary histogram
  - the generator and independent QA reader needed one shared definition so the new branch and histogram could be validated deterministically
- Decision:
  - extend the mandatory `events` tree with a `v2` branch
  - extend the mandatory embedded QA objects with a `v2` histogram
  - define `events.v2` with the historical eventwise second-harmonic Q-vector magnitude:
    - `Q2x = sum_i cos(2 * phi_i)`
    - `Q2y = sum_i sin(2 * phi_i)`
    - `v2 = sqrt(Q2x^2 + Q2y^2) / Nch`
  - compute the observable from the written final-state particle azimuths rather than from the initial-state `psi2`
  - keep the implementation shared between producer and QA in `PhysicsUtils`
- Consequences:
  - downstream readers can treat `events.v2` as a stable default final-state anisotropy summary
  - `v2` is now distinct by contract from initial-state `eps2` and from any future `mean(cos(2 * (phi - psi2)))` or `v2(pT)` outputs
  - older sample files may legitimately miss `events.v2` and `v2`, so long-lived QA artifacts should be regenerated when the repository wants fully current reference files

## DEC-005 Generalize Flow Selection Into A Fluid-Element Velocity Sampler Surface

- Status: accepted
- Date: 2026-04-23
- Context:
  - the repository already shipped one covariance-ellipse normal-flow implementation
  - the new `density-normal` method should not freeze the public abstraction around one specific profile name because more velocity-sampling methods may be added later
  - the project still needed to preserve the existing default behavior and ROOT contract while separating geometry recovery, density evaluation, and sampler dispatch in the ROOT-free core
- Decision:
  - generalize the public flow-selection surface into a fluid-element velocity sampler option rather than a narrowly named `flow-model`
  - keep `covariance-ellipse` as the default sampler
  - add `density-normal` as a parallel sampler choice under `flow-velocity-sampler`
  - add `flow-density-sigma` as a dedicated density-reconstruction width independent of `smear`
  - keep `rho0`, `rho2`, and `flow-power` on the public surface, with `rho2` intentionally parsed but unused by `density-normal`
  - keep `events.eps2` and `events.psi2` on the existing covariance definition independent of sampler choice
- Consequences:
  - future flow-field extensions should plug into the sampler dispatch surface instead of renaming the public interface again
  - sample configs, help text, and durable tests must use `flow-velocity-sampler` / `flow-density-sigma` rather than reviving `flow-model`
  - `smear` and `flow-density-sigma` now carry different responsibilities and should not be silently coupled

## DEC-006 Adopt `EventMedium` And `EmissionSite` As Internal Expansion Interfaces

- Status: accepted
- Date: 2026-04-28
- Context:
  - future work is expected to change particle emission from direct participant-hotspot smearing to a pipeline where participants build a density field, the density field may expand, and particles are sampled from the emission-stage field
  - the current implementation already had participant geometry, density-gradient flow, and ROOT density snapshots, but still routed event generation through hotspot-specific helper names
  - the user explicitly requested a full internal naming migration without retaining `FlowFieldContext` / `buildFlowFieldContext` compatibility shims
- Decision:
  - use `EventMedium` as the event-level ROOT-free medium state
  - distinguish `participantGeometry`, `initialDensity`, `emissionDensity`, and `emissionGeometry`
  - keep `DensityEvolutionMode::None` as the only implemented evolution mode for now, so initial and emission-stage fields are currently identical
  - route transverse emission through `EmissionSite` and `sampleEmissionSites()`
  - keep CLI/config keys and ROOT schema unchanged for this internal refactor
- Consequences:
  - future density-expansion implementations should update `emissionDensity` / `emissionGeometry` in `buildEventMedium()` rather than changing `events.eps2` / `events.psi2`
  - future density-field emission backends should add an `EmissionSamplerMode` and still return `EmissionSite`
  - writer-side density snapshots should serialize the generator-owned `EventMedium` state instead of rebuilding density independently
