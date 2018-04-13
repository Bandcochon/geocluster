CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

SET(SOURCES src/main.c 
            src/file.c 
            src/arguments.c
            src/point.c
            src/points_array.c
            src/cluster.c
            src/convert.c
            src/json_convertion.c
            src/config.c
            src/server.c
            src/route.c)

SET( CMAKE_CXX_FLAGS  "-O3 -finline-functions -ffast-math" )
PROJECT(geocluster)
ADD_EXECUTABLE(geocluster ${SOURCES})

# Libraries
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(LIBJANSSON REQUIRED jansson)
PKG_CHECK_MODULES(LIBYAML REQUIRED yaml-0.1)
TARGET_LINK_LIBRARIES(geocluster ${LIBJANSSON_LIBRARIES} ${LIBYAML_LIBRARIES})
