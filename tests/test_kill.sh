#!/bin/bash

# Enable job control to allow background processes to be killed by signals
# This is needed for the kill command to work properly
set -m

# Test that we can kill a process with SIGINT
timeout 5 rcc 'while(true){}' &
pid=$!
echo "PID: $pid"

sleep 1

echo "Killing process $pid with SIGINT"
kill -SIGINT $pid

# rcc should return non-zero if process was killed by signal
wait $pid
ec=$?
echo "ExitStatus: $ec"
if [ "$ec" -eq 124 ]; then
    echo "timeout, process not killed" && exit 1
fi
