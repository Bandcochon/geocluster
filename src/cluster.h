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

#ifndef __CLUSTER_H__
#define __CLUSTER_H__

#include "point.h"
#include "points_array.h"
#include "common.h"

#include <stdint.h>

typedef struct Cluster_t Cluster_t;
struct Cluster_t
{
    Cluster_t *** groups_exists;
    Cluster_t *** groups_disappeared;

    PointArray_t *points_array;
    uint8_t width, height;
    double north, south, east, west, lat, lng;
};

Cluster_t *cluster_create(uint8_t width, uint8_t height, PointArray_t *points_array);
void cluster_dispose(Cluster_t *cluster);
void cluster_set_bounds(Cluster_t *cluster, double north, double south, double east, double west);
void cluster_compute(Cluster_t *cluster, double excluded_lat, double excluded_lng, int clusterize);
void cluster_compute_barycenter(Cluster_t * cluster);

#endif
