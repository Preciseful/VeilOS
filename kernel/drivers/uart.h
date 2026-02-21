#pragma once

#include <interface/syscall.h>

#define UART_RECV_NOTIFICATION 0

void UartInit();
void UartNotify();