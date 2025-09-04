#pragma once

#include <lib/list.h>

unsigned long strlen(const char *str);
unsigned long ustrlen(const unsigned char *str);
int strcmp(const char *s1, const char *s2);
void strcpy(void *dest, const void *source);
char *strcat(char *dest, const char *src);
int ustrcmp(const unsigned char *s1, const unsigned char *s2);
char toupper(char x);
unsigned char utoupper(unsigned char x);

List GetSubstrings(const char *string, char split);