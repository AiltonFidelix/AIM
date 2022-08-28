# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
FILE(GLOB_RECURSE web_app ${CMAKE_SOURCE_DIR}/webapp/*)

idf_component_register(SRCS ${app_sources} EMBED_FILES ${web_app})