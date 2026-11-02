add_library(desa68 STATIC desa68.c)

configure_file(
	config.h.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

target_include_directories(desa68
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(desa68
	PRIVATE
	HAVE_CONFIG_H=1
)