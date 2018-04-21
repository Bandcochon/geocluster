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

#include "server.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <event2/event.h>

Server_t *server_create(char *address, uint16_t port)
{
    Server_t *server = NULL;

    server = (Server_t *) malloc(sizeof(Server_t));
    if (!server)
    {
        log_critical("Unable to allocate server object");
        exit(EXIT_FAILURE);
    }

    server->address = address;
    server->port = port;
    server->base = event_base_new();
    server->http = evhttp_new(server->base);

    return server;
}

void server_dispose(Server_t *server)
{
    if (server)
    {
        event_base_free(server->base);
        evhttp_free(server->http);

        if (server->address)
        {
            free(server->address);

            if (server->socket)
            {
                close(server->socket);
            }
        }

        free(server);
    }
}

void server_add_route(Server_t *server, const char *path, ServerCallback callback, void *data)
{
    evhttp_set_cb(server->http, path, callback, data);
}

void server_run(Server_t *server)
{
    struct evhttp_bound_socket *handle;
    char uri_root[512];
    struct sockaddr_storage addr_storage;
    evutil_socket_t fd;
    int got_port;
    ev_socklen_t socklen = sizeof(addr_storage);
    char addrbuf[128];
    void *inaddr;
    const char *addr;


    log_info("Try to acquire the socket at %s:%d", server->address, server->port);

    handle = evhttp_bind_socket_with_handle(server->http, server->address, server->port);
    if (!handle)
    {
        log_critical("Unable to listen the port %d", server->port);
        exit(EXIT_FAILURE);
    }

    /* Extract and display the address we're listening on. */

    fd = evhttp_bound_socket_get_fd(handle);
    memset(&addr_storage, 0, sizeof(addr_storage));

    if (getsockname(fd, (struct sockaddr *) &addr_storage, &socklen))
    {
        log_critical("getsockname() failed because: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (addr_storage.ss_family == AF_INET)
    {
        log_debug("Using IPv4");
        got_port = ntohs(((struct sockaddr_in *) &addr_storage)->sin_port);
        inaddr = &((struct sockaddr_in *) &addr_storage)->sin_addr;
    }

    else if (addr_storage.ss_family == AF_INET6)
    {
        log_debug("Using IPv6");
        got_port = ntohs(((struct sockaddr_in6 *) &addr_storage)->sin6_port);
        inaddr = &((struct sockaddr_in6 *) &addr_storage)->sin6_addr;
    }

    else
    {
        log_critical("Weird address family %d\n", addr_storage.ss_family);
        exit(EXIT_FAILURE);
    }

    addr = evutil_inet_ntop(addr_storage.ss_family, inaddr, addrbuf, sizeof(addrbuf));

    log_info("Listening on %s:%d", addr, got_port);
    evutil_snprintf(uri_root, sizeof(uri_root), "http://%s:%d", addr, got_port);

    event_base_dispatch(server->base);
}
