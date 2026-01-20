#!/bin/bash

out=$(rcc --clang++ "1+2")

diff <(
    cat <<EOF
3
EOF
) <(echo "$out") || exit 1
