CMAKE_MINIMUM_REQUIRED ( VERSION 3.24 )

INCLUDE ( pandocology )

FILE ( GLOB_RECURSE HELP_FILES *.md )

FOREACH ( hpath ${HELP_FILES} )
    CMAKE_PATH ( GET hpath STEM hbase )

    IF ( EXISTS ${PANDOC_EXECUTABLE} )
        ADD_DOCUMENT ( TARGET ${hbase}
                       OUTPUT_FILE ${hbase}.html
                       SOURCES ${hbase}.md
        )

        INSTALL ( FILES ${CMAKE_CURRENT_BINARY_DIR}/${hbase}.html DESTINATION help )

        ADD_CUSTOM_COMMAND ( TARGET ${hbase} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${hbase}.html
                ${CMAKE_BINARY_DIR}/src/vsp/help/${conf_suffix}${hbase}.html )

        ADD_CUSTOM_COMMAND ( TARGET ${hbase} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${hbase}.html
                ${CMAKE_CURRENT_SOURCE_DIR}/html/${hbase}.html )
    ELSE()
        INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/html/${hbase}.html DESTINATION help )

        ADD_CUSTOM_TARGET ( copy_html
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/html/${hbase}.html
                ${CMAKE_BINARY_DIR}/src/vsp/help/${conf_suffix}${hbase}.html )
    ENDIF()

ENDFOREACH ()


