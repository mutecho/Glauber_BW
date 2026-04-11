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
  Core geometry, source sampling, flow, boost, and validation logic.
- `apps/generate_blastwave_events.cpp`
  CLI driver that generates events and writes the ROOT file.
- `apps/qa_blastwave_output.cpp`
  Independent reader that validates the output contract and rewrites QA histograms.
- `CMakeLists.txt`
  Main build definition.
- `CMakePresets.json`
  Default configure/build preset with `compile_commands.json` enabled.
- `docs/`
  Planning and handoff documents.
- `qa/`
  Local generated sample files and QA output files.
- `qiye/`
  Reference macros kept for comparison only. Do not copy code from here.

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
- compilation database: `build/compile_commands.json`

Example generation:

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 5000 --b 8 --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_5000.root'"
```

Example generation from a config file:

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/qa/test_b8.cfg --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_override.root'"
```

The repository ships an example config at `qa/test_b8.cfg`.

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
  - `tau0`
  - `smear`
  - `sigma-nn`
  - `seed`
  - `output`
  - `vmax`
  - `kappa2`
  - `sigma-eta`
  - `eta-plateau`
  - `nbd-mu`
  - `nbd-k`
  - `r-ref`
- relative `output` paths inside the config file are resolved relative to the
  config file directory

Minimal config example:

```text
# qa/test_b8.cfg
nevents = 5000
b = 8
temperature = 0.2
tau0 = 10.0
smear = 0.5
output = test_b8_from_config.root
```

The same parameters remain available as explicit CLI flags:

- `--nevents <int>`: number of events
- `--b <fm>`: impact parameter
- `--temperature <GeV>`: thermal momentum scale
- `--tau0 <fm/c>`: fixed proper time
- `--smear <fm>`: transverse Gaussian hotspot smearing
- `--sigma-nn <fm^2>`: inelastic nucleon-nucleon cross section, default `7.0`
- `--seed <uint64>`: RNG seed
- `--output <path>`: ROOT output path
- `--vmax <value>`: maximum transverse flow scale
- `--kappa2 <value>`: elliptic flow response coefficient
- `--sigma-eta <value>`: Gaussian tail width of source `eta_s`
- `--eta-plateau <value>`: flat core half width of source `eta_s`
- `--nbd-mu <value>`: mean multiplicity parameter per hotspot
- `--nbd-k <value>`: NBD shape parameter
- `--r-ref <fm>`: reference transverse size entering the flow profile

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
  - `nCharged`
- `ParticleRecord`
  Per-particle payload:
  - event mapping
  - particle identity
  - emission coordinates
  - final four-momentum
  - source-space metadata
- `GeneratedEvent`
  Bundles one `EventInfo` plus a `std::vector<ParticleRecord>`.
- `BlastWaveGenerator`
  Main class:
  - constructor `explicit BlastWaveGenerator(BlastWaveConfig config)`
  - method `GeneratedEvent generateEvent(int eventId)`

Important implementation note:

- The core physics layer is ROOT-independent.
- ROOT types are used only in the app-level I/O wrappers.
- ROOT tree floating branches are written as `Double_t`, not `Float_t`, because `Float_t` caused visible mass-shell drift in high-`|eta_s|` tails.

## ROOT Output Contract

One generated ROOT file contains three trees plus QA histograms.

### Tree `events`

Branches:

- `event_id`
- `b`
- `Npart`
- `eps2`
- `psi2`
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

- Sample the momentum magnitude from a Gamma distribution with shape `3` and scale `T`.
- Sample direction isotropically.
- Put the particle on shell with the configured mass.

This is a lightweight thermal-like sampler, not a full Cooper-Frye implementation.

### 7. Flow Field

- Compute transverse radius `r` and azimuth `phi` from the emission point.
- Build an elliptically modulated radial profile:
  - `modulation = 1 + 2 * kappa2 * eps2 * cos(2 * (phi - psi2))`
  - `profile = clamp((r / referenceRadius) * modulation, 0, 1)`
  - `vT = clamp(vMax * profile, 0, 0.95)`
- Convert to transverse rapidity and combine with `eta_s` in a Bjorken-like flow field.
- Convert the flow four-velocity to a three-velocity `beta`.

### 8. Proper Boost

- Sample momentum in the local rest frame.
- Apply a full Lorentz boost with the local `beta`.
- Do not add flow as a direct `px`/`py` offset. That old approach is explicitly avoided.

### 9. Validation

The core validates:

- all particle fields are finite
- the flow field is not superluminal
- on-shell relation holds within a tight internal tolerance

The QA executable additionally validates:

- required trees exist
- required histograms exist
- event IDs are continuous
- `Nch` matches the particle count per event
- tree values contain no `NaN` or `Inf`
- mass-shell deviation stays below `1e-4 GeV^2`

## Known Behavior And Interpretation Notes

- The file now stores a `participants` tree, so downstream code can inspect participant geometry directly instead of inferring it from `source_x/source_y` in the particle tree.
- `eta` is controlled by the source `eta_s` profile, but the final particle `eta` is not expected to match experimental charged-hadron data exactly because the model has only one direct pion species and no resonance feed-down.
- A raw inclusive `px-py` histogram is a weak anisotropy diagnostic because low-`pT` thermal particles dominate the density. Use `phi`, `v2`, or high-`pT` cuts if directional anisotropy needs to be diagnosed more sharply.
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
- replace the simplified thermal sampler with a more formal source-function or Cooper-Frye sampling
- support minimum-bias impact-parameter sampling and centrality slicing
