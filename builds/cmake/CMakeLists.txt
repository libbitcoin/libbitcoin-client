###############################################################################
#  Copyright (c) 2014-2023 libbitcoin-client developers (see COPYING).
#
#         GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
#
###############################################################################

# libbitcoin-client project configuration.
#------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(libbitcoin-client LANGUAGES C CXX)

enable_testing()

list( APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules" )
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckLinkerFlag)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
  set( CANONICAL_LIB_NAME libbitcoin-client )
else ()
  set( CANONICAL_LIB_NAME bitcoin-client )
  find_package( PkgConfig REQUIRED )

  set( prefix "${CMAKE_PREFIX_PATH}" )
  set( exec_prefix "\${prefix}" )
  set( libdir "\${exec_prefix}/lib" )
  set( includedir "\${exec_prefix}/include" )

  set( PACKAGE_VERSION "3.8.0" )
  set( VERSION "${PACKAGE_VERSION}" )
endif ()

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# Add compiler options
#------------------------------------------------------------------------------
# Warn on all stuff.
check_cxx_compiler_flag( "-Wall" HAS_FLAG_WALL )
if ( HAS_FLAG_WALL )
    add_compile_options( "-Wall" )
else()
    message( FATAL_ERROR "Compiler does not support -Wall" )
endif()

# Warn on extra stuff.
check_cxx_compiler_flag( "-Wextra" HAS_FLAG_WEXTRA )
if ( HAS_FLAG_WEXTRA )
    add_compile_options( "-Wextra" )
else()
    message( FATAL_ERROR "Compiler does not support -Wextra" )
endif()

# Be really annoying.
check_cxx_compiler_flag( "-Wpedantic" HAS_FLAG_WPEDANTIC )
if ( HAS_FLAG_WPEDANTIC )
    add_compile_options( "-Wpedantic" )
else()
    message( FATAL_ERROR "Compiler does not support -Wpedantic" )
endif()

# Disallow warning on style order of declarations.
check_cxx_compiler_flag( "-Wno-reorder" HAS_FLAG_WNO-REORDER )
if ( HAS_FLAG_WNO-REORDER )
    add_compile_options( "-Wno-reorder" )
else()
    message( FATAL_ERROR "Compiler does not support -Wno-reorder" )
endif()

# Suppress warning for incomplete field initialization.
check_cxx_compiler_flag( "-Wno-missing-field-initializers" HAS_FLAG_WNO-MISSING-FIELD-INITIALIZERS )
if ( HAS_FLAG_WNO-MISSING-FIELD-INITIALIZERS )
    add_compile_options( "-Wno-missing-field-initializers" )
else()
    message( FATAL_ERROR "Compiler does not support -Wno-missing-field-initializers" )
endif()

# Conform to style.
check_cxx_compiler_flag( "-Wno-missing-braces" HAS_FLAG_WNO-MISSING-BRACES )
if ( HAS_FLAG_WNO-MISSING-BRACES )
    add_compile_options( "-Wno-missing-braces" )
else()
    message( FATAL_ERROR "Compiler does not support -Wno-missing-braces" )
endif()

# Ignore comments within comments or commenting of backslash extended lines.
check_cxx_compiler_flag( "-Wno-comment" HAS_FLAG_WNO-COMMENT )
if ( HAS_FLAG_WNO-COMMENT )
    add_compile_options( "-Wno-comment" )
else()
    message( FATAL_ERROR "Compiler does not support -Wno-comment" )
endif()

# Suppress warning for copy of implicitly generated copy constructor.
check_cxx_compiler_flag( "-Wno-deprecated-copy" HAS_FLAG_WNO-DEPRECATED-COPY )
if ( HAS_FLAG_WNO-DEPRECATED-COPY )
    add_compile_options( "-Wno-deprecated-copy" )
else()
    message( FATAL_ERROR "Compiler does not support -Wno-deprecated-copy" )
endif()

# Conflict in stdlib under clang.
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    check_cxx_compiler_flag( "-Wno-mismatched-tags" HAS_FLAG_WNO-MISMATCHED-TAGS )
    if ( HAS_FLAG_WNO-MISMATCHED-TAGS )
        add_compile_options( "-Wno-mismatched-tags" )
    else()
        message( FATAL_ERROR "Compiler does not support -Wno-mismatched-tags" )
    endif()
endif()

# Implement -Dpkgconfigdir and output ${pkgconfigdir}.
#------------------------------------------------------------------------------
set( pkgconfigdir "${libdir}/pkgconfig" CACHE PATH "Path to pkgconfig directory." )

# Implement -Dwith-tests and declare with-tests.
#------------------------------------------------------------------------------
set( with-tests "yes" CACHE BOOL "Compile with unit tests." )

# Implement -Dwith-examples and declare with-examples.
#------------------------------------------------------------------------------
set( with-examples "yes" CACHE BOOL "Compile with examples." )

# Implement -Denable-ndebug and define NDEBUG.
#------------------------------------------------------------------------------
set( enable-ndebug "yes" CACHE BOOL "Compile without debug assertions." )

if (enable-ndebug)
    add_definitions( -DNDEBUG )
endif()

# Inherit -Denable-shared and define BOOST_ALL_DYN_LINK.
#------------------------------------------------------------------------------
if (BUILD_SHARED_LIBS)
    add_definitions( -DBOOST_ALL_DYN_LINK )
endif()

if (BUILD_SHARED_LIBS)
    set( Boost_USE_STATIC_LIBS "off" )
else()
    set( Boost_USE_STATIC_LIBS "on" )
endif()

# Find boost
#------------------------------------------------------------------------------
find_package( Boost 1.73.0 REQUIRED COMPONENTS
    unit_test_framework )

set( boost_unit_test_framework_LIBS "-lboost_unit_test_framework" )

set( Boost_LIBRARY_DIR $<IF:$<BOOL:${enable-ndebug}>,"${Boost_LIBRARY_DIR_RELEASE}","${Boost_LIBRARY_DIR_DEBUG}">)

set( boost_CPPFLAGS "-I${Boost_INCLUDE_DIR}" )
set( boost_LDFLAGS "-L${Boost_LIBRARY_DIR}" )

# Find bitcoin-system
#------------------------------------------------------------------------------
find_package( Bitcoin-System 3.8.0 REQUIRED )

# Find bitcoin-protocol
#------------------------------------------------------------------------------
find_package( Bitcoin-Protocol 3.8.0 REQUIRED )

# Define project common includes for build.
#------------------------------------------------------------------------------
if (BUILD_SHARED_LIBS)
    set( bitcoin_system_FOR_BUILD_INCLUDE_DIRS ${bitcoin_system_INCLUDE_DIRS} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_INCLUDE_DIRS ${bitcoin_protocol_INCLUDE_DIRS} CACHE STRING "Placeholder" )
else()
    set( bitcoin_system_FOR_BUILD_INCLUDE_DIRS ${bitcoin_system_STATIC_INCLUDE_DIRS} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_INCLUDE_DIRS ${bitcoin_protocol_STATIC_INCLUDE_DIRS} CACHE STRING "Placeholder" )
endif()

# Define project common includes directories
#------------------------------------------------------------------------------
include_directories( SYSTEM
    ${bitcoin_system_FOR_BUILD_INCLUDE_DIRS}
    ${bitcoin_protocol_FOR_BUILD_INCLUDE_DIRS} )

# Define project common library directories for build.
#------------------------------------------------------------------------------
if (BUILD_SHARED_LIBS)
    set( bitcoin_system_FOR_BUILD_LIBRARY_DIRS ${bitcoin_system_LIBRARY_DIRS} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_LIBRARY_DIRS ${bitcoin_protocol_LIBRARY_DIRS} CACHE STRING "Placeholder" )
else()
    set( bitcoin_system_FOR_BUILD_LIBRARY_DIRS ${bitcoin_system_STATIC_LIBRARY_DIRS} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_LIBRARY_DIRS ${bitcoin_protocol_STATIC_LIBRARY_DIRS} CACHE STRING "Placeholder" )
endif()

# Define project common library directories
#------------------------------------------------------------------------------
link_directories(
    ${bitcoin_system_FOR_BUILD_LIBRARY_DIRS}
    ${bitcoin_protocol_FOR_BUILD_LIBRARY_DIRS} )

# Define project common linker flags.
#------------------------------------------------------------------------------
check_linker_flag(CXX "-no_fixup_chains" "no_fixup_chains_FOUND")

if (no_fixup_chains_FOUND)
    add_link_options("-no_fixup_chains")
endif()

check_linker_flag(CXX "-fstack-protector" "fstack_protector_FOUND")

if (fstack_protector_FOUND)
    add_link_options("-fstack-protector")
endif()

check_linker_flag(CXX "-fstack-protector-all" "fstack_protector_all_FOUND")

if (fstack_protector_all_FOUND)
    add_link_options("-fstack-protector-all")
endif()


# Define common library usage for build.
#------------------------------------------------------------------------------
if (BUILD_SHARED_LIBS)
    set( bitcoin_system_FOR_BUILD_LIBRARIES ${bitcoin_system_LIBRARIES} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_LIBRARIES ${bitcoin_protocol_LIBRARIES} CACHE STRING "Placeholder" )
else()
    set( bitcoin_system_FOR_BUILD_LIBRARIES ${bitcoin_system_STATIC_LIBRARIES} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_LIBRARIES ${bitcoin_protocol_STATIC_LIBRARIES} CACHE STRING "Placeholder" )
endif()

# Define project common libraries.
#------------------------------------------------------------------------------
link_libraries(
    ${bitcoin_system_FOR_BUILD_LIBRARIES}
    ${bitcoin_protocol_FOR_BUILD_LIBRARIES} )

# Define ${CANONICAL_LIB_NAME} project.
#------------------------------------------------------------------------------
add_library( ${CANONICAL_LIB_NAME}
    "../../src/dealer.cpp"
    "../../src/obelisk_client.cpp"
    "../../src/proxy.cpp"
    "../../src/server_codec_impl.cpp.bak"
    "../../src/socket_stream.cpp"
    "../../src/uniform_uint32_generator.cpp.bak" )

# ${CANONICAL_LIB_NAME} project specific include directory normalization for build.
#------------------------------------------------------------------------------
if (BUILD_SHARED_LIBS)
    set( bitcoin_system_FOR_BUILD_INCLUDE_DIRS ${bitcoin_system_INCLUDE_DIRS} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_INCLUDE_DIRS ${bitcoin_protocol_INCLUDE_DIRS} CACHE STRING "Placeholder" )
else()
    set( bitcoin_system_FOR_BUILD_INCLUDE_DIRS ${bitcoin_system_STATIC_INCLUDE_DIRS} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_INCLUDE_DIRS ${bitcoin_protocol_STATIC_INCLUDE_DIRS} CACHE STRING "Placeholder" )
endif()

# ${CANONICAL_LIB_NAME} project specific include directories.
#------------------------------------------------------------------------------
target_include_directories( ${CANONICAL_LIB_NAME} PRIVATE
    "../../include"
    ${bitcoin_system_FOR_BUILD_INCLUDE_DIRS}
    ${bitcoin_protocol_FOR_BUILD_INCLUDE_DIRS} )

target_include_directories( ${CANONICAL_LIB_NAME} PUBLIC
"../../include" )

# ${CANONICAL_LIB_NAME} project specific libraries noramalization for build.
#------------------------------------------------------------------------------
if (BUILD_SHARED_LIBS)
    set( bitcoin_system_FOR_BUILD_LIBRARIES ${bitcoin_system_LIBRARIES} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_LIBRARIES ${bitcoin_protocol_LIBRARIES} CACHE STRING "Placeholder" )
else()
    set( bitcoin_system_FOR_BUILD_LIBRARIES ${bitcoin_system_STATIC_LIBRARIES} CACHE STRING "Placeholder" )
    set( bitcoin_protocol_FOR_BUILD_LIBRARIES ${bitcoin_protocol_STATIC_LIBRARIES} CACHE STRING "Placeholder" )
endif()

# ${CANONICAL_LIB_NAME} project specific libraries/linker flags.
#------------------------------------------------------------------------------
target_link_libraries( ${CANONICAL_LIB_NAME}
    ${bitcoin_system_FOR_BUILD_LIBRARIES}
    ${bitcoin_protocol_FOR_BUILD_LIBRARIES} )

# Define libbitcoin-client-test project.
#------------------------------------------------------------------------------
if (with-tests)
    add_executable( libbitcoin-client-test
        "../../test/main.cpp"
        "../../test/proxy.cpp" )

    add_test( NAME libbitcoin-client-test COMMAND libbitcoin-client-test
            --run_test=*
            --show_progress=no
            --detect_memory_leak=0
            --report_level=no
            --build_info=yes )

#     libbitcoin-client-test project specific include directories.
#------------------------------------------------------------------------------
    target_include_directories( libbitcoin-client-test PRIVATE
        "../../include" )

#     libbitcoin-client-test project specific libraries/linker flags.
#------------------------------------------------------------------------------
    target_link_libraries( libbitcoin-client-test
        ${CANONICAL_LIB_NAME}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )

endif()

# Define console project.
#------------------------------------------------------------------------------
if (with-examples)
    add_executable( console
        "../../examples/console/client.cpp"
        "../../examples/console/client.hpp"
        "../../examples/console/connection.cpp"
        "../../examples/console/connection.hpp"
        "../../examples/console/main.cpp"
        "../../examples/console/read_line.cpp"
        "../../examples/console/read_line.hpp" )

#     console project specific include directories.
#------------------------------------------------------------------------------
    target_include_directories( console PRIVATE
        "../../include" )

#     console project specific libraries/linker flags.
#------------------------------------------------------------------------------
    target_link_libraries( console
        ${CANONICAL_LIB_NAME} )

endif()

# Define get_height project.
#------------------------------------------------------------------------------
if (with-examples)
    add_executable( get_height
        "../../examples/get_height/main.cpp" )

#     get_height project specific include directories.
#------------------------------------------------------------------------------
    target_include_directories( get_height PRIVATE
        "../../include" )

#     get_height project specific libraries/linker flags.
#------------------------------------------------------------------------------
    target_link_libraries( get_height
        ${CANONICAL_LIB_NAME} )

endif()

# Manage pkgconfig installation.
#------------------------------------------------------------------------------
configure_file(
  "../../libbitcoin-client.pc.in"
  "libbitcoin-client.pc" @ONLY )

install( FILES
    "${CMAKE_CURRENT_BINARY_DIR}/libbitcoin-client.pc"
    DESTINATION "${pkgconfigdir}" )

# Manage installation of docs.
#------------------------------------------------------------------------------
install( FILES
    "../../AUTHORS"
    "../../COPYING"
    "../../ChangeLog"
    "../../INSTALL"
    "../../NEWS"
    "../../README"
    DESTINATION share/doc/libbitcoin-client )

# Manage ${CANONICAL_LIB_NAME} installation.
#------------------------------------------------------------------------------
install( TARGETS ${CANONICAL_LIB_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include )

# Manage include installation.
#------------------------------------------------------------------------------
install( DIRECTORY "../../include/bitcoin"
    DESTINATION include )

