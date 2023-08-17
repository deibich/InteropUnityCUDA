find_package(gl3w CONFIG REQUIRED)

add_library(PluginInteropUnityCUDA SHARED
    "src/unity_plugin.cpp"
    "src/factory.cpp"
    "src/dllmain.cpp"
    "src/Texture/texture.cpp"
    "src/Texture/texture_OpenGLCoreES.cpp"
    "src/Texture/texture_D3D11.cpp"
    "src/RenderAPI/renderAPI.cpp"
    "src/RenderAPI/renderAPI_D3D11.cpp"
    "src/RenderAPI/renderAPI_D3D12.cpp"
    "src/RenderAPI/renderAPI_OpenGLCoreES.cpp"
    "src/Buffer/vertex_buffer.cpp"
    "src/Buffer/vertex_buffer_D3D11.cpp"
    "src/Buffer/vertex_buffer_OpenGLCoreES.cpp"
)

set_target_properties(PluginInteropUnityCUDA
                       PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

target_include_directories(PluginInteropUnityCUDA
    PUBLIC
        "include"
        "include/Buffer"
        "include/RenderAPI"
        "include/Texture"
)

target_link_libraries(PluginInteropUnityCUDA
    PUBLIC
        d3d11.lib
        dxgi.lib
        CUDA::cudart
        Utilities
        unity_api
        unofficial::gl3w::gl3w
)

install(TARGETS PluginInteropUnityCUDA DESTINATION ${UNITY_PROJECT_ROOT_DIR}/Assets/Plugin)
