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

typedef int (lash_builtin)(const struct lash_command * const);

#define LASH_COMMAND_LIST \
    X(lash_help, "help")  \
    X(lash_exit, "exit")

const char * const LASH_BUILTIN_NAMES[] = {
    #define X(fn, name) name,
        LASH_COMMAND_LIST
    #undef X
};

enum {
    LASH_NUM_BUILTIN = sizeof(LASH_BUILTIN_NAMES) / sizeof(char*)
};

static
int lash_help(
    const struct lash_command * const cmd)
{
    printf("built-in commands:\n");

    for (size_t i = 0; i < LASH_NUM_BUILTIN; ++i)
        printf("\t%s\n", LASH_BUILTIN_NAMES[i]);

    return 0;
}

static
int lash_exit(
    const struct lash_command * const cmd)
{
    exit(EXIT_SUCCESS);
    return 0;
}

lash_builtin * const LASH_BUILTIN_FUNCS[] = {
    #define X(fn, name) &fn,
        LASH_COMMAND_LIST
    #undef X
};


