#!/bin/bash

out=$(rcc --clang++ "cout<<(1+2)<<endl;")

diff <(
    cat <<EOF
3
EOF
) <(echo "$out") || exit 1
