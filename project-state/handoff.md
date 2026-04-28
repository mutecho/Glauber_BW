# Handoff

## Latest Durable Handoff

- Stage completed: V1a fixed-parameter affine density response plus `kappa2` second-order response contract
- Artifact produced:
  - added `DensityEvolutionMode::AffineGaussianResponse` as the default medium mode and kept `None` for comparison
  - evolved `s0` into freeze-out `sf` with fixed `lambda_in = 1.20`, `lambda_out = 1.05`, and `sigma_evo = 0.5 fm`
  - extended `DensityField` to support a shared full 2D Gaussian covariance and analytic gradients
  - added density-field emission for V1a while preserving original participant anchors in `source_x/source_y`
  - kept `flow-velocity-sampler` orthogonal to `density-evolution`
  - replaced public `rho2` with `kappa2` and defined the event-level amplitude as `kappa2 * eps2_initial`
  - tied the V1a second-order response plane to initial `psi2`, while keeping `eps2_f` / `psi2_f` as freeze-out diagnostics
  - added mandatory freeze-out geometry diagnostics `events.eps2_f`, `events.psi2_f`, `events.chi2` and matching QA histograms
  - refreshed higher-authority docs and the `project-state/` ledger
- Current contract snapshot:
  - `events` carries `b`, `Npart`, `eps2`, `psi2`, `eps2_f`, `psi2_f`, `chi2`, `v2`, `centrality`, and `Nch`
  - mandatory QA objects remain `Npart`, `eps2`, `psi2`, `v2`, `cent`, `participant_x-y`, `participant_x-y_canvas`, `x-y`, `px-py`, `pT`, `eta`, and `phi`
  - mandatory QA objects now also include `eps2_f`, `psi2_f`, and `chi2`
  - optional debug output remains `flow_ellipse_debug` plus `flow_ellipse_participant_norm_x-y`, gated by `debug-flow-ellipse`
  - optional sampler-specific output now includes `density_normal_event_density_x-y`, gated by `flow-velocity-sampler = density-normal`
  - the default fluid-element velocity sampler remains `covariance-ellipse`
  - the parallel sampler surface now also accepts `density-normal`
  - public flow/runtime knobs now include `rho0`, `kappa2`, `flow-power`, `flow-velocity-sampler`, `density-evolution`, `flow-density-sigma`, and `debug-flow-ellipse`
  - legacy `vmax`, `rho2`, and `r-ref` still fail fast with explicit migration guidance
  - `density-evolution = affine-gaussian` is the default; `density-evolution = none` preserves the previous identity medium and participant-hotspot emission path
- Remaining durable follow-up:
  - regenerate long-lived sample files under `qa/` when a fully current reference artifact set is needed
  - keep using outside-sandbox ROOT smoke runs as the authoritative Codex validation path on this machine
- project_state_sync_status: `written`
- verification_status carried forward: `verified`

## Next Recommended Step

- If you need one authoritative reusable sample artifact, regenerate `qa/test_b8_5000.root` and its QA companion from the current checkout, then record that run in `project-state/tests.md`.
- If future density-field emission modes are added, implement them as new `EmissionSamplerMode` values that return `EmissionSite`.
- If future density evolution variants are added, extend `buildEventMedium()` so they update `emissionDensity` / `emissionGeometry` while preserving `participantGeometry` for event-summary `eps2/psi2`.
- If future sampler work adds more velocity-selection modes, extend the `flow-velocity-sampler` surface and `FlowFieldModel` dispatch instead of introducing another narrowly named top-level flow switch.
- If future schema work touches trees, histograms, or optional debug payloads, update `docs/agent_guide.md`, `docs/项目说明.md`, `project-state/guide.md`, and `project-state/current-status.md` in the same change.

## Suggested Validation Sequence

1. Check doc and whitespace hygiene:
   - `git diff --check`
2. Re-run the ROOT-free regression if code changed together with doc updates:
   - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
3. Reuse the authoritative ROOT path when runtime validation is needed:
   - generate with `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /tmp/blastwave_docsync_smoke.root`
   - validate with `/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_docsync_smoke.root --output /tmp/blastwave_docsync_smoke_validation.root --expect-nevents 5000`
   - for the legacy comparison path, also run `--density-evolution none`
   - if a non-default sampler changed, also run `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --output /tmp/blastwave_density_normal_sampler_smoke.root`

## What The Next Owner Should Not Redo

- Do not revert the guide or handoff docs to the pre-covariance-ellipse wording.
- Do not rename the generalized sampler surface back to a narrowly scoped `flow-model` contract unless the higher-authority docs explicitly change direction.
- Do not silently couple `flow-density-sigma` back to `smear`; they now carry different responsibilities.
- Do not reintroduce `FlowFieldContext` / `buildFlowFieldContext` naming; the unified internal language is now `EventMedium`.
- Do not rebuild the density-normal snapshot from participant records in the writer; serialize `event.medium.emissionDensity`.
- Do not treat `events.eps2_f` as a replacement for `events.eps2`; it is the freeze-out diagnostic, while `eps2` remains initial participant geometry.
- Do not describe `flow_ellipse_debug` or `flow_ellipse_participant_norm_x-y` as mandatory default objects.
- Do not describe `events.v2` as interchangeable with the initial-state `eps2`.
- Do not treat sandboxed `alienv` ROOT smoke output as authoritative when it emits PCM/module noise; rerun outside the sandbox instead.
