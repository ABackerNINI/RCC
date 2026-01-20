#!/bin/bash

# Test auto-wrap with only one argument

out=$(rcc "1+1")

diff <(
    cat <<EOF
2
EOF
) <(echo "$out") || exit 1

# Test auto-wrap the last argument

out=$(rcc "int a=1,b=2;" "a+b")

diff <(
    cat <<EOF
3
EOF
) <(echo "$out") || exit 1

# Test auto-wrap fall back to non-warp for the last argument with trailing spaces

out=$(rcc "int a=1,b=2;" "cout<<a+b<<endl; ")

diff <(
    cat <<EOF
3
EOF
) <(echo "$out") || exit 1

