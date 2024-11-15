BRed="\033[1;31m"
BGreen="\033[1;32m"
Color_Off="\033[0m"

if [ ! -d "/media/${USER}/bootfs" ]; then
    echo "${BRed}Raspberry pi microSD wasn't connected. ${Color_Off}"
else
  /bin/cp -rf kernel8.img "/media/${USER}/bootfs"
  echo "${BGreen}Copied to the raspberry pi microSD. ${Color_Off}"
fi