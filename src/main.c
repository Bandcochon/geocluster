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
#include "cluster.h"
#include "json_convertion.h"
#include "config.h"
#include "server.h"
#include "database.h"
#include "log.h"

#include <string.h>
#include <unistd.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include <evhttp.h>
#include <time.h>


static uint8_t MaxSize = 100;

typedef struct Application_t
{
    Configuration_t * config;
    PointArray_t * points;
} Application_t;

/*
 * Display the program usage
 *
 * @param args: The argument structure.
 */
static void usage_if_needed(Argument_t *args)
{
    if (args && args->help)
    {
        fprintf(stderr, "Usage: geocluster [OPTIONS]\n");
        fprintf(stderr, "Options are:\n");
        fprintf(stderr, "   -h|--help          : Display this message\n");
        fprintf(stderr, "   -f|--file FILENAME : The file to inspect\n");
        fprintf(stderr, "\n");

        exit(EXIT_SUCCESS);
    }
}

/*
 * Do the clustering  with the database result.
 *
 * @param points_array:
 */
static char *process_clustering(PointArray_t *points_array, Configuration_t *config, Bound_t bounds, int clusterize)
{
    Cluster_t *cluster = NULL;
    char *result = NULL;

    uint8_t width = clusterize == 0 ? MaxSize : config->width;
    uint8_t height = clusterize == 0 ? MaxSize : config->width;

    cluster = cluster_create(width, height, points_array);
    cluster_set_bounds(cluster, bounds.north, bounds.south, bounds.east, bounds.west);
    cluster_compute(cluster, config->excluded.lat, config->excluded.lng, clusterize);
    result = convert_from_cluster(cluster);
    cluster_dispose(cluster);

    return result;
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
    Configuration_t * config;
    Bound_t bounds;

    struct evbuffer *buf = NULL;
    PointArray_t *array = NULL;
    char *json_result = NULL;
    int result = 0;
    int clusterize = 1;

    log_info("Got something from %s", req->remote_host);

    array = ((Application_t *) data)->points;
    config = ((Application_t *) data)->config;

    memset(&bounds, 0, sizeof(Bound_t));

    result = evhttp_parse_query_str(evhttp_uri_get_query(evhttp_request_get_evhttp_uri(req)), &params);
    if (result == -1)
    {
        log_error("There's no parameters");
        evhttp_send_reply(req, 400, "Bad Request", NULL);
        return;
    }
    else
    {
        int got_north = 0, got_west = 0, got_east = 0, got_south = 0;

        log_debug("Got parameters: %s", req->uri);
        for (struct evkeyval *i = params.tqh_first; i; i = i->next.tqe_next)
        {
            log_debug("Key: %s , Value: %s", i->key, i->value);

            if (!strcmp("north", i->key))
            {
                bounds.north = atof(i->value);
                got_north = 1;
            }
            else if (!strcmp("south", i->key))
            {
                bounds.south = atof(i->value);
                got_south = 1;
            }
            else if (!strcmp("east", i->key))
            {
                bounds.east = atof(i->value);
                got_east = 1;
            }
            else if (!strcmp("west", i->key))
            {
                bounds.west = atof(i->value);
                got_west = 1;
            }
            else if (!strcmp("cluster", i->key))
            {
                clusterize = !strcmp("false", i->value) ? 0 : 1;
            }
            else
            {
                log_error("Unknown key %s, with this value %s\n", i->key, i->value);
                evhttp_send_reply(req, 400, "Bad Request", NULL);
                return;
            }
        }

        log_debug("Parameters are: north:%f south:%f east:%f west:%f",
                  bounds.north, bounds.south, bounds.east, bounds.west);


        if (!(got_east && got_north && got_south && got_west))
        {
            log_error("Missing parameters");
            evhttp_send_reply(req, 400, "Bad Request: Missing parameters", NULL);
            return;
        }

        clock_t begin = clock();

        json_result =  process_clustering(array, config, bounds, clusterize);
        if (!json_result)
        {
            log_error("No results");
            evhttp_send_reply(req, 200, "OK", NULL);
            return;
        }

        clock_t end = clock();
        log_info("Computation done in %.2f ms", ((float) (end - begin) / CLOCKS_PER_SEC) * 1000.f);

        buf = evbuffer_new();
        evbuffer_add_printf(buf, "%s", json_result);
        evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
        evhttp_send_reply(req, 200, "OK", buf);

        free(json_result);
        evbuffer_free(buf);
    }
}

static void start_web_server(Configuration_t * config, PointArray_t *points)
{
    Server_t *server = NULL;
    Application_t container = {config, points};

    log_info("Start as micro service.");

    server = server_create(config->server.address, config->server.port);
    server_add_route(server, "/", (ServerCallback) on_process_response, &container);

    server_run(server);
    server_dispose(server);

}

static FILE *initialize_log(Configuration_t *config)
{
    char *debug_mode = NULL;
    FILE *log_file = NULL;
    pid_t pid;

    log_init(stderr, LOG_DEBUG);

    pid = getpid();
    log_info("Starting the app with PID=%d", pid);

    return log_file;
}

PointArray_t * get_points_from_database(Configuration_t * config)
{
    MYSQL * db = NULL;
    PointArray_t * points = NULL;

    db = database_connect(config);
    points = database_execute(db);
    mysql_close(db);

    return points;
}

int main(int argc, char **argv)
{
//    Application_t app;
    Argument_t *args = NULL;
    Configuration_t *config = NULL;
    FILE *log_file = NULL;
    PointArray_t * points;

    log_file = initialize_log(config);

    args = argument_check(argc, argv);
    printf("Help: %d\nConfig file: %s\nfilename: %s", args->help, args->filename, args->config_file);
    usage_if_needed(args);

    config = configuration_read(args->config_file);

    points = get_points_from_database(config);
    start_web_server(config, points);

    log_info("Shutting down");
    configuration_dispose(config);
    argument_dispose(args);
    if (log_file != NULL)
    {
        fclose(log_file);
    }

    log_info("End");

    return 0;
}
