#!/bin/bash

out=$(rcc --function 'int test_function(string str){return (int)str.size();}' \
    'cout << test_function("Hello, World!") << endl;')

diff <(
    cat <<EOF
13
EOF
) <(echo "$out") || exit 1

out=$(rcc --function 'int test_function1(string str){return (int)str.size();}' \
    --function 'int test_function2(string str){return (int)str.size();}' \
    'cout << test_function1("Hello, ") << test_function2("World!") << endl;')

diff <(
    cat <<EOF
76
EOF
) <(echo "$out") || exit 1

