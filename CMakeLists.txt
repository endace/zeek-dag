
cmake_minimum_required(VERSION 2.8)

project(BroPluginDAG)

include(BroPlugin)

find_package(DAG REQUIRED)

include_directories(BEFORE ${DAG_INCLUDE_DIR})
#Hack: copy README.md to README in top level directory to keep ConfigurePackaging happy
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_SOURCE_DIR}/README COPYONLY)
bro_plugin_begin(Endace DAG)
bro_plugin_cc(src/Plugin.cc) 
bro_plugin_cc(src/PktDagSrc.cc)
bro_plugin_link_library(${DAG_LIBRARY})
bro_plugin_bif(src/dag.bif)
bro_plugin_dist_files(broctl/lb_dag.py)
bro_plugin_dist_files(README CHANGES COPYING VERSION)
bro_plugin_end()

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION LIMIT_COUNT 1)

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(CPACK_PACKAGE_NAME "bro-plugin-dag")
    set(PACKAGE_NAME_PREFIX ${CPACK_PACKAGE_NAME})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Bro endace:: packet source and broctl plugin that provides native support for Endace DAG capture cards.")
    set(CPACK_PACKAGE_VENDOR "Endace")
    set(CPACK_PACKAGE_CONTACT "Endace support <support@endace.com>")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "dag-base, bro (>= 2.5)")
    # Only suggest broctl since some bro packages bundle it in the main bro package
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "broctl")
    set(CPACK_DEBIAN_PACKAGE_SECTION "net")
    set(CPACK_DEBIAN_HOMEPAGE "https://www.endace.com/")
    # Allows building rpm/deb packages via "make package" in build dir.
    include(ConfigurePackaging)
    ConfigurePackaging(${VERSION})
endif ()
