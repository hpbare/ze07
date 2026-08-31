if(ESP_PLATFORM)
    idf_component_register(
        SRCS            "ze07.c"
        INCLUDE_DIRS    "."
    )
else()
    add_library(ze07 STATIC ze07.c)
    target_include_directories(ze07 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()
