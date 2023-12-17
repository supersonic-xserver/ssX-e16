/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2023 Kim Woelders
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <X11/Xlib.h>
#include "E.h"

Display        *disp;

static char     buf[10240];
static int      nin;            // Bytes in buffer
static Client  *e;

static void
process_line(char *line)
{
    if (!line)
        exit(0);
    if (*line == '\0')
        return;

    CommsSend(e, line);
    XSync(disp, False);
}

static void
stdin_state_setup(void)
{
}

static void
stdin_state_restore(void)
{
}

static void
stdin_read(void)
{
    int             nr;
    char           *p;

    nr = read(STDIN_FILENO, buf + nin, sizeof(buf) - 1 - nin);
    if (nr <= 0)
        exit(0);
    nin += nr;
    buf[nin] = '\0';

    for (;;)
    {
        p = strchr(buf, '\n');
        if (!p)
            break;

        *p++ = '\0';            // Terminate line at \n

        process_line(buf);
        if (*p == '\0')
        {
            nin = 0;            // No more input
            break;
        }

        // Move remaining data to start of buffer
        nr = p - buf;
        nin -= nr;
        memmove(buf, buf + nr, nin);
    }
}

int
main(int argc, char **argv)
{
    XEvent          ev;
    Window          my_win, comms_win;
    Client         *me;
    int             i;
    struct pollfd   pfd[2];
    int             nfd, timeout;
    char           *command, *s;
    char            mode;
    int             len, l;
    const char     *space;

    mode = 0;

    for (i = 1; i < argc; i++)
    {
        s = argv[i];
        if (*s != '-')
            break;

        if (!strcmp(argv[i], "-h") ||
            !strcmp(argv[i], "-help") || !strcmp(argv[i], "--help"))
        {
            printf
                ("eesh sends commands to E\n\n"
                 "Examples:\n"
                 "  eesh window_list all           : Show window list\n"
                 "  eesh win_op Pager-0 move 10 20 : Move Pager-0 to position 10,20\n\n");
            printf("Use eesh by itself to enter the \"interactive mode\"\n"
                   "  Ctrl-D will exit interactive mode\n"
                   "  Use \"help\" from inside interactive mode for further assistance\n");
            exit(0);
        }
    }

    disp = XOpenDisplay(NULL);
    if (!disp)
    {
        fprintf(stderr, "Failed to connect to X server\n");
        exit(1);
    }

    CommsInit();
    comms_win = CommsFindCommsWindow();
    my_win = CommsSetup(comms_win);

    e = ClientCreate(comms_win);
    me = ClientCreate(my_win);

    CommsSend(e, "set clientname eesh");
    CommsSend(e, "set version 0.2");

    command = NULL;
    space = "";
    if (i < argc)
    {
        mode = 1;
        len = 0;
        for (; i < argc; i++)
        {
            s = argv[i];
            l = strlen(s);
            command = EREALLOC(char, command, len + l + 4);

            if (strchr(s, ' '))
                l = snprintf(command + len, l + 4, "%s'%s'", space, s);
            else
                l = snprintf(command + len, l + 4, "%s%s", space, s);
            len += l;
            space = " ";
        }
    }

    if (command)
    {
        /* Non-interactive */
        CommsSend(e, command);
        XSync(disp, False);
#if 0                           /* No - Wait for ack */
        if (mode <= 0)
            goto done;
#endif
    }
    else
    {
        /* Interactive */
        stdin_state_setup();
        atexit(stdin_state_restore);
    }

    memset(pfd, 0, sizeof(pfd));
    pfd[0].fd = ConnectionNumber(disp);
    pfd[0].events = POLLIN;
    pfd[1].fd = STDIN_FILENO;
    pfd[1].events = POLLIN;
    nfd = command ? 1 : 2;
    timeout = -1;

    for (;;)
    {
        XSync(disp, False);
        while (XPending(disp))
        {
            XNextEvent(disp, &ev);
            switch (ev.type)
            {
            case ClientMessage:
                s = CommsGet(me, &ev);
                if (!s)
                    break;
                if (*s)
                    printf("%s", s);
                fflush(stdout);
                Efree(s);
                if (mode)
                    goto done;
                break;
            case DestroyNotify:
                goto done;
            }
        }

        if (poll(pfd, nfd, timeout) < 0)
            break;

        if (pfd[1].revents)
        {
            if (pfd[1].revents & POLLIN)
            {
                stdin_read();
            }
            if (pfd[1].revents & POLLHUP)
                break;
        }
    }

  done:
    ClientDestroy(e);
    ClientDestroy(me);
    free(command);

    return 0;
}
