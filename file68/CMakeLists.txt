include(CheckIncludeFile)

set(FILE68_UNICE68 ON)
set(FILE68_Z ON)
set(FILE68_CURL OFF)
set(FILE68_AO OFF)

configure_file(
	config.h.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/sc68
)

configure_file(
    sc68/file68_features.h.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sc68/file68_features.h
    @ONLY
)

add_library(file68 STATIC)

target_sources(file68
    PRIVATE
        src/endian68.c
        src/error68.c
        src/file68.c
        src/gzip68.c
        src/ice68.c
        src/init68.c
        src/msg68.c
        src/option68.c
        src/registry68.c
        src/replay68.c
        src/rsc68.c
        src/string68.c
        src/timedb68.c
        src/uri68.c
        src/vfs68.c
        src/vfs68_ao.c
        src/vfs68_curl.c
        src/vfs68_fd.c
        src/vfs68_file.c
        src/vfs68_mem.c
        src/vfs68_null.c
        src/vfs68_z.c
)

target_include_directories(file68
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/sc68
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
        
)

target_link_libraries(file68
    PRIVATE
        unice68
        zlib
)

target_compile_definitions(file68
    PRIVATE
        FILE68_EXPORT=1
        USE_UNICE68=1
        USE_REPLAY68=1
        USE_Z=1
        HAVE_CONFIG_H=1
)

if(WIN32)
    target_compile_definitions(file68
        PRIVATE
            ISTREAM68_NO_FD=1
            USE_REGISTRY68=1
    )
endif()


