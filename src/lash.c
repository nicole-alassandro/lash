// Copyright (C) 2021  Nicole Alassandro

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma clang diagnostic ignored "-Wunused-label"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lash_constants.c"
#include "lash_command.c"
#include "lash_builtins.c"

static inline
int lash_exec(
    const struct lash_command * const cmd)
{
    assert(cmd);
    assert(cmd->buff);
    assert(cmd->argc);
    assert(cmd->argv);

builtin_fn:
    {
        for (size_t idx = 0; idx < LASH_NUM_BUILTIN; ++idx)
            if (strcmp(cmd->argv[0], LASH_BUILTIN_NAMES[idx]) == 0)
                return LASH_BUILTIN_FUNCS[idx](cmd);
    }

extern_fn:
    {
        const pid_t pid = fork();

        if (pid < 0)
        {
            printf("Failed to fork process\n");
            return -1;
        }

        if (pid == 0)
        {
            if (execvp(cmd->argv[0], cmd->argv + 1) == -1)
                printf("%s\n", strerror(errno));

            exit(EXIT_FAILURE);
        }
        else
        {
            int status;
            pid_t wpid;

            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }

    return 0;
}

int main(void)
{
    signal(SIGINT,  &exit);
    signal(SIGTERM, &exit);

    do
    {
        printf("# ");
        fflush(stdout);

        struct lash_command cmd = {NULL};

        const int error = lash_command_alloc(&cmd);

        if (error == -1)
            continue;

        if (error == 1)
            break;

        lash_exec(&cmd);
        lash_command_free(&cmd);
    }
    while (true);

    return EXIT_SUCCESS;
}
