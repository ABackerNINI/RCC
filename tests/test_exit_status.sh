#!/bin/bash

# Test return 0 with no return statement
out=$(rcc "cout<<(1+1)<<endl;")

diff <(
    cat <<EOF
2
EOF
) <(echo "$out") || exit 1

# Test return 0 with explicit return statement
out=$(rcc "cout<<(1+2)<<endl; return 0;")

diff <(
    cat <<EOF
3
EOF
) <(echo "$out") || exit 1

# Test return 1
rcc 'cout<<(1+3)<<endl; return 1;' && exit 1
ec=$?

if [ "$ec" -ne 1 ]; then
    exit 1
fi

# Test return 6
rcc 'return 6;' 1>/dev/null 2>&1 && exit 1
ec=$?

if [ "$ec" -ne 6 ]; then
    exit 1
fi

# Test error return for compile error
rcc 'int a = ;' 1>/dev/null 2>&1 && exit 1

exit 0
