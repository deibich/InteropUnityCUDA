add_library(Utilities SHARED
    "src/log.cpp"
)

target_compile_definitions(Utilities PRIVATE UTILITIES_SHARED)
target_compile_definitions(Utilities PRIVATE UTILITIES_SHARED_EXPORTS)


target_include_directories(Utilities
    PUBLIC
        "include"
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

target_link_libraries(Utilities
    PUBLIC
        CUDA::cudart
)

install(TARGETS Utilities DESTINATION ${UNITY_PROJECT_ROOT_DIR}/Assets/Plugin)
