#if defined(_MSC_VER) && !defined(_WIN32_WCE) && !defined(__S3E__)
#include <direct.h> // For _getcwd()
#endif
#ifdef _WIN32_WCE
#include <windows.h> // For GetModuleFileName()
#endif
#if defined(__S3E__) || defined(__APPLE__) || defined(__GNUC__)
#include <unistd.h> // For getcwd()
#endif


#include "utils.h"

#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif
#include <map>

using namespace std;





void Assert(asIScriptGeneric *gen)
{
	bool expr;
	if( sizeof(bool) == 1 )
		expr = gen->GetArgByte(0) ? true : false;
	else
		expr = gen->GetArgDWord(0) ? true : false;
	if( !expr )
	{
		PRINTF("--- Assert failed ---\n");
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
		{
			const asIScriptFunction *function = ctx->GetFunction();
			if( function != 0 )
			{
				PRINTF("func: %s\n", function->GetDeclaration());
				PRINTF("mdle: %s\n", function->GetModuleName());
				PRINTF("sect: %s\n", function->GetScriptSectionName());
			}
			PRINTF("line: %d\n", ctx->GetLineNumber());
			ctx->SetException("Assert failed", false);
			PRINTF("---------------------\n");
		}
	}
}

//#define TRACK_LOCATIONS
//#define TRACK_SIZES

static int numAllocs            = 0;
static int numFrees             = 0;
static size_t currentMemAlloc   = 0;
static size_t maxMemAlloc       = 0;
static int maxNumAllocsSameTime = 0;
static asQWORD sumAllocSize     = 0;

static map<void*,size_t> memSize;
static map<void*,int> memCount;

#ifdef TRACK_SIZES
static map<size_t,int> meanSize;
#endif

#ifdef TRACK_LOCATIONS
struct loc
{
	const char *file;
	int line;

	bool operator <(const loc &other) const
	{
		if( file < other.file ) return true;
		if( file == other.file && line < other.line ) return true;
		return false;
	}
};

struct counters
{
	int allocs;
	int frees;
	int totalMemAlloced;
	int totalMemFreed;
};

static map<loc, counters> locCount;
static map<void*, loc> memAllocedFrom;
#endif

void *MyAllocWithStats(size_t size, const char *file, int line)
{
	// Avoid compiler warning when variables aren't used
	UNUSED_VAR(line);
	UNUSED_VAR(file);

	// Allocate the memory
	void *ptr = malloc(size);
#if !defined(__psp2__) && !defined(__CELLOS_LV2__)
	// Count number of allocations made
	numAllocs++;

	// Count total amount of memory allocated
	sumAllocSize += size;

	// Update currently allocated memory
	currentMemAlloc += size;
	if( currentMemAlloc > maxMemAlloc ) maxMemAlloc = currentMemAlloc;

	// Remember the size of the memory allocated at this pointer
	memSize.insert(map<void*,size_t>::value_type(ptr,size));

	// Remember the currently allocated memory blocks, with the allocation number so that we can debug later
	memCount.insert(map<void*,int>::value_type(ptr,numAllocs));

	// Determine the maximum number of allocations at the same time
	if( numAllocs - numFrees > maxNumAllocsSameTime )
		maxNumAllocsSameTime = numAllocs - numFrees;

#ifdef TRACK_SIZES
	// Determine the mean size of the memory allocations
	map<size_t,int>::iterator i = meanSize.find(size);
	if( i != meanSize.end() )
		i->second++;
	else
		meanSize.insert(map<size_t,int>::value_type(size,1));
#endif

#ifdef TRACK_LOCATIONS
	// Count the number of allocations for each location in the library
	loc l = {file, line};
	map<loc, counters>::iterator i2 = locCount.find(l);
	if (i2 != locCount.end())
	{
		i2->second.allocs++;
		i2->second.totalMemAlloced += size;
	}
	else
	{
		counters c = { 1,0,size,0 };
		locCount.insert(map<loc, counters>::value_type(l, c));
	}

	// Remember where the allocation is from
	memAllocedFrom.insert(map<void*, loc>::value_type(ptr, l));
#endif
#endif
	return ptr;
}

void MyFreeWithStats(void *address)
{
#if !defined(__psp2__) && !defined(__CELLOS_LV2__)
	// Count the number of deallocations made
	numFrees++;

	// Remove the memory block from the list of allocated blocks
	size_t allocSize = 0;
	map<void*,size_t>::iterator i = memSize.find(address);
	if( i != memSize.end() )
	{
		// Decrease the current amount of allocated memory
		allocSize = i->second;
		currentMemAlloc -= allocSize;
		memSize.erase(i);
	}
	else
		assert(false);

	// Verify which memory we are currently removing so we know we did the allocation, and where it was allocated
	map<void*,int>::iterator i2 = memCount.find(address);
	if( i2 != memCount.end() )
	{
//		int numAlloc = i2->second;
		memCount.erase(i2);
	}
	else
		assert(false);

#ifdef TRACK_LOCATIONS
	// Find out where the allocation was from
	map<void*, loc>::iterator it = memAllocedFrom.find(address);
	if (it != memAllocedFrom.end())
	{
		map<loc, counters>::iterator i2 = locCount.find(it->second);
		if (i2 != locCount.end())
		{
			i2->second.frees++;
			i2->second.totalMemFreed += allocSize;
		}

		memAllocedFrom.erase(it);
	}
	else
		assert(false);
#endif
#endif
	// Free the actual memory
	free(address);
}

void InstallMemoryManager()
{
#ifdef TRACK_LOCATIONS
	assert( strstr(asGetLibraryOptions(), " AS_DEBUG ") );
#endif

	asSetGlobalMemoryFunctions((asALLOCFUNC_t)MyAllocWithStats, MyFreeWithStats);
}

void PrintAllocIndices()
{
	map<void*,int>::iterator i = memCount.begin();
	while( i != memCount.end() )
	{
		PRINTF("%d\n", i->second);
		i++;
	}
}

void PrintLocationCounters()
{
#ifdef TRACK_LOCATIONS
	// Print allocation counts per location
	map<loc, counters>::iterator i2 = locCount.begin();
	while (i2 != locCount.end())
	{
		const char *file = i2->first.file;
		int         line = i2->first.line;
		int         count = i2->second.allocs;
		int         frees = i2->second.frees;
		int         totalMem = i2->second.totalMemAlloced;
		int         totalFree = i2->second.totalMemFreed;
		PRINTF("%s (%d): %d, %d, %d, %d\n", file, line, count, count-frees, totalMem, totalMem - totalFree);
		i2++;
	}
#endif
}

void RemoveMemoryManager()
{
	asThreadCleanup();

	PrintAllocIndices();

//	assert( numAllocs == numFrees );
//	assert( currentMemAlloc == 0 );

	PRINTF("---------\n");
	PRINTF("MEMORY STATISTICS\n");
	PRINTF("number of allocations                 : %d\n", numAllocs);                   // 125744
	PRINTF("max allocated memory at any one time  : %d\n", (int)maxMemAlloc);                 // 121042
	PRINTF("max number of simultaneous allocations: %d\n", maxNumAllocsSameTime);        // 2134
	PRINTF("total amount of allocated memory      : %d\n", (int)sumAllocSize);                // 10106765
	PRINTF("medium size of allocations            : %d\n", numAllocs ? (int)sumAllocSize/numAllocs : 0);

#ifdef TRACK_SIZES
	// Find the mean size of allocations
	map<size_t,int>::iterator i = meanSize.begin();
	int n = 0;
	int meanAllocSize = 0;
	while( i != meanSize.end() )
	{
		if( n + i->second > numAllocs / 2 )
		{
			meanAllocSize = (int)i->first;
			break;
		}

		n += i->second;
		i++;
	}
	PRINTF("mean size of allocations              : %d\n", meanAllocSize);
	PRINTF("smallest allocation size              : %d\n", meanSize.begin()->first);
	PRINTF("largest allocation size               : %d\n", meanSize.rbegin()->first);
	PRINTF("number of different allocation sizes  : %d\n", meanSize.size());

	// Print allocation sizes
	i = meanSize.begin();
	while( i != meanSize.end() )
	{
		if( i->second >= 1000 )
			PRINTF("alloc size %d: %d\n", i->first, i->second);
		i++;
	}
#endif

	PrintLocationCounters();

	asResetGlobalMemoryFunctions();
}

int GetNumAllocs()
{
	return numAllocs;
}

int GetAllocedMem()
{
	return int(currentMemAlloc);
}


asDWORD ComputeCRC32(const asBYTE *buf, asUINT length)
{
	// Compute the lookup table
	asDWORD lup[256];
	for( asUINT pos = 0; pos < 256; pos++ )
	{
		asDWORD val = pos;
		for( int i = 8; i > 0; i-- )
		{
			if( val & 1 )
				val = (val >> 1) ^ 0xEDB88320;
			else
				val >>= 1;
		}
		lup[pos] = val;
	}

	// Calculate the CRC32 value
	asDWORD crc = 0xFFFFFFFF;
	for( asUINT i = 0; i < length; i++ )
	{
		crc = ((crc) >> 8) ^ lup[*buf++ ^ (crc & 0x000000FF)];
	}

	return ~crc;
}

bool ValidateByteCode(asIScriptFunction *func, asBYTE *expect)
{
	if (func == 0) return false;
	asUINT len;
	asDWORD *bc = func->GetByteCode(&len);
	for( asUINT n = 0, i = 0; n < len; )
	{
		asBYTE c = *(asBYTE*)(&bc[n]);
		if( c != expect[i] )
			return false;
		n += asBCTypeSize[asBCInfo[c].type];

		if( expect[i++] == asBC_RET && n < len )
			return false;
	}

	return true;
}

string GetCurrentDir()
{
	string str;
	char buffer[1024];
#ifdef _MSC_VER
#ifdef _WIN32_WCE
    static TCHAR apppath[MAX_PATH] = TEXT("");
    if (!apppath[0])
    {
        GetModuleFileName(NULL, apppath, MAX_PATH);


        int appLen = _tcslen(apppath);

        // Look for the last backslash in the path, which would be the end
        // of the path itself and the start of the filename.  We only want
        // the path part of the exe's full-path filename
        // Safety is that we make sure not to walk off the front of the
        // array (in case the path is nothing more than a filename)
        while (appLen > 1)
        {
            if (apppath[appLen-1] == TEXT('\\'))
                break;
            appLen--;
        }

        // Terminate the string after the trailing backslash
        apppath[appLen] = TEXT('\0');
    }
#ifdef _UNICODE
    wcstombs(buffer, apppath, min(1024, wcslen(apppath)*sizeof(wchar_t)));
#else
    memcpy(buffer, apppath, min(1024, strlen(apppath)));
#endif

    str = buffer;
#elif defined(__S3E__)
	// Marmalade uses its own portable C library
	str = getcwd(buffer, (int)1024);
#elif _XBOX_VER >= 200
	// XBox 360 doesn't support the getcwd function, just use the root folder
	str = "game:\\";
#elif defined(_M_ARM)
	// TODO: How to determine current working dir on Windows Phone?
	str = ""; 
#else
	str = _getcwd(buffer, (int)1024);
#endif // _MSC_VER
#elif defined(__APPLE__) || defined(__GNUC__)
	str = getcwd(buffer, 1024);
#else
	str = "";
#endif

	// Replace backslashes for forward slashes
	size_t pos = 0;
	while( (pos = str.find("\\", pos)) != string::npos )
		str[pos] = '/';

	return str;
}
