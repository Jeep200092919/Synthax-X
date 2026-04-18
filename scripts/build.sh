#!/usr/bin/env bash
# Convenience build wrapper for Synthax X.
set -euo pipefail

cd "$(dirname "$0")/.."
bash scripts/check_tools.sh
make iso "$@"
echo "Build complete: bin/synthax-x.iso"
