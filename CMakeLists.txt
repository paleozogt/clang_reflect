cmake_minimum_required(VERSION 3.10.2)

project(clang_reflect)
set(CMAKE_CXX_STANDARD 11)

set(LLVM_ENABLE_PROJECTS  clang CACHE STRING "LLVM subprojects")
set(LIBCLANG_BUILD_STATIC ON    CACHE BOOL   "static libclang")
add_subdirectory(libs/llvm/llvm)

add_executable(clang_reflect src/clang_reflect.cpp)
target_link_libraries(clang_reflect PUBLIC libclang_static)
target_include_directories(clang_reflect PUBLIC 
                            libs/llvm/clang/include
)

add_subdirectory(test)
