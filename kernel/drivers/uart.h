#pragma once

#include <system/syscall.h>

#define UART_RECV_NOTIFICATION 0

void UartInit();
void UartNotify();