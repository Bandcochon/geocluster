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
static char *process_clustering(PointArray_t *points_array, Configuration_t *config, Bound_t bounds)
{
    Cluster_t *cluster = NULL;
    char *result = NULL;

    cluster = cluster_create(config->width, config->height, points_array);
    cluster_set_bounds(cluster, bounds.north, bounds.south, bounds.east, bounds.west);
    cluster_compute(cluster, config->excluded.lat, config->excluded.lng);
//    result = convert_from_cluster(cluster);
    result = strdup("{\"cleaned\":[], \"uncleaned\":[]");
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

        log_debug("Got parameters");
        for (struct evkeyval *i = params.tqh_first; i; i = i->next.tqe_next)
        {
            if (!strcmp("north", i->key))
            {
//                bounds.north = -20.800551419646325;
                bounds.north = atof(i->value);
                got_north = 1;
            }
            else if (!strcmp("south", i->key))
            {
//                bounds.south = -21.441065626573486;
                bounds.south = atof(i->value);
                got_south = 1;
            }
            else if (!strcmp("east", i->key))
            {
//                bounds.east = 56.351302046051046;
                bounds.east = atof(i->value);
                got_east = 1;
            }
            else if (!strcmp("west", i->key))
            {
//                bounds.west = 54.703352827301046;
                bounds.west = atof(i->value);
                got_west = 1;
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

        json_result =  process_clustering(array, config, bounds);
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

    debug_mode = getenv("DEBUG");
    if (debug_mode && !strcmp(debug_mode, "1"))
    {
        log_init(stderr, LOG_DEBUG);
    }
    else
    {
        log_file = fopen(config->logfile, "a+");
        log_init(log_file, LOG_INFO);
    }

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
    Application_t app;
    Argument_t *args = NULL;
    Configuration_t *config = NULL;
    FILE *log_file = NULL;
    PointArray_t * points;

    args = argument_check(argc, argv);
    usage_if_needed(args);

    config = configuration_read(args->config_file);
    log_file = initialize_log(config);

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
