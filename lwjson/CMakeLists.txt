cmake_minimum_required(VERSION 3.22)


# Setup generic source files
set(lwjson_core_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/lwjson.c
)

#${CMAKE_CURRENT_LIST_DIR}/lwjson_stream.c

# # Debug sources
# set(lwjson_debug_SRCS
#     ${CMAKE_CURRENT_LIST_DIR}/lwjson/lwjson_debug.c
#     )
# 

# Register core library to the system
add_library(lwjson INTERFACE)
target_sources(lwjson PUBLIC ${lwjson_core_SRCS})
target_include_directories(lwjson INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# # Register lwjson debug module
# add_library(lwjson_debug INTERFACE)
# target_sources(lwjson_debug PUBLIC ${lwjson_debug_SRCS})
# target_include_directories(lwjson_debug INTERFACE ${lwjson_include_DIRS})
