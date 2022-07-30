#include <stdio.h>
#include <conio.h>
#if defined(_MSC_VER)
#include <crtdbg.h>
#endif
#include "angelscript.h"

namespace TestBasic { void Test(); }
namespace TestBigArrays { void Test(); }
namespace TestManySymbols { void Test(); }
namespace TestManyFuncs { void Test(); }
namespace TestComplex { void Test(); }
namespace TestRebuild { void Test(); }
namespace TestHugeAPI { void Test(); }

void DetectMemoryLeaks()
{
#if defined(_MSC_VER)
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	_CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);
#endif
}

int main(int argc, char **argv)
{
	DetectMemoryLeaks();

	printf("Build performance test");
#ifdef _DEBUG 
	printf(" (DEBUG)");
#endif
	printf("\n");
	printf("AngelScript %s\n", asGetLibraryVersion()); 

	TestBasic::Test();
	TestBigArrays::Test();
	TestManySymbols::Test();
	TestManyFuncs::Test();
	TestComplex::Test();
	TestRebuild::Test();
	TestHugeAPI::Test();
	
	printf("--------------------------------------------\n");
	printf("Press any key to quit.\n");
	while(!getch());
	return 0;
}
