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

#include "cluster.h"
#include "convert.h"
#include "log.h"


static Cluster_t ***cluster_create_sub_clusters(Cluster_t *cluster)
{
    Cluster_t ***group = malloc(sizeof(Cluster_t *) * cluster->height * cluster->width);

    double inc_lat = (cluster->south - cluster->north) / cluster->height;
    double inc_lng = (cluster->east - cluster->west) / cluster->width;
    double north = cluster->north;
    double west;

    for (register int i = 0; i < cluster->height; i++)
    {
        west = cluster->west;
        group[i] = malloc(sizeof(Cluster_t *) * cluster->height);

        for (register int j = 0; j < cluster->width; j++)
        {
            Cluster_t *c = cluster_create(1, 1, points_array_create(ARRAY_EMPTY));
            c->north = north;
            c->south = north + inc_lat;
            c->east = west + inc_lng;
            c->west = west;

            group[i][j] = c;

            west += inc_lng;
        }
        north += inc_lat;
    }

    return group;
}

static inline char cluster_contains(Cluster_t *cluster, Point_t *point)
{
    return point->position.lat >= cluster->north &&
        point->position.lat <= cluster->south &&
        point->position.lng >= cluster->west &&
        point->position.lng <= cluster->east;
}

static void cluster_populate_groups(Cluster_t *cluster, double excluded_lat, double excluded_lng)
{
    register int length = (int) cluster->points_array->length;

    for (register int i = 0; i < cluster->height; i++)
    {
        for (register int j = 0; j < cluster->width; j++)
        {
            for (register int p = 0; p < length; p++)
            {
                if (cluster->points_array->points[p]->position.lat == excluded_lat
                    && cluster->points_array->points[p]->position.lng == excluded_lng)
                {
                    continue;
                }

                if (cluster_contains(cluster, cluster->points_array->points[p]))
                {
                    if (cluster->points_array->points[p]->disappeared)
                    {
                        if (cluster_contains(cluster->groups_exists[i][j], cluster->points_array->points[p]))
                        {
                            points_array_append_point(cluster->groups_exists[i][j]->points_array,
                                                      cluster->points_array->points[p]);
                        }
                    }
                    else
                    {
                        if (cluster_contains(cluster->groups_disappeared[i][j], cluster->points_array->points[p]))
                        {
                            points_array_append_point(cluster->groups_disappeared[i][j]->points_array,
                                                      cluster->points_array->points[p]);
                        }
                    }
                }
            }
        }
    }
}

Cluster_t *cluster_create(uint8_t width, uint8_t height, PointArray_t *points_array)
{
    Cluster_t *cluster = NULL;

    cluster = (Cluster_t *) malloc(sizeof(Cluster_t));
    if (!cluster)
    {
        log_critical("Memory error while allocating cluster of points\n");
        exit(1);
    }

    cluster->groups_disappeared = NULL;
    cluster->groups_exists = NULL;
    cluster->points_array = points_array;
    cluster->height = height;
    cluster->width = width;
    cluster->north = 0.;
    cluster->south = 0.;
    cluster->east = 0.;
    cluster->west = 0.;
    cluster->lat = 0.;
    cluster->lng = 0.;

    return cluster;
}

void cluster_dispose(Cluster_t *cluster)
{
    for (register int i = 0; i < cluster->height; i++)
    {
        for (register int j = 0; j < cluster->width; j++)
        {
            DELETE(cluster->groups_disappeared[i][j]->points_array->points);
            DELETE(cluster->groups_disappeared[i][j]->points_array);
            DELETE(cluster->groups_disappeared[i][j]);

            DELETE(cluster->groups_exists[i][j]->points_array->points);
            DELETE(cluster->groups_exists[i][j]->points_array);
            DELETE(cluster->groups_exists[i][j]);
        }

        DELETE(cluster->groups_exists[i]);
        DELETE(cluster->groups_disappeared[i]);
    }

    DELETE(cluster->groups_disappeared);
    DELETE(cluster->groups_exists);

    DELETE(cluster);
}

void cluster_set_bounds(Cluster_t *cluster, double north, double south, double east, double west)
{
    cluster->north = convert_lat_from_gps(north);
    cluster->south = convert_lat_from_gps(south);
    cluster->east = convert_lng_from_gps(east);
    cluster->west = convert_lng_from_gps(west);
}

void cluster_compute(Cluster_t *cluster, double excluded_lat, double excluded_lng, int clusterize)
{
    log_info("Clusterize: %d", clusterize);
    log_info("Width: %d, Height: %d", cluster->width, cluster->height);

    cluster->groups_disappeared = cluster_create_sub_clusters(cluster);
    cluster->groups_exists = cluster_create_sub_clusters(cluster);

    cluster_populate_groups(cluster, excluded_lat, excluded_lng);
}

void cluster_compute_barycenter(Cluster_t *cluster)
{
    double s_lat = 0., s_lng = 0.;
    size_t i;

    for (i = 0; i < cluster->points_array->length; i++)
    {
        s_lat += cluster->points_array->points[i]->position.lat;
        s_lng += cluster->points_array->points[i]->position.lng;
    }

    cluster->lat = s_lat / (double) cluster->points_array->length;
    cluster->lng = s_lng / (double) cluster->points_array->length;
}
