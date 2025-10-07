#!/usr/bin/env bash
set -euo pipefail

cmake -S "$(dirname "$0")/.." -B "$(dirname "$0")/../build" "$@"
