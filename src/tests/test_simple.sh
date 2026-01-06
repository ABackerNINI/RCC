#!/bin/bash

out=$(rcc 'cout << "Hello, World!" << endl;')

diff <(
    cat <<EOF
Hello, World!
EOF
) <(echo "$out") || exit 1

out=$(rcc 'cout << "Hello, World!" << endl;' 'cout << "Hello, World!" << endl;')

diff <(
    cat <<EOF
Hello, World!
Hello, World!
EOF
) <(echo "$out") || exit 1
