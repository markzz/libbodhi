cmake_minimum_required(VERSION 3.0)
project(bodhi C)

set(CMAKE_C_STANDARD 90)

include_directories(lib)

add_library(bodhi SHARED
        lib/libbodhi/list.c
        lib/libbodhi/list.h
        lib/libbodhi/util.c
        lib/libbodhi/util.h
        lib/libbodhi/hmap.c
        lib/libbodhi/hmap.h
        lib/libbodhi/patricia.c
        lib/libbodhi/patricia.h)

install(FILES
        lib/libbodhi/hmap.h lib/libbodhi/list.h
        lib/libbodhi/patricia.h
        DESTINATION include/libbodhi)
install(TARGETS bodhi
        LIBRARY DESTINATION lib)
