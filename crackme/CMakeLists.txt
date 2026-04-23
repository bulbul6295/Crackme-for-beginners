cmake_minimum_required(VERSION 3.15)
project(UltimateCrackMe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable security features for crackme challenge
if(MSVC)
    add_compile_options(/MT /O2 /W4 /GS- /D_CRT_SECURE_NO_WARNINGS)
    add_link_options(/SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /HIGHENTROPYVA:NO)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/protection.cpp
    src/crypto.cpp
)

set(HEADERS
    src/protection.h
    src/crypto.h
    src/obfuscation.h
)

# Create executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    user32
    gdi32
    comctl32
    advapi32
    iphlpapi
)

# Output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
