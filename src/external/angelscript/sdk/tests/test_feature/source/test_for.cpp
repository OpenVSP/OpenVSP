#include "utils.h"

namespace TestFor
{

static const char * const TESTNAME = "TestFor";

static const char *script1 = "\
float[] myArray(70);                       \n\
                                           \n\
float aSize = 8;                           \n\
                                           \n\
float MyFunction(float a)                  \n\
{                                          \n\
  return a;                                \n\
}                                          \n\
                                           \n\
void Test()                                \n\
{                                          \n\
  for (float k = 0; k< aSize; k++)         \n\
  {                                        \n\
    myArray[int(MyFunction(k*aSize))] = k; \n\
  }                                        \n\
                                           \n\
  for (int i = 0; i< aSize*aSize; i++)     \n\
  {                                        \n\
//  Print(\"i = \" + i + \"\\n\");         \n\
    myArray[i] = i;                        \n\
  }                                        \n\
}                                          \n";


void Print_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetArgAddress(0);
	PRINTF("%s", str->buffer.c_str());
}

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;

	// Basic for loops
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptArray(engine, true);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		RegisterScriptString_Generic(engine);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void Print(const string &in)", asFUNCTION(Print_Generic), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}
		r = ExecuteString(engine, "Test()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// TODO: runtime optimize: bytecode compiler should optimize away the first jump
		r = ExecuteString(engine, "bool called = false; for(;;) { called = true; break; } Assert( called );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test for-loop with multiple variables and increment statements
	// http://www.gamedev.net/topic/668075-support-for-multiple-increment-statements-in-for-loop/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"void func() \n"
			"{ \n"
			"  int result = 0; \n"
			"  for( int a = 1, b = 1; a < 5 && b < 5; a++, b = a+1 ) \n"
			"    result += a*b; \n"
			"  assert( result == (1 + 6 + 12) ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "func()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

