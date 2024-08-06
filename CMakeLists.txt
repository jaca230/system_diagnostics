cmake_minimum_required(VERSION 3.0)

project(system_diagnostics)

# Set the C++ standard to C++11 (or newer)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source directory (adjust if needed)
set(SOURCE_DIR src)

# Add library target
file(GLOB LIBRARY_SOURCE_FILES ${SOURCE_DIR}/*.cpp)
add_library(system_diagnostics_lib ${LIBRARY_SOURCE_FILES})

# Include directories
target_include_directories(system_diagnostics_lib
   PUBLIC
   ${PROJECT_SOURCE_DIR}/include  # Replace with your actual include directory path
)

# Set the output name of the library
set_target_properties(system_diagnostics_lib PROPERTIES OUTPUT_NAME system_diagnostics)

# Specify custom output directory for the library
set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set_target_properties(system_diagnostics_lib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY}
)

# Create the system_diagnostics executable
file(GLOB EXECUTABLE_SOURCE_FILES ${SOURCE_DIR}/*.cpp)
add_executable(system_diagnostics ${EXECUTABLE_SOURCE_FILES})

# Link the executable with the library and pthread
target_link_libraries(system_diagnostics PRIVATE system_diagnostics_lib pthread)

# Set the installation directory to the parent directory
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")

# Install targets
install(TARGETS system_diagnostics
    DESTINATION bin
)
install(TARGETS system_diagnostics_lib
    DESTINATION lib
)
