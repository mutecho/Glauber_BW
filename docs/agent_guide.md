# Blast-Wave Project Agent Guide

## Purpose

This file is the agent-facing map for the repository.
It is intentionally shorter than the full project manual and focuses on:

- where the authoritative runtime description lives
- which semantics are easy to break or misread
- which docs must stay synchronized when behavior changes

## Document Roles

- `docs/项目说明.md`
  - main human-written runtime and physics reference
  - the right place for detailed operator usage, public parameter descriptions, and fuller algorithm walkthroughs
- `docs/数学物理公式流程说明.md`
  - formula-oriented map from participant geometry through medium evolution, emission sampling, flow boost, and flow observables
  - the right place for math-heavy explanations that would make the runtime manual too dense
- `docs/手记文档.md`
  - short note for easy-to-confuse semantics and the current mental model
- `config/test_b8.cfg`
  - canonical tracked example config
- `project-state/guide.md`
  - current coordination view in Chinese
- `project-state/current-status.md`
  - compact snapshot of the current baseline and caveats
- `project-state/tests.md`
  - summarized durable validation evidence
- `project-state/decisions.md`
  - durable contract decisions and supersession history

Do not turn this file back into a second copy of `docs/项目说明.md`.

## Current Scope

- C++17 + ROOT blast-wave event generator for fixed-`b` Pb-Pb-like toy events
- current particle species: direct `pi+`
- default physics path:
  - `density-evolution = affine-gaussian`
  - `flow-velocity-sampler = covariance-ellipse`
- optional comparison path:
  - `density-evolution = none`
- optional affine-effective closure path:
  - `density-evolution = affine-gaussian`
  - `flow-velocity-sampler = affine-effective`
  - default `affine-effective-mode = additive-rho`, optional `full-tensor`
- optional coupled V2 path:
  - `density-evolution = gradient-response`
  - `flow-velocity-sampler = gradient-response`
- optional differential analysis:
  - configured `v2{2}(pT)` through `v2pt-bins`
  - joint writing during generation or standalone post-processing through `analyze_blastwave_v2pt`

## Repository Map

- `include/` + `src/`
  - ROOT-free physics core, shared math helpers, and output-schema helpers
- `apps/generate_blastwave_events.cpp` + `apps/generate_blastwave/`
  - generator entrypoint, CLI/config parsing, and ROOT writing
- `apps/qa_blastwave_output.cpp`
  - independent ROOT-reading validation app
- `apps/analyze_blastwave_v2pt.cpp`
  - standalone differential-flow post-processing
- `tests/`
  - ROOT-free regression coverage
- `config/`
  - tracked example configs
- `project-state/`
  - coordination ledger, lower authority than code and human-written design docs

## Operator Entry Points

- build inside the O2Physics ROOT environment
- generation supports:
  - `generate_blastwave_events [options]`
  - `generate_blastwave_events --config <path> [options]`
  - `generate_blastwave_events <config-path> [options]`
- QA supports:
  - `qa_blastwave_output --input <result.root> --output <validation.root> --expect-nevents <N>`
- differential `v2{2}(pT)` post-processing supports:
  - `analyze_blastwave_v2pt --input <result.root> [--output <analysis.root> | --inplace]`
- precedence rules:
  - explicit CLI values override config-file values
  - config-file values override built-in defaults
  - relative `output` and `v2pt-output` paths resolve relative to the config file directory

For exact example commands, use `docs/项目说明.md`.

## Change-Sensitive Semantics

- `events.eps2` / `events.psi2` are initial participant-geometry observables.
- `events.eps2_f` / `events.psi2_f` / `events.chi2` are freeze-out diagnostics and do not replace the initial-state summary.
- `source_x/source_y`, `x0/y0`, and `x/y` are different coordinate stages:
  - participant anchor
  - marker initial position
  - final emission position
- `events.v2` is the event-level final-state summary observable.
- differential `v2{2}(pT)` is a separate analysis payload and currently uses unit track weights only.
- `density-normal-kappa-compensation` is opt-in and only meaningful for `affine-gaussian + density-normal`.
- `affine-effective` is opt-in and only valid for `affine-gaussian`.
- `affine-effective-mode = additive-rho` keeps density-normal direction and keeps `rho0` as baseline average flow.
- `affine-effective-mode = full-tensor` is opt-in and directly uses principal-axis tensor velocity closure.
- `affine-kappa-aniso` remains parse/finite-compatible but is legacy/no-op in both affine-effective modes.
- `gradient-response` medium and flow are intentionally coupled; enabling only one side is invalid.
- `v2pt-output-mode = separate-file` may leave the main result file in a metadata-only state with `v2_2_pt_edges` but without `v2_2_pt` or `v2_2_pt_canvas`.
- sandboxed `alienv` ROOT smoke output on this machine is not authoritative when PCM/module noise appears; rerun outside the sandbox.

## Output Contract Highlights

- mandatory trees:
  - `events`
  - `participants`
  - `particles`
- mandatory event summary highlights:
  - `centrality`
  - `v2`
  - `eps2_f`
  - `psi2_f`
  - `chi2`
  - `r2_0`
  - `r2_f`
  - `r2_ratio`
- mandatory particle payload highlights:
  - `x0`
  - `y0`
  - `emission_weight`
- optional payload groups:
  - flow-ellipse debug objects, optionally extended with affine closure diagnostics
  - affine-effective before/after density maps for the first valid event
  - density-normal event-density snapshot
  - gradient-response debug histograms
  - differential `v2{2}(pT)` metadata and analysis objects

For the exhaustive object list, use `docs/项目说明.md` or `include/blastwave/io/RootOutputSchema.h`.
Do not duplicate the full object table here unless this file's role changes.

## Validation Expectations

- keep ROOT-free tests aligned with parser, flow, emission, thermal, and shared-analysis math changes
- keep `project-state/tests.md` as the concise durable evidence home rather than a raw command archive
- when physics, config parsing, schema, QA behavior, or operator flow changes, update docs in the same patch

## Required Doc Sync

When a task changes any of the items below, update these docs together:

- algorithm or runtime semantics:
  - `docs/项目说明.md`
  - `docs/手记文档.md` when the change is easy to misread
  - `project-state/guide.md`
  - `project-state/current-status.md`
- config surface or operator flow:
  - `README.md` if entrypoints or top-level structure moved
  - `docs/项目说明.md`
  - `project-state/guide.md`
- output schema or QA behavior:
  - `docs/项目说明.md`
  - `project-state/current-status.md`
  - `project-state/tests.md`
- durable decisions or follow-up ownership:
  - `project-state/handoff.md`
  - `project-state/decisions.md`
  - `project-state/issues.md` / `project-state/work-items.md` when still open

## What This File Should Not Become

- not a second project manual
- not a raw changelog
- not a dump of full parameter tables copied from help text or config comments
- not the primary source of exact ROOT object names when the schema code already owns them
