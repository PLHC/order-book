#!/bin/bash
#
clear

# Function to clean up child processes on SIGINT (Ctrl+C)
cleanup() {
    echo ""
    echo "Caught SIGINT, terminating all child processes..."
    # Send SIGTERM to all child processes
    pkill -P $$    # Kill all child processes of this script's process
    wait           # Wait for all child processes to finish
    echo "All processes have been terminated"
}

# Trap Ctrl+C (SIGINT) and call the cleanup function
trap cleanup SIGINT

# Start multiple programs in parallel
./cmake-build-debug/trading_platform A B C &
./cmake-build-debug/random_order_generator_client 1 A 80 B 60 C 100 &
./cmake-build-debug/display_client 2 18 A B C

# Wait for all programs to finish
wait

echo "shell done"


#
## Function to clean up all child processes in all tmux panes
#cleanup() {
#    echo "Caught SIGINT, terminating all processes..."
#    tmux kill-session -t $SESSION
#    wait
#    echo "All programs have finished."
#}
#
## Trap Ctrl+C (SIGINT) and call the cleanup function
#trap cleanup SIGINT
#
## Create a new tmux session named "order-book-session"
#SESSION="order-book-session"
#tmux new-session -d -s $SESSION
#
## Pane 0: Run trading_platform in this pane
#tmux send-keys -t $SESSION:0 './cmake-build-debug/trading_platform A B' C-m
#
## Split Pane 0 horizontally (top/bottom split)
#tmux split-window -v -t $SESSION:0
#
## Pane 1: Run random_order_generator_client 1 A in this pane (bottom pane)
#tmux send-keys -t $SESSION:0.1 './cmake-build-debug/random_order_generator_client 1 A 50' C-m
#
## Split Pane 1 vertically (left/right split) for the second client
#tmux split-window -h -t $SESSION:0.1
#
## Pane 2: Run random_order_generator_client 2 B in this pane (right bottom pane)
#tmux send-keys -t $SESSION:0.2 './cmake-build-debug/random_order_generator_client 2 B 50' C-m
#
## Attach to the session
#tmux attach -t $SESSION