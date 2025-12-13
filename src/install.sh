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

# For test_cpp_standard.sh
chmod +x scripts/*.sh

# The rcc cache directory
rcc_cache_dir="$HOME/.cache/rcc"

# The compiler to use to both compile the rcc and use inside rcc
#! The clang++ compiler uses a different strategy for Pre-Compiled Header.
#! So use g++ for now.
compiler="g++"

echo "${YELLOW}Using compiler:${NORMAL} ${UNDERLINE}$compiler${NORMAL}"

# Get C++ highest standard
echo "${YELLOW}Testing C++ standard support of${NORMAL} ${UNDERLINE}$compiler${NORMAL}"
cpp_std=$(scripts/test_cpp_standard.sh "$compiler") || exit 1
echo "${YELLOW}Found C++ standard:${NORMAL} ${UNDERLINE}$cpp_std${NORMAL}"

echo "${YELLOW}Cache directory:${NORMAL} ${UNDERLINE}$rcc_cache_dir${NORMAL}"
echo ""

# Build rcc
echo "${YELLOW}Building rcc with${NORMAL} ${UNDERLINE}$compiler${NORMAL} and ${UNDERLINE}$cpp_std${NORMAL}"
make clean
check_error "make clean"
make release "CPP_COMPILER=$compiler" "CPP_STD=$cpp_std" "RCC_CACHE_DIR=$rcc_cache_dir"
check_error "make release"

# Copy rcc to path
echo "${YELLOW}Installing rcc to /usr/local/bin/${NORMAL}"
sudo cp rcc /usr/local/bin/
check_error "sudo cp rcc /usr/local/bin/"

# Create rcc cache dir
echo "${YELLOW}Creating cache directory \"$rcc_cache_dir\"${NORMAL}"
mkdir -p "$rcc_cache_dir"
check_error "mkdir -p $rcc_cache_dir"

# Build Pre-Compiled Header
echo "${YELLOW}Building Pre-Compiled Header${NORMAL}"
cd template || exit 1
make clean
check_error "make clean"
make "CPP_COMPILER=$compiler" "CPP_STD=$cpp_std"
check_error "make"
cd ..

# Copy templates and PCH to rcc cache dir
echo "${YELLOW}Copying templates and PCH to cache directory${NORMAL}"
cp -r template/*.cpp template/*.h template/*.gch -t "$rcc_cache_dir"

echo ""
echo "${GREEN}${UNDERLINE}INSTALLATION COMPLETE!${NORMAL}"
