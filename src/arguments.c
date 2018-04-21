/*
 * Geoclustering micro service 
 * (c) Prince Cuberdon 2018
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its 
 *    contributors may be used to endorse or promote products derived from 
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "arguments.h"
#include "file.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

const char *DefaultConfigFile = "./config.ini";
const int DIR_SIZE = 1024;

Argument_t *argument_create(void)
{
    Argument_t *args = (Argument_t *)malloc(sizeof(Argument_t));
    if (!args)
    {
        log_critical("An error occured while allocating memory for arguments");
        exit(1);
    }

    args->help = 0;
    args->filename = NULL;
    args->config_file = NULL;

    return args;
}

void argument_dispose(Argument_t *args)
{
    if (args)
    {
        if (args->filename)
        {
            free(args->filename);
        }
        if (args->config_file)
        {
            free(args->config_file);
        }
        free(args);
    }
}

Argument_t *argument_check(int argc, char **argv)
{
    Argument_t *args;
    int i;
    uint8_t expect_file = 0, expected_config = 0;

    args = argument_create();

    for (i = 1; i < argc; i++)
    {
        if (expect_file)
        {
            args->filename = strdup(argv[i]);
            expect_file = 0;
            continue;
        }

        if (expected_config)
        {
            args->config_file = strdup(argv[i]);
            expected_config = 0;
            continue;
        }

        if (!strcmp("-h", argv[i]) || !strcmp("--help", argv[i]))
        {
            args->help = 1;
            continue;
        }

        if (!strcmp("-f", argv[i]) || !strcmp("--file", argv[i]))
        {
            expect_file = 1;
            continue;
        }

        if (!strcmp("-c", argv[i]) || !strcmp("--config", argv[i]))
        {
            expected_config = 1;
            continue;
        }

        log_critical("Unknown arguments %s\n", argv[i]);
        exit(1);
    }

    if (!args->config_file)
    {

        char  *config_file = NULL;
        char *current_dir = NULL;

        log_error("Missing configuration file. Try get it from current directory.");

        current_dir = (char *)malloc(sizeof(char *) * DIR_SIZE);

        if (!getcwd(current_dir, DIR_SIZE))
        {
            log_critical("A probleme occured while getting the current directory");
            exit(EXIT_FAILURE);
        }

        config_file = strcat(current_dir, DefaultConfigFile);
        file_ensure_exists(config_file);
        args->config_file = strdup(config_file);
        
        free(current_dir);
    }

    return args;
}
