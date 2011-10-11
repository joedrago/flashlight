
project(flashlight)

set(flashlight_srcs
    flashlight.c
    flashlight.h
    flarray.c
    flarray.h
    flwalk.c
    flwalk.h
)

if(UNIX)
    set(flashlight_srcs ${flashlight_srcs}
        posix/walk.c
    )
else()
    set(flashlight_srcs ${flashlight_srcs}
        win/walk.c
    )
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${cjson_SOURCE_DIR})
add_library(flashlight ${flashlight_srcs})

