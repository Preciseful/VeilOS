#!/bin/bash

scripts/go_rpi.sh $1 $2 $3 ""

tmux kill-session -t "veil-debug" 2>/dev/null
tmux new-session -d -s "veil-debug" -n debug

tmux send-keys -t "veil-debug:debug.0" \
    "scripts/view_tty.sh $2 $3" C-m

tmux split-window -v -t "veil-debug:debug.0"
tmux split-window -h -t "veil-debug:debug.1"

tmux send-keys -t "veil-debug:debug.1" \
    'clear; until aarch64-none-elf-gdb -q -batch -ex "target remote :3333" -ex "monitor halt" build/kernel8.elf >/dev/null 2>&1; do sleep 1; done; aarch64-none-elf-gdb -q build/kernel8.elf -ex "target remote :3333" -ex "set \$x1 = 0"' C-m

tmux send-keys -t "veil-debug:debug.2" \
    'until openocd -f scripts/jlink.cfg; do sleep 1; done' C-m

tmux select-pane -t "veil-debug:debug.1"
tmux attach-session -t "veil-debug"