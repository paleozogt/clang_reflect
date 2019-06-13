
macro(clang_reflect file)
    if (NOT CLANG_REFLECT)
        set(CLANG_REFLECT clang_reflect)
    endif()

    get_filename_component(abspath ${file} ABSOLUTE)
    file(RELATIVE_PATH relpath ${CMAKE_CURRENT_BINARY_DIR} ${abspath})

    get_filename_component(name ${file} NAME_WE)
    set(output_name ${CMAKE_CURRENT_BINARY_DIR}/${name}Reflect.hpp)

    set(reflect_includes)
    foreach (include ${ARGN})
        list(APPEND reflect_includes -I${include})
    endforeach()

    add_custom_command(OUTPUT ${output_name}
                       DEPENDS ${abspath}
                       COMMAND ${CLANG_REFLECT} ${reflect_includes} ${relpath}
                      )
    add_custom_target(gen_${name} DEPENDS ${output_name})
endmacro()
