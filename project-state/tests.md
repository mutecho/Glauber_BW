# Tests

## Durable Verification Baseline

- Status: verified
- Last comprehensive refresh: 2026-04-29
- Evidence shape:
  - local rebuild of touched binaries and regression targets passed
  - local `ctest` passed
  - authoritative outside-sandbox ROOT generate+QA smokes passed
  - standalone differential analysis passed where relevant
  - ROOT key inspection was used only when output placement semantics mattered

This file now keeps durable conclusions only.
Long command transcripts and repeated smoke-command variants were intentionally removed during documentation compaction.

## Command Families Still Used

- `cmake --build ...`
- `cd build && ctest --output-on-failure`
- authoritative outside-sandbox `generate_blastwave_events ...`
- authoritative outside-sandbox `qa_blastwave_output ...`
- authoritative outside-sandbox `analyze_blastwave_v2pt ...`
- ROOT key inspection through the shared inspector scripts when payload placement needs confirmation

## T-017 Opt-In Affine-Effective Closure Flow Sampler

- Status: passed on 2026-04-29
- Evidence:
  - local build passed after extending `EventMedium`, `FlowFieldModel`, generator validation, ROOT debug writing, and QA
  - local `ctest` passed with new affine-effective coverage in `test_flow_field_model` and `test_run_options`
  - authoritative default V1a generate + QA smoke passed
  - authoritative `config/test_b8_affine_effective.cfg` generate + QA smoke passed
- Locked conclusions:
  - `flow-velocity-sampler = affine-effective` is opt-in and only valid with `density-evolution = affine-gaussian`
  - `affine-delta-tau-ref`, `affine-kappa-flow`, `affine-kappa-aniso`, and `affine-u-max` are public validated knobs
  - `debug-flow-ellipse` may now carry affine closure diagnostics, and QA validates them when present
  - `rho0`, `kappa2`, and `density-normal-kappa-compensation` remain public but are intentionally unused by affine-effective

## T-016 Differential `v2{2}(pT)` Joint And Standalone Contract

- Status: passed on 2026-04-29
- Evidence:
  - local `ctest` passed with the differential cumulant coverage in tree
  - authoritative same-file generate + QA passed
  - authoritative separate-file generate passed and QA accepted the metadata-only main file
  - standalone analysis passed in both separate-file and `--inplace` modes
  - ROOT key inspection confirmed the dedicated analysis file contains only `v2_2_pt_edges`, `v2_2_pt`, and `v2_2_pt_canvas`
- Locked conclusions:
  - configuring `v2pt-bins` always writes `v2_2_pt_edges`
  - `v2pt-output-mode = separate-file` may leave the main result file metadata-only
  - differential `v2{2}(pT)` is separate from `events.v2` and currently uses unit track weights only

## T-015 Freeze-Out Eccentricity Response TH1 Contract

- Status: passed on 2026-04-29
- Evidence:
  - authoritative generate + QA smoke passed
  - ROOT key inspection confirmed `chi2` is present and `chi2_canvas` is not required
- Locked conclusion:
  - the default contract keeps the `chi2` histogram only; there is no second mandatory canvas object for this diagnostic

## T-014 Public Affine V1a Evolution Knob Surface

- Status: passed on 2026-04-28
- Evidence:
  - `test_run_options` coverage passed
  - full local `ctest` passed after exposing the knobs publicly
- Locked conclusion:
  - `affine-lambda-in`, `affine-lambda-out`, and `affine-sigma-evo` are public validated knobs with unchanged defaults

## T-013 Affine Density-Normal Compensation Contract

- Status: passed on 2026-04-28
- Evidence:
  - parser and regression coverage passed
  - authoritative default-off, opt-in-on, and invalid-combination smokes passed
- Locked conclusion:
  - `density-normal-kappa-compensation` is opt-in and only valid for `affine-gaussian + density-normal`

## T-012 V2 Gradient-Response Medium And Flow

- Status: passed on 2026-04-28
- Evidence:
  - local build and `ctest` passed
  - authoritative default V1a control smoke passed
  - authoritative V2 gradient, V2 debug, and zero-event debug checks passed
- Locked conclusions:
  - `density-evolution = gradient-response` and `flow-velocity-sampler = gradient-response` must be selected together
  - `r2_0/r2_f/r2_ratio`, `x0/y0`, and `emission_weight` are part of the current contract

## T-011 `kappa2` Public Flow Response Contract

- Status: passed on 2026-04-28
- Evidence:
  - parser/test coverage passed
  - authoritative smoke validation passed after the public naming switch
- Locked conclusions:
  - `kappa2` replaced `rho2`
  - the V1a second-order response uses initial `eps2/psi2`, not freeze-out geometry diagnostics

## T-010 V1a Affine Gaussian Density Response

- Status: passed on 2026-04-28
- Evidence:
  - local build and `ctest` passed
  - authoritative default V1a smoke passed
  - authoritative `density-evolution = none` comparison smoke passed
  - authoritative V1a `density-normal` smoke passed
- Locked conclusions:
  - `affine-gaussian` is the default medium mode
  - `none` remains the legacy comparison path
  - freeze-out geometry diagnostics are mandatory in the current schema

## Earlier Checks Still Relied On

- T-001 Baseline build and smoke
  - configure, build, generate, and QA path established
- T-002 Participant output contract
  - `participants` tree plus participant visualization payload remain part of the current schema
- T-003 Config-file CLI contract
  - `--config` and positional config entry both remain public; relative outputs resolve against the config directory
- T-004 Thermal sampler switch
  - `maxwell-juttner` remains the default and `gamma` remains the explicit compatibility mode
- T-005 Covariance-ellipse default flow replacement
  - covariance-ellipse remains the default transverse-flow source
- T-006 Event-level `v2` output contract
  - `events.v2` and the `v2` summary histogram remain validated through shared observable helpers
- T-007 Example launcher ROOT alignment
  - launcher ROOT preflight and explicit `ROOT::HistPainter` link remain intentional
- T-008 Fluid-element velocity sampler surface
  - `flow-velocity-sampler` and `flow-density-sigma` remain the public abstraction
- T-009 `EventMedium` and `EmissionSite` interface refactor
  - the internal medium/emission split remains the intended extension boundary
- T-011 `kappa2` public flow response contract
  - covariance-style V1a flow still uses initial `eps2/psi2`

## Environment Note

- On this machine, sandboxed `alienv` ROOT runs are not authoritative when PCM / module noise appears.
- Durable ROOT evidence should continue to come from the outside-sandbox O2Physics path.
