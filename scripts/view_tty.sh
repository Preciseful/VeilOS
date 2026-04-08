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

        sudo screen $1 115200
        exit 0

    elif [[ $ANSWER == [Nn] ]]; then
        if [[ $SETTING == [Yy] ]]; then
            echo "VIEW_TTY=$ANSWER" >> conf
        fi

        exit 0
    fi
done