#!/bin/bash
#
clear

# Function to clean up child processes on SIGINT (Ctrl+C)
cleanup() {
    echo ""
    echo "Caught SIGINT, terminating all child processes"
    # Send SIGTERM to all child processes
    pkill -P $$    # Kill all child processes of this script's process
    wait           # Wait for all child processes to finish
    echo "All processes have been terminated"
}

# Trap Ctrl+C (SIGINT) and call the cleanup function
trap cleanup SIGINT

# Start multiple programs in parallel
./cmake-build-release/trading_platform A B C &
./cmake-build-release/random_order_generator_client random A 80 B 60 C 100 &
./cmake-build-release/display_client display 15 A B C &

# Wait for all programs to finish
wait

echo "shell done"