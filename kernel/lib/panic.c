#include <lib/panic.h>
#include <lib/string.h>
#include <boot/interrupts.h>
#include <lib/printf.h>

[[noreturn]]
void panic(const char *message)
{
    LOG("[!] %s [!]", message);
    while (1)
        ;
}