#pragma once

#include <interface/syscall.h>

void UartInit();
void UartPut(char c);
void UartPuts(const char *str);
char UartRecv();
char UartCharacter();
SYSCALL_HANDLER(uart_print);