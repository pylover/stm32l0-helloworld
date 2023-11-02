
# Register core library to the system
add_library(lwjson INTERFACE)
target_sources(lwjson PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/lwjson.c
    # ${CMAKE_CURRENT_LIST_DIR}/lwjson_stream.c
)

# # Register lwjson debug module
# add_library(lwjson_debug INTERFACE)
# target_sources(lwjson PUBLIC 
#     ${CMAKE_CURRENT_LIST_DIR}/lwjson_debug.c
# )
