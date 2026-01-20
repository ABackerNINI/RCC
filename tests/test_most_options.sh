#!/bin/bash

out=$(rcc --include test_option_compile_with.h --compile-with test_option_compile_with.cpp --include-all -lm \
    --put-above-main 'using namespace rcc;' --function 'void func(){cout<<"this is a function"<<endl;}' \
    'TestClass t; t.test_method();' 'func(); cout<<"argc: "<<argc<<endl;' -- \
    1 2 3 xx "x x")

diff <(
    cat <<EOF
Test method called!
this is a function
argc: 6
EOF
) <(echo "$out") || exit 1
