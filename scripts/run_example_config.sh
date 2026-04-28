#!/bin/bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
script_path="${repo_root}/scripts/run_example_config.sh"
build_dir="${repo_root}/build"
cache_path="${build_dir}/CMakeCache.txt"
generator_path="${repo_root}/bin/generate_blastwave_events"
config_path="${repo_root}/config/test_b8_pdf_evo.cfg"
# output_path="${repo_root}/qa/test_b8_5000.root"

# Re-enter the canonical O2Physics runtime before inspecting or rebuilding ROOT-linked binaries.
if [[ "${1:-}" != "--inside-alienv" ]]; then
  exec /bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c bash '${script_path}' --inside-alienv"
fi

# Read the ROOT prefix captured by the last configure so stale build caches can be detected.
read_cached_root_prefix() {
  if [[ ! -f "${cache_path}" ]]; then
    return 0
  fi

  local root_dir=""
  root_dir="$(awk -F= '$1 == "ROOT_DIR:PATH" { print $2 }' "${cache_path}")"
  if [[ -n "${root_dir}" ]]; then
    printf '%s\n' "${root_dir%/cmake}"
  fi
}

# Inspect the generator rpath because the executable itself can still point at an older ROOT build.
read_binary_root_prefix() {
  if [[ ! -x "${generator_path}" ]]; then
    return 0
  fi

  local rpath=""
  while IFS= read -r rpath; do
    if [[ "${rpath}" == */ROOT/*/lib ]]; then
      printf '%s\n' "${rpath%/lib}"
      return 0
    fi
  done < <(
    otool -l "${generator_path}" 2>/dev/null | awk '
      $1 == "cmd" && $2 == "LC_RPATH" { capture = 1; next }
      capture && $1 == "path" { print $2; capture = 0 }
    '
  )
}

# Refresh the configure/build pair whenever the active alienv ROOT no longer matches the cached build.
ensure_generator_matches_runtime_root() {
  local runtime_root="${ROOTSYS:-}"
  if [[ -z "${runtime_root}" ]]; then
    echo "run_example_config.sh failed: ROOTSYS is not set inside the O2Physics runtime." >&2
    exit 1
  fi

  local cached_root=""
  cached_root="$(read_cached_root_prefix)"
  local binary_root=""
  binary_root="$(read_binary_root_prefix)"

  local needs_refresh=0
  if [[ ! -x "${generator_path}" ]]; then
    needs_refresh=1
  fi
  if [[ -n "${cached_root}" && "${cached_root}" != "${runtime_root}" ]]; then
    needs_refresh=1
  fi
  if [[ -n "${binary_root}" && "${binary_root}" != "${runtime_root}" ]]; then
    needs_refresh=1
  fi
  if [[ -z "${cached_root}" && -z "${binary_root}" ]]; then
    needs_refresh=1
  fi

  if [[ "${needs_refresh}" -eq 1 ]]; then
    echo "Refreshing Blast_wave build for ROOTSYS=${runtime_root}" >&2
    cmake --preset default -S "${repo_root}" -DROOT_DIR="${runtime_root}/cmake"
    cmake --build "${build_dir}" --target generate_blastwave_events

    cached_root="$(read_cached_root_prefix)"
    binary_root="$(read_binary_root_prefix)"
    if [[ -n "${cached_root}" && "${cached_root}" != "${runtime_root}" ]]; then
      echo "run_example_config.sh failed: build ROOT mismatch persists (runtime=${runtime_root}, cache=${cached_root}, binary=${binary_root:-<missing>})." >&2
      exit 1
    fi
    if [[ -n "${binary_root}" && "${binary_root}" != "${runtime_root}" ]]; then
      echo "run_example_config.sh failed: build ROOT mismatch persists (runtime=${runtime_root}, cache=${cached_root:-<missing>}, binary=${binary_root})." >&2
      exit 1
    fi
    if [[ -z "${cached_root}" && -z "${binary_root}" ]]; then
      echo "run_example_config.sh failed: build ROOT mismatch persists (runtime=${runtime_root}, cache=${cached_root:-<missing>}, binary=${binary_root:-<missing>})." >&2
      exit 1
    fi
  fi
}

# Use the tracked example config only after the generator matches the active ROOT runtime.
ensure_generator_matches_runtime_root
exec "${generator_path}" "${config_path}"
# exec "${generator_path}" "${config_path}" --output "${output_path}"
