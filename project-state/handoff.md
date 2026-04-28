# Handoff

## Latest Durable Handoff

- Stage completed: public affine V1a evolution knob exposure
- Artifact produced:
  - added public CLI/config keys `affine-lambda-in`, `affine-lambda-out`, and `affine-sigma-evo`
  - moved the V1a affine response defaults onto `BlastWaveConfig` instead of hardcoding them at the generator boundary
  - kept the previous defaults unchanged: `1.20`, `1.05`, and `0.5 fm`
  - added validation for invalid affine expansion and smoothing values
  - updated regression tests, sample configs, user docs, and `project-state/`
- Current contract snapshot:
  - `covariance-ellipse` remains unchanged and still uses the initial-state `kappa2 * eps2` response vector
  - `density-normal` always derives direction from `emissionDensity` gradient with an `emissionGeometry` fallback
  - `affine-gaussian + density-normal` still defaults to no explicit `kappa2` modulation
  - `density-normal-kappa-compensation = true` restores the historical affine density-normal exponential multiplier and is only legal with `density-evolution = affine-gaussian` plus `flow-velocity-sampler = density-normal`
  - public flow/runtime knobs now include `rho0`, `kappa2`, `flow-power`, `flow-velocity-sampler`, `density-evolution`, `flow-density-sigma`, `affine-lambda-in`, `affine-lambda-out`, `affine-sigma-evo`, `density-normal-kappa-compensation`, V2 `gradient-*` parameters, `debug-flow-ellipse`, `debug-gradient-response`, and `cooper-frye-weight`
  - ROOT trees and optional histogram payloads are unchanged by this contract update
- Remaining durable follow-up:
  - regenerate long-lived sample files under `qa/` when a fully current reference artifact set is needed
  - keep using outside-sandbox ROOT smoke runs as the authoritative Codex validation path on this machine
- project_state_sync_status: `written`
- verification_status carried forward: `tested`

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
   - if affine `density-normal` changed, run both:
    - `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --output /tmp/blastwave_density_normal_sampler_smoke.root`
    - `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --density-normal-kappa-compensation --output /tmp/blastwave_density_normal_sampler_compensated_smoke.root`

## What The Next Owner Should Not Redo

- Do not revert the guide or handoff docs to the pre-covariance-ellipse wording.
- Do not rename the generalized sampler surface back to a narrowly scoped `flow-model` contract unless the higher-authority docs explicitly change direction.
- Do not silently couple `flow-density-sigma` back to `smear`; they now carry different responsibilities.
- Do not reintroduce `FlowFieldContext` / `buildFlowFieldContext` naming; the unified internal language is now `EventMedium`.
- Do not rebuild the density-normal snapshot from participant records in the writer; serialize `event.medium.emissionDensity`.
- Do not silently re-enable explicit `kappa2` modulation in affine `density-normal`; compensation is opt-in now.
- Do not treat `events.eps2_f` as a replacement for `events.eps2`; it is the freeze-out diagnostic, while `eps2` remains initial participant geometry.
- Do not treat `x0/y0`, `source_x/source_y`, and `x/y` as interchangeable; they are marker initial position, participant anchor, and final emission position respectively.
- Do not enable only one side of V2 `gradient-response`; density evolution and flow velocity sampler are intentionally coupled.
- Do not describe `flow_ellipse_debug` or `flow_ellipse_participant_norm_x-y` as mandatory default objects.
- Do not describe `events.v2` as interchangeable with the initial-state `eps2`.
- Do not treat sandboxed `alienv` ROOT smoke output as authoritative when it emits PCM/module noise; rerun outside the sandbox instead.
