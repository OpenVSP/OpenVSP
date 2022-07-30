//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include "memory_stream.h"
#include <string>
#include <sstream>
using std::string;
using std::stringstream;

// http://www.gamedev.net/topic/669305-very-slow-loading-of-bytecode-on-ios/

namespace TestHugeAPI
{

#define TESTNAME "TestHugeAPI"

void Test()
{
	printf("---------------------------------------------\n");
	printf("%s\n\n", TESTNAME);
	printf("AngelScript 2.31.0: Build 0.541 secs, Save 2.58 secs, Load 0.173 secs\n");

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	////////////////////////////////////////////
	printf("\nRegistering...\n");

#ifdef _DEBUG
	const int iterations = 10;
#else
	const int iterations = 20000;
#endif

	for( int n = 0; n < iterations; n++ )
	{
		char buf1[500];
		sprintf(buf1, "int glob%d", n);
		engine->RegisterGlobalProperty(buf1, (void*)1);

		sprintf(buf1, "void func%d(int, float)", n);
		engine->RegisterGlobalFunction(buf1, asFUNCTION(0), asCALL_GENERIC);

		sprintf(buf1, "obj%d", n);
		engine->RegisterObjectType(buf1, 4, asOBJ_VALUE|asOBJ_POD);
		engine->RegisterObjectMethod(buf1, "int func(float, float)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectProperty(buf1, "int val", 0);
	}

	////////////////////////////////////////////
	printf("\nGenerating...\n");

	string script;
	
	for( int i = 0; i < iterations; i++ )
	{
		char buf[500];
		sprintf(buf, "void f%d() {\n", i);
		script += buf;
	
		sprintf(buf, "  glob%d = %d;\n", i, i);
		script += buf;

		sprintf(buf, "  func%d(1, 3.12);\n", i);
		script += buf;

		sprintf(buf, "  obj%d o; o.val = o.func(3.12, 3.12);\n", i);
		script += buf;
		script += "}\n";
	}

	////////////////////////////////////////////
	printf("\nBuilding...\n");

	double time = GetSystemTimer();

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script.c_str(), script.size(), 0);
	int r = mod->Build();

	time = GetSystemTimer() - time;

	if( r != 0 )
		printf("Build failed\n");
	else
		printf("Time = %f secs\n", time);

	////////////////////////////////////////////
	printf("\nSaving...\n");

	time = GetSystemTimer();

	CBytecodeStream stream("");
	mod->SaveByteCode(&stream);

	time = GetSystemTimer() - time;
	printf("Time = %f secs\n", time);
	printf("Size = %d\n", int(stream.buffer.size()));

	////////////////////////////////////////////
	printf("\nLoading...\n");

	time = GetSystemTimer();

	asIScriptModule *mod2 = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod2->LoadByteCode(&stream);

	time = GetSystemTimer() - time;
	printf("Time = %f secs\n", time);

	engine->Release();
}

} // namespace



