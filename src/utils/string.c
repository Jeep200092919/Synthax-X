#include <stddef.h>
#include "utils/string.h"

size_t strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

char *strcopy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++) != '\0') { }
    return dst;
}
