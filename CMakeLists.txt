cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)
project(assload
  VERSION 0.1
  LANGUAGES C CXX)

##### Options
option(PROJ_WERROR "enable treating warning as errors" ON)
option(PROJ_SANITIZER "enable sanitizer for supported compilers" ON)
option(CMAKE_EXPORT_COMPILE_COMMANDS "generating compile commands when possible" ON)

##### settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")
set(OpenGL_GL_PREFERENCE GLVND) # for opengl on linux

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


set(ASSIMP_ROOT_DIR ${PROJECT_SOURCE_DIR})
find_package(Assimp REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_subdirectory(src)

if (WIN32)
  file(GLOB DLLS "dlls/*")
  file(COPY ${DLLS} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  #make msvc happy
  file(COPY ${DLLS} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
  file(COPY ${DLLS} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
endif()
