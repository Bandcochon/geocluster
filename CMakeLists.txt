CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

PROJECT(geocluster)

SET(SOURCES src/main.c
        src/file.h src/file.c
        src/arguments.h src/arguments.c
        src/point.h src/point.c
        src/points_array.h src/points_array.c
        src/cluster.h src/cluster.c
        src/convert.h src/convert.c
        src/json_convertion.h src/json_convertion.c
        src/config.h src/config.c
        src/server.h src/server.c
        src/database.h src/database.c
        src/ini.h src/ini.c src/log.c src/log.h src/common.h)

#SET(CMAKE_CXX_FLAGS "-O3 -finline-functions -ffast-math -Wall")
#SET(CMAKE_C_FLAGS "-O3 -finline-functions -ffast-math -Wall")

SET(CMAKE_CXX_FLAGS "-g -Wall")
SET(CMAKE_C_FLAGS "-g -Wall")

ADD_EXECUTABLE(geocluster ${SOURCES})

# Libraries
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(LIBJANSSON REQUIRED jansson)
PKG_CHECK_MODULES(LIBMARIADB_CLIENT REQUIRED mariadb)
FIND_PATH(LIBEVENT_INCLUDE_DIR event.h PATHS /usr/include PATH_SUFFIXES event)
FIND_LIBRARY(LIBEVENT_LIBRARIES NAMES event PATHS /usr/lib /usr/local/lib)

TARGET_LINK_LIBRARIES(geocluster
        ${LIBJANSSON_LIBRARIES}
        ${LIBEVENT_LIBRARIES}
        ${LIBMARIADB_CLIENT_LIBRARIES}
        )
