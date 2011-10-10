
set(flashlight_srcs
    main.c
    flashlight.c
    flarray.c
    flwalk.c
    posix/walk.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${cjson_SOURCE_DIR})
add_executable(flashlight ${flashlight_srcs})
target_link_libraries(flashlight cjson)

