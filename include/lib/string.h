#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    unsigned long strlen(const unsigned char *str);
    int strcmp(const unsigned char *s1, unsigned const char *s2);
    int memcmp(const unsigned char *s1, const unsigned char *s2, unsigned long size);
    unsigned char *trim(const unsigned char *s);
    unsigned char *rtrim(const unsigned char *s);
    unsigned char *ltrim(const unsigned char *s);
    int isspace(const unsigned char s);
    void *memset(void *dest, int value, unsigned long size);
    bool strcontains(const unsigned char *sub, const unsigned char *s);
    unsigned char toupper(unsigned char x);
    unsigned char *strcat(unsigned char *x, unsigned char *y);

#ifdef __cplusplus
}
#endif

#endif