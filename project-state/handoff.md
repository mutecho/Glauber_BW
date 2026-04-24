# Handoff

## Latest Durable Handoff

- Stage completed: density-normal single-event density snapshot output with preserved covariance-ellipse baseline behavior
- Artifact produced:
  - added the optional sampler-specific ROOT histogram `density_normal_event_density_x-y`
  - `RootEventFileWriter` now captures the first event with participants when `flow-velocity-sampler = density-normal` and serializes its smeared participant density as a `TH2`
  - the new histogram is stored with default draw option `LEGO1` so ROOT opens it as a 3D height-style view rather than a heatmap
  - extended `qa_blastwave_output` so it validates that histogram when present for type, finite non-negative bins, positive support, and 3D draw option semantics
  - refreshed the relevant docs and `project-state/` ledger files for the new sampler-specific output contract and validation evidence
- Current contract snapshot:
  - `events` still carries `b`, `Npart`, `eps2`, `psi2`, `v2`, `centrality`, and `Nch`
  - mandatory QA objects remain `Npart`, `eps2`, `psi2`, `v2`, `cent`, `participant_x-y`, `participant_x-y_canvas`, `x-y`, `px-py`, `pT`, `eta`, and `phi`
  - optional debug output remains `flow_ellipse_debug` plus `flow_ellipse_participant_norm_x-y`, gated by `debug-flow-ellipse`
  - optional sampler-specific output now includes `density_normal_event_density_x-y`, gated by `flow-velocity-sampler = density-normal`
  - the default fluid-element velocity sampler remains `covariance-ellipse`
  - the parallel sampler surface now also accepts `density-normal`
  - public flow/runtime knobs now include `rho0`, `rho2`, `flow-power`, `flow-velocity-sampler`, `flow-density-sigma`, and `debug-flow-ellipse`
  - legacy `vmax`, `kappa2`, and `r-ref` still fail fast with explicit migration guidance
- Remaining durable follow-up:
  - regenerate long-lived sample files under `qa/` when a fully current reference artifact set is needed
  - keep using outside-sandbox ROOT smoke runs as the authoritative Codex validation path on this machine
- project_state_sync_status: `written`
- verification_status carried forward: `verified`

## Next Recommended Step

- If you need one authoritative reusable sample artifact, regenerate `qa/test_b8_5000.root` and its QA companion from the current checkout, then record that run in `project-state/tests.md`.
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
   - if a non-default sampler changed, also run `/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 20 --flow-velocity-sampler density-normal --flow-density-sigma 0.5 --output /tmp/blastwave_density_normal_sampler_smoke.root`

## What The Next Owner Should Not Redo

- Do not revert the guide or handoff docs to the pre-covariance-ellipse wording.
- Do not rename the generalized sampler surface back to a narrowly scoped `flow-model` contract unless the higher-authority docs explicitly change direction.
- Do not silently couple `flow-density-sigma` back to `smear`; they now carry different responsibilities.
- Do not describe `flow_ellipse_debug` or `flow_ellipse_participant_norm_x-y` as mandatory default objects.
- Do not describe `events.v2` as interchangeable with the initial-state `eps2`.
- Do not treat sandboxed `alienv` ROOT smoke output as authoritative when it emits PCM/module noise; rerun outside the sandbox instead.
