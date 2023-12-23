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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <X11/Xlib.h>
#include "E.h"

#if USE_LINEEDIT_EDITLINE
#define USE_LINEEDIT 1
#include <editline.h>
#elif USE_LINEEDIT_READLINE
#define USE_LINEEDIT 1
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define PROMPT "eesh> "

Display        *disp;

static Client  *e;
static bool     use_prompt;
static bool     input_pending;
static bool     reply_pending;

static void
process_line(char *line)
{
    input_pending = false;

    if (!line)
        exit(0);

    if (*line)
    {
        CommsSend(e, line);
        XSync(disp, False);
        reply_pending = true;
    }

#if USE_LINEEDIT
    if (!use_prompt)
        return;
    rl_set_prompt("");
    add_history(line);
#endif
}

static void
stdin_read(void)
{
    static char     buf[10240];
    static int      nin;        // Bytes in buffer
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

#if USE_LINEEDIT
static void
stdin_state_setup(void)
{
    rl_callback_handler_install("", process_line);
}

static void
stdin_state_restore(void)
{
    rl_callback_handler_remove();
}
#endif

int
main(int argc, char **argv)
{
    XEvent          ev;
    Window          my_win, comms_win;
    Client         *me;
    int             rc, i;
    struct pollfd   pfd[2];
    int             nfd, timeout;
    char           *command, *s;
    int             len, l;
    const char     *space;
    bool            interactive;

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

    input_pending = false;
    reply_pending = false;

    if (command)
    {
        /* Non-interactive */
        interactive = use_prompt = false;
        CommsSend(e, command);
        XSync(disp, False);
        reply_pending = true;
    }
    else
    {
        /* Interactive */
        interactive = true;
        use_prompt = isatty(STDIN_FILENO);
#if USE_LINEEDIT
        if (use_prompt)
        {
            stdin_state_setup();
            atexit(stdin_state_restore);
        }
#endif
    }

    memset(pfd, 0, sizeof(pfd));
    pfd[0].fd = ConnectionNumber(disp);
    pfd[0].events = POLLIN;
    pfd[1].fd = STDIN_FILENO;
    pfd[1].events = POLLIN;
    nfd = command ? 1 : 2;

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
                reply_pending = false;
                if (*s)
                {
                    printf("%s", s);
                    fflush(stdout);
                }
                Efree(s);
                if (!interactive)
                    goto done;
                break;
            case DestroyNotify:
                goto done;
            }
        }

        if (use_prompt && !input_pending && !reply_pending)
        {
#if USE_LINEEDIT
            rl_set_prompt(PROMPT);
            rl_forced_update_display();
#else
            printf(PROMPT);
            fflush(stdout);
#endif
        }

        timeout = reply_pending ? 10 : -1;

        rc = poll(pfd, nfd, timeout);
        if (rc < 0)
            break;

        if (rc == 0)
        {
            reply_pending = false;
            continue;
        }

        if (pfd[1].revents)
        {
            if (pfd[1].revents & POLLIN)
            {
                input_pending = true;
#if USE_LINEEDIT
                if (use_prompt)
                    rl_callback_read_char();
                else
#endif
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
