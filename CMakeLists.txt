cmake_minimum_required(VERSION 2.6)

project(flashlight)

if(UNIX)
#    set(CMAKE_CXX_FLAGS "-g -Wno-invalid-offsetof")
    set(CMAKE_C_FLAGS "-g")
endif()

set(platform "unknown")
if(APPLE)
    set(platform "osxc")
else()
    if(UNIX)
        set(platform "linuxc")
    endif()
    if(MSVC)
        set(platform "winc")
    endif()
endif()

#add_subdirectory(external)
add_subdirectory(flashlight)

