#!/bin/bash

out=$(rcc --put-above-main '#define print_string_macro(s) cout << s << endl;' \
    'print_string_macro("Hello, World!");')

diff <(
    cat <<EOF
Hello, World!
EOF
) <(echo "$out") || exit 1
