cmake_minimum_required(VERSION 3.12)
project(bodhi C)

set(CMAKE_C_STANDARD 90)

include_directories(lib/libbodhi)

add_library(bodhi SHARED
        lib/libbodhi/list.c
        lib/libbodhi/list.h
        lib/libbodhi/util.c
        lib/libbodhi/util.h
        lib/libbodhi/hmap.c
        lib/libbodhi/hmap.h)

install(FILES
        lib/libbodhi/hmap.h lib/libbodhi/list.h
        DESTINATION include/bodhi)
install(TARGETS bodhi
        LIBRARY DESTINATION lib)
