cmake_minimum_required(VERSION 3.10)

# 给整个大工程换个名字
project(FenixGatewayEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

# ✅ 武器库改名为 fenix_core
add_library(fenix_core STATIC ${SOURCES})

#add_executable(gateway_app main.cpp)
# ✅ 链接器也要找对新的名字
#target_link_libraries(gateway_app pthread)

# 编译高频网关
add_executable(gateway_app main.cpp)
target_link_libraries(gateway_app pthread)

# 编译性能压测工具
add_executable(benchmark_app benchmark_ringbuffer.cpp)
target_link_libraries(benchmark_app pthread)

add_executable(benchmark_fenix benchmark_fenix.cpp)
target_link_libraries(benchmark_fenix pthread)
# 如果你配置了全局优化，最好确保有 target_compile_options(benchmark_fenix PRIVATE -O3)


# 寻找 OpenSSL 和 Boost 库
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(crypto_real_gateway crypto_real_gateway.cpp)

# 链接网络库、加密库和多线程库
target_link_libraries(crypto_real_gateway 
    pthread 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    Boost::system
)
target_compile_options(crypto_real_gateway PRIVATE -O3)
