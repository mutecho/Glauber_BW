# Current Status

## Snapshot

- Date: 2026-04-11
- Repository: `/Users/allenzhou/Research_software/Blast_wave`
- Branch state: repository currently has local working-tree modifications from ongoing feature work; this ledger entry reflects the code state after the participant-geometry output update, config-file CLI support, and a shipped example config
- Active coordination task: config-file CLI support, example config publication, and validation follow-up for the new file-driven entrypoint

## Confirmed Baseline

- The repository contains a C++17 + ROOT blast-wave event generator with a ROOT-free physics core in `include/` and `src/`, a generation app in `apps/generate_blastwave_events.cpp`, and an independent QA app in `apps/qa_blastwave_output.cpp`.
- Human-written project docs describe the current intended scope as fixed-impact-parameter Pb-Pb-like event generation for one direct charged pion species, with ROOT trees plus embedded QA histograms.
- The repository already contains sample QA artifacts under `qa/`, including `test_b4.root`, `test_b8.root`, `test_b8_5000.root`, and matching validation outputs.
- The output contract now includes a dedicated `participants` tree plus a `participant_x-y` histogram and `participant_x-y_canvas` object that overlays nucleus-A and nucleus-B circle outlines using the configured impact parameter and Woods-Saxon radius.
- The participant canvas now keeps the ROOT stats box enabled so the saved visual object includes the standard entries/mean/RMS summary in the upper-right corner.（人工修复了有两个stats标签的问题）
- The generator CLI now accepts configuration files in addition to explicit flags:
  - `generate_blastwave_events --config <path> [options]`
  - `generate_blastwave_events <config-path> [options]`
- The configuration file contract is now a durable public interface:
  - plain-text `key = value`
  - blank lines and full-line `#` comments ignored
  - keys aligned with the current public CLI flag surface
  - explicit CLI options override config-file values, which override built-in defaults
  - relative `output` paths inside config files resolve relative to the config file directory
- The repository now ships a runnable example config at `qa/test_b8.cfg`.

## Evidence Used

- Human-written markdown:
  - `docs/agent_guide.md`
  - `docs/项目说明.md`
  - `docs/blastwave_generator_agent_handoff.md`
- Repository structure and artifact presence:
  - `include/blastwave/BlastWaveGenerator.h`
  - `src/BlastWaveGenerator.cpp`
  - `apps/generate_blastwave_events.cpp`
  - `apps/qa_blastwave_output.cpp`
  - `qa/`
- Fresh authoritative validation commands run on 2026-04-10:
  - configure: `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"`
  - build: `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
  - generate: `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 10 --output /Users/allenzhou/Research_software/Blast_wave/qa/test_psi2.root'"`
  - validate: `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/test_psi2.root --output /Users/allenzhou/Research_software/Blast_wave/qa/test_psi2_validation.root --expect-nevents 10'"`
- Fresh config-file CLI evidence gathered on 2026-04-11:
  - build: `/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"`
  - help output confirms the new config-file entrypoints and precedence text
  - generator run: `./bin/generate_blastwave_events qa/test_b8.cfg`
  - parser failure paths checked:
    - missing config file
    - invalid `key=value` line
    - unknown key
    - duplicate key
    - invalid numeric value
    - simultaneous positional config path and `--config`

## Current Gaps And Blockers

- Historical large sample files such as `qa/test_b8_5000.root` predate the new participant output contract and must be regenerated if downstream work expects `participants`, `participant_x-y`, or `participant_x-y_canvas` in those files.
- The ledger is present and now reflects fresh validation, but future engineering work still needs incremental updates rather than full-file rewrites.
- In this Codex session, ROOT read-side validation is currently not authoritative:
  - `qa_blastwave_output` failed with ROOT PCM/module loading errors even when pointed at pre-existing sample files
  - generator writes still succeeded, so the current gap is specific to this session's ROOT read environment rather than the config parser itself

## Verification Status

- verification_status: `partially verified`
- Rationale:
  - build completed successfully after the config-file CLI change
  - the new config-file entrypoints, precedence rules, and parser error paths were exercised successfully
  - a shipped sample config generated a new ROOT file
  - independent QA reader validation for this task was not reproduced in the current Codex ROOT read environment, so end-to-end validation remains incomplete for this specific change set
