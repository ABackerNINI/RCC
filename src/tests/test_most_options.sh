#!/bin/bash

out=$(rcc --include ../paths.h --compile-with ../paths.cpp --include-all -lm \
    --put-above-main 'using namespace rcc;' --function 'void func(){cout<<"this is a function"<<endl;}' \
    'Path path{"/"}; path.join("abc"); cout<<path.get_path()<<endl; func(); cout<<"argc: "<<argc<<endl;' -- \
    1 2 3 xx "x x")

diff <(
    cat <<EOF
/abc
this is a function
argc: 6
EOF
) <(echo "$out") || exit 1
