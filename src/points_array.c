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

#include "points_array.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>

PointArray_t *points_array_create(unsigned int size)
{
    PointArray_t *arr = (PointArray_t *)malloc(sizeof(PointArray_t));
    if (!arr)
    {
        log_critical("Memory error while allocating array");
        exit(1);
    }

    arr->length = size;
    arr->position = 0;
    if (size)
    {
        arr->points = malloc(sizeof(Point_t) * size);
    }
    else
    {
        arr->points = NULL;
    }

    return arr;
}

void points_array_dispose(PointArray_t *arr)
{
    for (int i = 0; i < arr->length; i++)
    {
        point_dispose(arr->points[i]);
    }

    free(arr->points);
    free(arr);
}

void points_array_add_point(PointArray_t *arr, Point_t *point)
{
    arr->points[arr->position] = point;
    arr->position++;
}

void points_array_append_point(PointArray_t *arr, Point_t *point)
{
    arr->length++;
    // Not the most efficient, but it works
    arr->points = realloc(arr->points, sizeof(Point_t *) * arr->length);
    arr->points[arr->position] = point;
    arr->position++;
}
