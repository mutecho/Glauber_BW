# Handoff

## Latest Durable Handoff

- Stage completed: Maxwell-Juttner thermal-sampler switch plus `project-state/`/docs sync
- Artifact produced:
  - added the ROOT-free `MaxwellJuttnerMomentumSampler` core component and switched the default thermal spectrum mode to `maxwell-juttner`
  - kept `gamma` as an explicit compatibility mode with the existing ROOT output contract
  - extended the runtime/config contract with `thermal-sampler`, `mj-pmax`, and `mj-grid-points`
  - added and passed the `test_maxwell_juttner_sampler` CTest target
  - updated higher-authority docs in `docs/` to the canonical example-config path `config/test_b8.cfg`
  - recorded one authoritative default-MJ config-file generate+QA run and one authoritative explicit-gamma compatibility smoke
- Remaining durable follow-up:
  - regenerate any long-lived sample ROOT files under `qa/` that should represent the new default Maxwell-Juttner baseline
  - keep using outside-sandbox ROOT smoke commands in Codex sessions until the sandbox/`alienv` PCM issue is fixed
- project_state_sync_status: `written`
- verification_status carried forward: `verified`

## Next Recommended Step

- If a durable reference sample should match the new default physics baseline, regenerate a tracked `qa/` ROOT sample under `maxwell-juttner` and rerun the independent QA reader on it.
- If future Codex-driven ROOT validation is expected, preserve the outside-sandbox invocation pattern because the sandboxed `alienv` ROOT runtime is still not authoritative on this machine.

## Suggested Validation Sequence

1. Configure with the default preset:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
2. Build the current checkout:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
3. Run the ROOT-free sampler regression:
   - `cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure`
4. Run the canonical config-file smoke outside the sandbox:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /tmp/blastwave_mj_default_smoke.root'"`
5. Validate the default-MJ output outside the sandbox:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_mj_default_smoke.root --output /tmp/blastwave_mj_default_smoke_validation.root --expect-nevents 5000'"`
6. Run the explicit gamma compatibility smoke outside the sandbox:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --nevents 100 --thermal-sampler gamma --output /tmp/blastwave_gamma_smoke.root'"`
7. Validate the gamma output outside the sandbox:
   - `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /tmp/blastwave_gamma_smoke.root --output /tmp/blastwave_gamma_smoke_validation.root --expect-nevents 100'"`

## What The Next Owner Should Not Redo

- Do not revert the thermal default to Gamma unless the public config/CLI contract is intentionally changed again.
- Do not reintroduce `qa/test_b8.cfg` into higher-authority docs; keep `config/test_b8.cfg` as the canonical tracked path.
- Do not treat sandboxed `alienv` ROOT smoke output as authoritative when it emits PCM/module errors; rerun outside the sandbox instead.
- Do not treat historical `qa/` ROOT files as interchangeable proof of the new Maxwell-Juttner default baseline.
