#!/bin/bash

rcc_cache_dir="$HOME/.cache/rcc"

# For test_cpp_standard.sh
chmod +x scripts/*.sh

# Get C++ highest standard
cpp_std=`scripts/test_cpp_standard.sh` || exit 1

# Build rcc
make rebuild "CPP_STD=$cpp_std" "RCC_CACHE_DIR=$rcc_cache_dir" || exit 1

# Copy rcc to path
sudo cp rcc /usr/local/bin/ || exit 1

# Make or Clear rcc cache dir
if [ -d "$rcc_cache_dir" ]; then
    rm -rf "$rcc_cache_dir"
    mkdir -p "$rcc_cache_dir"
else
    mkdir -p "$rcc_cache_dir"
fi

# Build Pre-Compiled Header
cd template
make rebuild "CPP_STD=$cpp_std"
cd ..

# Copy templates to rcc cache dir
cp -r template/*.cpp template/*.h template/*.gch -t "$rcc_cache_dir"
