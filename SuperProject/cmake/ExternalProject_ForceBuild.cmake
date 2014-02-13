MACRO( EP_ForceBuild proj )
	ExternalProject_Add_Step( ${proj} forcebuild
		COMMAND ${CMAKE_COMMAND} -E remove ${base}/Stamp/${proj}/${proj}-build
		DEPENDEES configure
		DEPENDERS build
		ALWAYS 1
	)
ENDMACRO()
