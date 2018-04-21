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

#include <jansson.h>

static json_t *_create_array(Cluster_t *root, Cluster_t ***cluster);
static json_t *_create_object_from_point(Cluster_t *point);

/*
 * Parse the content and convert it into the struct we want.
 * If the content is not json complient, exit with error 1
 */
PointArray_t *convert_from_string(char *content)
{
    json_t *root;
    json_error_t error;
    PointArray_t *points_array;
    int register len = 0;

    root = json_loads(content, 0, &error);
    if (!root)
    {
        fprintf(stderr, "Error: on line %d: %s\n", error.line, error.text);
        exit(EXIT_FAILURE);
    }

    len = json_array_size(root);
    points_array = points_array_create(len);
    for (register int i = 0; i < len; i++)
    {
        json_t *data = json_array_get(root, i), *lat, *lng, *disappeared, *desc, *pk;
        Point_t *point;

        lat = json_object_get(data, "lat");
        lng = json_object_get(data, "lng");
        disappeared = json_object_get(data, "disappeared");
        desc = json_object_get(data, "desc");
        pk = json_object_get(data, "pk");

        point = point_create(json_number_value(lat),
                             json_number_value(lng),
                             json_boolean_value(disappeared),
                             json_number_value(pk),
                             json_string_value(desc));

        points_array_add_point(points_array, point);
    }

    json_decref(root);

    return points_array;
}

char *convert_from_cluster(Cluster_t *cluster)
{
    json_t *root = json_object();
    json_t *exists_array = _create_array(cluster, cluster->groups_exists);
    json_t *disappeared_array = _create_array(cluster, cluster->groups_disappeared);

    json_object_set(root, "uncleaned", exists_array);
    json_object_set(root, "cleaned", disappeared_array);

    return json_dumps(root, 0);
}

static json_t *_create_array(Cluster_t *root, Cluster_t ***cluster)
{
    json_t *array = json_array();

    for (register int i = 0; i < root->height; i++)
    {
        json_t * rows = json_array();
        for (register int j = 0; j < root->width; j++)
        {
            json_array_append(rows, _create_object_from_point(cluster[i][j]));
        }

        json_array_append(array, rows);
    }

    return array;
}

static json_t *_create_object_from_point(Cluster_t *cluster)
{
    json_t *obj, *integer, *lat, *lng, *desc, *pk;

    if (!cluster->points_array->length)
    {
        return json_null();
    }

    obj = json_object();
    integer = json_integer(cluster->points_array->length);
    if (cluster->points_array->length == 1)
    {
        lat = json_real(convert_lat_to_gps(cluster->points_array->points[0]->position.lat));
        lng = json_real(convert_lng_to_gps(cluster->points_array->points[0]->position.lng));

        if (cluster->points_array->points[0]->desc)
        {
            desc = json_string(cluster->points_array->points[0]->desc);
            json_object_set(obj, "desc", desc);

            pk = json_integer(cluster->points_array->points[0]->pk);
            json_object_set(obj, "pk", pk);
        }
    }
    else
    {
        // Faire le barycentre
        lat = json_real(convert_lat_to_gps((cluster->south - cluster->north) / 2.0));
        lng = json_real(convert_lng_to_gps((cluster->east - cluster->west) / 2));
    }

    json_object_set(obj, "count", integer);
    json_object_set(obj, "lat", lat);
    json_object_set(obj, "lng", lng);

    return obj;
}