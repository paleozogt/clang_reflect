set(CLANG_REFLECT_USE_FILE ${CMAKE_CURRENT_LIST_FILE})
get_filename_component(CLANG_REFLECT_USE_FILE ${CLANG_REFLECT_USE_FILE} ABSOLUTE)
set_property(GLOBAL PROPERTY CLANG_REFLECT_USE_FILE ${CLANG_REFLECT_USE_FILE})

# clang_reflect(<filename> [<targetname>])
#
# Run clang_reflect on a header file.
#
# Example:
#     include(/path/to/stuff/share/clang_reflect/UseClangReflect.cmake)
#
#     clang_reflect(src/foo/MyHeader.hpp)
#     target_include_directories(MyHeaderReflect INTERFACE src/bar)
#
#
macro(clang_reflect file)
    if (NOT CLANG_REFLECT_USE_FILE)
        get_property(CLANG_REFLECT_USE_FILE GLOBAL PROPERTY CLANG_REFLECT_USE_FILE)
    endif()

    if (NOT CLANG_REFLECT_HOME)
        get_filename_component(CLANG_REFLECT_HOME ${CLANG_REFLECT_USE_FILE}/../../.. ABSOLUTE)
    endif()

    if (NOT CLANG_REFLECT)
        set(CLANG_REFLECT ${CLANG_REFLECT_HOME}/bin/clang_reflect${CMAKE_EXECUTABLE_SUFFIX})
    endif()

    if (NOT CLANG_REFLECT_INCLUDES)
        set(CLANG_REFLECT_INCLUDES ${CLANG_REFLECT_HOME}/include/clang_reflect)
    endif()

    # get absolute and relative paths to the file
    #
    get_filename_component(abspath ${file} ABSOLUTE)
    file(RELATIVE_PATH relpath ${CMAKE_CURRENT_BINARY_DIR} ${abspath})

    # output name and target names
    #
    get_filename_component(reflect_name ${file} NAME_WE)
    set(output_name ${CMAKE_CURRENT_BINARY_DIR}/${reflect_name}Reflect.hpp)
    set(reflect_generate_target  gen${reflect_name}Reflect)
    set(reflect_interface_target ${ARGV1})
    if (NOT reflect_interface_target)
        set(reflect_interface_target ${reflect_name}Reflect)
    endif()

    # make an interface target that clients can use for target_link_libraries()
    # as well as hang target_include_directories() on
    #
    add_library(${reflect_interface_target} INTERFACE)
    target_include_directories(${reflect_interface_target} INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                $<BUILD_INTERFACE:${CLANG_REFLECT_INCLUDES}>
            )

    # get include dirs from the interface target.
    # note that we do this with generator expressions rather
    # than directly reading the property, which allows
    # clients to add include directories *after* this macro returns
    #
    set(target_includes $<TARGET_PROPERTY:${reflect_interface_target},INTERFACE_INCLUDE_DIRECTORIES>)
    set(includes_join "$<$<BOOL:${target_includes}>:-I$<JOIN:${target_includes}, -I>>")

    # generate a cmake script with the clang_reflect invocation.
    # using complex generator expressions ("output expressions") aren't
    # supported with add_custom_command(), so we have to do it
    # indirectly via a generated script.
    # 
    set(script_path ${CMAKE_CURRENT_BINARY_DIR}/gen${reflect_name}Reflect.cmake)
    set(script "execute_process(COMMAND ${CLANG_REFLECT} ${CLANG_REFLECT_FLAGS} ${includes_join} ${relpath} RESULT_VARIABLE RESULT)\n")
    set(script "${script}if (NOT RESULT EQUAL 0)\n")
    set(script "${script}    message(FATAL_ERROR \"clang_reflect failed\")\n")
    set(script "${script}endif()\n")
    file(GENERATE OUTPUT ${script_path} CONTENT "${script}")

    # run the generated cmake script
    #
    add_custom_command(OUTPUT ${output_name}
                       DEPENDS ${abspath} ${CLANG_REFLECT}
                       COMMAND ${CMAKE_COMMAND} -P ${script_path}
                       VERBATIM
                      )
    add_custom_target(${reflect_generate_target} DEPENDS ${output_name})
endmacro()
