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

#ifndef __CONFIG_H___
#define __CONFIG_H___

#include "point.h"
#include <stdint.h>
#include <mysql/mysql.h>

typedef struct {
    double north;
    double south;
    double east;
    double west;
} Bound_t;

typedef struct {
    uint16_t port;
    char * address;
} ServerConfig_t;

typedef struct {
    ServerConfig_t server;
    char * username;
    char * password;
    char * database;
    MYSQL * db;
} DatabaseConfig_t;

typedef struct
{
    int width, height;
    LatLng_t excluded;
    Bound_t bounds;
    ServerConfig_t server;
    DatabaseConfig_t database;
} Configuration_t;

/*
 * Read the configuration from disk
 *
 * @param config_path: The file to read
 * @return The configuration object
 */
Configuration_t *configuration_read(const char *config_path);

/*
 * Dispose all configuration from memory
 *
 * @param config: The configuration object
 */
void configuration_dispose(Configuration_t *config);

#endif
