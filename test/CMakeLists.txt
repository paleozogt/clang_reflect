include(../cmake/UseClangReflect.cmake)

# gtest
add_library(${PROJECT_NAME}_gtest STATIC
                ${CMAKE_SOURCE_DIR}/libs/gtest/googletest/src/gtest-all.cc
                ${CMAKE_SOURCE_DIR}/libs/gtest/googletest/src/gtest_main.cc)
target_include_directories(${PROJECT_NAME}_gtest PUBLIC
                               ${CMAKE_SOURCE_DIR}/libs/gtest/googletest/include
                               ${CMAKE_SOURCE_DIR}/libs/gtest/googletest
)

# generate reflection
clang_reflect(foobase.hpp ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test)
clang_reflect(foobar.hpp  ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test)

# test runner
add_executable(${PROJECT_NAME}_test_runner ${PROJECT_NAME}_tests.cpp foobarReflect.hpp foobaseReflect.hpp)
set_target_properties(${PROJECT_NAME}_test_runner PROPERTIES OUTPUT_NAME test_runner)
target_link_libraries(${PROJECT_NAME}_test_runner PUBLIC ${PROJECT_NAME}_gtest)
target_include_directories(${PROJECT_NAME}_test_runner PUBLIC 
                                ${CMAKE_SOURCE_DIR}/src 
                                ${CMAKE_CURRENT_BINARY_DIR}
)
add_test(NAME ${PROJECT_NAME}_unittests 
         COMMAND $<TARGET_NAME:${PROJECT_NAME}_test_runner>
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# test target
add_custom_target(check_${PROJECT_NAME} COMMAND ${CMAKE_CTEST_COMMAND} -V
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_dependencies(check_${PROJECT_NAME} ${PROJECT_NAME}_test_runner)
