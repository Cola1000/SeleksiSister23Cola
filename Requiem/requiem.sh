#!/bin/bash
#!/usr/bin/env bash

# set -euo pipefail
# SRC="./src/furina_no_requiem.c"
# OUT="./bin/Se mettre sur son trente-et-un"

# gcc -o "$OUT" "$SRC"

# echo "Built: ./$OUT"


set -euo pipefail
SRC="./src/focalor_no_requiem.c"
OUT="./bin/Sinner's-Finale"

gcc -o "$OUT" "$SRC"

echo "Built: ./$OUT"