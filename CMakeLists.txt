# See the file "COPYING" in the main distribution directory for copyright.

cmake_minimum_required(VERSION 2.8)

project(BroPluginDAG)

include(BroPlugin)

find_package(DAG REQUIRED)

include_directories(${DAG_INCLUDE_DIR})
#Hack: copy README.md to README in top level directory to keep ConfigurePackaging happy
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_SOURCE_DIR}/README COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CHANGES.md ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES COPYONLY)
bro_plugin_begin(Endace DAG)
bro_plugin_cc(src/Plugin.cc)
bro_plugin_cc(src/PktDagSrc.cc)
bro_plugin_link_library(${DAG_LIBRARIES})
bro_plugin_dist_files(broctl/lb_dag.py)
bro_plugin_dist_files(README CHANGES COPYING VERSION)
bro_plugin_end()

if (NOT TARGET uninstall)
    if (BRO_PLUGIN_BRO_PLUGIN_INSTALL_PATH)
        add_custom_target(uninstall COMMAND
            ${CMAKE_COMMAND} -E remove_directory "${BRO_PLUGIN_BRO_PLUGIN_INSTALL_PATH}/Endace_DAG"
                COMMENT "Removing: ${BRO_PLUGIN_BRO_PLUGIN_INSTALL_PATH}/Endace_DAG")
    endif ()
endif ()

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION LIMIT_COUNT 1)

# Pull the Zeek version number out of zeek-config.h, ignoring the release for now since it may not be consistent in packages.
# Annoyingly Zeek doesn't seem to record this in a cmake variable for plugins.
if (NOT ZEEK_VERSION)
    get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    find_file(ZEEK_CONFIG_H_FILEPATH NAMES "zeek-config.h" "bro-config.h" PATH_SUFFIXES "zeek" PATHS ${dirs} NO_DEFAULT_PATH)

    set(ZEEK_VERSION_DEFINE_REGEX "#define[ \t]+(ZEEK_VERSION|VERSION)[ \t]+\"(.+)\"")
    file(STRINGS "${ZEEK_CONFIG_H_FILEPATH}" ZEEK_VERSION_DEFINE REGEX "${ZEEK_VERSION_DEFINE_REGEX}")
    string(REGEX REPLACE ${ZEEK_VERSION_DEFINE_REGEX} "\\2" ZEEK_VERSION "${ZEEK_VERSION_DEFINE}")
endif ()
string(REGEX REPLACE "([^-]+).*" "\\1" ZEEK_VERSION_PART "${ZEEK_VERSION}")

message(STATUS "Zeek version         : ${ZEEK_VERSION}")

set(ZEEK_PACKAGE_NAME "zeek" CACHE STRING "Zeek package name prefix for dependencies e.g. zeek-lts")

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(CPACK_PACKAGE_NAME "${ZEEK_PACKAGE_NAME}-plugin-dag")
    set(PACKAGE_NAME_PREFIX ${CPACK_PACKAGE_NAME})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Zeek Endace DAG plugin")
    set(CPACK_PACKAGE_DESCRIPTION "Zeek endace:: packet source and zeekctl plugin that provides native support for Endace DAG and EndaceProbe Application Dock capture.")
    set(CPACK_PACKAGE_VENDOR "Endace")
    set(CPACK_PACKAGE_CONTACT "Endace support <support@endace.com>")
    # Include full description in debian package, not just summary. The leading space on the next line is required.
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n ${CPACK_PACKAGE_DESCRIPTION}")
    set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "dag-base, ${ZEEK_PACKAGE_NAME}-core (= ${ZEEK_VERSION_PART}) | ${ZEEK_PACKAGE_NAME} (= ${ZEEK_VERSION_PART})")
    set(CPACK_RPM_PACKAGE_REQUIRES "dag-base, ${ZEEK_PACKAGE_NAME} = ${ZEEK_VERSION_PART}")
    set(CPACK_DEBIAN_PACKAGE_PROVIDES "zeek-dag, bro-dag, bro-plugin-dag")
    set(CPACK_RPM_PACKAGE_PROVIDES "zeek-dag, bro-dag, bro-plugin-dag")
    set(CPACK_DEBIAN_PACKAGE_CONFLICTS "zeek-dag, bro-dag, bro-plugin-dag")
    set(CPACK_RPM_PACKAGE_CONFLICTS "zeek-dag, bro-dag, bro-plugin-dag")
    set(CPACK_DEBIAN_PACKAGE_REPLACES "bro-dag, bro-plugin-dag")
    set(CPACK_RPM_PACKAGE_OBSOLETES "bro-dag, bro-plugin-dag")
    # Only suggest broctl since some bro packages bundle it in the main bro package
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "zeekctl")
    set(CPACK_DEBIAN_PACKAGE_SECTION "net")
    set(CPACK_RPM_PACKAGE_GROUP "Productivity/Networking/Diagnostic")
    set(CPACK_DEBIAN_HOMEPAGE "https://www.endace.com/")

    # Work around CMakeRPM blindly including directories owned by other packages which is disallowed by rpm.
    # ConfigurePackaging overrides CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION, so instead replace the default list.
    list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST /usr /usr/local /opt)
    # Exclude each part of the directory tree of bro plugin path, since the bro package owns it (based on what CPackRPM does for install prefix)
    string(REPLACE "/" ";" _DIR_ELEMS ${BRO_PLUGIN_BRO_PLUGIN_INSTALL_PATH})
    foreach(_ELEM ${_DIR_ELEMS})
        list(APPEND _DIR_BUILDER_ELEMS "${_ELEM}")
        string(REPLACE ";" "/" _DIR "${_DIR_BUILDER_ELEMS}")
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "/${_DIR}")
    endforeach()

    # Allows building rpm/deb packages via "make package" in build dir.
    # TODO: The automatic RPM package needs work, using it is not recommended.
    include(ConfigurePackaging)
    ConfigurePackaging(${VERSION})
endif ()
