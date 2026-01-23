#!/bin/bash

function setup_color() {
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

            # Use them so that the Bash IDE does not complain about unused variables
            echo "$BOLD $UNDERLINE $STANDOUT $BLACK $RED $GREEN $YELLOW $BLUE $MAGENTA $CYAN $WHITE $NORMAL" > /dev/null
        fi
    fi
}

function check_error() {
    local exitcode=$?
    local message=$1
    local expected=$2
    local exitonerr=$3

    if [ -z "$expected" ]; then
        expected=0
    fi

    if [ $exitcode -ne "$expected" ]; then
        echo -e "  ${RED}[ERROR]${NORMAL}" "$message"
        echo ""
        echo -e "${RED}${UNDERLINE}EXIT CODE:${NORMAL} $exitcode"
        if [ -z "$exitonerr" ] || [ "$exitonerr" = "true" ]; then
            exit 1
        fi
    else
        echo -e "  ${GREEN}[OK]${NORMAL}" "$message"
    fi
}

setup_color
