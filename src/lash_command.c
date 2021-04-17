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

struct lash_command
{
    char   *  buff;
    size_t    argc;
    char   ** argv;
};

#define lash_isspace(x) (isspace(x) && (*(&x - 1) != '\\'))

static inline
void lash_command_free(
    struct lash_command * const cmd)
{
    assert(cmd);

    free(cmd->buff);
    cmd->buff = NULL;
    cmd->argc = 0;
    cmd->argv = NULL;
}

static inline
int lash_command_alloc(
    struct lash_command * const result)
{
    assert(result);

    result->buff = malloc(sizeof(char) * LASH_DEFAULT_BUF_COUNT);

    size_t compressed_len;
    size_t string_len = 0;
    size_t total_len  = LASH_DEFAULT_BUF_COUNT;

    while (true)
    {
        const size_t chunk = LASH_DEFAULT_BUF_COUNT;
        const size_t nread = read(
            STDIN_FILENO, result->buff + string_len, chunk
        ) / sizeof(char);

        if (!nread)
            goto error_eof;

        string_len += nread;

        if (nread <= chunk)
        {
            result->buff[string_len] = '\0';
            break;
        }

        result->buff = realloc(
            result->buff,
            sizeof(char) * (string_len + chunk)
        );

        total_len += chunk;
    }

    if (string_len <= 1)
        goto error_empty;

delimit_substrings:
    {
        char last = result->buff[0];

        size_t word_len = 0;
        size_t total_word_len = 0;

        for (size_t i = 0; i < string_len; ++i)
        {
            if (lash_isspace(result->buff[i]) && !isspace(last))
            {
                result->argc++;
                last = result->buff[i];
                result->buff[i] = '\0';

                total_word_len += word_len;
                word_len = 0;
            }
            else
            {
                if (!lash_isspace(result->buff[i]))
                    word_len++;

                last = result->buff[i];
            }
        }

        compressed_len = total_word_len + (result->argc - 1) + 1;
    }

compress_string:
    {
        char * read_ptr  = result->buff;
        char * write_ptr = result->buff;
        for (size_t i = 0; i < string_len; ++i)
        {
            if (!lash_isspace(*read_ptr))
            {
                *write_ptr = *read_ptr;
                write_ptr++;
                read_ptr++;
            }
            else
            {
                if (*read_ptr == '\0')
                    write_ptr++;

                read_ptr++;
            }
        }

        assert(write_ptr == (result->buff + compressed_len));
    }

alloc_argv:
    {
        total_len      += alignof(char*) - (compressed_len % alignof(char*));
        compressed_len += alignof(char*) - (compressed_len % alignof(char*));

        total_len += (
            (sizeof(char*) * (result->argc + 1)) - (total_len - compressed_len)
        );

        result->buff = realloc(result->buff, total_len);
        result->argv = (char**)(result->buff + compressed_len);

        size_t argc = 0;
        char   last = result->buff[0];

        result->argv[argc++] = result->buff;

        for (size_t i = 0; i < total_len; ++i)
        {
            if (result->buff[i] != '\0' && last == '\0')
                result->argv[argc++] = &result->buff[i];
            else if (result->buff[i] == '\0' && last == '\0')
                break;

            last = result->buff[i];
        }

        result->argv[argc] = NULL;
    }

    return 0;

error_empty:
    lash_command_free(result);
    return LASH_ERR_EMPTY_BUF;

error_eof:
    lash_command_free(result);
    return LASH_ERR_EOF;
}
