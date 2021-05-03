#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <math.h>
#include <vector>

#include <angelscript.h>

#include "../../../add_on/scriptarray/scriptarray.h"
#include "scriptstring.h"
#include "../../../add_on/scriptstdstring/scriptstdstring.h"
#include "../../../add_on/scripthelper/scripthelper.h"
#if !defined(_MSC_VER) || _MSC_VER > 1200
// This doesn't work on MSVC6. The template implementation in that compiler isn't good enough.
#include "../../../add_on/autowrapper/aswrappedcall.h"
#endif

// Conditional define to call platform output.
// In all cases this macro use the same rules of standard printf
// PRINTF macro provided by Jordi Oliveras 
#if defined(ANDROID) || defined(__ANDROID__)
	#include <android/log.h>

	#define  LOG_TAG    "AS_ANDROID"
	#define IS_ANDROID
	#define  PRINTF(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    // Is this a Mac or an IPhone (or other iOS device)?
    #if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1
		// ios
        #include "xcodeLog.h"
		#define IS_IOS
        #define PRINTF printfXcode
    #else
        #define PRINTF printf
    #endif
#else
	#define PRINTF printf
#endif

#ifdef __BORLANDC__
// C++Builder doesn't define most of the non-standard float-specific math functions with
// "*f" suffix; instead it provides overloads for the standard math functions which take
// "float" arguments.
inline float fabsf (float arg) { return std::fabs (arg); }

// C++Builder 2006 and earlier don't pull "memcpy" into the global namespace.
using std::memcpy; 
#endif


#ifdef AS_USE_NAMESPACE
using namespace AngelScript;
#endif

#if defined(__GNUC__) && !(defined(__ppc__) || defined(__PPC__))
#define STDCALL __attribute__((stdcall))
#elif defined(_MSC_VER) || defined(__BORLANDC__)
#define STDCALL __stdcall
#else
#define STDCALL
#endif

class COutStream
{
public:
	void Callback(asSMessageInfo *msg) 
	{ 
		const char *msgType = 0;
		if( msg->type == 0 ) msgType = "Error  ";
		if( msg->type == 1 ) msgType = "Warning";
		if( msg->type == 2 ) msgType = "Info   ";

		PRINTF("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, msgType, msg->message);
	}
};

class CBufferedOutStream
{
public:
	void Callback(asSMessageInfo *msg) 
	{ 
		const char *msgType = 0;
		if( msg->type == 0 ) msgType = "Error  ";
		if( msg->type == 1 ) msgType = "Warning";
		if( msg->type == 2 ) msgType = "Info   ";

		char buf[1024];
#ifdef _MSC_VER
#if _MSC_VER >= 1500
		_snprintf_s(buf, 1023, 1023, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, msgType, msg->message);
#else
		_snprintf(buf, 1023, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, msgType, msg->message);
#endif
#else
		snprintf(buf, 1023, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, msgType, msg->message);
#endif
		buf[1023] = '\0';

		buffer += buf;
	}

	std::string buffer;
};

class CBytecodeFileStream : public asIBinaryStream
{
public:
	CBytecodeFileStream(const char *name) { this->name = name; f = 0; isReading = false; position = 0; }
	~CBytecodeFileStream() { if (f) fclose(f); }

	int Write(const void *ptr, asUINT size)
	{
		if (size == 0) return 0;
		if (f == 0 || isReading)
		{
			if (f)
				fclose(f);
#ifdef _MSC_VER
			fopen_s(&f, name.c_str(), "wb");
#else
			f = fopen(name.c_str(), "wb");
#endif
			isReading = false;
		}
		size_t r = fwrite(ptr, size, 1, f);
		fflush(f);
		position += size;
		if (r != 1)
			return -1;
		return 0;
	}
	int Read(void *ptr, asUINT size)
	{
		if (size == 0) return 0;
		if (f == 0 || !isReading)
		{
			if (f)
				fclose(f);
#ifdef _MSC_VER
			fopen_s(&f, name.c_str(), "rb");
#else
			f = fopen(name.c_str(), "rb");
#endif
			isReading = true;
		}
		size_t r = fread(ptr, size, 1, f);
		position += size;
		if (r != 1)
			return -1;
		return 0;
	}
	void Restart() { if (f) { fclose(f); f = 0; } position = 0; }

protected:
	std::string name;
	FILE *f;
	bool isReading;
	asDWORD position;
};

class CBytecodeStream : public asIBinaryStream
{
public:
	CBytecodeStream(const char * /*name*/) {wpointer = 0;rpointer = 0;}

	int Write(const void *ptr, asUINT size) 
	{
		if( size == 0 ) return 0; 
		buffer.resize(buffer.size() + size);
		memcpy(&buffer[wpointer], ptr, size); 
		wpointer += size;
		return 0;
/*		// Are we writing zeroes?
		for( asUINT n = 0; n < size; n++ )
			if( *(asBYTE*)ptr == 0 )
			{
				n = n; // <== Set break point here
				break;
			}
*/	}
	int Read(void *ptr, asUINT size) 
	{
		if (rpointer + size > buffer.size())
			return -1;
		memcpy(ptr, &buffer[rpointer], size); 
		rpointer += size;
		return 0;
	}
	void Restart() {rpointer = 0;}

	asUINT CountZeroes() { asUINT z = 0; for( asUINT n = 0; n < buffer.size(); n++ ) if( buffer[n] == 0 ) z++; return z; }
	std::vector<asBYTE> buffer;

protected:
	int rpointer;
	int wpointer;
};

void Assert(asIScriptGeneric *gen);

void InstallMemoryManager();
void RemoveMemoryManager();
int  GetNumAllocs();
int  GetAllocedMem();
void PrintLocationCounters();

// Returns true if bytecode is as expected
bool ValidateByteCode(asIScriptFunction *func, asBYTE *expectBC);

#if defined(_MSC_VER) && _MSC_VER <= 1200 // MSVC++ 6
	#define I64(x) x##l
#else // MSVC++ 7, GNUC, etc
	#define I64(x) x##ll
#endif

#endif

inline bool CompareDouble(double a,double b)
{
	// I'm using a quite low accuracy on the double comparison 
	// due to the known inaccuracies added by the Valgrind CPU 
	// simulator
	if( fabs( a - b ) > 0.000000001 )
		return false;
	return true;
}

inline bool CompareFloat(float a,float b)
{
	if( fabsf( a - b ) > 0.000001f )
		return false;
	return true;
}

asDWORD ComputeCRC32(const asBYTE *buf, asUINT length);

#define UNUSED_VAR(x) ((void)(x))

#define TEST_FAILED do { fail = true; PRINTF("Failed on line %d in %s\n", __LINE__, __FILE__); } while(!fail)

#define RET_ON_MAX_PORT if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") ) { PRINTF("Test on line %d in %s skipped\n", __LINE__, __FILE__); return false; }

#define SKIP_ON_MAX_PORT if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") ) { PRINTF("Test on line %d in %s skipped\n", __LINE__, __FILE__); } else

std::string GetCurrentDir();


