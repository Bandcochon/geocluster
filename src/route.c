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

#include "route.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Route_t *route_create(void)
{
    Route_t *route = NULL;

    route = (Route_t *)malloc(sizeof(Route_t));
    if (!route)
    {
        perror("Unable to allocate a route object");
        exit(EXIT_FAILURE);
    }

    route->path = NULL;
    route->callback = NULL;
    route->data = NULL;
    route->next = NULL;

    return route;
}

Route_t *route_create_with_values(const char *path, ServerCallback callback, void *data)
{
    Route_t *route = NULL;

    route = route_create();
    route->path = strdup(path);
    route->callback = callback;
    route->data = data;
    route->next = NULL;

    return route;
}

void route_dispose(Route_t *route)
{
    if (route)
    {
        if (route->path)
        {
            free(route->path);
        }

        free(route);
    }
}

void route_dispose_all(Route_t *route)
{
    Route_t *next_one = NULL;

    while (route->next)
    {
        next_one = route->next;
        route_dispose(next_one);
        route = next_one;
    }

    route_dispose(route);
}

void route_add_route(Route_t *route, Route_t *route_to_add)
{
    Route_t *saved = route;

    if (!route_find_by_path(route, route_to_add->path))
    {
        fprintf(stderr, "Warning: THe route %s already exists.", route_to_add->path);
        return;
    }

    while (saved->next)
    {
        saved = saved->next;
    }

    saved->next = route_to_add;
}

Route_t *route_find_by_path(Route_t *route, const char *path)
{
    Route_t *saved = route;

    while (saved->next)
    {
        if (!strcmp(saved->path, path))
        {
            return saved;
        }

        saved = saved->next;
    }

    return NULL;
}