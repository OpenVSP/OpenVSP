CMAKE_MINIMUM_REQUIRED ( VERSION 3.24 )

if ( is_multi_config )
    SET( conf_suffix "$<CONFIG>/" )
endif()

INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images/ DESTINATION help/images )

ADD_CUSTOM_TARGET ( copy_images ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/images/
        ${CMAKE_BINARY_DIR}/src/vsp/help/images/

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/images/
        ${CMAKE_CURRENT_BINARY_DIR}/images/ )

INSTALL( FILES ${CMAKE_CURRENT_SOURCE_DIR}/github-pandoc.css DESTINATION help )

ADD_CUSTOM_TARGET ( copy_css ALL
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/github-pandoc.css
        ${CMAKE_BINARY_DIR}/src/vsp/help/${conf_suffix}$ )

INCLUDE ( pandocology )

FILE ( GLOB_RECURSE HELP_FILES *.md )

FOREACH ( hpath ${HELP_FILES} )
    CMAKE_PATH ( GET hpath STEM hbase )

    # Pandoc 3.1.9 required for tex_math_gfm
    IF ( EXISTS ${PANDOC_EXECUTABLE} AND ${PANDOC_VERSION} VERSION_GREATER_EQUAL 3.1.9 )
        ADD_DOCUMENT ( TARGET ${hbase}
                       OUTPUT_FILE ${hbase}.html
                       SOURCES ${hbase}.md
                       RESOURCE_FILES links-to-html.lua github-pandoc.css
                       PANDOC_DIRECTIVES --lua-filter=links-to-html.lua
                                         --standalone
                                         -f gfm+tex_math_gfm+yaml_metadata_block
                                         --css=github-pandoc.css
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
        # Don't fail build if some HTML help files are missing.
        IF ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/html/${hbase}.html )
            INSTALL ( FILES ${CMAKE_CURRENT_SOURCE_DIR}/html/${hbase}.html DESTINATION help )

            ADD_CUSTOM_TARGET ( copy_html_${hbase} ALL
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_CURRENT_SOURCE_DIR}/html/${hbase}.html
                    ${CMAKE_BINARY_DIR}/src/vsp/help/${conf_suffix}${hbase}.html )
        ENDIF()
    ENDIF()

ENDFOREACH ()


