#ifndef STRING
#define STRING

void memcpy(void *dst, const void *src, unsigned long size)
{
    for (unsigned long i = 0; i < size; i++)
        ((char *)dst)[i] = ((char *)src)[i];
}

void *memset(void *dest, int value, unsigned long size)
{
    unsigned char *ptr = (unsigned char *)dest;
    while (size--)
    {
        *ptr++ = (unsigned char)value;
    }
    return dest;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }

    return (*s1 > *s2) - (*s2 > *s1);
}

unsigned long strlen(const char *str)
{
    unsigned long length = 0;
    while (str[length] != '\0')
        length++;

    return length;
}

int atoi(const char *s)
{
    int sign = 1, res = 0, idx = 0;
    // Ignore leading whitespaces
    while (s[idx] == ' ')
    {
        idx++;
    }
    // Store the sign of number
    if (s[idx] == '-' || s[idx] == '+')
    {
        if (s[idx++] == '-')
        {
            sign = -1;
        }
    }
    // Construct the number digit by digit
    while (s[idx] >= '0' && s[idx] <= '9')
        res = 10 * res + (s[idx++] - '0');

    return res * sign;
}

#endif