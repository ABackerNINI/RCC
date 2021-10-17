#!/bin/bash

# For test_cpp_standard.sh
chmod +x ./*.sh

# Get C++ highest standard
cpp_std=`./test_cpp_standard.sh` || exit 1

# Build and run test
cd template
make test "CPP_STD=$cpp_std" && ./test
cd ..
