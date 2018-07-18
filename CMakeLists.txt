# See the file "COPYING" in the main distribution directory for copyright.

cmake_minimum_required(VERSION 2.8)

project(BroPluginDAG)

include(BroPlugin)

find_package(DAG REQUIRED)

include_directories(BEFORE ${DAG_INCLUDE_DIR})
#Hack: copy README.md to README in top level directory to keep ConfigurePackaging happy
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_SOURCE_DIR}/README COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CHANGES.md ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES COPYONLY)
bro_plugin_begin(Endace DAG)
bro_plugin_cc(src/Plugin.cc)
bro_plugin_cc(src/PktDagSrc.cc)
bro_plugin_link_library(${DAG_LIBRARIES})
bro_plugin_bif(src/dag.bif)
bro_plugin_dist_files(broctl/lb_dag.py)
bro_plugin_dist_files(README CHANGES COPYING VERSION)
bro_plugin_end()

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION LIMIT_COUNT 1)

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(CPACK_PACKAGE_NAME "bro-plugin-dag")
    set(PACKAGE_NAME_PREFIX ${CPACK_PACKAGE_NAME})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Bro Endace DAG plugin")
    set(CPACK_PACKAGE_DESCRIPTION "Bro endace:: packet source and broctl plugin that provides native support for Endace DAG capture cards.")
    set(CPACK_PACKAGE_VENDOR "Endace")
    set(CPACK_PACKAGE_CONTACT "Endace support <support@endace.com>")
    # Include full description in debian package, not just summary. The leading space on the next line is required.
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n ${CPACK_PACKAGE_DESCRIPTION}")
    set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "dag-base, bro (>= 2.5)")
    set(CPACK_RPM_PACKAGE_REQUIRES "dag-base, bro >= 2.5")
    # Only suggest broctl since some bro packages bundle it in the main bro package
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "broctl")
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
