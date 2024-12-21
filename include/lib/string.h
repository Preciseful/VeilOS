#ifndef STRING_H
#define STRING_H

#ifdef __cplusplus
extern "C"
{
#endif

    unsigned long strlen(const unsigned char *str);
    int strcmp(const unsigned char *s1, unsigned const char *s2);

#ifdef __cplusplus
}
#endif

#endif