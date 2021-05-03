/*
   AngelCode Scripting Library
   Copyright (c) 2003-2021 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


//
// angelscript.h
//
// The script engine interface
//


//! \file angelscript.h
//! \brief The API definition for AngelScript.
//!
//! This header file describes the complete application programming interface for AngelScript.

#ifndef ANGELSCRIPT_H
#define ANGELSCRIPT_H

#include <stddef.h>
#ifndef _MSC_VER
#include <stdint.h>
#endif

#ifdef AS_USE_NAMESPACE
 #define BEGIN_AS_NAMESPACE namespace AngelScript {
 #define END_AS_NAMESPACE }
 #define AS_NAMESPACE_QUALIFIER AngelScript::
#else
 #define BEGIN_AS_NAMESPACE
 #define END_AS_NAMESPACE
 #define AS_NAMESPACE_QUALIFIER ::
#endif

BEGIN_AS_NAMESPACE

// AngelScript version

//! Version 2.35.0
#define ANGELSCRIPT_VERSION        23500
#define ANGELSCRIPT_VERSION_STRING "2.35.0"

// Data types

class asIScriptEngine;
class asIScriptModule;
class asIScriptContext;
class asIScriptGeneric;
class asIScriptObject;
class asITypeInfo;
class asIScriptFunction;
class asIBinaryStream;
class asIJITCompiler;
class asIThreadManager;
class asILockableSharedBool;
class asIStringFactory;

// Enumerations and constants

// Return codes
//! Return codes
enum asERetCodes
{
	//! Success
	asSUCCESS                              =  0,
	//! Failure
	asERROR                                = -1,
	//! The context is active
	asCONTEXT_ACTIVE                       = -2,
	//! The context is not finished
	asCONTEXT_NOT_FINISHED                 = -3,
	//! The context is not prepared
	asCONTEXT_NOT_PREPARED                 = -4,
	//! Invalid argument
	asINVALID_ARG                          = -5,
	//! The function was not found
	asNO_FUNCTION                          = -6,
	//! Not supported
	asNOT_SUPPORTED                        = -7,
	//! Invalid name
	asINVALID_NAME                         = -8,
	//! The name is already taken
	asNAME_TAKEN                           = -9,
	//! Invalid declaration
	asINVALID_DECLARATION                  = -10,
	//! Invalid object
	asINVALID_OBJECT                       = -11,
	//! Invalid type
	asINVALID_TYPE                         = -12,
	//! Already registered
	asALREADY_REGISTERED                   = -13,
	//! Multiple matching functions
	asMULTIPLE_FUNCTIONS                   = -14,
	//! The module was not found
	asNO_MODULE                            = -15,
	//! The global variable was not found
	asNO_GLOBAL_VAR                        = -16,
	//! Invalid configuration
	asINVALID_CONFIGURATION                = -17,
	//! Invalid interface
	asINVALID_INTERFACE                    = -18,
	//! All imported functions couldn't be bound
	asCANT_BIND_ALL_FUNCTIONS              = -19,
	//! The array sub type has not been registered yet
	asLOWER_ARRAY_DIMENSION_NOT_REGISTERED = -20,
	//! Wrong configuration group
	asWRONG_CONFIG_GROUP                   = -21,
	//! The configuration group is in use
	asCONFIG_GROUP_IS_IN_USE               = -22,
	//! Illegal behaviour for the type
	asILLEGAL_BEHAVIOUR_FOR_TYPE           = -23,
	//! The specified calling convention doesn't match the function/method pointer
	asWRONG_CALLING_CONV                   = -24,
	//! A build is currently in progress
	asBUILD_IN_PROGRESS                    = -25,
	//! The initialization of global variables failed
	asINIT_GLOBAL_VARS_FAILED              = -26,
	//! It wasn't possible to allocate the needed memory
	asOUT_OF_MEMORY                        = -27,
	//! The module is referred to by live objects or from the application
	asMODULE_IS_IN_USE                     = -28
};

// Engine properties
//! Engine properties
enum asEEngineProp
{
	//! Allow unsafe references. Default: false.
	asEP_ALLOW_UNSAFE_REFERENCES            = 1,
	//! Optimize byte code. Default: true.
	asEP_OPTIMIZE_BYTECODE                  = 2,
	//! Copy script section memory. Default: true.
	asEP_COPY_SCRIPT_SECTIONS               = 3,
	//! Maximum stack size in bytes for script contexts. Default: 0 (no limit).
	asEP_MAX_STACK_SIZE                     = 4,
	//! Interpret single quoted strings as character literals. Default: false.
	asEP_USE_CHARACTER_LITERALS             = 5,
	//! Allow linebreaks in string constants. Default: false.
	asEP_ALLOW_MULTILINE_STRINGS            = 6,
	//! Allow script to declare implicit handle types. Default: false.
	asEP_ALLOW_IMPLICIT_HANDLE_TYPES        = 7,
	//! Remove SUSPEND instructions between each statement. Default: false.
	asEP_BUILD_WITHOUT_LINE_CUES            = 8,
	//! Initialize global variables after a build. Default: true.
	asEP_INIT_GLOBAL_VARS_AFTER_BUILD       = 9,
	//! When set the enum values must be prefixed with the enum type. Default: false.
	asEP_REQUIRE_ENUM_SCOPE                 = 10,
	//! Select scanning method: 0 - ASCII, 1 - UTF8. Default: 1 (UTF8).
	asEP_SCRIPT_SCANNER                     = 11,
	//! When set extra bytecode instructions needed for JIT compiled funcions will be included. Default: false.
	asEP_INCLUDE_JIT_INSTRUCTIONS           = 12,
	//! Select string encoding for literals: 0 - UTF8/ASCII, 1 - UTF16. Default: 0 (UTF8)
	asEP_STRING_ENCODING                    = 13,
	//! Enable or disable property accessors: 0 - no accessors, 1 - app registered accessors only, property keyword optional, 2 - app and script created accessors, property keyword optional, 3 - app and script created accesors, property keyword required. Default: 3
	asEP_PROPERTY_ACCESSOR_MODE             = 14,
	//! Format default array in template form in messages and declarations. Default: false
	asEP_EXPAND_DEF_ARRAY_TO_TMPL           = 15,
	//! Enable or disable automatic garbage collection. Default: true
	asEP_AUTO_GARBAGE_COLLECT               = 16,
	//! Disallow the use of global variables in the script. Default: false
	asEP_DISALLOW_GLOBAL_VARS               = 17,
	//! When true, the compiler will always provide a default constructor for script classes. Default: false
	asEP_ALWAYS_IMPL_DEFAULT_CONSTRUCT      = 18,
	//! Set how warnings should be treated: 0 - dismiss, 1 - emit, 2 - treat as error
	asEP_COMPILER_WARNINGS                  = 19,
	//! Disallow value assignment for reference types to avoid ambiguity. Default: false
	asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE = 20,
	//! Change the script syntax for named arguments: 0 - no change, 1 - accept '=' but warn, 2 - accept '=' without warning. Default: 0
	asEP_ALTER_SYNTAX_NAMED_ARGS            = 21,
	//! When true, the / and /= operators will perform floating-point division (i.e. 1/2 = 0.5 instead of 0). Default: false
	asEP_DISABLE_INTEGER_DIVISION           = 22,
	//! When true, the initialization lists may not contain empty elements. Default: false
	asEP_DISALLOW_EMPTY_LIST_ELEMENTS       = 23,
	//! When true, private properties behave like protected properties. Default: false
	asEP_PRIVATE_PROP_AS_PROTECTED          = 24,
	//! When true, the compiler will not give an error if identifiers contain characters with byte value above 127, thus permit identifiers to contain international characters. Default: false
	asEP_ALLOW_UNICODE_IDENTIFIERS          = 25,
	//! Define how heredoc strings will be trimmed by the compiler: 0 - never trim, 1 - trim if multiple lines, 2 - always trim. Default: 1
	asEP_HEREDOC_TRIM_MODE                  = 26,
	//! Define the maximum number of nested calls the script engine will allow. Default: 100
	asEP_MAX_NESTED_CALLS                   = 27,
	//! Define how generic calling convention treats handles: 0 - ignore auto handles, 1 - treat them the same way as native calling convention. Default: 1
	asEP_GENERIC_CALL_MODE                  = 28,
	//! Initial stack size in bytes for script contexts. Default: 4096
	asEP_INIT_STACK_SIZE                    = 29,
	//! Initial call stack size for script contexts. Default: 10
	asEP_INIT_CALL_STACK_SIZE               = 30,
	//! Maximum call stack size for script contexts. Default: 0 (no limit)
	asEP_MAX_CALL_STACK_SIZE                = 31,

	asEP_LAST_PROPERTY
};

// Calling conventions
//! Calling conventions
enum asECallConvTypes
{
	//! A cdecl function.
	asCALL_CDECL             = 0,
	//! A stdcall function.
	asCALL_STDCALL           = 1,
	//! A thiscall class method registered as a global function.
	asCALL_THISCALL_ASGLOBAL = 2,
	//! A thiscall class method.
	asCALL_THISCALL          = 3,
	//! A cdecl function that takes the object pointer as the last parameter.
	asCALL_CDECL_OBJLAST     = 4,
	//! A cdecl function that takes the object pointer as the first parameter.
	asCALL_CDECL_OBJFIRST    = 5,
	//! A function using the generic calling convention.
	asCALL_GENERIC           = 6,
	//! A thiscall class method registered as a functor object.
	asCALL_THISCALL_OBJLAST  = 7,
	//! A thiscall class method registered as a functor object.
	asCALL_THISCALL_OBJFIRST = 8
};

// Object type flags
//! Object type flags
enum asEObjTypeFlags
{
	//! A reference type.
	asOBJ_REF                        = (1<<0),
	//! A value type.
	asOBJ_VALUE                      = (1<<1),
	//! A garbage collected type. Only valid for reference types.
	asOBJ_GC                         = (1<<2),
	//! A plain-old-data type. Only valid for value types.
	asOBJ_POD                        = (1<<3),
	//! This reference type doesn't allow handles to be held. Only valid for reference types.
	asOBJ_NOHANDLE                   = (1<<4),
	//! The life time of objects of this type are controlled by the scope of the variable. Only valid for reference types.
	asOBJ_SCOPED                     = (1<<5),
	//! A template type.
	asOBJ_TEMPLATE                   = (1<<6),
	//! The value type should be treated as a handle.
	asOBJ_ASHANDLE                   = (1<<7),
	//! The C++ type is a class type. Only valid for value types.
	asOBJ_APP_CLASS                  = (1<<8),
	//! The C++ class has an explicit constructor. Only valid for value types.
	asOBJ_APP_CLASS_CONSTRUCTOR      = (1<<9),
	//! The C++ class has an explicit destructor. Only valid for value types.
	asOBJ_APP_CLASS_DESTRUCTOR       = (1<<10),
	//! The C++ class has an explicit assignment operator. Only valid for value types.
	asOBJ_APP_CLASS_ASSIGNMENT       = (1<<11),
	//! The C++ class has an explicit copy constructor. Only valid for value types.
	asOBJ_APP_CLASS_COPY_CONSTRUCTOR = (1<<12),
	//! The C++ type is a class with a constructor.
	asOBJ_APP_CLASS_C                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR),
	//! The C++ type is a class with a constructor and destructor.
	asOBJ_APP_CLASS_CD               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR),
	//! The C++ type is a class with a constructor and assignment operator.
	asOBJ_APP_CLASS_CA               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	//! The C++ type is a class with a constructor and copy constructor.
	asOBJ_APP_CLASS_CK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	//! The C++ type is a class with a constructor, destructor, and assignment operator.
	asOBJ_APP_CLASS_CDA              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	//! The C++ type is a class with a constructor, destructor, and copy constructor.
	asOBJ_APP_CLASS_CDK              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	//! The C++ type is a class with a constructor, assignment operator, and copy constructor.
	asOBJ_APP_CLASS_CAK              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	//! The C++ type is a class with a constructor, destructor, assignment operator, and copy constructor.
	asOBJ_APP_CLASS_CDAK             = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	//! The C++ type is a class with a destructor.
	asOBJ_APP_CLASS_D                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR),
	//! The C++ type is a class with a destructor and assignment operator.
	asOBJ_APP_CLASS_DA               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	//! The C++ type is a class with a destructor and copy constructor.
	asOBJ_APP_CLASS_DK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	//! The C++ type is a class with a destructor, assignment operator, and copy constructor.
	asOBJ_APP_CLASS_DAK              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	//! The C++ type is a class with an assignment operator.
	asOBJ_APP_CLASS_A                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_ASSIGNMENT),
	//! The C++ type is a class with an assignment operator and copy constructor.
	asOBJ_APP_CLASS_AK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	//! The C++ type is a class with a copy constructor.
	asOBJ_APP_CLASS_K                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	//! The C++ class has additional constructors beyond the default and copy constructors
	asOBJ_APP_CLASS_MORE_CONSTRUCTORS = (1<<31),
	//! The C++ type is a primitive type. Only valid for value types.
	asOBJ_APP_PRIMITIVE              = (1<<13),
	//! The C++ type is a float or double. Only valid for value types.
	asOBJ_APP_FLOAT                  = (1<<14),
	//! The C++ type is a static array. Only valid for value types.
	asOBJ_APP_ARRAY                  = (1<<15),
	//! The C++ class can be treated as if all its members are integers.
	asOBJ_APP_CLASS_ALLINTS          = (1<<16),
	//! The C++ class can be treated as if all its members are floats or doubles.
	asOBJ_APP_CLASS_ALLFLOATS        = (1<<17),
	//! The type doesn't use reference counting. Only valid for reference types.
	asOBJ_NOCOUNT                    = (1<<18),
	//! The C++ class contains types that may require 8byte alignment. Only valid for value types.
	asOBJ_APP_CLASS_ALIGN8           = (1<<19),
	//! The object is declared for implicit handle. Only valid for reference types.
	asOBJ_IMPLICIT_HANDLE            = (1<<20),
	//! This mask shows which flags are value for RegisterObjectType
	asOBJ_MASK_VALID_FLAGS           = 0x801FFFFF,
	// Internal flags
	//! The object is a script class or an interface.
	asOBJ_SCRIPT_OBJECT              = (1<<21),
	//! Type object type is shared between modules.
	asOBJ_SHARED                     = (1<<22),
	//! The object type is marked as final and cannot be inherited.
	asOBJ_NOINHERIT                  = (1<<23),
	//! The type is a script funcdef
	asOBJ_FUNCDEF                    = (1<<24),
	//! Internal type. Do not use
	asOBJ_LIST_PATTERN               = (1<<25),
	//! The type is an enum
	asOBJ_ENUM                       = (1<<26),
	//! Internal type. Do no use
	asOBJ_TEMPLATE_SUBTYPE           = (1<<27),
	//! The type is a typedef
	asOBJ_TYPEDEF                    = (1<<28),
	//! The class is abstract, i.e. cannot be instantiated
	asOBJ_ABSTRACT                   = (1<<29),
	//! Reserved for future use.
	asOBJ_APP_ALIGN16                = (1<<30)
};

// Behaviours
//! Behaviours
enum asEBehaviours
{
	// Value object memory management
	//! \brief Constructor
	asBEHAVE_CONSTRUCT,
	//! \brief Constructor used exclusively for initialization lists
	asBEHAVE_LIST_CONSTRUCT,
	//! \brief Destructor
	asBEHAVE_DESTRUCT,

	// Reference object memory management
	//! \brief Factory
	asBEHAVE_FACTORY,
	//! \brief Factory used exclusively for initialization lists
	asBEHAVE_LIST_FACTORY,
	//! \brief AddRef
	asBEHAVE_ADDREF,
	//! \brief Release
	asBEHAVE_RELEASE,
	//! \brief Obtain weak ref flag
	asBEHAVE_GET_WEAKREF_FLAG,

	// Object operators
	//! \brief Callback for validating template instances
	asBEHAVE_TEMPLATE_CALLBACK,

	// Garbage collection behaviours
	asBEHAVE_FIRST_GC,
	//! \brief (GC) Get reference count
	 asBEHAVE_GETREFCOUNT = asBEHAVE_FIRST_GC,
	 //! \brief (GC) Set GC flag
	 asBEHAVE_SETGCFLAG,
	 //! \brief (GC) Get GC flag
	 asBEHAVE_GETGCFLAG,
	 //! \brief (GC) Enumerate held references
	 asBEHAVE_ENUMREFS,
	 //! \brief (GC) Release all references
	 asBEHAVE_RELEASEREFS,
	asBEHAVE_LAST_GC = asBEHAVE_RELEASEREFS,

	asBEHAVE_MAX
};

// Context states
//! \brief Context states.
enum asEContextState
{
	//! The context has successfully completed the execution.
	asEXECUTION_FINISHED      = 0,
	//! The execution is suspended and can be resumed.
	asEXECUTION_SUSPENDED     = 1,
	//! The execution was aborted by the application.
	asEXECUTION_ABORTED       = 2,
	//! The execution was terminated by an unhandled script exception.
	asEXECUTION_EXCEPTION     = 3,
	//! The context has been prepared for a new execution.
	asEXECUTION_PREPARED      = 4,
	//! The context is not initialized.
	asEXECUTION_UNINITIALIZED = 5,
	//! The context is currently executing a function call.
	asEXECUTION_ACTIVE        = 6,
	//! The context has encountered an error and must be reinitialized.
	asEXECUTION_ERROR         = 7
};

// Message types
//! \brief Compiler message types.
enum asEMsgType
{
	//! The message is an error.
	asMSGTYPE_ERROR       = 0,
	//! The message is a warning.
	asMSGTYPE_WARNING     = 1,
	//! The message is informational only.
	asMSGTYPE_INFORMATION = 2
};

// Garbage collector flags
//! \brief Garbage collector flags.
enum asEGCFlags
{
	//! Execute a full cycle.
	asGC_FULL_CYCLE      = 1,
	//! Execute only one step
	asGC_ONE_STEP        = 2,
	//! Destroy known garbage
	asGC_DESTROY_GARBAGE = 4,
	//! Detect garbage with circular references
	asGC_DETECT_GARBAGE  = 8
};

// Token classes
//! \brief Token classes.
enum asETokenClass
{
	//! Unknown token.
	asTC_UNKNOWN    = 0,
	//! Keyword token.
	asTC_KEYWORD    = 1,
	//! Literal value token.
	asTC_VALUE      = 2,
	//! Identifier token.
	asTC_IDENTIFIER = 3,
	//! Comment token.
	asTC_COMMENT    = 4,
	//! White space token.
	asTC_WHITESPACE = 5
};

// Type id flags
//! \brief Type id flags
enum asETypeIdFlags
{
	//! The type id for void
	asTYPEID_VOID           = 0,
	//! The type id for bool
	asTYPEID_BOOL           = 1,
	//! The type id for int8
	asTYPEID_INT8           = 2,
	//! The type id for int16
	asTYPEID_INT16          = 3,
	//! The type id for int
	asTYPEID_INT32          = 4,
	//! The type id for int64
	asTYPEID_INT64          = 5,
	//! The type id for uint8
	asTYPEID_UINT8          = 6,
	//! The type id for uint16
	asTYPEID_UINT16         = 7,
	//! The type id for uint
	asTYPEID_UINT32         = 8,
	//! The type id for uint64
	asTYPEID_UINT64         = 9,
	//! The type id for float
	asTYPEID_FLOAT          = 10,
	//! The type id for double
	asTYPEID_DOUBLE         = 11,
	//! The bit that shows if the type is a handle
	asTYPEID_OBJHANDLE      = 0x40000000,
	//! The bit that shows if the type is a handle to a const
	asTYPEID_HANDLETOCONST  = 0x20000000,
	//! If any of these bits are set, then the type is an object
	asTYPEID_MASK_OBJECT    = 0x1C000000,
	//! The bit that shows if the type is an application registered type
	asTYPEID_APPOBJECT      = 0x04000000,
	//! The bit that shows if the type is a script class
	asTYPEID_SCRIPTOBJECT   = 0x08000000,
	//! The bit that shows if the type is a template type
	asTYPEID_TEMPLATE       = 0x10000000,
	//! The mask for the type id sequence number
	asTYPEID_MASK_SEQNBR    = 0x03FFFFFF
};

// Type modifiers
//! \brief Type modifiers
enum asETypeModifiers
{
	//! No modification
	asTM_NONE     = 0,
	//! Input reference
	asTM_INREF    = 1,
	//! Output reference
	asTM_OUTREF   = 2,
	//! In/out reference
	asTM_INOUTREF = 3,
	//! Read only
	asTM_CONST    = 4
};

// GetModule flags
//! \brief Flags for GetModule.
enum asEGMFlags
{
	//! \brief Don't return any module if it is not found.
	asGM_ONLY_IF_EXISTS       = 0,
	//! \brief Create the module if it doesn't exist.
	asGM_CREATE_IF_NOT_EXISTS = 1,
	//! \brief Always create a new module, discarding the existing one.
	asGM_ALWAYS_CREATE        = 2
};

// Compile flags
//! \brief Flags for compilation
enum asECompileFlags
{
	//! \brief The compiled function should be added to the scope of the module.
	asCOMP_ADD_TO_MODULE = 1
};

// Function types
//! \brief Function types
enum asEFuncType
{
	asFUNC_DUMMY     =-1,
	//! \brief An application registered function
	asFUNC_SYSTEM    = 0,
	//! \brief A script implemented function
	asFUNC_SCRIPT    = 1,
	//! \brief An interface method
	asFUNC_INTERFACE = 2,
	//! \brief A virtual method for script classes
	asFUNC_VIRTUAL   = 3,
	//! \brief A function definition
	asFUNC_FUNCDEF   = 4,
	//! \brief An imported function
	asFUNC_IMPORTED  = 5,
	//! \brief A function delegate
	asFUNC_DELEGATE  = 6
};



//! \typedef asINT8
//! \brief 8 bit signed integer

//! \typedef asINT16
//! \brief 16 bit signed integer

//! \typedef asBYTE
//! \brief 8 bit unsigned integer

//! \typedef asWORD
//! \brief 16 bit unsigned integer

//! \typedef asDWORD
//! \brief 32 bit unsigned integer

//! \typedef asQWORD
//! \brief 64 bit unsigned integer

//! \typedef asUINT
//! \brief 32 bit unsigned integer

//! \typedef asINT64
//! \brief 64 bit integer

//! \typedef asPWORD
//! \brief Unsigned integer with the size of a pointer.

//
// asBYTE  =  8 bits
// asWORD  = 16 bits
// asDWORD = 32 bits
// asQWORD = 64 bits
// asPWORD = size of pointer
//
typedef signed char    asINT8;
typedef signed short   asINT16;
typedef unsigned char  asBYTE;
typedef unsigned short asWORD;
typedef unsigned int   asUINT;
#if (defined(_MSC_VER) && _MSC_VER <= 1200) || defined(__S3E__) || (defined(_MSC_VER) && defined(__clang__))
	// size_t is not really correct, since it only guaranteed to be large enough to hold the segment size.
	// For example, on 16bit systems the size_t may be 16bits only even if pointers are 32bit. But nobody
	// is likely to use MSVC6 to compile for 16bit systems anymore, so this should be ok.
	typedef size_t         asPWORD;
#else
	typedef uintptr_t      asPWORD;
#endif
#ifdef __LP64__
	typedef unsigned int  asDWORD;
	typedef unsigned long asQWORD;
	typedef long asINT64;
#else
	typedef unsigned long asDWORD;
  #if !defined(_MSC_VER) && (defined(__GNUC__) || defined(__MWERKS__) || defined(__SUNPRO_CC) || defined(__psp2__))
	typedef uint64_t asQWORD;
	typedef int64_t asINT64;
  #else
	typedef unsigned __int64 asQWORD;
	typedef __int64 asINT64;
  #endif
#endif

// Is the target a 64bit system?
#if defined(__LP64__) || defined(__amd64__) || defined(__x86_64__) || defined(_M_X64)
	#ifndef AS_64BIT_PTR
		#define AS_64BIT_PTR
	#endif
#endif

typedef void (*asFUNCTION_t)();
typedef void (*asGENFUNC_t)(asIScriptGeneric *);

//! The function signature for the custom memory allocation function
typedef void *(*asALLOCFUNC_t)(size_t);
//! The function signature for the custom memory deallocation function
typedef void (*asFREEFUNC_t)(void *);
//! The function signature for the engine cleanup callback function
typedef void (*asCLEANENGINEFUNC_t)(asIScriptEngine *);
//! The function signature for the module cleanup callback function
typedef void (*asCLEANMODULEFUNC_t)(asIScriptModule *);
//! The function signature for the context cleanup callback function
typedef void (*asCLEANCONTEXTFUNC_t)(asIScriptContext *);
//! The function signature for the function cleanup callback function
typedef void (*asCLEANFUNCTIONFUNC_t)(asIScriptFunction *);
//! The function signature for the type info cleanup callback function
typedef void (*asCLEANTYPEINFOFUNC_t)(asITypeInfo *);
//! The function signature for the script object cleanup callback function
typedef void (*asCLEANSCRIPTOBJECTFUNC_t)(asIScriptObject *);
//! The function signature for the request context callback
typedef asIScriptContext *(*asREQUESTCONTEXTFUNC_t)(asIScriptEngine *, void *);
//! The function signature for the return context callback
typedef void (*asRETURNCONTEXTFUNC_t)(asIScriptEngine *, asIScriptContext *, void *);
//! The function signature for the callback used when detecting a circular reference in garbage
typedef void (*asCIRCULARREFFUNC_t)(asITypeInfo *, const void *, void *);

// Check if the compiler can use C++11 features
#if !defined(_MSC_VER) || _MSC_VER >= 1700   // MSVC 2012
 #if !defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)  // gnuc 4.7 or clang
  #if !(defined(__GNUC__) && defined(__cplusplus) && __cplusplus < 201103L) // gnuc and clang require compiler flag -std=c++11
   #if !defined(__SUNPRO_CC) // Oracle Solaris Studio
    //! \brief This macro is defined if the compiler supports the C++11 feature set
    #define AS_CAN_USE_CPP11 1
   #endif
  #endif
 #endif
#endif

// This macro does basically the same thing as offsetof defined in stddef.h, but
// GNUC should not complain about the usage as I'm not using 0 as the base pointer.
//! \brief Returns the offset of an attribute in a struct
#define asOFFSET(s,m) ((int)(size_t)(&reinterpret_cast<s*>(100000)->m)-100000)

//! \brief Returns an asSFuncPtr representing the function specified by the name
#define asFUNCTION(f) asFunctionPtr(f)
//! \brief Returns an asSFuncPtr representing the function specified by the name, parameter list, and return type
#if (defined(_MSC_VER) && _MSC_VER <= 1200) || (defined(__BORLANDC__) && __BORLANDC__ < 0x590)
// MSVC 6 has a bug that prevents it from properly compiling using the correct asFUNCTIONPR with operator >
// so we need to use ordinary C style cast instead of static_cast. The drawback is that the compiler can't
// check that the cast is really valid.
// BCC v5.8 (C++Builder 2006) and earlier have a similar bug which forces us to fall back to a C-style cast.
#define asFUNCTIONPR(f,p,r) asFunctionPtr((void (*)())((r (*)p)(f)))
#else
#define asFUNCTIONPR(f,p,r) asFunctionPtr(reinterpret_cast<void (*)()>(static_cast<r (*)p>(f)))
#endif

#ifndef AS_NO_CLASS_METHODS

class asCUnknownClass;
typedef void (asCUnknownClass::*asMETHOD_t)();

//! \brief Represents a function or method pointer.
struct asSFuncPtr
{
	asSFuncPtr(asBYTE f = 0)
	{
		for( size_t n = 0; n < sizeof(ptr.dummy); n++ )
			ptr.dummy[n] = 0;
		flag = f;
	}

	void CopyMethodPtr(const void *mthdPtr, size_t size)
	{
		for( size_t n = 0; n < size; n++ )
			ptr.dummy[n] = reinterpret_cast<const char *>(mthdPtr)[n];
	}

	union
	{
		// The largest known method point is 20 bytes (MSVC 64bit),
		// but with 8byte alignment this becomes 24 bytes. So we need
		// to be able to store at least that much.
		char dummy[25];
		struct {asMETHOD_t   mthd; char dummy[25-sizeof(asMETHOD_t)];} m;
		struct {asFUNCTION_t func; char dummy[25-sizeof(asFUNCTION_t)];} f;
	} ptr;
	asBYTE flag; // 1 = generic, 2 = global func, 3 = method
};

#if defined(__BORLANDC__)
// A bug in BCC (QC #85374) makes it impossible to distinguish const/non-const method overloads
// with static_cast<>. The workaround is to use an _implicit_cast instead.

 #if  __BORLANDC__ < 0x590
 // BCC v5.8 (C++Builder 2006) and earlier have an even more annoying bug which causes
 // the "pretty" workaround below (with _implicit_cast<>) to fail. For these compilers
 // we need to use a traditional C-style cast.
  #define AS_METHOD_AMBIGUITY_CAST(t) (t)
 #else
template <typename T>
  T _implicit_cast (T val)
{ return val; }
  #define AS_METHOD_AMBIGUITY_CAST(t) AS_NAMESPACE_QUALIFIER _implicit_cast<t >
 #endif
#else
 #define AS_METHOD_AMBIGUITY_CAST(t) static_cast<t >
#endif

//! \brief Returns an asSFuncPtr representing the class method specified by class and method name.
#define asMETHOD(c,m) asSMethodPtr<sizeof(void (c::*)())>::Convert((void (c::*)())(&c::m))
//! \brief Returns an asSFuncPtr representing the class method specified by class, method name, parameter list, return type.
#define asMETHODPR(c,m,p,r) asSMethodPtr<sizeof(void (c::*)())>::Convert(AS_METHOD_AMBIGUITY_CAST(r (c::*)p)(&c::m))

#else // Class methods are disabled

struct asSFuncPtr
{
	asSFuncPtr(asBYTE f)
	{
		for( int n = 0; n < sizeof(ptr.dummy); n++ )
			ptr.dummy[n] = 0;
		flag = f;
	}

	union
	{
		char dummy[25]; // largest known class method pointer
		struct {asFUNCTION_t func; char dummy[25-sizeof(asFUNCTION_t)];} f;
	} ptr;
	asBYTE flag; // 1 = generic, 2 = global func
};

#endif

//! \brief Represents a compiler message
struct asSMessageInfo
{
	//! The script section where the message is raised
	const char *section;
	//! The row number
	int         row;
	//! The column
	int         col;
	//! The type of message
	asEMsgType  type;
	//! The message text
	const char *message;
};


// API functions

// ANGELSCRIPT_EXPORT is defined when compiling the dll or lib
// ANGELSCRIPT_DLL_LIBRARY_IMPORT is defined when dynamically linking to the
// dll through the link lib automatically generated by MSVC++
// ANGELSCRIPT_DLL_MANUAL_IMPORT is defined when manually loading the dll
// Don't define anything when linking statically to the lib

//! \def AS_API
//! \brief A define that specifies how the function should be imported

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
  #if defined(ANGELSCRIPT_EXPORT)
    #define AS_API __declspec(dllexport)
  #elif defined(ANGELSCRIPT_DLL_LIBRARY_IMPORT)
    #define AS_API __declspec(dllimport)
  #else // statically linked library
    #define AS_API
  #endif
#elif defined(__GNUC__)
  #if defined(ANGELSCRIPT_EXPORT)
    #define AS_API __attribute__((visibility ("default")))
  #else
    #define AS_API
  #endif
#else
  #define AS_API
#endif

#ifndef ANGELSCRIPT_DLL_MANUAL_IMPORT
extern "C"
{
	// Engine
	//! \ingroup api_principal_functions
	//! \brief Creates the script engine.
	//! \param[in] version The library version. Should always be \ref ANGELSCRIPT_VERSION.
	//! \return A pointer to the script engine interface, or null on error.
	//!
	//! Call this function to create a new script engine. When you're done with the
	//! script engine, i.e. after you've executed all your scripts, you should call
	//! \ref asIScriptEngine::ShutDownAndRelease "ShutDownAndRelease" on the pointer
	//! to cleanup any objects that may still be alive and free the engine object.
	//!
	//! The \a version argument is there to allow AngelScript to validate that the 
	//! application has been compiled with the correct interface. This is especially
	//! important when linking dynamically against the library. If the version is 
	//! incorrect a null pointer is returned.
	AS_API asIScriptEngine *asCreateScriptEngine(asDWORD version = ANGELSCRIPT_VERSION);
	//! \ingroup api_auxiliary_functions
	//! \brief Returns the version of the compiled library.
	//! \return A null terminated string with the library version.
	//!
	//! The returned string can be used for presenting the library version in a log file, or in the GUI.
	AS_API const char      *asGetLibraryVersion();
	//! \ingroup api_auxiliary_functions
	//! \brief Returns the options used to compile the library.
	//! \return A null terminated string with indicators that identify the options
	//!         used to compile the script library.
	//!
	//! This can be used to identify at run-time different ways to configure the engine.
	//! For example, if the returned string contain the identifier AS_MAX_PORTABILITY then
	//! functions and methods must be registered with the \ref asCALL_GENERIC calling convention.
	AS_API const char      *asGetLibraryOptions();

	// Context
	//! \ingroup api_principal_functions
	//! \brief Returns the currently active context.
	//! \return A pointer to the currently executing context, or null if no context is executing.
	//!
	//! This function is most useful for registered functions, as it will allow them to obtain
	//! a pointer to the context that is calling the function, and through that get the engine,
	//! or custom user data.
	//!
	//! If the script library is compiled with multithread support, this function will return
	//! the context that is currently active in the thread that is being executed. It will thus
	//! work even if there are multiple threads executing scripts at the same time.
	//!
	//! This function does not increase the reference count of the context.
	AS_API asIScriptContext *asGetActiveContext();

	// Thread support
	//! \ingroup api_multithread_functions
	//! \brief Sets up the internally shared resources for multithreading
	//! \param[in] externalMgr Pre-existent thread manager (optional)
	//! \return A negative value on error
	//! \retval asINVALID_ARG externalMgr informed even though local manager already exists
	//!
	//! Call this function from the main thread to set up shared resources 
	//! for multithreading if engines are to be created in multiple threads.
	//!
	//! If multiple modules (dlls) are used it may be necessary to call this
	//! with the thread manager retrieved from \ref asGetThreadManager() in the main
	//! module in order for all modules to share the same thread manager.
	//!
	//! \see \ref doc_adv_multithread
	AS_API int               asPrepareMultithread(asIThreadManager *externalMgr = 0);
	//! \ingroup api_multithread_functions
	//! \brief Frees resources prepared for multithreading
	//!
	//! If \ref asPrepareMultithread() has been called, then this function
	//! should be called after the last engine has been released to free the
	//! resources prepared for multithreading.
	AS_API void              asUnprepareMultithread();
	//! \ingroup api_multithread_functions
	//! \brief Get the thread manager used by the application
	//! \return The thread manager prepared with \ref asPrepareMultithread()
	AS_API asIThreadManager *asGetThreadManager();
	//! \ingroup api_multithread_functions
	//! \brief Acquire an exclusive lock.
	//!
	//! This function will block the calling thread until there are no 
	//! other threads that hold shared or exclusive locks.
	AS_API void              asAcquireExclusiveLock();
	//! \ingroup api_multithread_functions
	//! \brief Release an exclusive lock.
	//!
	//! Releases the previously acquired exclusive lock.
	AS_API void              asReleaseExclusiveLock();
	//! \ingroup api_multithread_functions
	//! \brief Acquire a shared lock.
	//!
	//! This function will block the calling thread until there are no 
	//! other threads that hold exclusive locks. Other threads may hold
	//! shared locks.
	AS_API void              asAcquireSharedLock();
	//! \ingroup api_multithread_functions
	//! \brief Release a shared lock.
	//!
	//! Releases the previously acquired shared lock.
	AS_API void              asReleaseSharedLock();
	//! \ingroup api_multithread_functions
	//! \brief Increments the value by one and returns the result as a single atomic instruction
	//! \param[in] value A reference to the value that should be incremented
	//! \return The incremented value
	//!
	//! This function is especially useful for implementing thread safe reference counters.
	AS_API int               asAtomicInc(int &value);
	//! \ingroup api_multithread_functions
	//! \brief Decrements the value by one and returns the result as a single atomic instruction
	//! \param[in] value A reference to the value that should be decremented
	//! \return The decremented value
	//!
	//! This function is especially useful for implementing thread safe reference counters.
	AS_API int               asAtomicDec(int &value);
	//! \ingroup api_multithread_functions
	//! \brief Cleans up memory allocated for the current thread.
	//! \return A negative value on error.
	//! \retval asCONTEXT_ACTIVE A context is still active.
	//!
	//! Call this function before terminating a thread that has
	//! accessed the engine to clean up memory allocated for that thread.
	//!
	//! It's not necessary to call this if only a single thread accesses the engine.
	AS_API int               asThreadCleanup();

	// Memory management
	//! \ingroup api_memory_functions
	//! \brief Set the memory management functions that AngelScript should use.
	//! \param[in] allocFunc The function that will be used to allocate memory.
	//! \param[in] freeFunc The function that will be used to free the memory.
	//! \return A negative value on error.
	//!
	//! Call this method to register the global memory allocation and deallocation
	//! functions that AngelScript should use for memory management. This function
	//! should be called before \ref asCreateScriptEngine.
	//!
	//! If not called, AngelScript will use the malloc and free functions from the
	//! standard C library.
	AS_API int   asSetGlobalMemoryFunctions(asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc);
	//! \ingroup api_memory_functions
	//! \brief Remove previously registered memory management functions.
	//! \return A negative value on error.
	//!
	//! Call this method to restore the default memory management functions.
	AS_API int   asResetGlobalMemoryFunctions();
	//! \ingroup api_memory_functions
	//! \brief Allocate memory using the memory function registered with AngelScript
	//! \param[in] size The size of the buffer to allocate
	//! \return A pointer to the allocated buffer, or null on error.
	AS_API void *asAllocMem(size_t size);
	//! \ingroup api_memory_functions
	//! \brief Deallocates memory using the memory function registered with AngelScript
	//! \param[in] mem A pointer to the buffer to deallocate
	AS_API void  asFreeMem(void *mem);

	// Auxiliary
	//! \ingroup api_multithread_functions
	//! \brief Create a lockable shared boolean
	//! \return A new lockable shared boolean.
	//!
	//! The lockable shared boolean will be created with
	//! an initial reference count of 1, and the boolean
	//! value false.
	//!
	//! The object can be used for weak reference flags.
	AS_API asILockableSharedBool *asCreateLockableSharedBool();
}
#endif // ANGELSCRIPT_DLL_MANUAL_IMPORT

// Determine traits of a type for registration of value types
// Relies on C++11 features so it can not be used with non-compliant compilers
#ifdef AS_CAN_USE_CPP11

#if 0 // Doxygen doesn't like this
END_AS_NAMESPACE
#include <type_traits>
BEGIN_AS_NAMESPACE
#endif

//! \ingroup api_principal_functions
//! \brief Returns the appropriate flags for use with RegisterObjectType.
//! \tparam T The type for which the flags should be determined
//! \return The flags necessary to register this type as a value type
//!
//! \note This function is only availabe if the compiler supports C++11 feature set. Check existance with \#if \ref AS_CAN_USE_CPP11.
//!
//! This template function uses C++11 STL template functions to determine
//! the appropriate flags to use when registering the desired type as a value
//! type with \ref asIScriptEngine::RegisterObjectType.
//!
//! It is capable to determine all the \ref asEObjTypeFlags "asOBJ_APP_xxx" flags, except for 
//! \ref asOBJ_APP_CLASS_ALLINTS, \ref asOBJ_APP_CLASS_ALLFLOATS, and \ref asOBJ_APP_CLASS_ALIGN8. 
//! These flags must still be informed manually when needed.
//!
//! \see \ref doc_reg_val_2
template<typename T>
asUINT asGetTypeTraits()
{
#if defined(_MSC_VER) || defined(_LIBCPP_TYPE_TRAITS) || (__GNUC__ >= 5) || defined(__clang__)
	// MSVC, XCode/Clang, and gnuc 5+
	// C++11 compliant code
	bool hasConstructor        = std::is_default_constructible<T>::value && !std::is_trivially_default_constructible<T>::value;
	bool hasDestructor         = std::is_destructible<T>::value          && !std::is_trivially_destructible<T>::value;
	bool hasAssignmentOperator = std::is_copy_assignable<T>::value       && !std::is_trivially_copy_assignable<T>::value;
	bool hasCopyConstructor    = std::is_copy_constructible<T>::value    && !std::is_trivially_copy_constructible<T>::value;
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
	// gnuc 4.8 is using a mix of C++11 standard and pre-standard templates
	bool hasConstructor        = std::is_default_constructible<T>::value && !std::has_trivial_default_constructor<T>::value;
	bool hasDestructor         = std::is_destructible<T>::value          && !std::is_trivially_destructible<T>::value;
	bool hasAssignmentOperator = std::is_copy_assignable<T>::value       && !std::has_trivial_copy_assign<T>::value;
	bool hasCopyConstructor    = std::is_copy_constructible<T>::value    && !std::has_trivial_copy_constructor<T>::value;
#else
	// All other compilers and versions are assumed to use non C++11 compliant code until proven otherwise
	// Not fully C++11 compliant. The has_trivial checks were used while the standard was still
	// being elaborated, but were then removed in favor of the above is_trivially checks
	// http://stackoverflow.com/questions/12702103/writing-code-that-works-when-has-trivial-destructor-is-defined-instead-of-is
	// https://github.com/mozart/mozart2/issues/51
	bool hasConstructor        = std::is_default_constructible<T>::value && !std::has_trivial_default_constructor<T>::value;
	bool hasDestructor         = std::is_destructible<T>::value          && !std::has_trivial_destructor<T>::value;
	bool hasAssignmentOperator = std::is_copy_assignable<T>::value       && !std::has_trivial_copy_assign<T>::value;
	bool hasCopyConstructor    = std::is_copy_constructible<T>::value    && !std::has_trivial_copy_constructor<T>::value;
#endif
	bool isFloat     = std::is_floating_point<T>::value;
	bool isPrimitive = std::is_integral<T>::value || std::is_pointer<T>::value || std::is_enum<T>::value;
	bool isClass     = std::is_class<T>::value;
	bool isArray     = std::is_array<T>::value;

	if( isFloat )
		return asOBJ_APP_FLOAT;
	if( isPrimitive )
		return asOBJ_APP_PRIMITIVE;

	if( isClass )
	{
		asDWORD flags = asOBJ_APP_CLASS;
		if( hasConstructor )
			flags |= asOBJ_APP_CLASS_CONSTRUCTOR;
		if( hasDestructor )
			flags |= asOBJ_APP_CLASS_DESTRUCTOR;
		if( hasAssignmentOperator )
			flags |= asOBJ_APP_CLASS_ASSIGNMENT;
		if( hasCopyConstructor )
			flags |= asOBJ_APP_CLASS_COPY_CONSTRUCTOR;
		return flags;
	}

	if( isArray )
		return asOBJ_APP_ARRAY;

	// Unknown type traits
	return 0;
}

#endif // c++11

// Interface declarations

//! \defgroup api_principal_interfaces Principal interfaces
//! \defgroup api_secondary_interfaces Secondary interfaces
//! \defgroup api_auxiliary_interfaces Auxiliary interfaces
//! \defgroup api_principal_functions Principal functions
//! \defgroup api_memory_functions Memory functions
//! \defgroup api_multithread_functions Multi-thread support functions
//! \defgroup api_auxiliary_functions Auxiliary functions

//! \ingroup api_principal_interfaces
//! \brief The engine interface
//!
//! The engine is the central object. It is where the application 
//! \ref doc_register_api_topic "registers the application interface"
//! that the scripts should be able to use, and it is where the application can 
//! \ref doc_compile_script "request modules to build scripts" and 
//! \ref doc_call_script_func "contexts to execute them".
//!
//! The engine instance is created with a call to \ref asCreateScriptEngine. 
//!
//! It is allowed to have multiple instances of script engines, but there is rarely a need for it.
//! Even if the application needs to expose different interfaces to different types of scripts
//! this can usually be accomplished through the use of \ref doc_adv_dynamic_config "configuration groups" 
//! and \ref doc_adv_access_mask "access profiles".
class asIScriptEngine
{
public:
	// Memory management
	//! \name Memory management
	//! \{

	//! \brief Increase reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when storing an additional reference to the object.
	//! Remember that the first reference that is received from \ref asCreateScriptEngine
	//! is already accounted for.
	virtual int AddRef() const = 0;
	//! \brief Decrease reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when you will no longer use the references that you own.
	//!
	//! If you know that the engine is supposed to be shut down, then 
	//! it is recommended to call the \ref ShutDownAndRelease method instead.
	virtual int Release() const = 0;
	//! \brief Shuts down the engine then decrease the reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when you know it is time to shut down the engine. This
	//! will automatically discard all the script modules and run a complete 
	//! garbage collection cycle.
	//!
	//! Calling this method rather than the ordinary \ref Release method will
	//! avoid potential memory leaks if for example there are objects in the modules
	//! or garbage collector that indirectly holds a reference to the engine.
	virtual int ShutDownAndRelease() = 0;
	//! \}

	// Engine properties
	//! \name Engine properties
	//! \{

	//! \brief Dynamically change some engine properties.
	//! \param[in] property One of the \ref asEEngineProp values.
	//! \param[in] value The new value of the property.
	//! \return Negative value on error.
	//! \retval asINVALID_ARG Invalid property.
	//!
	//! With this method you can change the way the script engine works in some regards.
	virtual int     SetEngineProperty(asEEngineProp property, asPWORD value) = 0;
	//! \brief Retrieve current engine property settings.
	//! \param[in] property One of the \ref asEEngineProp values.
	//! \return The value of the property, or 0 if it is an invalid property.
	//!
	//! Calling this method lets you determine the current value of the engine properties.
	virtual asPWORD GetEngineProperty(asEEngineProp property) const = 0;
	//! \}

	// Compiler messages
	//! \name Compiler messages
	//! \{

	//! \brief Sets a message callback that will receive compiler messages.
	//! \param[in] callback A function or class method pointer.
	//! \param[in] obj      The object for methods, or an optional parameter for functions.
	//! \param[in] callConv The calling convention.
	//! \return A negative value for an error.
	//! \retval asINVALID_ARG   One of the arguments is incorrect, e.g. obj is null for a class method.
	//! \retval asNOT_SUPPORTED The arguments are not supported, e.g. asCALL_GENERIC.
	//!
	//! This method sets the callback routine that will receive compiler messages.
	//! The callback routine can be either a class method, e.g:
	//! \code
	//! void MyClass::MessageCallback(const asSMessageInfo *msg);
	//! r = engine->SetMessageCallback(asMETHOD(MyClass,MessageCallback), &obj, asCALL_THISCALL);
	//! \endcode
	//! or a global function, e.g:
	//! \code
	//! void MessageCallback(const asSMessageInfo *msg, void *param);
	//! r = engine->SetMessageCallback(asFUNCTION(MessageCallback), param, asCALL_CDECL);
	//! \endcode
	//! It is recommended to register the message callback routine right after creating the engine,
	//! as some of the registration functions can provide useful information to better explain errors.
	virtual int SetMessageCallback(const asSFuncPtr &callback, void *obj, asDWORD callConv) = 0;
	//! \brief Clears the registered message callback routine.
	//! \return A negative value on error.
	//!
	//! Call this method to remove the message callback.
	virtual int ClearMessageCallback() = 0;
	//! \brief Writes a message to the message callback.
	//! \param[in] section The name of the script section.
	//! \param[in] row The row number.
	//! \param[in] col The column number.
	//! \param[in] type The message type.
	//! \param[in] message The message text.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The section or message is null.
	//!
	//! This method can be used by the application to write messages
	//! to the same message callback that the script compiler uses. This
	//! is useful for example if a preprocessor is used.
	virtual int WriteMessage(const char *section, int row, int col, asEMsgType type, const char *message) = 0;
	//! \}

	// JIT Compiler
	//! \name JIT compiler
	//! \{

	//! \brief Sets the JIT compiler
	//! \param[in] compiler A pointer to the JIT compiler
	//! \return A negative value on error.
	//! 
	//! This method is used to set the JIT compiler. The engine
	//! will automatically invoke the JIT compiler when it is set
	//! after compiling scripts or loading pre-compiled byte code.
	//! 
	//! \see \ref doc_adv_jit
	virtual int SetJITCompiler(asIJITCompiler *compiler) = 0;
	//! \brief Returns the JIT compiler
	//! \return Returns a pointer to the JIT compiler
	virtual asIJITCompiler *GetJITCompiler() const = 0;
	//! \}

	// Global functions
	//! \name Global functions
	//! \{

	//! \brief Registers a global function.
	//! \param[in] declaration The declaration of the global function in script syntax.
	//! \param[in] funcPointer The function pointer.
	//! \param[in] callConv The calling convention for the function.
	//! \param[in] auxiliary A helper object for use with some calling conventions.
	//! \return A negative value on error, or the function id if successful.
	//! \retval asNOT_SUPPORTED The calling convention is not supported.
	//! \retval asWRONG_CALLING_CONV The function's calling convention doesn't match \a callConv.
	//! \retval asINVALID_DECLARATION The function declaration is invalid.
	//! \retval asNAME_TAKEN The function name is already used elsewhere.
	//! \retval asALREADY_REGISTERED The function has already been registered with the same parameter list.
	//! \retval asINVALID_ARG The \a auxiliary pointer wasn't set according to calling convention.
	//!
	//! This method registers system functions that the scripts may use to communicate with the host application.
	//!
	//! The \a auxiliary pointer can optionally be used with \ref asCALL_GENERIC.
	//! For the calling convention \ref asCALL_THISCALL_ASGLOBAL the \a auxiliary is required.
	//!
	//! \see \ref doc_register_func
	virtual int                RegisterGlobalFunction(const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0) = 0;
	//! \brief Returns the number of registered functions.
	//! \return The number of registered functions.
	virtual asUINT             GetGlobalFunctionCount() const = 0;
	//! \brief Returns the registered function.
	//! \param[in] index The index of the registered global function.
	//! \return The function object, or null on error.
	virtual asIScriptFunction *GetGlobalFunctionByIndex(asUINT index) const = 0;
	//! \brief Returns the registered function.
	//! \param[in] declaration The signature of the function.
	//! \return The function object, or null on error.
	virtual asIScriptFunction *GetGlobalFunctionByDecl(const char *declaration) const = 0;
	//! \}

	// Global properties
	//! \name Global properties
	//! \{

	//! \brief Registers a global property.
	//! \param[in] declaration The declaration of the global property in script syntax.
	//! \param[in] pointer The address of the property that will be used to access the property value.
	//! \return The index of the property on success, or a negative value on error.
	//! \retval asINVALID_DECLARATION The declaration has invalid syntax.
	//! \retval asINVALID_TYPE The declaration is a reference.
	//! \retval asINVALID_ARG The pointer is null.
	//! \retval asNAME_TAKEN The name is already taken.
	//!
	//! Use this method to register a global property that the scripts will be
	//! able to access as global variables. The property may optionally be registered
	//! as const, if the scripts shouldn't be allowed to modify it.
	//!
	//! When registering the property, the application must pass the address to
	//! the actual value. The application must also make sure that this address
	//! remains valid throughout the life time of this registration, i.e. until
	//! the engine is released or the dynamic configuration group is removed.
	//!
	//! Upon success the function returns the index of the registered property  
	//! that can be used to lookup the info with \ref GetGlobalPropertyByIndex.
	//! Note that this index may not stay valid after a \ref doc_adv_dynamic_config "dynamic config group" has 
	//! been removed, which would reorganize the internal structure.
	virtual int    RegisterGlobalProperty(const char *declaration, void *pointer) = 0;
	//! \brief Returns the number of registered global properties.
	//! \return The number of registered global properties.
	virtual asUINT GetGlobalPropertyCount() const = 0;
	//! \brief Returns the detail on the registered global property.
	//! \param[in] index The index of the global variable.
	//! \param[out] name Receives the name of the property.
	//! \param[out] nameSpace Receives the namespace of the property.
	//! \param[out] typeId Receives the typeId of the property.
	//! \param[out] isConst Receives the constness indicator of the property.
	//! \param[out] configGroup Receives the config group in which the property was registered.
	//! \param[out] pointer Receives the pointer of the property.
	//! \param[out] accessMask Receives the access mask of the property.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG \a index is too large.
	virtual int    GetGlobalPropertyByIndex(asUINT index, const char **name, const char **nameSpace = 0, int *typeId = 0, bool *isConst = 0, const char **configGroup = 0, void **pointer = 0, asDWORD *accessMask = 0) const = 0;
	//! \brief Returns the index of the property.
	//! \param[in] name The name of the property.
	//! \return The index of the matching property or negative on error.
	//! \retval asNO_GLOBAL_VAR No matching property was found.
	//! \retval asINVALID_ARG The name and scope for search cannot be determined.
	//!
	//! The search for global properties will be performed in the default namespace as given 
	//! by \ref SetDefaultNamespace unless the name is prefixed with a scope, using 
	//! the scoping operator ::. If the scope starts with :: it will be used as the 
	//! absolute scope, otherwise it will be relative to the default namespace.
	virtual int    GetGlobalPropertyIndexByName(const char *name) const = 0;
	//! \brief Returns the index of the property.
	//! \param[in] decl The declaration of the property to search for.
	//! \return The index of the matching property or negative on error.
	//! \retval asNO_GLOBAL_VAR No matching property was found.
	//! \retval asINVALID_DECLARATION The given declaration is invalid.
	virtual int    GetGlobalPropertyIndexByDecl(const char *decl) const = 0;
	//! \}

	// Object types
	//! \name Object types
	//! \{

	//! \brief Registers a new object type.
	//! \param[in] obj The name of the type.
	//! \param[in] byteSize The size of the type in bytes. Only necessary for value types.
	//! \param[in] flags One or more of the asEObjTypeFlags.
	//! \return The type id on success or a negative value on error.
	//! \retval asINVALID_ARG The flags are invalid.
	//! \retval asINVALID_NAME The name is invalid.
	//! \retval asALREADY_REGISTERED Another type of the same name already exists.
	//! \retval asNAME_TAKEN The name conflicts with other symbol names.
	//! \retval asLOWER_ARRAY_DIMENSION_NOT_REGISTERED When registering an array type the array element must be a primitive or a registered type.
	//! \retval asINVALID_TYPE The array type was not properly formed.
	//! \retval asNOT_SUPPORTED The array type is not supported, or already in use preventing it from being overloaded.
	//!
	//! Use this method to register new types that should be available to the scripts.
	//! Reference types, which have their memory managed by the application, should be registered with \ref asOBJ_REF.
	//! Value types, which have their memory managed by the engine, should be registered with \ref asOBJ_VALUE.
	//!
	//! \see \ref doc_register_type
	virtual int            RegisterObjectType(const char *obj, int byteSize, asDWORD flags) = 0;
	//! \brief Registers a property for the object type.
	//! \param[in] obj The name of the type.
	//! \param[in] declaration The property declaration in script syntax.
	//! \param[in] byteOffset The offset into the memory block where this property is found.
	//! \param[in] compositeOffset The offset to the composite object.
	//! \param[in] isCompositeIndirect Set to false if the composite object is inline, and true if it is refered to by pointer.
	//! \return The index of the property on success, or a negative value on error.
	//! \retval asWRONG_CONFIG_GROUP The object type was registered in a different configuration group.
	//! \retval asINVALID_OBJECT The \a obj does not specify an object type.
	//! \retval asINVALID_TYPE The \a obj parameter has invalid syntax.
	//! \retval asINVALID_DECLARATION The \a declaration is invalid.
	//! \retval asNAME_TAKEN The name conflicts with other members.
	//!
	//! Use this method to register a member property of a class. The property must
	//! be local to the object, i.e. not a global variable or a static member. The
	//! easiest way to get the offset of the property is to use the asOFFSET macro.
	//!
	//! \code
	//! struct MyType {float prop;};
	//! r = engine->RegisterObjectProperty("MyType", "float prop", asOFFSET(MyType, prop)));
	//! \endcode
	//! 
	//! In case the property to be registered is part of a composite member, then the compositeOffset should be used
	//! to give the offset to the composite member, and byteOffset should be the offset to the property in that composite member.
	//! If the composite member is inline then set isCompositeIndirect as false, else set it to true for proper indirection.
	//!
	//! The method returns the index of the property upon success. This can be used to look up the 
	//! property in the object type with \ref asITypeInfo::GetProperty.
	virtual int            RegisterObjectProperty(const char *obj, const char *declaration, int byteOffset, int compositeOffset = 0, bool isCompositeIndirect = false) = 0;
	//! \brief Registers a method for the object type.
	//! \param[in] obj The name of the type.
	//! \param[in] declaration The declaration of the method in script syntax.
	//! \param[in] funcPointer The method or function pointer.
	//! \param[in] callConv The calling convention for the method or function.
	//! \param[in] auxiliary A helper object for use with some calling conventions.
	//! \param[in] compositeOffset The offset to the composite object.
	//! \param[in] isCompositeIndirect Set to false if the composite object is inline, and true if it is refered to by pointer.
	//! \return A negative value on error, or the function id if successful.
	//! \retval asWRONG_CONFIG_GROUP The object type was registered in a different configuration group.
	//! \retval asNOT_SUPPORTED The calling convention is not supported.
	//! \retval asINVALID_TYPE The \a obj parameter is not a valid object name.
	//! \retval asINVALID_DECLARATION The \a declaration is invalid.
	//! \retval asNAME_TAKEN The name conflicts with other members.
	//! \retval asWRONG_CALLING_CONV The function's calling convention isn't compatible with \a callConv.
	//! \retval asALREADY_REGISTERED The method has already been registered with the same parameter list.
	//! \retval asINVALID_ARG The \a auxiliary pointer wasn't set according to calling convention.
	//!
	//! Use this method to register a member method for the type. The method
	//! that is registered may be an actual class method, or a global function
	//! that takes the object pointer as either the first or last parameter. Or
	//! it may be a global function implemented with the generic calling convention.
	//!
	//! The \a auxiliary pointer can optionally be used with \ref asCALL_GENERIC.
	//! For the calling conventions \ref asCALL_THISCALL_OBJFIRST and asCALL_THISCALL_OBJLAST the \a auxiliary is required.
	//!
	//! In case the method to be registered is part of a composite member, then the compositeOffset should be used
	//! to give the offset to the composite member, and the method pointer should be method of the composite member.
	//! If the composite member is inline then set isCompositeIndirect as false, else set it to true for proper indirection.
	//!
	//! \see \ref doc_register_func
	virtual int            RegisterObjectMethod(const char *obj, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0, int compositeOffset = 0, bool isCompositeIndirect = false) = 0;
	//! \brief Registers a behaviour for the object type.
	//! \param[in] obj The name of the type.
	//! \param[in] behaviour One of the object behaviours from \ref asEBehaviours.
	//! \param[in] declaration The declaration of the method in script syntax.
	//! \param[in] funcPointer The method or function pointer.
	//! \param[in] callConv The calling convention for the method or function.
	//! \param[in] auxiliary A helper object for use with some calling conventions.
	//! \param[in] compositeOffset The offset to the composite object.
	//! \param[in] isCompositeIndirect Set to false if the composite object is inline, and true if it is refered to by pointer.
	//! \return A negative value on error, or the function id is successful.
	//! \retval asWRONG_CONFIG_GROUP The object type was registered in a different configuration group.
	//! \retval asINVALID_ARG \a obj is not set, or a global behaviour is given in \a behaviour, or the \a objForThiscall pointer wasn't set according to calling convention.
	//! \retval asWRONG_CALLING_CONV The function's calling convention isn't compatible with \a callConv.
	//! \retval asNOT_SUPPORTED The calling convention or the behaviour signature is not supported.
	//! \retval asINVALID_TYPE The \a obj parameter is not a valid object name.
	//! \retval asINVALID_DECLARATION The \a declaration is invalid.
	//! \retval asILLEGAL_BEHAVIOUR_FOR_TYPE The \a behaviour is not allowed for this type.
	//! \retval asALREADY_REGISTERED The behaviour is already registered with the same signature.
	//!
	//! Use this method to register behaviour functions that will be called by
	//! the virtual machine to perform certain operations, such as memory management,
	//! math operations, comparisons, etc.
	//! 
	//! The \a declaration must form a valid function signature, but the give function name will
	//! not be used or stored in the application so there is no need to provide a meaningful function 
	//! name.
	//!
	//! The \a auxiliary pointer can optionally be used with \ref asCALL_GENERIC.
	//! For the calling conventions \ref asCALL_THISCALL_ASGLOBAL, \ref asCALL_THISCALL_OBJFIRST and asCALL_THISCALL_OBJLAST the \a auxiliary is required.
	//!
	//! In case the method to be registered is part of a composite member, then the compositeOffset should be used
	//! to give the offset to the composite member, and the method pointer should be method of the composite member.
	//! If the composite member is inline then set isCompositeIndirect as false, else set it to true for proper indirection.
	//!
	//! \see \ref doc_register_func, \ref doc_reg_opbeh
	virtual int            RegisterObjectBehaviour(const char *obj, asEBehaviours behaviour, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0, int compositeOffset = 0, bool isCompositeIndirect = false) = 0;
	//! \brief Registers a script interface.
	//! \param[in] name The name of the interface.
	//! \return The type id of the interface on success, else a negative value on error.
	//! \retval asINVALID_NAME The \a name is null, or a reserved keyword.
	//! \retval asALREADY_REGISTERED An object type with this name already exists.
	//! \retval asERROR The \a name is not a proper identifier.
	//! \retval asNAME_TAKEN The \a name is already used elsewhere.
	//!
	//! This registers an interface that script classes can implement. By doing this the application 
	//! can register functions and methods that receives an \ref asIScriptObject and still be sure that the 
	//! class implements certain methods needed by the application. 
	//!
	//! \see \ref doc_use_script_class_3
	virtual int            RegisterInterface(const char *name) = 0;
	//! \brief Registers a script interface method.
	//! \param[in] intf The name of the interface.
	//! \param[in] declaration The method declaration.
	//! \return A negative value on error.
	//! \retval asWRONG_CONFIG_GROUP The interface was registered in another configuration group.
	//! \retval asINVALID_TYPE \a intf is not an interface type.
	//! \retval asINVALID_DECLARATION The \a declaration is invalid.
	//! \retval asNAME_TAKEN The method name is already taken.
	//!
	//! This registers a method that the class that implements the script interface must have.
	virtual int            RegisterInterfaceMethod(const char *intf, const char *declaration) = 0;
	//! \brief Returns the number of registered object types.
	//! \return The number of object types registered by the application.
	virtual asUINT         GetObjectTypeCount() const = 0;
	//! \brief Returns the object type interface by index.
	//! \param[in] index The index of the type.
	//! \return The registered object type interface for the type, or null if not found.
	virtual asITypeInfo   *GetObjectTypeByIndex(asUINT index) const = 0;
	//! \}

	// String factory
	//! \name String factory
	//! \{

	//! \brief Registers the string factory.
	//! \param[in] datatype The datatype that the string factory returns
	//! \param[in] factory The pointer to the factory object
	//! \return A negative value on error, or the function id if successful.
	//! \retval asINVALID_ARG The \a factory is null.
	//! \retval asINVALID_TYPE The \a datatype is not a valid type, or it is a reference or handle.
	//!
	//! Use this function to register a string factory that will be called during compilation 
	//! to create instances of a string constant. The string factory will also be used while
	//! saving bytecode in order to get the raw string data for serialization.
	//!
	//! The data type that represents the string type should be informed without reference or handle
	//! token, as the script engine will assume a const reference anyway.
	//!
	//! \see \ref doc_strings
	virtual int RegisterStringFactory(const char *datatype, asIStringFactory *factory) = 0;
	//! \brief Returns the type id of the type that the string factory returns.
	//! \return The type id of the type that the string type returns, or a negative value on error.
	//! \param[out] flags The \ref asETypeModifiers "type modifiers" for the return type
	//! \retval asNO_FUNCTION The string factory has not been registered.
	virtual int GetStringFactoryReturnTypeId(asDWORD *flags = 0) const = 0;
	//! \}

	// Default array type
	//! \name Default array type
	//! \{

	//! \brief Registers the type that should be used as the default array
	//! \param[in] type The name of the template type, e.g. array<T>
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The type is not a template type
	virtual int RegisterDefaultArrayType(const char *type) = 0;
	//! \brief Returns the type id of the registered type
	//! \return The type id, or a negative value on error.
	//! \retval asINVALID_TYPE The default array type hasn't been registered.
	virtual int GetDefaultArrayTypeId() const = 0;
	//! \}

	// Enums
	//! \name Enums
	//! \{

	//! \brief Registers an enum type.
	//! \param[in] type The name of the enum type.
	//! \return The type id on success, or a negative value on error.
	//! \retval asINVALID_NAME \a type is null, not an identifier, or it is a reserved keyword.
	//! \retval asALREADY_REGISTERED Another type with this name already exists.
	//! \retval asERROR The \a type couldn't be parsed.
	//! \retval asNAME_TAKEN The type name is already taken.
	//!
	//! This method registers an enum type in the engine. The enum values should then be registered 
	//! with \ref RegisterEnumValue.
	virtual int          RegisterEnum(const char *type) = 0;
	//! \brief Registers an enum value.
	//! \param[in] type The name of the enum type.
	//! \param[in] name The name of the enum value.
	//! \param[in] value The integer value of the enum value.
	//! \return A negative value on error.
	//! \retval asWRONG_CONFIG_GROUP The enum \a type was registered in a different configuration group.
	//! \retval asINVALID_TYPE The \a type is invalid.
	//! \retval asALREADY_REGISTERED The \a name is already registered for this enum.
	//!
	//! This method registers an enum value for a previously registered enum type.
	virtual int          RegisterEnumValue(const char *type, const char *name, int value) = 0;
	//! \brief Returns the number of registered enum types.
	//! \return The number of registered enum types.
	virtual asUINT       GetEnumCount() const = 0;
	//! \brief Returns the registered enum type.
	//! \param[in] index The index of the enum type.
	//! \return The type info of the registered enum type, or null on error.
	virtual asITypeInfo *GetEnumByIndex(asUINT index) const = 0;
	//! \}

	// Funcdefs
	//! \name Funcdefs
	//! \{

	//! \brief Registers a function definition.
	//! \param[in] decl The declaration of the function definition.
	//! \return The type id on success, else a negative value on error.
	//! \retval asINVALID_ARG The \a decl parameter is not given.
	//! \retval asINVALID_DECLARATION \a decl is not a valid function definition.
	//! \retval asNAME_TAKEN The name of the funcdef conflicts with another name.
	//!
	//! \ref doc_datatypes_funcptr "Funcdefs" are used to define the signature of
	//! function pointers. If the application is going to receive function pointers
	//! from scripts, it is necessary to first register the funcdef before registering
	//! the function or property that will be used to receive it.
	//! 
	//! Funcdefs are usually registered as global entities, but can also be registered
	//! as a child of a class. To do this simply prefix the name of the funcdef with the 
	//! name of the class and the scope operator to specify which class should be the owner.
	virtual int          RegisterFuncdef(const char *decl) = 0;
	//! \brief Returns the number of registered function definitions.
	//! \return The number of registered funcdefs.
	virtual asUINT       GetFuncdefCount() const = 0;
	//! \brief Returns a registered function definition.
	//! \param[in] index The index of the funcdef.
	//! \return The type info of the funcdef.
	//!
	//! This function does not increase the reference count of the returned function definition.
	virtual asITypeInfo *GetFuncdefByIndex(asUINT index) const = 0;
	//! \}

	// Typedefs
	//! \name Typedefs
	//! \{

	//! \brief Registers a typedef.
	//! \param[in] type The name of the new typedef
	//! \param[in] decl The datatype that the typedef represents
	//! \return The type id on success, else a negative value on error.
	//! \retval asINVALID_NAME The \a type is null, is not an identifier, or it is a reserved keyword.
	//! \retval asALREADY_REGISTERED A type with the same name already exists.
	//! \retval asINVALID_TYPE The \a decl is not a primitive type.
	//! \retval asNAME_TAKEN The name is already used elsewhere.
	//!
	//! This method registers an alias for a data type.
	//!
	//! Currently typedefs can only be registered for built-in primitive types.
	virtual int          RegisterTypedef(const char *type, const char *decl) = 0;
	//! \brief Returns the number of registered typedefs.
	//! \return The number of registered typedefs.
	virtual asUINT       GetTypedefCount() const = 0;
	//! \brief Returns a registered typedef.
	//! \param[in] index The index of the typedef.
	//! \return The type info of the typedef.
	virtual asITypeInfo *GetTypedefByIndex(asUINT index) const = 0;
	//! \}

	// Configuration groups
	//! \name Configuration groups
	//! \{

	//! \brief Starts a new dynamic configuration group.
	//! \param[in] groupName The name of the configuration group
	//! \return A negative value on error
	//! \retval asNAME_TAKEN Another group with the same name already exists.
	//! \retval asNOT_SUPPORTED Nesting configuration groups is not supported.
	//!
	//! Starts a new dynamic configuration group. This group can be setup so that it is only 
	//! visible to specific modules, and it can also be removed when it is no longer used.
	//!
	//! \see \ref doc_adv_dynamic_config
	virtual int         BeginConfigGroup(const char *groupName) = 0;
	//! \brief Ends the configuration group.
	//! \return A negative value on error
	//! \retval asERROR Can't end a group that hasn't been begun.
	//!
	//! Ends the current configuration group. Once finished a config group cannot be changed, 
	//! but it can be removed when it is no longer used.
	//!
	//! \see \ref doc_adv_dynamic_config
	virtual int         EndConfigGroup() = 0;
	//! \brief Removes a previously registered configuration group.
	//! \param[in] groupName The name of the configuration group
	//! \return A negative value on error
	//! \retval asCONFIG_GROUP_IS_IN_USE The group is in use and cannot be removed.
	//!
	//! Remove the configuration group. If something in the configuration group is currently in 
	//! use, the function will return with an error code. Examples of uses are compiled modules 
	//! that have function calls to functions in the group and global variables of types registered 
	//! in the group.
	//!
	//! \see \ref doc_adv_dynamic_config
	virtual int         RemoveConfigGroup(const char *groupName) = 0;
	//! \brief Sets the access mask that should be used for subsequent registered entities.
	//! \param[in] defaultMask The default access bit mask.
	//! \return The previous default mask.
	//!
	//! \see \ref doc_adv_access_mask
	virtual asDWORD     SetDefaultAccessMask(asDWORD defaultMask) = 0;
	//! \brief Sets the current default namespace for registrations and searches.
	//! \param[in] nameSpace The namespace that should be used.
	//! \return A negative value on error
	//! \retval asINVALID_ARG The namespace is invalid
	//!
	//! Call this method to set the default namespace for which the following calls
	//! should assume. This applies to registration of the application interface and
	//! also to the functions that searches for registered entities.
	//!
	//! Nested namespaces can be informed by separating them with the scope token, i.e. ::
	virtual int         SetDefaultNamespace(const char *nameSpace) = 0;
	//! \brief Returns the current default namespace
	//! \return The current default namespace
	virtual const char *GetDefaultNamespace() const = 0;
	//! \}

	// Script modules
	//! \name Script modules
	//! \{

	//! \brief Return an interface pointer to the module.
	//! \param[in] module The name of the module
	//! \param[in] flag One of the \ref asEGMFlags flags
	//! \return A pointer to the module interface
	//!
	//! Use this method to get access to the module interface, which will
	//! let you build new scripts, and enumerate functions and types in
	//! existing modules.
	//!
	//! If \ref asGM_ALWAYS_CREATE is informed as the flag the previous
	//! module with the same name will be discarded, thus any pointers that
	//! the engine holds to it will be invalid after the call.
	virtual asIScriptModule *GetModule(const char *module, asEGMFlags flag = asGM_ONLY_IF_EXISTS) = 0;
	//! \brief Discard a module.
	//! \param[in] module The name of the module
	//! \return A negative value on error
	//! \retval asNO_MODULE The module was not found.
	//!
	//! Discards a module and frees its memory. Any pointers that the application holds 
	//! to this module will be invalid after this call.
	virtual int              DiscardModule(const char *module) = 0;
	//! \brief Get the number of modules.
	//! \return The number of modules.
	virtual asUINT           GetModuleCount() const = 0;
	//! \brief Get a module by index.
	//! \param[in] index The index of the module.
	//! \return A pointer to the module or null on error.
	virtual asIScriptModule *GetModuleByIndex(asUINT index) const = 0;
	//! \}

	// Script functions
	//! \name Script functions
	//! \{

	//! \brief Returns the function by its id.
	//! \param[in] funcId The id of the function or method.
	//! \return A pointer to the function description interface, or null if not found.
	//!
	//! This does not increment the reference count of the returned function interface.
	virtual asIScriptFunction *GetFunctionById(int funcId) const = 0;
	//! \}

	// Type identification
	//! \name Type identification
	//! \{

	//! \brief Returns a type id by declaration.
	//! \param[in] decl The declaration of the type.
	//! \return A negative value on error, or the type id of the type.
	//! \retval asINVALID_TYPE \a decl is not a valid type.
	//!
	//! Translates a type declaration into a type id. The returned type id is valid for as long as
	//! the type is valid, so you can safely store it for later use to avoid potential overhead by 
	//! calling this function each time. Just remember to update the type id, any time the type is 
	//! changed within the engine, e.g. when recompiling script declared classes, or changing the 
	//! engine configuration.
	//!
	//! The type id is based on a sequence number and depends on the order in which the type ids are
	//! queried, thus is not guaranteed to always be the same for each execution of the application.
	//! The \ref asETypeIdFlags can be used to obtain some information about the type directly from the id.
	//!
	//! A base type yields the same type id whether the declaration is const or not, however if the
	//! const is for the subtype then the type id is different, e.g. string@ isn't the same as const
	//! string@ but string is the same as const string.
	//!
	//! This method is only able to return the type id that are not specific for a script module, i.e.
	//! built-in types and application registered types. Type ids for script declared types should
	//! be obtained through the script module's \ref asIScriptModule::GetTypeIdByDecl "GetTypeIdByDecl".
	virtual int            GetTypeIdByDecl(const char *decl) const = 0;
	//! \brief Returns a type declaration.
	//! \param[in] typeId The type id of the type.
	//! \param[in] includeNamespace Set to true if the namespace should be included in the formatted declaration.
	//! \return A null terminated string with the type declaration, or null if not found.
	virtual const char    *GetTypeDeclaration(int typeId, bool includeNamespace = false) const = 0;
	//! \brief Returns the size of a primitive type.
	//! \param[in] typeId The type id of the type.
	//! \return The size of the type in bytes, or zero if it is not a primitive type.
	//!
	//! This method can be used to return the size of any built-in primitive type,
	//! and also for script declared or application registered enums.
	virtual int            GetSizeOfPrimitiveType(int typeId) const = 0;
	//! \brief Returns the type interface for type.
	//! \param[in] typeId The type id of the type.
	//! \return The type interface for the type, or null if not found.
	//!
	//! This does not increment the reference count of the returned type.
	virtual asITypeInfo   *GetTypeInfoById(int typeId) const = 0;
	//! \brief Returns the type interface by name.
	//! \param[in] name The name of the type.
	//! \return The type interface for the type, or null if not found.
	//!
	//! The search for types will be performed in the default namespace as given 
	//! by \ref SetDefaultNamespace unless the name is prefixed with a scope, using 
	//! the scoping operator ::. If the scope starts with :: it will be used as the 
	//! absolute scope, otherwise it will be relative to the default namespace.
	virtual asITypeInfo   *GetTypeInfoByName(const char *name) const = 0;
	//! \brief Returns a type by declaration.
	//! \param[in] decl The declaration of the type.
	//! \return The type or null on error.
	//!
	//! Translates a type declaration into the type info. The returned type is valid for as 
	//! long as the type is valid, so you can safely store it for later use to avoid potential overhead from 
	//! calling this function each time. Just remember to update the type info pointer any time the type is 
	//! changed within the engine, e.g. when recompiling script declared classes, or changing the 
	//! engine configuration.
	virtual asITypeInfo   *GetTypeInfoByDecl(const char *decl) const = 0;
	//! \}

	// Script execution
	//! \name Script execution
	//! \{

	//! \brief Creates a new script context.
	//! \return A pointer to the new script context.
	//!
	//! This method creates a context that will be used to execute the script functions. 
	//! The context interface created will have its reference counter already increased.
	//!
	//! \see \ref RequestContext
	virtual asIScriptContext      *CreateContext() = 0;
	//! \brief Creates an object defined by its type.
	//! \param[in] type The type of the object to create.
	//! \return A pointer to the new object if successful, or null if not.
	//!
	//! This method is used to create an object based on it's type. The method will 
	//! call the object type's default factory. If the object type doesn't have a default 
	//! factory the call will fail and no object will be created.
	//!
	//! Created objects will have their reference counter set to 1 so the application 
	//! needs to release the pointer when it will no longer use it.
	//!
	//! If the type is a registered value type, then the memory for the object will be 
	//! allocated using the \ref asSetGlobalMemoryFunctions "default memory routine". To 
	//! destroy and and deallocate the object it is best to use \ref ReleaseScriptObject.
	//!
	//! The method only works for objects, for primitive types and object handles the method 
	//! doesn't do anything and returns a null pointer.
	virtual void                  *CreateScriptObject(const asITypeInfo *type) = 0;
	//! \brief Creates a copy of a script object.
	//! \param[in] obj A pointer to the source object.
	//! \param[in] type The type of the object.
	//! \return A pointer to the new object if successful, or null if not.
	//!
	//! This method is used to create a copy of an existing object.
	//!
	//! This only works for objects, for primitive types and object handles the method 
	//! doesn't do anything and returns a null pointer.
	virtual void                  *CreateScriptObjectCopy(void *obj, const asITypeInfo *type) = 0;
	//! \brief Creates an uninitialized script object defined by its type.
	//! \param[in] type The type of the object to create.
	//! \return A pointer to the new object if successful, or null if not.
	//!
	//! This method can only be used to create instances of script classes. 
	//!
	//! The returned object will only be initialized so far that
	//! there are no invalid pointers or references. The constructor of the
	//! script class will not be invoked.
	//!
	//! If the script class has any registered types as members, the default 
	//! constructor for those members will be executed.
	//!
	//! This method is meant for objects that will be initialized manually 
	//! by the application, e.g. when restoring a serialized object.
	virtual void                  *CreateUninitializedScriptObject(const asITypeInfo *type) = 0;
	//! \brief Create a delegate for an object and method
	//! \param[in] func The object method
	//! \param[in] obj The object pointer
	//! \return The new delegate instance
	virtual asIScriptFunction     *CreateDelegate(asIScriptFunction *func, void *obj) = 0;
	//! \brief Copy one script object to another.
	//! \param[in] dstObj A pointer to the destination object.
	//! \param[in] srcObj A pointer to the source object.
	//! \param[in] type The type of the objects.
	//! \return A negative value on error
	//! \retval asINVALID_ARG One of the arguments is null
	//! \retval asNOT_SUPPORTED The object type is a ref type and value assignment has been turned off
	//!
	//! This calls the assignment operator to copy the object from one to the other.
	//!
	//! This only works for objects.
	virtual int                    AssignScriptObject(void *dstObj, void *srcObj, const asITypeInfo *type) = 0;
	//! \brief Release the object pointer.
	//! \param[in] obj A pointer to the object.
	//! \param[in] type The type of the object.
	//!
	//! This calls the release method of the object to release the reference.
	//!
	//! If the type is a value type, the method will destroy the object and deallocate
	//! the memory using the \ref asSetGlobalMemoryFunctions "default memory routine".
	virtual void                   ReleaseScriptObject(void *obj, const asITypeInfo *type) = 0;
	//! \brief Increase the reference counter for the script object.
	//! \param[in] obj A pointer to the object.
	//! \param[in] type The type of the object.
	//!
	//! This calls the add ref method of the object to increase the reference count.
	virtual void                   AddRefScriptObject(void *obj, const asITypeInfo *type) = 0;
	//! \brief Returns the handle on a successful reference cast to desired type
	//! \param[in] obj A pointer to the object.
	//! \param[in] fromType The type of the object.
	//! \param[in] toType The desired type for the cast.
	//! \param[out] newPtr The new pointer to the object if successful.
	//! \param[in] useOnlyImplicitCast If only the implicit reference cast operators should be used.
	//! \return A negative value on error
	//! \retval asINVALID_ARG A null pointer was supplied
	//!
	//! This method is used to cast an pointer to a different type. While both the new 
	//! and old pointers are expected to refer to the same instance, the address of the 
	//! pointers are not necessarily the same.
	//! 
	//! If the cast is successful the \a newPtr will be set to the new pointer,
	//! and the reference counter will be incremented. If the cast is not successful,
	//! the \a newPtr will be set to null, and the reference count left unchanged.
	virtual int                    RefCastObject(void *obj, asITypeInfo *fromType, asITypeInfo *toType, void **newPtr, bool useOnlyImplicitCast = false) = 0;
	//! \brief Returns the weak ref flag from the object.
	//! \param[in] obj The object
	//! \param[in] type The object type
	//! \return The weak ref flag, if the object supports weak references.
	//!
	//! As long as the weak ref flag is not set, the owning object is still alive. Once the weak ref flag
	//! is set, the object is dead and should no longer be accessed. Check if the flag is set with the 
	//! \ref asILockableSharedBool::Get method.
	//!
	//! This method doesn't increase the reference to the returned shared boolean.
	virtual asILockableSharedBool *GetWeakRefFlagOfScriptObject(void *obj, const asITypeInfo *type) const = 0;
	//! \}

	// Context pooling
	//! \name Context pooling
	//! \{

	//! \brief Request a context
	//! \return An unprepared context
	//!
	//! This method will invoke the registered request context callback
	//! and return an available context in an unprepared state.
	//! 
	//! Contexts obtained through this method shouldn't be released, instead
	//! they should be returned to the origin with a call to \ref ReturnContext.
	//!
	//! \see \ref CreateContext
	virtual asIScriptContext      *RequestContext() = 0;
	//! \brief Return a context when it won't be used anymore
	//! \param[in] ctx The context that should be returned to the origin
	virtual void                   ReturnContext(asIScriptContext *ctx) = 0;
	//! \brief Register context callbacks for pooling
	//! \param[in] requestCtx The request context callback function
	//! \param[in] returnCtx The return context callback function
	//! \param[in] param An optional parameter that will be passed to the callback
	//! \return A negative value on error
	//! \retval asINVALID_ARG Only one of the context functions is informed
	//!
	//! This method can be used by the application to implement a context pool, 
	//! or to perform custom configuration on the contexts that the engine uses internally.
	//!
	//! This can for example be used to debug calls to initialize global variables
	//! when building modules, or to detect script exceptions that may occur in 
	//! script class destructors when called from the garbage collector.
	virtual int                    SetContextCallbacks(asREQUESTCONTEXTFUNC_t requestCtx, asRETURNCONTEXTFUNC_t returnCtx, void *param = 0) = 0;
	//! \}

	// String interpretation
	//! \name String interpretation
	//! \{

	//! \brief Returns the class and length of the first token in the string.
	//! \param[in] string The string to parse.
	//! \param[in] stringLength The length of the string. Can be 0 if the string is null terminated.
	//! \param[out] tokenLength Gives the length of the identified token.
	//! \return One of the \ref asETokenClass values.
	//!
	//! This function is useful for those applications that want to tokenize strings into 
	//! tokens that the script language uses, e.g. IDEs providing syntax highlighting, or intellisense.
	//! It can also be used to parse the meta data strings that may be declared for script entities.
	virtual asETokenClass ParseToken(const char *string, size_t stringLength = 0, asUINT *tokenLength = 0) const = 0;
	//! \}

	// Garbage collection
	//! \name Garbage collection
	//! \{

	//! \brief Perform garbage collection.
	//! \param[in] flags Set to a combination of the \ref asEGCFlags.
	//! \param[in] numIterations The number of iterations to perform when not doing a full cycle
	//! \return 1 if the cycle wasn't completed, 0 if it was.
	//!
	//! This method will free script objects that can no longer be reached. When the engine 
	//! is released the garbage collector will automatically do a full cycle to release all 
	//! objects still alive. If the engine is long living it is important to call this method 
	//! every once in a while to free up memory allocated by the scripts. If a script does a 
	//! lot of allocations before returning it may be necessary to implement a line callback 
	//! function that calls the garbage collector during execution of the script.
	//! 
	//! It is not necessary to do a full cycle with every call. This makes it possible to spread 
	//! out the garbage collection time over a large period, thus not impacting the responsiveness 
	//! of the application.
	//!
	//! \see \ref doc_gc
	virtual int  GarbageCollect(asDWORD flags = asGC_FULL_CYCLE, asUINT numIterations = 1) = 0;
	//! \brief Obtain statistics from the garbage collector.
	//! \param[out] currentSize The current number of objects known to the garbage collector.
	//! \param[out] totalDestroyed The total number of objects destroyed by the garbage collector.
	//! \param[out] totalDetected The total number of objects detected as garbage with circular references.
	//! \param[out] newObjects The current number of objects in the new generation.
	//! \param[out] totalNewDestroyed The total number of objects destroyed while still in the new generation.
	//!
	//! This method can be used to query the number of objects that the garbage collector is 
	//! keeping track of. If the number is very large then it is probably time to call the 
	//! \ref GarbageCollect method so that some of the objects ca be freed.
	//!
	//! \see \ref doc_gc
	virtual void GetGCStatistics(asUINT *currentSize, asUINT *totalDestroyed = 0, asUINT *totalDetected = 0, asUINT *newObjects = 0, asUINT *totalNewDestroyed = 0) const = 0;
	//! \brief Notify the garbage collector of a new object that needs to be managed.
	//! \param[in] obj A pointer to the newly created object.
	//! \param[in] type The type of the object.
	//! \return The sequence number of the added object, or a negative value on error.
	//! \retval asINVALID_ARG Either the object or the type is null
	//!
	//! This method should be called when a new garbage collected object is created. 
	//! The GC will then store a reference to the object so that it can automatically 
	//! detect whether the object is involved in any circular references that should be released.
	//!
	//! \see \ref doc_gc_object
	virtual int  NotifyGarbageCollectorOfNewObject(void *obj, asITypeInfo *type) = 0;
	//! \brief Gets an object in the garbage collector
	//! \param[in] idx The index of the desired object
	//! \param[out] seqNbr The sequence number of the obtained object
	//! \param[out] obj The object pointer
	//! \param[out] type The type of the obtained object
	//! \return A negative value on error
	//! \retval asINVALID_ARG The index is not valid
	virtual int  GetObjectInGC(asUINT idx, asUINT *seqNbr = 0, void **obj = 0, asITypeInfo **type = 0) = 0;
	//! \brief Used by the garbage collector to enumerate all references held by an object.
	//! \param[in] reference A pointer to the referenced object.
	//!
	//! When processing the \ref asBEHAVE_ENUMREFS call the called object should call GCEnumCallback 
	//! for each of the references it holds to other objects. If the object holds a value type
	//! that may contain references, then use the \ref ForwardGCEnumReferences.
	//!
	//! \see \ref doc_gc_object
	virtual void GCEnumCallback(void *reference) = 0;
	//! \brief Used to forward GC callback to held value types that may contain references
	//! \param[in] ref The object pointer
	//! \param[in] type The type of the object
	//!
	//! This should be used by reference types that implement the \ref asBEHAVE_ENUMREFS
	//! behaviour when the object holds a value type that can in turn contain references.
	//!
	//! \see \ref doc_gc_object
	virtual void ForwardGCEnumReferences(void *ref, asITypeInfo *type) = 0;
	//! \brief Used to forward GC callback to held value types that may contain references
	//! \param[in] ref The object pointer
	//! \param[in] type The type of the object
	//!
	//! This should be used by reference types that implement the \ref asBEHAVE_RELEASEREFS
	//! behaviour when the object holds a value type that can in turn contain references.
	//!
	//! \see \ref doc_gc_object
	virtual void ForwardGCReleaseReferences(void *ref, asITypeInfo *type) = 0;
	//! \brief Set a callback for capturing more info on circular reference for debugging
	//! \param[in] callback The callback function
	//! \param[in] param Optional parameter that will be passed back to the callback function
	//!
	//! This callback is meant to be used during development to help identify scripts that are
	//! creating circular references. As the callback will be invoked when the objects in the circular
	//! reference are detected, but before they are destroyed, the application can investigate their
	//! content to get hints where the objects are created from.
	//!
	//! \see \ref doc_gc
	virtual void SetCircularRefDetectedCallback(asCIRCULARREFFUNC_t callback, void *param = 0) = 0;
	//! \}

	// User data
	//! \name User data
	//! \{

	//! \brief Register the memory address of some user data.
	//! \param[in] data A pointer to the user data.
	//! \param[in] type An identifier specifying the user data to set.
	//! \return The previous pointer stored in the engine.
	//!
	//! This method allows the application to associate a value, e.g. a pointer, with the engine instance.
	//!
	//! The type values 1000 through 1999 are reserved for use by the official add-ons.
	//!
	//! Optionally, a callback function can be \ref SetEngineUserDataCleanupCallback "registered" to clean up the user data when the engine is destroyed.
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	//! \brief Returns the address of the previously registered user data.
	//! \param[in] type An identifier specifying the user data to get.
	//! \return The pointer to the user data.
	virtual void *GetUserData(asPWORD type = 0) const = 0;
	//! \brief Set the function that should be called when the engine is destroyed
	//! \param[in] callback A pointer to the function
	//! \param[in] type An identifier specifying which user data the callback is to be used with.
	//!
	//! The function given with this call will be invoked when the engine
	//! is destroyed if any \ref SetUserData "user data" has been registered with the engine.
	//!
	//! The function is called from within the engine destructor, so the callback
	//! should not be used for anything but cleaning up the user data itself.
	virtual void  SetEngineUserDataCleanupCallback(asCLEANENGINEFUNC_t callback, asPWORD type = 0) = 0;
	//! \brief Set the function that should be called when the module is destroyed
	//! \param[in] callback A pointer to the function
	//! \param[in] type An identifier specifying which user data the callback is to be used with.
	//!
	//! The function given with this call will be invoked when the module
	//! is destroyed if any \ref asIScriptModule::SetUserData "user data" has been registered with the module.
	//!
	//! The function is called from within the module destructor, so the callback
	//! should not be used for anything but cleaning up the user data itself.
	virtual void  SetModuleUserDataCleanupCallback(asCLEANMODULEFUNC_t callback, asPWORD type = 0) = 0;
	//! \brief Set the function that should be called when a context is destroyed
	//! \param[in] callback A pointer to the function
	//! \param[in] type An identifier specifying which user data the callback is to be used with.
	//!
	//! The function given with this call will be invoked when a context
	//! is destroyed if any \ref asIScriptContext::SetUserData "user data" has been registered with the context.
	//!
	//! The function is called from within the context destructor, so the callback
	//! should not be used for anything but cleaning up the user data itself.
	virtual void  SetContextUserDataCleanupCallback(asCLEANCONTEXTFUNC_t callback, asPWORD type = 0) = 0;
	//! \brief Set the function that should be called when a function is destroyed
	//! \param[in] callback A pointer to the function
	//! \param[in] type An identifier specifying which user data the callback is to be used with.
	//!
	//! The function given with this call will be invoked when a function
	//! is destroyed if any \ref asIScriptFunction::SetUserData "user data" has been registered with the function.
	//!
	//! The function is called from within the function destructor, so the callback
	//! should not be used for anything but cleaning up the user data itself.
	virtual void  SetFunctionUserDataCleanupCallback(asCLEANFUNCTIONFUNC_t callback, asPWORD type = 0) = 0;
	//! \brief Set the function that should be called when a type info is destroyed
	//! \param[in] callback A pointer to the function
	//! \param[in] type An identifier specifying which user data the callback is to be used with.
	//!
	//! The function given with this call will be invoked when a type info
	//! is destroyed if any \ref asITypeInfo::SetUserData "user data" has been registered with the type.
	//!
	//! The function is called from within the type info destructor, so the callback
	//! should not be used for anything but cleaning up the user data itself.
	virtual void  SetTypeInfoUserDataCleanupCallback(asCLEANTYPEINFOFUNC_t callback, asPWORD type = 0) = 0;
	//! \brief Set the function that should be called when a script object is destroyed
	//! \param[in] callback A pointer to the function
	//! \param[in] type An identifier specifying which user data the callback is to be used with.
	//!
	//! The function given with this call will be invoked when a script object instance
	//! is destroyed if any \ref asIScriptObject::SetUserData "user data" has been registered with the type.
	//!
	//! The function is called from within the script object destructor, so the callback
	//! should not be used for anything but cleaning up the user data itself.
	virtual void  SetScriptObjectUserDataCleanupCallback(asCLEANSCRIPTOBJECTFUNC_t callback, asPWORD type = 0) = 0;
	//! \}

	// Exception handling
	//! \name Exception handling
	//! \{

	//! \brief Register the exception translation callback.
	//! \param[in] callback The callback function/method that should be called upon an exception.
	//! \param[in] param A user defined parameter, or the object pointer on which the callback is called.
	//! \param[in] callConv The calling convention of the callback function/method.
	//! \return A negative value on error.
	//! \retval asNOT_SUPPORTED Calling convention must not be asCALL_GENERIC, or the routine's calling convention is not supported.
	//! \retval asINVALID_ARG   \a param must not be null for class methods.
	//! \retval asWRONG_CALLING_CONV \a callConv isn't compatible with the routines' calling convention.
	//!
	//! This callback function will be called by the VM when an application exception is raised, which 
	//! allow the application to translate the exception into a useful string to inform in \ref asIScriptContext::SetException "SetException".
	//!
	//! The callback function signature must be either:
	//!
	//! <pre>  void (*)(asIScriptContext *, void *);</pre>
	//!
	//! or
	//!
	//! <pre>  void (param::*)(asIScriptContext *);</pre>
	//!
	//! See \ref doc_cpp_exceptions_1 for an example on how to use this.
	virtual int SetTranslateAppExceptionCallback(asSFuncPtr callback, void *param, int callConv) = 0;
	//! \}

protected:
	virtual ~asIScriptEngine() {}
};

//! \ingroup api_secondary_interfaces
//! \brief The interface for the string factory
//!
//! This interface is used to manage the string constants that the scripts
//! use. If string constants should be supported the application must implement
//! this object and register it with \ref asIScriptEngine::RegisterStringFactory.
class asIStringFactory
{
public:
	//! \brief Called by engine to instantiate a string constant
	//! \param[in] data The content of the string
	//! \param[in] length The length in bytes of the data buffer
	//! \return The pointer to the instantiated string constant
	//!
	//! The contents of \a data must be copied by the string factory, as the 
	//! engine will not keep a copy of the original data.
	//!
	//! The string factory can cache and return a pointer to the same instance
	//! multiple times if the same string content is requested multiple times.
	//! If the same instance is returned multiple times the string factory must
	//! keep track of the number of instances as \ref ReleaseStringConstant
	//! will be called for each of them.
	virtual const void *GetStringConstant(const char *data, asUINT length) = 0;
	//! \brief Called by engine when the string constant is no longer used.
	//! \param[in] str The same pointer returned by \ref GetStringConstant
	//! \return A negative value on error.
	//!
	//! The engine will call this method for each pointer returned by \ref GetStringConstant.
	//! If the string factory returns a pointer to the same instance multiple times, then
	//! the string instance can only be destroyed when the last call to ReleaseStringConstant
	//! for that pointer is made.
	virtual int         ReleaseStringConstant(const void *str) = 0;
	//! \brief Called by engine to get the raw string data for serialization.
	//! \param[in] str The same pointer returned by \ref GetStringConstant
	//! \param[out] data A pointer to the data buffer that should be filled with the content
	//! \param[out] length A pointer to the variable that should be set with the length of the data
	//! \return A negative value on error.
	//!
	//! The engine will first call this with \a data set to null to retrieve the size of the 
	//! buffer that must be allocated. Then the engine will call the method once more with
	//! the allocated data buffer to be filled with the content. The length should always be
	//! informed in number of bytes.
	virtual int         GetRawStringData(const void *str, char *data, asUINT *length) const = 0;

protected:
	virtual ~asIStringFactory() {}
};
#endif

//! \ingroup api_auxiliary_functions
//! \brief The interface for the thread manager
//!
//! This interface is used to represent the internal thread manager
//! prepared with \ref asPrepareMultithread() and returned by 
//! \ref asGetThreadManager(). The application shouldn't do anything
//! with this except pass it to another call to \ref asPrepareMultithread()
//! in case there is a need to share a common thread manager across multiple
//! application modules (dlls).
class asIThreadManager
{
protected:
	virtual ~asIThreadManager() {}
};

//! \ingroup api_principal_interfaces
//! \brief The interface to the script modules
//!
//! A script module can be thought of a library of script functions, classes, and global variables. 
//! 
//! A pointer to the module interface is obtained by calling asIScriptEngine::GetModule. The module can
//! then be \ref asIScriptModule::Build "built" from a single or multiple script files, also known as script sections. Alternatively
//! pre-built bytecode can be \ref asIScriptModule::LoadByteCode "loaded", if it has been saved from a previous build.
//! 
//! \see \ref doc_module
class asIScriptModule
{
public:
	//! \name Miscellaneous
	//! \{

	//! \brief Returns a pointer to the engine.
	//! \return A pointer to the engine.
	virtual asIScriptEngine *GetEngine() const = 0;
	//! \brief Sets the name of the module.
	//! \param[in] name The new name.
	//!
	//! Sets the name of the script module.
	virtual void             SetName(const char *name) = 0;
	//! \brief Gets the name of the module.
	//! \return The name of the module.
	virtual const char      *GetName() const = 0;
	//! \brief Discards the module.
	//!
	//! This method is used to discard the module and any
	//! compiled bytecode it has. After calling this method
	//! the module pointer is no longer valid and shouldn't
	//! be used by the application.
	virtual void             Discard() = 0;
	//! \}

	// Compilation
	//! \name Compilation
	//! \{

	//! \brief Add a script section for the next build.
	//! \param[in] name The name of the script section
	//! \param[in] code The script code buffer
	//! \param[in] codeLength The length of the script code
	//! \param[in] lineOffset An offset that will be added to compiler message line numbers
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The \a code argument is null.
	//! \retval asNOT_SUPPORTED Compiler support is disabled in the engine.
	//! \retval asOUT_OF_MEMORY The necessary memory to hold the script code couldn't be allocated.
	//!
	//! This adds a script section to the module. The script section isn't processed with this
	//! call. Only when \ref Build is called will the script be parsed and compiled into 
	//! executable byte code.
	//!
	//! Error messages from the compiler will refer to the name of the script section and the position
	//! within it. Normally each section is the content of a source file, so it is recommended to name
	//! the script sections as the name of the source file.
	//!
	//! The code added is copied by the engine, so there is no need to keep the original buffer after the call.
	//! Note that this can be changed by setting the engine property \ref asEP_COPY_SCRIPT_SECTIONS
	//! with \ref asIScriptEngine::SetEngineProperty.
	virtual int         AddScriptSection(const char *name, const char *code, size_t codeLength = 0, int lineOffset = 0) = 0;
	//! \brief Build the previously added script sections.
	//! \return A negative value on error
	//! \retval asINVALID_CONFIGURATION The engine configuration is invalid.
	//! \retval asERROR The script failed to build.
	//! \retval asBUILD_IN_PROGRESS Another thread is currently building. 
	//! \retval asINIT_GLOBAL_VARS_FAILED It was not possible to initialize at least one of the global variables.
	//! \retval asNOT_SUPPORTED Compiler support is disabled in the engine.
	//! \retval asMODULE_IS_IN_USE The code in the module is still being used and and cannot be removed. 
	//!
	//! Builds the script based on the previously \ref AddScriptSection "added sections", \ref doc_register_api "registered types and functions". 
	//! After the build is complete the script sections are removed to free memory. 
	//!
	//! Before starting the build the \ref Build method removes any previously compiled script content, including the
	//! dynamically added content from \ref CompileFunction and \ref CompileGlobalVar. If the script 
	//! module needs to be rebuilt all of the script sections needs to be added again.
	//!
	//! Compiler messages are sent to the message callback function set with \ref asIScriptEngine::SetMessageCallback. 
	//! If there are no errors or warnings, no messages will be sent to the callback function.
	//!
	//! Any global variables found in the script will be initialized by the
	//! compiler if the engine property \ref asEP_INIT_GLOBAL_VARS_AFTER_BUILD is set. If you get the error
	//! \ref asINIT_GLOBAL_VARS_FAILED, then it is probable that one of the global variables during the initialization 
	//! is trying to access another global variable before it has been initialized. 
	//!
	//! \see \ref doc_compile_script
	virtual int         Build() = 0;
	//! \brief Compile a single function.
	//! \param[in] sectionName The name of the script section
	//! \param[in] code The script code buffer
	//! \param[in] lineOffset An offset that will be added to compiler message line numbers
	//! \param[in] compileFlags One of \ref asECompileFlags values
	//! \param[out] outFunc Optional parameter to receive the compiled function descriptor 
	//! \return A negative value on error
	//! \retval asINVALID_ARG One or more arguments have invalid values.
	//! \retval asINVALID_CONFIGURATION The engine configuration is invalid.
	//! \retval asBUILD_IN_PROGRESS Another build is in progress.
	//! \retval asERROR The compilation failed.
	//! \retval asNOT_SUPPORTED Compiler support is disabled in the engine.
	//!
	//! Use this to compile a single function. Any existing compiled code in the module can be used
	//! by the function. 
	//!
	//! The newly compiled function can be optionally added to the scope of the module where it can later
	//! be referred to by the application or used in subsequent compilations. If not added to the module 
	//! the function can still be returned in the output parameter, which will allow the application 
	//! to execute it and then discard it when it is no longer needed.
	//!
	//! If the output function parameter is set, remember to release the function object when you're done with it.
	virtual int         CompileFunction(const char *sectionName, const char *code, int lineOffset, asDWORD compileFlags, asIScriptFunction **outFunc) = 0;
	//! \brief Compile a single global variable and add it to the scope of the module
	//!
	//! \param[in] sectionName The name of the script section
	//! \param[in] code The script code buffer
	//! \param[in] lineOffset An offset that will be added to compiler message line numbers
	//! \return A negative value on error
	//! \retval asINVALID_ARG One or more arguments have invalid values.
	//! \retval asINVALID_CONFIGURATION The engine configuration is invalid.
	//! \retval asBUILD_IN_PROGRESS Another build is in progress.
	//! \retval asERROR The compilation failed.
	//! \retval asNOT_SUPPORTED Compiler support is disabled in the engine.
	//!
	//! Use this to add a single global variable to the scope of a module. The variable can then
	//! be referred to by the application and subsequent compilations.
	//!
	//! The script code may contain an initialization expression, which will be executed by the
	//! compiler if the engine property \ref asEP_INIT_GLOBAL_VARS_AFTER_BUILD is set.
	//!
	//! Any existing compiled code in the module can be used in the initialization expression.
	virtual int         CompileGlobalVar(const char *sectionName, const char *code, int lineOffset) = 0;
	//! \brief Sets the access mask that should be used during the compilation.
	//! \param[in] accessMask The access bit mask
	//! \return The previous access mask.
	//!
	//! The module's access mask with be bitwise and-ed with the registered entity's access mask
	//! in order to determine if the module is allowed to access the entity. If the result is zero
	//! then the script in the module will not be able to use the entity.
	//!
	//! This can be used to provide different interfaces to scripts that serve different purposes
	//! in the application. 
	//!
	//! \see \ref doc_adv_access_mask
	virtual asDWORD     SetAccessMask(asDWORD accessMask) = 0;
	//! \brief Sets the default namespace that should be used in the following calls.
	//! \param[in] nameSpace The namespace that should be used.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The namespace is null.
	//! \retval asINVALID_DECLARATION The namespace is invalid.
	//!
	//! Set the default namespace that should be used in the following 
	//! calls for searching for declared entities, or when compiling new
	//! individual entities.
	virtual int         SetDefaultNamespace(const char *nameSpace) = 0;
	//! \brief Returns the current default namespace
	//! \return The current default namespace
	virtual const char *GetDefaultNamespace() const = 0;
	//! \}

	// Functions
	//! \name Functions
	//! \{

	//! \brief Returns the number of global functions in the module.
	//! \return The number of global functions in this module.
	//!
	//! This method retrieves the number of compiled script functions.
	virtual asUINT             GetFunctionCount() const = 0;
	//! \brief Returns the function by index
	//! \param[in] index The index of the function
	//! \return The function or null in case of error.
	virtual asIScriptFunction *GetFunctionByIndex(asUINT index) const = 0;
	//! \brief Returns the function by its declaration
	//! \param[in] decl The function declaration.
	//! \return The function or null in case of error.
	virtual asIScriptFunction *GetFunctionByDecl(const char *decl) const = 0;
	//! \brief Returns the function by its name
	//! \param[in] name The function name
	//! \return The function or null if not found or there are multiple matches.
	//!
	//! The search for functions will be performed in the default namespace as given 
	//! by \ref SetDefaultNamespace unless the name is prefixed with a scope, using 
	//! the scoping operator ::. If the scope starts with :: it will be used as the 
	//! absolute scope, otherwise it will be relative to the default namespace.
	virtual asIScriptFunction *GetFunctionByName(const char *name) const = 0;
	//! \brief Remove a single function from the scope of the module
	//! \param[in] func The pointer to the function that should be removed.
	//! \return A negative value on error.
	//! \retval asNO_FUNCTION The function is not part of the scope.
	//!
	//! This method allows the application to remove a single function from the
	//! scope of the module. The function is not destroyed immediately though,
	//! only when no more references point to it.
	virtual int                RemoveFunction(asIScriptFunction *func) = 0;
	//! \}

	// Global variables
	//! \name Global variables
	//! \{

	//! \brief Reset the global variables of the module.
	//! \param[in] ctx Optional script context.
	//! \return A negative value on error.
	//! \retval asERROR The module was not compiled successfully.
	//! \retval asINIT_GLOBAL_VARS_FAILED The initialization of the global variables failed.
	//!
	//! Resets the global variables declared in this module to their initial value. The context should be 
	//! informed if the application needs to have extra control over how the initialization is done, for 
	//! example for debugging, or for catching exceptions.
	virtual int         ResetGlobalVars(asIScriptContext *ctx = 0) = 0;
	//! \brief Returns the number of global variables in the module.
	//! \return The number of global variables in the module.
	virtual asUINT      GetGlobalVarCount() const = 0;
	//! \brief Returns the global variable index by name.
	//! \param[in] name The name of the global variable.
	//! \return A negative value on error, or the global variable index.
	//! \retval asINVALID_ARG The name and scope for search cannot be determined
	//! \retval asNO_GLOBAL_VAR The matching global variable was found.
	//!
	//! This method should be used to retrieve the index of the script variable that you wish to access.
	//!
	//! The search for global variables will be performed in the default namespace as given 
	//! by \ref SetDefaultNamespace unless the name is prefixed with a scope, using 
	//! the scoping operator ::. If the scope starts with :: it will be used as the 
	//! absolute scope, otherwise it will be relative to the default namespace.
	virtual int         GetGlobalVarIndexByName(const char *name) const = 0;
	//! \brief Returns the global variable index by declaration.
	//! \param[in] decl The global variable declaration.
	//! \return A negative value on error, or the global variable index.
	//! \retval asERROR The module was not built successfully.
	//! \retval asNO_GLOBAL_VAR No matching global variable was found.
	//! \retval asINVALID_DECLARATION The given declaration is invalid.
	//!
	//! This method should be used to retrieve the index of the script variable that you wish to access.
	//!
	//! The method will find the script variable with the exact same declaration.
	//!
	//! If the variable is declared in a namespace first call \ref SetDefaultNamespace 
	//! to set the namespace that should be searched first for the variable.
	virtual int         GetGlobalVarIndexByDecl(const char *decl) const = 0;
	//! \brief Returns the global variable declaration.
	//! \param[in] index The index of the global variable.
	//! \param[in] includeNamespace Set to true if the namespace should be included in the declaration.
	//! \return A null terminated string with the variable declaration, or null if not found.
	//!
	//! This method can be used to retrieve the variable declaration of the script variables 
	//! that the host application will access. Verifying the declaration is important because, 
	//! even though the script may compile correctly the user may not have used the variable 
	//! types as intended.
	virtual const char *GetGlobalVarDeclaration(asUINT index, bool includeNamespace = false) const = 0;
	//! \brief Returns the global variable properties.
	//! \param[in] index The index of the global variable.
	//! \param[out] name The name of the variable.
	//! \param[out] nameSpace The namespace of the variable.
	//! \param[out] typeId The type of the variable.
	//! \param[out] isConst Whether or not the variable is const.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The index is out of range.
	virtual int         GetGlobalVar(asUINT index, const char **name, const char **nameSpace = 0, int *typeId = 0, bool *isConst = 0) const = 0;
	//! \brief Returns the pointer to the global variable.
	//! \param[in] index The index of the global variable.
	//! \return A pointer to the global variable, or null if not found.
	//!
	//! This method should be used to retrieve the pointer of a variable that you wish to access.
	virtual void       *GetAddressOfGlobalVar(asUINT index) = 0;
	//! \brief Remove the global variable from the scope of the module.
	//! \param[in] index The index of the global variable.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The index is out of range.
	//!
	//! The global variable is removed from the scope of the module, but 
	//! it is not destroyed until all functions that access it are freed.
	virtual int         RemoveGlobalVar(asUINT index) = 0;
	//! \}

	// Type identification
	//! \name Type identification
	//! \{

	//! \brief Returns the number of object types.
	//! \return The number of object types declared in the module.
	virtual asUINT         GetObjectTypeCount() const = 0;
	//! \brief Returns the object type interface by index.
	//! \param[in] index The index of the type.
	//! \return The object type interface for the type, or null if not found.
	//!
	//! This does not increase the reference count of the returned object.
	virtual asITypeInfo   *GetObjectTypeByIndex(asUINT index) const = 0;
	//! \brief Returns a type id by declaration.
	//! \param[in] decl The declaration of the type.
	//! \return A negative value on error, or the type id of the type.
	//! \retval asINVALID_TYPE \a decl is not a valid type.
	//!
	//! Translates a type declaration into a type id. The returned type id is valid for as long as
	//! the type is valid, so you can safely store it for later use to avoid potential overhead from 
	//! calling this function each time. Just remember to update the type id, any time the type is 
	//! changed within the engine, e.g. when recompiling script declared classes, or changing the 
	//! engine configuration.
	//!
	//! The type id is based on a sequence number and depends on the order in which the type ids are 
	//! queried, thus is not guaranteed to always be the same for each execution of the application.
	//! The \ref asETypeIdFlags can be used to obtain some information about the type directly from the id.
	//!
	//! A base type yields the same type id whether the declaration is const or not, however if the 
	//! const is for the subtype then the type id is different, e.g. string@ isn't the same as const 
	//! string@ but string is the same as const string. 
	virtual int            GetTypeIdByDecl(const char *decl) const = 0;
	//! \brief Returns the type interface by name.
	//! \param[in] name The name of the type.
	//! \return The type interface for the type, or null if not found.
	//!
	//! The search for types will be performed in the default namespace as given 
	//! by \ref SetDefaultNamespace unless the name is prefixed with a scope, using 
	//! the scoping operator ::. If the scope starts with :: it will be used as the 
	//! absolute scope, otherwise it will be relative to the default namespace.
	virtual asITypeInfo   *GetTypeInfoByName(const char *name) const = 0;
	//! \brief Returns a type by declaration.
	//! \param[in] decl The declaration of the type.
	//! \return The type or null on error.
	//!
	//! Translates a type declaration into the type info. The returned type is valid for as 
	//! long as the type is valid, so you can safely store it for later use to avoid potential overhead from 
	//! calling this function each time. Just remember to update the type info pointer any time the type is 
	//! changed within the engine, e.g. when recompiling script declared classes, or changing the 
	//! engine configuration.
	virtual asITypeInfo   *GetTypeInfoByDecl(const char *decl) const = 0;
	//! \}

	// Enums
	//! \name Enums
	//! \{

	//! \brief Returns the number of enum types declared in the module.
	//! \return The number of enum types in the module.
	virtual asUINT       GetEnumCount() const = 0;
	//! \brief Returns the enum type.
	//! \param[in] index The index of the enum type.
	//! \return The type info of the enum type, or null on error.
	virtual asITypeInfo *GetEnumByIndex(asUINT index) const = 0;
	//! \}

	// Typedefs
	//! \name Typedefs
	//! \{

	//! \brief Returns the number of typedefs in the module.
	//! \return The number of typedefs in the module.
	virtual asUINT       GetTypedefCount() const = 0;
	//! \brief Returns the typedef.
	//! \param[in] index The index of the typedef.
	//! \return The type info of the typedef, or null on error.
	virtual asITypeInfo *GetTypedefByIndex(asUINT index) const = 0;
	//! \}

	// Dynamic binding between modules
	//! \name Dynamic binding between modules
	//! \{

	//! \brief Returns the number of functions declared for import.
	//! \return The number of imported functions.
	//!
	//! This function returns the number of functions that are imported in a module. These 
	//! functions need to be bound before they can be used, or a script exception will be thrown.
	virtual asUINT      GetImportedFunctionCount() const = 0;
	//! \brief Returns the imported function index by declaration.
	//! \param[in] decl The function declaration of the imported function.
	//! \return A negative value on error, or the index of the imported function.
	//! \retval asERROR The module was not built successfully.
	//! \retval asMULTIPLE_FUNCTIONS Found multiple matching functions.
	//! \retval asNO_FUNCTION Didn't find any matching function.
	//!
	//! This function is used to find a specific imported function by its declaration.
	virtual int         GetImportedFunctionIndexByDecl(const char *decl) const = 0;
	//! \brief Returns the imported function declaration.
	//! \param[in] importIndex The index of the imported function.
	//! \return A null terminated string with the function declaration, or null if not found.
	//!
	//! Use this function to get the declaration of the imported function. The returned 
	//! declaration can be used to find a matching function in another module that can be bound 
	//! to the imported function. 
	virtual const char *GetImportedFunctionDeclaration(asUINT importIndex) const = 0;
	//! \brief Returns the declared imported function source module.
	//! \param[in] importIndex The index of the imported function.
	//! \return A null terminated string with the name of the source module, or null if not found.
	//!
	//! Use this function to get the name of the suggested module to import the function from.
	virtual const char *GetImportedFunctionSourceModule(asUINT importIndex) const = 0;
	//! \brief Binds an imported function to the function from another module.
	//! \param[in] importIndex The index of the imported function.
	//! \param[in] func The true function that will be bound to the imported function.
	//! \return A negative value on error.
	//! \retval asNO_FUNCTION \a importIndex or \a func is incorrect.
	//! \retval asINVALID_INTERFACE The signature of function doesn't match the import statement.
	//!
	//! The imported function is only bound if the functions have the exact same signature, 
	//! i.e the same return type, and parameters.
	virtual int         BindImportedFunction(asUINT importIndex, asIScriptFunction *func) = 0;
	//! \brief Unbinds an imported function.
	//! \param[in] importIndex The index of the imported function.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The index is not valid.
	//!
	//! Unbinds the imported function.
	virtual int         UnbindImportedFunction(asUINT importIndex) = 0;

	//! \brief Binds all imported functions in a module, by searching their equivalents in the declared source modules.
	//! \return A negative value on error.
	//! \retval asERROR An error occurred.
	//! \retval asCANT_BIND_ALL_FUNCTIONS Not all functions where bound.
	//!
	//! This functions tries to bind all imported functions in the module by searching for matching 
	//! functions in the suggested modules. If a function cannot be bound the function will give an 
	//! error \ref asCANT_BIND_ALL_FUNCTIONS, but it will continue binding the rest of the functions.
	virtual int         BindAllImportedFunctions() = 0;
	//! \brief Unbinds all imported functions.
	//! \return A negative value on error.
	//!
	//! Unbinds all imported functions in the module.
	virtual int         UnbindAllImportedFunctions() = 0;
	//! \}

	// Byte code saving and loading
	//! \name Byte code saving and loading
	//! \{

	//! \brief Save compiled byte code to a binary stream.
	//! \param[in] out The output stream.
	//! \param[in] stripDebugInfo Set to true to skip saving the debug information.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The stream object wasn't specified.
	//! \retval asNOT_SUPPORTED Compiler support is disabled in the engine.
	//! \retval asERROR Nothing has been compiled in the module.
	//!
	//! This method is used to save pre-compiled byte code to disk or memory, for a later restoral.
	//! The application must implement an object that inherits from \ref asIBinaryStream to provide
	//! the necessary stream operations.
	//!
	//! \see \ref doc_adv_precompile
	virtual int SaveByteCode(asIBinaryStream *out, bool stripDebugInfo = false) const = 0;
	//! \brief Load pre-compiled byte code from a binary stream.
	//!
	//! \param[in] in The input stream.
	//! \param[out] wasDebugInfoStripped Set to true if the byte code was saved without debug information.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The stream object wasn't specified.
	//! \retval asBUILD_IN_PROGRESS Another thread is currently building.
	//! \retval asOUT_OF_MEMORY The engine ran out of memory while loading the byte code.
	//! \retval asMODULE_IS_IN_USE The code in the module is still being used and and cannot be removed. 
	//! \retval asERROR It was not possible to load the byte code.
	//!
	//! This method is used to load pre-compiled byte code from disk or memory. The application must
	//! implement an object that inherits from \ref asIBinaryStream to provide the necessary stream operations.
	//!
	//! It is expected that the application performs the necessary validations to make sure the
	//! pre-compiled byte code is from a trusted source. The application should also make sure the
	//! pre-compiled byte code is compatible with the current engine configuration, i.e. that the
	//! engine has been configured in the same way as when the byte code was first compiled.
	//!
	//! If the method returns asERROR it is either because the byte code is incorrect, e.g. corrupted due to
	//! disk failure, or it has been compiled with a different engine configuration. If possible the engine 
	//! provides information about the type of error that caused the failure while loading the byte code to the
	//! \ref asIScriptEngine::SetMessageCallback "message stream". 
	//!
	//! \see \ref doc_adv_precompile
	virtual int LoadByteCode(asIBinaryStream *in, bool *wasDebugInfoStripped = 0) = 0;
	//! \}

	// User data
	//! \name User data
	//! \{

	//! \brief Register the memory address of some user data.
	//! \param[in] data A pointer to the user data.
	//! \param[in] type An identifier specifying the user data to set.
	//! \return The previous pointer stored in the module
	//!
	//! This method allows the application to associate a value, e.g. a pointer, with the module instance.
	//!
	//! The type values 1000 through 1999 are reserved for use by the official add-ons.
	//!
	//! Optionally, a callback function can be \ref asIScriptEngine::SetModuleUserDataCleanupCallback "registered" to clean up the user data when the module is destroyed.
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	//! \brief Returns the address of the previously registered user data.
	//! \param[in] type An identifier specifying the user data to set.
	//! \return The pointer to the user data.
	virtual void *GetUserData(asPWORD type = 0) const = 0;
	//! \}

protected:
	virtual ~asIScriptModule() {}
};

//! \ingroup api_principal_interfaces
//! \brief The interface to the virtual machine
//!
//! The script context provides the interface for a single script execution. The object stores the call
//! stack where the local variables used by the execution are kept, however it doesn't keep copies of 
//! global variables as these are stored in the \ref asIScriptModule, and only referenced by the context.
//!
//! The value stored in a global variable is shared between all contexts, as they all refer to the same 
//! memory. This means that the global variables outlive the execution of a script, and will keep their
//! values between executions.
//!
//! It is seldom necessary to maintain more than one script context at a time, with the only exceptions
//! being when a script calls an application function that in turn calls another script before returning, 
//! and if multiple scripts are running in parallel. 
//!
//! Try to avoid associating a unique context with each object that may need to call scripts. Instead 
//! keep a shared pool of contexts that can be requested by the objects on a need-to basis.
class asIScriptContext
{
public:
	// Memory management
	//! \name Memory management
	//! \{

	//! \brief Increase reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when storing an additional reference to the object.
	//! Remember that the first reference that is received from \ref asIScriptEngine::CreateContext
	//! is already accounted for.
	virtual int AddRef() const = 0;
	//! \brief Decrease reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when you will no longer use the references that you own.
	virtual int Release() const = 0;
	//! \}

	// Miscellaneous
	//! \name Miscellaneous
	//! \{

	//! \brief Returns a pointer to the engine.
	//! \return A pointer to the engine.
	virtual asIScriptEngine *GetEngine() const = 0;
	//! \}

	// Execution
	//! \name Execution
	//! \{

	//! \brief Prepares the context for execution of the function
	//! \param[in] func The id of the function/method that will be executed.
	//! \return A negative value on error.
	//! \retval asCONTEXT_ACTIVE The context is still active or suspended.
	//! \retval asNO_FUNCTION The function pointer is null.
	//! \retval asINVALID_ARG The function is from a different engine than the context.
	//! \retval asOUT_OF_MEMORY The context ran out of memory while allocating call stack.
	//!
	//! This method prepares the context for executeion of a script function. It allocates
	//! the stack space required and reserves space for return value and parameters. The
	//! default value for parameters and return value is 0.
	//!
	//! \see \ref doc_call_script_func
	virtual int             Prepare(asIScriptFunction *func) = 0;
	//! \brief Frees resources held by the context.
	//! \return A negative value on error.
	//! \retval asCONTEXT_ACTIVE The context is still active or suspended.
	//!
	//! This function frees resources held by the context. It's usually not necessary 
	//! to call this function as the resources are automatically freed when the context
	//! is released, or reused when \ref Prepare is called again.
	virtual int             Unprepare() = 0;
	//! \brief Executes the prepared function.
	//! \return A negative value on error, or one of \ref asEContextState.
	//! \retval asCONTEXT_NOT_PREPARED The context is not prepared or it is not in suspended state.
	//! \retval asEXECUTION_ABORTED The execution was aborted with a call to \ref Abort.
	//! \retval asEXECUTION_SUSPENDED The execution was suspended with a call to \ref Suspend.
	//! \retval asEXECUTION_FINISHED The execution finished successfully.
	//! \retval asEXECUTION_EXCEPTION The execution ended with an exception.
	//!
	//! Executes the prepared function until the script returns. If the execution was previously 
	//! suspended the function resumes where it left of.
	//!
	//! Note that if the script freezes, e.g. if trapped in a never ending loop, you may call 
	//! \ref Abort from another thread to stop execution.
	//!
	//! If the function returns asEXECUTION_EXCEPTION, use the \ref GetExceptionString, \ref GetExceptionFunction, 
	//! and \ref GetExceptionLineNumber to obtain more information on the exception and where it occurred.
	//!
	//! \see \ref doc_call_script_func
	virtual int             Execute() = 0;
	//! \brief Aborts the execution.
	//! \return A negative value on error.
	//! \retval asERROR Invalid context object.
	//!
	//! Aborts the current execution of a script.
	//!
	//! If the call to Abort is done from within a function called by the
	//! script, it will only take effect after that function returns.
	virtual int             Abort() = 0;
	//! \brief Suspends the execution, which can then be resumed by calling Execute again.
	//! \return A negative value on error.
	//! \retval asERROR Invalid context object.
	//!
	//! Suspends the current execution of a script. The execution can then be resumed by calling \ref Execute again.
	//!
	//! If the call to Suspend is done from within a function called by the 
	//! script, it will only take effect after that function returns.
	//!
	//! \see \ref doc_call_script_func
	virtual int             Suspend() = 0;
	//! \brief Returns the state of the context.
	//! \return The current state of the context.
	virtual asEContextState GetState() const = 0;
	//! \brief Backups the current state to prepare the context for a nested call.
	//! \return A negative value on error.
	//! \retval asERROR The current context is not active.
	//! \retval asOUT_OF_MEMORY Couldn't allocate memory to store state.
	//!
	//! This method can be invoked on an active context in order
	//! to reuse the context for a nested call, e.g. when a function
	//! called by a script needs to call another script before returning.
	//! After the method returns with success the method \ref Prepare()
	//! shall be invoked to prepare the new execution.
	//!
	//! By reusing an active context the application can avoid creating a 
	//! temporary context, and thus improve the run-time performance.
	//!
	//! For each successful call to PushState() the method \ref PopState() must
	//! be called to return the state in order to resume the previous script 
	//! execution.
	//!
	//! If PushState() fails, the context was not modified, so the application
	//! can just create a different context instead, and when it is done with it
	//! the original context can be resumed normally.
	virtual int             PushState() = 0;
	//! \brief Restores the state to resume previous script execution
	//! \return A negative value on error.
	//! \retval asERROR The state couldn't be restored.
	virtual int             PopState() = 0;
	//! \brief Returns true if the context has any nested calls.
	//! \param[out] nestCount This argument receives the nesting level.
	//! \return true if there are any nested calls.
	virtual bool            IsNested(asUINT *nestCount = 0) const = 0;
	//! \}

	// Object pointer for calling class methods
	//! \name Object pointer for calling class methods
	//! \{

	//! \brief Sets the object for a class method call.
	//! \param[in] obj A pointer to the object.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asERROR The prepared function is not a class method.
	//!
	//! This method sets object pointer when calling class methods.
	virtual int   SetObject(void *obj) = 0;
	//! \}

	// Arguments
	//! \name Arguments
	//! \{

	//! \brief Sets an 8-bit argument value.
	//! \param[in] arg The argument index.
	//! \param[in] value The value of the argument.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not an 8-bit value.
	//!
	//! Sets a 1 byte argument.
	virtual int   SetArgByte(asUINT arg, asBYTE value) = 0;
	//! \brief Sets a 16-bit argument value.
	//! \param[in] arg The argument index.
	//! \param[in] value The value of the argument.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not a 16-bit value.
	//!
	//! Sets a 2 byte argument.
	virtual int   SetArgWord(asUINT arg, asWORD value) = 0;
	//! \brief Sets a 32-bit integer argument value.
	//! \param[in] arg The argument index.
	//! \param[in] value The value of the argument.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not a 32-bit value.
	//!
	//! Sets a 4 byte argument.
	virtual int   SetArgDWord(asUINT arg, asDWORD value) = 0;
	//! \brief Sets a 64-bit integer argument value.
	//! \param[in] arg The argument index.
	//! \param[in] value The value of the argument.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not a 64-bit value.
	//!
	//! Sets an 8 byte argument.
	virtual int   SetArgQWord(asUINT arg, asQWORD value) = 0;
	//! \brief Sets a float argument value.
	//! \param[in] arg The argument index.
	//! \param[in] value The value of the argument.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not a 32-bit value.
	//!
	//! Sets a float argument.
	virtual int   SetArgFloat(asUINT arg, float value) = 0;
	//! \brief Sets a double argument value.
	//! \param[in] arg The argument index.
	//! \param[in] value The value of the argument.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not a 64-bit value.
	//!
	//! Sets a double argument.
	virtual int   SetArgDouble(asUINT arg, double value) = 0;
	//! \brief Sets the address of a reference or handle argument.
	//! \param[in] arg The argument index.
	//! \param[in] addr The address that should be passed in the argument.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not a reference or an object handle.
	//!
	//! Sets an address argument, e.g. an object handle or a reference.
	virtual int   SetArgAddress(asUINT arg, void *addr) = 0;
	//! \brief Sets the object argument value.
	//! \param[in] arg The argument index.
	//! \param[in] obj A pointer to the object.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not an object or handle.
	//!
	//! Sets an object argument. If the argument is an object handle AngelScript will increment the reference
	//! for the object. If the argument is an object value AngelScript will make a copy of the object.
	virtual int   SetArgObject(asUINT arg, void *obj) = 0;
	//! \brief Sets the variable argument value and type.
	//! \param[in] arg The argument index.
	//! \param[in] ptr A pointer to the value.
	//! \param[in] typeId The type id of the value.
	//! \return A negative value on error.
	//! \retval asCONTEXT_NOT_PREPARED The context is not in prepared state.
	//! \retval asINVALID_ARG The \a arg is larger than the number of arguments in the prepared function.
	//! \retval asINVALID_TYPE The argument is not a variable type.
	//!
	//! This method should be used when setting the argument for functions
	//! with \ref doc_adv_var_type "variable parameter types".
	virtual int   SetArgVarType(asUINT arg, void *ptr, int typeId) = 0;
	//! \brief Returns a pointer to the argument for assignment.
	//! \param[in] arg The argument index.
	//! \return A pointer to the argument on the stack.
	//!
	//! This method returns a pointer to the argument on the stack for assignment. For object handles, you
	//! should increment the reference counter. For object values, you should pass a pointer to a copy of the
	//! object.
	virtual void *GetAddressOfArg(asUINT arg) = 0;
	//! \}	

	// Return value
	//! \name Return value
	//! \{

	//! \brief Returns the 8-bit return value.
	//! \return The 1 byte value returned from the script function, or 0 on error.
	virtual asBYTE  GetReturnByte() = 0;
	//! \brief Returns the 16-bit return value.
	//! \return The 2 byte value returned from the script function, or 0 on error.
	virtual asWORD  GetReturnWord() = 0;
	//! \brief Returns the 32-bit return value.
	//! \return The 4 byte value returned from the script function, or 0 on error.
	virtual asDWORD GetReturnDWord() = 0;
	//! \brief Returns the 64-bit return value.
	//! \return The 8 byte value returned from the script function, or 0 on error.
	virtual asQWORD GetReturnQWord() = 0;
	//! \brief Returns the float return value.
	//! \return The float value returned from the script function, or 0 on error.
	virtual float   GetReturnFloat() = 0;
	//! \brief Returns the double return value.
	//! \return The double value returned from the script function, or 0 on error.
	virtual double  GetReturnDouble() = 0;
	//! \brief Returns the address for a reference or handle return type.
	//! \return The address value returned from the script function, or 0 on error.
	//!
	//! The method doesn't increase the reference counter with this call, so if you store
	//! the pointer of a reference counted object you need to increase the reference manually
	//! otherwise the object will be released when the context is released or reused.
	virtual void   *GetReturnAddress() = 0;
	//! \brief Return a pointer to the returned object.
	//! \return A pointer to the object returned from the script function, or 0 on error.
	//! 
	//! The method doesn't increase the reference counter with this call, so if you store
	//! the pointer of a reference counted object you need to increase the reference manually
	//! otherwise the object will be released when the context is released or reused.
	virtual void   *GetReturnObject() = 0;
	//! \brief Returns the address of the returned value
	//! \return A pointer to the return value returned from the script function, or 0 on error.
	virtual void   *GetAddressOfReturnValue() = 0;
	//! \}

	// Exception handling
	//! \name Exception handling
	//! \{

	//! \brief Sets an exception, which aborts the execution.
	//! \param[in] info A string that describes the exception that occurred.
	//! \param[in] allowCatch Set to false if the script shouldn't be allowed to catch the exception.
	//! \return A negative value on error.
	//! \retval asERROR The context isn't currently calling an application registered function.
	//!
	//! This method sets a script exception in the context. This will only work if the 
	//! context is currently calling a system function, thus this method can only be 
	//! used for system functions.
	//!
	//! Note that if your system function sets an exception, it should not return any 
	//! object references because the engine will not release the returned reference.
	virtual int                SetException(const char *info, bool allowCatch = true) = 0;
	//! \brief Returns the line number where the exception occurred.
	//! \param[out] column The variable will receive the column number.
	//! \param[out] sectionName The variable will receive the name of the script section.
	//! \return The line number where the exception occurred.
	//!
	//! This method returns the line number where the exception ocurred. The line number 
	//! is relative to the script section where the function was implemented.
	//! 
	//! Observe that the returned sectionName can be null, e.g. if the function in which
	//! the exception occurred was a generated stub function.
	virtual int                GetExceptionLineNumber(int *column = 0, const char **sectionName = 0) = 0;
	//! \brief Returns the function where the exception occurred.
	//! \return The function where the exception occurred.
	virtual asIScriptFunction *GetExceptionFunction() = 0;
	//! \brief Returns the exception string text.
	//! \return A null terminated string describing the exception that occurred.
	virtual const char *       GetExceptionString() = 0;
	//! \brief Determine if the current exception will be caught by the script
	//! \return True if the exception will be caught by the script.
	//!
	//! This method is intended to be used from the \ref SetExceptionCallback "exception callback", 
	//! where the application can potentially make a different decision depending on whether the 
	//! script will catch the exception or not.
	virtual bool               WillExceptionBeCaught() = 0;
	//! \brief Sets an exception callback function. The function will be called if a script exception occurs.
	//! \param[in] callback The callback function/method that should be called upon an exception.
	//! \param[in] obj The object pointer on which the callback is called.
	//! \param[in] callConv The calling convention of the callback function/method.
	//! \return A negative value on error.
	//! \retval asNOT_SUPPORTED Calling convention must not be asCALL_GENERIC, or the routine's calling convention is not supported.
	//! \retval asINVALID_ARG   \a obj must not be null for class methods.
	//! \retval asWRONG_CALLING_CONV \a callConv isn't compatible with the routines' calling convention.
	//!
	//! This callback function will be called by the VM when a script exception is raised, which 
	//! allow the application to examine the callstack and generate a detailed report before the 
	//! callstack is cleaned up.
	//!
	//! The callback function can be either a global function or a class method. For a global function 
	//! the VM will pass two parameters, first the context pointer and then the object pointer specified 
	//! by the application. For a class method, the VM will call the method using the object pointer 
	//! as the owner.
	//!
	//! \code
	//! void Callback(asIScriptContext *ctx, void *obj);
	//! void Object::Callback(asIScriptContext *ctx);
	//! \endcode
	//!
	//! The global function can use either \ref asCALL_CDECL or \ref asCALL_STDCALL, and the class method can use either 
	//! \ref asCALL_THISCALL, \ref asCALL_CDECL_OBJLAST, or \ref asCALL_CDECL_OBJFIRST.
	//! 
	//! \see \ref doc_call_script_4
	virtual int                SetExceptionCallback(asSFuncPtr callback, void *obj, int callConv) = 0;
	//! \brief Removes the registered callback.
	//!
	//! Removes a previously registered callback.
	virtual void               ClearExceptionCallback() = 0;
	//! \}

	// Debugging
	//! \name Debugging
	//! \{

	//! \brief Sets a line callback function. The function will be called for each executed script statement.
	//! \param[in] callback The callback function/method that should be called for each script line executed.
	//! \param[in] obj The object pointer on which the callback is called.
	//! \param[in] callConv The calling convention of the callback function/method.
	//! \return A negative value on error.
	//! \retval asNOT_SUPPORTED Calling convention must not be asCALL_GENERIC, or the routine's calling convention is not supported.
	//! \retval asINVALID_ARG   \a obj must not be null for class methods.
	//! \retval asWRONG_CALLING_CONV \a callConv isn't compatible with the routines' calling convention.
	//!
	//! This function sets a callback function that will be called by the VM each time the SUSPEND 
	//! instruction is encounted. Generally this instruction is placed before each statement. Thus by 
	//! setting this callback function it is possible to monitor the execution, and suspend the execution 
	//! at application defined locations.
	//!
	//! The callback function can be either a global function or a class method. For a global function 
	//! the VM will pass two parameters, first the context pointer and then the object pointer specified 
	//! by the application. For a class method, the VM will call the method using the object pointer 
	//! as the owner.
	//!
	//! \code
	//! void Callback(asIScriptContext *ctx, void *obj);
	//! void Object::Callback(asIScriptContext *ctx);
	//! \endcode
	//!
	//! The global function can use either \ref asCALL_CDECL or \ref asCALL_STDCALL, and the class method can use either 
	//! \ref asCALL_THISCALL, \ref asCALL_CDECL_OBJLAST, or \ref asCALL_CDECL_OBJFIRST.
	//!
	//! \see \ref doc_debug
	virtual int                SetLineCallback(asSFuncPtr callback, void *obj, int callConv) = 0;
	//! \brief Removes the registered callback.
	//!
	//! Removes a previously registered callback.
	virtual void               ClearLineCallback() = 0;
	//! \brief Returns the size of the callstack, i.e. the number of functions that have yet to complete.
	//! \return The number of functions on the call stack, including the current function.
	//!
	//! This methods returns the size of the callstack. It can be used to enumerate the callstack in order 
	//! to generate a detailed report when an exception occurs, or for debugging running scripts.
	virtual asUINT             GetCallstackSize() const = 0;
	//! \brief Returns the function at the specified callstack level.
	//! \param[in] stackLevel The index on the call stack.
	//! \return The function descriptor on the call stack referred to by the index.
	//!
	//! Index 0 refers to the current function, index 1 to the calling function, and so on. 
	//! The highest index is the originating function that the application called.
	//!
	//! The returned value will be null if the stackLevel is invalid, or if the requested 
	//! stack level doesn't have a defined function. The latter scenario happens when 
	//! the engine performs a nested call internally, e.g. to call a constructor for a
	//! script object indirectly created.
	//!
	//! If the application performs a nested call, then the returned value will give the
	//! application registered function that was called by the previous script. 
	//!
	//! \see \ref PushState
	virtual asIScriptFunction *GetFunction(asUINT stackLevel = 0) = 0;
	//! \brief Returns the line number at the specified callstack level.
	//! \param[in] stackLevel The index on the call stack.
	//! \param[out] column The variable will receive the column number.
	//! \param[out] sectionName The variable will receive the name of the script section.
	//! \return The line number for the call stack level referred to by the index.
	//!
	//! This function returns the line number, and optionally the column number
	//! and the name of the script section where the program is current at. 
	//!
	//! The sectionName pointer will point to an internal buffer, so do not deallocate it.
	//! If the function doesn't have any debug info sectionName will be set to null.
	virtual int                GetLineNumber(asUINT stackLevel = 0, int *column = 0, const char **sectionName = 0) = 0;
	//! \brief Returns the number of local variables at the specified callstack level.
	//! \param[in] stackLevel The index on the call stack.
	//! \return The number of variables in the function on the call stack level. Or negative value on error.
	//! \retval asINVALID_ARG The stackLevel is invalid.
	//!
	//! Returns the number of declared variables, including the parameters, in the function on the stack.
	virtual int                GetVarCount(asUINT stackLevel = 0) = 0;
	//! \brief Returns the name of local variable at the specified callstack level.
	//! \param[in] varIndex The index of the variable.
	//! \param[in] stackLevel The index on the call stack.
	//! \return A null terminated string with the name of the variable.
	virtual const char        *GetVarName(asUINT varIndex, asUINT stackLevel = 0) = 0;
	//! \brief Returns the declaration of a local variable at the specified callstack level.
	//! \param[in] varIndex The index of the variable.
	//! \param[in] stackLevel The index on the call stack.
	//! \param[in] includeNamespace Set to true if the namespace should be included in the declaration.
	//! \return A null terminated string with the declaration of the variable.
	virtual const char        *GetVarDeclaration(asUINT varIndex, asUINT stackLevel = 0, bool includeNamespace = false) = 0;
	//! \brief Returns the type id of a local variable at the specified callstack level.
	//! \param[in] varIndex The index of the variable.
	//! \param[in] stackLevel The index on the call stack.
	//! \return The type id of the variable, or a negative value on error.
	//! \retval asINVALID_ARG The index or stack level is invalid.
	virtual int                GetVarTypeId(asUINT varIndex, asUINT stackLevel = 0) = 0;
	//! \brief Returns a pointer to a local variable at the specified callstack level.
	//! \param[in] varIndex The index of the variable.
	//! \param[in] stackLevel The index on the call stack.
	//! \return A pointer to the variable.
	//!
	//! Returns a pointer to the variable, so that its value can be read and written. The 
	//! address is valid until the script function returns.
	//!
	//! Note that object variables may not be initalized at all moments, thus you must verify 
	//! if the address returned points to a null pointer, before you try to dereference it.
	virtual void              *GetAddressOfVar(asUINT varIndex, asUINT stackLevel = 0) = 0;
	//! \brief Informs whether the variable is in scope at the current program position.
	//! \param[in] varIndex The index of the variable.
	//! \param[in] stackLevel The index on the call stack.
	//! \return True if variable is in scope.
	//!
	//! This method can be used to determine if a variable is currently visible from the 
	//! current program position. This is especially useful if multiple variables with the
	//! same name is declared in different scopes.
	virtual bool               IsVarInScope(asUINT varIndex, asUINT stackLevel = 0) = 0;
	//! \brief Returns the type id of the object, if a class method is being executed.
	//! \param[in] stackLevel The index on the call stack.
	//! \return Returns the type id of the object if it is a class method.
	virtual int                GetThisTypeId(asUINT stackLevel = 0) = 0;
	//! \brief Returns a pointer to the object, if a class method is being executed.
	//! \param[in] stackLevel The index on the call stack.
	//! \return Returns a pointer to the object if it is a class method.
	virtual void              *GetThisPointer(asUINT stackLevel = 0) = 0;
	//! \brief Returns the registered function that is currently being called by the context.
	//! \return Returns the registered function that is currently being called, or null if no registered function is being called at the moment.
	virtual asIScriptFunction *GetSystemFunction() = 0;
	//! \}

	// User data
	//! \name User data
	//! \{

	//! \brief Register the memory address of some user data.
	//! \param[in] data A pointer to the user data.
	//! \param[in] type An identifier specifying the user data to set.
	//! \return The previous pointer stored in the context.
	//!
	//! This method allows the application to associate a value, e.g. a pointer, with the context instance.
	//!
	//! The type values 1000 through 1999 are reserved for use by the official add-ons.
	//!
	//! Optionally, a callback function can be \ref asIScriptEngine::SetContextUserDataCleanupCallback "registered" 
	//! to clean up the user data when the context is destroyed. As the callback is registered with the engine, it is
	//! only necessary to do it once, even if more than one context is used.
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	//! \brief Returns the address of the previously registered user data.
	//! \param[in] type An identifier specifying the user data to set.
	//! \return The pointer to the user data.
	virtual void *GetUserData(asPWORD type = 0) const = 0;
	//! \}

protected:
	virtual ~asIScriptContext() {}
};

//! \ingroup api_secondary_interfaces
//! \brief The interface for the generic calling convention
class asIScriptGeneric
{
public:
	// Miscellaneous
	//! \name Miscellaneous
	//! \{

	//! \brief Returns a pointer to the script engine.
	//! \return A pointer to the engine.
	virtual asIScriptEngine   *GetEngine() const = 0;
	//! \brief Returns the function that is being called.
	//! \return The function that is being called.
	virtual asIScriptFunction *GetFunction() const = 0;
	//! \brief Return the auxiliary registered with the function.
	//! \return The auxiliary registered with the function.
	virtual void              *GetAuxiliary() const = 0;
	//! \}

	// Object
	//! \name Object
	//! \{

	//! \brief Returns the object pointer if this is a class method, or null if it not.
	//! \return A pointer to the object, if this is a method.
	virtual void   *GetObject() = 0;
	//! \brief Returns the type id of the object if this is a class method.
	//! \return The type id of the object if this is a method.
	virtual int     GetObjectTypeId() const = 0;
	//! \}

	// Arguments
	//! \name Arguments
	//! \{

	//! \brief Returns the number of arguments.
	//! \return The number of arguments to the function.
	virtual int     GetArgCount() const = 0;
	//! \brief Returns the type id of the argument.
	//! \param[in] arg The argument index.
	//! \param[out] flags A combination of \ref asETypeModifiers.
	//! \return The type id of the argument.
	virtual int     GetArgTypeId(asUINT arg, asDWORD *flags = 0) const = 0;
	//! \brief Returns the value of an 8-bit argument.
	//! \param[in] arg The argument index.
	//! \return The 1 byte argument value.
	virtual asBYTE  GetArgByte(asUINT arg) = 0;
	//! \brief Returns the value of a 16-bit argument.
	//! \param[in] arg The argument index.
	//! \return The 2 byte argument value.
	virtual asWORD  GetArgWord(asUINT arg) = 0;
	//! \brief Returns the value of a 32-bit integer argument.
	//! \param[in] arg The argument index.
	//! \return The 4 byte argument value.
	virtual asDWORD GetArgDWord(asUINT arg) = 0;
	//! \brief Returns the value of a 64-bit integer argument.
	//! \param[in] arg The argument index.
	//! \return The 8 byte argument value.
	virtual asQWORD GetArgQWord(asUINT arg) = 0;
	//! \brief Returns the value of a float argument.
	//! \param[in] arg The argument index.
	//! \return The float argument value.
	virtual float   GetArgFloat(asUINT arg) = 0;
	//! \brief Returns the value of a double argument.
	//! \param[in] arg The argument index.
	//! \return The double argument value.
	virtual double  GetArgDouble(asUINT arg) = 0;
	//! \brief Returns the address held in a reference or handle argument.
	//! \param[in] arg The argument index.
	//! \return The address argument value, which can be a reference or and object handle.
	//!
	//! Don't release the pointer if this is an object or object handle, the asIScriptGeneric object will 
	//! do that for you.
	virtual void   *GetArgAddress(asUINT arg) = 0;
	//! \brief Returns a pointer to the object in a object argument.
	//! \param[in] arg The argument index.
	//! \return A pointer to the object argument, which can be an object value or object handle.
	//!
	//! Don't release the pointer if this is an object handle, the asIScriptGeneric object will 
	//! do that for you.
	virtual void   *GetArgObject(asUINT arg) = 0;
	//! \brief Returns a pointer to the argument value.
	//! \param[in] arg The argument index.
	//! \return A pointer to the argument value.
	virtual void   *GetAddressOfArg(asUINT arg) = 0;
	//! \}

	// Return value
	//! \name Return value
	//! \{

	//! \brief Gets the type id of the return value.
	//! \param[out] flags A combination of \ref asETypeModifiers.
	//! \return The type id of the return value.
	virtual int     GetReturnTypeId(asDWORD *flags = 0) const = 0;
	//! \brief Sets the 8-bit return value.
	//! \param[in] val The return value.
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The return type is not an 8-bit value.
	//! Sets the 1 byte return value.
	virtual int     SetReturnByte(asBYTE val) = 0;
	//! \brief Sets the 16-bit return value.
	//! \param[in] val The return value.
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The return type is not a 16-bit value.
	//! Sets the 2 byte return value.
	virtual int     SetReturnWord(asWORD val) = 0;
	//! \brief Sets the 32-bit integer return value.
	//! \param[in] val The return value.
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The return type is not a 32-bit value.
	//! Sets the 4 byte return value.
	virtual int     SetReturnDWord(asDWORD val) = 0;
	//! \brief Sets the 64-bit integer return value.
	//! \param[in] val The return value.
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The return type is not a 64-bit value.
	//! Sets the 8 byte return value.
	virtual int     SetReturnQWord(asQWORD val) = 0;
	//! \brief Sets the float return value.
	//! \param[in] val The return value.
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The return type is not a 32-bit value.
	//! Sets the float return value.
	virtual int     SetReturnFloat(float val) = 0;
	//! \brief Sets the double return value.
	//! \param[in] val The return value.
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The return type is not a 64-bit value.
	//! Sets the double return value.
	virtual int     SetReturnDouble(double val) = 0;
	//! \brief Sets the address return value when the return is a reference or handle.
	//! \param[in] addr The return value, which is an address.
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The return type is not a reference or handle.
	//!
	//! Sets the address return value. If an object handle the application must first 
	//! increment the reference counter, unless it won't keep a reference itself.
	virtual int     SetReturnAddress(void *addr) = 0;
	//! \brief Sets the object return value.
	//! \param[in] obj A pointer to the object return value.
	//! \return A negative value on error.
	//! \retval asINVALID_TYPE The return type is not an object value or handle.
	//!
	//! If the function returns an object, the library will automatically do what is 
	//! necessary based on how the object was declared, i.e. if the function was 
	//! registered to return a handle then the library will call the addref behaviour. 
	//! If it was registered to return an object by value, then the library will make 
	//! a copy of the object.
	virtual int     SetReturnObject(void *obj) = 0;
	//! \brief Gets the address to the memory where the return value should be placed.
	//! \return A pointer to the memory where the return values is to be placed.
	//!
	//! The memory is not initialized, so if you're going to return a complex type by value
	//! you shouldn't use the assignment operator to initialize it. Instead use the placement new
	//! operator to call the type's copy constructor to perform the initialization.
	//!
	//! \code
	//! new(gen->GetAddressOfReturnLocation()) std::string(myRetValue);
	//! \endcode
	//!
	//! The placement new operator works for primitive types too, so this method is ideal
	//! for writing automatically generated functions that works the same way for all types.
	virtual void   *GetAddressOfReturnLocation() = 0;
	//! \}

protected:
	virtual ~asIScriptGeneric() {}
};

//! \ingroup api_secondary_interfaces
//! \brief The interface for an instance of a script object
class asIScriptObject
{
public:
	// Memory management
	//! \name Memory management
	//! \{

	//! \brief Increase reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when storing an additional reference to the object.
	virtual int                    AddRef() const = 0;
	//! \brief Decrease reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when you will no longer use the references that you own.
	virtual int                    Release() const = 0;
	//! \brief Returns the weak ref flag for the object.
	//! \return The weak ref flag for the object.
	//!
	//! This is a short-cut for the \ref asIScriptEngine::GetWeakRefFlagOfScriptObject method.
	virtual asILockableSharedBool *GetWeakRefFlag() const = 0;
	//! \}

	// Type info
	//! \name Type info
	//! \{

	//! \brief Returns the type id of the object.
	//! \return The type id of the script object.
	virtual int            GetTypeId() const = 0;
	//! \brief Returns the object type interface for the object.
	//! \return The object type interface of the script object.
	//!
	//! This does not increase the reference count of the returned object type.
	virtual asITypeInfo   *GetObjectType() const = 0;
	//! \}

	// Class properties
	//! \name Properties
	//! \{

	//! \brief Returns the number of properties that the object contains.
	//! \return The number of member properties of the script object.
	virtual asUINT      GetPropertyCount() const = 0;
	//! \brief Returns the type id of the property referenced by prop.
	//! \param[in] prop The property index.
	//! \return The type id of the member property, or a negative value on error.
	//! \retval asINVALID_ARG \a prop is too large
	virtual int         GetPropertyTypeId(asUINT prop) const = 0;
	//! \brief Returns the name of the property referenced by prop.
	//! \param[in] prop The property index.
	//! \return A null terminated string with the property name.
	virtual const char *GetPropertyName(asUINT prop) const = 0;
	//! \brief Returns a pointer to the property referenced by prop.
	//! \param[in] prop The property index.
	//! \return A pointer to the property value.
	//!
	//! The method returns a pointer to the memory location for the property. Use the type 
	//! id for the property to determine the content of the pointer, and how to handle it.
	virtual void       *GetAddressOfProperty(asUINT prop) = 0;
	//! \}

	// Miscellaneous
	//! \name Miscellaneous
	//! \{

	//! \brief Return the script engine.
	//! \return The script engine.
	virtual asIScriptEngine *GetEngine() const = 0;
	//! \brief Copies the content from another object of the same type.
	//! \param[in] other A pointer to the source object.
	//! \return A negative value on error.
	//! \retval asINVALID_ARG  The argument is null.
	//! \retval asINVALID_TYPE The other object is of different type.
	//!
	//! This method copies the contents of the other object to this one.
	virtual int              CopyFrom(const asIScriptObject *other) = 0;
	//! \}

	// User data
	//! \name User data
	//! \{

	//! \brief Sets user data on the script object instance.
	//! \param[in] data A pointer to the user data.
	//! \param[in] type An identifier used to identify which user data to set.
	//! \return The previous pointer stored in the object type.
	//!
	//! This method allows the application to associate a value, e.g. a pointer, with the script object instance.
	//! Multiple different values can be defined where the type argument identifies which is referred to.
	//!
	//! The user data types identifiers between 1000 and 1999 are reserved for use by official add-ons.
	//!
	//! Optionally, a callback function can be \ref asIScriptEngine::SetScriptObjectUserDataCleanupCallback "registered" 
	//! to clean up the user data when the script object is destroyed.
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	//! \brief Gets user data from the script object instance.
	//! \param[in] type An identifier used to identify which user data to get.
	//! \return The pointer to the user data.
	virtual void *GetUserData(asPWORD type = 0) const = 0;
	//! \}

protected:
	virtual ~asIScriptObject() {}
};


//! \ingroup api_secondary_interfaces
//! \brief The interface for describing types
//! This interface is used to describe the types in the script engine. 
//!
//! It can represent object types, funcdefs, typedefs, and enums. To 
//! determine which family the type belong to verify the flags.
class asITypeInfo
{
public:
	// Miscellaneous
	//! \name Miscellaneous
	//! \{

	//! \brief Returns a pointer to the script engine.
	//! \return A pointer to the engine.
	virtual asIScriptEngine *GetEngine() const = 0;
	//! \brief Returns the config group in which the type was registered.
	//! \return The name of the config group, or null if not set.
	virtual const char      *GetConfigGroup() const = 0;
	//! \brief Returns the access mask for this type.
	//! \return The access mask for this type.
	virtual asDWORD          GetAccessMask() const = 0;
	//! \brief Returns the module where the type is declared.
	//! \return The module where the type is declared.
	//!
	//! The returned value can be null if the module doesn't exist anymore.
	virtual asIScriptModule *GetModule() const = 0;
	//! \}

	// Memory management
	//! \name Memory management
	//! \{

	//! \brief Increases the reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when storing an additional reference to the object.
	virtual int AddRef() const = 0;
	//! \brief Decrease reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when you will no longer use the references that you own.
	virtual int Release() const = 0;
	//! \}

	// Type info
	//! \name Type info
	//! \{

	//! \brief Returns a temporary pointer to the name of the datatype.
	//! \return A null terminated string with the name of the object type.
	virtual const char      *GetName() const = 0;
	//! \brief Return the namespace of the type.
	//! \returns The namespace of the type, or null if not defined.
	//!
	//! If the namespace is not defined it means that this is a child type.
	virtual	const char      *GetNamespace() const = 0;
	//! \brief Returns the object type that this type derives from.
	//! \return A pointer to the object type that this type derives from.
	//!
	//! This method will only return a pointer in case of script classes that 
	//! derives from another script class.
	//!
	//! This does not increase the reference count of the returned object type.
	virtual asITypeInfo     *GetBaseType() const = 0;
	//! \brief Returns true if the type inherits directly or indirectly from the informed type.
	//! \param[in] objType The potential parent type.
	//! \return True if the type inherits directly or indirectly from the informed type.
	virtual bool             DerivesFrom(const asITypeInfo *objType) const = 0;
	//! \brief Returns the type flags.
	//! \return A bit mask with the flags from \ref asEObjTypeFlags.
	//!
	//! Object types are identified by having the flags \ref asOBJ_REF or \ref asOBJ_VALUE set.
	//!
	//! Enums are identified by having the flag \ref asOBJ_ENUM set.
	//!
	//! Funcdefs are identified by having the flag \ref asOBJ_FUNCDEF set.
	//!
	//! Typedefs are identified by having the flag \ref asOBJ_TYPEDEF set.
	//!
	//! Script classes are identified by having the \ref asOBJ_SCRIPT_OBJECT flag set. 
	//! Interfaces are identified as a script class with a size of zero.
	//!
	//! \see \ref GetSize
	virtual asDWORD          GetFlags() const = 0;
	//! \brief Returns the size of the object type.
	//! \return The number of bytes necessary to store instances of this type.
	//!
	//! Application registered reference types doesn't store this information, 
	//! as the script engine doesn't allocate memory for these itself.
	virtual asUINT           GetSize() const = 0;
	//! \brief Returns the type id for the object type.
	//! \return The type id for the object type.
	virtual int              GetTypeId() const = 0;
	//! \brief Returns the type id of the template sub type.
	//! \param[in] subTypeIndex The zero based index of the template sub type.
	//! \return The type id of the template sub type, or a negative value on error.
	//! \retval asERROR The type is not a template type.
	virtual int              GetSubTypeId(asUINT subTypeIndex = 0) const = 0;
	//! \brief Returns the template subtype, in case it is an object type.
	//! \param[in] subTypeIndex The zero based index of the template sub type.
	//! \return The type info of the template sub type, or null if the template subtype is a primitive.
	virtual asITypeInfo     *GetSubType(asUINT subTypeIndex = 0) const = 0;
	//! \brief Returns the number of template sub types.
	//! \return The number of template sub types.
	virtual asUINT           GetSubTypeCount() const = 0;
	//! \}

	// Interfaces
	//! \name Interfaces
	//! \{

	//! \brief Returns the number of interfaces implemented.
	//! \return The number of interfaces implemented by this type.
	virtual asUINT           GetInterfaceCount() const = 0;
	//! \brief Returns a temporary pointer to the specified interface or null if none are found.
	//! \param[in] index The interface index.
	//! \return A pointer to the interface type.
	virtual asITypeInfo     *GetInterface(asUINT index) const = 0;
	//! \brief Returns true if the type implements the informed interface type.
	//! \param[in] objType The interface type.
	//! \return True if the type implements the informed interface type.
	virtual bool             Implements(const asITypeInfo *objType) const = 0;
	//! \}

	// Factories
	//! \name Factories
	//! \{

	//! \brief Returns the number of factory functions for the object type.
	//! \return The number of factory functions for this object.
	virtual asUINT             GetFactoryCount() const = 0;
	//! \brief Returns the factory function by the index
	//! \param[in] index The index of the factory function.
	//! \return The factory function or null if the index is invalid.
	virtual asIScriptFunction *GetFactoryByIndex(asUINT index) const = 0;
	//! \brief Returns the factory function by the declaration
	//! \param[in] decl The declaration of the function
	//! \return The matching factory function or null if there are no matches
	virtual asIScriptFunction *GetFactoryByDecl(const char *decl) const = 0;
	//! \}

	// Methods
	//! \name Methods
	//! \{

	//! \brief Returns the number of methods for the object type.
	//! \return The number of methods for this object.
	virtual asUINT             GetMethodCount() const = 0;
	//! \brief Returns the method by index.
	//! \param[in] index The index of the method.
	//! \param[in] getVirtual Set to true if the virtual method or the real method should be retrieved.
	//! \return The method or null on error.
	//!
	//! This method should be used to retrieve the script method for the object 
	//! that you wish to execute. The method is then sent to the context's \ref asIScriptContext::Prepare "Prepare" method.
	//!
	//! By default this returns the virtual method for script classes. This will allow you to 
	//! call the virtual method on classes, and rely on the polymorphism to call the correct 
	//! implementation. If you wish to inspect the real method, then you should set the second
	//! parameter to false to retrieve the real method.
	virtual asIScriptFunction *GetMethodByIndex(asUINT index, bool getVirtual = true) const = 0;
	//! \brief Returns the method by name.
	//! \param[in] name The name of the method.
	//! \param[in] getVirtual Set to true if the virtual method or the real method should be retrieved.
	//! \return Tthe method or null in case of error
	//!
	//! This method should be used to retrieve the script method for the object 
	//! that you wish to execute. The method is then sent to the context's \ref asIScriptContext::Prepare "Prepare" method.
	virtual asIScriptFunction *GetMethodByName(const char *name, bool getVirtual = true) const = 0;
	//! \brief Returns the method by declaration.
	//! \param[in] decl The method signature.
	//! \param[in] getVirtual Set to true if the virtual method or the real method should be retrieved.
	//! \return The method or null on error.
	//!
	//! This method should be used to retrieve the script method for the object 
	//! that you wish to execute. The method is then sent to the context's \ref asIScriptContext::Prepare "Prepare" method.
	//!
	//! The method will find the script method with the exact same declaration.
	virtual asIScriptFunction *GetMethodByDecl(const char *decl, bool getVirtual = true) const = 0;
	//! \}

	// Properties
	//! \name Properties
	//! \{

	//! \brief Returns the number of properties that the object contains.
	//! \return The number of member properties of the script object.
	virtual asUINT      GetPropertyCount() const = 0;
	//! \brief Returns the attributes of the property.
	//! \param[in] index The index of the property
	//! \param[out] name The name of the property
	//! \param[out] typeId The type of the property
	//! \param[out] isPrivate Whether the property is private or not
	//! \param[out] isProtected Whether the property is protected or not
	//! \param[out] offset The offset into the object where the property is stored
	//! \param[out] isReference True if the property is not stored inline
	//! \param[out] accessMask The access mask of the property
	//! \param[out] compositeOffset The offset to composite type if used
	//! \param[out] isCompositeIndirect Set to false if the composite type is inline
	//! \return A negative value on error
	//! \retval asINVALID_ARG The \a index is out of bounds
	virtual int         GetProperty(asUINT index, const char **name, int *typeId = 0, bool *isPrivate = 0, bool *isProtected = 0, int *offset = 0, bool *isReference = 0, asDWORD *accessMask = 0, int *compositeOffset = 0, bool *isCompositeIndirect = 0) const = 0;
	//! \brief Returns the declaration of the property
	//! \param[in] index The index of the property
	//! \param[in] includeNamespace Set to true if the namespace should be included in the declaration.
	//! \return The declaration of the property, or null on error.
	virtual const char *GetPropertyDeclaration(asUINT index, bool includeNamespace = false) const = 0;
	//! \}

	// Behaviours
	//! \name Behaviours
	//! \{

	//! \brief Returns the number of behaviours.
	//! \return The number of behaviours for this type.
	virtual asUINT             GetBehaviourCount() const = 0;
	//! \brief Returns the function and type of the behaviour.
	//! \param[in] index The index of the behaviour.
	//! \param[out] outBehaviour Receives the type of the behaviour.
	//! \return The function of the behaviour, or null on error.
	virtual asIScriptFunction *GetBehaviourByIndex(asUINT index, asEBehaviours *outBehaviour) const = 0;
	//! \}

	// Child types
	//! \name Child types
	//! \{

	//! \brief Returns the number of child funcdefs declared in the class.
	//! \return The number of child funcdefs declared in the class.
	virtual asUINT       GetChildFuncdefCount() const = 0;
	//! \brief Returns a child funcdef by index.
	//! \return The child funcdef matching the index.
	virtual asITypeInfo *GetChildFuncdef(asUINT index) const = 0;
	//! \brief Returns the parent type if this is a child type.
	//! \return The parent type if this is a child type.
	virtual asITypeInfo *GetParentType() const = 0;
	//! \}

	// Enums
	//! \name Enums
	//! \{

	//! \brief Returns the number of values defined for the enum type.
	//! \return The number of enum values.
	virtual asUINT      GetEnumValueCount() const = 0;
	//! \brief Returns the name and value of the enum value for the enum type.
	//! \param[in] index The index of the enum value.
	//! \param[out] outValue Receives the value of the enum value.
	//! \return The name of the enum value.
	virtual const char *GetEnumValueByIndex(asUINT index, int *outValue) const = 0;
	//! \}

	// Typedef
	//! \name Typedef
	//! \{

	//! \brief Returns the type id that the typedef represents.
	//! \return The type id that the typedef represents.
	virtual int GetTypedefTypeId() const = 0;
	//! \}

	// Funcdef
	//! \name Funcdef
	//! \{

	//! \brief Returns the function description for the funcdef type.
	//! \return A pointer to the function description interface, or null if not a funcdef type.
	//!
	//! This does not increment the reference count of the returned function interface.
	virtual asIScriptFunction *GetFuncdefSignature() const = 0;
	//! \}

	// User data
	//! \name User data
	//! \{

	//! \brief Register the memory address of some user data.
	//! \param[in] data A pointer to the user data.
	//! \param[in] type An identifier used to identify which user data to set.
	//! \return The previous pointer stored in the object type.
	//!
	//! This method allows the application to associate a value, e.g. a pointer, with the object type instance.
	//! Multiple different values can be defined where the type argument identifies which is referred to.
	//!
	//! The user data types identifiers between 1000 and 1999 are reserved for use by official add-ons.
	//!
	//! Optionally, a callback function can be \ref asIScriptEngine::SetTypeInfoUserDataCleanupCallback "registered" 
	//! to clean up the user data when the object type is destroyed.
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	//! \brief Returns the address of the previously registered user data.
	//! \param[in] type An identifier used to identify which user data to get.
	//! \return The pointer to the user data.
	virtual void *GetUserData(asPWORD type = 0) const = 0;
	//! \}

protected:
	virtual ~asITypeInfo() {}
};

//! \ingroup api_secondary_interfaces
//! \brief The interface for a script function description
class asIScriptFunction
{
public:
	//! \name Miscellaneous
	//! \{

	//! \brief Returns a pointer to the script engine.
	//! \return A pointer to the engine.
	virtual asIScriptEngine *GetEngine() const = 0;

	// Memory management
	//! \brief Increases the reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when storing an additional reference to the object.
	virtual int              AddRef() const = 0;
	//! \brief Decrease reference counter.
	//! \return The number of references to this object.
	//!
	//! Call this method when you will no longer use the references that you own.
	virtual int              Release() const = 0;

	// Miscellaneous
	//! \brief Returns the id of the function
	//! \return The id of the function
	virtual int              GetId() const = 0;
	//! \brief Returns the type of the function
	//! \return The type of the function
	virtual asEFuncType      GetFuncType() const = 0;
	//! \brief Returns the name of the module where the function was implemented
	//! \return A null terminated string with the module name.
	virtual const char      *GetModuleName() const = 0;
	//! \brief Returns the module where the function is declared.
	//! \return The module where the function is declared.
	//!
	//! The returned value can be null if the module doesn't exist anymore.
	virtual asIScriptModule *GetModule() const = 0;
	//! \brief Returns the name of the script section where the function was implemented.
	//! \return A null terminated string with the script section name where the function was implemented.
	//!
	//! The returned pointer is null when the function doesn't originate from a script file, i.e.
	//! a registered function or an auto-generated script function. It can also be null if the information
	//! has been removed, e.g. when saving bytecode without debug info.
	virtual const char      *GetScriptSectionName() const = 0;
	//! \brief Returns the name of the config group in which the function was registered.
	//! \return The name of the config group, or null if not in any group.
	virtual const char      *GetConfigGroup() const = 0;
	//! \brief Returns the access mast of the function.
	//! \return The access mask of the function.
	virtual asDWORD          GetAccessMask() const = 0;
	//! \brief Returns the auxiliary object registered with the function.
	//! \return The auxiliary object registered with the function.
	virtual void            *GetAuxiliary() const = 0;
	//! \}

	//! \name Function signature
	//! \{

	// Function signature
	//! \brief Returns the object type for class or interface method
	//! \return A pointer to the object type interface if this is a method.
	//!
	//! This does not increase the reference count of the returned object type.
	virtual asITypeInfo     *GetObjectType() const = 0;
	//! \brief Returns the name of the object for class or interface methods
	//! \return A null terminated string with the name of the object type if this a method.
	virtual const char      *GetObjectName() const = 0;
	//! \brief Returns the name of the function or method
	//! \return A null terminated string with the name of the function.
	virtual const char      *GetName() const = 0;
	//! \brief Returns the namespace of the function.
	//! \return The namespace of the function, or null if not defined.
	virtual const char      *GetNamespace() const = 0;
	//! \brief Returns the function declaration
	//! \param[in] includeObjectName Indicate whether the object name should be prepended to the function name
	//! \param[in] includeNamespace Indicates whether the namespace should be prepended to the function name and types
	//! \param[in] includeParamNames Indicates whether parameter names should be added to the declaration
	//! \return A null terminated string with the function declaration. 
	//!
	//! The parameter names are not stored for \ref asFUNC_VIRTUAL "virtual methods". If you want to know the 
	//! name of parameters to class methods, be sure to get the actual implementation rather than the virtual method.
	//!
	//! The namespace will always be included for types that are declared in a different namespace than the function itself.
	virtual const char      *GetDeclaration(bool includeObjectName = true, bool includeNamespace = false, bool includeParamNames = false) const = 0;
	//! \brief Returns true if the class method is read-only
	//! \return True if the class method is read-only
	virtual bool             IsReadOnly() const = 0;
	//! \brief Returns true if the class method is private
	//! \return True if the class method is private
	virtual bool             IsPrivate() const = 0;
	//! \brief Returns true if the class method is protected
	//! \return True if the class method is protected
	virtual bool             IsProtected() const = 0;
	//! \brief Returns true if the method is final.
	//! \return True if the method is final.
	virtual bool             IsFinal() const = 0;
	//! \brief Returns true if the method is meant to override a method in the base class.
	//! \return True if the method is meant to override a method in the base class.
	virtual bool             IsOverride() const = 0;
	//! \brief Returns true if the function is shared.
	//! \return True if the function is shared.
	virtual bool             IsShared() const = 0;
	//! \brief Returns true if the function is declared as 'explicit'.
	//! \return True if the function is explicit.
	virtual bool             IsExplicit() const = 0;
	//! \brief Returns true if the function is declared as 'property'.
	//! \return True if the function is a property accessor.
	virtual bool             IsProperty() const = 0;
	//! \brief Returns the number of parameters for this function.
	//! \return The number of parameters.
	virtual asUINT           GetParamCount() const = 0;
	//! \brief Returns the type id of the specified parameter.
	//! \param[in] index The zero based parameter index.
	//! \param[out] typeId The typeId of the parameter.
	//! \param[out] flags A combination of \ref asETypeModifiers.
	//! \param[out] name The name of the parameter (or null if not defined).
	//! \param[out] defaultArg The default argument expression (or null if not defined).
	//! \return A negative value on error.
	//! \retval asINVALID_ARG The index is out of bounds.
	//!
	//! The parameter names are not stored for \ref asFUNC_VIRTUAL "virtual methods". If you want to know the 
	//! name of parameters to class methods, be sure to get the actual implementation rather than the virtual method.
	virtual int              GetParam(asUINT index, int *typeId, asDWORD *flags = 0, const char **name = 0, const char **defaultArg = 0) const = 0;
	//! \brief Returns the type id of the return type.
	//! \param[out] flags A combination of \ref asETypeModifiers.
	//! \return The type id of the return type.
	virtual int              GetReturnTypeId(asDWORD *flags = 0) const = 0;
	//! \}

	//! \name Type id for function pointers
	//! \{

	// Type id for function pointers
	//! \brief Returns the type id representing a function pointer for this function
	//! \return The type id that represents a function pointer for this function
	virtual int              GetTypeId() const = 0;
	//! \brief Checks if the given type id can represent this function.
	//! \return Returns true if the type id can represent this function.
	virtual bool             IsCompatibleWithTypeId(int typeId) const = 0;
	//! \}

	//! \name Delegates
	//! \{

	// Delegates
	//! \brief Returns the object for the delegate
	//! \return A pointer to the delegated object
	virtual void              *GetDelegateObject() const = 0;
	//! \brief Returns the type of the delegated object
	//! \return A pointer to the object type of the delegated object.
	virtual asITypeInfo       *GetDelegateObjectType() const = 0;
	//! \brief Returns the function for the delegate
	//! \return A pointer to the delegated function
	virtual asIScriptFunction *GetDelegateFunction() const = 0;
	//! \}

	//! \name Debug information
	//! \{

	// Debug information
	//! \brief Returns the number of local variables in the function
	//! \return The number of local variables in the function
	virtual asUINT           GetVarCount() const = 0;
	//! \brief Returns information about a local variable
	//! \param[in] index The zero based index of the local variable
	//! \param[out] name Receives the name of the variable
	//! \param[out] typeId Receives the typeId of the variable
	//! \return A negative value on error
	//! \retval asINVALID_ARG The \a index is out of range
	//! \retval asNOT_SUPPORTED The function is not a script function
	virtual int              GetVar(asUINT index, const char **name, int *typeId = 0) const = 0;
	//! \brief Returns the declaration of a local variable
	//! \param[in] index The zero based index of the local variable
	//! \param[in] includeNamespace Set to true if the namespace should be included in the declaration.
	//! \return The declaration string, or null on error
	virtual const char      *GetVarDecl(asUINT index, bool includeNamespace = false) const = 0;
	//! \brief Returns the next line number with code
	//! \param[in] line A line number
	//! \return The number of the next line with code, or a negative value if the line is outside the function.
	virtual int              FindNextLineWithCode(int line) const = 0;
	//! \}

	//! \name JIT compilation
	//! \{

	// For JIT compilation
	//! \brief Returns the byte code buffer and length.
	//! \param[out] length The length of the byte code buffer in DWORDs
	//! \return A pointer to the byte code buffer, or 0 if this is not a script function.
	//!
	//! This function is used by the \ref asIJITCompiler to obtain the byte
	//! code buffer for building the native machine code representation.
	virtual asDWORD         *GetByteCode(asUINT *length = 0) = 0;
	//! \}

	// User data
	//! \name User data
	//! \{

	//! \brief Register the memory address of some user data.
	//! \param[in] userData A pointer to the user data.
	//! \param[in] type An identifier specifying the user data to set.
	//! \return The previous pointer stored in the context.
	//!
	//! This method allows the application to associate a value, e.g. a pointer, with the context instance.
	//!
	//! The type values 1000 through 1999 are reserved for use by the official add-ons.
	//!
	//! Optionally, a callback function can be \ref asIScriptEngine::SetFunctionUserDataCleanupCallback "registered" 
	//! to clean up the user data when the function is destroyed. As the callback is registered with the engine, it is
	//! only necessary to do it once.
	virtual void            *SetUserData(void *userData, asPWORD type = 0) = 0;
	//! \brief Returns the address of the previously registered user data.
	//! \param[in] type An identifier specifying the user data to set.
	//! \return The pointer to the user data.
	virtual void            *GetUserData(asPWORD type = 0) const = 0;
	//! \}

protected:
	virtual ~asIScriptFunction() {};
};

//! \ingroup api_auxiliary_interfaces
//! \brief A binary stream interface.
//!
//! This interface is used when storing compiled bytecode to disk or memory, and then loading it into the engine again.
//!
//! \see \ref asIScriptModule::SaveByteCode, \ref asIScriptModule::LoadByteCode
class asIBinaryStream
{
public:
	//! \brief Read size bytes from the stream into the memory pointed to by ptr.
	//! \param[out] ptr A pointer to the buffer that will receive the data.
	//! \param[in] size The number of bytes to read.
	//! \return A negative value on error.
	//!
	//! Read \a size bytes from the data stream into the memory pointed to by \a ptr.
	virtual int Read(void *ptr, asUINT size) = 0;
	//! \brief Write size bytes to the stream from the memory pointed to by ptr.
	//! \param[in] ptr A pointer to the buffer that the data should written from.
	//! \param[in] size The number of bytes to write.
	//! \return A negative value on error.
	//!
	//! Write \a size bytes to the data stream from the memory pointed to by \a ptr.
	virtual int Write(const void *ptr, asUINT size) = 0;

public:
	virtual ~asIBinaryStream() {}
};

//! \ingroup api_auxiliary_interfaces
//! \brief A lockable shared boolean.
//!
//! This interface represents a lockable shared boolean.
class asILockableSharedBool
{
public:
	// Memory management
	//! \brief Adds a reference to the shared boolean
	//! \return The new reference count
	virtual int AddRef() const = 0;
	//! \brief Releases a reference to the shared boolean
	//! \return The new reference count
	virtual int Release() const = 0;

	// Value
	//! \brief Get the value of the shared boolean
	//! \return The value of the shared boolean
	virtual bool Get() const = 0;
	//! \brief Sets the value of the shared boolean
	//! \param[in] val The new value
	virtual void Set(bool val) = 0;

	// Thread management
	//! \brief Locks the shared boolean
	//!
	//! If the boolean is already locked, then this method 
	//! will wait until it is unlocked before returning.
	//!
	//! Unlock the shared boolean with a call to \ref Unlock
	virtual void Lock() const = 0;
	//! \brief Unlocks the shared boolean
	//!
	//! Unlock the shared boolean after a call to \ref Lock
	virtual void Unlock() const = 0;

protected:
	virtual ~asILockableSharedBool() {}
};

//-----------------------------------------------------------------
// Function pointers

// Template function to capture all global functions,
// except the ones using the generic calling convention
template <class T>
inline asSFuncPtr asFunctionPtr(T func)
{
	// Mark this as a global function
	asSFuncPtr p(2);

#ifdef AS_64BIT_PTR
	// The size_t cast is to avoid a compiler warning with asFUNCTION(0)
	// on 64bit, as 0 is interpreted as a 32bit int value
	p.ptr.f.func = reinterpret_cast<asFUNCTION_t>(size_t(func));
#else
	// MSVC6 doesn't like the size_t cast above so I
	// solved this with a separate code for 32bit.
	p.ptr.f.func = reinterpret_cast<asFUNCTION_t>(func);
#endif

	return p;
}

// Specialization for functions using the generic calling convention
template<>
inline asSFuncPtr asFunctionPtr<asGENFUNC_t>(asGENFUNC_t func)
{
	// Mark this as a generic function
	asSFuncPtr p(1);
	p.ptr.f.func = reinterpret_cast<asFUNCTION_t>(func);
	return p;
}

#ifndef AS_NO_CLASS_METHODS

// Method pointers

// Declare a dummy class so that we can determine the size of a simple method pointer
class asCSimpleDummy {};
typedef void (asCSimpleDummy::*asSIMPLEMETHOD_t)();
const int SINGLE_PTR_SIZE = sizeof(asSIMPLEMETHOD_t);

// Define template
template <int N>
struct asSMethodPtr
{
	template<class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// This version of the function should never be executed, nor compiled,
		// as it would mean that the size of the method pointer cannot be determined.

		int ERROR_UnsupportedMethodPtr[N-100];

		asSFuncPtr p(0);
		return p;
	}
};

// Template specialization
template <>
struct asSMethodPtr<SINGLE_PTR_SIZE>
{
	template<class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE);
		return p;
	}
};

#if defined(_MSC_VER) && !defined(__MWERKS__)

// MSVC and Intel uses different sizes for different class method pointers
template <>
struct asSMethodPtr<SINGLE_PTR_SIZE+1*sizeof(int)>
{
	template <class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE+sizeof(int));
		return p;
	}
};

template <>
struct asSMethodPtr<SINGLE_PTR_SIZE+2*sizeof(int)>
{
	template <class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// On 32bit platforms with is where a class with virtual inheritance falls.
		// On 64bit platforms we can also fall here if 8byte data alignments is used.

		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE+2*sizeof(int));

		// Microsoft has a terrible optimization on class methods with virtual inheritance.
		// They are hardcoding an important offset, which is not coming in the method pointer.

#if defined(_MSC_VER) && !defined(AS_64BIT_PTR)
			// Method pointers for virtual inheritance is not supported,
			// as it requires the location of the vbase table, which is
			// only available to the C++ compiler, but not in the method
			// pointer.

			// You can get around this by forward declaring the class and
			// storing the sizeof its method pointer in a constant. Example:

			// class ClassWithVirtualInheritance;
			// const int ClassWithVirtualInheritance_workaround = sizeof(void ClassWithVirtualInheritance::*());

			// This will force the compiler to use the unknown type
			// for the class, which falls under the next case


			// Copy the virtual table index to the 4th dword so that AngelScript
			// can properly detect and deny the use of methods with virtual inheritance.
			*(reinterpret_cast<asDWORD*>(&p)+3) = *(reinterpret_cast<asDWORD*>(&p)+2);
#endif

		return p;
	}
};

template <>
struct asSMethodPtr<SINGLE_PTR_SIZE+3*sizeof(int)>
{
	template <class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE+3*sizeof(int));
		return p;
	}
};

template <>
struct asSMethodPtr<SINGLE_PTR_SIZE+4*sizeof(int)>
{
	template <class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// On 64bit platforms with 8byte data alignment
		// the unknown class method pointers will come here.

		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE+4*sizeof(int));
		return p;
	}
};

#endif

#endif // AS_NO_CLASS_METHODS

//----------------------------------------------------------------
// JIT compiler

//! \brief A struct with registers from the VM sent to a JIT compiled function
//!
//! The JIT compiled function will receive a pointer to this structure when called.
//! It is the responsibility of the JIT compiled function to make sure these
//! values are updated correctly before control is returned to the VM.
//!
//! \see \ref doc_adv_jit
struct asSVMRegisters
{
	//! \brief Points to the current bytecode instruction
	asDWORD          *programPointer;     // points to current bytecode instruction
	//! \brief Function stack frame. This doesn't change during the function execution.
	asDWORD          *stackFramePointer;  // function stack frame
	//! \brief Top of the stack (grows downward)
	asDWORD          *stackPointer;       // top of stack (grows downward)
	//! \brief Temporary register for primitives and unmanaged references
	asQWORD           valueRegister;      // temp register for primitives
	//! \brief Temporary register for managed object references/handles
	void             *objectRegister;     // temp register for objects and handles
	//! \brief Type of the object held in the object register
	asITypeInfo      *objectType;         // type of object held in object register
	//! \brief Set to true if the SUSPEND instruction should be processed. Do not update this value.
	bool              doProcessSuspend;   // whether or not the JIT should break out when it encounters a suspend instruction
	//! \brief The context to which the registers belong.
	asIScriptContext *ctx;                // the active context
};

//! \brief The function signature of a JIT compiled function
//! \param [in] registers  A pointer to the virtual machine's registers.
//! \param [in] jitArg     The value defined by the JIT compiler for the current entry point in the JIT function.
//!
//! A JIT function receives a pointer to the virtual machine's registers when called and 
//! an argument telling it where in the script function to continue the execution. The JIT
//! function must make sure to update the VM's registers according to the actions performed
//! before returning control to the VM.
//!
//! \see \ref doc_adv_jit
typedef void (*asJITFunction)(asSVMRegisters *registers, asPWORD jitArg);

//! \ingroup api_auxiliary_interfaces
//! \brief The interface that AS use to interact with the JIT compiler
//!
//! This is the minimal interface that the JIT compiler must implement
//! so that AngelScript can request the compilation of the script functions.
//!
//! \see \ref doc_adv_jit
class asIJITCompiler
{
public:
	//! \brief Called by AngelScript to begin the compilation
	//!
	//! \param [in] function A pointer to the script function
	//! \param [out] output The JIT compiled function
	//! \return A negative value on error.
	//!
	//! AngelScript will call this function to request the compilation of
	//! a script function. The JIT compiler should produce the native machine
	//! code representation of the function and update the JitEntry instructions
	//! in the byte code to allow the VM to transfer the control to the JIT compiled
	//! function.
	virtual int  CompileFunction(asIScriptFunction *function, asJITFunction *output) = 0;
	//! \brief Called by AngelScript when the JIT function is released
	//! \param [in] func Pointer to the JIT function
	virtual void ReleaseJITFunction(asJITFunction func) = 0;
public:
	virtual ~asIJITCompiler() {}
};

// Byte code instructions
//! \brief The bytecode instructions used by the VM
//! \see \ref doc_adv_jit_1
enum asEBCInstr
{
	//! \brief Removes a pointer from the stack
	asBC_PopPtr			= 0,
	//! \brief Pushes a pointer from a global variable onto the stack
	asBC_PshGPtr		= 1,
	//! \brief Push the 32bit value in the argument onto the stack
	asBC_PshC4			= 2,
	//! \brief Push the 32bit value from a variable onto the stack
	asBC_PshV4			= 3,
	//! \brief Push the address of the stack frame onto the stack
	asBC_PSF			= 4,
	//! \brief Swap the top two pointers on the stack
	asBC_SwapPtr		= 5,
	//! \brief Perform a boolean not on the value in a variable
	asBC_NOT			= 6,
	//! \brief Push the 32bit value from a global variable onto the stack
	asBC_PshG4			= 7,
	//! \brief Perform the actions of \ref asBC_LDG followed by \ref asBC_RDR4
	asBC_LdGRdR4		= 8,
	//! \brief Jump to a script function, indexed by the argument
	asBC_CALL			= 9,
	//! \brief Return to the instruction after the last executed call
	asBC_RET			= 10,
	//! \brief Unconditional jump to a relative position in this function
	asBC_JMP			= 11,
	//! \brief If the value register is 0 jump to a relative position in this function
	asBC_JZ				= 12,
	//! \brief If the value register is not 0 jump to a relative position in this function
	asBC_JNZ			= 13,
	//! \brief If the value register is less than 0 jump to a relative position in this function
	asBC_JS				= 14,
	//! \brief If the value register is greater than or equal to 0 jump to a relative position in this function
	asBC_JNS			= 15,
	//! \brief If the value register is greater than 0 jump to a relative position in this function
	asBC_JP				= 16,
	//! \brief If the value register is less than or equal to 0 jump to a relative position in this function
	asBC_JNP			= 17,
	//! \brief If the value register is 0 set it to 1
	asBC_TZ				= 18,
	//! \brief If the value register is not 0 set it to 1
	asBC_TNZ			= 19,
	//! \brief If the value register is less than 0 set it to 1
	asBC_TS				= 20,
	//! \brief If the value register is greater than or equal to 0 set it to 1
	asBC_TNS			= 21,
	//! \brief If the value register is greater than 0 set it to 1
	asBC_TP				= 22,
	//! \brief If the value register is less than or equal to 0 set it to 1
	asBC_TNP			= 23,
	//! \brief Negate the 32bit integer value in the variable
	asBC_NEGi			= 24,
	//! \brief Negate the float value in the variable
	asBC_NEGf			= 25,
	//! \brief Negate the double value in the variable
	asBC_NEGd			= 26,
	//! \brief Increment the 16bit integer value that is stored at the address pointed to by the reference in the value register
	asBC_INCi16			= 27,
	//! \brief Increment the 8bit integer value that is stored at the address pointed to by the reference in the value register
	asBC_INCi8			= 28,
	//! \brief Decrement the 16bit integer value that is stored at the address pointed to by the reference in the value register
	asBC_DECi16			= 29,
	//! \brief Increment the 8bit integer value that is stored at the address pointed to by the reference in the value register
	asBC_DECi8			= 30,
	//! \brief Increment the 32bit integer value that is stored at the address pointed to by the reference in the value register
	asBC_INCi			= 31,
	//! \brief Decrement the 32bit integer value that is stored at the address pointed to by the reference in the value register
	asBC_DECi			= 32,
	//! \brief Increment the float value that is stored at the address pointed to by the reference in the value register
	asBC_INCf			= 33,
	//! \brief Decrement the float value that is stored at the address pointed to by the reference in the value register
	asBC_DECf			= 34,
	//! \brief Increment the double value that is stored at the address pointed to by the reference in the value register
	asBC_INCd			= 35,
	//! \brief Decrement the double value that is stored at the address pointed to by the reference in the value register
	asBC_DECd			= 36,
	//! \brief Increment the 32bit integer value in the variable
	asBC_IncVi			= 37,
	//! \brief Decrement the 32bit integer value in the variable
	asBC_DecVi			= 38,
	//! \brief Perform a bitwise complement on the 32bit value in the variable
	asBC_BNOT			= 39,
	//! \brief Perform a bitwise and of two 32bit values and store the result in a third variable
	asBC_BAND			= 40,
	//! \brief Perform a bitwise or of two 32bit values and store the result in a third variable
	asBC_BOR			= 41,
	//! \brief Perform a bitwise exclusive or of two 32bit values and store the result in a third variable
	asBC_BXOR			= 42,
	//! \brief Perform a logical left shift of a 32bit value and store the result in a third variable
	asBC_BSLL			= 43,
	//! \brief Perform a logical right shift of a 32bit value and store the result in a third variable
	asBC_BSRL			= 44,
	//! \brief Perform a arithmetical right shift of a 32bit value and store the result in a third variable
	asBC_BSRA			= 45,
	//! \brief Pop the destination and source addresses from the stack. Perform a bitwise copy of the referred object. Push the destination address on the stack.
	asBC_COPY			= 46,
	//! \brief Push a 64bit value on the stack
	asBC_PshC8			= 47,
	//! \brief Push a pointer from the variable on the stack
	asBC_PshVPtr		= 48,
	//! \brief Pop top address, read a pointer from it, and push the pointer onto the stack
	asBC_RDSPtr			= 49,
	//! \brief Compare two double variables and store the result in the value register
	asBC_CMPd			= 50,
	//! \brief Compare two unsigned 32bit integer variables and store the result in the value register
	asBC_CMPu			= 51,
	//! \brief Compare two float variables and store the result in the value register
	asBC_CMPf			= 52,
	//! \brief Compare two 32bit integer variables and store the result in the value register
	asBC_CMPi			= 53,
	//! \brief Compare 32bit integer variable with constant and store the result in value register
	asBC_CMPIi			= 54,
	//! \brief Compare float variable with constant and store the result in value register
	asBC_CMPIf			= 55,
	//! \brief Compare unsigned 32bit integer variable with constant and store the result in value register
	asBC_CMPIu			= 56,
	//! \brief Jump to relative position in the function where the offset is stored in a variable
	asBC_JMPP			= 57,
	//! \brief Pop a pointer from the stack and store it in the value register
	asBC_PopRPtr		= 58,
	//! \brief Push a pointer from the value register onto the stack
	asBC_PshRPtr		= 59,
	//! \brief Not used
	asBC_STR			= 60,
	//! \brief Call registered function. Suspend further execution if requested.
	asBC_CALLSYS		= 61,
	//! \brief Jump to an imported script function, indexed by the argument
	asBC_CALLBND		= 62,
	//! \brief Call line callback function if set. Suspend execution if requested.
	asBC_SUSPEND		= 63,
	//! \brief Allocate the memory for the object. If the type is a script object then jump to the constructor, else call the registered constructor behaviour. Suspend further execution if requested.
	asBC_ALLOC			= 64,
	//! \brief Pop the address of the object variable from the stack. If ref type, call the release method, else call the destructor then free the memory. Clear the pointer in the variable.
	asBC_FREE			= 65,
	//! \brief Copy the object pointer from a variable to the object register. Clear the variable.
	asBC_LOADOBJ		= 66,
	//! \brief Copy the object pointer from the object register to the variable. Clear the object register.
	asBC_STOREOBJ		= 67,
	//! \brief Move object pointer from variable onto stack location.
	asBC_GETOBJ			= 68,
	//! \brief Pop destination handle reference. Perform a handle assignment, while updating the reference count for both previous and new objects.
	asBC_REFCPY			= 69,
	//! \brief Throw an exception if the pointer on the top of the stack is null.
	asBC_CHKREF			= 70,
	//! \brief Replace a variable index on the stack with the object handle stored in that variable.
	asBC_GETOBJREF		= 71,
	//! \brief Replace a variable index on the stack with the address of the variable.
	asBC_GETREF			= 72,
	//! \brief Push a null pointer on the stack.
	asBC_PshNull		= 73,
	//! \brief Clear pointer in a variable.
	asBC_ClrVPtr		= 74,
	//! \brief Push the pointer argument onto the stack. The pointer is a pointer to an object type structure
	asBC_OBJTYPE		= 75,
	//! \brief Push the type id onto the stack. Equivalent to \ref asBC_PshC4 "PshC4".
	asBC_TYPEID			= 76,
	//! \brief Initialize the variable with a DWORD.
	asBC_SetV4			= 77,
	//! \brief Initialize the variable with a QWORD.
	asBC_SetV8			= 78,
	//! \brief Add a value to the top pointer on the stack, thus updating the address itself.
	asBC_ADDSi			= 79,
	//! \brief Copy a DWORD from one variable to another
	asBC_CpyVtoV4		= 80,
	//! \brief Copy a QWORD from one variable to another
	asBC_CpyVtoV8		= 81,
	//! \brief Copy a DWORD from a variable into the value register
	asBC_CpyVtoR4		= 82,
	//! \brief Copy a QWORD from a variable into the value register
	asBC_CpyVtoR8		= 83,
	//! \brief Copy a DWORD from a local variable to a global variable
	asBC_CpyVtoG4		= 84,
	//! \brief Copy a DWORD from the value register into a variable
	asBC_CpyRtoV4		= 85,
	//! \brief Copy a QWORD from the value register into a variable
	asBC_CpyRtoV8		= 86,
	//! \brief Copy a DWORD from a global variable to a local variable
	asBC_CpyGtoV4		= 87,
	//! \brief Copy a BYTE from a variable to the address held in the value register
	asBC_WRTV1			= 88,
	//! \brief Copy a WORD from a variable to the address held in the value register
	asBC_WRTV2			= 89,
	//! \brief Copy a DWORD from a variable to the address held in the value register
	asBC_WRTV4			= 90,
	//! \brief Copy a QWORD from a variable to the address held in the value register
	asBC_WRTV8			= 91,
	//! \brief Copy a BYTE from address held in the value register to a variable. Clear the top bytes in the variable
	asBC_RDR1			= 92,
	//! \brief Copy a WORD from address held in the value register to a variable. Clear the top word in the variable
	asBC_RDR2			= 93,
	//! \brief Copy a DWORD from address held in the value register to a variable.
	asBC_RDR4			= 94,
	//! \brief Copy a QWORD from address held in the value register to a variable.
	asBC_RDR8			= 95,
	//! \brief Load the address of a global variable into the value register
	asBC_LDG			= 96,
	//! \brief Load the address of a local variable into the value register
	asBC_LDV			= 97,
	//! \brief Push the address of a global variable on the stack
	asBC_PGA			= 98,
	//! \brief Compare two pointers.
	asBC_CmpPtr			= 99,
	//! \brief Push the index of the variable on the stack, with the size of a pointer.
	asBC_VAR			= 100,
	//! \brief Convert the 32bit integer value to a float in the variable
	asBC_iTOf			= 101,
	//! \brief Convert the float value to a 32bit integer in the variable
	asBC_fTOi			= 102,
	//! \brief Convert the unsigned 32bit integer value to a float in the variable
	asBC_uTOf			= 103,
	//! \brief Convert the float value to an unsigned 32bit integer in the variable
	asBC_fTOu			= 104,
	//! \brief Expand the low byte as a signed value to a full 32bit integer in the variable
	asBC_sbTOi			= 105,
	//! \brief Expand the low word as a signed value to a full 32bit integer in the variable
	asBC_swTOi			= 106,
	//! \brief Expand the low byte as an unsigned value to a full 32bit integer in the variable
	asBC_ubTOi			= 107,
	//! \brief Expand the low word as an unsigned value to a full 32bit integer in the variable
	asBC_uwTOi			= 108,
	//! \brief Convert the double value in one variable to a 32bit integer in another variable
	asBC_dTOi			= 109,
	//! \brief Convert the double value in one variable to a 32bit unsigned integer in another variable
	asBC_dTOu			= 110,
	//! \brief Convert the double value in one variable to a float in another variable
	asBC_dTOf			= 111,
	//! \brief Convert the 32bit integer value in one variable to a double in another variable
	asBC_iTOd			= 112,
	//! \brief Convert the 32bit unsigned integer value in one variable to a double in another variable
	asBC_uTOd			= 113,
	//! \brief Convert the float value in one variable to a double in another variable
	asBC_fTOd			= 114,
	//! \brief Add the values of two 32bit integer variables and store in a third variable
	asBC_ADDi			= 115,
	//! \brief Subtract the values of two 32bit integer variables and store in a third variable
	asBC_SUBi			= 116,
	//! \brief Multiply the values of two 32bit integer variables and store in a third variable
	asBC_MULi			= 117,
	//! \brief Divide the values of two 32bit integer variables and store in a third variable
	asBC_DIVi			= 118,
	//! \brief Calculate the modulo of values of two 32bit integer variables and store in a third variable
	asBC_MODi			= 119,
	//! \brief Add the values of two float variables and store in a third variable
	asBC_ADDf			= 120,
	//! \brief Subtract the values of two float variables and store in a third variable
	asBC_SUBf			= 121,
	//! \brief Multiply the values of two float variables and store in a third variable
	asBC_MULf			= 122,
	//! \brief Divide the values of two float variables and store in a third variable
	asBC_DIVf			= 123,
	//! \brief Calculate the modulo of values of two float variables and store in a third variable
	asBC_MODf			= 124,
	//! \brief Add the values of two double variables and store in a third variable
	asBC_ADDd			= 125,
	//! \brief Subtract the values of two double variables and store in a third variable
	asBC_SUBd			= 126,
	//! \brief Multiply the values of two double variables and store in a third variable
	asBC_MULd			= 127,
	//! \brief Divide the values of two double variables and store in a third variable
	asBC_DIVd			= 128,
	//! \brief Calculate the modulo of values of two double variables and store in a third variable
	asBC_MODd			= 129,
	//! \brief Add a 32bit integer variable with a constant value and store the result in another variable
	asBC_ADDIi			= 130,
	//! \brief Subtract a 32bit integer variable with a constant value and store the result in another variable
	asBC_SUBIi			= 131,
	//! \brief Multiply a 32bit integer variable with a constant value and store the result in another variable
	asBC_MULIi			= 132,
	//! \brief Add a float variable with a constant value and store the result in another variable
	asBC_ADDIf			= 133,
	//! \brief Subtract a float variable with a constant value and store the result in another variable
	asBC_SUBIf			= 134,
	//! \brief Multiply a float variable with a constant value and store the result in another variable
	asBC_MULIf			= 135,
	//! \brief Set the value of global variable to a 32bit word
	asBC_SetG4			= 136,
	//! \brief Throw an exception if the address stored on the stack points to a null pointer
	asBC_ChkRefS		= 137,
	//! \brief Throw an exception if the variable is null
	asBC_ChkNullV		= 138,
	//! \brief Jump to an interface method, indexed by the argument
	asBC_CALLINTF		= 139,
	//! \brief Convert a 32bit integer in a variable to a byte, clearing the top bytes
	asBC_iTOb			= 140,
	//! \brief Convert a 32bit integer in a variable to a word, clearing the top word
	asBC_iTOw			= 141,
	//! \brief Same as \ref asBC_SetV4 "SetV4"
	asBC_SetV1			= 142,
	//! \brief Same as \ref asBC_SetV4 "SetV4"
	asBC_SetV2			= 143,
	//! \brief Pop an object handle to a script class from the stack. Perform a dynamic cast on it and store the result in the object register.
	asBC_Cast			= 144,
	//! \brief Convert the 64bit integer value in one variable to a 32bit integer in another variable
	asBC_i64TOi			= 145,
	//! \brief Convert the 32bit unsigned integer value in one variable to a 64bit integer in another variable
	asBC_uTOi64			= 146,
	//! \brief Convert the 32bit integer value in one variable to a 64bit integer in another variable
	asBC_iTOi64			= 147,
	//! \brief Convert the float value in one variable to a 64bit integer in another variable
	asBC_fTOi64			= 148,
	//! \brief Convert the double value in the variable to a 64bit integer
	asBC_dTOi64			= 149,
	//! \brief Convert the float value in one variable to a 64bit unsigned integer in another variable
	asBC_fTOu64			= 150,
	//! \brief Convert the double value in the variable to a 64bit unsigned integer
	asBC_dTOu64			= 151,
	//! \brief Convert the 64bit integer value in one variable to a float in another variable
	asBC_i64TOf			= 152,
	//! \brief Convert the 64bit unsigned integer value in one variable to a float in another variable
	asBC_u64TOf			= 153,
	//! \brief Convert the 32bit integer value in the variable to a double
	asBC_i64TOd			= 154,
	//! \brief Convert the 32bit unsigned integer value in the variable to a double
	asBC_u64TOd			= 155,
	//! \brief Negate the 64bit integer value in the variable
	asBC_NEGi64			= 156,
	//! \brief Increment the 64bit integer value that is stored at the address pointed to by the reference in the value register
	asBC_INCi64			= 157,
	//! \brief Decrement the 64bit integer value that is stored at the address pointed to by the reference in the value register
	asBC_DECi64			= 158,
	//! \brief Perform a bitwise complement on the 64bit value in the variable
	asBC_BNOT64			= 159,
	//! \brief Perform an addition with two 64bit integer variables and store the result in a third variable
	asBC_ADDi64			= 160,
	//! \brief Perform a subtraction with two 64bit integer variables and store the result in a third variable
	asBC_SUBi64			= 161,
	//! \brief Perform a multiplication with two 64bit integer variables and store the result in a third variable
	asBC_MULi64			= 162,
	//! \brief Perform a division with two 64bit integer variables and store the result in a third variable
	asBC_DIVi64			= 163,
	//! \brief Perform the modulo operation with two 64bit integer variables and store the result in a third variable
	asBC_MODi64			= 164,
	//! \brief Perform a bitwise and of two 64bit values and store the result in a third variable
	asBC_BAND64			= 165,
	//! \brief Perform a bitwise or of two 64bit values and store the result in a third variable
	asBC_BOR64			= 166,
	//! \brief Perform a bitwise exclusive or of two 64bit values and store the result in a third variable
	asBC_BXOR64			= 167,
	//! \brief Perform a logical left shift of a 64bit value and store the result in a third variable
	asBC_BSLL64			= 168,
	//! \brief Perform a logical right shift of a 64bit value and store the result in a third variable
	asBC_BSRL64			= 169,
	//! \brief Perform a arithmetical right shift of a 64bit value and store the result in a third variable
	asBC_BSRA64			= 170,
	//! \brief Compare two 64bit integer variables and store the result in the value register
	asBC_CMPi64			= 171,
	//! \brief Compare two unsigned 64bit integer variables and store the result in the value register
	asBC_CMPu64			= 172,
	//! \brief Check if a pointer on the stack is null, and if it is throw an exception. The argument is relative to the top of the stack
	asBC_ChkNullS		= 173,
	//! \brief Clear the upper bytes of the value register so that only the value in the lowest byte is kept
	asBC_ClrHi			= 174,
	//! \brief If a JIT function is available and the argument is not 0 then call the JIT function
	asBC_JitEntry		= 175,
	//! \brief Call a function stored in a local function pointer
	asBC_CallPtr		= 176,
	//! \brief Push a function pointer on the stack
	asBC_FuncPtr		= 177,
	//! \brief Load the address to a property of the local object into the stack. PshV4 0, ADDSi x, PopRPtr
	asBC_LoadThisR		= 178,
	//! \brief Push the 64bit value from a variable onto the stack
	asBC_PshV8			= 179,
	//! \brief Divide the values of two 32bit unsigned integer variables and store in a third variable
	asBC_DIVu			= 180,
	//! \brief Calculate the modulo of values of two 32bit unsigned integer variables and store in a third variable
	asBC_MODu			= 181,
	//! \brief Divide the values of two 64bit unsigned integer variables and store in a third variable
	asBC_DIVu64			= 182,
	//! \brief Calculate the modulo of values of two 64bit unsigned integer variables and store in a third variable
	asBC_MODu64			= 183,
	//! \brief Load address of member of reference object into register
	asBC_LoadRObjR		= 184,
	//! \brief Load address of member of value object into register
	asBC_LoadVObjR		= 185,
	//! \brief Copies a handle to a variable
	asBC_RefCpyV		= 186,
	//! \brief Jump if low byte of value register is 0
	asBC_JLowZ			= 187,
	//! \brief Jump if low byte of value register is not 0
	asBC_JLowNZ			= 188,
	//! \brief Allocates memory for an initialization list buffer
	asBC_AllocMem		= 189,
	//! \brief Sets a repeat count in the list buffer
	asBC_SetListSize	= 190,
	//! \brief Pushes the address of an element in the list buffer on the stack
	asBC_PshListElmnt	= 191,
	//! \brief Sets the type of the next element in the list buffer
	asBC_SetListType	= 192,
	//! \brief Computes the power of for two int values
	asBC_POWi			= 193,
	//! \brief Computes the power of for two uint values
	asBC_POWu			= 194,
	//! \brief Computes the power of for two float values
	asBC_POWf			= 195,
	//! \brief Computes the power of for two double values
	asBC_POWd			= 196,
	//! \brief Computes the power of where base is a double and exponent is an int value
	asBC_POWdi			= 197,
	//! \brief Computes the power of for two int64 values
	asBC_POWi64			= 198,
	//! \brief Computes the power of for two uint64 values
	asBC_POWu64			= 199,
	//! \brief Call registered function with single 32bit integer argument. Suspend further execution if requested.
	asBC_Thiscall1		= 200,

	asBC_MAXBYTECODE	= 201,

	// Temporary tokens. Can't be output to the final program
	asBC_TryBlock		= 250,
	asBC_VarDecl		= 251,
	asBC_Block			= 252,
	asBC_ObjInfo		= 253,
	asBC_LINE			= 254,
	asBC_LABEL			= 255
};

// Instruction types
//! \brief Describes the structure of a bytecode instruction
enum asEBCType
{
	asBCTYPE_INFO         = 0,
	//! \brief Instruction + no args
	asBCTYPE_NO_ARG       = 1,
	//! \brief Instruction + WORD arg
	asBCTYPE_W_ARG        = 2,
	//! \brief Instruction + WORD arg (dest var)
	asBCTYPE_wW_ARG       = 3,
	//! \brief Instruction + DWORD arg
	asBCTYPE_DW_ARG       = 4,
	//! \brief Instruction + WORD arg (source var) + DWORD arg
	asBCTYPE_rW_DW_ARG    = 5,
	//! \brief Instruction + QWORD arg
	asBCTYPE_QW_ARG       = 6,
	//! \brief Instruction + DWORD arg + DWORD arg
	asBCTYPE_DW_DW_ARG    = 7,
	//! \brief Instruction + WORD arg (dest var) + WORD arg (source var) + WORD arg (source var)
	asBCTYPE_wW_rW_rW_ARG = 8,
	//! \brief Instruction + WORD arg (dest var) + QWORD arg
	asBCTYPE_wW_QW_ARG    = 9,
	//! \brief Instruction + WORD arg (dest var) + WORD arg (source var)
	asBCTYPE_wW_rW_ARG    = 10,
	//! \brief Instruction + WORD arg (source var)
	asBCTYPE_rW_ARG       = 11,
	//! \brief Instruction + WORD arg (dest var) + DWORD arg
	asBCTYPE_wW_DW_ARG    = 12,
	//! \brief Instruction + WORD arg (dest var) + WORD arg (source var) + DWORD arg
	asBCTYPE_wW_rW_DW_ARG = 13,
	//! \brief Instruction + WORD arg (source var) + WORD arg (source var)
	asBCTYPE_rW_rW_ARG    = 14,
	//! \brief Instruction + WORD arg (dest var) + WORD arg
	asBCTYPE_wW_W_ARG     = 15,
	//! \brief Instruction + QWORD arg + DWORD arg
	asBCTYPE_QW_DW_ARG    = 16,
	//! \brief Instruction + WORD arg (source var) + QWORD arg
	asBCTYPE_rW_QW_ARG    = 17,
	//! \brief Instruction + WORD arg + DWORD arg
	asBCTYPE_W_DW_ARG     = 18,
	//! \brief Instruction + WORD arg(source var) + WORD arg + DWORD arg
	asBCTYPE_rW_W_DW_ARG  = 19,
	//! \brief Instruction + WORD arg(source var) + DWORD arg + DWORD arg
	asBCTYPE_rW_DW_DW_ARG = 20
};

// Instruction type sizes
//! \brief Lookup table for determining the size of each \ref asEBCType "type" of bytecode instruction.
const int asBCTypeSize[21] =
{
	0, // asBCTYPE_INFO
	1, // asBCTYPE_NO_ARG
	1, // asBCTYPE_W_ARG
	1, // asBCTYPE_wW_ARG
	2, // asBCTYPE_DW_ARG
	2, // asBCTYPE_rW_DW_ARG
	3, // asBCTYPE_QW_ARG
	3, // asBCTYPE_DW_DW_ARG
	2, // asBCTYPE_wW_rW_rW_ARG
	3, // asBCTYPE_wW_QW_ARG
	2, // asBCTYPE_wW_rW_ARG
	1, // asBCTYPE_rW_ARG
	2, // asBCTYPE_wW_DW_ARG
	3, // asBCTYPE_wW_rW_DW_ARG
	2, // asBCTYPE_rW_rW_ARG
	2, // asBCTYPE_wW_W_ARG
	4, // asBCTYPE_QW_DW_ARG
	3, // asBCTYPE_rW_QW_ARG
	2, // asBCTYPE_W_DW_ARG
	3, // asBCTYPE_rW_W_DW_ARG
	3  // asBCTYPE_rW_DW_DW_ARG
};

// Instruction info
//! \brief Information on a bytecode instruction
//!
//! This structure can be obtained for each bytecode instruction
//! by looking it up in the \ref asBCInfo array.
//!
//! The size of the instruction can be obtained by looking up the 
//! type in the \ref asBCTypeSize array.
//!
//! \see \ref doc_adv_jit
struct asSBCInfo
{
	//! \brief Bytecode instruction id
	asEBCInstr  bc;
	//! \brief Instruction argument layout
	asEBCType   type;
	//! \brief How much this argument increments the stack pointer. 0xFFFF if it depends on the arguments.
	int         stackInc;
	//! \brief Name of the instruction for debugging
	const char *name;
};

#ifndef AS_64BIT_PTR
	#define asBCTYPE_PTR_ARG    asBCTYPE_DW_ARG
	#define asBCTYPE_PTR_DW_ARG asBCTYPE_DW_DW_ARG
	#define asBCTYPE_wW_PTR_ARG asBCTYPE_wW_DW_ARG
	#define asBCTYPE_rW_PTR_ARG asBCTYPE_rW_DW_ARG
	#ifndef AS_PTR_SIZE
		#define AS_PTR_SIZE 1
	#endif
#else
	#define asBCTYPE_PTR_ARG    asBCTYPE_QW_ARG
	#define asBCTYPE_PTR_DW_ARG asBCTYPE_QW_DW_ARG
	#define asBCTYPE_wW_PTR_ARG asBCTYPE_wW_QW_ARG
	#define asBCTYPE_rW_PTR_ARG asBCTYPE_rW_QW_ARG
	#ifndef AS_PTR_SIZE
		#define AS_PTR_SIZE 2
	#endif
#endif

#define asBCINFO(b,t,s) {asBC_##b, asBCTYPE_##t, s, #b}
#define asBCINFO_DUMMY(b) {asBC_MAXBYTECODE, asBCTYPE_INFO, 0, "BC_" #b}

//! \brief Information on each bytecode instruction
const asSBCInfo asBCInfo[256] =
{
	asBCINFO(PopPtr,	NO_ARG,			-AS_PTR_SIZE),
	asBCINFO(PshGPtr,	PTR_ARG,		AS_PTR_SIZE),
	asBCINFO(PshC4,		DW_ARG,			1),
	asBCINFO(PshV4,		rW_ARG,			1),
	asBCINFO(PSF,		rW_ARG,			AS_PTR_SIZE),
	asBCINFO(SwapPtr,	NO_ARG,			0),
	asBCINFO(NOT,		rW_ARG,			0),
	asBCINFO(PshG4,		PTR_ARG,		1),
	asBCINFO(LdGRdR4,	wW_PTR_ARG,		0),
	asBCINFO(CALL,		DW_ARG,			0xFFFF),
	asBCINFO(RET,		W_ARG,			0xFFFF),
	asBCINFO(JMP,		DW_ARG,			0),
	asBCINFO(JZ,		DW_ARG,			0),
	asBCINFO(JNZ,		DW_ARG,			0),
	asBCINFO(JS,		DW_ARG,			0),
	asBCINFO(JNS,		DW_ARG,			0),
	asBCINFO(JP,		DW_ARG,			0),
	asBCINFO(JNP,		DW_ARG,			0),
	asBCINFO(TZ,		NO_ARG,			0),
	asBCINFO(TNZ,		NO_ARG,			0),
	asBCINFO(TS,		NO_ARG,			0),
	asBCINFO(TNS,		NO_ARG,			0),
	asBCINFO(TP,		NO_ARG,			0),
	asBCINFO(TNP,		NO_ARG,			0),
	asBCINFO(NEGi,		rW_ARG,			0),
	asBCINFO(NEGf,		rW_ARG,			0),
	asBCINFO(NEGd,		rW_ARG,			0),
	asBCINFO(INCi16,	NO_ARG,			0),
	asBCINFO(INCi8,		NO_ARG,			0),
	asBCINFO(DECi16,	NO_ARG,			0),
	asBCINFO(DECi8,		NO_ARG,			0),
	asBCINFO(INCi,		NO_ARG,			0),
	asBCINFO(DECi,		NO_ARG,			0),
	asBCINFO(INCf,		NO_ARG,			0),
	asBCINFO(DECf,		NO_ARG,			0),
	asBCINFO(INCd,		NO_ARG,			0),
	asBCINFO(DECd,		NO_ARG,			0),
	asBCINFO(IncVi,		rW_ARG,			0),
	asBCINFO(DecVi,		rW_ARG,			0),
	asBCINFO(BNOT,		rW_ARG,			0),
	asBCINFO(BAND,		wW_rW_rW_ARG,	0),
	asBCINFO(BOR,		wW_rW_rW_ARG,	0),
	asBCINFO(BXOR,		wW_rW_rW_ARG,	0),
	asBCINFO(BSLL,		wW_rW_rW_ARG,	0),
	asBCINFO(BSRL,		wW_rW_rW_ARG,	0),
	asBCINFO(BSRA,		wW_rW_rW_ARG,	0),
	asBCINFO(COPY,		W_DW_ARG,		-AS_PTR_SIZE),
	asBCINFO(PshC8,		QW_ARG,			2),
	asBCINFO(PshVPtr,	rW_ARG,			AS_PTR_SIZE),
	asBCINFO(RDSPtr,	NO_ARG,			0),
	asBCINFO(CMPd,		rW_rW_ARG,		0),
	asBCINFO(CMPu,		rW_rW_ARG,		0),
	asBCINFO(CMPf,		rW_rW_ARG,		0),
	asBCINFO(CMPi,		rW_rW_ARG,		0),
	asBCINFO(CMPIi,		rW_DW_ARG,		0),
	asBCINFO(CMPIf,		rW_DW_ARG,		0),
	asBCINFO(CMPIu,		rW_DW_ARG,		0),
	asBCINFO(JMPP,		rW_ARG,			0),
	asBCINFO(PopRPtr,	NO_ARG,			-AS_PTR_SIZE),
	asBCINFO(PshRPtr,	NO_ARG,			AS_PTR_SIZE),
	asBCINFO(STR,		W_ARG,			1+AS_PTR_SIZE),
	asBCINFO(CALLSYS,	DW_ARG,			0xFFFF),
	asBCINFO(CALLBND,	DW_ARG,			0xFFFF),
	asBCINFO(SUSPEND,	NO_ARG,			0),
	asBCINFO(ALLOC,		PTR_DW_ARG,		0xFFFF),
	asBCINFO(FREE,		wW_PTR_ARG,		0),
	asBCINFO(LOADOBJ,	rW_ARG,			0),
	asBCINFO(STOREOBJ,	wW_ARG,			0),
	asBCINFO(GETOBJ,	W_ARG,			0),
	asBCINFO(REFCPY,	PTR_ARG,		-AS_PTR_SIZE),
	asBCINFO(CHKREF,	NO_ARG,			0),
	asBCINFO(GETOBJREF,	W_ARG,			0),
	asBCINFO(GETREF,	W_ARG,			0),
	asBCINFO(PshNull,	NO_ARG,			AS_PTR_SIZE),
	asBCINFO(ClrVPtr,	wW_ARG,			0),
	asBCINFO(OBJTYPE,	PTR_ARG,		AS_PTR_SIZE),
	asBCINFO(TYPEID,	DW_ARG,			1),
	asBCINFO(SetV4,		wW_DW_ARG,		0),
	asBCINFO(SetV8,		wW_QW_ARG,		0),
	asBCINFO(ADDSi,		W_DW_ARG,		0),
	asBCINFO(CpyVtoV4,	wW_rW_ARG,		0),
	asBCINFO(CpyVtoV8,	wW_rW_ARG,		0),
	asBCINFO(CpyVtoR4,	rW_ARG,			0),
	asBCINFO(CpyVtoR8,	rW_ARG,			0),
	asBCINFO(CpyVtoG4,	rW_PTR_ARG,		0),
	asBCINFO(CpyRtoV4,	wW_ARG,			0),
	asBCINFO(CpyRtoV8,	wW_ARG,			0),
	asBCINFO(CpyGtoV4,	wW_PTR_ARG,		0),
	asBCINFO(WRTV1,		rW_ARG,			0),
	asBCINFO(WRTV2,		rW_ARG,			0),
	asBCINFO(WRTV4,		rW_ARG,			0),
	asBCINFO(WRTV8,		rW_ARG,			0),
	asBCINFO(RDR1,		wW_ARG,			0),
	asBCINFO(RDR2,		wW_ARG,			0),
	asBCINFO(RDR4,		wW_ARG,			0),
	asBCINFO(RDR8,		wW_ARG,			0),
	asBCINFO(LDG,		PTR_ARG,		0),
	asBCINFO(LDV,		rW_ARG,			0),
	asBCINFO(PGA,		PTR_ARG,		AS_PTR_SIZE),
	asBCINFO(CmpPtr,	rW_rW_ARG,		0),
	asBCINFO(VAR,		rW_ARG,			AS_PTR_SIZE),
	asBCINFO(iTOf,		rW_ARG,			0),
	asBCINFO(fTOi,		rW_ARG,			0),
	asBCINFO(uTOf,		rW_ARG,			0),
	asBCINFO(fTOu,		rW_ARG,			0),
	asBCINFO(sbTOi,		rW_ARG,			0),
	asBCINFO(swTOi,		rW_ARG,			0),
	asBCINFO(ubTOi,		rW_ARG,			0),
	asBCINFO(uwTOi,		rW_ARG,			0),
	asBCINFO(dTOi,		wW_rW_ARG,		0),
	asBCINFO(dTOu,		wW_rW_ARG,		0),
	asBCINFO(dTOf,		wW_rW_ARG,		0),
	asBCINFO(iTOd,		wW_rW_ARG,		0),
	asBCINFO(uTOd,		wW_rW_ARG,		0),
	asBCINFO(fTOd,		wW_rW_ARG,		0),
	asBCINFO(ADDi,		wW_rW_rW_ARG,	0),
	asBCINFO(SUBi,		wW_rW_rW_ARG,	0),
	asBCINFO(MULi,		wW_rW_rW_ARG,	0),
	asBCINFO(DIVi,		wW_rW_rW_ARG,	0),
	asBCINFO(MODi,		wW_rW_rW_ARG,	0),
	asBCINFO(ADDf,		wW_rW_rW_ARG,	0),
	asBCINFO(SUBf,		wW_rW_rW_ARG,	0),
	asBCINFO(MULf,		wW_rW_rW_ARG,	0),
	asBCINFO(DIVf,		wW_rW_rW_ARG,	0),
	asBCINFO(MODf,		wW_rW_rW_ARG,	0),
	asBCINFO(ADDd,		wW_rW_rW_ARG,	0),
	asBCINFO(SUBd,		wW_rW_rW_ARG,	0),
	asBCINFO(MULd,		wW_rW_rW_ARG,	0),
	asBCINFO(DIVd,		wW_rW_rW_ARG,	0),
	asBCINFO(MODd,		wW_rW_rW_ARG,	0),
	asBCINFO(ADDIi,		wW_rW_DW_ARG,	0),
	asBCINFO(SUBIi,		wW_rW_DW_ARG,	0),
	asBCINFO(MULIi,		wW_rW_DW_ARG,	0),
	asBCINFO(ADDIf,		wW_rW_DW_ARG,	0),
	asBCINFO(SUBIf,		wW_rW_DW_ARG,	0),
	asBCINFO(MULIf,		wW_rW_DW_ARG,	0),
	asBCINFO(SetG4,		PTR_DW_ARG,		0),
	asBCINFO(ChkRefS,	NO_ARG,			0),
	asBCINFO(ChkNullV,	rW_ARG,			0),
	asBCINFO(CALLINTF,	DW_ARG,			0xFFFF),
	asBCINFO(iTOb,		rW_ARG,			0),
	asBCINFO(iTOw,		rW_ARG,			0),
	asBCINFO(SetV1,		wW_DW_ARG,		0),
	asBCINFO(SetV2,		wW_DW_ARG,		0),
	asBCINFO(Cast,		DW_ARG,			-AS_PTR_SIZE),
	asBCINFO(i64TOi,	wW_rW_ARG,		0),
	asBCINFO(uTOi64,	wW_rW_ARG,		0),
	asBCINFO(iTOi64,	wW_rW_ARG,		0),
	asBCINFO(fTOi64,	wW_rW_ARG,		0),
	asBCINFO(dTOi64,	rW_ARG,			0),
	asBCINFO(fTOu64,	wW_rW_ARG,		0),
	asBCINFO(dTOu64,	rW_ARG,			0),
	asBCINFO(i64TOf,	wW_rW_ARG,		0),
	asBCINFO(u64TOf,	wW_rW_ARG,		0),
	asBCINFO(i64TOd,	rW_ARG,			0),
	asBCINFO(u64TOd,	rW_ARG,			0),
	asBCINFO(NEGi64,	rW_ARG,			0),
	asBCINFO(INCi64,	NO_ARG,			0),
	asBCINFO(DECi64,	NO_ARG,			0),
	asBCINFO(BNOT64,	rW_ARG,			0),
	asBCINFO(ADDi64,	wW_rW_rW_ARG,	0),
	asBCINFO(SUBi64,	wW_rW_rW_ARG,	0),
	asBCINFO(MULi64,	wW_rW_rW_ARG,	0),
	asBCINFO(DIVi64,	wW_rW_rW_ARG,	0),
	asBCINFO(MODi64,	wW_rW_rW_ARG,	0),
	asBCINFO(BAND64,	wW_rW_rW_ARG,	0),
	asBCINFO(BOR64,		wW_rW_rW_ARG,	0),
	asBCINFO(BXOR64,	wW_rW_rW_ARG,	0),
	asBCINFO(BSLL64,	wW_rW_rW_ARG,	0),
	asBCINFO(BSRL64,	wW_rW_rW_ARG,	0),
	asBCINFO(BSRA64,	wW_rW_rW_ARG,	0),
	asBCINFO(CMPi64,	rW_rW_ARG,		0),
	asBCINFO(CMPu64,	rW_rW_ARG,		0),
	asBCINFO(ChkNullS,	W_ARG,			0),
	asBCINFO(ClrHi,		NO_ARG,			0),
	asBCINFO(JitEntry,	PTR_ARG,		0),
	asBCINFO(CallPtr,	rW_ARG,			0xFFFF),
	asBCINFO(FuncPtr,	PTR_ARG,		AS_PTR_SIZE),
	asBCINFO(LoadThisR,	W_DW_ARG,		0),
	asBCINFO(PshV8,		rW_ARG,			2),
	asBCINFO(DIVu,		wW_rW_rW_ARG,	0),
	asBCINFO(MODu,		wW_rW_rW_ARG,	0),
	asBCINFO(DIVu64,	wW_rW_rW_ARG,	0),
	asBCINFO(MODu64,	wW_rW_rW_ARG,	0),
	asBCINFO(LoadRObjR,	rW_W_DW_ARG,	0),
	asBCINFO(LoadVObjR,	rW_W_DW_ARG,	0),
	asBCINFO(RefCpyV,	wW_PTR_ARG,		0),
	asBCINFO(JLowZ,		DW_ARG,			0),
	asBCINFO(JLowNZ,	DW_ARG,			0),
	asBCINFO(AllocMem,	wW_DW_ARG,		0),
	asBCINFO(SetListSize, rW_DW_DW_ARG,	0),
	asBCINFO(PshListElmnt, rW_DW_ARG,	AS_PTR_SIZE),
	asBCINFO(SetListType, rW_DW_DW_ARG,	0),
	asBCINFO(POWi,		wW_rW_rW_ARG,	0),
	asBCINFO(POWu,		wW_rW_rW_ARG,	0),
	asBCINFO(POWf,		wW_rW_rW_ARG,	0),
	asBCINFO(POWd,		wW_rW_rW_ARG,	0),
	asBCINFO(POWdi,		wW_rW_rW_ARG,	0),
	asBCINFO(POWi64,	wW_rW_rW_ARG,	0),
	asBCINFO(POWu64,	wW_rW_rW_ARG,	0),
	asBCINFO(Thiscall1, DW_ARG,			-AS_PTR_SIZE-1),

	asBCINFO_DUMMY(201),
	asBCINFO_DUMMY(202),
	asBCINFO_DUMMY(203),
	asBCINFO_DUMMY(204),
	asBCINFO_DUMMY(205),
	asBCINFO_DUMMY(206),
	asBCINFO_DUMMY(207),
	asBCINFO_DUMMY(208),
	asBCINFO_DUMMY(209),
	asBCINFO_DUMMY(210),
	asBCINFO_DUMMY(211),
	asBCINFO_DUMMY(212),
	asBCINFO_DUMMY(213),
	asBCINFO_DUMMY(214),
	asBCINFO_DUMMY(215),
	asBCINFO_DUMMY(216),
	asBCINFO_DUMMY(217),
	asBCINFO_DUMMY(218),
	asBCINFO_DUMMY(219),
	asBCINFO_DUMMY(220),
	asBCINFO_DUMMY(221),
	asBCINFO_DUMMY(222),
	asBCINFO_DUMMY(223),
	asBCINFO_DUMMY(224),
	asBCINFO_DUMMY(225),
	asBCINFO_DUMMY(226),
	asBCINFO_DUMMY(227),
	asBCINFO_DUMMY(228),
	asBCINFO_DUMMY(229),
	asBCINFO_DUMMY(230),
	asBCINFO_DUMMY(231),
	asBCINFO_DUMMY(232),
	asBCINFO_DUMMY(233),
	asBCINFO_DUMMY(234),
	asBCINFO_DUMMY(235),
	asBCINFO_DUMMY(236),
	asBCINFO_DUMMY(237),
	asBCINFO_DUMMY(238),
	asBCINFO_DUMMY(239),
	asBCINFO_DUMMY(240),
	asBCINFO_DUMMY(241),
	asBCINFO_DUMMY(242),
	asBCINFO_DUMMY(243),
	asBCINFO_DUMMY(244),
	asBCINFO_DUMMY(245),
	asBCINFO_DUMMY(246),
	asBCINFO_DUMMY(247),
	asBCINFO_DUMMY(248),
	asBCINFO_DUMMY(249),

	asBCINFO(TryBlock,		DW_ARG,			0),
	asBCINFO(VarDecl,		W_ARG,			0),
	asBCINFO(Block,			INFO,			0),
	asBCINFO(ObjInfo,		rW_DW_ARG,		0),
	asBCINFO(LINE,			INFO,			0),
	asBCINFO(LABEL,			INFO,			0)
};

// Macros to access bytecode instruction arguments
//! \brief Macro to access the first DWORD argument in the bytecode instruction
#define asBC_DWORDARG(x)  (*(((asDWORD*)x)+1))
//! \brief Macro to access the first 32bit integer argument in the bytecode instruction
#define asBC_INTARG(x)    (*(int*)(((asDWORD*)x)+1))
//! \brief Macro to access the first QWORD argument in the bytecode instruction
#define asBC_QWORDARG(x)  (*(asQWORD*)(((asDWORD*)x)+1))
//! \brief Macro to access the first float argument in the bytecode instruction
#define asBC_FLOATARG(x)  (*(float*)(((asDWORD*)x)+1))
//! \brief Macro to access the first pointer argument in the bytecode instruction
#define asBC_PTRARG(x)    (*(asPWORD*)(((asDWORD*)x)+1))
//! \brief Macro to access the first WORD argument in the bytecode instruction
#define asBC_WORDARG0(x)  (*(((asWORD*)x)+1))
//! \brief Macro to access the second WORD argument in the bytecode instruction
#define asBC_WORDARG1(x)  (*(((asWORD*)x)+2))
//! \brief Macro to access the first signed WORD argument in the bytecode instruction
#define asBC_SWORDARG0(x) (*(((short*)x)+1))
//! \brief Macro to access the second signed WORD argument in the bytecode instruction
#define asBC_SWORDARG1(x) (*(((short*)x)+2))
//! \brief Macro to access the third signed WORD argument in the bytecode instruction
#define asBC_SWORDARG2(x) (*(((short*)x)+3))


END_AS_NAMESPACE

#endif
