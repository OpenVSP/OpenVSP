#include <stdio.h>
#include <conio.h>
#if defined(_MSC_VER)
#include <crtdbg.h>
#endif

namespace TestSharedString { bool Test(); }
namespace TestThreadMgr { bool Test(); }
namespace TestGC { bool Test(); }

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

	if( TestSharedString::Test()      ) goto failed; else printf("TestSharedString passed\n");
	if( TestThreadMgr::Test()         ) goto failed; else printf("TestThreadMgr passed\n");
	if( TestGC::Test()                ) goto failed; else printf("TestGC passed\n");

	printf("--------------------------------------------\n");
	printf("All of the tests passed with success.\n\n");
	printf("Press any key to quit.\n");
	while(!_getch());
	return 0;

failed:
	printf("--------------------------------------------\n");
	printf("One of the tests failed, see details above.\n\n");
	printf("Press any key to quit.\n");
	while(!_getch());
	return 0;
}
