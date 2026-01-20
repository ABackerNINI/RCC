#!/bin/bash

out=$(rcc --compile-with test_option_compile_with.cpp --include test_option_compile_with.h \
    'rcc::print_string("Hello, World!");' 'rcc::TestClass t; t.test_method();')

diff <(
    cat <<EOF
Hello, World!
Test method called!
EOF
) <(echo "$out") || exit 1
