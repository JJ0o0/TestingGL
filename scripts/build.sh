#!/bin/bash
set -euo pipefail

case "${1:-debug}" in
    debug)
        build_type="Debug"
        ;;

    release)
        build_type="Release"
        ;;

    *)
        echo "Uso: $0 [debug|release]"
        exit 1
        ;;
esac

cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE="$build_type"
cmake --build build
