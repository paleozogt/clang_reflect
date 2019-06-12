
macro(clang_reflect)
    if (NOT CLANG_REFLECT)
        set(CLANG_REFLECT clang_reflect)
    endif()

    foreach(file ${ARGN})
        get_filename_component(realpath ${file} ABSOLUTE)

        get_filename_component(name ${file} NAME_WE)
        set(output_name ${CMAKE_CURRENT_BINARY_DIR}/${name}Reflect.hpp)

        add_custom_command(OUTPUT ${output_name}
                           COMMAND ${CLANG_REFLECT} ${realpath}
                          )
        add_custom_target(gen_${name} DEPENDS ${output_name})
    endforeach()
endmacro()
