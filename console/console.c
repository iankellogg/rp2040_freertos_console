// Copyright 2016-2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "console/console.h"
#include "console/linenoise.h"
#include "console/argtable3.h"
#include "sys/queue.h"
#include <errno.h>

#define ANSI_COLOR_DEFAULT      39      /** Default foreground color */

typedef struct cmd_item_ {
    /**
     * Command name (statically allocated by application)
     */
    const char *command;
    /**
     * Help text (statically allocated by application), may be NULL.
     */
    const char *help;
    /**
     * H text, usually lists possible arguments, dynamically allocated.
     * May be NULL.
     */
    char *hint;
    console_cmd_func_t func;    //!< poer to the command handler
    void *argtable;                 //!< optional poer to arg table
    SLIST_ENTRY(cmd_item_) next;    //!< next command in the list
} cmd_item_t;

/** linked list of command structures */
static SLIST_HEAD(cmd_list_, cmd_item_) s_cmd_list;

/** run-time configuration options */
static console_config_t s_config;

/** temporary buffer used for command line parsing */
static char *s_tmp_line_buf;

static const cmd_item_t *find_command_by_name(const char *name);

int console_init(const console_config_t *config)
{
    if (s_tmp_line_buf) {
        return ENOTRECOVERABLE ;
    }
    memcpy(&s_config, config, sizeof(s_config));
    if (s_config.hint_color == 0) {
        s_config.hint_color = ANSI_COLOR_DEFAULT;
    }
    s_tmp_line_buf = calloc(config->max_cmdline_length, 1);
    if (s_tmp_line_buf == NULL) {
        return ENOMEM;
    }
    return 0;
}

int console_deinit()
{
    if (!s_tmp_line_buf) {
        return ENOTRECOVERABLE ;
    }
    free(s_tmp_line_buf);
    cmd_item_t *it, *tmp;
    SLIST_FOREACH_SAFE(it, &s_cmd_list, next, tmp) {
        free(it->hint);
        free(it);
    }
    return 0;
}

int console_cmd_register(const console_cmd_t *cmd)
{

    cmd_item_t *item = (cmd_item_t *) calloc(1, sizeof(*item));
    if (item == NULL) {
        return ENOMEM;
    }
    if (cmd->command == NULL) {
        free(item);
        return EINVAL;
    }
    if (strchr(cmd->command, ' ') != NULL) {
        free(item);
        return EINVAL;
    }
    item->command = cmd->command;
    item->help = cmd->help;
    if (cmd->hint) {
        /* Prepend a space before the hint. It separates command name and
         * the hint. arg_print_syntax below adds this space as well.
         */
        int unused __attribute__((unused));
        unused = asprintf(&item->hint, " %s", cmd->hint);
    } else if (cmd->argtable) {
        /* Generate hint based on cmd->argtable */
        char *buf = NULL;
        size_t buf_size = 0;
        FILE *f = open_memstream(&buf, &buf_size);
    	//char *buf = malloc(80);
    	//FILE *f = fmemopen(buf,80,"w");
        if (f != NULL) {
            arg_print_syntax(f, cmd->argtable, NULL);
            fclose(f);
        }
        item->hint = buf;
    }
    item->argtable = cmd->argtable;
    item->func = cmd->func;
    cmd_item_t *last = SLIST_FIRST(&s_cmd_list);
    if (last == NULL) {
        SLIST_INSERT_HEAD(&s_cmd_list, item, next);
    } else {
        cmd_item_t *it;
        while ((it = SLIST_NEXT(last, next)) != NULL) {
            last = it;
        }
        SLIST_INSERT_AFTER(last, item, next);
    }
    return 0;
}

void console_get_completion(const char *buf, linenoiseCompletions *lc)
{
    size_t len = strlen(buf);
    if (len == 0) {
        return;
    }
    cmd_item_t *it;
    SLIST_FOREACH(it, &s_cmd_list, next) {
        /* Check if command starts with buf */
        if (strncmp(buf, it->command, len) == 0) {
            linenoiseAddCompletion(lc, it->command);
        }
    }
}

const char *console_get_hint(const char *buf, int *color, int *bold)
{
    size_t len = strlen(buf);
    cmd_item_t *it;
    SLIST_FOREACH(it, &s_cmd_list, next) {
        if (strlen(it->command) == len &&
                strncmp(buf, it->command, len) == 0) {
            *color = s_config.hint_color;
            *bold = s_config.hint_bold;
            return it->hint;
        }
    }
    return NULL;
}

static const cmd_item_t *find_command_by_name(const char *name)
{
    const cmd_item_t *cmd = NULL;
    cmd_item_t *it;
    SLIST_FOREACH(it, &s_cmd_list, next) {
        if (strcmp(name, it->command) == 0) {
            cmd = it;
            break;
        }
    }
    return cmd;
}

int console_run(const char *cmdline, int *cmd_ret)
{
    if (s_tmp_line_buf == NULL) {
        return ENOTRECOVERABLE ;
    }
    char **argv = (char **) calloc(s_config.max_cmdline_args, sizeof(char *));
    if (argv == NULL) {
        return ENOMEM;
    }
    strlcpy(s_tmp_line_buf, cmdline, s_config.max_cmdline_length);

    size_t argc = console_split_argv(s_tmp_line_buf, argv,
                                         s_config.max_cmdline_args);
    if (argc == 0) {
        free(argv);
        return ENOMEM;
    }
    const cmd_item_t *cmd = find_command_by_name(argv[0]);
    if (cmd == NULL) {
        free(argv);
        return ENOENT;
    }
    *cmd_ret = (*cmd->func)(argc, argv);
    free(argv);
    return 0;
}

static int help_command(int argc, char **argv)
{
    cmd_item_t *it;

    /* Print summary of each command */
    SLIST_FOREACH(it, &s_cmd_list, next) {
        if (it->help == NULL) {
            continue;
        }
        /* First line: command name and hint
         * Pad all the hints to the same column
         */
        const char *hint = (it->hint) ? it->hint : "";
        printf("%-s %s\n", it->command, hint);
        /* Second line: print help.
         * Argtable has a nice helper function for this which does line
         * wrapping.
         */
        printf("  "); // arg_print_formatted does not indent the first line
        arg_print_formatted(stdout, 2, 78, it->help);
        /* Finally, print the list of arguments */
        if (it->argtable) {
            arg_print_glossary(stdout, (void **) it->argtable, "  %12s  %s\n");
        }
        printf("\n");
    }
    return 0;
}


int console_register_help_command()
{
    console_cmd_t command = {
        .command = "help",
        .help = "Pr the list of registered commands",
        .func = &help_command
    };
    return console_cmd_register(&command);
}
