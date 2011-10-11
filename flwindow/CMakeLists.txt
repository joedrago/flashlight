
project(flwindow)

set(flwindow_srcs
    main.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${cjson_SOURCE_DIR}
    ${flashlight_SOURCE_DIR}
)
add_executable(flwindow WIN32 ${flwindow_srcs})
target_link_libraries(flwindow flashlight cjson)
if(UNIX)
    target_link_libraries(flwindow m)
endif()

