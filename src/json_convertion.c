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

#include "json_convertion.h"
#include "cluster.h"
#include "convert.h"
#include "log.h"

#include <jansson.h>

static json_t *_create_array(Cluster_t *root, Cluster_t ***cluster);

static json_t *_create_object_from_point(Cluster_t *point);


char *convert_from_cluster(Cluster_t *cluster) {
    char *result = NULL;
    json_t *root = json_object();
    json_t *exists_array = _create_array(cluster, cluster->groups_exists);
    json_t *disappeared_array = _create_array(cluster,
                                              cluster->groups_disappeared);

    json_object_set(root, "uncleaned", disappeared_array);
    json_object_set(root, "cleaned", exists_array);

    result = json_dumps(root, 0);

    json_decref(disappeared_array);
    json_decref(exists_array);
    json_decref(root);

    return result;
}

static json_t *_create_array(Cluster_t *root, Cluster_t ***cluster) {
    json_t *array = json_array();

    for (register int i = 0; i < root->height; i++) {
        json_t *rows = json_array();
        for (register int j = 0; j < root->width; j++) {
            json_t *point = _create_object_from_point(cluster[i][j]);
            json_array_append(rows, point);
            json_decref(point);
        }

        json_array_append(array, rows);
        json_decref(rows);
    }

    return array;
}

static json_t *_create_object_from_point(Cluster_t *cluster) {
    json_t *obj, *count, *lat, *lng, *desc, *pk;

    if (!cluster->points_array->length) {
        return json_null();
    }

    obj = json_object();
    count = json_integer(cluster->points_array->length);
    if (cluster->points_array->length == 1) {
        lat = json_real(convert_lat_to_gps(
                cluster->points_array->points[0]->position.lat));
        lng = json_real(convert_lng_to_gps(
                cluster->points_array->points[0]->position.lng));

        desc = json_string(cluster->points_array->points[0]->desc);
        json_object_set(obj, "desc", desc);
        json_decref(desc);

        pk = json_integer(cluster->points_array->points[0]->pk);
        json_object_set(obj, "id", pk);
        json_decref(pk);
    } else {
        cluster_compute_barycenter(cluster);
        lat = json_real(convert_lat_to_gps(cluster->lat));
        lng = json_real(convert_lng_to_gps(cluster->lng));
    }

    json_object_set(obj, "count", count);
    json_object_set(obj, "lat", lat);
    json_object_set(obj, "lng", lng);

    json_decref(count);
    json_decref(lat);
    json_decref(lng);

    return obj;
}