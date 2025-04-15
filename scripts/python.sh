#!/bin/bash

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

if [ -z "$1" ]; then
    echo "missing script"
    exit 0
fi

$SCRIPT_DIR/setup_python_venv.sh

source "$SCRIPT_DIR/.venv/bin/activate"

python $@
