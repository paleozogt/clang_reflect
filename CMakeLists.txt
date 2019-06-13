cmake_minimum_required(VERSION 3.10.0)

project(clang_reflect)
set(CMAKE_CXX_STANDARD 11)
execute_process(COMMAND git describe OUTPUT_VARIABLE VERSION)
string(REGEX REPLACE "\n$" "" VERSION "${VERSION}")
message(STATUS "VERSION ${VERSION}")

set(LLVM_ENABLE_PROJECTS  clang  CACHE STRING "LLVM subprojects")
set(LIBCLANG_BUILD_STATIC ON     CACHE BOOL   "static libclang")
set(ENABLE_STATIC         STATIC CACHE BOOL   "static libs")
add_subdirectory(libs/llvm/llvm)

add_executable(clang_reflect src/ClangReflect.cpp)
target_link_libraries(clang_reflect PUBLIC libclang_static)
target_compile_options(clang_reflect PUBLIC -DVERSION=${VERSION})
target_include_directories(clang_reflect PUBLIC 
                            libs/llvm/clang/include
)
install(TARGETS clang_reflect
        COMPONENT ${PROJECT_NAME}
        RUNTIME DESTINATION bin
)
install(FILES src/reflect.hpp
        COMPONENT ${PROJECT_NAME}
        DESTINATION include/${PROJECT_NAME}
)
install(FILES cmake/UseClangReflect.cmake
        COMPONENT ${PROJECT_NAME}
        DESTINATION share/${PROJECT_NAME}
)
add_custom_target(install_${PROJECT_NAME} 
                  COMMAND ${CMAKE_COMMAND} --build . --target ${PROJECT_NAME}
                  COMMAND ${CMAKE_COMMAND} -DCOMPONENT=${PROJECT_NAME} -P cmake_install.cmake
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory(test)
