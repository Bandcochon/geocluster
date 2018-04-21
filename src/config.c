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
#include "ini.h"

#include <stdlib.h>
#include <string.h>


static Configuration_t *configuration_create(void)
{
    Configuration_t *config = NULL;

    config = (Configuration_t *) malloc(sizeof(Configuration_t));
    if (!config)
    {
        perror("An error occured while allocation configuration");
        exit(1);
    }

    config->height = 0;
    config->width = 0;

    config->server.address = NULL;
    config->server.port = 0;

    config->bounds.north = 0.0;
    config->bounds.south = 0.0;
    config->bounds.east = 0.0;
    config->bounds.west = 0.0;

    config->excluded.lat = 0.0;
    config->excluded.lng = 0.0;

    config->database.database = NULL;
    config->database.username = NULL;
    config->database.password = NULL;
    config->database.server.address = NULL;
    config->database.server.port = 0;

    return config;
}

static void handle_section_server(Configuration_t *conf, const char *section, const char *name, const char *value)
{
    if (strcmp(section, "server") != 0)
    {
        return;
    }

    if (!strcmp(name, "address"))
    {
        conf->server.address = strdup(value);
    }
    else if (!strcmp(name, "port"))
    {
        conf->server.port = (uint16_t) atoi(value);
    }
}

static void handle_section_map(Configuration_t *conf, const char *section, const char *name, const char *value)
{
    if (strcmp(section, "map") != 0)
    {
        return;
    }

    if (!strcmp(name, "width"))
    {
        conf->width = atoi(value);
    }
    else if (!strcmp(name, "height"))
    {
        conf->height = atoi(value);
    }
}

static void handle_section_database(Configuration_t *conf, const char *section, const char *name, const char *value)
{
    if (strcmp(section, "database") != 0)
    {
        return;
    }

    if (!strcmp(name, "host"))
    {
        conf->database.server.address = strdup(value);
    }
    else if (!strcmp(name, "port"))
    {
        conf->database.server.port = (uint16_t) atoi(value);
    }
    else if (!strcmp(name, "user"))
    {
        conf->database.username = strdup(value);
    }
    else if (!strcmp(name, "password"))
    {
        conf->database.password = strdup(value);
    }
    else if (!strcmp(name, "database"))
    {
        conf->database.database = strdup(value);
    }

}

static void handle_section_excluded(Configuration_t *conf, const char *section, const char *name, const char *value)
{
    if (strcmp(section, "excluded") != 0)
    {
        return;
    }
    if (!strcmp(name, "lat"))
    {
        conf->excluded.lat = atof(value);
    }
    else if (!strcmp(name, "lng"))
    {
        conf->excluded.lng = atof(value);
    }

}

static int handler(void *config, const char *section, const char *name, const char *value)
{
    Configuration_t *conf = (Configuration_t *) config;

    handle_section_map(conf, section, name, value);
    handle_section_database(conf, section, name, value);
    handle_section_server(conf, section, name, value);
    handle_section_excluded(conf, section, name, value);
}

Configuration_t *configuration_read(const char *config_path)
{
    Configuration_t *configuration;

    if (!config_path)
    {
        fprintf(stderr, "I don't have any configuration file.\n");
        exit(EXIT_FAILURE);
    }

    file_ensure_exists(config_path);
    configuration = configuration_create();
    ini_parse(config_path, handler, configuration);

    return configuration;
}

#define DELETE(arg) if (arg) free(arg)

void configuration_dispose(Configuration_t *config)
{
    if (config)
    {
        DELETE(config->server.address);
        DELETE(config->database.server.address);
        DELETE(config->database.database);
        DELETE(config->database.username);
        DELETE(config->database.password);

        free(config);
    }
}
