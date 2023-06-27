add_library(gl3w INTERFACE)

target_sources(gl3w
    INTERFACE
        "src/gl3w.c"
)

target_include_directories(gl3w
    INTERFACE
        "include"
)
