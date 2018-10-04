cmake_minimum_required(VERSION 3.5)
project(image_comparer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-fPIC")

set_source_files_properties( tags PROPERTIES GENERATED true)
add_custom_command ( OUTPUT tags
        COMMAND ctags -R .. --exclude=build --exclude=cscope.*
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR} )

set_source_files_properties( scope PROPERTIES GENERATED true)
add_custom_command ( OUTPUT scope
        COMMAND cscope -b -q -U -R ../src/* ../hdr/*
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR} )

#add_custom_target(copy
#        COMMAND
#        adfsadf
#        DEPENDS image_comparer
#        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})


file(GLOB SOURCES "src/*.c")

include_directories(hdr)

find_library(JPEG_LIB jpeg)
add_executable(image_comparer ${SOURCES} tags scope ${JPEG_LIB})
target_link_libraries(image_comparer jpeg)
