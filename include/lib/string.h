#ifndef STRING_H
#define STRING_H

#ifdef __cplusplus
extern "C"
{
#endif

    unsigned long strlen(const unsigned char *str);
    int strcmp(const unsigned char *s1, unsigned const char *s2);
    unsigned char *trim(const unsigned char *s);
    unsigned char *rtrim(const unsigned char *s);
    unsigned char *ltrim(const unsigned char *s);
    int isspace(const unsigned char s);
    void *memset(void *dest, int value, unsigned long size);

#ifdef __cplusplus
}
#endif

#endif