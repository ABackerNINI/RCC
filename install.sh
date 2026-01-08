#!/bin/bash

function setup() {
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
            echo "$BOLD $UNDERLINE $STANDOUT $BLACK $RED $GREEN $YELLOW $BLUE $MAGENTA $CYAN $WHITE $NORMAL" >/dev/null
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

set -e # Exit on error

function usage() {
    cat <<EOF
Usage: $0 [OPTION]...

OPTIONS:
    -h, --help          Display this help and exit.
    -d, --debug         Enable debug mode.
    -c, --compiler      Specify the compiler to use.
    -s, --std           Specify the C++ standard to use.

EXAMPLES:
    $0 -d --compiler=g++ --std=c++17
EOF
    exit 0
}

# -o: short option
# --long: long option
# -n: program name
# --: end of options
# "$0": program name
# "$@": all arguments
TEMP=$(getopt -o hdc:s: --long help,debug,compiler:,std: -n "$0" -- "$@")

# Check for errors in argument parsing. If getopt returns a non-zero status, it means there was an error.
# shellcheck disable=SC2181
if [[ $? -ne 0 ]]; then
    echo "Error: there is an error in the argument parsing." >&2
    usage
fi

# Reassign the result of getopt parsing back to "$@", so that these parsed options and arguments can be used in the script.
eval set -- "$TEMP"

DEBUG=false

# The compiler to use to both compile the rcc and use inside rcc
COMPILER="g++"
# COMPILER="clang++"

# The C++ standard to use when compiling the rcc and using inside rcc.
# If not specified, the default is to use the latest standard supported by the compiler.
CPP_STD=""

# The rcc cache directory
CACHE_DIR="$HOME/.cache/rcc"

# Parse the options and arguments
while true; do
    case "$1" in
    -h | --help)
        usage
        ;;
    -d | --debug)
        DEBUG=true
        shift
        ;;
    -c | --compiler)
        COMPILER="$2"
        shift 2
        ;;
    -s | --std)
        CPP_STD="$2"
        shift 2
        ;;
    --)
        shift
        break
        ;;
    *)
        echo "Internal Error!" >&2
        exit 1
        ;;
    esac
done

# The remaining arguments
# EXTRA_ARGS=("$@")

################################################################################

# For test_cpp_standard.sh
chmod +x scripts/*.sh

echo "${YELLOW}Using compiler:${NORMAL} ${UNDERLINE}$COMPILER${NORMAL}"

if [ -z "$CPP_STD" ]; then
    # Get C++ highest standard
    echo "${YELLOW}Testing C++ standard support of${NORMAL} ${UNDERLINE}$COMPILER${NORMAL}"
    CPP_STD=$(scripts/test_cpp_standard.sh "$COMPILER") || exit 1
    echo "${YELLOW}Found C++ standard:${NORMAL} ${UNDERLINE}$CPP_STD${NORMAL}"
fi

echo "${YELLOW}Cache directory:${NORMAL} ${UNDERLINE}$CACHE_DIR${NORMAL}"
echo "${YELLOW}DEBUG:${NORMAL} ${UNDERLINE}$DEBUG${NORMAL}"
echo ""

cd src || exit 1

mkdir -p build
echo -e "COMPILER=$COMPILER\nCPP_STD=$CPP_STD\nRCC_CACHE_DIR=$CACHE_DIR" > build/build_config.txt
if [ -f build/last_build_config.txt ]; then
    if ! diff -q build/build_config.txt build/last_build_config.txt >/dev/null 2>&1; then
        echo -e "${YELLOW}Build configuration changed, cleaning build${NORMAL}"
        make clean
        check_error "make clean"
    fi
fi

# Build rcc
echo "${YELLOW}Building rcc with${NORMAL} ${UNDERLINE}$COMPILER${NORMAL} and ${UNDERLINE}$CPP_STD${NORMAL}"
if [ $DEBUG == true ]; then
    make debug "CPP_COMPILER=$COMPILER" "CPP_STD=$CPP_STD" "RCC_CACHE_DIR=$CACHE_DIR" BUILD_PCH=FALSE
    check_error "make debug"
else
    make release "CPP_COMPILER=$COMPILER" "CPP_STD=$CPP_STD" "RCC_CACHE_DIR=$CACHE_DIR" BUILD_PCH=FALSE
    check_error "make release"
fi

# Copy rcc to path
echo "${YELLOW}Installing rcc to /usr/local/bin/${NORMAL}"
sudo cp rcc /usr/local/bin/
check_error "sudo cp rcc /usr/local/bin/"

# Remove and then create rcc cache dir
if [ -d "$CACHE_DIR" ]; then
    echo "${YELLOW}Removing old cache directory \"$CACHE_DIR\"${NORMAL}"
    rm -rf "$CACHE_DIR/cache"/*
    check_error "rm -r \"$CACHE_DIR/cache\"/*"
    rm -rf "$CACHE_DIR/templates"/*.hpp "$CACHE_DIR/templates"/*.cpp
    check_error "rm -rf \"$CACHE_DIR/templates\"/*.hpp \"$CACHE_DIR/templates\"/*.cpp"
fi
echo "${YELLOW}Creating cache directory \"$CACHE_DIR\"${NORMAL}"
mkdir -p "$CACHE_DIR/cache"
check_error "mkdir -p \"$CACHE_DIR/cache\""
mkdir -p "$CACHE_DIR/templates"
check_error "mkdir -p \"$CACHE_DIR/templates\""

# Copy templates to rcc cache dir
make -C template clean "CPP_COMPILER=$COMPILER"
check_error "make clean"
echo "${YELLOW}Copying templates to cache directory${NORMAL}"
cp -r --preserve=timestamps template/* -t "$CACHE_DIR/templates"
check_error "cp -r template/* -t \"$CACHE_DIR/templates\""
# copy templates header files to the cache sub-directory so that the ide
# can find it when we open one source file in the ide instead of showing lots of errors.
# This is not necessary for the build process but it helps with the ide experience.
#! This might interfere with the PCH matching process, so ...
# cp -r template/*.hpp -t "$CACHE_DIR/cache"
# check_error "cp -r template/*.hpp -t \"$CACHE_DIR/cache\""

# Build Pre-Compiled Header
echo "${YELLOW}Building Pre-Compiled Header${NORMAL}"
make -C "$CACHE_DIR/templates" "CPP_COMPILER=$COMPILER" "CPP_STD=$CPP_STD"
check_error "make"

echo -e "COMPILER=$COMPILER\nCPP_STD=$CPP_STD\nRCC_CACHE_DIR=$CACHE_DIR" > build/last_build_config.txt

echo ""
echo "${GREEN}${UNDERLINE}INSTALLATION COMPLETE!${NORMAL}"
