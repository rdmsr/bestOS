#include <lib/alloc.h>
#include <lib/str.h>

size_t strlen(const char *str)
{
    size_t length = 0;

    for (length = 0; str[length] != '\0'; ++length)
        ;

    return length;
}

bool str_eq(const char *s1, const char *s2)
{
    if (strlen(s1) != strlen(s2))
        return false;
    for (size_t i = 0; i < strlen(s1); i++)
    {
        if (s1[i] != s2[i])
            return false;
    }
    return true;
}

char *strstr(char *str, char *substr)
{
    char *a = str, *b = substr;
    while (true)
    {
        if (!*b)
            return (char *)str;
        if (!*a)
            return NULL;

        if (*a++ != *b++)
        {
            a = ++str;
            b = substr;
        }
    }
}

char *basename(char *path)
{
    if (path[0] != '/')
        return path;

    if (strlen(path) == 1)
        return path;

    char *ssc = strstr(path, "/");
    int l = 0;

    do
    {
        l = strlen(ssc) + 1;

        path = &path[strlen(path) - l + 2];

        ssc = strstr(path, "/");
    } while (ssc);

    return path;
}

char *strchr(char *s, char c)
{
    while (*s != c)
    {
        if (!*s++)
        {
            return NULL;
        }
    }
    return s;
}

// strtok
char *strtok(char *str, char *delim)
{
    static char *s = NULL;
    char *token;

    if (str != NULL)
        s = str;

    if (s == NULL)
        return NULL;

    token = s;

    while (*s != '\0')
    {
        if (strchr(delim, *s) != NULL)
        {
            *s++ = '\0';
            return token;
        }
        s++;
    }

    s = NULL;
    return token;
}

char *strdup(char *s)
{

    char *str;
    char *p;
    int len = 0;

    while (s[len])
        len++;
    str = malloc(len + 1);
    p = str;
    while (*s)
        *p++ = *s++;
    *p = '\0';
    return str;
}
char *dirname(char *path)
{
    char *dir = malloc(strlen(path) + 1);

    int i = 0;
    int j = 0;

    while (path[i] != '\0')
    {
        if (path[i] == '/')
        {
            j = i;
        }
        i++;
    }

    if (j == 0)
    {
        dir[0] = '/';
        dir[1] = '\0';
        return dir;
    }
    else
    {
        for (int k = 0; k < j; k++)
        {
            dir[k] = path[k];
        }
        dir[j] = '\0';
        return dir;
    }
}

char *strcat(char *s, char *s2)
{
    char *p = s;
    while (*s)
        s++;
    while (*s2)
        *s++ = *s2++;
    *s = '\0';
    return p;
}
