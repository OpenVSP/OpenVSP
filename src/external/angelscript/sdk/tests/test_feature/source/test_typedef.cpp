#include "utils.h"

namespace TestTypedef
{

static const char * const TESTNAME = "TestTypedef";
#define TESTMODULE		"TestTypedef"
#define TESTTYPE		"TestType1"

//	Script for testing attributes
static const char *const script =
"typedef int8  TestType1;								\n"
"typedef int16 TestType2;								\n"
"typedef int32 TestType3;								\n"
"typedef int64 TestType4;								\n"
"typedef float real;                                    \n"
"TestType1 TEST1 = 1;									\n"
"TestType2 TEST2 = 2;									\n"
"TestType3 TEST3 = 4;									\n"
"TestType4 TEST4 = 8;									\n"
"														\n"
"TestType4 func(TestType1 a) {return a;}                \n"
"														\n"
"void Test()											\n"
"{														\n"
"	TestType1	val1;									\n"
"	TestType2	val2;									\n"
"	TestType3	val3;									\n"
"														\n"
"														\n"
"	val1 = TEST1;										\n"
"	if(val1 == TEST1)	{	val2 = 0;	}				\n"
"	if(TEST2 <= TEST3)	{	val2 = 0;	}				\n"
"	if(TEST2 < TEST3)	{	val2 = 0;	}				\n"
"	if(TEST2 > TEST3)	{	val2 = 0;	}				\n"
"	if(TEST2 != TEST3)	{	val2 = 0;	}				\n"
"	if(TEST2 <= TEST3)	{	val2 = 0;	}				\n"
"	if(TEST2 >= TEST3)	{	val2 = 0;	}				\n"
"	val1 = val2;										\n"
"														\n"
"}														\n";


//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////

static int testTypedef(CBytecodeStream &codeStream, bool save)
{
	asIScriptEngine		*engine;
	COutStream			out;
	int					r;

 	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if(NULL == engine) 
	{
		return -1;
	}

	r = engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	if( r >= 0 ) r = engine->RegisterTypedef("float32", "float");

	// Test working example
	if(true == save)
	{
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		if(r >= 0) 
		{
			r = mod->AddScriptSection(NULL, script);
		}

		if(r >= 0) 
		{
			r = mod->Build();
		}

		r = mod->SaveByteCode(&codeStream);
	}
	else 
	{
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&codeStream);
		if(r >= 0) 
		{
			mod->BindAllImportedFunctions();
		}
	}

	engine->Release();

	// Success
	return r;
}


bool Test()
{
	int r;
	CBytecodeStream	stream(__FILE__"1");

	r = testTypedef(stream, true);
	if(r >= 0) 
	{
		r = testTypedef(stream, false);
	}

	return (r < 0);
}

} // namespace

