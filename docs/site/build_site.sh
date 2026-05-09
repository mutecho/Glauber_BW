#!/usr/bin/env bash
set -euo pipefail

site_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${site_dir}/../.." && pwd)"

readonly math_format="markdown+tex_math_single_backslash+tex_math_dollars"
readonly asset_version="20260509g"

# Render one Pandoc page with the shared site chrome and MathJax contract.
render_page() {
  local title="$1"
  local output="$2"
  shift 2

  pandoc \
    -f "${math_format}" \
    --standalone \
    --toc \
    --mathjax \
    --css "theme.css?v=${asset_version}" \
    --include-after-body="${site_dir}/enhance.html" \
    --metadata "title=${title}" \
    "$@" \
    -o "${site_dir}/${output}"
}

# Render the main hub without a local TOC; it only routes to focused documents.
render_home() {
  pandoc \
    -f "${math_format}" \
    --standalone \
    --mathjax \
    --css "theme.css?v=${asset_version}" \
    --include-after-body="${site_dir}/enhance.html" \
    "${site_dir}/home.md" \
    -o "${site_dir}/index.html"
}

cd "${repo_root}"
render_home
render_page "Blast-Wave Overview" "overview.html" README.md docs/README.md
render_page "Blast-Wave 项目说明" "project.html" docs/项目说明.md
render_page "Blast-Wave 数学物理公式流程说明" "formulas.html" docs/数学物理公式流程说明.md
render_page "Blast-Wave 简明手记" "notes.html" docs/手记文档.md
