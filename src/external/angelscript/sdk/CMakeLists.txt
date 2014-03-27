cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

SET(ANGELSCRIPT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/angelscript/include CACHE PATH "Path to angelscript headers")
SET(ANGELSCRIPT_ADD_ON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/add_on/ CACHE PATH "Path to angelscript add_on headers")

project(Angelscript)

set(ANGELSCRIPT_SOURCE
        ./angelscript/source/as_atomic.cpp
        ./angelscript/source/as_builder.cpp
        ./angelscript/source/as_bytecode.cpp
        ./angelscript/source/as_callfunc.cpp
        ./angelscript/source/as_callfunc_x86.cpp
        ./angelscript/source/as_callfunc_x64_gcc.cpp
        ./angelscript/source/as_callfunc_x64_msvc.cpp
        ./angelscript/source/as_callfunc_x64_mingw.cpp
        ./angelscript/source/as_compiler.cpp
        ./angelscript/source/as_configgroup.cpp
        ./angelscript/source/as_context.cpp
        ./angelscript/source/as_datatype.cpp
        ./angelscript/source/as_gc.cpp
        ./angelscript/source/as_generic.cpp
        ./angelscript/source/as_globalproperty.cpp
        ./angelscript/source/as_memory.cpp
        ./angelscript/source/as_module.cpp
        ./angelscript/source/as_objecttype.cpp
        ./angelscript/source/as_outputbuffer.cpp
        ./angelscript/source/as_parser.cpp
        ./angelscript/source/as_restore.cpp
        ./angelscript/source/as_scriptcode.cpp
        ./angelscript/source/as_scriptengine.cpp
        ./angelscript/source/as_scriptfunction.cpp
        ./angelscript/source/as_scriptnode.cpp
        ./angelscript/source/as_scriptobject.cpp
        ./angelscript/source/as_string.cpp
        ./angelscript/source/as_string_util.cpp
        ./angelscript/source/as_thread.cpp
        ./angelscript/source/as_tokenizer.cpp
        ./angelscript/source/as_typeinfo.cpp
        ./angelscript/source/as_variablescope.cpp
        ./add_on/scriptstdstring/scriptstdstring.cpp
        ./add_on/scriptstdstring/scriptstdstring_utils.cpp
        ./add_on/scriptarray/scriptarray.cpp
        ./add_on/scriptany/scriptany.cpp
        ./add_on/scriptfile/scriptfile.cpp
        ./add_on/scriptmath/scriptmath.cpp
        ./add_on/scriptbuilder/scriptbuilder.cpp
 
)

if(MSVC AND CMAKE_CL_64)
            enable_language(ASM_MASM)
            if(CMAKE_ASM_MASM_COMPILER_WORKS)
                            set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ./angelscript/source/as_callfunc_x64_msvc_asm.asm)
            else()
                            message(FATAL ERROR "MSVC x86_64 target requires a working assembler")
            endif()
endif()

if(ANDROID)
        enable_language(ASM)
        if(CMAKE_ASM_COMPILER_WORKS)
                set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ./angelscript/source/as_callfunc_arm.cpp ./angelscript/source/as_callfunc_arm_gcc.S)
        else()
                message(FATAL ERROR "Android target requires a working assembler")
        endif(CMAKE_ASM_COMPILER_WORKS)
endif()

set(ANGELSCRIPT_HEADERS
        ./angelscript/include/angelscript.h
        ./angelscript/source/as_array.h
        ./angelscript/source/as_builder.h
        ./angelscript/source/as_bytecode.h
        ./angelscript/source/as_callfunc.h
        ./angelscript/source/as_compiler.h
        ./angelscript/source/as_config.h
        ./angelscript/source/as_configgroup.h
        ./angelscript/source/as_context.h
        ./angelscript/source/as_criticalsection.h
        ./angelscript/source/as_datatype.h
        ./angelscript/source/as_debug.h
        ./angelscript/source/as_generic.h
        ./angelscript/source/as_map.h
        ./angelscript/source/as_memory.h
        ./angelscript/source/as_module.h
        ./angelscript/source/as_objecttype.h
        ./angelscript/source/as_outputbuffer.h
        ./angelscript/source/as_parser.h
        ./angelscript/source/as_property.h
        ./angelscript/source/as_restore.h
        ./angelscript/source/as_scriptcode.h
        ./angelscript/source/as_scriptengine.h
        ./angelscript/source/as_scriptfunction.h
        ./angelscript/source/as_scriptnode.h
        ./angelscript/source/as_scriptobject.h
        ./angelscript/source/as_string.h
        ./angelscript/source/as_string_util.h
        ./angelscript/source/as_texts.h
        ./angelscript/source/as_thread.h
        ./angelscript/source/as_tokendef.h
        ./angelscript/source/as_tokenizer.h
        ./angelscript/source/as_typeinfo.h
        ./angelscript/source/as_variablescope.h
        ./add_on/scriptstdstring/scriptstdstring.h
        ./add_on/scriptarray/scriptarray.h
        ./add_on/scriptany/scriptany.h
        ./add_on/scriptfile/scriptfile.h
        ./add_on/scriptmath/scriptmath.h
        ./add_on/scriptbuilder/scriptbuilder.h

)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/./angelscript/include)

add_definitions("-D_CRT_SECURE_NO_WARNINGS -DANGELSCRIPT_EXPORT -D_LIB")

# Fix x64 issues on Linux
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" AND NOT APPLE)
        add_definitions(-fPIC)
endif()

add_library(Angelscript STATIC ${ANGELSCRIPT_SOURCE} ${ANGELSCRIPT_HEADERS})
set(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/./angelscript/lib)

find_package(Threads)
target_link_libraries(Angelscript ${CMAKE_THREAD_LIBS_INIT})

if(MSVC)
        set_target_properties(Angelscript PROPERTIES COMPILE_FLAGS "/MP")
endif(MSVC)

set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/./angelscript/bin)

#add_subdirectory(./angelscript/../samples/game/projects/cmake/ ./game)

