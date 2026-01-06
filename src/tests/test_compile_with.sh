#!/bin/bash

out=$(rcc --compile-with test_compile_with.cpp --include test_compile_with.h \
    'print_string("Hello, World!");' 'TestClass t; t.test_method();')

diff <(
    cat <<EOF
Hello, World!
Test method called!
EOF
) <(echo "$out") || exit 1
