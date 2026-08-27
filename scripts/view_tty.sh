#!/bin/bash

ANSWER=$2

while [ true ]; do
    if [[ $ANSWER == "" ]]; then
        read -p "View terminal output? [Y/n]: " ANSWER
        read -p "Save setting? [Y/n]: " SETTING
    fi

    if [[ $ANSWER == [Yy] ]]; then
        if [[ $SETTING == [Yy] ]]; then
            echo "VIEW_TTY=$ANSWER" >> conf
        fi

        clear

        while [ ! -c "$1" ]; do
            sleep 0.1
        done

        picocom -b 115200 "$1"

        exit 0

    elif [[ $ANSWER == [Nn] ]]; then
        if [[ $SETTING == [Yy] ]]; then
            echo "VIEW_TTY=$ANSWER" >> conf
        fi

        exit 0
    fi
done