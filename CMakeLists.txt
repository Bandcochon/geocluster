cmake_minimum_required(VERSION 3.7)

project(geocluster)

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else ()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

set(SOURCES src/main.c
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

add_executable(geocluster ${SOURCES})

conan_target_link_libraries(geocluster)
