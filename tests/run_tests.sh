#!/bin/bash

source utils.sh

which rcc || { echo "rcc not found"; exit 1; }

tests=$(ls ./*.sh)
for test in $tests; do
    if [ "$test" == "./run_tests.sh" ]; then
        continue
    fi

    echo -e "${YELLOW}TESTING $test${NORMAL}"
    bash "$test"
    check_error "Test $test"
done

echo ""
echo -e "${GREEN}${UNDERLINE}ALL TESTS PASSED!${NORMAL}"
