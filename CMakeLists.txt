cmake_minimum_required(VERSION 3.17)
project(learnc__)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fexec-charset=GBK ")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -static -lws2_32")
set(CMAKE_CXX_STANDARD 20)
#link_libraries(ws2_32 wsock32)#添加静态链接
include_directories(${PROJECT_SOURCE_DIR}/src)#查找目录下头文件
#add_library(<name> [STATIC | SHARED | MODULE] [EXCLUDE_FROM_ALL] [source1] [source2 ...])
#add_subdirectory(src)#添加子文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src src)#所有源文件
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${src} ws2_32)#加动态链接