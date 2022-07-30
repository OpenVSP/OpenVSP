//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include "memory_stream.h"
#include <string>
using std::string;

namespace TestRebuild
{

#define TESTNAME "TestRebuild"


void Test()
{
	int r = 0;

	printf("---------------------------------------------\n");
	printf("%s\n\n", TESTNAME);
	printf("AngelScript 2.31.0: 0.844 secs\n");

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptArray(engine, true);
	RegisterStdString(engine);

	////////////////////////////////////////////
	printf("\nBuilding...\n");

	double timeFirst = 0;
	double timeMiddle = 0;
	double timeLast = 0;
	double time = GetSystemTimer();

#ifdef _DEBUG
	const int iterations = 10;
#else
	const int iterations = 5000;
#endif

	for( int n = 0; n < iterations; n++ )
	{
		double timeSingle = GetSystemTimer();

		// Build a script with circular references between entities
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"Test t; \n"
			"void globalFunc() { \n"
			"  for( uint n = 0; n < 10; n++ ) \n"
			"    t.children[n].doSomething(); \n"
			"} \n"
			"class Test { \n"
			"  Test @next; \n"
			"  array<Test@> children; \n"
			"  void doSomething() { if( t is this ) doSomethingElse(); } \n"
			"  void doSomethingElse() { globalFunc(); } \n"
			"  Test() {} \n"
			"  Test @opAssign() { return this; } \n"
			"} \n"
			"void main() { \n"
			"  for( uint n = 0; n < 10; n++ ) \n"
			"    t.children.insertLast(Test()); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			break;

		// Discard the module
		mod->Discard();

		timeSingle = GetSystemTimer() - timeSingle;
		if( n == 0 )
			timeFirst = timeSingle;
		else if( n == iterations/2 )
			timeMiddle = timeSingle;
		else
			timeLast = timeSingle;
	}

	time = GetSystemTimer() - time;

	if( r != 0 )
		printf("Build failed\n");
	else
	{
		printf("Time = %f secs\n", time);
		printf("\n");
		printf("Time first   = %f secs\n", timeFirst);
		printf("Time middle  = %f secs\n", timeMiddle);
		printf("Time last    = %f secs\n", timeLast);
		printf("Time average = %f secs\n", time/iterations);
	}

	engine->Release();
}

} // namespace



