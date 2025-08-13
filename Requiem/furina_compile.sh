#!/bin/bash

#!/usr/bin/env bash
set -euo pipefail
SRC="./src/furina_no_requiem.c"
OUT="./bin/Se mettre sur son trente-et-un"

gcc -o "$OUT" "$SRC"

echo "Built: ./$OUT"


# SOURCE_FILE=.\src\furina_no_requiem.c
# OUTPUT_NAME=".\bin\Se mettre sur son trente-et-un.exe"

# gcc -o %OUTPUT_NAME% %SOURCE_FILE%

# if [ $? -eq 0 ]; then
#   echo "Compile successful! Furina has successfully sung her requiem."
# else
#   echo "Compile failed."
# fi