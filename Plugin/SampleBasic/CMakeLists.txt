add_library(SampleBasic SHARED
    "src/action_sample_texture_array.cpp"
    "src/action_sample_texture.cpp"
    "src/action_sample_vertex_buffer.cpp"
    "src/sample_kernels.cu"
)

target_link_libraries(SampleBasic
    PluginInteropUnityCUDA
)

target_include_directories(SampleBasic
    PUBLIC
        "include"
)

install(TARGETS SampleBasic DESTINATION ${UNITY_PROJECT_ROOT_DIR}/Assets/Plugin)
