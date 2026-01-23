#!/bin/bash

rcc --clean-cache

rcc --g++ 'cout<<"test pch usage for g++ from test_pch_usage.sh"<<endl;' -H >out.txt 2>err.txt
err=$(cat err.txt) || exit 1

# g++ sample output for using PCH:
# ! xxx/.cache/rcc/templates/rcc_template.hpp.gch/g++.c++17.53ca0b095dd1.default.gch
if echo "$err" | grep -E '^! .*\.gch$' >/dev/null 2>&1; then
    rm out.txt err.txt
    echo "PCH was used for g++"
else
    rm out.txt err.txt
    echo "It seems that PCH was not used for g++"
    exit 1
fi

# clang++ does not output if PCH is used, however it output the following if PCH is not used:
# . xxx/.cache/rcc/templates/rcc_template.hpp
rcc --clang++ 'cout<<"test pch usage for clang++ from test_pch_usage.sh"<<endl;' -H >out.txt 2>err.txt
err=$(cat err.txt) || exit 1

if echo "$err" | grep -E '^\. .*/rcc_template\.hpp$' >/dev/null 2>&1; then
    rm out.txt err.txt
    echo "It seems that PCH was not used for clang++"
    exit 1
else
    rm out.txt err.txt
    echo "PCH was used for clang++"
fi
