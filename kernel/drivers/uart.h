#pragma once

void uart_init();
void uart_put(char c);
void uart_puts(const char *str);
char uart_recv();
char uart_character();