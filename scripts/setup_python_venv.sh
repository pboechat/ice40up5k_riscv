#!/bin/bash

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

try_install() {
	local package="$1"
	if ! python -m pip list 2>/dev/null | grep -q "$package"; then
		pip install "$package"
	fi
}

VENV_DIR="$SCRIPT_DIR/.venv"

set +x

if [[ ! -d "$VENV_DIR" ]]; then
	python -m venv "$VENV_DIR" 
fi

source "$VENV_DIR/bin/activate"

while IFS= read -r line; do
    [[ -z "$line" || "$line" =~ ^# ]] && continue
    try_install "$line"
done < "$SCRIPT_DIR/requirements.txt"
