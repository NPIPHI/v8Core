cmake_minimum_required(VERSION 3.4.1)

project(v8Core)

add_library(v8Core STATIC
        include/v8Core/v8Runtime.h
        src/v8Runtime.cpp
        src/log.h
        )


option(USE_V8_INSPECTOR "include the v8 inspector in the v8core library" OFF)

if(${USE_V8_INSPECTOR} OR TRUE)

    add_dependencies(v8Core v8Inspector)

    target_link_libraries(v8Core PUBLIC v8Inspector)

    add_definitions(-DUSE_V8_INSPECTOR=true)

endif()


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