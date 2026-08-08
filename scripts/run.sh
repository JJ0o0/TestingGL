#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"

CONFIG="${1:-Debug}"
BIN_DIR="$ROOT_DIR/build/bin/$CONFIG"

cd "$BIN_DIR"
exec ./TestingGL
