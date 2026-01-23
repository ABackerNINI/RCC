#!/bin/bash

# The fmt library is NOT included by default, so it should fail.
rcc 'fmt::print("{}\n", 1+2);' 1>/dev/null 2>&1 && exit 1

out=$(rcc --include-fmt 'fmt::print("{}\n", 1+2);')

diff <(
    cat <<EOF
3
EOF
) <(echo "$out") || exit 1

# Test the -fmt option as well. This is equivalent to --include-fmt.
out=$(rcc -fmt 'fmt::print("{}\n", 1+3);')

diff <(
    cat <<EOF
4
EOF
) <(echo "$out") || exit 1
