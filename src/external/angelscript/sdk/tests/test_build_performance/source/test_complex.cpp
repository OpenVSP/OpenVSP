//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include "memory_stream.h"
#include <string>
using std::string;

namespace TestComplex
{

#define TESTNAME "TestComplex"

#ifdef _DEBUG
const int numCases = 2;
const int numIfs = 2;
const int numLevels = 2;
#else
const int numCases = 2;
const int numIfs = 5;
const int numLevels = 4;
#endif

string buildSwitch(int levels)
{
	if( levels == 0 ) return "";

	string script;

	script += "  switch( var ) { \n";

	for( int n = 0; n < numCases; n++ )
	{
		string buf;
		buf.reserve(500);
		sprintf(&buf[0], "  case %d: \n", n);
		script += &buf[0];

		script += "    { \n";
		script += "      func(var+var, var-var, var*var, var/var, var+var, var-var, var*var, var/var); \n";

		for( int m = 0; m < numIfs; m++ )
		{
			if( m == 0 )
				script += "      if( var + var > 0 ) { \n";
			else
				script += "      } else if( var + var > 0 ) { \n";

	 		script += "        func(var+var, var-var, var*var, var/var, var+var, var-var, var*var, var/var); \n";

			script += buildSwitch( levels - 1 );
		}
		script += "      } \n";

		script += "    } \n";
	}

	script += "  } \n";

	return script;
}

void Test()
{
	printf("---------------------------------------------\n");
	printf("%s\n\n", TESTNAME);
	printf("AngelScript 2.31.0: Build 1.28 secs, Save 0.0348 secs, Load 0.0592 secs\n");

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptArray(engine, true);
	RegisterStdString(engine);

//	engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);

	////////////////////////////////////////////
	printf("\nGenerating...\n");

	string script;
	script += "int var; \n";
	script += "int func(int, int, int, int, int, int, int, int) { return 0; } \n";
	script += "void main() { \n";
	script += buildSwitch(numLevels);
	script += "} \n";

	int lines = 0;
	for( unsigned int n = 0; n < script.size(); n++ )
		if( script[n] == '\n' ) lines++;
	printf("Number of lines: %d\n", lines);

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



