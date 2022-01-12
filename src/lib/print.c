#include <lib/print.h>
#include <lib/str.h>
#include <stdarg.h>

MAYBE_UNUSED static char *string_convert(unsigned int num, int base)
{
    static char representation[] = "0123456789ABCDEF";
    static char buffer[50];
    char *ptr;

    ptr = &buffer[49];
    *ptr = '\0';

    do
    {
        *--ptr = representation[num % base];
        num /= base;
    } while (num != 0);
    return (ptr);
}

void fmt_str(void (*callback)(char *), char *fmt, va_list lst)
{
    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;

            switch (*fmt)
            {
            case 'c':
            {
                char c = va_arg(lst, int);

                callback((char[]){c, 0});
                break;
            }

            case 's':
            {
                callback(va_arg(lst, char *));
                break;
            }

            case 'd':
            {
                callback(string_convert(va_arg(lst, int), 10));
                break;
            }

            case 'x':
            {
                callback(string_convert(va_arg(lst, int), 16));
                break;
            }
            }
        }

        else
        {
            callback((char[]){*fmt, 0});
        }

        fmt++;
    }
}

void printf(char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    fmt_str(print, fmt, arg);

    va_end(arg);
}

void _log(int line, char *file, char *function, char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    printf("\033[1m%s:%d \033[1;35m%s \033[0m", file, line, function);
    fmt_str(print, fmt, arg);

    print("\n");

    va_end(arg);
}
void print(char *str)
{
    host_print(str, strlen(str));
}