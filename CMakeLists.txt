cmake_minimum_required(VERSION 3.16)
project(Codes VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Recommend out-of-source builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(WARNING "In-source build detected. It's recommended to build in a separate 'build' directory.")
endif()

# Enable automoc/autorcc globally for subdirectories
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add the src subdirectory that contains the target
add_subdirectory(src)
