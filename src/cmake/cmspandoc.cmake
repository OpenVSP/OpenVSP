################################################################################
## Configure Pandoc to use
## Usage:
## 1º Include Module:
##   LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/cmake-pandocology")
##   INCLUDE(pandocology)
##   INCLUDE(cmspandoc)
## 2º Call macro CONFIGUREPANDOCOLOGY() to configure
################################################################################

MACRO(CONFIGUREPANDOCOLOGY)
  MESSAGE(STATUS "Configuring pandoc variables...")

  # Set predefined options
  SET(PANDOC_OUTPUT_FORMAT pdf CACHE STRING "Choose final document")
  SET(PANDOC_STANDALONE YES CACHE BOOL "Produce output with an appropriate header and footer (e.g. a standalone HTML, LaTeX, TEI,
      or RTF file, not a fragment). This option is set automatically for pdf, epub, epub3, fb2, docx, and odt output.")
  SET(PANDOC_DOCUMENTCLASS "report" CACHE STRING "Document class with Latex")
  SET(PANDOC_LANGUAGE en CACHE STRING "Language with BCP 47 identifier")
  SET(PANDOC_LATEX_ENGINE xelatex CACHE STRING "Latex engine")
  SET(PANDOC_FONT_SIZE 12pt CACHE STRING "Font size")
  SET(PANDOC_PAPER_SIZE a4 CACHE STRING "Paper size")
  SET(PANDOC_MARGIN_LEFT 3cm CACHE STRING "Margin left")
  SET(PANDOC_MARGIN_RIGHT 3cm CACHE STRING "Margin right")
  SET(PANDOC_MARGIN_TOP 3cm CACHE STRING "Margin top")
  SET(PANDOC_MARGIN_BOTTOM 3cm CACHE STRING "Margin bottom")
  SET(PANDOC_FONT_MAIN "Liberation Sans" CACHE STRING "Font")
  SET(PANDOC_FONT_MONO "Liberation Sans" CACHE STRING "Font")
  SET(PANDOC_INCLUDE_CONTENT TRUE CACHE BOOL "Choose if you want include Tables of content (toc)")
  SET(PANDOC_INCLUDE_FIGURE FALSE CACHE BOOL "Choose if you want include Tables of content (lof)")
  SET(PANDOC_INCLUDE_TABLE FALSE CACHE BOOL "Choose if you want include Tables of content (lot)")
  #SET(PANDOC_SECTION_NUMBERS en CACHE STRING "Add section numbers")

  # Set multiple options in cache variables
  SET_PROPERTY(CACHE PANDOC_DOCUMENTCLASS PROPERTY STRINGS article report book letter slides)
  SET_PROPERTY(CACHE PANDOC_OUTPUT_FORMAT PROPERTY STRINGS pdf doc odt html)
  SET_PROPERTY(CACHE PANDOC_LANGUAGE PROPERTY STRINGS en es)
  SET_PROPERTY(CACHE PANDOC_LATEX_ENGINE PROPERTY STRINGS pdflatex xelatex lualatex)
  SET_PROPERTY(CACHE PANDOC_FONT_SIZE PROPERTY STRINGS 10pt 12pt 14pt 16pt 18pt 20pt 30pt 35pt 40pt)
  SET_PROPERTY(CACHE PANDOC_PAPER_SIZE PROPERTY STRINGS a4 a3)
  LIST(APPEND PANDOC_FONTS "Liberation Sans" "Inconsolata" "FreeMono" "Palatino")
  SET_PROPERTY(CACHE PANDOC_FONT_MAIN PROPERTY STRINGS ${PANDOC_FONTS})
  SET_PROPERTY(CACHE PANDOC_FONT_MONO PROPERTY STRINGS ${PANDOC_FONTS})
  #SET_PROPERTY(CACHE PANDOC_SECTION_NUMBERS PROPERTY STRINGS yes no)

  # CMake Options
  SET(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Release" FORCE)
  SET(CMAKE_INSTALL_PREFIX
      ${PROJECT_BINARY_DIR}/install
      CACHE STRING "Build path" FORCE)

  # FORMAT Options

  # Pdf Options
  SET(IMG_EXT_PDF pdf)
  #LIST(APPEND PARAMS_PDF "-V babel-lang=$(LANG)")

  # Others
  # odt TODO
  # doc TODO
  # html TODO
  # presentation TODO

  # Condition
  IF(PANDOC_STANDALONE)
    SET(PANDOC_STANDALONE_VAR -s)
  ELSE()
    SET(PANDOC_STANDALONE_VAR )
  ENDIF()

  # Configure pandoc for all projects, you can use your own variable into different modules
  SET(PANDOC_PDF_CONFIG
          ${PANDOC_STANDALONE_VAR}
          --variable documentclass=${PANDOC_DOCUMENTCLASS}
          --latex-engine=${PANDOC_LATEX_ENGINE}
          --default-image-extension=${IMG_EXT_PDF}
          --variable lang=${PANDOC_LANGUAGE}
          --variable fontsize=${PANDOC_FONT_SIZE}
          --variable papersize=${PANDOC_PAPER_SIZE}
          #--variable fontfamily=${PANDOC_FONT_FAMILY}  # only pdflatex
          --variable mainfont=${PANDOC_FONT_MAIN}
          --variable monofont=${PANDOC_FONT_MONO}
          --variable margin-left=${PANDOC_MARGIN_LEFT}
          --variable margin-right=${PANDOC_MARGIN_RIGHT}
          --variable margin-top=${PANDOC_MARGIN_TOP}
          --variable margin-bottom=${PANDOC_MARGIN_BOTTOM}
        )

  # Add extra includes
  IF(PANDOC_INCLUDE_CONTENT)
    SET(PANDOC_PDF_CONFIG ${PANDOC_PDF_CONFIG} --variable toc)
  ENDIF()

  IF(PANDOC_INCLUDE_FIGURE)
    SET(PANDOC_PDF_CONFIG ${PANDOC_PDF_CONFIG} --variable lof)
  ENDIF()

  IF(PANDOC_INCLUDE_TABLE)
    SET(PANDOC_PDF_CONFIG ${PANDOC_PDF_CONFIG} --variable lot)
  ENDIF()

  MESSAGE(STATUS "pandoc variables configured!")
ENDMACRO()
