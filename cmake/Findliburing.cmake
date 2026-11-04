if(TARGET liburing::liburing)
    return()
endif()

find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(PC_LIBURING QUIET liburing)
endif()

find_path(LIBURING_INCLUDE_DIR
    NAMES liburing.h
    HINTS
        ${PC_LIBURING_INCLUDEDIR}
        ${PC_LIBURING_INCLUDE_DIRS}
        ${LIBURING_ROOT}
        ENV LIBURING_ROOT
    PATH_SUFFIXES include
)

find_library(LIBURING_LIBRARY
    NAMES uring liburing
    HINTS
        ${PC_LIBURING_LIBDIR}
        ${PC_LIBURING_LIBRARY_DIRS}
        ${LIBURING_ROOT}
        ENV LIBURING_ROOT
    PATH_SUFFIXES lib lib64
)

if(PC_LIBURING_VERSION)
    set(liburing_VERSION "${PC_LIBURING_VERSION}")
elseif(LIBURING_INCLUDE_DIR AND EXISTS "${LIBURING_INCLUDE_DIR}/liburing/io_uring_version.h")
    file(READ "${LIBURING_INCLUDE_DIR}/liburing/io_uring_version.h" _liburing_version_h)
    string(REGEX MATCH "IO_URING_VERSION_MAJOR[ \t]+([0-9]+)" _ "${_liburing_version_h}")
    set(_liburing_major "${CMAKE_MATCH_1}")
    string(REGEX MATCH "IO_URING_VERSION_MINOR[ \t]+([0-9]+)" _ "${_liburing_version_h}")
    set(_liburing_minor "${CMAKE_MATCH_1}")
    if(_liburing_major AND _liburing_minor)
        set(liburing_VERSION "${_liburing_major}.${_liburing_minor}")
    endif()
    unset(_liburing_version_h)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(liburing
    REQUIRED_VARS LIBURING_LIBRARY LIBURING_INCLUDE_DIR
    VERSION_VAR liburing_VERSION
    FAIL_MESSAGE
        "liburing не найден. Ubuntu/Debian: apt install liburing-dev, Fedora: dnf install liburing-devel, либо укажите -DLIBURING_ROOT=/path/to/liburing"
)

if(liburing_FOUND)
    add_library(liburing::liburing UNKNOWN IMPORTED)
    set_target_properties(liburing::liburing PROPERTIES
        IMPORTED_LOCATION "${LIBURING_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBURING_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(LIBURING_INCLUDE_DIR LIBURING_LIBRARY)
