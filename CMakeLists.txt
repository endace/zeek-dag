
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
    # Allows building rpm/deb packages via "make package" in build dir.
    include(ConfigurePackaging)
    ConfigurePackaging(${VERSION})
endif ()
