# Current Status

## Snapshot

- Date: 2026-04-13
- Repository: `/Users/allenzhou/Research_software/Blast_wave`
- Branch state: `master` is aligned with `upstream/master`; the task-start working tree was clean.
- Active coordination task: resync `project-state/` to the post-centrality-output baseline and carry forward the remaining config-file validation/doc-path follow-up.

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
- The currently tracked example config is `config/test_b8.cfg`, and the repository also tracks a convenience launcher at `config/run.sh`.
- The `qa/` directory contains historical sample ROOT outputs from multiple schema revisions; they are useful artifacts, but not all of them represent the latest full output contract.

## Evidence Used

- Higher-authority human-written docs:
  - `docs/agent_guide.md`
  - `docs/项目说明.md`
  - `docs/blastwave_generator_agent_handoff.md`
- Current code and schema sources:
  - `include/blastwave/BlastWaveGenerator.h`
  - `include/blastwave/io/RootOutputSchema.h`
  - `src/BlastWaveGenerator.cpp`
  - `src/RootOutputSchema.cpp`
  - `apps/generate_blastwave_events.cpp`
  - `apps/qa_blastwave_output.cpp`
- Tracked runtime/config artifacts:
  - `config/test_b8.cfg`
  - `config/run.sh`
- Existing durable validation ledger entries:
  - `project-state/tests.md` (`T-001`, `T-002`, `T-003`)
- Current commit baseline:
  - `ff10639 add cent based on b-param`
  - `8ba4af9 reoeganize struct & support config file as input`
  - `35315a8 add part tree and x-y graph`

## Current Gaps And Blockers

- Higher-authority docs under `docs/` still refer to the old config example path `qa/test_b8.cfg`, while the tracked file in the current checkout is `config/test_b8.cfg`.
- The config-file CLI contract remains only partially verified:
  - parser and entrypoint behavior were exercised historically
  - the repository now has a tracked sample config path
  - but there is not yet a durable generate-and-validate QA record tied to the current tracked config path
- Historical sample ROOT outputs in `qa/` span multiple contract generations:
  - older files may miss `participants`, `participant_x-y`, or `participant_x-y_canvas`
  - files generated before the centrality extension may also miss `events.centrality` and `cent`
- The ledger path has already been normalized to `project-state/`; future updates should not reintroduce `.project-state/` references.

## Verification Status

- verification_status: `partially verified`
- Rationale:
  - the explicit-CLI smoke path and the current centrality contract have a passed QA record on 2026-04-13
  - the participant output contract also has a passed QA record
  - the remaining verification gap is narrower now: it is the durable end-to-end record for the current tracked config-file sample path, plus the stale human-facing path references that can misdirect operators
