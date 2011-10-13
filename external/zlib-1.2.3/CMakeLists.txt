
if(MSVC)
	set(CMAKE_C_FLAGS /wd4996)
endif()

project(zlib)
set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzio.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    trees.c
    uncompr.c
    zutil.c
)
add_library(zlib ${ZLIB_SRCS})

