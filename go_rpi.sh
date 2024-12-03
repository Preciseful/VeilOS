BRed="\033[1;31m"
BGreen="\033[1;32m"
Color_Off="\033[0m"

PATH="/media/${USER}/bootfs"

if [ ! -d $PATH ]; then
    echo "${BRed}Raspberry pi microSD wasn't connected. ${Color_Off}"
else
  /bin/cp -rf kernel8.img $PATH
  echo "${BGreen}Copied to the raspberry pi microSD. ${Color_Off}"
fi