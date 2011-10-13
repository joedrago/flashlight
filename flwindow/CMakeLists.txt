
project(flwindow)

set(flwindow_srcs
    image.c
    image.h
    main.c
    theme.c
    theme.h
    util.c
    util.h
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${cjson_SOURCE_DIR}
    ${zlib_SOURCE_DIR}
    ${libpng_SOURCE_DIR}
    ${flashlight_SOURCE_DIR}
)
add_executable(flwindow WIN32 ${flwindow_srcs})
target_link_libraries(flwindow flashlight cjson libpng zlib)
if(UNIX)
    target_link_libraries(flwindow m)
endif()

