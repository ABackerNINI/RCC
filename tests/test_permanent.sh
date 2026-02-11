#!/bin/bash

source utils.sh

# Test permanent options

rcc --permanent test_permanent --desc "This is a test permanent" 'cout<<"This is a test message for test_permanent"<<endl;'
check_error "rcc --permanent"

rcc --run-permanent test_permanent >out.txt 2>err.txt
check_error "rcc --run-permanent"

diff <(echo "This is a test message for test_permanent") out.txt
check_error "checking rcc --run-permanent output"

rcc --list-permanent | grep test_permanent
check_error "rcc --list-permanent"

rcc --remove-permanent test_permanent
check_error "rcc --remove-permanent"

# Test permanent subcommands

rcc list | grep test_permanent
check_error "rcc list after remove" "1" true

rcc create test_permanent --desc "This is a test permanent" 'cout<<"This is a test message for test_permanent"<<endl;'
check_error "rcc create"

rcc run test_permanent >out.txt 2>err.txt
check_error "rcc run"

diff <(echo "This is a test message for test_permanent") out.txt
check_error "checking rcc run output"

rcc list | grep test_permanent
check_error "rcc list"

rcc remove test_permanent
check_error "rcc remove"

rcc list | grep test_permanent
check_error "rcc list after remove" "1" true

# Test some edge cases

# Test --permanent without code
rcc --permanent test_permanent --desc "This is a test permanent" >out.txt 2>err.txt
check_error "rcc --permanent no code" 1

# Test create without code
rcc create test_permanent --desc "This is a test permanent" >out.txt 2>err.txt
check_error "rcc create no code" 1

rm out.txt err.txt
