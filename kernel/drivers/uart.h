#pragma once

void UartInit();
void UartPut(char c);
void UartPuts(const char *str);
char UartRecv();
char UartCharacter();
void UartPortalRead(unsigned char *buf, unsigned long length);
void UartPortalWrite(unsigned char *buf, unsigned long length);