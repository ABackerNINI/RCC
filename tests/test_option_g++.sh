#!/bin/bash

out=$(rcc --g++ "cout<<(1+1)<<endl;")

diff <(
    cat <<EOF
2
EOF
) <(echo "$out") || exit 1
