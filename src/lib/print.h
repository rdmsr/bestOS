#ifndef LIB_PRINT_H
#define LIB_PRINT_H
#include <lib/base.h>

void host_print(char *str, size_t s);
char host_getc();

void print(char *str);

#define putc(c) print((char[]){c, 0})

void fmt_str(void (*callback)(char *), char *fmt, va_list lst);

void _log(int line, char *file, char *function, char *fmt, ...);
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define log(FMT, ...) _log(__LINE__, __FILENAME__, (char *)__FUNCTION__, FMT, ##__VA_ARGS__)

#endif