/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2005-2023 Kim Woelders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "config.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void           *
Ememdup(const void *ptr, unsigned int len)
{
    void           *ret;

    if (len == 0)
        return NULL;

    ret = malloc(len);
    if (!ret)
        return 0;
    memcpy(ret, ptr, len);

    return ret;
}

__EXPORT__ void
EfreeNull(void **p)
{
    Efree(*p);
    *p = NULL;
}

void
EfreeSet(void **p, void *s)
{
    Efree(*p);
    *p = s;
}

void
EfreeDup(char **p, const char *s)
{
    Efree(*p);
    *p = Estrdup(s);
}

char           *
Estrdup(const char *s)
{
    if (!s)
        return NULL;
#if USE_LIBC_STRDUP
    return strdup(s);
#else
    return Ememdup(s, strlen(s) + 1);
#endif
}

char           *
Estrndup(const char *s, size_t n)
{
    if (!s)
        return NULL;
#if USE_LIBC_STRNDUP
    return strndup(s, n);
#else
    char           *ss;

    ss = Ememdup(s, n + 1);
    ss[n] = '\0';
    return ss;
#endif
}

char           *
Estrdupcat2(char *ss, const char *s1, const char *s2)
{
    char           *s;
    int             len, l1, l2;

    if (!ss)
        return Estrdup(s2);

    len = strlen(ss);
    l1 = (s1) ? strlen(s1) : 0;
    l2 = (s2) ? strlen(s2) : 0;

    s = EREALLOC(char, ss, len + l1 + l2 + 1);
    if (!s)
        return NULL;
    if (s1 && l1)
        memcpy(s + len, s1, l1);
    if (s2 && l2)
        memcpy(s + len + l1, s2, l2);
    s[len + l1 + l2] = '\0';

    return s;
}

void
Esetenv(const char *name, const char *value)
{
    if (value)
    {
#if HAVE_SETENV
        setenv(name, value, 1);
#else
        char            buf[4096];

        Esnprintf(buf, sizeof(buf), "%s=%s", name, value);
        putenv(Estrdup(buf));
#endif
    }
    else
    {
#if HAVE_UNSETENV
        unsetenv(name);
#else
        if (getenv(name))
            putenv((char *)name);
#endif
    }
}
