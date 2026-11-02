add_library(zlib STATIC)

if(HAVE_UNISTD_H)
    target_compile_definitions(zlib PRIVATE HAVE_UNISTD_H=1)
endif()

target_sources(zlib
	PRIVATE
		adler32.c
		compress.c
		crc32.c
		deflate.c
		gzclose.c
		gzlib.c
		gzread.c
		gzwrite.c
		infback.c
		inffast.c
		inflate.c
		inftrees.c
		trees.c
		uncompr.c
		zutil.c
)

target_include_directories(zlib
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)


