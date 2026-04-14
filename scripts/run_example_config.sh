#!/bin/bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
config_path="${repo_root}/config/test_b8.cfg"
output_path="${repo_root}/qa/test_b8_5000.root"

# Use the tracked example config to run one canonical local smoke generation.
exec /bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '\"${repo_root}/bin/generate_blastwave_events\" \"${config_path}\" --output \"${output_path}\"'"
