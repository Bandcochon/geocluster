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


#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


/*
 *  Display the database error, then exit.
 *
 *  @param mysql: The mysql connection structure
 */
static void show_mysql_error(MYSQL *mysql)
{
    printf("Error(%d) [%s] \"%s\"", mysql_errno(mysql),
           mysql_sqlstate(mysql),
           mysql_error(mysql));
    mysql_close(mysql);
    exit(-1);
}

MYSQL *database_connect(void)
{
    MYSQL *db = mysql_init(NULL);
    if (!mysql_real_connect(db, "172.17.0.1", "bandcochon", "bandcochon", "bandcochon", 0, NULL, 0))
    {
        show_mysql_error(db);
    }
    return db;
}

PointArray_t *database_execute(Configuration_t *config)
{
    MYSQL_RES *db_result = NULL;
    MYSQL_ROW row = NULL;
    PointArray_t *points_array = NULL;
    my_ulonglong num_rows = 0;
    int result = 0;

    result =
        mysql_query(config->database.db,
                    "SELECT id, latti AS lat, longi AS lng, disappeared, `desc` FROM bandcochon_picture "
                    "WHERE trash=0" // and latti >= struc and latti <= struc and longi >= machin and longi <= bidule
        );
    if (result)
    {
        show_mysql_error(config->database.db);
    }

    db_result = mysql_store_result(config->database.db);
    if (!db_result)
    {
        return NULL;
    }

    num_rows = mysql_num_rows(db_result);
    points_array = points_array_create((uint32_t) num_rows);

    while ((row = mysql_fetch_row(db_result)))
    {
        uint32_t pk = atoi(row[0]);
        double lat = atof(row[1]);
        double lng = atof(row[2]);
        char disa = (char) atoi(row[3]);
        char *desc = strdup(row[4]);

        Point_t *p = point_create(lat, lng, disa, pk, desc);
        points_array_add_point(points_array, p);
    }

    mysql_free_result(db_result);

    return points_array;
}