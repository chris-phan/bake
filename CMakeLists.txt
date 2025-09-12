cmake_minimum_required(VERSION 3.15)
project(bake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(antlr4-runtime REQUIRED)
find_package(LLVM REQUIRED)
find_package(GTest REQUIRED)

llvm_map_components_to_libnames(LLVM_LIBS orcjit executionengine core support native)


file(GLOB_RECURSE SRC_FILES "src/*.cpp")
list(FILTER SRC_FILES EXCLUDE REGEX ".*/main\\.cpp$")
add_library(bakeSrc ${SRC_FILES})
target_include_directories(bakeSrc PUBLIC src ${LLVM_INCLUDE_DIRS})
target_compile_definitions(bakeSrc PUBLIC ${LLVM_DEFINITIONS})
target_link_libraries(bakeSrc PUBLIC antlr4_static ${LLVM_LIBS})


file(GLOB ANTLR_SOURCES "src/parser/Generated/*.cpp")
add_executable(${PROJECT_NAME} src/main.cpp ${ANTLR_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE bakeSrc)


file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE bakeSrc GTest::gtest GTest::gtest_main)

enable_testing()
add_test(NAME BakeTests COMMAND tests)

