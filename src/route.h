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

#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "server_types.h"

/* 
 * Create a route object
 * 
 * @return: The allocated route object
 */
Route_t * route_create(void);

/* 
 * Create a route object with all necessary arguments.
 * 
 * @param path: The route path
 * @param callback: The route callback when match with the path
 * @param data: arbitray data
 */
Route_t * route_create_with_values(const char * path, ServerCallback callback, void * data);

/*
 * Remove allocated route from memory. Also dispose the path.
 * 
 * @param route: The route object
 */
void route_dispose(Route_t * route);

/*
 * Remove all routes from memory using route_dispose.
 * 
 * @param route: The route object
 */
void route_dispose_all(Route_t * route);

/*
 * Add a new route to the route object.  If the route exists, do nothing
 * 
 * @param route: THe main route object
 * @param route_to_add: The route object to add to the routes
 */
void route_add_route(Route_t * route, Route_t * route_to_add);

/* 
 * Find a route by its path.
 * 
 * @param route: The main route object
 * @param path: The URL path
 * @return The route object corresponding to the path or NULL
 */
Route_t * route_find_by_path(Route_t * route, const char * path);

#endif 