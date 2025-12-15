cmake_minimum_required(VERSION 3.16)
# Assume project() is declared in the top-level CMakeLists.txt

# Require Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Source files (keep resources separately)
set(SRC_FILES
    main.cpp
    dino.cpp
    gamewindow.cpp
)

# Compile Qt resources
qt_add_resources(RCC_SRCS ../resources/resources.qrc)

# Create executable (PROJECT_NAME expected from top-level CMake)
add_executable(${PROJECT_NAME} ${SRC_FILES} ${RCC_SRCS})

# Include headers from the top-level include/ directory
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Enable automatic Qt tools
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Recommended C++ standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
