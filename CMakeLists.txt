cmake_minimum_required(VERSION 3.4.1)

project(v8Core)

add_subdirectory(deps)

add_library(v8Core STATIC
        include/v8Core/v8Runtime.h
        src/FileManager.cpp src/FileManager.h
        src/v8Runtime.cpp
        src/log.h
        )

target_link_libraries(v8Core PUBLIC v8Inspector)

if(DEFINED V8_INCLUDE_DIR)
    target_include_directories(v8Core PUBLIC ${V8_INCLUDE_DIR})
else()
    message(FATAL_ERROR "V8_INCLUDE_DIR not set")
endif()

if(DEFINED V8_MONOLITH)
    add_library(v8Base STATIC IMPORTED)

    set_target_properties(v8Base PROPERTIES IMPORTED_LOCATION ${V8_MONOLITH})

    target_link_libraries(v8Core PUBLIC v8Base)
else()
    message(FATAL_ERROR "V8_MONOLITH not set")
endif()

target_include_directories(v8Core PUBLIC include)