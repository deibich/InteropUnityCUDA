cmake_minimum_required(VERSION 3.25)

project(InteropUnityCUDA
  VERSION 1.0
  LANGUAGES CXX CUDA
)

set(UNITY_PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/../InteropUnityCUDA" CACHE PATH "Root Directory of the project you are working on")

find_package(CUDAToolkit)

add_subdirectory(thirdParty)
add_subdirectory(Utilities)
add_subdirectory(PluginInteropUnityCUDA)
add_subdirectory(SampleBasic)

