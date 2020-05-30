cmake_minimum_required(VERSION 3.16)
project(dfa
        VERSION 0.0.1
        LANGUAGES CXX
        DESCRIPTION "DFA simulator."
        )
message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")

# Options related to weed whacking, testing, and documentation.
option(DFA_ENABLE_ALLWARNINGS "Add GCC/Clang compatible compile options." OFF)
option(DFA_BUILD_TESTING "Enable unit testing." OFF)
option(DFA_BUILD_DOCUMENTATION "Generate Doxygen documentation." ON)
option(DFA_BUILD_EXECUTABLE "Build DFA executable." ON)

# Set configuration: Either Debug, Release (default), MinSizeRel, or RelWithDebInfo.
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(default_build_type "Release")
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
            STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

# Set C++17 unless specified otherwise.
if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    message(STATUS "${PROJECT_NAME} using CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
endif ()

# Set compiler warning flags if option is on, and using GCC/Clang.
if (DFA_ENABLE_ALLWARNINGS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(DFA_EXTRA_COMPILE_OPTIONS
                -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual
                -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2
                -Winit-self -Wmissing-include-dirs -Wold-style-cast
                -Woverloaded-virtual -Wredundant-decls -Wshadow
                -Wsign-promo -Wswitch-default -Wundef -Wno-unused
                CACHE STRING "Add extra compile options to the CXX targets")
    endif ()
endif ()

message(STATUS "DFA_EXTRA_COMPILE_OPTIONS: ${DFA_EXTRA_COMPILE_OPTIONS}")
message(STATUS "DFA_BUILD_TESTING: ${DFA_BUILD_TESTING}")

# Set library headers and sources.
set(dfa_headers
        dfa.h
        )
set(dfa_sources
        dfa.cc
        )

# Specify source directory.
list(TRANSFORM dfa_headers PREPEND "dfa/")
list(TRANSFORM dfa_sources PREPEND "dfa/")

# Find dependencies.
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Doxygen)

# Create shared library.
add_library(dfa SHARED ${dfa_sources})
include(GNUInstallDirs) # Define LIBDIR, INCLUDEDIR, DOCDIR
target_include_directories(dfa PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
target_compile_options(dfa PRIVATE ${DFA_EXTRA_COMPILE_OPTIONS})

set(DFA_LIBRARIES dfa)

# Create executable if specified.
if (DFA_BUILD_EXECUTABLE)
    set(dfash_sources
            main.cc
            )

    list(TRANSFORM dfash_sources PREPEND "dfa/")

    add_executable(dfash ${dfash_sources})
    target_include_directories(dfash PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_BINDIR}>
            )
    target_compile_options(dfash PRIVATE ${DFA_EXTRA_COMPILE_OPTIONS})
    target_compile_options(dfash PRIVATE ${DFA_EXTRA_COMPILE_OPTIONS})

    target_link_libraries(dfash dfa nlohmann_json::nlohmann_json)

    install(TARGETS dfash
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT runtime
            )
endif ()

# Link dependencies
target_link_libraries(dfa nlohmann_json::nlohmann_json)

# Install shared object.
install(TARGETS ${DFA_LIBRARIES}
        EXPORT dfaTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
        )

# Install public header(s).
install(FILES ${dfa_headers}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dfa
        COMPONENT development
        )

# Export to the build tree.
set(dfa_export_file "${PROJECT_BINARY_DIR}/dfaTargets.cmake")
export(TARGETS ${DFA_LIBRARIES}
        NAMESPACE dfa::
        APPEND FILE ${dfa_export_file}
        )

# Build GTest if unit testing enabled.
if (DFA_BUILD_TESTING)
    enable_testing()
    set(INSTALL_GTEST OFF)
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # Propagate INSTALL_GTEST=OFF to subproject
    set(GTEST_LIBRARIES gtest gtest_main)
    include(GoogleTest)
    #############################################################################
    # Fetch GTest
    include(FetchContent)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.8.x
    )

    FetchContent_GetProperties(googletest)
    if (NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    endif ()

    add_subdirectory(dfa/test)
endif ()

if (DFA_BUILD_DOCUMENTATION)
    if (NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif ()

    # Don't try to generate docs for testing.
    set(DOXYGEN_RECURSIVE NO)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
    doxygen_add_docs(
            doc
            ALL
            ${CMAKE_CURRENT_SOURCE_DIR}/dfa
            ${CMAKE_CURRENT_SOURCE_DIR}/README.md
            ${CMAKE_CURRENT_SOURCE_DIR}/.github/CONTRIBUTING.md
            COMMENT "Generate documentation"
    )

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
            COMPONENT development
            )
endif ()

# Install
set(install_cmake_dir "${CMAKE_INSTALL_LIBDIR}/cmake/dfa")

install(EXPORT dfaTargets
        NAMESPACE dfa::
        DESTINATION ${install_cmake_dir}
        )

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/dfaConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/dfaConfigVersion.cmake
        DESTINATION ${install_cmake_dir}
        )

include(CMakePackageConfigHelpers)

write_basic_package_version_file(dfaConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
        )

# Build tree
set(dfa_TARGETS_FILE ${dfa_export_file})
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dfaConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/dfaConfig.cmake
        INSTALL_DESTINATION ${install_cmake_dir}
        PATH_VARS dfa_TARGETS_FILE
        NO_CHECK_REQUIRED_COMPONENTS_MACRO # dfa does not provide components
        INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}
)

# Install tree
set(dfa_TARGETS_FILE ${CMAKE_INSTALL_PREFIX}/${install_cmake_dir}/dfaTargets.cmake)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dfaConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/dfaConfig.cmake
        INSTALL_DESTINATION ${install_cmake_dir}
        PATH_VARS dfa_TARGETS_FILE
        NO_CHECK_REQUIRED_COMPONENTS_MACRO # dfa does not provide components
)

# Add custom target to only install component: runtime (libraries)
add_custom_target(dfa-install-runtime
        ${CMAKE_COMMAND}
        -DCMAKE_INSTALL_COMPONENT=runtime
        -P "${PROJECT_BINARY_DIR}/cmake_install.cmake"
        DEPENDS ${DFA_LIBRARIES}
        )
message(STATUS "DFA_LIBRARIES: ${DFA_LIBRARIES}")
add_dependencies(dfa-install-runtime ${DFA_LIBRARIES})
