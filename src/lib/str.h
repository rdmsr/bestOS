#ifndef LIB_STR_H
#define LIB_STR_H
#include <lib/base.h>

size_t strlen(const char *str);

bool str_eq(const char *s1, const char *s2);

char *strstr(char *str, char *substr);
char *strchr(char *str, char c);
char *strrchr(char *str, char c);
char *basename(char *s);
char *dirname(char *path);

char *strtok(char *str, char *delim);
char *strdup(char *s);
char *strcat(char *s, char *s2);
char *strcpy(char *dest, const char *src);

#endif
