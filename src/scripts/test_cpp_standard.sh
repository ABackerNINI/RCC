#!/bin/bash

# Test and print the g++ compiler's highest supported C++ standard.
# If none of these standards are supported, exit with 1.

compiler="$1"

src_file="scripts/test_cpp_standard.cpp"

# Rcc uses c++11 functionalities, so the least standards should be c++11.
standards="c++23 c++20 c++17 c++14 c++11"

for standard in $standards; do
    if $compiler "-std=$standard" $src_file -o scripts/test 2>/dev/null; then
        rm -f scripts/test
        echo "$standard"
        exit 0
    fi
done

echo "Current g++ does not support one of the standards \"$standards\""
echo "Please install a higher version of g++"
exit 1
