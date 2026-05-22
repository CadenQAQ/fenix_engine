cmake_minimum_required(VERSION 3.10)

# 给整个大工程换个名字
project(FenixGatewayEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

# ✅ 武器库改名为 fenix_core
add_library(fenix_core STATIC ${SOURCES})

add_executable(gateway_app main.cpp)

# ✅ 链接器也要找对新的名字
target_link_libraries(gateway_app fenix_core)