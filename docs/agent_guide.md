# Blast-Wave Project Agent Guide

## Purpose

This repository contains a small C++17 + ROOT blast-wave event generator for fixed-impact-parameter heavy-ion events. The current v1 target is a single direct charged pion species with event-by-event participant fluctuations from a Monte Carlo Glauber geometry.

The code is intentionally split into:

- a ROOT-free physics core in `include/` and `src/`
- a ROOT-linked generation driver in `apps/generate_blastwave_events.cpp`
- a ROOT-linked QA reader in `apps/qa_blastwave_output.cpp`

## Current Scope

- System: Pb-Pb-like fixed-`b` collisions
- Species: one direct charged pion species (`pid = 211`, `charge = +1`, `mass = 0.13957 GeV`)
- Geometry: participant Monte Carlo Glauber
- Flow: local blast-wave velocity field plus proper Lorentz boost
- Output: ROOT trees plus embedded QA histograms
- Build: CMake

Not implemented in v1:

- minimum-bias `b` sampling
- centrality calibration
- resonance decay
- hadronic rescattering
- multi-species hadron chemistry
- full Cooper-Frye hypersurface sampling

## Repository Layout

- `include/blastwave/BlastWaveGenerator.h`
  Public physics data structures and the generator class.
- `src/BlastWaveGenerator.cpp`
  Event orchestration that ties geometry, sampling, and validation together.
- `src/BlastWaveGeneratorGeometry.cpp`
  Participant sampling and event-shape extraction.
- `src/BlastWaveGeneratorSampling.cpp`
  Multiplicity, source, thermal momentum, flow, and boost sampling.
- `src/BlastWaveGeneratorValidation.cpp`
  Generator-side validation and config checks.
- `include/blastwave/FlowFieldModel.h` and `src/FlowFieldModel.cpp`
  ROOT-free participant covariance ellipse and ellipse-normal flow evaluation.
- `include/blastwave/PhysicsUtils.h` and `src/PhysicsUtils.cpp`
  Shared helpers for azimuth, pseudorapidity, `centrality`, and event-level `v2`.
- `apps/generate_blastwave_events.cpp`
  Top-level CLI driver that hands parsing and ROOT writing to app-layer helpers.
- `apps/generate_blastwave/RunOptions.cpp`
  CLI/config parsing plus progress behavior.
- `apps/generate_blastwave/RootEventFileWriter.cpp`
  ROOT tree and embedded-QA writing.
- `apps/qa_blastwave_output.cpp`
  Independent reader that validates the output contract and rewrites QA histograms.
- `CMakeLists.txt`
  Main build definition.
- `CMakePresets.json`
  Default configure/build preset with `compile_commands.json` enabled.
- `config/`
  Tracked example configuration files only.
- `scripts/`
  Local helper launchers that wrap common run commands.
- `docs/`
  Planning and handoff documents.
- `reference/legacy-root-macros/`
  Historical reference macros kept for comparison only. Do not copy code from here.
- `qa/`
  Local generated sample files and QA output files.
- `project-state/`
  Repository-local engineering coordination ledger.

## Build And Run

This project expects ROOT from the local O2Physics environment on this machine.

From the repository root:

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"
```

Build products:

- executable: `bin/generate_blastwave_events`
- executable: `bin/qa_blastwave_output`
- executable: `bin/test_maxwell_juttner_sampler`
- executable: `bin/test_flow_field_model`
- executable: `bin/test_physics_utils`
- executable: `bin/test_output_path_utils`
- compilation database: `build/compile_commands.json`

ROOT-free core regression test:

```bash
cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure
```

Example generation:

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 5000 --b 8 --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_5000.root'"
```

Example generation from a config file:

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_override.root'"
```

The repository ships an example config at `config/test_b8.cfg`.

Example validation:

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_5000.root --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_5000_validation.root --expect-nevents 5000'"
```

## CLI Contract

`generate_blastwave_events` supports three equivalent entry styles:

- `generate_blastwave_events [options]`
- `generate_blastwave_events --config <path> [options]`
- `generate_blastwave_events <config-path> [options]`

Explicit CLI options override configuration file values. Configuration file
values override the built-in `BlastWaveConfig` defaults.

Configuration files use a lightweight `key = value` format:

- blank lines are ignored
- full-line `#` comments are ignored
- supported keys are:
  - `nevents`
  - `b`
  - `temperature`
  - `thermal-sampler`
  - `mj-pmax`
  - `mj-grid-points`
  - `tau0`
  - `smear`
  - `sigma-nn`
  - `seed`
  - `output`
  - `progress`
  - `rho0`
  - `rho2`
  - `flow-power`
  - `debug-flow-ellipse`
  - `sigma-eta`
  - `eta-plateau`
  - `nbd-mu`
  - `nbd-k`
- relative `output` paths inside the config file are resolved relative to the
  config file directory

Minimal config example:

```text
# config/test_b8.cfg
nevents = 5000
b = 8
temperature = 0.2
thermal-sampler = maxwell-juttner
progress = true
# mj-pmax = 8.0
# mj-grid-points = 4096
tau0 = 10.0
smear = 0.5
output = test_b8_from_config.root
```

The same parameters remain available as explicit CLI flags:

- `--nevents <int>`: number of events
- `--b <fm>`: impact parameter
- `--temperature <GeV>`: thermal momentum scale
- `--thermal-sampler <maxwell-juttner|gamma>`: local-rest-frame momentum-magnitude sampler
- `--mj-pmax <GeV>`: upper momentum cutoff of the Maxwell-Juttner lookup table
- `--mj-grid-points <int>`: grid size of the Maxwell-Juttner lookup table
- `--tau0 <fm/c>`: fixed proper time
- `--smear <fm>`: transverse Gaussian hotspot smearing
- `--sigma-nn <fm^2>`: inelastic nucleon-nucleon cross section, default `7.0`
- `--seed <uint64>`: RNG seed
- `--output <path>`: ROOT output path
- `--progress`: force-enable the progress bar
- `--no-progress`: force-disable the progress bar
- `--rho0 <value>`: isotropic rapidity baseline of the covariance-ellipse flow field
- `--rho2 <value>`: second-order rapidity amplitude multiplied by event `eps2`
- `--flow-power <value>`: power of the normalized ellipse radius `rTilde`
- `--debug-flow-ellipse`: write `flow_ellipse_debug` and `flow_ellipse_participant_norm_x-y`
- `--no-debug-flow-ellipse`: force-disable the optional debug payload
- `--sigma-eta <value>`: Gaussian tail width of source `eta_s`
- `--eta-plateau <value>`: flat core half width of source `eta_s`
- `--nbd-mu <value>`: mean multiplicity parameter per hotspot
- `--nbd-k <value>`: NBD shape parameter

Deprecated flow knobs now fail fast with migration guidance:

- `vmax -> rho0 = atanh(vmax)`
- `kappa2 -> re-tuned rho2`
- `r-ref -> absorbed by event-ellipse semi-axes`

If `progress` is not set in the config file and neither CLI flag is present, the
generator only shows the progress bar when `stderr` is attached to a TTY.

`qa_blastwave_output` supports:

- `--input <file.root>`
- `--output <qa.root>`
- `--expect-nevents <int>`

## Public C++ Interface

Namespace: `blastwave`

Key data contracts:

- `BlastWaveConfig`
  Generator configuration and defaults.
- `EventInfo`
  Per-event summary:
  - `eventId`
  - `impactParameter`
  - `nParticipants`
  - `eps2`
  - `psi2`
  - `v2`
  - `centrality`
  - `nCharged`
- `ParticleRecord`
  Per-particle payload:
  - event mapping
  - particle identity
  - emission coordinates
  - final four-momentum
  - source-space metadata
- `GeneratedEvent`
  Bundles one `EventInfo`, one `FlowEllipseInfo`, participant records, and particle records.
- `BlastWaveGenerator`
  Main class:
  - constructor `explicit BlastWaveGenerator(BlastWaveConfig config)`
  - method `GeneratedEvent generateEvent(int eventId)`

Important implementation note:

- The core physics layer is ROOT-independent.
- ROOT types are used only in the app-level I/O wrappers.
- ROOT tree floating branches are written as `Double_t`, not `Float_t`, because `Float_t` caused visible mass-shell drift in high-`|eta_s|` tails.

## ROOT Output Contract

One generated ROOT file contains three mandatory trees, mandatory QA histograms,
and one optional debug payload.

### Tree `events`

Branches:

- `event_id`
- `b`
- `Npart`
- `eps2`
- `psi2`
- `v2`
- `centrality`
- `Nch`

### Tree `participants`

Branches:

- `event_id`
- `nucleus_id`
- `x`
- `y`
- `z`

### Tree `particles`

Branches:

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

### Embedded Histograms

- `Npart`
- `eps2`
- `psi2`
- `v2`
- `cent`
- `participant_x-y`
- `x-y`
- `px-py`
- `pT`
- `eta`
- `phi`

### Embedded Visual Object

- `participant_x-y_canvas`
  A ROOT canvas that draws `participant_x-y` and overlays circle outlines for nucleus A and nucleus B using the configured impact parameter and Woods-Saxon radius.

The QA executable expects all of these objects to exist.

When `debug-flow-ellipse` is enabled, the file also contains:

- `flow_ellipse_debug`
  Per-event debug tree with centroid, covariance, eigenvalues, semi-axes, axes, `eps2`, `psi2`, and `valid`
- `flow_ellipse_participant_norm_x-y`
  Aggregate `TH2` of participant coordinates after centroid shift, principal-axis projection, and semi-axis normalization

When `flow-velocity-sampler = density-normal`, the file also contains:

- `density_normal_event_density_x-y`
  Single-event smeared participant-density `TH2` captured from the first event with participants and stored with the default draw option `LEGO1` so ROOT opens it as a 3D height-style plot

The QA policy for these optional objects is “validate if present, ignore if absent.”

## Physics And Algorithm Summary

### 1. Geometry

- Sample `A = nucleonsPerNucleus` nucleons independently in 3D using a Woods-Saxon density.
- Shift the two nuclei by `x = ±b/2`.
- Tag two nucleons as participants if their transverse distance satisfies:
  - `dx^2 + dy^2 <= sigmaNN / pi`
- Return the union of participant nucleons from both nuclei.

### 2. Participant Shape

- Compute the participant transverse centroid.
- Build centered second moments:
  - `sigmaX2`
  - `sigmaY2`
  - `sigmaXY`
- Define the participant eccentricity vector:
  - `eccentricityX = sigmaY2 - sigmaX2`
  - `eccentricityY = 2 * sigmaXY`
- Extract:
  - `eps2 = sqrt(ex^2 + ey^2) / (sigmaX2 + sigmaY2)`
  - `psi2 = 0.5 * atan2(ey, ex)`

Interpretation:

- `eps2` measures how elliptic the participant distribution is.
- `psi2` is the second-order participant-plane angle.

### 3. Multiplicity Per Participant

- Each participant acts as a hotspot.
- The number of emitted particles from one hotspot is sampled with a Gamma-Poisson mixture:
  - Gamma draw for `lambda`
  - Poisson draw for multiplicity
- This is an NBD-like fluctuation model controlled by `nbdMu` and `nbdK`.

### 4. Emission Coordinates

- The hotspot position is transversely smeared with a Gaussian of width `smearSigma`.
- Emission proper time is fixed:
  - `t = tau0 * cosh(eta_s)`
  - `z = tau0 * sinh(eta_s)`

### 5. Longitudinal Source Profile

Current implementation is not pure Gaussian anymore.

- If `etaPlateauHalfWidth > 0`, the source uses a flat midrapidity core.
- Outside the core, it attaches Gaussian tails with width `sigmaEta`.
- If `etaPlateauHalfWidth = 0`, the profile reduces to a pure Gaussian.

This change was introduced because the original pure Gaussian source produced a visibly over-peaked midrapidity `eta` distribution for the intended use.

### 6. Thermal Momentum In The Local Rest Frame

- Default mode: pretabulate a Maxwell-Juttner momentum CDF on a uniform `p` grid over `[0, mjPMax]` using
  `w(p) = p^2 exp(-sqrt(p^2 + m^2) / T)`, then invert it with `lower_bound` plus linear interpolation.
- Compatibility mode: keep the legacy Gamma sampler with shape `3` and scale `T`.
- After the magnitude is chosen, sample direction isotropically and put the particle on shell with the configured mass.
- If `temperature <= 0`, the generator returns zero three-momentum and `E = m` without building the Maxwell-Juttner table.

This keeps the core ROOT-free while upgrading the default thermal spectrum beyond the earlier Gamma-only approximation.

### 7. Flow Field

- Build the participant covariance ellipse from the event centroid and second moments.
- Diagonalize it analytically to recover `lambdaMajor`, `lambdaMinor`, `radiusMajor`, `radiusMinor`, and a deterministic right-handed principal-axis basis.
- For each emission point:
  - shift by the event centroid
  - project onto the principal-axis basis
  - evaluate `rTilde = sqrt(x'^2 / radiusMajor^2 + y'^2 / radiusMinor^2)`
  - evaluate the ellipse-normal angle `phiB = atan2(y' / radiusMinor^2, x' / radiusMajor^2)`
  - evaluate `rhoRaw = pow(rTilde, flowPower) * (rho0 + rho2 * eps2 * cos(2 * phiB))`
  - convert to `betaT = tanh(max(0, rhoRaw))`, clipped to `0.95`
- Combine the transverse ellipse-normal flow with `eta_s` in a Bjorken-like flow field.

### 8. Proper Boost

- Sample momentum in the local rest frame.
- Apply a full Lorentz boost with the local `beta`.
- Do not add flow as a direct `px`/`py` offset. That old approach is explicitly avoided.

### 9. Validation

The core validates:

- all particle fields are finite
- the flow field is not superluminal
- optional flow-ellipse debug outputs, when present, satisfy the recorded covariance and orthonormal-axis invariants
- on-shell relation holds within a tight internal tolerance

The QA executable additionally validates:

- required trees exist
- required histograms exist
- event IDs are continuous
- `Nch` matches the particle count per event
- `events.v2` matches the particle-level second-harmonic Q-vector
- the `v2` histogram entry count and mean match the `events.v2` payload
- `centrality` stays within `[0, 100]` and matches the fixed-`b` mapping
- tree values contain no `NaN` or `Inf`
- mass-shell deviation stays below `1e-4 GeV^2`
- optional flow-ellipse debug objects, when present, match the recorded entry-count
  and covariance-shape invariants
- optional `density_normal_event_density_x-y`, when present, is a finite non-negative `TH2`
  and advertises a 3D draw option such as `LEGO` or `SURF`

## Known Behavior And Interpretation Notes

- The file now stores a `participants` tree, so downstream code can inspect participant geometry directly instead of inferring it from `source_x/source_y` in the particle tree.
- `eta` is controlled by the source `eta_s` profile, but the final particle `eta` is not expected to match experimental charged-hadron data exactly because the model has only one direct pion species and no resonance feed-down.
- A raw inclusive `px-py` histogram is a weak anisotropy diagnostic because low-`pT` thermal particles dominate the density. Use `phi`, `v2`, or high-`pT` cuts if directional anisotropy needs to be diagnosed more sharply.
- `eps2` is an initial-state participant-shape observable, while `v2` is a final-state event observable reconstructed from particle azimuths. They are related but not interchangeable.
- `Nch` in v1 means the number of generated particles in the event, not a realistic detector-level charged multiplicity.
- Events with `Npart = 0` or `Npart = 1` are legal and remain in the `events` tree.

## Change-Sensitive Areas

If you modify the project, preserve these contracts unless intentionally versioning them:

- tree names: `events`, `particles`
- participant tree name: `participants`
- branch names in both trees
- embedded histogram and canvas names listed above
- QA expectation that all objects exist
- use of `Double_t` for tree floating fields

## Recommended Next Extensions

- add species support and resonance decay
- add better anisotropy observables such as `phi - psi2` and `v2(pT)`
- add adaptive Maxwell-Juttner table sizing or a more formal source-function / Cooper-Frye sampling
- support minimum-bias impact-parameter sampling and centrality slicing
