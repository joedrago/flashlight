
project(flashlight)

set(flashlight_srcs
    flashlight.c
    flashlight.h
    flarray.c
    flarray.h
    flwalk.c
    flwalk.h
    image.c
    image.h
    jpath.c
    jpath.h
)

if(UNIX)
    set(flashlight_srcs ${flashlight_srcs}
        posix/walk.c
    )
else()
    set(flashlight_srcs ${flashlight_srcs}
        win/walk.c
        win/exec.c
    )
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${cjson_SOURCE_DIR}
    ${zlib_SOURCE_DIR}
    ${libpng_SOURCE_DIR}
)
add_library(flashlight ${flashlight_srcs})
target_link_libraries(flashlight cjson libpng zlib)
if(NOT UNIX)
    target_link_libraries(flashlight msimg32.lib)
endif()


