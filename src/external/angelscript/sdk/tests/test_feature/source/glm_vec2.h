// This code is taken from https://github.com/g-truc/glm to test compatibility with the glm::vec2 type
// It has been slightly modified to avoid dependency on the full GLM SDK.
//
// Source files: 
//  https://github.com/g-truc/glm/blob/master/glm/detail/type_vec2.hpp
//  https://github.com/g-truc/glm/blob/master/glm/detail/type_vec2.inl
//  https://github.com/g-truc/glm/blob/master/glm/ext/vector_float2.hpp
//  https://github.com/g-truc/glm/blob/master/glm/detail/qualifier.hpp
//  https://github.com/g-truc/glm/blob/master/glm/detail/setup.hpp





/// @ref core
/// @file glm/detail/type_vec2.hpp

#pragma once

//#include "qualifier.hpp"
//#include "setup.hpp
#ifndef GLM_SETUP_INCLUDED

#include <cassert>
#include <cstddef>

#define GLM_VERSION_MAJOR			0
#define GLM_VERSION_MINOR			9
#define GLM_VERSION_PATCH			9
#define GLM_VERSION_REVISION		7
#define GLM_VERSION					997
#define GLM_VERSION_MESSAGE			"GLM: version 0.9.9.7"

#define GLM_SETUP_INCLUDED			GLM_VERSION

///////////////////////////////////////////////////////////////////////////////////
// Active states

#define GLM_DISABLE		0
#define GLM_ENABLE		1

///////////////////////////////////////////////////////////////////////////////////
// Messages

#if defined(GLM_FORCE_MESSAGES)
#	define GLM_MESSAGES GLM_ENABLE
#else
#	define GLM_MESSAGES GLM_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Detect the platform

//#include "../simd/platform.h"
#pragma once

///////////////////////////////////////////////////////////////////////////////////
// Platform

#define GLM_PLATFORM_UNKNOWN		0x00000000
#define GLM_PLATFORM_WINDOWS		0x00010000
#define GLM_PLATFORM_LINUX			0x00020000
#define GLM_PLATFORM_APPLE			0x00040000
//#define GLM_PLATFORM_IOS			0x00080000
#define GLM_PLATFORM_ANDROID		0x00100000
#define GLM_PLATFORM_CHROME_NACL	0x00200000
#define GLM_PLATFORM_UNIX			0x00400000
#define GLM_PLATFORM_QNXNTO			0x00800000
#define GLM_PLATFORM_WINCE			0x01000000
#define GLM_PLATFORM_CYGWIN			0x02000000

#ifdef GLM_FORCE_PLATFORM_UNKNOWN
#	define GLM_PLATFORM GLM_PLATFORM_UNKNOWN
#elif defined(__CYGWIN__)
#	define GLM_PLATFORM GLM_PLATFORM_CYGWIN
#elif defined(__QNXNTO__)
#	define GLM_PLATFORM GLM_PLATFORM_QNXNTO
#elif defined(__APPLE__)
#	define GLM_PLATFORM GLM_PLATFORM_APPLE
#elif defined(WINCE)
#	define GLM_PLATFORM GLM_PLATFORM_WINCE
#elif defined(_WIN32)
#	define GLM_PLATFORM GLM_PLATFORM_WINDOWS
#elif defined(__native_client__)
#	define GLM_PLATFORM GLM_PLATFORM_CHROME_NACL
#elif defined(__ANDROID__)
#	define GLM_PLATFORM GLM_PLATFORM_ANDROID
#elif defined(__linux)
#	define GLM_PLATFORM GLM_PLATFORM_LINUX
#elif defined(__unix)
#	define GLM_PLATFORM GLM_PLATFORM_UNIX
#else
#	define GLM_PLATFORM GLM_PLATFORM_UNKNOWN
#endif//

///////////////////////////////////////////////////////////////////////////////////
// Compiler

#define GLM_COMPILER_UNKNOWN		0x00000000

// Intel
#define GLM_COMPILER_INTEL			0x00100000
#define GLM_COMPILER_INTEL14		0x00100040
#define GLM_COMPILER_INTEL15		0x00100050
#define GLM_COMPILER_INTEL16		0x00100060
#define GLM_COMPILER_INTEL17		0x00100070

// Visual C++ defines
#define GLM_COMPILER_VC				0x01000000
#define GLM_COMPILER_VC12			0x01000001
#define GLM_COMPILER_VC14			0x01000002
#define GLM_COMPILER_VC15			0x01000003
#define GLM_COMPILER_VC15_3			0x01000004
#define GLM_COMPILER_VC15_5			0x01000005
#define GLM_COMPILER_VC15_6			0x01000006
#define GLM_COMPILER_VC15_7			0x01000007
#define GLM_COMPILER_VC15_8			0x01000008
#define GLM_COMPILER_VC15_9			0x01000009
#define GLM_COMPILER_VC16			0x0100000A

// GCC defines
#define GLM_COMPILER_GCC			0x02000000
#define GLM_COMPILER_GCC46			0x020000D0
#define GLM_COMPILER_GCC47			0x020000E0
#define GLM_COMPILER_GCC48			0x020000F0
#define GLM_COMPILER_GCC49			0x02000100
#define GLM_COMPILER_GCC5			0x02000200
#define GLM_COMPILER_GCC6			0x02000300
#define GLM_COMPILER_GCC7			0x02000400
#define GLM_COMPILER_GCC8			0x02000500

// CUDA
#define GLM_COMPILER_CUDA			0x10000000
#define GLM_COMPILER_CUDA75			0x10000001
#define GLM_COMPILER_CUDA80			0x10000002
#define GLM_COMPILER_CUDA90			0x10000004

// SYCL
#define GLM_COMPILER_SYCL			0x00300000

// Clang
#define GLM_COMPILER_CLANG			0x20000000
#define GLM_COMPILER_CLANG34		0x20000050
#define GLM_COMPILER_CLANG35		0x20000060
#define GLM_COMPILER_CLANG36		0x20000070
#define GLM_COMPILER_CLANG37		0x20000080
#define GLM_COMPILER_CLANG38		0x20000090
#define GLM_COMPILER_CLANG39		0x200000A0
#define GLM_COMPILER_CLANG40		0x200000B0
#define GLM_COMPILER_CLANG41		0x200000C0
#define GLM_COMPILER_CLANG42		0x200000D0

// Build model
#define GLM_MODEL_32				0x00000010
#define GLM_MODEL_64				0x00000020

// Force generic C++ compiler
#ifdef GLM_FORCE_COMPILER_UNKNOWN
#	define GLM_COMPILER GLM_COMPILER_UNKNOWN

#elif defined(__INTEL_COMPILER)
#	if __INTEL_COMPILER >= 1700
#		define GLM_COMPILER GLM_COMPILER_INTEL17
#	elif __INTEL_COMPILER >= 1600
#		define GLM_COMPILER GLM_COMPILER_INTEL16
#	elif __INTEL_COMPILER >= 1500
#		define GLM_COMPILER GLM_COMPILER_INTEL15
#	elif __INTEL_COMPILER >= 1400
#		define GLM_COMPILER GLM_COMPILER_INTEL14
#	elif __INTEL_COMPILER < 1400
#		error "GLM requires ICC 2013 SP1 or newer"
#	endif

// CUDA
#elif defined(__CUDACC__)
#	if !defined(CUDA_VERSION) && !defined(GLM_FORCE_CUDA)
#		include <cuda.h>  // make sure version is defined since nvcc does not define it itself!
#	endif
#	if CUDA_VERSION >= 8000
#		define GLM_COMPILER GLM_COMPILER_CUDA80
#	elif CUDA_VERSION >= 7500
#		define GLM_COMPILER GLM_COMPILER_CUDA75
#	elif CUDA_VERSION >= 7000
#		define GLM_COMPILER GLM_COMPILER_CUDA70
#	elif CUDA_VERSION < 7000
#		error "GLM requires CUDA 7.0 or higher"
#	endif

// SYCL
#elif defined(__SYCL_DEVICE_ONLY__)
#	define GLM_COMPILER GLM_COMPILER_SYCL

// Clang
#elif defined(__clang__)
#	if defined(__apple_build_version__)
#		if (__clang_major__ < 6)
#			error "GLM requires Clang 3.4 / Apple Clang 6.0 or higher"
#		elif __clang_major__ == 6 && __clang_minor__ == 0
#			define GLM_COMPILER GLM_COMPILER_CLANG35
#		elif __clang_major__ == 6 && __clang_minor__ >= 1
#			define GLM_COMPILER GLM_COMPILER_CLANG36
#		elif __clang_major__ >= 7
#			define GLM_COMPILER GLM_COMPILER_CLANG37
#		endif
#	else
#		if ((__clang_major__ == 3) && (__clang_minor__ < 4)) || (__clang_major__ < 3)
#			error "GLM requires Clang 3.4 or higher"
#		elif __clang_major__ == 3 && __clang_minor__ == 4
#			define GLM_COMPILER GLM_COMPILER_CLANG34
#		elif __clang_major__ == 3 && __clang_minor__ == 5
#			define GLM_COMPILER GLM_COMPILER_CLANG35
#		elif __clang_major__ == 3 && __clang_minor__ == 6
#			define GLM_COMPILER GLM_COMPILER_CLANG36
#		elif __clang_major__ == 3 && __clang_minor__ == 7
#			define GLM_COMPILER GLM_COMPILER_CLANG37
#		elif __clang_major__ == 3 && __clang_minor__ == 8
#			define GLM_COMPILER GLM_COMPILER_CLANG38
#		elif __clang_major__ == 3 && __clang_minor__ >= 9
#			define GLM_COMPILER GLM_COMPILER_CLANG39
#		elif __clang_major__ == 4 && __clang_minor__ == 0
#			define GLM_COMPILER GLM_COMPILER_CLANG40
#		elif __clang_major__ == 4 && __clang_minor__ == 1
#			define GLM_COMPILER GLM_COMPILER_CLANG41
#		elif __clang_major__ == 4 && __clang_minor__ >= 2
#			define GLM_COMPILER GLM_COMPILER_CLANG42
#		elif __clang_major__ >= 4
#			define GLM_COMPILER GLM_COMPILER_CLANG42
#		endif
#	endif

// Visual C++
#elif defined(_MSC_VER)
#	if _MSC_VER >= 1920
#		define GLM_COMPILER GLM_COMPILER_VC16
#	elif _MSC_VER >= 1916
#		define GLM_COMPILER GLM_COMPILER_VC15_9
#	elif _MSC_VER >= 1915
#		define GLM_COMPILER GLM_COMPILER_VC15_8
#	elif _MSC_VER >= 1914
#		define GLM_COMPILER GLM_COMPILER_VC15_7
#	elif _MSC_VER >= 1913
#		define GLM_COMPILER GLM_COMPILER_VC15_6
#	elif _MSC_VER >= 1912
#		define GLM_COMPILER GLM_COMPILER_VC15_5
#	elif _MSC_VER >= 1911
#		define GLM_COMPILER GLM_COMPILER_VC15_3
#	elif _MSC_VER >= 1910
#		define GLM_COMPILER GLM_COMPILER_VC15
#	elif _MSC_VER >= 1900
#		define GLM_COMPILER GLM_COMPILER_VC14
#	elif _MSC_VER >= 1800
#		define GLM_COMPILER GLM_COMPILER_VC12
#	elif _MSC_VER < 1800
#		error "GLM requires Visual C++ 12 - 2013 or higher"
#	endif//_MSC_VER

// G++
#elif defined(__GNUC__) || defined(__MINGW32__)
#	if __GNUC__ >= 8
#		define GLM_COMPILER GLM_COMPILER_GCC8
#	elif __GNUC__ >= 7
#		define GLM_COMPILER GLM_COMPILER_GCC7
#	elif __GNUC__ >= 6
#		define GLM_COMPILER GLM_COMPILER_GCC6
#	elif __GNUC__ >= 5
#		define GLM_COMPILER GLM_COMPILER_GCC5
#	elif __GNUC__ == 4 && __GNUC_MINOR__ >= 9
#		define GLM_COMPILER GLM_COMPILER_GCC49
#	elif __GNUC__ == 4 && __GNUC_MINOR__ >= 8
#		define GLM_COMPILER GLM_COMPILER_GCC48
#	elif __GNUC__ == 4 && __GNUC_MINOR__ >= 7
#		define GLM_COMPILER GLM_COMPILER_GCC47
#	elif __GNUC__ == 4 && __GNUC_MINOR__ >= 6
#		define GLM_COMPILER GLM_COMPILER_GCC46
#	elif ((__GNUC__ == 4) && (__GNUC_MINOR__ < 6)) || (__GNUC__ < 4)
#		error "GLM requires GCC 4.6 or higher"
#	endif

#else
#	define GLM_COMPILER GLM_COMPILER_UNKNOWN
#endif

#ifndef GLM_COMPILER
#	error "GLM_COMPILER undefined, your compiler may not be supported by GLM. Add #define GLM_COMPILER 0 to ignore this message."
#endif//GLM_COMPILER

///////////////////////////////////////////////////////////////////////////////////
// Instruction sets

// User defines: GLM_FORCE_PURE GLM_FORCE_INTRINSICS GLM_FORCE_SSE2 GLM_FORCE_SSE3 GLM_FORCE_AVX GLM_FORCE_AVX2 GLM_FORCE_AVX2

#define GLM_ARCH_MIPS_BIT	  (0x10000000)
#define GLM_ARCH_PPC_BIT	  (0x20000000)
#define GLM_ARCH_ARM_BIT	  (0x40000000)
#define GLM_ARCH_ARMV8_BIT  (0x01000000)
#define GLM_ARCH_X86_BIT	  (0x80000000)

#define GLM_ARCH_SIMD_BIT	(0x00001000)

#define GLM_ARCH_NEON_BIT	(0x00000001)
#define GLM_ARCH_SSE_BIT	(0x00000002)
#define GLM_ARCH_SSE2_BIT	(0x00000004)
#define GLM_ARCH_SSE3_BIT	(0x00000008)
#define GLM_ARCH_SSSE3_BIT	(0x00000010)
#define GLM_ARCH_SSE41_BIT	(0x00000020)
#define GLM_ARCH_SSE42_BIT	(0x00000040)
#define GLM_ARCH_AVX_BIT	(0x00000080)
#define GLM_ARCH_AVX2_BIT	(0x00000100)

#define GLM_ARCH_UNKNOWN	(0)
#define GLM_ARCH_X86		(GLM_ARCH_X86_BIT)
#define GLM_ARCH_SSE		(GLM_ARCH_SSE_BIT | GLM_ARCH_SIMD_BIT | GLM_ARCH_X86)
#define GLM_ARCH_SSE2		(GLM_ARCH_SSE2_BIT | GLM_ARCH_SSE)
#define GLM_ARCH_SSE3		(GLM_ARCH_SSE3_BIT | GLM_ARCH_SSE2)
#define GLM_ARCH_SSSE3		(GLM_ARCH_SSSE3_BIT | GLM_ARCH_SSE3)
#define GLM_ARCH_SSE41		(GLM_ARCH_SSE41_BIT | GLM_ARCH_SSSE3)
#define GLM_ARCH_SSE42		(GLM_ARCH_SSE42_BIT | GLM_ARCH_SSE41)
#define GLM_ARCH_AVX		(GLM_ARCH_AVX_BIT | GLM_ARCH_SSE42)
#define GLM_ARCH_AVX2		(GLM_ARCH_AVX2_BIT | GLM_ARCH_AVX)
#define GLM_ARCH_ARM		(GLM_ARCH_ARM_BIT)
#define GLM_ARCH_ARMV8		(GLM_ARCH_NEON_BIT | GLM_ARCH_SIMD_BIT | GLM_ARCH_ARM | GLM_ARCH_ARMV8_BIT)
#define GLM_ARCH_NEON		(GLM_ARCH_NEON_BIT | GLM_ARCH_SIMD_BIT | GLM_ARCH_ARM)
#define GLM_ARCH_MIPS		(GLM_ARCH_MIPS_BIT)
#define GLM_ARCH_PPC		(GLM_ARCH_PPC_BIT)

#if defined(GLM_FORCE_ARCH_UNKNOWN) || defined(GLM_FORCE_PURE)
#	define GLM_ARCH GLM_ARCH_UNKNOWN
#elif defined(GLM_FORCE_NEON)
#	if __ARM_ARCH >= 8
#		define GLM_ARCH (GLM_ARCH_ARMV8)
#	else
#		define GLM_ARCH (GLM_ARCH_NEON)
#	endif
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_AVX2)
#	define GLM_ARCH (GLM_ARCH_AVX2)
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_AVX)
#	define GLM_ARCH (GLM_ARCH_AVX)
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_SSE42)
#	define GLM_ARCH (GLM_ARCH_SSE42)
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_SSE41)
#	define GLM_ARCH (GLM_ARCH_SSE41)
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_SSSE3)
#	define GLM_ARCH (GLM_ARCH_SSSE3)
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_SSE3)
#	define GLM_ARCH (GLM_ARCH_SSE3)
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_SSE2)
#	define GLM_ARCH (GLM_ARCH_SSE2)
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_SSE)
#	define GLM_ARCH (GLM_ARCH_SSE)
#	define GLM_FORCE_INTRINSICS
#elif defined(GLM_FORCE_INTRINSICS) && !defined(GLM_FORCE_XYZW_ONLY)
#	if defined(__AVX2__)
#		define GLM_ARCH (GLM_ARCH_AVX2)
#	elif defined(__AVX__)
#		define GLM_ARCH (GLM_ARCH_AVX)
#	elif defined(__SSE4_2__)
#		define GLM_ARCH (GLM_ARCH_SSE42)
#	elif defined(__SSE4_1__)
#		define GLM_ARCH (GLM_ARCH_SSE41)
#	elif defined(__SSSE3__)
#		define GLM_ARCH (GLM_ARCH_SSSE3)
#	elif defined(__SSE3__)
#		define GLM_ARCH (GLM_ARCH_SSE3)
#	elif defined(__SSE2__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86_FP)
#		define GLM_ARCH (GLM_ARCH_SSE2)
#	elif defined(__i386__)
#		define GLM_ARCH (GLM_ARCH_X86)
#	elif defined(__ARM_ARCH) && (__ARM_ARCH >= 8)
#		define GLM_ARCH (GLM_ARCH_ARMV8)
#	elif defined(__ARM_NEON)
#		define GLM_ARCH (GLM_ARCH_ARM | GLM_ARCH_NEON)
#	elif defined(__arm__ ) || defined(_M_ARM)
#		define GLM_ARCH (GLM_ARCH_ARM)
#	elif defined(__mips__ )
#		define GLM_ARCH (GLM_ARCH_MIPS)
#	elif defined(__powerpc__ ) || defined(_M_PPC)
#		define GLM_ARCH (GLM_ARCH_PPC)
#	else
#		define GLM_ARCH (GLM_ARCH_UNKNOWN)
#	endif
#else
#	if defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86) || defined(__i386__)
#		define GLM_ARCH (GLM_ARCH_X86)
#	elif defined(__arm__) || defined(_M_ARM)
#		define GLM_ARCH (GLM_ARCH_ARM)
#	elif defined(__powerpc__) || defined(_M_PPC)
#		define GLM_ARCH (GLM_ARCH_PPC)
#	elif defined(__mips__)
#		define GLM_ARCH (GLM_ARCH_MIPS)
#	else
#		define GLM_ARCH (GLM_ARCH_UNKNOWN)
#	endif
#endif

#if GLM_ARCH & GLM_ARCH_AVX2_BIT
#	include <immintrin.h>
#elif GLM_ARCH & GLM_ARCH_AVX_BIT
#	include <immintrin.h>
#elif GLM_ARCH & GLM_ARCH_SSE42_BIT
#	if GLM_COMPILER & GLM_COMPILER_CLANG
#		include <popcntintrin.h>
#	endif
#	include <nmmintrin.h>
#elif GLM_ARCH & GLM_ARCH_SSE41_BIT
#	include <smmintrin.h>
#elif GLM_ARCH & GLM_ARCH_SSSE3_BIT
#	include <tmmintrin.h>
#elif GLM_ARCH & GLM_ARCH_SSE3_BIT
#	include <pmmintrin.h>
#elif GLM_ARCH & GLM_ARCH_SSE2_BIT
#	include <emmintrin.h>
#elif GLM_ARCH & GLM_ARCH_NEON_BIT
#	include "neon.h"
#endif//GLM_ARCH

#if GLM_ARCH & GLM_ARCH_SSE2_BIT
	typedef __m128			glm_f32vec4;
	typedef __m128i			glm_i32vec4;
	typedef __m128i			glm_u32vec4;
	typedef __m128d			glm_f64vec2;
	typedef __m128i			glm_i64vec2;
	typedef __m128i			glm_u64vec2;

	typedef glm_f32vec4		glm_vec4;
	typedef glm_i32vec4		glm_ivec4;
	typedef glm_u32vec4		glm_uvec4;
	typedef glm_f64vec2		glm_dvec2;
#endif

#if GLM_ARCH & GLM_ARCH_AVX_BIT
	typedef __m256d			glm_f64vec4;
	typedef glm_f64vec4		glm_dvec4;
#endif

#if GLM_ARCH & GLM_ARCH_AVX2_BIT
	typedef __m256i			glm_i64vec4;
	typedef __m256i			glm_u64vec4;
#endif

#if GLM_ARCH & GLM_ARCH_NEON_BIT
	typedef float32x4_t			glm_f32vec4;
	typedef int32x4_t			glm_i32vec4;
	typedef uint32x4_t			glm_u32vec4;
#endif


///////////////////////////////////////////////////////////////////////////////////
// Build model

#if defined(_M_ARM64) || defined(__LP64__) || defined(_M_X64) || defined(__ppc64__) || defined(__x86_64__)
#	define GLM_MODEL	GLM_MODEL_64
#elif defined(__i386__) || defined(__ppc__) || defined(__ILP32__) || defined(_M_ARM)
#	define GLM_MODEL	GLM_MODEL_32
#else
#	define GLM_MODEL	GLM_MODEL_32
#endif//

#if !defined(GLM_MODEL) && GLM_COMPILER != 0
#	error "GLM_MODEL undefined, your compiler may not be supported by GLM. Add #define GLM_MODEL 0 to ignore this message."
#endif//GLM_MODEL

///////////////////////////////////////////////////////////////////////////////////
// C++ Version

// User defines: GLM_FORCE_CXX98, GLM_FORCE_CXX03, GLM_FORCE_CXX11, GLM_FORCE_CXX14, GLM_FORCE_CXX17, GLM_FORCE_CXX2A

#define GLM_LANG_CXX98_FLAG			(1 << 1)
#define GLM_LANG_CXX03_FLAG			(1 << 2)
#define GLM_LANG_CXX0X_FLAG			(1 << 3)
#define GLM_LANG_CXX11_FLAG			(1 << 4)
#define GLM_LANG_CXX14_FLAG			(1 << 5)
#define GLM_LANG_CXX17_FLAG			(1 << 6)
#define GLM_LANG_CXX2A_FLAG			(1 << 7)
#define GLM_LANG_CXXMS_FLAG			(1 << 8)
#define GLM_LANG_CXXGNU_FLAG		(1 << 9)

#define GLM_LANG_CXX98			GLM_LANG_CXX98_FLAG
#define GLM_LANG_CXX03			(GLM_LANG_CXX98 | GLM_LANG_CXX03_FLAG)
#define GLM_LANG_CXX0X			(GLM_LANG_CXX03 | GLM_LANG_CXX0X_FLAG)
#define GLM_LANG_CXX11			(GLM_LANG_CXX0X | GLM_LANG_CXX11_FLAG)
#define GLM_LANG_CXX14			(GLM_LANG_CXX11 | GLM_LANG_CXX14_FLAG)
#define GLM_LANG_CXX17			(GLM_LANG_CXX14 | GLM_LANG_CXX17_FLAG)
#define GLM_LANG_CXX2A			(GLM_LANG_CXX17 | GLM_LANG_CXX2A_FLAG)
#define GLM_LANG_CXXMS			GLM_LANG_CXXMS_FLAG
#define GLM_LANG_CXXGNU			GLM_LANG_CXXGNU_FLAG

#if (defined(_MSC_EXTENSIONS))
#	define GLM_LANG_EXT GLM_LANG_CXXMS_FLAG
#elif ((GLM_COMPILER & (GLM_COMPILER_CLANG | GLM_COMPILER_GCC)) && (GLM_ARCH & GLM_ARCH_SIMD_BIT))
#	define GLM_LANG_EXT GLM_LANG_CXXMS_FLAG
#else
#	define GLM_LANG_EXT 0
#endif

#if (defined(GLM_FORCE_CXX_UNKNOWN))
#	define GLM_LANG 0
#elif defined(GLM_FORCE_CXX2A)
#	define GLM_LANG (GLM_LANG_CXX2A | GLM_LANG_EXT)
#	define GLM_LANG_STL11_FORCED
#elif defined(GLM_FORCE_CXX17)
#	define GLM_LANG (GLM_LANG_CXX17 | GLM_LANG_EXT)
#	define GLM_LANG_STL11_FORCED
#elif defined(GLM_FORCE_CXX14)
#	define GLM_LANG (GLM_LANG_CXX14 | GLM_LANG_EXT)
#	define GLM_LANG_STL11_FORCED
#elif defined(GLM_FORCE_CXX11)
#	define GLM_LANG (GLM_LANG_CXX11 | GLM_LANG_EXT)
#	define GLM_LANG_STL11_FORCED
#elif defined(GLM_FORCE_CXX03)
#	define GLM_LANG (GLM_LANG_CXX03 | GLM_LANG_EXT)
#elif defined(GLM_FORCE_CXX98)
#	define GLM_LANG (GLM_LANG_CXX98 | GLM_LANG_EXT)
#else
#	if GLM_COMPILER & GLM_COMPILER_VC && defined(_MSVC_LANG)
#		if GLM_COMPILER >= GLM_COMPILER_VC15_7
#			define GLM_LANG_PLATFORM _MSVC_LANG
#		elif GLM_COMPILER >= GLM_COMPILER_VC15
#			if _MSVC_LANG > 201402L
#				define GLM_LANG_PLATFORM 201402L
#			else
#				define GLM_LANG_PLATFORM _MSVC_LANG
#			endif
#		else
#			define GLM_LANG_PLATFORM 0
#		endif
#	else
#		define GLM_LANG_PLATFORM 0
#	endif

#	if __cplusplus > 201703L || GLM_LANG_PLATFORM > 201703L
#		define GLM_LANG (GLM_LANG_CXX2A | GLM_LANG_EXT)
#	elif __cplusplus == 201703L || GLM_LANG_PLATFORM == 201703L
#		define GLM_LANG (GLM_LANG_CXX17 | GLM_LANG_EXT)
#	elif __cplusplus == 201402L || __cplusplus == 201500L || GLM_LANG_PLATFORM == 201402L
#		define GLM_LANG (GLM_LANG_CXX14 | GLM_LANG_EXT)
#	elif __cplusplus == 201103L || GLM_LANG_PLATFORM == 201103L
#		define GLM_LANG (GLM_LANG_CXX11 | GLM_LANG_EXT)
#	elif defined(__INTEL_CXX11_MODE__) || defined(_MSC_VER) || defined(__GXX_EXPERIMENTAL_CXX0X__)
#		define GLM_LANG (GLM_LANG_CXX0X | GLM_LANG_EXT)
#	elif __cplusplus == 199711L
#		define GLM_LANG (GLM_LANG_CXX98 | GLM_LANG_EXT)
#	else
#		define GLM_LANG (0 | GLM_LANG_EXT)
#	endif
#endif

///////////////////////////////////////////////////////////////////////////////////
// Has of C++ features

// http://clang.llvm.org/cxx_status.html
// http://gcc.gnu.org/projects/cxx0x.html
// http://msdn.microsoft.com/en-us/library/vstudio/hh567368(v=vs.120).aspx

// Android has multiple STLs but C++11 STL detection doesn't always work #284 #564
#if GLM_PLATFORM == GLM_PLATFORM_ANDROID && !defined(GLM_LANG_STL11_FORCED)
#	define GLM_HAS_CXX11_STL 0
#elif GLM_COMPILER & GLM_COMPILER_CLANG
#	if (defined(_LIBCPP_VERSION) || (GLM_LANG & GLM_LANG_CXX11_FLAG) || defined(GLM_LANG_STL11_FORCED))
#		define GLM_HAS_CXX11_STL 1
#	else
#		define GLM_HAS_CXX11_STL 0
#	endif
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_CXX11_STL 1
#else
#	define GLM_HAS_CXX11_STL ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_GCC) && (GLM_COMPILER >= GLM_COMPILER_GCC48)) || \
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC12)) || \
		((GLM_PLATFORM != GLM_PLATFORM_WINDOWS) && (GLM_COMPILER & GLM_COMPILER_INTEL) && (GLM_COMPILER >= GLM_COMPILER_INTEL15))))
#endif

// N1720
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_STATIC_ASSERT __has_feature(cxx_static_assert)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_STATIC_ASSERT 1
#else
#	define GLM_HAS_STATIC_ASSERT ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_CUDA)) || \
		((GLM_COMPILER & GLM_COMPILER_VC))))
#endif

// N1988
#if GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_EXTENDED_INTEGER_TYPE 1
#else
#	define GLM_HAS_EXTENDED_INTEGER_TYPE (\
		((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (GLM_COMPILER & GLM_COMPILER_VC)) || \
		((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (GLM_COMPILER & GLM_COMPILER_CUDA)) || \
		((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (GLM_COMPILER & GLM_COMPILER_CLANG)))
#endif

// N2672 Initializer lists http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2672.htm
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_INITIALIZER_LISTS __has_feature(cxx_generalized_initializers)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_INITIALIZER_LISTS 1
#else
#	define GLM_HAS_INITIALIZER_LISTS ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC15)) || \
		((GLM_COMPILER & GLM_COMPILER_INTEL) && (GLM_COMPILER >= GLM_COMPILER_INTEL14)) || \
		((GLM_COMPILER & GLM_COMPILER_CUDA))))
#endif

// N2544 Unrestricted unions http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2544.pdf
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_UNRESTRICTED_UNIONS __has_feature(cxx_unrestricted_unions)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_UNRESTRICTED_UNIONS 1
#else
#	define GLM_HAS_UNRESTRICTED_UNIONS (GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		(GLM_COMPILER & GLM_COMPILER_VC) || \
		((GLM_COMPILER & GLM_COMPILER_CUDA)))
#endif

// N2346
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_DEFAULTED_FUNCTIONS __has_feature(cxx_defaulted_functions)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_DEFAULTED_FUNCTIONS 1
#else
#	define GLM_HAS_DEFAULTED_FUNCTIONS ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC12)) || \
		((GLM_COMPILER & GLM_COMPILER_INTEL)) || \
		(GLM_COMPILER & GLM_COMPILER_CUDA)))
#endif

// N2118
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_RVALUE_REFERENCES __has_feature(cxx_rvalue_references)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_RVALUE_REFERENCES 1
#else
#	define GLM_HAS_RVALUE_REFERENCES ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_VC)) || \
		((GLM_COMPILER & GLM_COMPILER_CUDA))))
#endif

// N2437 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2437.pdf
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_EXPLICIT_CONVERSION_OPERATORS __has_feature(cxx_explicit_conversions)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_EXPLICIT_CONVERSION_OPERATORS 1
#else
#	define GLM_HAS_EXPLICIT_CONVERSION_OPERATORS ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_INTEL) && (GLM_COMPILER >= GLM_COMPILER_INTEL14)) || \
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC12)) || \
		((GLM_COMPILER & GLM_COMPILER_CUDA))))
#endif

// N2258 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2258.pdf
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_TEMPLATE_ALIASES __has_feature(cxx_alias_templates)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_TEMPLATE_ALIASES 1
#else
#	define GLM_HAS_TEMPLATE_ALIASES ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_INTEL)) || \
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC12)) || \
		((GLM_COMPILER & GLM_COMPILER_CUDA))))
#endif

// N2930 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2009/n2930.html
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_RANGE_FOR __has_feature(cxx_range_for)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_RANGE_FOR 1
#else
#	define GLM_HAS_RANGE_FOR ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_INTEL)) || \
		((GLM_COMPILER & GLM_COMPILER_VC)) || \
		((GLM_COMPILER & GLM_COMPILER_CUDA))))
#endif

// N2341 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2341.pdf
#if GLM_COMPILER & GLM_COMPILER_CLANG
#	define GLM_HAS_ALIGNOF __has_feature(cxx_alignas)
#elif GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_ALIGNOF 1
#else
#	define GLM_HAS_ALIGNOF ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_INTEL) && (GLM_COMPILER >= GLM_COMPILER_INTEL15)) || \
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC14)) || \
		((GLM_COMPILER & GLM_COMPILER_CUDA))))
#endif

// N2235 Generalized Constant Expressions http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2235.pdf
// N3652 Extended Constant Expressions http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3652.html
#if (GLM_ARCH & GLM_ARCH_SIMD_BIT) // Compiler SIMD intrinsics don't support constexpr...
#	define GLM_HAS_CONSTEXPR 0
#elif (GLM_COMPILER & GLM_COMPILER_CLANG)
#	define GLM_HAS_CONSTEXPR __has_feature(cxx_relaxed_constexpr)
#elif (GLM_LANG & GLM_LANG_CXX14_FLAG)
#	define GLM_HAS_CONSTEXPR 1
#else
#	define GLM_HAS_CONSTEXPR ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && GLM_HAS_INITIALIZER_LISTS && (\
		((GLM_COMPILER & GLM_COMPILER_INTEL) && (GLM_COMPILER >= GLM_COMPILER_INTEL17)) || \
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC15))))
#endif

#if GLM_HAS_CONSTEXPR
#	define GLM_CONSTEXPR constexpr
#else
#	define GLM_CONSTEXPR
#endif

//
#if GLM_HAS_CONSTEXPR
# if (GLM_COMPILER & GLM_COMPILER_CLANG)
#	if __has_feature(cxx_if_constexpr)
#		define GLM_HAS_IF_CONSTEXPR 1
#	else
# 		define GLM_HAS_IF_CONSTEXPR 0
#	endif
# elif (GLM_LANG & GLM_LANG_CXX17_FLAG)
# 	define GLM_HAS_IF_CONSTEXPR 1
# else
# 	define GLM_HAS_IF_CONSTEXPR 0
# endif
#else
#	define GLM_HAS_IF_CONSTEXPR 0
#endif

#if GLM_HAS_IF_CONSTEXPR
# 	define GLM_IF_CONSTEXPR if constexpr
#else
#	define GLM_IF_CONSTEXPR if
#endif

//
#if GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_ASSIGNABLE 1
#else
#	define GLM_HAS_ASSIGNABLE ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC15)) || \
		((GLM_COMPILER & GLM_COMPILER_GCC) && (GLM_COMPILER >= GLM_COMPILER_GCC49))))
#endif

//
#define GLM_HAS_TRIVIAL_QUERIES 0

//
#if GLM_LANG & GLM_LANG_CXX11_FLAG
#	define GLM_HAS_MAKE_SIGNED 1
#else
#	define GLM_HAS_MAKE_SIGNED ((GLM_LANG & GLM_LANG_CXX0X_FLAG) && (\
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC12)) || \
		((GLM_COMPILER & GLM_COMPILER_CUDA))))
#endif

//
#if defined(GLM_FORCE_INTRINSICS)
#	define GLM_HAS_BITSCAN_WINDOWS ((GLM_PLATFORM & GLM_PLATFORM_WINDOWS) && (\
		((GLM_COMPILER & GLM_COMPILER_INTEL)) || \
		((GLM_COMPILER & GLM_COMPILER_VC) && (GLM_COMPILER >= GLM_COMPILER_VC14) && (GLM_ARCH & GLM_ARCH_X86_BIT))))
#else
#	define GLM_HAS_BITSCAN_WINDOWS 0
#endif

///////////////////////////////////////////////////////////////////////////////////
// OpenMP
#ifdef _OPENMP
#	if GLM_COMPILER & GLM_COMPILER_GCC
#		if GLM_COMPILER >= GLM_COMPILER_GCC61
#			define GLM_HAS_OPENMP 45
#		elif GLM_COMPILER >= GLM_COMPILER_GCC49
#			define GLM_HAS_OPENMP 40
#		elif GLM_COMPILER >= GLM_COMPILER_GCC47
#			define GLM_HAS_OPENMP 31
#		else
#			define GLM_HAS_OPENMP 0
#		endif
#	elif GLM_COMPILER & GLM_COMPILER_CLANG
#		if GLM_COMPILER >= GLM_COMPILER_CLANG38
#			define GLM_HAS_OPENMP 31
#		else
#			define GLM_HAS_OPENMP 0
#		endif
#	elif GLM_COMPILER & GLM_COMPILER_VC
#		define GLM_HAS_OPENMP 20
#	elif GLM_COMPILER & GLM_COMPILER_INTEL
#		if GLM_COMPILER >= GLM_COMPILER_INTEL16
#			define GLM_HAS_OPENMP 40
#		else
#			define GLM_HAS_OPENMP 0
#		endif
#	else
#		define GLM_HAS_OPENMP 0
#	endif
#else
#	define GLM_HAS_OPENMP 0
#endif

///////////////////////////////////////////////////////////////////////////////////
// nullptr

#if GLM_LANG & GLM_LANG_CXX0X_FLAG
#	define GLM_CONFIG_NULLPTR GLM_ENABLE
#else
#	define GLM_CONFIG_NULLPTR GLM_DISABLE
#endif

#if GLM_CONFIG_NULLPTR == GLM_ENABLE
#	define GLM_NULLPTR nullptr
#else
#	define GLM_NULLPTR 0
#endif

///////////////////////////////////////////////////////////////////////////////////
// Static assert

#if GLM_HAS_STATIC_ASSERT
#	define GLM_STATIC_ASSERT(x, message) static_assert(x, message)
#elif GLM_COMPILER & GLM_COMPILER_VC
#	define GLM_STATIC_ASSERT(x, message) typedef char __CASSERT__##__LINE__[(x) ? 1 : -1]
#else
#	define GLM_STATIC_ASSERT(x, message) assert(x)
#endif//GLM_LANG

///////////////////////////////////////////////////////////////////////////////////
// Qualifiers

#if GLM_COMPILER & GLM_COMPILER_CUDA
#	define GLM_CUDA_FUNC_DEF __device__ __host__
#	define GLM_CUDA_FUNC_DECL __device__ __host__
#else
#	define GLM_CUDA_FUNC_DEF
#	define GLM_CUDA_FUNC_DECL
#endif

#if defined(GLM_FORCE_INLINE)
#	if GLM_COMPILER & GLM_COMPILER_VC
#		define GLM_INLINE __forceinline
#		define GLM_NEVER_INLINE __declspec((noinline))
#	elif GLM_COMPILER & (GLM_COMPILER_GCC | GLM_COMPILER_CLANG)
#		define GLM_INLINE inline __attribute__((__always_inline__))
#		define GLM_NEVER_INLINE __attribute__((__noinline__))
#	elif GLM_COMPILER & GLM_COMPILER_CUDA
#		define GLM_INLINE __forceinline__
#		define GLM_NEVER_INLINE __noinline__
#	else
#		define GLM_INLINE inline
#		define GLM_NEVER_INLINE
#	endif//GLM_COMPILER
#else
#	define GLM_INLINE inline
#	define GLM_NEVER_INLINE
#endif//defined(GLM_FORCE_INLINE)

#define GLM_FUNC_DECL GLM_CUDA_FUNC_DECL
#define GLM_FUNC_QUALIFIER GLM_CUDA_FUNC_DEF GLM_INLINE

///////////////////////////////////////////////////////////////////////////////////
// Swizzle operators

// User defines: GLM_FORCE_SWIZZLE

#define GLM_SWIZZLE_DISABLED		0
#define GLM_SWIZZLE_OPERATOR		1
#define GLM_SWIZZLE_FUNCTION		2

#if defined(GLM_FORCE_XYZW_ONLY)
#	undef GLM_FORCE_SWIZZLE
#endif

#if defined(GLM_SWIZZLE)
#	pragma message("GLM: GLM_SWIZZLE is deprecated, use GLM_FORCE_SWIZZLE instead.")
#	define GLM_FORCE_SWIZZLE
#endif

#if defined(GLM_FORCE_SWIZZLE) && (GLM_LANG & GLM_LANG_CXXMS_FLAG)
#	define GLM_CONFIG_SWIZZLE GLM_SWIZZLE_OPERATOR
#elif defined(GLM_FORCE_SWIZZLE)
#	define GLM_CONFIG_SWIZZLE GLM_SWIZZLE_FUNCTION
#else
#	define GLM_CONFIG_SWIZZLE GLM_SWIZZLE_DISABLED
#endif

///////////////////////////////////////////////////////////////////////////////////
// Allows using not basic types as genType

// #define GLM_FORCE_UNRESTRICTED_GENTYPE

#ifdef GLM_FORCE_UNRESTRICTED_GENTYPE
#	define GLM_CONFIG_UNRESTRICTED_GENTYPE GLM_ENABLE
#else
#	define GLM_CONFIG_UNRESTRICTED_GENTYPE GLM_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Clip control, define GLM_FORCE_DEPTH_ZERO_TO_ONE before including GLM
// to use a clip space between 0 to 1.
// Coordinate system, define GLM_FORCE_LEFT_HANDED before including GLM
// to use left handed coordinate system by default.

#define GLM_CLIP_CONTROL_ZO_BIT		(1 << 0) // ZERO_TO_ONE
#define GLM_CLIP_CONTROL_NO_BIT		(1 << 1) // NEGATIVE_ONE_TO_ONE
#define GLM_CLIP_CONTROL_LH_BIT		(1 << 2) // LEFT_HANDED, For DirectX, Metal, Vulkan
#define GLM_CLIP_CONTROL_RH_BIT		(1 << 3) // RIGHT_HANDED, For OpenGL, default in GLM

#define GLM_CLIP_CONTROL_LH_ZO (GLM_CLIP_CONTROL_LH_BIT | GLM_CLIP_CONTROL_ZO_BIT)
#define GLM_CLIP_CONTROL_LH_NO (GLM_CLIP_CONTROL_LH_BIT | GLM_CLIP_CONTROL_NO_BIT)
#define GLM_CLIP_CONTROL_RH_ZO (GLM_CLIP_CONTROL_RH_BIT | GLM_CLIP_CONTROL_ZO_BIT)
#define GLM_CLIP_CONTROL_RH_NO (GLM_CLIP_CONTROL_RH_BIT | GLM_CLIP_CONTROL_NO_BIT)

#ifdef GLM_FORCE_DEPTH_ZERO_TO_ONE
#	ifdef GLM_FORCE_LEFT_HANDED
#		define GLM_CONFIG_CLIP_CONTROL GLM_CLIP_CONTROL_LH_ZO
#	else
#		define GLM_CONFIG_CLIP_CONTROL GLM_CLIP_CONTROL_RH_ZO
#	endif
#else
#	ifdef GLM_FORCE_LEFT_HANDED
#		define GLM_CONFIG_CLIP_CONTROL GLM_CLIP_CONTROL_LH_NO
#	else
#		define GLM_CONFIG_CLIP_CONTROL GLM_CLIP_CONTROL_RH_NO
#	endif
#endif

///////////////////////////////////////////////////////////////////////////////////
// Qualifiers

#if (GLM_COMPILER & GLM_COMPILER_VC) || ((GLM_COMPILER & GLM_COMPILER_INTEL) && (GLM_PLATFORM & GLM_PLATFORM_WINDOWS))
#	define GLM_DEPRECATED __declspec(deprecated)
#	define GLM_ALIGNED_TYPEDEF(type, name, alignment) typedef __declspec(align(alignment)) type name
#elif GLM_COMPILER & (GLM_COMPILER_GCC | GLM_COMPILER_CLANG | GLM_COMPILER_INTEL)
#	define GLM_DEPRECATED __attribute__((__deprecated__))
#	define GLM_ALIGNED_TYPEDEF(type, name, alignment) typedef type name __attribute__((aligned(alignment)))
#elif GLM_COMPILER & GLM_COMPILER_CUDA
#	define GLM_DEPRECATED
#	define GLM_ALIGNED_TYPEDEF(type, name, alignment) typedef type name __align__(x)
#else
#	define GLM_DEPRECATED
#	define GLM_ALIGNED_TYPEDEF(type, name, alignment) typedef type name
#endif

///////////////////////////////////////////////////////////////////////////////////

#ifdef GLM_FORCE_EXPLICIT_CTOR
#	define GLM_EXPLICIT explicit
#else
#	define GLM_EXPLICIT
#endif

///////////////////////////////////////////////////////////////////////////////////
// SYCL
/*
#if GLM_COMPILER==GLM_COMPILER_SYCL

#include <CL/sycl.hpp>
#include <limits>

namespace glm {
namespace std {
	// Import SYCL's functions into the namespace glm::std to force their usages.
	// It's important to use the math built-in function (sin, exp, ...)
	// of SYCL instead the std ones.
	using namespace cl::sycl;

	///////////////////////////////////////////////////////////////////////////////
	// Import some "harmless" std's stuffs used by glm into
	// the new glm::std namespace.
	template<typename T>
	using numeric_limits = ::std::numeric_limits<T>;

	using ::std::size_t;

	using ::std::uint8_t;
	using ::std::uint16_t;
	using ::std::uint32_t;
	using ::std::uint64_t;

	using ::std::int8_t;
	using ::std::int16_t;
	using ::std::int32_t;
	using ::std::int64_t;

	using ::std::make_unsigned;
	///////////////////////////////////////////////////////////////////////////////
} //namespace std
} //namespace glm

#endif
*/
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Length type: all length functions returns a length_t type.
// When GLM_FORCE_SIZE_T_LENGTH is defined, length_t is a typedef of size_t otherwise
// length_t is a typedef of int like GLSL defines it.

#define GLM_LENGTH_INT		1
#define GLM_LENGTH_SIZE_T	2

#ifdef GLM_FORCE_SIZE_T_LENGTH
#	define GLM_CONFIG_LENGTH_TYPE		GLM_LENGTH_SIZE_T
#else
#	define GLM_CONFIG_LENGTH_TYPE		GLM_LENGTH_INT
#endif

namespace glm
{
	using std::size_t;
#	if GLM_CONFIG_LENGTH_TYPE == GLM_LENGTH_SIZE_T
		typedef size_t length_t;
#	else
		typedef int length_t;
#	endif
}//namespace glm

///////////////////////////////////////////////////////////////////////////////////
// constexpr

#if GLM_HAS_CONSTEXPR
#	define GLM_CONFIG_CONSTEXP GLM_ENABLE

	namespace glm
	{
		template<typename T, std::size_t N>
		constexpr std::size_t countof(T const (&)[N])
		{
			return N;
		}
	}//namespace glm
#	define GLM_COUNTOF(arr) glm::countof(arr)
#elif defined(_MSC_VER)
#	define GLM_CONFIG_CONSTEXP GLM_DISABLE

#	define GLM_COUNTOF(arr) _countof(arr)
#else
#	define GLM_CONFIG_CONSTEXP GLM_DISABLE

#	define GLM_COUNTOF(arr) sizeof(arr) / sizeof(arr[0])
#endif

///////////////////////////////////////////////////////////////////////////////////
// uint

namespace glm{
namespace detail
{
	template<typename T>
	struct is_int
	{
		enum test {value = 0};
	};

	template<>
	struct is_int<unsigned int>
	{
		enum test {value = ~0};
	};

	template<>
	struct is_int<signed int>
	{
		enum test {value = ~0};
	};
}//namespace detail

	typedef unsigned int	uint;
}//namespace glm

///////////////////////////////////////////////////////////////////////////////////
// 64-bit int

#if GLM_HAS_EXTENDED_INTEGER_TYPE
#	include <cstdint>
#endif

namespace glm{
namespace detail
{
#	if GLM_HAS_EXTENDED_INTEGER_TYPE
		typedef std::uint64_t						uint64;
		typedef std::int64_t						int64;
#	elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) // C99 detected, 64 bit types available
		typedef uint64_t							uint64;
		typedef int64_t								int64;
#	elif GLM_COMPILER & GLM_COMPILER_VC
		typedef unsigned __int64					uint64;
		typedef signed __int64						int64;
#	elif GLM_COMPILER & GLM_COMPILER_GCC
#		pragma GCC diagnostic ignored "-Wlong-long"
		__extension__ typedef unsigned long long	uint64;
		__extension__ typedef signed long long		int64;
#	elif (GLM_COMPILER & GLM_COMPILER_CLANG)
#		pragma clang diagnostic ignored "-Wc++11-long-long"
		typedef unsigned long long					uint64;
		typedef signed long long					int64;
#	else//unknown compiler
		typedef unsigned long long					uint64;
		typedef signed long long					int64;
#	endif
}//namespace detail
}//namespace glm

///////////////////////////////////////////////////////////////////////////////////
// make_unsigned

#if GLM_HAS_MAKE_SIGNED
#	include <type_traits>

namespace glm{
namespace detail
{
	using std::make_unsigned;
}//namespace detail
}//namespace glm

#else

namespace glm{
namespace detail
{
	template<typename genType>
	struct make_unsigned
	{};

	template<>
	struct make_unsigned<char>
	{
		typedef unsigned char type;
	};

	template<>
	struct make_unsigned<signed char>
	{
		typedef unsigned char type;
	};

	template<>
	struct make_unsigned<short>
	{
		typedef unsigned short type;
	};

	template<>
	struct make_unsigned<int>
	{
		typedef unsigned int type;
	};

	template<>
	struct make_unsigned<long>
	{
		typedef unsigned long type;
	};

	template<>
	struct make_unsigned<int64>
	{
		typedef uint64 type;
	};

	template<>
	struct make_unsigned<unsigned char>
	{
		typedef unsigned char type;
	};

	template<>
	struct make_unsigned<unsigned short>
	{
		typedef unsigned short type;
	};

	template<>
	struct make_unsigned<unsigned int>
	{
		typedef unsigned int type;
	};

	template<>
	struct make_unsigned<unsigned long>
	{
		typedef unsigned long type;
	};

	template<>
	struct make_unsigned<uint64>
	{
		typedef uint64 type;
	};
}//namespace detail
}//namespace glm
#endif

///////////////////////////////////////////////////////////////////////////////////
// Only use x, y, z, w as vector type components

#ifdef GLM_FORCE_XYZW_ONLY
#	define GLM_CONFIG_XYZW_ONLY GLM_ENABLE
#else
#	define GLM_CONFIG_XYZW_ONLY GLM_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Configure the use of defaulted initialized types

#define GLM_CTOR_INIT_DISABLE		0
#define GLM_CTOR_INITIALIZER_LIST	1
#define GLM_CTOR_INITIALISATION		2

#if defined(GLM_FORCE_CTOR_INIT) && GLM_HAS_INITIALIZER_LISTS
#	define GLM_CONFIG_CTOR_INIT GLM_CTOR_INITIALIZER_LIST
#elif defined(GLM_FORCE_CTOR_INIT) && !GLM_HAS_INITIALIZER_LISTS
#	define GLM_CONFIG_CTOR_INIT GLM_CTOR_INITIALISATION
#else
#	define GLM_CONFIG_CTOR_INIT GLM_CTOR_INIT_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Use SIMD instruction sets

#if GLM_HAS_ALIGNOF && (GLM_LANG & GLM_LANG_CXXMS_FLAG) && (GLM_ARCH & GLM_ARCH_SIMD_BIT)
#	define GLM_CONFIG_SIMD GLM_ENABLE
#else
#	define GLM_CONFIG_SIMD GLM_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Configure the use of defaulted function

#if GLM_HAS_DEFAULTED_FUNCTIONS && GLM_CONFIG_CTOR_INIT == GLM_CTOR_INIT_DISABLE
#	define GLM_CONFIG_DEFAULTED_FUNCTIONS GLM_ENABLE
#	define GLM_DEFAULT = default
#else
#	define GLM_CONFIG_DEFAULTED_FUNCTIONS GLM_DISABLE
#	define GLM_DEFAULT
#endif

///////////////////////////////////////////////////////////////////////////////////
// Configure the use of aligned gentypes

#ifdef GLM_FORCE_ALIGNED // Legacy define
#	define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif

#ifdef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#	define GLM_FORCE_ALIGNED_GENTYPES
#endif

#if GLM_HAS_ALIGNOF && (GLM_LANG & GLM_LANG_CXXMS_FLAG) && (defined(GLM_FORCE_ALIGNED_GENTYPES) || (GLM_CONFIG_SIMD == GLM_ENABLE))
#	define GLM_CONFIG_ALIGNED_GENTYPES GLM_ENABLE
#else
#	define GLM_CONFIG_ALIGNED_GENTYPES GLM_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Configure the use of anonymous structure as implementation detail

#if ((GLM_CONFIG_SIMD == GLM_ENABLE) || (GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_OPERATOR) || (GLM_CONFIG_ALIGNED_GENTYPES == GLM_ENABLE))
#	define GLM_CONFIG_ANONYMOUS_STRUCT GLM_ENABLE
#else
#	define GLM_CONFIG_ANONYMOUS_STRUCT GLM_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Silent warnings

#ifdef GLM_FORCE_SILENT_WARNINGS
#	define GLM_SILENT_WARNINGS GLM_ENABLE
#else
#	define GLM_SILENT_WARNINGS GLM_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Precision

#define GLM_HIGHP		1
#define GLM_MEDIUMP		2
#define GLM_LOWP		3

#if defined(GLM_FORCE_PRECISION_HIGHP_BOOL) || defined(GLM_PRECISION_HIGHP_BOOL)
#	define GLM_CONFIG_PRECISION_BOOL		GLM_HIGHP
#elif defined(GLM_FORCE_PRECISION_MEDIUMP_BOOL) || defined(GLM_PRECISION_MEDIUMP_BOOL)
#	define GLM_CONFIG_PRECISION_BOOL		GLM_MEDIUMP
#elif defined(GLM_FORCE_PRECISION_LOWP_BOOL) || defined(GLM_PRECISION_LOWP_BOOL)
#	define GLM_CONFIG_PRECISION_BOOL		GLM_LOWP
#else
#	define GLM_CONFIG_PRECISION_BOOL		GLM_HIGHP
#endif

#if defined(GLM_FORCE_PRECISION_HIGHP_INT) || defined(GLM_PRECISION_HIGHP_INT)
#	define GLM_CONFIG_PRECISION_INT			GLM_HIGHP
#elif defined(GLM_FORCE_PRECISION_MEDIUMP_INT) || defined(GLM_PRECISION_MEDIUMP_INT)
#	define GLM_CONFIG_PRECISION_INT			GLM_MEDIUMP
#elif defined(GLM_FORCE_PRECISION_LOWP_INT) || defined(GLM_PRECISION_LOWP_INT)
#	define GLM_CONFIG_PRECISION_INT			GLM_LOWP
#else
#	define GLM_CONFIG_PRECISION_INT			GLM_HIGHP
#endif

#if defined(GLM_FORCE_PRECISION_HIGHP_UINT) || defined(GLM_PRECISION_HIGHP_UINT)
#	define GLM_CONFIG_PRECISION_UINT		GLM_HIGHP
#elif defined(GLM_FORCE_PRECISION_MEDIUMP_UINT) || defined(GLM_PRECISION_MEDIUMP_UINT)
#	define GLM_CONFIG_PRECISION_UINT		GLM_MEDIUMP
#elif defined(GLM_FORCE_PRECISION_LOWP_UINT) || defined(GLM_PRECISION_LOWP_UINT)
#	define GLM_CONFIG_PRECISION_UINT		GLM_LOWP
#else
#	define GLM_CONFIG_PRECISION_UINT		GLM_HIGHP
#endif

#if defined(GLM_FORCE_PRECISION_HIGHP_FLOAT) || defined(GLM_PRECISION_HIGHP_FLOAT)
#	define GLM_CONFIG_PRECISION_FLOAT		GLM_HIGHP
#elif defined(GLM_FORCE_PRECISION_MEDIUMP_FLOAT) || defined(GLM_PRECISION_MEDIUMP_FLOAT)
#	define GLM_CONFIG_PRECISION_FLOAT		GLM_MEDIUMP
#elif defined(GLM_FORCE_PRECISION_LOWP_FLOAT) || defined(GLM_PRECISION_LOWP_FLOAT)
#	define GLM_CONFIG_PRECISION_FLOAT		GLM_LOWP
#else
#	define GLM_CONFIG_PRECISION_FLOAT		GLM_HIGHP
#endif

#if defined(GLM_FORCE_PRECISION_HIGHP_DOUBLE) || defined(GLM_PRECISION_HIGHP_DOUBLE)
#	define GLM_CONFIG_PRECISION_DOUBLE		GLM_HIGHP
#elif defined(GLM_FORCE_PRECISION_MEDIUMP_DOUBLE) || defined(GLM_PRECISION_MEDIUMP_DOUBLE)
#	define GLM_CONFIG_PRECISION_DOUBLE		GLM_MEDIUMP
#elif defined(GLM_FORCE_PRECISION_LOWP_DOUBLE) || defined(GLM_PRECISION_LOWP_DOUBLE)
#	define GLM_CONFIG_PRECISION_DOUBLE		GLM_LOWP
#else
#	define GLM_CONFIG_PRECISION_DOUBLE		GLM_HIGHP
#endif

///////////////////////////////////////////////////////////////////////////////////
// Check inclusions of different versions of GLM

#elif ((GLM_SETUP_INCLUDED != GLM_VERSION) && !defined(GLM_FORCE_IGNORE_VERSION))
#	error "GLM error: A different version of GLM is already included. Define GLM_FORCE_IGNORE_VERSION before including GLM headers to ignore this error."
#elif GLM_SETUP_INCLUDED == GLM_VERSION

///////////////////////////////////////////////////////////////////////////////////
// Messages

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_MESSAGE_DISPLAYED)
#	define GLM_MESSAGE_DISPLAYED
#		define GLM_STR_HELPER(x) #x
#		define GLM_STR(x) GLM_STR_HELPER(x)

	// Report GLM version
#		pragma message (GLM_STR(GLM_VERSION_MESSAGE))

	// Report C++ language
#	if (GLM_LANG & GLM_LANG_CXX2A_FLAG) && (GLM_LANG & GLM_LANG_EXT)
#		pragma message("GLM: C++ 2A with extensions")
#	elif (GLM_LANG & GLM_LANG_CXX2A_FLAG)
#		pragma message("GLM: C++ 2A")
#	elif (GLM_LANG & GLM_LANG_CXX17_FLAG) && (GLM_LANG & GLM_LANG_EXT)
#		pragma message("GLM: C++ 17 with extensions")
#	elif (GLM_LANG & GLM_LANG_CXX17_FLAG)
#		pragma message("GLM: C++ 17")
#	elif (GLM_LANG & GLM_LANG_CXX14_FLAG) && (GLM_LANG & GLM_LANG_EXT)
#		pragma message("GLM: C++ 14 with extensions")
#	elif (GLM_LANG & GLM_LANG_CXX14_FLAG)
#		pragma message("GLM: C++ 14")
#	elif (GLM_LANG & GLM_LANG_CXX11_FLAG) && (GLM_LANG & GLM_LANG_EXT)
#		pragma message("GLM: C++ 11 with extensions")
#	elif (GLM_LANG & GLM_LANG_CXX11_FLAG)
#		pragma message("GLM: C++ 11")
#	elif (GLM_LANG & GLM_LANG_CXX0X_FLAG) && (GLM_LANG & GLM_LANG_EXT)
#		pragma message("GLM: C++ 0x with extensions")
#	elif (GLM_LANG & GLM_LANG_CXX0X_FLAG)
#		pragma message("GLM: C++ 0x")
#	elif (GLM_LANG & GLM_LANG_CXX03_FLAG) && (GLM_LANG & GLM_LANG_EXT)
#		pragma message("GLM: C++ 03 with extensions")
#	elif (GLM_LANG & GLM_LANG_CXX03_FLAG)
#		pragma message("GLM: C++ 03")
#	elif (GLM_LANG & GLM_LANG_CXX98_FLAG) && (GLM_LANG & GLM_LANG_EXT)
#		pragma message("GLM: C++ 98 with extensions")
#	elif (GLM_LANG & GLM_LANG_CXX98_FLAG)
#		pragma message("GLM: C++ 98")
#	else
#		pragma message("GLM: C++ language undetected")
#	endif//GLM_LANG

	// Report compiler detection
#	if GLM_COMPILER & GLM_COMPILER_CUDA
#		pragma message("GLM: CUDA compiler detected")
#	elif GLM_COMPILER & GLM_COMPILER_VC
#		pragma message("GLM: Visual C++ compiler detected")
#	elif GLM_COMPILER & GLM_COMPILER_CLANG
#		pragma message("GLM: Clang compiler detected")
#	elif GLM_COMPILER & GLM_COMPILER_INTEL
#		pragma message("GLM: Intel Compiler detected")
#	elif GLM_COMPILER & GLM_COMPILER_GCC
#		pragma message("GLM: GCC compiler detected")
#	else
#		pragma message("GLM: Compiler not detected")
#	endif

	// Report build target
#	if (GLM_ARCH & GLM_ARCH_AVX2_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: x86 64 bits with AVX2 instruction set build target")
#	elif (GLM_ARCH & GLM_ARCH_AVX2_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: x86 32 bits with AVX2 instruction set build target")

#	elif (GLM_ARCH & GLM_ARCH_AVX_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: x86 64 bits with AVX instruction set build target")
#	elif (GLM_ARCH & GLM_ARCH_AVX_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: x86 32 bits with AVX instruction set build target")

#	elif (GLM_ARCH & GLM_ARCH_SSE42_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: x86 64 bits with SSE4.2 instruction set build target")
#	elif (GLM_ARCH & GLM_ARCH_SSE42_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: x86 32 bits with SSE4.2 instruction set build target")

#	elif (GLM_ARCH & GLM_ARCH_SSE41_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: x86 64 bits with SSE4.1 instruction set build target")
#	elif (GLM_ARCH & GLM_ARCH_SSE41_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: x86 32 bits with SSE4.1 instruction set build target")

#	elif (GLM_ARCH & GLM_ARCH_SSSE3_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: x86 64 bits with SSSE3 instruction set build target")
#	elif (GLM_ARCH & GLM_ARCH_SSSE3_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: x86 32 bits with SSSE3 instruction set build target")

#	elif (GLM_ARCH & GLM_ARCH_SSE3_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: x86 64 bits with SSE3 instruction set build target")
#	elif (GLM_ARCH & GLM_ARCH_SSE3_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: x86 32 bits with SSE3 instruction set build target")

#	elif (GLM_ARCH & GLM_ARCH_SSE2_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: x86 64 bits with SSE2 instruction set build target")
#	elif (GLM_ARCH & GLM_ARCH_SSE2_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: x86 32 bits with SSE2 instruction set build target")

#	elif (GLM_ARCH & GLM_ARCH_X86_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: x86 64 bits build target")
#	elif (GLM_ARCH & GLM_ARCH_X86_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: x86 32 bits build target")

#	elif (GLM_ARCH & GLM_ARCH_NEON_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: ARM 64 bits with Neon instruction set build target")
#	elif (GLM_ARCH & GLM_ARCH_NEON_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: ARM 32 bits with Neon instruction set build target")

#	elif (GLM_ARCH & GLM_ARCH_ARM_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: ARM 64 bits build target")
#	elif (GLM_ARCH & GLM_ARCH_ARM_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: ARM 32 bits build target")

#	elif (GLM_ARCH & GLM_ARCH_MIPS_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: MIPS 64 bits build target")
#	elif (GLM_ARCH & GLM_ARCH_MIPS_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: MIPS 32 bits build target")

#	elif (GLM_ARCH & GLM_ARCH_PPC_BIT) && (GLM_MODEL == GLM_MODEL_64)
#		pragma message("GLM: PowerPC 64 bits build target")
#	elif (GLM_ARCH & GLM_ARCH_PPC_BIT) && (GLM_MODEL == GLM_MODEL_32)
#		pragma message("GLM: PowerPC 32 bits build target")
#	else
#		pragma message("GLM: Unknown build target")
#	endif//GLM_ARCH

	// Report platform name
#	if(GLM_PLATFORM & GLM_PLATFORM_QNXNTO)
#		pragma message("GLM: QNX platform detected")
//#	elif(GLM_PLATFORM & GLM_PLATFORM_IOS)
//#		pragma message("GLM: iOS platform detected")
#	elif(GLM_PLATFORM & GLM_PLATFORM_APPLE)
#		pragma message("GLM: Apple platform detected")
#	elif(GLM_PLATFORM & GLM_PLATFORM_WINCE)
#		pragma message("GLM: WinCE platform detected")
#	elif(GLM_PLATFORM & GLM_PLATFORM_WINDOWS)
#		pragma message("GLM: Windows platform detected")
#	elif(GLM_PLATFORM & GLM_PLATFORM_CHROME_NACL)
#		pragma message("GLM: Native Client detected")
#	elif(GLM_PLATFORM & GLM_PLATFORM_ANDROID)
#		pragma message("GLM: Android platform detected")
#	elif(GLM_PLATFORM & GLM_PLATFORM_LINUX)
#		pragma message("GLM: Linux platform detected")
#	elif(GLM_PLATFORM & GLM_PLATFORM_UNIX)
#		pragma message("GLM: UNIX platform detected")
#	elif(GLM_PLATFORM & GLM_PLATFORM_UNKNOWN)
#		pragma message("GLM: platform unknown")
#	else
#		pragma message("GLM: platform not detected")
#	endif

	// Report whether only xyzw component are used
#	if defined GLM_FORCE_XYZW_ONLY
#		pragma message("GLM: GLM_FORCE_XYZW_ONLY is defined. Only x, y, z and w component are available in vector type. This define disables swizzle operators and SIMD instruction sets.")
#	endif

	// Report swizzle operator support
#	if GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_OPERATOR
#		pragma message("GLM: GLM_FORCE_SWIZZLE is defined, swizzling operators enabled.")
#	elif GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_FUNCTION
#		pragma message("GLM: GLM_FORCE_SWIZZLE is defined, swizzling functions enabled. Enable compiler C++ language extensions to enable swizzle operators.")
#	else
#		pragma message("GLM: GLM_FORCE_SWIZZLE is undefined. swizzling functions or operators are disabled.")
#	endif

	// Report .length() type
#	if GLM_CONFIG_LENGTH_TYPE == GLM_LENGTH_SIZE_T
#		pragma message("GLM: GLM_FORCE_SIZE_T_LENGTH is defined. .length() returns a glm::length_t, a typedef of std::size_t.")
#	else
#		pragma message("GLM: GLM_FORCE_SIZE_T_LENGTH is undefined. .length() returns a glm::length_t, a typedef of int following GLSL.")
#	endif

#	if GLM_CONFIG_UNRESTRICTED_GENTYPE == GLM_ENABLE
#		pragma message("GLM: GLM_FORCE_UNRESTRICTED_GENTYPE is defined. Removes GLSL restrictions on valid function genTypes.")
#	else
#		pragma message("GLM: GLM_FORCE_UNRESTRICTED_GENTYPE is undefined. Follows strictly GLSL on valid function genTypes.")
#	endif

#	if GLM_SILENT_WARNINGS == GLM_ENABLE
#		pragma message("GLM: GLM_FORCE_SILENT_WARNINGS is defined. Ignores C++ warnings from using C++ language extensions.")
#	else
#		pragma message("GLM: GLM_FORCE_SILENT_WARNINGS is undefined. Shows C++ warnings from using C++ language extensions.")
#	endif

#	ifdef GLM_FORCE_SINGLE_ONLY
#		pragma message("GLM: GLM_FORCE_SINGLE_ONLY is defined. Using only single precision floating-point types.")
#	endif

#	if defined(GLM_FORCE_ALIGNED_GENTYPES) && (GLM_CONFIG_ALIGNED_GENTYPES == GLM_ENABLE)
#		undef GLM_FORCE_ALIGNED_GENTYPES
#		pragma message("GLM: GLM_FORCE_ALIGNED_GENTYPES is defined, allowing aligned types. This prevents the use of C++ constexpr.")
#	elif defined(GLM_FORCE_ALIGNED_GENTYPES) && (GLM_CONFIG_ALIGNED_GENTYPES == GLM_DISABLE)
#		undef GLM_FORCE_ALIGNED_GENTYPES
#		pragma message("GLM: GLM_FORCE_ALIGNED_GENTYPES is defined but is disabled. It requires C++11 and language extensions.")
#	endif

#	if defined(GLM_FORCE_DEFAULT_ALIGNED_GENTYPES)
#		if GLM_CONFIG_ALIGNED_GENTYPES == GLM_DISABLE
#			undef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#			pragma message("GLM: GLM_FORCE_DEFAULT_ALIGNED_GENTYPES is defined but is disabled. It requires C++11 and language extensions.")
#		elif GLM_CONFIG_ALIGNED_GENTYPES == GLM_ENABLE
#			pragma message("GLM: GLM_FORCE_DEFAULT_ALIGNED_GENTYPES is defined. All gentypes (e.g. vec3) will be aligned and padded by default.")
#		endif
#	endif

#	if GLM_CONFIG_CLIP_CONTROL & GLM_CLIP_CONTROL_ZO_BIT
#		pragma message("GLM: GLM_FORCE_DEPTH_ZERO_TO_ONE is defined. Using zero to one depth clip space.")
#	else
#		pragma message("GLM: GLM_FORCE_DEPTH_ZERO_TO_ONE is undefined. Using negative one to one depth clip space.")
#	endif

#	if GLM_CONFIG_CLIP_CONTROL & GLM_CLIP_CONTROL_LH_BIT
#		pragma message("GLM: GLM_FORCE_LEFT_HANDED is defined. Using left handed coordinate system.")
#	else
#		pragma message("GLM: GLM_FORCE_LEFT_HANDED is undefined. Using right handed coordinate system.")
#	endif
#endif//GLM_MESSAGES

#endif//GLM_SETUP_INCLUDED

namespace glm
{
	/// Qualify GLM types in term of alignment (packed, aligned) and precision in term of ULPs (lowp, mediump, highp)
	enum qualifier
	{
		packed_highp, ///< Typed data is tightly packed in memory and operations are executed with high precision in term of ULPs
		packed_mediump, ///< Typed data is tightly packed in memory  and operations are executed with medium precision in term of ULPs for higher performance
		packed_lowp, ///< Typed data is tightly packed in memory  and operations are executed with low precision in term of ULPs to maximize performance

#		if GLM_CONFIG_ALIGNED_GENTYPES == GLM_ENABLE
			aligned_highp, ///< Typed data is aligned in memory allowing SIMD optimizations and operations are executed with high precision in term of ULPs
			aligned_mediump, ///< Typed data is aligned in memory allowing SIMD optimizations and operations are executed with high precision in term of ULPs for higher performance
			aligned_lowp, // ///< Typed data is aligned in memory allowing SIMD optimizations and operations are executed with high precision in term of ULPs to maximize performance
			aligned = aligned_highp, ///< By default aligned qualifier is also high precision
#		endif

		highp = packed_highp, ///< By default highp qualifier is also packed
		mediump = packed_mediump, ///< By default mediump qualifier is also packed
		lowp = packed_lowp, ///< By default lowp qualifier is also packed
		packed = packed_highp, ///< By default packed qualifier is also high precision

#		if GLM_CONFIG_ALIGNED_GENTYPES == GLM_ENABLE && defined(GLM_FORCE_DEFAULT_ALIGNED_GENTYPES)
			defaultp = aligned_highp
#		else
			defaultp = highp
#		endif
	};

	typedef qualifier precision;

	template<length_t L, typename T, qualifier Q = defaultp> struct vec;
	template<length_t C, length_t R, typename T, qualifier Q = defaultp> struct mat;
	template<typename T, qualifier Q = defaultp> struct qua;

#	if GLM_HAS_TEMPLATE_ALIASES
		template <typename T, qualifier Q = defaultp> using tvec1 = vec<1, T, Q>;
		template <typename T, qualifier Q = defaultp> using tvec2 = vec<2, T, Q>;
		template <typename T, qualifier Q = defaultp> using tvec3 = vec<3, T, Q>;
		template <typename T, qualifier Q = defaultp> using tvec4 = vec<4, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat2x2 = mat<2, 2, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat2x3 = mat<2, 3, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat2x4 = mat<2, 4, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat3x2 = mat<3, 2, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat3x3 = mat<3, 3, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat3x4 = mat<3, 4, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat4x2 = mat<4, 2, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat4x3 = mat<4, 3, T, Q>;
		template <typename T, qualifier Q = defaultp> using tmat4x4 = mat<4, 4, T, Q>;
		template <typename T, qualifier Q = defaultp> using tquat = qua<T, Q>;
#	endif

namespace detail
{
	template<glm::qualifier P>
	struct is_aligned
	{
		static const bool value = false;
	};

#	if GLM_CONFIG_ALIGNED_GENTYPES == GLM_ENABLE
		template<>
		struct is_aligned<glm::aligned_lowp>
		{
			static const bool value = true;
		};

		template<>
		struct is_aligned<glm::aligned_mediump>
		{
			static const bool value = true;
		};

		template<>
		struct is_aligned<glm::aligned_highp>
		{
			static const bool value = true;
		};
#	endif

	template<length_t L, typename T, bool is_aligned>
	struct storage
	{
		typedef struct type {
			T data[L];
		} type;
	};

#	if GLM_HAS_ALIGNOF
		template<length_t L, typename T>
		struct storage<L, T, true>
		{
			typedef struct alignas(L * sizeof(T)) type {
				T data[L];
			} type;
		};

		template<typename T>
		struct storage<3, T, true>
		{
			typedef struct alignas(4 * sizeof(T)) type {
				T data[4];
			} type;
		};
#	endif

#	if GLM_ARCH & GLM_ARCH_SSE2_BIT
	template<>
	struct storage<4, float, true>
	{
		typedef glm_f32vec4 type;
	};

	template<>
	struct storage<4, int, true>
	{
		typedef glm_i32vec4 type;
	};

	template<>
	struct storage<4, unsigned int, true>
	{
		typedef glm_u32vec4 type;
	};

	template<>
	struct storage<2, double, true>
	{
		typedef glm_f64vec2 type;
	};

	template<>
	struct storage<2, detail::int64, true>
	{
		typedef glm_i64vec2 type;
	};

	template<>
	struct storage<2, detail::uint64, true>
	{
		typedef glm_u64vec2 type;
	};
#	endif

#	if (GLM_ARCH & GLM_ARCH_AVX_BIT)
	template<>
	struct storage<4, double, true>
	{
		typedef glm_f64vec4 type;
	};
#	endif

#	if (GLM_ARCH & GLM_ARCH_AVX2_BIT)
	template<>
	struct storage<4, detail::int64, true>
	{
		typedef glm_i64vec4 type;
	};

	template<>
	struct storage<4, detail::uint64, true>
	{
		typedef glm_u64vec4 type;
	};
#	endif

#	if GLM_ARCH & GLM_ARCH_NEON_BIT
	template<>
	struct storage<4, float, true>
	{
		typedef glm_f32vec4 type;
	};

	template<>
	struct storage<4, int, true>
	{
		typedef glm_i32vec4 type;
	};

	template<>
	struct storage<4, unsigned int, true>
	{
		typedef glm_u32vec4 type;
	};
#	endif

	enum genTypeEnum
	{
		GENTYPE_VEC,
		GENTYPE_MAT,
		GENTYPE_QUAT
	};

	template <typename genType>
	struct genTypeTrait
	{};

	template <length_t C, length_t R, typename T>
	struct genTypeTrait<mat<C, R, T> >
	{
		static const genTypeEnum GENTYPE = GENTYPE_MAT;
	};

	template<typename genType, genTypeEnum type>
	struct init_gentype
	{
	};

	template<typename genType>
	struct init_gentype<genType, GENTYPE_QUAT>
	{
		GLM_FUNC_QUALIFIER GLM_CONSTEXPR static genType identity()
		{
			return genType(1, 0, 0, 0);
		}
	};

	template<typename genType>
	struct init_gentype<genType, GENTYPE_MAT>
	{
		GLM_FUNC_QUALIFIER GLM_CONSTEXPR static genType identity()
		{
			return genType(1);
		}
	};
}//namespace detail
}//namespace glm



//#if GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_OPERATOR
//#	include "_swizzle.hpp"
//#elif GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_FUNCTION
//#	include "_swizzle_func.hpp"
//#endif
#include <cstddef>

namespace glm
{
	template<typename T, qualifier Q>
	struct vec<2, T, Q>
	{
		// -- Implementation detail --

		typedef T value_type;
		typedef vec<2, T, Q> type;
		typedef vec<2, bool, Q> bool_type;

		// -- Data --

#		if GLM_SILENT_WARNINGS == GLM_ENABLE
#			if GLM_COMPILER & GLM_COMPILER_GCC
#				pragma GCC diagnostic push
#				pragma GCC diagnostic ignored "-Wpedantic"
#			elif GLM_COMPILER & GLM_COMPILER_CLANG
#				pragma clang diagnostic push
#				pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#				pragma clang diagnostic ignored "-Wnested-anon-types"
#			elif GLM_COMPILER & GLM_COMPILER_VC
#				pragma warning(push)
#				pragma warning(disable: 4201)  // nonstandard extension used : nameless struct/union
#			endif
#		endif

#		if GLM_CONFIG_XYZW_ONLY
			T x, y;
#		elif GLM_CONFIG_ANONYMOUS_STRUCT == GLM_ENABLE
			union
			{
				struct{ T x, y; };
				struct{ T r, g; };
				struct{ T s, t; };

				typename detail::storage<2, T, detail::is_aligned<Q>::value>::type data;
/*
#				if GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_OPERATOR
					GLM_SWIZZLE2_2_MEMBERS(T, Q, x, y)
					GLM_SWIZZLE2_2_MEMBERS(T, Q, r, g)
					GLM_SWIZZLE2_2_MEMBERS(T, Q, s, t)
					GLM_SWIZZLE2_3_MEMBERS(T, Q, x, y)
					GLM_SWIZZLE2_3_MEMBERS(T, Q, r, g)
					GLM_SWIZZLE2_3_MEMBERS(T, Q, s, t)
					GLM_SWIZZLE2_4_MEMBERS(T, Q, x, y)
					GLM_SWIZZLE2_4_MEMBERS(T, Q, r, g)
					GLM_SWIZZLE2_4_MEMBERS(T, Q, s, t)
#				endif
*/
			};
#		else
			union {T x, r, s;};
			union {T y, g, t;};
/*
#			if GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_FUNCTION
				GLM_SWIZZLE_GEN_VEC_FROM_VEC2(T, Q)
#			endif//GLM_CONFIG_SWIZZLE
*/
#		endif

#		if GLM_SILENT_WARNINGS == GLM_ENABLE
#			if GLM_COMPILER & GLM_COMPILER_CLANG
#				pragma clang diagnostic pop
#			elif GLM_COMPILER & GLM_COMPILER_GCC
#				pragma GCC diagnostic pop
#			elif GLM_COMPILER & GLM_COMPILER_VC
#				pragma warning(pop)
#			endif
#		endif

		// -- Component accesses --

		/// Return the count of components of the vector
		typedef length_t length_type;
		GLM_FUNC_DECL static GLM_CONSTEXPR length_type length(){return 2;}

		GLM_FUNC_DECL GLM_CONSTEXPR T& operator[](length_type i);
		GLM_FUNC_DECL GLM_CONSTEXPR T const& operator[](length_type i) const;

		// -- Implicit basic constructors --

		GLM_FUNC_DECL GLM_CONSTEXPR vec() GLM_DEFAULT;
		GLM_FUNC_DECL GLM_CONSTEXPR vec(vec const& v) GLM_DEFAULT;
		template<qualifier P>
		GLM_FUNC_DECL GLM_CONSTEXPR vec(vec<2, T, P> const& v);

		// -- Explicit basic constructors --

		GLM_FUNC_DECL GLM_CONSTEXPR explicit vec(T scalar);
		GLM_FUNC_DECL GLM_CONSTEXPR vec(T x, T y);

		// -- Conversion constructors --

		template<typename U, qualifier P>
		GLM_FUNC_DECL GLM_CONSTEXPR explicit vec(vec<1, U, P> const& v);

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B>
		GLM_FUNC_DECL GLM_CONSTEXPR vec(A x, B y);
		template<typename A, typename B>
		GLM_FUNC_DECL GLM_CONSTEXPR vec(vec<1, A, Q> const& x, B y);
		template<typename A, typename B>
		GLM_FUNC_DECL GLM_CONSTEXPR vec(A x, vec<1, B, Q> const& y);
		template<typename A, typename B>
		GLM_FUNC_DECL GLM_CONSTEXPR vec(vec<1, A, Q> const& x, vec<1, B, Q> const& y);

		// -- Conversion vector constructors --

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		GLM_FUNC_DECL GLM_CONSTEXPR GLM_EXPLICIT vec(vec<3, U, P> const& v);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		GLM_FUNC_DECL GLM_CONSTEXPR GLM_EXPLICIT vec(vec<4, U, P> const& v);

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		GLM_FUNC_DECL GLM_CONSTEXPR GLM_EXPLICIT vec(vec<2, U, P> const& v);

		// -- Swizzle constructors --
#		if GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_OPERATOR
			template<int E0, int E1>
			GLM_FUNC_DECL GLM_CONSTEXPR vec(detail::_swizzle<2, T, Q, E0, E1,-1,-2> const& that)
			{
				*this = that();
			}
#		endif//GLM_CONFIG_SWIZZLE == GLM_SWIZZLE_OPERATOR

		// -- Unary arithmetic operators --

		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator=(vec const& v) GLM_DEFAULT;

		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator+=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator+=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator+=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator-=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator-=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator-=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator*=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator*=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator*=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator/=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator/=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator/=(vec<2, U, Q> const& v);

		// -- Increment and decrement operators --

		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator++();
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator--();
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator++(int);
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator--(int);

		// -- Unary bit operators --

		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator%=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator%=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator%=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator&=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator&=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator&=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator|=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator|=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator|=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator^=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator^=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator^=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator<<=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator<<=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator<<=(vec<2, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator>>=(U scalar);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator>>=(vec<1, U, Q> const& v);
		template<typename U>
		GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> & operator>>=(vec<2, U, Q> const& v);
	};

	// -- Unary operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v);

	// -- Binary operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator+(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator+(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator-(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator-(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator*(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator*(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator/(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator/(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator%(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator%(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator&(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator&(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator|(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator|(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator^(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator^(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator<<(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator<<(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator>>(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator>>(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, T, Q> operator~(vec<2, T, Q> const& v);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR bool operator==(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR bool operator!=(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, bool, Q> operator&&(vec<2, bool, Q> const& v1, vec<2, bool, Q> const& v2);

	template<qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR vec<2, bool, Q> operator||(vec<2, bool, Q> const& v1, vec<2, bool, Q> const& v2);
}//namespace glm

#ifndef GLM_EXTERNAL_TEMPLATE
//#include "type_vec2.inl"
/// @ref core

//#include "./compute_vector_relational.hpp"

namespace glm
{
	// -- Implicit basic constructors --

#	if GLM_CONFIG_DEFAULTED_FUNCTIONS == GLM_DISABLE
		template<typename T, qualifier Q>
		GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec()
#			if GLM_CONFIG_CTOR_INIT != GLM_CTOR_INIT_DISABLE
				: x(0), y(0)
#			endif
		{}

		template<typename T, qualifier Q>
		GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(vec<2, T, Q> const& v)
			: x(v.x), y(v.y)
		{}
#	endif

	template<typename T, qualifier Q>
	template<qualifier P>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(vec<2, T, P> const& v)
		: x(v.x), y(v.y)
	{}

	// -- Explicit basic constructors --

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(T scalar)
		: x(scalar), y(scalar)
	{}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(T _x, T _y)
		: x(_x), y(_y)
	{}

	// -- Conversion scalar constructors --

	template<typename T, qualifier Q>
	template<typename U, qualifier P>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(vec<1, U, P> const& v)
		: x(static_cast<T>(v.x))
		, y(static_cast<T>(v.x))
	{}

	template<typename T, qualifier Q>
	template<typename A, typename B>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(A _x, B _y)
		: x(static_cast<T>(_x))
		, y(static_cast<T>(_y))
	{}

	template<typename T, qualifier Q>
	template<typename A, typename B>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(vec<1, A, Q> const& _x, B _y)
		: x(static_cast<T>(_x.x))
		, y(static_cast<T>(_y))
	{}

	template<typename T, qualifier Q>
	template<typename A, typename B>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(A _x, vec<1, B, Q> const& _y)
		: x(static_cast<T>(_x))
		, y(static_cast<T>(_y.x))
	{}

	template<typename T, qualifier Q>
	template<typename A, typename B>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(vec<1, A, Q> const& _x, vec<1, B, Q> const& _y)
		: x(static_cast<T>(_x.x))
		, y(static_cast<T>(_y.x))
	{}

	// -- Conversion vector constructors --

	template<typename T, qualifier Q>
	template<typename U, qualifier P>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(vec<2, U, P> const& v)
		: x(static_cast<T>(v.x))
		, y(static_cast<T>(v.y))
	{}

	template<typename T, qualifier Q>
	template<typename U, qualifier P>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(vec<3, U, P> const& v)
		: x(static_cast<T>(v.x))
		, y(static_cast<T>(v.y))
	{}

	template<typename T, qualifier Q>
	template<typename U, qualifier P>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q>::vec(vec<4, U, P> const& v)
		: x(static_cast<T>(v.x))
		, y(static_cast<T>(v.y))
	{}

	// -- Component accesses --

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR T & vec<2, T, Q>::operator[](typename vec<2, T, Q>::length_type i)
	{
		assert(i >= 0 && i < this->length());
		switch(i)
		{
		default:
		case 0:
			return x;
		case 1:
			return y;
		}
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR T const& vec<2, T, Q>::operator[](typename vec<2, T, Q>::length_type i) const
	{
		assert(i >= 0 && i < this->length());
		switch(i)
		{
		default:
		case 0:
			return x;
		case 1:
			return y;
		}
	}

	// -- Unary arithmetic operators --

#	if GLM_CONFIG_DEFAULTED_FUNCTIONS == GLM_DISABLE
		template<typename T, qualifier Q>
		GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator=(vec<2, T, Q> const& v)
		{
			this->x = v.x;
			this->y = v.y;
			return *this;
		}
#	endif

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator=(vec<2, U, Q> const& v)
	{
		this->x = static_cast<T>(v.x);
		this->y = static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator+=(U scalar)
	{
		this->x += static_cast<T>(scalar);
		this->y += static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator+=(vec<1, U, Q> const& v)
	{
		this->x += static_cast<T>(v.x);
		this->y += static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator+=(vec<2, U, Q> const& v)
	{
		this->x += static_cast<T>(v.x);
		this->y += static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator-=(U scalar)
	{
		this->x -= static_cast<T>(scalar);
		this->y -= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator-=(vec<1, U, Q> const& v)
	{
		this->x -= static_cast<T>(v.x);
		this->y -= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator-=(vec<2, U, Q> const& v)
	{
		this->x -= static_cast<T>(v.x);
		this->y -= static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator*=(U scalar)
	{
		this->x *= static_cast<T>(scalar);
		this->y *= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator*=(vec<1, U, Q> const& v)
	{
		this->x *= static_cast<T>(v.x);
		this->y *= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator*=(vec<2, U, Q> const& v)
	{
		this->x *= static_cast<T>(v.x);
		this->y *= static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator/=(U scalar)
	{
		this->x /= static_cast<T>(scalar);
		this->y /= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator/=(vec<1, U, Q> const& v)
	{
		this->x /= static_cast<T>(v.x);
		this->y /= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator/=(vec<2, U, Q> const& v)
	{
		this->x /= static_cast<T>(v.x);
		this->y /= static_cast<T>(v.y);
		return *this;
	}

	// -- Increment and decrement operators --

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator++()
	{
		++this->x;
		++this->y;
		return *this;
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator--()
	{
		--this->x;
		--this->y;
		return *this;
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> vec<2, T, Q>::operator++(int)
	{
		vec<2, T, Q> Result(*this);
		++*this;
		return Result;
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> vec<2, T, Q>::operator--(int)
	{
		vec<2, T, Q> Result(*this);
		--*this;
		return Result;
	}

	// -- Unary bit operators --

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator%=(U scalar)
	{
		this->x %= static_cast<T>(scalar);
		this->y %= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator%=(vec<1, U, Q> const& v)
	{
		this->x %= static_cast<T>(v.x);
		this->y %= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator%=(vec<2, U, Q> const& v)
	{
		this->x %= static_cast<T>(v.x);
		this->y %= static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator&=(U scalar)
	{
		this->x &= static_cast<T>(scalar);
		this->y &= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator&=(vec<1, U, Q> const& v)
	{
		this->x &= static_cast<T>(v.x);
		this->y &= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator&=(vec<2, U, Q> const& v)
	{
		this->x &= static_cast<T>(v.x);
		this->y &= static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator|=(U scalar)
	{
		this->x |= static_cast<T>(scalar);
		this->y |= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator|=(vec<1, U, Q> const& v)
	{
		this->x |= static_cast<T>(v.x);
		this->y |= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator|=(vec<2, U, Q> const& v)
	{
		this->x |= static_cast<T>(v.x);
		this->y |= static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator^=(U scalar)
	{
		this->x ^= static_cast<T>(scalar);
		this->y ^= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator^=(vec<1, U, Q> const& v)
	{
		this->x ^= static_cast<T>(v.x);
		this->y ^= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator^=(vec<2, U, Q> const& v)
	{
		this->x ^= static_cast<T>(v.x);
		this->y ^= static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator<<=(U scalar)
	{
		this->x <<= static_cast<T>(scalar);
		this->y <<= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator<<=(vec<1, U, Q> const& v)
	{
		this->x <<= static_cast<T>(v.x);
		this->y <<= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator<<=(vec<2, U, Q> const& v)
	{
		this->x <<= static_cast<T>(v.x);
		this->y <<= static_cast<T>(v.y);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator>>=(U scalar)
	{
		this->x >>= static_cast<T>(scalar);
		this->y >>= static_cast<T>(scalar);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator>>=(vec<1, U, Q> const& v)
	{
		this->x >>= static_cast<T>(v.x);
		this->y >>= static_cast<T>(v.x);
		return *this;
	}

	template<typename T, qualifier Q>
	template<typename U>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> & vec<2, T, Q>::operator>>=(vec<2, U, Q> const& v)
	{
		this->x >>= static_cast<T>(v.x);
		this->y >>= static_cast<T>(v.y);
		return *this;
	}

	// -- Unary arithmetic operators --

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v)
	{
		return v;
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			-v.x,
			-v.y);
	}

	// -- Binary arithmetic operators --

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x + scalar,
			v.y + scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x + v2.x,
			v1.y + v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator+(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar + v.x,
			scalar + v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator+(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x + v2.x,
			v1.x + v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x + v2.x,
			v1.y + v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x - scalar,
			v.y - scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x - v2.x,
			v1.y - v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator-(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar - v.x,
			scalar - v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator-(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x - v2.x,
			v1.x - v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x - v2.x,
			v1.y - v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x * scalar,
			v.y * scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x * v2.x,
			v1.y * v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator*(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar * v.x,
			scalar * v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator*(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x * v2.x,
			v1.x * v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x * v2.x,
			v1.y * v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x / scalar,
			v.y / scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x / v2.x,
			v1.y / v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator/(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar / v.x,
			scalar / v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator/(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x / v2.x,
			v1.x / v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x / v2.x,
			v1.y / v2.y);
	}

	// -- Binary bit operators --

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x % scalar,
			v.y % scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x % v2.x,
			v1.y % v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator%(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar % v.x,
			scalar % v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator%(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x % v2.x,
			v1.x % v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x % v2.x,
			v1.y % v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x & scalar,
			v.y & scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x & v2.x,
			v1.y & v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator&(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar & v.x,
			scalar & v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator&(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x & v2.x,
			v1.x & v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x & v2.x,
			v1.y & v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x | scalar,
			v.y | scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x | v2.x,
			v1.y | v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator|(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar | v.x,
			scalar | v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator|(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x | v2.x,
			v1.x | v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x | v2.x,
			v1.y | v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x ^ scalar,
			v.y ^ scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x ^ v2.x,
			v1.y ^ v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator^(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar ^ v.x,
			scalar ^ v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator^(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x ^ v2.x,
			v1.x ^ v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x ^ v2.x,
			v1.y ^ v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x << scalar,
			v.y << scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x << v2.x,
			v1.y << v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator<<(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar << v.x,
			scalar << v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator<<(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x << v2.x,
			v1.x << v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x << v2.x,
			v1.y << v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(
			v.x >> scalar,
			v.y >> scalar);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x >> v2.x,
			v1.y >> v2.x);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator>>(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			scalar >> v.x,
			scalar >> v.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator>>(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x >> v2.x,
			v1.x >> v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(
			v1.x >> v2.x,
			v1.y >> v2.y);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, T, Q> operator~(vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(
			~v.x,
			~v.y);
	}

	// -- Boolean operators --
/*
	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR bool operator==(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return
			detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::call(v1.x, v2.x) &&
			detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::call(v1.y, v2.y);
	}
*/
	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR bool operator!=(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return !(v1 == v2);
	}

	template<qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, bool, Q> operator&&(vec<2, bool, Q> const& v1, vec<2, bool, Q> const& v2)
	{
		return vec<2, bool, Q>(v1.x && v2.x, v1.y && v2.y);
	}

	template<qualifier Q>
	GLM_FUNC_QUALIFIER GLM_CONSTEXPR vec<2, bool, Q> operator||(vec<2, bool, Q> const& v1, vec<2, bool, Q> const& v2)
	{
		return vec<2, bool, Q>(v1.x || v2.x, v1.y || v2.y);
	}
}//namespace glm

#endif//GLM_EXTERNAL_TEMPLATE



/// @ref core
/// @file glm/ext/vector_float2.hpp

namespace glm
{
	/// @addtogroup core_vector
	/// @{

	/// 2 components vector of single-precision floating-point numbers.
	///
	/// @see <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.pdf">GLSL 4.20.8 specification, section 4.1.5 Vectors</a>
	typedef vec<2, float, defaultp>	vec2;

	/// @}
}//namespace glm