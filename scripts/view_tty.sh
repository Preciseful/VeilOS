#!/bin/bash

LOOP=1
FILE_EXISTS=0

if [ -f scripts/setting ]; then
    FILE_EXISTS=1
    ANSWER=$(head scripts/setting)
fi

while [ $LOOP -eq 1 ]; do
    if [ $FILE_EXISTS -eq 0 ]; then
        echo "View terminal output? [Y/n]: "
        read ANSWER
        echo "Save setting? [Y/n]: "
        read SETTING
    fi

    if [[ $ANSWER == "Y" || $ANSWER == "y" || $ANSWER == "" ]]; then
        LOOP=0

        if [[ $SETTING == "Y" || $SETTING == "y" || $SETTING == "" ]]; then
            echo $ANSWER > scripts/setting
        fi

        sudo screen $1 115200

    elif [[ $ANSWER == "N" || $ANSWER == "n" ]]; then
        LOOP=0

        if [[ $SETTING == "Y" || $SETTING == "y" || $SETTING == "" ]]; then
            echo $ANSWER > scripts/setting
        fi

        exit 0
    fi
done