#!/bin/bash

# The array header is NOT included by default, so it should fail.
rcc 'array<int, 2> a; cout << a.size() << endl;' 1>/dev/null 2>&1 && exit 1

out=$(rcc --include array 'array<int, 2> a; cout << a.size() << endl;')

diff <(
    cat <<EOF
2
EOF
) <(echo "$out") || exit 1
