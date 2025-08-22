#!/bin/bash

BRed="\033[1;31m"
BGreen="\033[1;32m"
Color_Off="\033[0m"

if [ ! -d $1 ]; then
    echo -e "${BRed}Raspberry pi microSD wasn't connected. ${Color_Off}"
else
  /bin/cp -rf kernel8.img $1
  echo -e "${BGreen}Copied to the raspberry pi microSD. ${Color_Off}"
fi