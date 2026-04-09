# Blast-Wave Generator Agent Handoff

## Stage Completed
- Requirement analysis of the existing plan in [/Users/allenzhou/Research_software/Blast_wave/docs/C++ Blast-Wave Event Generator Plan.md](/Users/allenzhou/Research_software/Blast_wave/docs/C++%20Blast-Wave%20Event%20Generator%20Plan.md).

## Source Evidence
- Primary requirement artifact: [/Users/allenzhou/Research_software/Blast_wave/docs/C++ Blast-Wave Event Generator Plan.md](/Users/allenzhou/Research_software/Blast_wave/docs/C++%20Blast-Wave%20Event%20Generator%20Plan.md)
- Reference-only implementation context:
  - [/Users/allenzhou/Research_software/Blast_wave/qiye/toy_glauber.C](/Users/allenzhou/Research_software/Blast_wave/qiye/toy_glauber.C)
  - [/Users/allenzhou/Research_software/Blast_wave/qiye/toy_glauber2.C](/Users/allenzhou/Research_software/Blast_wave/qiye/toy_glauber2.C)
  - [/Users/allenzhou/Research_software/Blast_wave/qiye/toy_glauber_flow.C](/Users/allenzhou/Research_software/Blast_wave/qiye/toy_glauber_flow.C)
  - [/Users/allenzhou/Research_software/Blast_wave/qiye/analyze_toy3D.C](/Users/allenzhou/Research_software/Blast_wave/qiye/analyze_toy3D.C)

## Assessment Summary
- The plan is directionally correct and already has a good scope boundary: independent C++ generator core, fixed-`b` first version, ROOT as I/O plus QA only, and no femto/pair-analysis scope.
- The plan is not yet fully downstream-ready for an implementation agent because several interfaces and physics defaults are still implicit.
- The most important gaps are:
  - CLI/config inconsistency: the plan says `sigmaNN` must be configurable, but the CLI list does not include `--sigma-nn`.
  - Testability gap: the dynamic smoke tests require changing anisotropy and radial-flow strength, but the public interface does not currently expose those controls.
  - Event semantics gap: the plan does not define how to handle zero-participant events, `Npart < 2`, or how `Nch` should be counted in those cases.
  - Physics-default gap: units and default values for `sigmaNN`, Woods-Saxon parameters, multiplicity model, longitudinal width, and transverse flow profile are not pinned down in the plan itself.
  - Delivery-structure gap: the repository currently contains only docs and reference macros, so the implementation agent still needs an explicit deliverable shape for source layout and build entrypoints.
- With the assumptions and defaults below, the task is specific enough for bounded implementation.

## Problem Definition
Implement a new first-version Blast-Wave event generator as independent C++ code in this repository. The generator must produce per-event and per-particle ROOT output suitable for downstream QA and later femto analysis, without copying code from the `qiye` reference macros and without reproducing their incorrect momentum-shift flow treatment.

## Goals
- Build an independent generator core that does not depend on ROOT headers for its internal physics logic.
- Support first-version fixed impact-parameter event generation with default `b = 8 fm`.
- Preserve event-by-event participant fluctuations from a 2D Monte Carlo Glauber geometry.
- Generate particle emission from participant hotspots with a simple blast-wave model using local four-velocity plus proper Lorentz boost.
- Write a stable ROOT output contract with `events` and `particles` trees plus a small QA payload.
- Provide a single executable entrypoint for command-line production runs.

## Non-Goals
- No minimum-bias impact-parameter sampling.
- No centrality calibration or centrality selection.
- No resonance decay.
- No hadronic rescattering.
- No femtoscopy function construction or pair analysis.
- No reuse of `qiye` code by copy-paste or direct linking.
- No external YAML/JSON configuration in v1.

## Confirmed Facts From The Requirement
- The target implementation must be independent from the `qiye` directory and only inherit its physical intent and lessons learned.
- The required core data contracts are `BlastWaveConfig`, `EventInfo`, `ParticleRecord`, and a `generateEvent(const BlastWaveConfig&)`-style API.
- The geometry stage must sample two nuclei, determine participants with `sigmaNN`, and compute participant-plane quantities `eps2` and `psi2`.
- The emission stage must use participant hotspots, Gaussian transverse smearing, fixed `tau0`, Gaussian `eta_s`, and proper boost-based flow treatment.
- The ROOT output must expose:
  - `events(event_id, b, Npart, eps2, psi2, Nch)`
  - `particles(event_id, pid, charge, mass, x, y, z, t, px, py, pz, E, eta_s, source_x, source_y)`
- The CLI must at least support event count, `b`, temperature, `tau0`, smearing, seed, and output path.
- The file output is the formal downstream interface for later analysis stages.

## Working Assumptions Adopted To Make The Task Executable
- System target for v1 is Pb-Pb only.
- Internal computation may use `double`; ROOT tree branches should use stable scalar types:
  - integer-like fields as `Int_t`
  - floating-point physics fields as `Float_t`
- `Nch` means "number of generated particles written for the event" in v1. Because v1 only generates one charged pion species, `Nch` is identical to the particle count for that event.
- Zero-participant events are retained in the `events` tree with:
  - `Npart = 0`
  - `eps2 = 0`
  - `psi2 = 0`
  - `Nch = 0`
  - no entries in the `particles` tree
- Events with `Npart = 1` are also retained. In that case `eps2 = 0` and `psi2 = 0`.
- The implementation should not silently resample or discard such events, because the plan explicitly expects continuous event IDs and a stable event/particle mapping.
- Default physics constants may be taken from the reference intent unless the user later overrides them:
  - `A = 208`
  - Woods-Saxon `R0 = 6.62 fm`
  - Woods-Saxon diffuseness `a0 = 0.546 fm`
  - `sigmaNN = 7.0 fm^2` as the internal default
  - CLI help should note that `7.0 fm^2` is equivalent to `70 mb`
  - default `T = 0.2 GeV`
  - default `tau0 = 10.0 fm/c`
  - default `smearSigma = 0.5 fm`
  - default longitudinal width `sigmaEta = 1.5`
  - default multiplicity model per participant hotspot may use NBD with `mu = 2.0` and `k = 1.5`
  - default radial-flow shape may use a bounded profile with `v_max = 0.8`, reference radius `R_ref = 6.0 fm`, and elliptic modulation strength `kappa2 = 0.5`
- The exact local-rest-frame thermal sampler is an implementation choice, provided it preserves on-shell kinematics and the acceptance criteria below.

## Constraints
- Do not copy functions or files from `qiye`.
- ROOT must remain an output and QA dependency, not a hard requirement for the generator-core physics classes.
- The executable must remain simple enough to run as a standard ROOT-linked C++ program.
- The implementation must avoid the old incorrect pattern of adding a flow offset directly to `px` and `py`.
- The first delivery must keep the scope bounded to single-species direct pion production.

## Dependencies
- C++17 or newer.
- ROOT available for executable linking, tree writing, histogram writing, and QA macro execution.
- No additional external configuration or physics libraries are required for v1.

## Recommended Deliverable Shape
- Core library surfaces:
  - `BlastWaveConfig`
  - `EventInfo`
  - `ParticleRecord`
  - a generator class or namespace-level API implementing `generateEvent`
- Executable:
  - `generate_blastwave_events`
- QA artifact:
  - one independent ROOT macro or executable that reads the produced ROOT file and fills/plots the requested smoke-test histograms
- Build artifact:
  - if the repository still has no build system, the implementing agent should add the smallest reasonable build entrypoint needed to produce the executable

## Public Interface Contract

### Required CLI Flags
- `--nevents`
- `--b`
- `--temperature`
- `--tau0`
- `--smear`
- `--sigma-nn`
- `--seed`
- `--output`

### Recommended Additional CLI Flags For QA Reproducibility
- `--vmax`
- `--kappa2`
- `--sigma-eta`
- `--nbd-mu`
- `--nbd-k`

These are recommended because the stated dynamic smoke tests cannot be driven from the outside unless anisotropy, radial-flow strength, and multiplicity parameters are externally adjustable.

## Output Contract

### ROOT Trees
- Tree `events` with branches:
  - `event_id`
  - `b`
  - `Npart`
  - `eps2`
  - `psi2`
  - `Nch`
- Tree `particles` with branches:
  - `event_id`
  - `pid`
  - `charge`
  - `mass`
  - `x`
  - `y`
  - `z`
  - `t`
  - `px`
  - `py`
  - `pz`
  - `E`
  - `eta_s`
  - `source_x`
  - `source_y`

### QA Payload
- Include QA histograms in the same ROOT file unless the implementer has a strong reason to separate them:
  - `Npart`
  - `eps2`
  - `x-y`
  - `pT`
  - `eta`
  - `phi`
- Histogram names and branch types should be treated as stable interface in v1.

## Boundary Conditions And Failure Modes
- `Npart = 0` and `Npart = 1` are legal events and must not break output continuity.
- If the participant-centroid tensor is degenerate, set `eps2 = 0` and `psi2 = 0`.
- The flow field must never exceed the speed of light. Clamping to a safe maximum below 1 is acceptable; silently writing superluminal output is not.
- The generator must never write `NaN` or `Inf` into the ROOT trees.
- If invalid kinematics are produced after the boost step, the implementation should fail loudly during development or QA rather than silently emitting corrupted output.

## Risks
- If `sigmaNN` units are not pinned, the implementation can drift by a factor of 10 between `mb` and `fm^2`.
- If zero-participant events are dropped, the `event_id` continuity requirement will be violated.
- If the thermal sampler is left too implicit, different implementers may produce materially different spectra while all claiming to match the plan.
- If ROOT types are not stabilized, downstream QA and later femto code may become brittle.
- If the executable does not expose enough tuning knobs, the promised smoke tests become difficult to reproduce externally.

## Remaining Missing Information
- No blocking missing information remains if the implementation agent accepts the working assumptions above.
- Owner confirmation is still useful if any of the following are intended to differ:
  - `sigmaNN` default or units
  - NBD defaults
  - radial-flow functional form
  - whether optional QA-facing CLI flags should be part of the public interface

## Suggested Acceptance Criteria
- The repository can build a `generate_blastwave_events` executable.
- Running the executable with `--nevents 100 --output test.root` produces a ROOT file containing both `events` and `particles` trees plus the requested QA histograms.
- The `events` tree contains exactly `nevents` entries with continuous `event_id` from `0` to `nevents - 1`.
- For every event, `events.Nch` matches the number of rows in `particles` with the same `event_id`.
- No particle row contains `NaN` or `Inf`.
- Recomputing `E^2 - p_x^2 - p_y^2 - p_z^2 - m^2` from the output tree stays within a practical floating-point tolerance of `1e-4 GeV^2`.
- The geometry smoke trend holds over a moderate sample:
  - smaller `b` produces larger mean `Npart`
  - smaller `b` produces smaller mean `eps2`
- The dynamics smoke trend holds:
  - reducing anisotropy strength toward zero drives the `phi` distribution toward axial symmetry
  - increasing radial-flow strength hardens the `pT` spectrum
- An independent QA reader can open the file and fill/plot `x-y`, `pT`, `eta`, and `phi` without needing access to generator-internal classes.

## Verification Status
- `unverified`

## Next-Step Recommendation
- Hand off to `implement-feature`.
- The implementing agent should treat this document as the execution contract and should avoid reopening scope unless it encounters a concrete build or interface blocker.
