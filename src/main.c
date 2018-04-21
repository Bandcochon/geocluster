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

#include "arguments.h"
#include "file.h"
#include "cluster.h"
#include "json_convertion.h"
#include "config.h"
#include "server.h"
#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include <evhttp.h>


/*
 * Display the program usage
 */
static void usage_if_needed(Argument_t *args)
{
    if (args->help)
    {
        fprintf(stderr, "Usage: geocluster [OPTIONS]\n");
        fprintf(stderr, "Options are:\n");
        fprintf(stderr, "   -h|--help          : Display this message\n");
        fprintf(stderr, "   -f|--file FILENAME : The file to inspect\n");
        fprintf(stderr, "\n");

        exit(EXIT_SUCCESS);
    }
}

static char *process_clustering(PointArray_t *points_array, Configuration_t *config)
{
    Cluster_t *cluster = NULL;
    char *result = NULL;

    cluster = cluster_create(config->width, config->height, points_array);

    cluster_set_bounds(cluster, config->bounds.north, config->bounds.south, config->bounds.east, config->bounds.west);
    cluster_compute(cluster, config->excluded.lat, config->excluded.lng);

    result = convert_from_cluster(cluster);

    cluster_dispose(cluster);

    return result;
}

static void get_and_process_file_content(const char *filename, Configuration_t *config)
{
    char *content = NULL;
    PointArray_t *points_array = NULL;

    content = file_load(filename);
    if (!content)
    {
        fprintf(stderr, "The content doesn't exist\n");
        return;
    }

    points_array = convert_from_string(content);
    process_clustering(points_array, config);
    free(content);
}

/*
 * Process the server request and send a response.
 * 
 * @param request: The server request
 * @param data: The data associated with the route
 */
static void on_process_response(struct evhttp_request *req, void *data)
{
    struct evkeyvalq params;
    struct evbuffer *buf = NULL;
    PointArray_t *array = NULL;
    char *json_result = NULL;
    int result = 0;

    fprintf(stdout, "Got something from %s\n", req->remote_host);

    result = evhttp_parse_query_str(evhttp_uri_get_query(evhttp_request_get_evhttp_uri(req)), &params);
    if (result == -1)
    {
        evhttp_send_reply(req, 400, "Bad Request", NULL);
    }
    else
    {
        Configuration_t *config = (Configuration_t *) data;
        int got_north = 0, got_west = 0, got_east = 0, got_south = 0;

        for (struct evkeyval *i = params.tqh_first; i; i = i->next.tqe_next)
        {
            if (!strcmp("north", i->key))
            {
                config->bounds.north = atof(i->value);
                got_north = 1;
            }
            else if (!strcmp("south", i->key))
            {
                config->bounds.south = atof(i->value);
                got_south = 1;
            }
            else if (!strcmp("east", i->key))
            {
                config->bounds.east = atof(i->value);
                got_east = 1;
            }
            else if (!strcmp("west", i->key))
            {
                config->bounds.west = atof(i->value);
                got_west = 1;
            }
            else
            {
                fprintf(stderr, "Unknown key %s, with this value %s\n", i->key, i->value);
                evhttp_send_reply(req, 400, "Bad Request", NULL);
                return;
            }
        }


        if (!(got_east && got_north && got_south && got_west))
        {
            fprintf(stderr, "Error : Missing parameters\n");
            evhttp_send_reply(req, 400, "Bad Request: Missing parameters", NULL);
            return;
        }
        array = database_execute(config);
        json_result = process_clustering(array, config);

        buf = evbuffer_new();
        evbuffer_add_printf(buf, "%s", json_result);
        free(json_result);
        evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
        evhttp_send_reply(req, 200, "OK", buf);
    }
}

int main(int argc, char **argv)
{
    Argument_t *args = NULL;
    Configuration_t *config = NULL;
    Server_t *server = NULL;

    args = argument_check(argc, argv);
    usage_if_needed(args);

    config = configuration_read(args->config_file);

    config->database.db = database_connect();

    if (args->filename)
    {
        // For testing purpose
        fprintf(stdout, "Start as normal executable\n");
        get_and_process_file_content(args->filename, config);
    }
    else
    {
        fprintf(stdout, "Start as micro service\n");

        server = server_create(config->server.address, config->server.port);
        server_add_route(server, "/", (ServerCallback) on_process_response, config);

        server_run(server);
        server_dispose(server);
    }

    configuration_dispose(config);
    argument_dispose(args);
    mysql_close(config->database.db);

    return 0;
}
