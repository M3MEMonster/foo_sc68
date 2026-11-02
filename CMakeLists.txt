cmake_minimum_required(VERSION 3.25)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckTypeSize)

project(sc68 LANGUAGES C CXX)

if(APPLE)
enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)

if(MSVC)
add_compile_options(/Zc:strictStrings-)
endif()

if(NOT DEFINED ENV{FB2KSDK})
message(FATAL_ERROR "Unable to find the foobar2000 SDK. Please set the root directory containing foobar2000 SDK as a system environment variable called 'FB2KSDK'")
elseif(NOT EXISTS "$ENV{FB2KSDK}/foobar2000/SDK/foobar2000.h")
message(FATAL_ERROR "Unable to find the foobar2000 SDK in $ENV{FB2KSDK}")
else()
include_directories("$ENV{FB2KSDK}")
include_directories("$ENV{FB2KSDK}/foobar2000")
endif()

if(WIN32)
if("${WTL_ROOT}" STREQUAL "")
message(FATAL_ERROR "Unable to find WTL. Please set WTL_ROOT to the root directory containing WTL. Get it here: http://wtl.sourceforge.net/")
elseif(NOT EXISTS "${WTL_ROOT}/Include/atlapp.h")
message(FATAL_ERROR "Unable to find WTL in ${WTL_ROOT}")
else()
include_directories("${WTL_ROOT}/Include")
endif()
endif(WIN32)

check_include_file("stdio.h" HAVE_STDIO_H)
check_include_file("stdlib.h" HAVE_STDLIB_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("assert.h" HAVE_ASSERT_H)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
check_include_file("limits.h" HAVE_LIMITS_H)
check_include_file("string.h" HAVE_STRING_H)
check_include_file("strings.h" HAVE_STRINGS_H)
check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("wchar.h" HAVE_WCHAR_H)
check_include_file("dlfcn.h" HAVE_DLFCN_H)
check_include_file("minix/config.h" HAVE_MINIX_CONFIG_H)
check_include_file("errno.h" HAVE_ERRNO_H)
check_include_file("stdarg.h" HAVE_STDARG_H)
check_include_file("libgen.h" HAVE_LIBGEN_H)
check_include_file("windows.h" HAVE_WINDOWS_H)
check_include_file("winreg.h" HAVE_WINREG_H)
check_include_file("fcntl.h" HAVE_FCNTL_H)
check_include_file("io.h" HAVE_IO_H)
check_include_file("ctype.h" HAVE_CTYPE_H)
check_include_file("getopt.h" HAVE_GETOPT_H)

check_symbol_exists(free "stdlib.h" HAVE_FREE)
check_symbol_exists(getenv "stdlib.h" HAVE_GETENV)
check_symbol_exists(malloc "stdlib.h" HAVE_MALLOC)
check_symbol_exists(strtol "stdlib.h" HAVE_STRTOL)
check_symbol_exists(strtoul "stdlib.h" HAVE_STRTOUL)
check_symbol_exists(fileno "stdio.h" HAVE_FILENO)
check_symbol_exists(vsprintf "stdio.h" HAVE_VSPRINTF)
check_symbol_exists(vsnprintf "stdio.h" HAVE_VSNPRINTF)
check_symbol_exists(stpcpy "string.h" HAVE_STPCPY)
check_symbol_exists(basename "libgen.h" HAVE_BASENAME)
check_symbol_exists(fsync "unistd.h" HAVE_FSYNC)
check_symbol_exists(getopt "unistd.h" HAVE_GETOPT)
check_symbol_exists(getopt_long "getopt.h" HAVE_GETOPT_LONG)
check_struct_has_member("struct option" name "getopt.h" HAVE_STRUCT_OPTION)
if(WIN32)
check_symbol_exists(Sleep "windows.h" HAVE_SLEEP)
else()
check_symbol_exists(sleep "unistd.h" HAVE_SLEEP)
endif()
check_symbol_exists(usleep "unistd.h" HAVE_USLEEP)
check_symbol_exists(ao_file_extension "ao/ao.h" HAVE_AO_FILE_EXTENSION)

check_symbol_exists(fdatasync "unistd.h" HAVE_FDATASYNC)
if(NOT HAVE_FDATASYNC)
unset(HAVE_FDATASYNC CACHE)
set(CMAKE_REQUIRED_LIBRARIES "rt")
check_symbol_exists(fdatasync "unistd.h" HAVE_FDATASYNC)
unset(CMAKE_REQUIRED_LIBRARIES)
endif()

if(WIN32)
    check_symbol_exists(_fileno "stdio.h" HAVE__FILENO)
    check_symbol_exists(_setmode "fcntl.h" HAVE__SETMODE)
	set(NATIVE_WIN32 TRUE)
endif()

check_type_size(intmax_t HAVE_INTMAX_T)
check_type_size(uintmax_t HAVE_UINTMAX_T)
check_type_size("long long" HAVE_LONG_LONG_INT)
check_type_size("unsigned long long" HAVE_UNSIGNED_LONG_LONG_INT)

check_c_source_compiles("__declspec(dllexport) 
int test_func() { return 0; }
int main() { return test_func(); }" HAVE_DECLSPEC)

# TEST for if the system supports filename > 14 chars
set(_test_file "${CMAKE_CURRENT_BINARY_DIR}/this_is_a_very_long_filename_test_file.txt")
file(WRITE "${_test_file}")
if(EXISTS "${_test_file}")
set(HAVE_LONG_FILE_NAMES TRUE)
message(STATUS "LONG FILE NAMES: YES")
else()
set(HAVE_LONG_FILE_NAMES FALSE)
message(STATUS "LONG FILE NAMES: NO")
endif()
file(REMOVE "${_test_file}")
# TEST END
if(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY)
set(HAVE_VISIBILITY TRUE)
message(STATUS "VISIBILITY: YES")
else()
set(HAVE_VISIBILITY FALSE)
message(STATUS "VISIBILITY: NO")
endif()



configure_file(common_config.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/common_config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(zlib)
add_subdirectory(unice68)
add_subdirectory(file68)
add_subdirectory(libsc68)
add_subdirectory(sc68)
add_subdirectory(desa68)
add_subdirectory(info68)
add_subdirectory(sourcer68)
add_subdirectory(as68)


add_library(foo_sc68 MODULE)

target_sources(foo_sc68
	PRIVATE
		sc68-fb2k/foo_sc68/fb2k_sc68.cpp
		sc68-fb2k/foo_sc68/global_config.cpp
		sc68-fb2k/foo_sc68/initquit_sc68.cpp
		sc68-fb2k/foo_sc68/input_sc68.cpp
		sc68-fb2k/foo_sc68/stdafx.cpp
		sc68-fb2k/foo_sc68/vfs_sc68.cpp
)

target_include_directories(foo_sc68
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)


if(WIN32)

if(CMAKE_SIZEOF_VOID_P EQUAL 8) # Judge the platform
set(ARCH "x64/")
set(ARCH2 "-x64")
else()
set(ARCH "")
set(ARCH2 "-Win32")
endif()

set(FOOBAR_SDK_LIBRARIES
	optimized "$ENV{FB2KSDK}/foobar2000/SDK/${ARCH}Release/foobar2000_SDK.lib"
	optimized "$ENV{FB2KSDK}/foobar2000/foobar2000_component_client/${ARCH}Release/foobar2000_component_client.lib"
	optimized "$ENV{FB2KSDK}/foobar2000/helpers/${ARCH}Release/foobar2000_sdk_helpers.lib"
	optimized "$ENV{FB2KSDK}/foobar2000/shared/shared${ARCH2}.lib"
	optimized "$ENV{FB2KSDK}/pfc/${ARCH}Release/pfc.lib"
	optimized "$ENV{FB2KSDK}/libPPUI/${ARCH}Release/libPPUI.lib"
	debug "$ENV{FB2KSDK}/foobar2000/SDK/${ARCH}Debug/foobar2000_SDK.lib"
	debug "$ENV{FB2KSDK}/foobar2000/foobar2000_component_client/${ARCH}Debug/foobar2000_component_client.lib"
	debug "$ENV{FB2KSDK}/foobar2000/helpers/${ARCH}Debug/foobar2000_sdk_helpers.lib"
	debug "$ENV{FB2KSDK}/foobar2000/shared/shared${ARCH2}.lib"
	debug "$ENV{FB2KSDK}/pfc/${ARCH}Debug/pfc.lib"
	debug "$ENV{FB2KSDK}/libPPUI/${ARCH}Debug/libPPUI.lib"
)

target_sources(foo_sc68 PRIVATE sc68-fb2k/foo_sc68/pref_sc68_win32.cpp sc68-fb2k/foo_sc68/pref_sc68_win32.rc)
target_compile_definitions(foo_sc68 PRIVATE UNICODE _UNICODE)

elseif(APPLE)

set(FOOBAR_SDK_LIBRARIES
	optimized "$ENV{FB2KSDK}/foobar2000/SDK/build/Release/libfoobar2000_SDK.a"
	optimized "$ENV{FB2KSDK}/foobar2000/foobar2000_component_client/build/Release/libfoobar2000_component_client.a"
	optimized "$ENV{FB2KSDK}/foobar2000/helpers/build/Release/libfoobar2000_SDK_helpers.a"
	optimized "$ENV{FB2KSDK}/foobar2000/shared/build/Release/libshared.a"
	optimized "$ENV{FB2KSDK}/pfc/build/Release/libpfc-Mac.a"
	debug "$ENV{FB2KSDK}/foobar2000/SDK/build/Debug/libfoobar2000_SDK.a"
	debug "$ENV{FB2KSDK}/foobar2000/foobar2000_component_client/build/Debug/libfoobar2000_component_client.a"
	debug "$ENV{FB2KSDK}/foobar2000/helpers/build/Debug/libfoobar2000_SDK_helpers.a"
	debug "$ENV{FB2KSDK}/foobar2000/shared/build/Debug/libshared.a"
	debug "$ENV{FB2KSDK}/pfc/build/Debug/libpfc-Mac.a"
)
target_sources(foo_sc68 PRIVATE sc68-fb2k/foo_sc68/pref_sc68_mac.mm sc68-fb2k/foo_sc68/pref_sc68_mac.h sc68-fb2k/foo_sc68/pref_sc68_mac.xib)


endif()

if(APPLE)
set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
target_link_libraries(foo_sc68 "-framework CoreFoundation" "-framework Cocoa")
set_target_properties(foo_sc68 PROPERTIES
	BUNDLE True
	BUNDLE_EXTENSION component
	MACOSX_BUNDLE_BUNDLE_NAME foo_sc68
	MACOSX_BUNDLE_BUNDLE_VERSION 1.0.10
	MACOSX_BUNDLE_GUI_IDENTIFIER com.ceroler.foo-sc68
	MACOSX_BUNDLE_ICON_FILE AppIcon
	MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.10"
	RESOURCE "sc68-fb2k/foo_sc68/pref_sc68_mac.xib"
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
	XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.ceroler.foo-sc68"
)
endif()

target_link_libraries(foo_sc68 zlib unice68 file68 libsc68 ${FOOBAR_SDK_LIBRARIES})
