add_executable(as68 as68.c error.c expression.c opcode.c word.c)

target_include_directories(as68
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
