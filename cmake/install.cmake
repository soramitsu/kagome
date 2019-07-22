function(install_deps_headers)
    install(DIRECTORY ${MICROSOFT.GSL_ROOT}/include/gsl
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(DIRECTORY deps/outcome/outcome
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

endfunction()

function(enable_installable)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs TARGETS HEADER_DIRS)
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN})

    install(TARGETS ${arg_TARGETS} EXPORT kagomeTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_PREFIX}
        )

    foreach (DIR IN ITEMS ${arg_HEADER_DIRS})
        get_filename_component(FULL_PATH ${DIR} ABSOLUTE)
        string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/core kagome RELATIVE_PATH ${FULL_PATH})
        get_filename_component(INSTALL_PREFIX ${RELATIVE_PATH} DIRECTORY)
        install(DIRECTORY ${DIR}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${INSTALL_PREFIX}
            FILES_MATCHING PATTERN "*.hpp")
    endforeach ()

    install(
        EXPORT kagomeTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kagome
        NAMESPACE kagome::
    )
    export(
        EXPORT kagomeTargets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/kagomeTargets.cmake
        NAMESPACE kagome::
    )

endfunction()
