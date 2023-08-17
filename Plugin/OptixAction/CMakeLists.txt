
OPTION(OWL_BUILD_SAMPLES ON)
set(owl_dir ${CMAKE_CURRENT_SOURCE_DIR}/submodules/owl)
add_subdirectory(${owl_dir} EXCLUDE_FROM_ALL)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${owl_dir}/owl/cmake/")

find_package(tinyxml2 CONFIG REQUIRED)
find_package(unofficial-cfitsio CONFIG REQUIRED)


add_library(OptixAction SHARED
    "src/action_test_optix_owl.cpp"
    "src/volume_renderer_test.cpp"
)

target_link_libraries(OptixAction
    PUBLIC
        PluginInteropUnityCUDA
        Utilities
    PRIVATE
		owl::owl
		int011-simpleTriangles-ptx
        tinyxml2::tinyxml2
        cfitsio
)

target_include_directories(OptixAction
    PUBLIC
        "include"
    PRIVATE
        ${OWL_INCLUDES}
)

embed_ptx(
  OUTPUT_TARGET
    int011-simpleTriangles-ptx
  PTX_LINK_LIBRARIES
    owl::owl
  SOURCES
    src/deviceCode.cu
)
install(TARGETS OptixAction DESTINATION ${UNITY_PROJECT_ROOT_DIR}/Assets/Plugin)

# add_library(OptixSourceVolumeActions SHARED
#     "src/action_source_volume_test.cpp"
#     "src/source_volume_renderer_test.cpp"
# )
# target_link_libraries(OptixSourceVolumeActions
#     PUBLIC
#         PluginInteropUnityCUDA
#         Utilities
#         d3d11
#     PRIVATE
# 		owl::owl
# 		int011-simpleTriangles-ptx
#         tinyxml2::tinyxml2
#         cfitsio
# )
# 
# 
# target_include_directories(OptixSourceVolumeActions
#     PUBLIC
#         "include"
#     PRIVATE
#         ${OWL_INCLUDES}
# )
# 
# 
# 
# install(TARGETS OptixSourceVolumeActions DESTINATION ${UNITY_PROJECT_ROOT_DIR}/Assets/Plugin)

