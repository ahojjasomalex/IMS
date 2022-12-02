find_path(simlib_ROOT_DIR
        NAMES include/simlib.h
        )

find_path(simlib_INCLUDE_DIR
        NAMES simlib.h
        HINTS ${simlib_ROOT_DIR}/include
        )

find_library(simlib_LIBRARY
        NAMES simlib
        HINTS ${simlib_ROOT_DIR}/lib
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(simlib DEFAULT_MSG
        simlib_LIBRARY
        simlib_INCLUDE_DIR
        )

include(CheckCSourceCompiles)
set(CMAKE_REQUIRED_LIBRARIES ${simlib_LIBRARY})
check_c_source_compiles("int main() { return 0; }" simlib_LINKS_SOLO)
set(CMAKE_REQUIRED_LIBRARIES)

# check if linking against libsimlib also needs to link against a thread library
if (NOT simlib_LINKS_SOLO)
    find_package(Threads)
    if (THREADS_FOUND)
        set(CMAKE_REQUIRED_LIBRARIES ${simlib_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
        check_c_source_compiles("int main() { return 0; }" simlib_NEEDS_THREADS)
        set(CMAKE_REQUIRED_LIBRARIES)
    endif ()
    if (THREADS_FOUND AND simlib_NEEDS_THREADS)
        set(_tmp ${simlib_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
        list(REMOVE_DUPLICATES _tmp)
        set(simlib_LIBRARY ${_tmp}
                CACHE STRING "Libraries needed to link against libsimlib" FORCE)
    else ()
        message(FATAL_ERROR "Couldn't determine how to link against libsimlib")
    endif ()
endif ()

include(CheckFunctionExists)
set(CMAKE_REQUIRED_LIBRARIES ${simlib_LIBRARY})
check_function_exists(simlib_get_pfring_id HAVE_PF_RING)
check_function_exists(simlib_dump_open_append HAVE_simlib_DUMP_OPEN_APPEND)
set(CMAKE_REQUIRED_LIBRARIES)

mark_as_advanced(
        simlib_ROOT_DIR
        simlib_INCLUDE_DIR
        simlib_LIBRARY
)