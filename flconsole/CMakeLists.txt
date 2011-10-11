
project(flconsole)

set(flconsole_srcs
    main.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${cjson_SOURCE_DIR}
    ${flashlight_SOURCE_DIR}
)
add_executable(flconsole ${flconsole_srcs})
target_link_libraries(flconsole flashlight cjson)

