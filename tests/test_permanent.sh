#!/bin/bash

source utils.sh

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

rm out.txt err.txt
