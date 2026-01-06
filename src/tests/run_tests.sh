#!/bin/bash

function setup(){
    # Check if output is tty
    if [ -t 1 ]; then
        local ncolors
        ncolors=$(tput colors)

        # Check if terminal supports colors
        if [ -n "$ncolors" ] && [ "$ncolors" -ge 8 ]; then
            BOLD="$(tput bold)"
            UNDERLINE="$(tput smul)"
            STANDOUT="$(tput smso)"
            NORMAL="$(tput sgr0)"
            BLACK="$(tput setaf 0)"
            RED="$(tput setaf 1)"
            GREEN="$(tput setaf 2)"
            YELLOW="$(tput setaf 3)"
            BLUE="$(tput setaf 4)"
            MAGENTA="$(tput setaf 5)"
            CYAN="$(tput setaf 6)"
            WHITE="$(tput setaf 7)"
        fi
    fi
}

function check_error() {
    local exitcode=$?
    local expected=$2

    if [ -z "$expected" ]; then
        expected=0
    fi

    if [ $exitcode -ne "$expected" ]; then
        echo -e "  ${RED}[ERROR]${NORMAL}" "$1"
        echo ""
        echo -e "${RED}${UNDERLINE}EXIT CODE:${NORMAL} $exitcode"
        exit 1
    else
        echo -e "  ${GREEN}[OK]${NORMAL}" "$1"
    fi
}

################################################################################

setup

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
