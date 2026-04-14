# Current Status

## Snapshot

- Date: 2026-04-14
- Repository: `/Users/allenzhou/Research_software/Blast_wave`
- Branch state: `master` with local repository-layout cleanup updates in the working tree.
- Active coordination task: normalize top-level structure so tracked configs, helper scripts, and historical reference macros no longer share ambiguous landing zones.

## Confirmed Baseline

- The repository contains a C++17 + ROOT blast-wave event generator with a ROOT-free physics core in `include/` and `src/`, a ROOT-writing generation app in `apps/generate_blastwave_events.cpp`, and an independent ROOT-reading QA app in `apps/qa_blastwave_output.cpp`.
- Human-written docs still describe the intended scope as fixed-impact-parameter Pb-Pb-like event generation for one direct charged pion species with participant geometry fluctuations.
- The current on-disk ROOT contract includes:
  - `events`, `participants`, and `particles` trees
  - `events.centrality` derived from the configured fixed `b`
  - QA objects `Npart`, `eps2`, `psi2`, `cent`, `participant_x-y`, `participant_x-y_canvas`, `x-y`, `px-py`, `pT`, `eta`, and `phi`
- The QA reader now validates:
  - participant tree presence and multiplicity consistency
  - participant histogram/canvas presence
  - `centrality` staying within `[0, 100]`
  - consistency between `events.centrality` and the current fixed-`b` mapping
  - fixed-`b` runs keeping one constant centrality value across the events tree
- The config-file CLI remains part of the public interface:
  - `generate_blastwave_events --config <path> [options]`
  - `generate_blastwave_events <config-path> [options]`
  - explicit CLI options override config-file values, which override built-in defaults
  - relative `output` paths resolve relative to the config file directory
- The thermal momentum contract now includes two explicit modes:
  - default `maxwell-juttner` lookup-table sampling in the local rest frame
  - legacy `gamma` sampling as an explicit compatibility mode
- The public config/CLI surface now exposes:
  - `thermal-sampler`
  - `mj-pmax`
  - `mj-grid-points`
- The build now registers a ROOT-free core regression test target:
  - `test_maxwell_juttner_sampler`
- The currently tracked example configs are `config/test_b8.cfg` and `config/b8.cfg`, while the convenience launcher now lives at `scripts/run_example_config.sh`.
- Historical reference ROOT macros now live under `reference/legacy-root-macros/` instead of a top-level personal-name directory.
- The `qa/` directory contains historical sample ROOT outputs from multiple schema revisions; they are useful artifacts, but not all of them represent the latest full output contract.

## Evidence Used

- Higher-authority human-written docs:
  - `docs/agent_guide.md`
  - `docs/项目说明.md`
  - `docs/blastwave_generator_agent_handoff.md`
- Current code and schema sources:
  - `include/blastwave/BlastWaveGenerator.h`
  - `include/blastwave/MaxwellJuttnerMomentumSampler.h`
  - `include/blastwave/io/RootOutputSchema.h`
  - `src/BlastWaveGenerator.cpp`
  - `src/MaxwellJuttnerMomentumSampler.cpp`
  - `src/RootOutputSchema.cpp`
  - `apps/generate_blastwave_events.cpp`
  - `apps/qa_blastwave_output.cpp`
  - `tests/MaxwellJuttnerMomentumSamplerTest.cpp`
- Tracked runtime/config artifacts:
  - `config/test_b8.cfg`
  - `config/b8.cfg`
  - `scripts/run_example_config.sh`
  - `reference/legacy-root-macros/README.md`
- Existing durable validation ledger entries:
  - `project-state/tests.md` (`T-001`, `T-002`, `T-003`, `T-004`)
- Current commit baseline:
  - `ff10639 add cent based on b-param`
  - `8ba4af9 reoeganize struct & support config file as input`
  - `35315a8 add part tree and x-y graph`

## Current Gaps And Blockers

- Historical sample ROOT outputs in `qa/` span multiple contract generations:
  - older files may miss `participants`, `participant_x-y`, or `participant_x-y_canvas`
  - files generated before the centrality extension may also miss `events.centrality` and `cent`
  - files generated before the Maxwell-Juttner switch may also reflect the older Gamma-only thermal default
- ROOT smoke commands launched inside the Codex sandbox are still not authoritative on this machine:
  - the 2026-04-14 sandboxed `alienv` ROOT runs emitted PCM/module errors
  - the same commands passed immediately after rerunning outside the sandbox with escalation
- The ledger path has already been normalized to `project-state/`; future updates should not reintroduce `.project-state/` references.

## Verification Status

- verification_status: `verified`
- Rationale:
  - the project was reconfigured and rebuilt on 2026-04-14 after the Maxwell-Juttner change set landed
  - the new ROOT-free core test target passed through `ctest`
  - `generate_blastwave_events --help` and the new parser failure paths for thermal-sampler options were exercised successfully
  - the canonical `config/test_b8.cfg` path now has one durable generate+QA record under the default Maxwell-Juttner mode
  - an explicit `--thermal-sampler gamma` compatibility smoke also passed the independent QA reader
