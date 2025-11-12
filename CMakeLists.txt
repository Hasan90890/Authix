cmake_minimum_required(VERSION 3.15)
project(Authix CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# Include FetchContent for downloading dependencies
include(FetchContent)

# DPP (Discord C++ library)
FetchContent_Declare(
    dpp
    GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
    GIT_TAG v10.0.30
)

# Crow (Web framework)
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.0+5
)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3
)

FetchContent_MakeAvailable(dpp crow json httplib)

# Source files
set(SOURCES
    src/main.cpp
    src/auth_manager.cpp
    src/http_client.cpp
    src/discord_bot.cpp
    src/web_server.cpp
)

# Create executable
add_executable(authix ${SOURCES})

# Include directories
target_include_directories(authix PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(authix PRIVATE
    dpp
    Crow::Crow
    nlohmann_json::nlohmann_json
    httplib::httplib
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
)

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(authix PRIVATE -Wall -Wextra -Wpedantic)
endif()
