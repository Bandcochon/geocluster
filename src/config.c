/*
 * Geoclustering micro service 
 * (c) Régis FLORET 2018
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

#include "config.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <yaml.h>

typedef struct wait_t
{
    char key : 1;
    char value : 1;
    char width : 1;
    char height : 1;
    char north : 1;
    char south : 1;
    char east : 1;
    char west : 1;
    char port : 1;
    char address : 1;
    char excluded : 1;
    char excluded_lat : 1;
    char excluded_lng : 1;
    char excluded_done: 2;
} wait_t;

static void config_get_yaml_key(Config_t *config, wait_t *wait, const char *value);
static void config_get_yaml_value(Config_t *config, wait_t *wait, const char *value);

static Config_t *config_create(void)
{
    Config_t *config = NULL;

    config = (Config_t *)malloc(sizeof(Config_t));
    if (!config)
    {
        perror("An error occured while allocation configuration");
        exit(1);
    }

    config->address = NULL;
    config->port = 0;
    config->width = 0;
    config->height = 0;
    config->north = 0.0;
    config->south = 0.0;
    config->east = 0.0;
    config->west = 0.0;
    config->excluded.lat = 0.0;
    config->excluded.lng = 0.0;

    return config;
}

Config_t *config_read(char *config_path)
{
    FILE *input_file = NULL;
    Config_t *config = NULL;
    wait_t wait;
    yaml_parser_t parser;
    yaml_token_t token;
    int done = 0;

    file_ensure_exists(config_path);

    config = config_create();
    memset(&parser, 0, sizeof(yaml_parser_t));
    memset(&token, 0, sizeof(yaml_token_t));
    memset(&wait, 0, sizeof(wait_t));

    yaml_parser_initialize(&parser);

    input_file = fopen(config_path, "r");

    yaml_parser_set_input_file(&parser, input_file);

    do
    {
        yaml_parser_scan(&parser, &token);

        switch (token.type)
        {
        case YAML_KEY_TOKEN:
            wait.key = 1;
            break;

        case YAML_VALUE_TOKEN:
            wait.value = 1;
            break;

        case YAML_SCALAR_TOKEN:
            if (wait.key)
            {
                config_get_yaml_key(config, &wait, token.data.scalar.value);
            }
            else if (wait.value)
            {
                config_get_yaml_value(config, &wait, token.data.scalar.value);
            }
            break;
        }
    } while (token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);

    fclose(input_file);

    return config;
}

void config_dispose(Config_t *config)
{
    if (config)
    {
        if (config->address)
        {
            free(config->address);
        }
        free(config);
    }
}

static void config_get_yaml_key(Config_t *config, wait_t *wait, const char *value)
{
    wait->key = 0;

    if (!strcmp(value, "width"))
    {
        wait->width = 1;
    }
    else if (!strcmp(value, "height"))
    {
        wait->height = 1;
    }
    else if (!strcmp(value, "north"))
    {
        wait->north = 1;
    }
    else if (!strcmp(value, "south"))
    {
        wait->south = 1;
    }
    else if (!strcmp(value, "east"))
    {
        wait->east = 1;
    }
    else if (!strcmp(value, "west"))
    {
        wait->west = 1;
    }
    else if (!strcmp(value, "address"))
    {
        wait->address = 1;
    }
    else if (!strcmp(value, "port"))
    {
        wait->port = 1;
    }
    else if (!strcmp(value, "excluded"))
    {
        wait->excluded = 1;
    }
    else if (!strcmp(value, "lat") && wait->excluded)
    {
        wait->excluded_lat = 1;
    }
    else if (!strcmp(value, "lng") && wait->excluded)
    {
        wait->excluded_lng = 1;
    }
}

static void config_get_yaml_value(Config_t *config, wait_t *wait, const char *value)
{
    wait->value = 0;

    if (wait->width)
    {
        wait->width = 0;
        config->width = atoi(value);
    }
    else if (wait->height)
    {
        wait->height = 0;
        config->height = atoi(value);
    }
    else if (wait->north)
    {
        wait->north = 0;
        config->north = atof(value);
    }
    else if (wait->south)
    {
        wait->south = 0;
        config->south = atof(value);
    }
    else if (wait->east)
    {
        wait->east = 0;
        config->east = atof(value);
    }
    else if (wait->west)
    {
        wait->west = 0;
        config->west = atof(value);
    }
    else if (wait->address)
    {
        wait->address = 0;
        config->address = strdup(value);
    }
    else if (wait->port)
    {
        wait->port = 0;
        config->port = atoi(value);
    }
    else if (wait->excluded)
    {
        if (wait->excluded_lng)
        {
            wait->excluded_done++;
            wait->excluded_lng = 0;
            config->excluded.lng = atof(value);
        }
        else if (wait->excluded_lat)
        {
            wait->excluded_done++;
            wait->excluded_lat = 0;
            config->excluded.lat = atof(value);
        }

        if (wait->excluded_done == 2) {
            wait->excluded = 0;
            wait->excluded_done = 0;
        }
    }
}