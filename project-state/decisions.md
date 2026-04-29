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
  - Superseded by DEC-008 for the second-order public knob: current flow tuning uses `kappa2`, not `rho2`.

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
  - Superseded by DEC-008 for the second-order public knob: current flow tuning uses `kappa2`, not `rho2`.

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

## DEC-007 Make V1a Affine Gaussian Response The Default Medium Mode

- Status: accepted
- Date: 2026-04-28
- Context:
  - `docs/演化V1_固定参数.md` defines the V1a target as a parameterized freeze-out response model where `s_f(x,y) != s_0(x,y)`
  - the existing `EventMedium` interface already separated `participantGeometry`, `initialDensity`, `emissionDensity`, and `emissionGeometry`
  - the user wanted the new mode switchable against the two existing flow-direction modes
- Decision:
  - add `density-evolution = affine-gaussian | none` as the public medium-mode switch
  - make `affine-gaussian` the default and keep `none` as the legacy identity-medium comparison path
  - keep `density-evolution` orthogonal to `flow-velocity-sampler = covariance-ellipse | density-normal`
  - use fixed V1a response parameters for now: `lambda_in = 1.20`, `lambda_out = 1.05`, and `sigma_evo = 0.5 fm`
  - write freeze-out geometry diagnostics as mandatory `events.eps2_f`, `events.psi2_f`, `events.chi2` plus matching QA histograms
  - preserve `events.eps2` and `events.psi2` as initial participant-geometry observables
- Consequences:
  - current QA rejects older ROOT files that lack `eps2_f`, `psi2_f`, or `chi2`
  - long-lived sample ROOT files should be regenerated when a current reference artifact set is needed
  - future V1b or parameter scans should extend this medium-mode contract rather than overloading `flow-velocity-sampler`

## DEC-008 Use `kappa2` As The Public Second-Order Flow Response Coefficient

- Status: accepted
- Date: 2026-04-28
- Context:
  - the V1 requirement defines the second-order response as `kappa2 * epsilon2`, not as an externally supplied final second-order flow amplitude
  - the previous intermediate implementation exposed `rho2`, which made the public knob read like the amplitude itself
  - V1a already separates initial participant geometry from freeze-out geometry diagnostics
- Decision:
  - replace public CLI/config `rho2` with `kappa2`
  - define the event-wise second-order rapidity modulation amplitude as `a2 = kappa2 * participantGeometry.eps2`
  - define the V1a second-order response plane as `participantGeometry.psi2`
  - keep `events.eps2_f`, `events.psi2_f`, and `events.chi2` as freeze-out diagnostics rather than flow-response inputs
  - make legacy `rho2` fail fast with migration guidance to `kappa2`
- Consequences:
  - docs, help text, configs, and tests must use `kappa2` for current flow tuning
  - historical references to `rho2` may remain only in explicitly historical planning records
  - future higher-harmonic extensions should follow the response-coefficient pattern rather than exposing harmonic amplitudes directly

## DEC-009 Add Opt-In V2 Gradient-Response Medium And Flow

- Status: accepted
- Date: 2026-04-28
- Context:
  - `docs/演化V2_梯度演化计划.md` defines the next evolution step as a coupled gradient response where participant-density gradients drive both transverse source displacement and transverse flow velocity
  - V1a must remain the default so existing runs and validation baselines do not silently change
  - the existing `EventMedium`, `EmissionSite`, `DensityFieldModel`, and `FlowFieldModel` boundaries already provide the right extension points
- Decision:
  - add `density-evolution = gradient-response` and `flow-velocity-sampler = gradient-response`
  - require the two `gradient-response` mode selections to be used together
  - define V2 with separate Gaussian point-cloud densities:
    - `s_em` for marker initial positions, using `sqrt(flowDensitySigma^2 + gradientSigmaEm^2)`
    - `s_dyn` for gradients, using `sqrt(flowDensitySigma^2 + gradientSigmaDyn^2)`
  - require `gradientSigmaDyn > gradientSigmaEm`
  - sample `r0` from each participant's `s_em` component, then use `-grad(s_dyn)/(s_dyn + floor)` to determine bounded displacement, optional diffusion, and site transverse velocity
  - keep participant anchors in `source_x/source_y`, write marker positions to `x0/y0`, final emission positions to `x/y`, and write `emission_weight` for optional analysis weighting
  - keep `cooper-frye-weight = none` as the default and expose `mt-cosh` only as an optional analysis-weight proxy
  - add event-level centered source-size diagnostics `r2_0`, `r2_f`, and `r2_ratio`
  - keep `debug-gradient-response` optional and serialize the four V2 density TH2s as one all-or-none payload
- Consequences:
  - V2 can be validated without changing the default V1a physics path
  - QA now rejects current-contract files that lack the new mandatory `r2` branches/histograms or particle `x0/y0/emission_weight`
  - downstream HBT or source-size studies can distinguish participant anchors, marker initial positions, and final emission points
  - future gradient-response variants should make coupling between medium response and velocity source explicit instead of hiding it behind an apparently orthogonal switch

## DEC-010 Make Affine `density-normal` Kappa Compensation Opt-In

- Status: accepted
- Date: 2026-04-28
- Context:
  - the current V1a `affine-gaussian + density-normal` path already derives the flow direction from the shared `emissionDensity` gradient
  - it still multiplied the density-normal strength by the same explicit `kappa2` second-order modulation used by `covariance-ellipse`
  - the user wanted affine density-normal to default to gradient-driven anisotropy, while keeping the old explicit-strength behavior available as an opt-in compatibility switch
- Decision:
  - add `density-normal-kappa-compensation = true|false` to the public CLI/config surface, defaulting to `false`
  - only allow that switch when `density-evolution = affine-gaussian` and `flow-velocity-sampler = density-normal`
  - keep `covariance-ellipse` behavior unchanged
  - define the default affine density-normal strength as `rhoRaw = rho0 * pow(rTilde, flowPower)`
  - when the switch is enabled, restore the existing explicit V1a multiplier:
    - `exp(2 * kappa2 * eps2_initial * cos(2 * (phiB - psi2_initial)))`
- Consequences:
  - affine density-normal validation baselines now split into two cases: default pure-gradient strength and opt-in compensated strength
  - `kappa2` remains the public second-order response coefficient, but affine density-normal only consumes it when explicit compensation is requested
  - sample configs, help text, tests, and durable docs must describe the new switch and reject invalid mode combinations

## DEC-011 Expose Affine V1a Evolution Parameters On The Public Config Surface

- Status: accepted
- Date: 2026-04-28
- Context:
  - the default `affine-gaussian` medium mode already used `lambda_in`, `lambda_out`, and `sigma_evo` internally through `EventMediumParameters`
  - those values were still hardcoded at the generator boundary, which blocked controlled scans and made config-level comparisons harder than necessary
  - the user explicitly asked to tune the affine evolution response from tracked config files
- Decision:
  - add public CLI/config keys `affine-lambda-in`, `affine-lambda-out`, and `affine-sigma-evo`
  - store the values directly on `BlastWaveConfig` and pass them through `buildMedium()` into `buildEventMedium()`
  - keep the current V1a defaults unchanged:
    - `affine-lambda-in = 1.20`
    - `affine-lambda-out = 1.05`
    - `affine-sigma-evo = 0.5 fm`
  - validate `affine-lambda-in > 0`, `affine-lambda-out > 0`, and `affine-sigma-evo >= 0`
- Consequences:
  - V1a parameter scans can now be expressed entirely through the public runtime surface without editing source
  - sample configs, help text, RunOptions regression coverage, and durable docs must list the three knobs
  - historical notes may still mention the old defaults, but current high-authority docs must describe them as defaults rather than fixed internals

## DEC-012 Add Opt-In Affine-Effective Closure Flow Sampling

- Status: accepted
- Date: 2026-04-29
- Context:
  - `docs/affine方案流补偿.md` defines a closure layer that maps the `affine-gaussian` initial and freeze-out geometry into an effective local flow field
  - the repository already had `EventMedium` and `FlowFieldModel` as the intended medium-to-flow extension boundary
  - the default V1a runtime behavior had to remain unchanged while exposing the closure path for controlled opt-in scans
- Decision:
  - add `flow-velocity-sampler = affine-effective` as a new opt-in transverse-flow source
  - only allow `affine-effective` with `density-evolution = affine-gaussian`
  - store the event-level closure diagnostics in a dedicated `EventMedium::affineEffectiveClosure` block instead of overloading `FlowEllipseInfo`
  - expose public CLI/config knobs:
    - `affine-delta-tau-ref`
    - `affine-kappa-flow`
    - `affine-kappa-aniso`
    - `affine-u-max`
  - keep `rho0`, `kappa2`, and `density-normal-kappa-compensation` public for other samplers, but intentionally unused by `affine-effective`
  - extend optional `debug-flow-ellipse` serialization and independent QA so affine closure diagnostics are validated only when present
  - keep `shell_weight` and `EmissionSite::emissionWeight` restructuring out of scope for this first affine-effective rollout
- Consequences:
  - the default `affine-gaussian + covariance-ellipse` runtime stays stable while affine-effective scans become reproducible from config files
  - future closure-driven flow variants should continue to use the `EventMedium -> FlowFieldModel` boundary rather than smuggling affine diagnostics through unrelated geometry structs
  - any later weighting or surface-sampling redesign must be documented as a separate contract change instead of being implied by the current closure-only sampler
