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

# Start multiple programs in parallel
./cmake-build-release/trading_platform A B C &
./cmake-build-release/random_order_generator_client random A 80 B 60 C 100 &
./cmake-build-release/display_client display 15 A B C &


# wait 10 seconds before shutting down
sleep 12 # 12 seconds because orders are only sent 2 seconds after the random_order_generator_client started

cleanup
# Wait for all programs to finish
wait

echo "shell done"