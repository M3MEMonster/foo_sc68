add_executable(sourcer68_cli)

target_sources(sourcer68_cli
	PRIVATE
	sourcer68.c
	src68_exe.c
	src68_eva.c
	src68_sec.c
	src68_mbk.c
	src68_tos.c
	src68_sym.c
	src68_opt.c
	src68_msg.c
	src68_vec.c
	src68_adr.c
	src68_dis.c
	src68_src.c
	src68_fmt.c
	../getopt-win/getopt.c
)

configure_file(
	config.h.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

target_include_directories(sourcer68_cli
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(sourcer68_cli
	PRIVATE
	file68
	desa68
)

target_compile_definitions(sourcer68_cli
	PRIVATE
	HAVE_CONFIG_H=1
)