#include "utils.h"

namespace TestArrayHandle
{

static const char * const TESTNAME = "TestArrayHandle";

static const char *script1 =
"void TestArrayHandle()                          \n"
"{                                               \n"
"   string[]@[]@ a;                              \n"
"   string[]@[] b(2);                            \n"
"   Assert(@a == null);                          \n"
"   @a = @string[]@[](2);                        \n"
"   Assert(@a != null);                          \n"
"   Assert(@a[0] == null);                       \n"
"   string@[] c(10);                             \n"
"   Assert(c.length() == 10);                    \n"
"   Assert(g.length() == 2);                     \n"
"}                                               \n"
"string@[] g(2);                                 \n";
 
static const char *script2 =
"void TestArrayHandle2()                         \n"
"{                                               \n"
"   string[] s(10);                              \n"
"   Append(s);                                   \n"
"                                                \n"
"   string[]@ sh = createArray();                \n"
"   double d1 = atof(sh[0]);                     \n"
"   double d2 = atof(s[0]);                      \n"
"}                                               \n"
"void Append(string[]@ s)                        \n"
"{                                               \n"
"   for( uint n = 0; n < s.length(); n++ )       \n"
"      s[n] += \".\";                            \n"
"}                                               \n"
"string[]@ createArray()                         \n"
"{                                               \n"
"   return string[](2);                          \n"
"}                                               \n";

void StringToDouble(asIScriptGeneric *gen)
{
	std::string s = ((CScriptString*)gen->GetArgAddress(0))->buffer;
	gen->SetReturnDouble(atof(s.c_str()));
}

bool Test()
{
	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	RegisterScriptString_Generic(engine);
	engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
	engine->RegisterGlobalFunction("double atof(const string &in)",asFUNCTION(StringToDouble),asCALL_GENERIC);

	COutStream out;

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	asIScriptContext *ctx = engine->CreateContext();
	r = ExecuteString(engine, "TestArrayHandle()", mod, ctx);
	if( r != asEXECUTION_FINISHED )
	{
		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());

		PRINTF("%s: Failed to execute script\n", TESTNAME);
		TEST_FAILED;
	}
	if( ctx ) ctx->Release();

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	ctx = engine->CreateContext();
	r = ExecuteString(engine, "TestArrayHandle2()", mod, ctx);
	if( r != asEXECUTION_FINISHED )
	{
		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());

		PRINTF("%s: Failed to execute script\n", TESTNAME);
		TEST_FAILED;
	}
	if( ctx ) ctx->Release();

	engine->Release();

	// 
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);

		const char *script = 
			"class Node \n"
			"{ \n"
			"  Node@[]@ GetSubnodes() { return subNodes; } \n"
			"  Node@[] subNodes; \n"
			"  int member; \n"
			"} \n"
			"void TestFunc(Node@ input) \n"
			"{ \n"
			"  Node@[]@ nodearray; \n"
			"  Node@ subnode; \n"
			"  // Case 1. Works as expected \n"
			"  @nodearray = @input.GetSubnodes(); \n"
			"  @subnode = @nodearray[0]; \n"
			"  int value1 = subnode.member; // <- ok \n"
			"  assert( value1 == 42 ); \n"
			"  // Case 2. Wrong address sent to following operations on 'subnode' \n"
			"  @subnode = @input.GetSubnodes()[0]; \n"
			"  int value2 = subnode.member; // <- weird behavior \n"
			"  assert( value2 == 42 ); \n"
			"} \n";

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) 
		{
			TEST_FAILED;
		}
		else
		{
			ctx = engine->CreateContext();
			r = ExecuteString(engine, "Node n; \n"
				                      "n.subNodes.resize(1); \n"
									  "@n.subNodes[0] = @Node(); \n"
									  "n.subNodes[0].member = 42; \n"
									  "TestFunc(n); \n", mod, ctx);
			if( r != asEXECUTION_FINISHED )
			{
				TEST_FAILED;
				if( r == asEXECUTION_EXCEPTION )
					PRINTF("%s", GetExceptionInfo(ctx).c_str());
			}
			ctx->Release();
		}

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

