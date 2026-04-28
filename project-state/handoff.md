# Handoff

## Latest Durable Handoff

- Stage completed: opt-in V2 gradient-response medium and flow implementation
- Artifact produced:
  - added `DensityEvolutionMode::GradientResponse` and `FlowVelocitySamplerMode::GradientResponse`
  - required V2 density and flow modes to be selected together
  - constructed separate `s_em` marker and `s_dyn` dynamics densities from participant Gaussian point clouds
  - sampled V2 marker positions `r0`, final emission positions `rf`, gradient-driven displacement, and site transverse velocities through `EmissionSite`
  - kept default `density-evolution = affine-gaussian` and legacy `density-evolution = none` behavior available
  - added optional `cooper-frye-weight = none|mt-cosh`, defaulting to unit weights
  - extended mandatory ROOT schema with event `r2_0/r2_f/r2_ratio` and particle `x0/y0/emission_weight`
  - added optional V2 debug density TH2s gated by `debug-gradient-response`
  - updated QA, regression tests, docs, and `project-state/` ledger
- Current contract snapshot:
  - `events` carries `b`, `Npart`, `eps2`, `psi2`, `eps2_f`, `psi2_f`, `chi2`, `r2_0`, `r2_f`, `r2_ratio`, `v2`, `centrality`, and `Nch`
  - `particles` carries the legacy kinematic/source payload plus `x0`, `y0`, and `emission_weight`
  - mandatory QA objects include `Npart`, `eps2`, `eps2_f`, `psi2`, `psi2_f`, `chi2`, `r2_0`, `r2_f`, `r2_ratio`, `v2`, `cent`, `participant_x-y`, `participant_x-y_canvas`, `x-y`, `px-py`, `pT`, `eta`, and `phi`
  - optional debug output remains `flow_ellipse_debug` plus `flow_ellipse_participant_norm_x-y`, gated by `debug-flow-ellipse`
  - optional sampler-specific output remains `density_normal_event_density_x-y`, gated by `flow-velocity-sampler = density-normal`
  - optional V2 debug output is `gradient_s0_x-y`, `gradient_s_em_x-y`, `gradient_s_dyn_x-y`, and `gradient_s_f_x-y`, gated by `debug-gradient-response`
  - public flow/runtime knobs now include `rho0`, `kappa2`, `flow-power`, `flow-velocity-sampler`, `density-evolution`, `flow-density-sigma`, V2 `gradient-*` parameters, `debug-flow-ellipse`, `debug-gradient-response`, and `cooper-frye-weight`
  - legacy `vmax`, `rho2`, and `r-ref` still fail fast with explicit migration guidance
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
- If future V2-like variants assume a shared density gradient, keep the density and velocity mode coupling explicit rather than silently mixing them with V1a or `none`.
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
   - for V2 changes, also run `--density-evolution gradient-response --flow-velocity-sampler gradient-response`
   - if a non-default sampler changed, also run `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --output /tmp/blastwave_density_normal_sampler_smoke.root`

## What The Next Owner Should Not Redo

- Do not revert the guide or handoff docs to the pre-covariance-ellipse wording.
- Do not rename the generalized sampler surface back to a narrowly scoped `flow-model` contract unless the higher-authority docs explicitly change direction.
- Do not silently couple `flow-density-sigma` back to `smear`; they now carry different responsibilities.
- Do not reintroduce `FlowFieldContext` / `buildFlowFieldContext` naming; the unified internal language is now `EventMedium`.
- Do not rebuild the density-normal snapshot from participant records in the writer; serialize `event.medium.emissionDensity`.
- Do not treat `events.eps2_f` as a replacement for `events.eps2`; it is the freeze-out diagnostic, while `eps2` remains initial participant geometry.
- Do not treat `x0/y0`, `source_x/source_y`, and `x/y` as interchangeable; they are marker initial position, participant anchor, and final emission position respectively.
- Do not enable only one side of V2 `gradient-response`; density evolution and flow velocity sampler are intentionally coupled.
- Do not describe `flow_ellipse_debug` or `flow_ellipse_participant_norm_x-y` as mandatory default objects.
- Do not describe `events.v2` as interchangeable with the initial-state `eps2`.
- Do not treat sandboxed `alienv` ROOT smoke output as authoritative when it emits PCM/module noise; rerun outside the sandbox instead.
