# Current Status

## Snapshot

- Date: 2026-04-10
- Repository: `/Users/allenzhou/Research_software/Blast_wave`
- Branch state: repository currently has local working-tree modifications from ongoing feature work; this ledger entry reflects the code state after the participant-geometry output update and a fresh local validation run
- Active coordination task: participant geometry output, participant canvas visualization, and project-state sync

## Confirmed Baseline

- The repository contains a C++17 + ROOT blast-wave event generator with a ROOT-free physics core in `include/` and `src/`, a generation app in `apps/generate_blastwave_events.cpp`, and an independent QA app in `apps/qa_blastwave_output.cpp`.
- Human-written project docs describe the current intended scope as fixed-impact-parameter Pb-Pb-like event generation for one direct charged pion species, with ROOT trees plus embedded QA histograms.
- The repository already contains sample QA artifacts under `qa/`, including `test_b4.root`, `test_b8.root`, `test_b8_5000.root`, and matching validation outputs.
- The output contract now includes a dedicated `participants` tree plus a `participant_x-y` histogram and `participant_x-y_canvas` object that overlays nucleus-A and nucleus-B circle outlines using the configured impact parameter and Woods-Saxon radius.
- The participant canvas now keeps the ROOT stats box enabled so the saved visual object includes the standard entries/mean/RMS summary in the upper-right corner.（人工修复了有两个stats标签的问题）

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

## Current Gaps And Blockers

- Historical large sample files such as `qa/test_b8_5000.root` predate the new participant output contract and must be regenerated if downstream work expects `participants`, `participant_x-y`, or `participant_x-y_canvas` in those files.
- The ledger is present and now reflects fresh validation, but future engineering work still needs incremental updates rather than full-file rewrites.

## Verification Status

- verification_status: `passed`
- Rationale: the current checkout was configured, built, used to generate a fresh 10-event sample, and successfully validated with the independent QA reader.
