add_executable(info68_cli info68.c)

configure_file(
	config.h.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

target_include_directories(info68_cli
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(info68_cli
	PRIVATE
	HAVE_CONFIG_H=1
)

target_link_libraries(info68_cli
	PRIVATE
	file68
	desa68
)


