include(CheckIncludeFile)
include(CheckSymbolExists)

configure_file(
	config.h.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_library(libsc68 STATIC)

add_library(emu68_obj OBJECT)

add_library(io68_obj OBJECT)

add_library(dial68_obj OBJECT)

target_sources(emu68_obj
	PRIVATE
		emu68/emu68.c
		emu68/error68.c
		emu68/getea68.c
		emu68/inst68.c
		emu68/ioplug68.c
		emu68/lines68.c
		emu68/mem68.c
)

target_include_directories(emu68_obj
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/emu68
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(emu68_obj
    PRIVATE
        file68
)

target_compile_definitions(emu68_obj
    PRIVATE
        HAVE_CONFIG_H=1
        EMU68_EXPORT=1
        EMU68_MONOLITIC=1
)

target_sources(io68_obj
	PRIVATE
		io68/io68.c
		io68/mfpemul.c
		io68/mfp_io.c
		io68/mwemul.c
		io68/mw_io.c
		io68/paulaemul.c
		io68/paula_io.c
		io68/shifter_io.c
		io68/ymemul.c
		io68/ym_blep.c
		io68/ym_dump.c
		io68/ym_envel.c
		io68/ym_io.c
		io68/ym_puls.c
)

target_include_directories(io68_obj
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/emu68
        ${CMAKE_CURRENT_SOURCE_DIR}/io68
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(io68_obj
    PRIVATE
        file68
)

target_compile_definitions(io68_obj
    PRIVATE
        HAVE_CONFIG_H=1
        IO68_EXPORT=1
)

target_sources(dial68_obj
	PRIVATE
		dial68/dial68.c
)

target_include_directories(dial68_obj
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/dial68
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(dial68_obj
    PRIVATE
        file68
)

target_compile_definitions(dial68_obj
	PRIVATE
		HAVE_CONFIG_H=1
)

target_sources(libsc68
	PRIVATE
		api68.c
		conf68.c
		libsc68.c
		mixer68.c
)

if(NOT HAVE_BASENAME)
	target_sources(libsc68
		PRIVATE
			../sc68-libc/basename.c
	)
	target_include_directories(libsc68
		PRIVATE
			../sc68-libc
	)
endif()

target_include_directories(libsc68
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/sc68
		${CMAKE_CURRENT_SOURCE_DIR}/dial68
		${CMAKE_CURRENT_SOURCE_DIR}/emu68
		${CMAKE_CURRENT_SOURCE_DIR}/io68
		
)

target_link_libraries(libsc68
    PRIVATE
		emu68_obj
		io68_obj
		dial68_obj
        file68
)

target_compile_definitions(libsc68
	PRIVATE
		LIBSC68_EXPORT=1
		HAVE_CONFIG_H=1
)
