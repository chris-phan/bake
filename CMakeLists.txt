cmake_minimum_required(VERSION 3.15)
project(bake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(antlr4-runtime REQUIRED)
find_package(LLVM REQUIRED)

file(GLOB ANTLR_SOURCES "src/parser/Generated/*.cpp")

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${ANTLR_SOURCES}
    src/parser/TypeCheckVisitor.cpp
)

target_link_libraries(${PROJECT_NAME} antlr4_static llvm-core::llvm-core)
