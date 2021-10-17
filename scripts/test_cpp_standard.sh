# Test and print the g++ compiler's highest supported C++ standard.
# If none of these standards are supported, exit with 1.

#!/bin/bash

# Rcc uses c++11 functionalities, so the least standards should be c++11.
standards="c++17 c++14 c++11"

for standard in $standards; do
    g++ "-std=$standard" rcc.cpp -o rcc 2>/dev/null
    if [ $? -eq 0 ]; then
        rm -f rcc
        echo "$standard"
        exit 0
    fi
done

echo "Current g++ does not support one of the standards \"$standards\""
echo "Please install a higher version of g++"
exit 1
