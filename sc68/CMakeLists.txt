add_executable(sc68_cli sc68.c ../getopt-win/getopt.c)

configure_file(
	config.h.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

target_include_directories(sc68_cli 
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(sc68_cli
	PRIVATE
	HAVE_CONFIG_H=1
)

target_link_libraries(sc68_cli
	PRIVATE
		file68
		libsc68
)

