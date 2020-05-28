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
#include "log.h"

#include <memory.h>


/*
 *  Display the database error, then exit.
 *
 *  @param mysql: The mysql connection structure
 */
static void show_mysql_error(MYSQL *mysql)
{
    log_critical("Error(%d) [%s] \"%s\"", mysql_errno(mysql), mysql_sqlstate(mysql), mysql_error(mysql));
    mysql_close(mysql);
    exit(-1);
}

MYSQL *database_connect(Configuration_t *config)
{
    MYSQL *db = mysql_init(NULL);
    log_info("Connect to the database");

    if (!mysql_real_connect(db,
                            config->database.server.address,
                            config->database.username,
                            config->database.password,
                            config->database.database,
                            config->database.server.port, NULL, 0))
    {
        show_mysql_error(db);
    }
    return db;
}

PointArray_t *database_execute(MYSQL * db)
{
    MYSQL_RES *db_result = NULL;
    MYSQL_ROW row = NULL;
    PointArray_t *points_array = NULL;
    my_ulonglong num_rows = 0;
    int result = 0;

    result = mysql_query(db,
                         "SELECT "
                            "id, "
                            "latti AS lat, "
                            "longi AS lng, "
                            "disappeared, "
                            "`desc` "
                        "FROM bandcochon_picture WHERE id NOT IN "
                        "(SELECT "
                            "id "
                        "FROM  "
                            "bandcochon_picture "
                        "WHERE "
                            "latti <= -21.121154270682 AND "
                            "latti >= -21.121154270683 AND "
                            "longi >= 55.5273274366760 AND "
                            "longi <= 55.5273274366761)");

    if (result)
    {
        log_warning("No result set found");
        show_mysql_error(db);
    }

    db_result = mysql_store_result(db);
    if (!db_result)
    {
        log_warning("Enable to store results");
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
        char *desc = strlen(row[4]) > 0 ? strdup(row[4]) : NULL;

        Point_t *p = point_create(lat, lng, disa, pk, desc);
        points_array_add_point(points_array, p);
    }

    mysql_free_result(db_result);

    return points_array;
}