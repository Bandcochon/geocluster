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
#include "points_array.h"
#include "cluster.h"
#include "json_convertion.h"
#include "config.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

static void process_clustering(char *content, Config_t * config)
{
    PointArray_t *points_array = NULL;
    Cluster_t *cluster = NULL;
    char *result = NULL;

    points_array= convert_from_string(content);
    cluster = cluster_create(config->width, config->height, points_array);

    cluster_set_bounds(cluster, config->north, config->south, config->east, config->west);
    cluster_compute(cluster, config->excluded.lat, config->excluded.lng);

    result = convert_from_cluster(cluster);

    cluster_dispose(cluster);
}

static void get_and_process_file_content(const char * filename, Config_t * config)
{
    char * content = NULL;

    content = file_load(filename);
    if (!content)
    {
        fprintf(stderr, "The content doesn't exist\n");
        return;
    }

    process_clustering(content, config);
    free(content);
}

/*
 * Process the server request and send a response.
 * 
 * @param request: The server request
 * @param response: The server response
 */
static void on_process_response(ServerRequest_t * request, ServerResponse_t * response, void * data)
{
    if (strcmp(request->method, "POST"))
    {
        response->status = 405;
        response->message = "Not implemented";
        return;
    }

    process_clustering(request->body, (Config_t *)data);
}

int main(int argc, char **argv)
{
    Argument_t *args = NULL;
    Config_t * config = NULL;
    Server_t * server = NULL;
    
    args = argument_check(argc, argv);
    usage_if_needed(args);

    config = config_read(args->config_file);

    if (args->filename)
    {
        // For testing purpose
        get_and_process_file_content(args->filename, config);
    }
    else 
    {
        server = server_create(config->address, config->port);
        server_add_route(server, "/", on_process_response, config);
        server_run(server);
    }
    config_dispose(config); 
    argument_dispose(args);

    return 0;
}
