// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>
// Windows defs
#ifdef WIN32
// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#include <tchar.h>

// linux / OSX defs
#else
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/timeb.h>

#ifdef LINUX
#include <linux/ioctl.h>
#endif

#include <time.h>
#include <pthread.h>

// these are needed for Win32/Linux string comparisons
#define stricmp strcasecmp
#define strnicmp strncasecmp

typedef int BOOL;
typedef long int LONG;
typedef short int SHORT;
typedef char CHAR;

typedef unsigned long int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int UINT;

typedef long long INT64;
typedef unsigned long long UINT64;

typedef int PT_FILEHANDLE;
typedef void * DLL_HANDLE;

typedef unsigned long DWORD;
typedef unsigned long* LPDWORD;
typedef void* LPOVERLAPPED;
typedef void* OVERLAPPED;
typedef void* LPVOID;
typedef void* PVOID;
typedef void VOID;
// note that handle here is assumed to be
typedef int HANDLE;
// a pointer to a file decriptor
typedef int* PHANDLE;
typedef int BOOL;

typedef unsigned long UINT32;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef long long INT64;
typedef long long LARGE_INTEGER;
typedef unsigned char BYTE;

/* These are defined so we can use TCHAR compatible string calls */
#define _T(arg) arg
#define TCHAR char
#define tstrcpy strcpy
#define tstrncpy strncpy
#define _tcscat strcat
#define _tcscpy(str1, str2) strcpy(str1, str2)
#define _tcslen(str1) strlen(str1)
#define _tfopen(filename, access) fopen(filename, access)
#define _gettc getc
#define _puttc putc
#define _stscanf sscanf
#define _stprintf sprintf
#define _tprintf printf

/* common constants */
#define SUCCESS 0
#define FAILURE -1

#define IOCTL_FAIL(status) (status < 0)

/** unusual return codes */
#define UNIMPLEMENTED -1001

// create some equivalent constants in linux that windows have
#define STATIC static

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif

/** sleep for x milliseconds */
inline void nap(unsigned long msec) { usleep(msec*1000); }

#define Sleep sleep

typedef double VWTIME;
/** returns the amount of time in seconds since some arbitrary moment. */
inline VWTIME VWGetTime() { return 0.0; }

// end linux defs
#endif

// TODO: reference additional headers your program requires here
