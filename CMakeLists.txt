cmake_minimum_required(VERSION 3.16)
project(AnimatedBusInfoBoard)

set(CMAKE_CXX_STANDARD 20)

# Find Qt
find_package(Qt6 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(AnimatedBusInfoBoard main.cpp)

# Link to Qt
target_link_libraries(AnimatedBusInfoBoard PRIVATE Qt6::Widgets)

# --- Custom command to copy data files ---
# This ensures that the data/ directory is copied to the build directory
# so the executable can find config.ini, stops.csv, and the ads/ folder.
add_custom_target(copy_data ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}
    COMMENT "Copying data files to build directory"
)

add_dependencies(AnimatedBusInfoBoard copy_data)
