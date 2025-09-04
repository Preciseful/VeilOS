#pragma once

void UartInit();
void UartPut(char c);
void UartPuts(const char *str);
char UartRecv();
char UartCharacter();
unsigned long UartPortalRead(void *obj, unsigned char *buf, unsigned long length);
unsigned long UartPortalWrite(void *obj, unsigned char *buf, unsigned long length);