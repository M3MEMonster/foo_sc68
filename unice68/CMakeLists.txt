include(CheckIncludeFile)

configure_file(
	config.h.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_library(unice68 STATIC
    unice68_pack.c
    unice68_unpack.c
	unice68_version.c
)

target_include_directories(unice68
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(unice68
	PRIVATE
		UNICE68_EXPORT=1
		HAVE_CONFIG_H=1
)

add_executable(unice68_cli unice68.c)

target_include_directories(unice68_cli
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(unice68_cli
	PRIVATE
	unice68
)

target_compile_definitions(unice68_cli
	PRIVATE
	HAVE_CONFIG_H=1
)

set_target_properties(unice68_cli PROPERTIES OUTPUT_NAME unice68)