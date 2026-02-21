#pragma once

void ConsoleRead(char *buf, unsigned long length);
char ConsoleRecv();
void ConsoleWrite(const char *buf);
void ConsolePutc(char c);
void ConsoleInit();
void ConsoleDrop();