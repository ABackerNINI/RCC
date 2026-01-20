#!/bin/bash

out=$(rcc --g++ "1+1")

diff <(
    cat <<EOF
2
EOF
) <(echo "$out") || exit 1
