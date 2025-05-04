FILE="kernel8.img"

stty -F /dev/ttyUSB0 115200 cs8 -cstopb -parenb -crtscts
BYTES=$(wc -c < "$FILE")

printf "copykernel %s\n" "$BYTES" > /dev/ttyUSB0
while IFS= read -r -n1 byte; do
    case "$byte" in
        $'\0') printf "\\0" > /dev/ttyUSB0 ;;
        $'\n') printf "\\n" > /dev/ttyUSB0 ;;
        *)     printf "%s" "$byte" > /dev/ttyUSB0 ;;
    esac
done < "$FILE"