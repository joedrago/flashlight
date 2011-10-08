
set(flashlight_srcs
    main.c
    flarray.c
    flstring.c
)

include_directories(${cjson_SOURCE_DIR})
add_executable(flashlight ${flashlight_srcs})
target_link_libraries(flashlight cjson)

