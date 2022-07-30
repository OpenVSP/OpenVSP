#include "utils.h"

using namespace std;

namespace TestAssign
{

static const char * const TESTNAME = "TestAssign";

static const char *script1 =
"void main()                      \n"
"{                                \n"
"  uint8[] a={2,3,4,5};           \n"
"                                 \n"
"  a[1] |= 0x30;                  \n"
"  a[2] += 0x30;                  \n"
"  print(a[1]);                   \n"
"  print(a[2]);                   \n"
"  assert(a[1] == 0x33);          \n"
"  assert(a[2] == 0x34);          \n"
"}                                \n";

string buffer;

void print_generic(asIScriptGeneric *gen)
{
	if( gen->GetArgTypeId(0) == asTYPEID_INT32 )
	{
		int a = *(int*)gen->GetAddressOfArg(0);
		UNUSED_VAR(a);
//		PRINTF("%d\n", a);
	}
	else
	{
		string s = **(string**)gen->GetAddressOfArg(0);
		UNUSED_VAR(s);
		buffer += s;
//		PRINTF("%s\n", s.c_str());
	}
}


bool Test()
{
	bool fail = false;
	int r;
	COutStream out;
	asIScriptContext *ctx;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, true);
	RegisterStdString(engine);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
	engine->RegisterGlobalFunction("void print(int)", asFUNCTION(print_generic), asCALL_GENERIC);
	engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print_generic), asCALL_GENERIC);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	ctx = engine->CreateContext();
	r = ExecuteString(engine, "main()", mod, ctx);
	if( r != asEXECUTION_FINISHED )
	{
		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());

		PRINTF("%s: Failed to execute script\n", TESTNAME);
		TEST_FAILED;
	}
	if( ctx ) ctx->Release();

	{
		const char *script =
			"class CTest \n"
			"{ \n"
			"	string name; \n"
			"	 \n"
			"   CTest() { name = 'temp'; print('CTest::CTest() for ' + name + '\\n'); } \n"
			"	CTest(string s) { name = s; print('CTest::CTest() for ' + name + '\\n'); } \n"
			"	~CTest(){ print('CTest::~CTest() for ' + name + '\\n'); } \n"
			"	 \n"
		//	"   CTest @opAssign(const CTest &in o) { print('CTest::opAssign(), ' + name + ' becomes ' + o.name + '\\n'); name = o.name; return this; } \n"
			"	void test(){ print('CTest::test() for ' + name + '\\n'); } \n"
			"} \n"
			"void test() \n"
			"{ \n"
			"	CTest t1('Ent1'); \n"
			"	CTest t2('Ent2'); \n"
			"	 \n"
			"	t1.test(); \n"
			"	t2.test(); \n"
			"	 \n"
			"	t2 = t1; \n"
			"} \n";

		mod->AddScriptSection(TESTNAME, script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}

		r = ExecuteString(engine, "test()", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		// There shouldn't be any temporary variable created for the assignment
		if( buffer != "CTest::CTest() for Ent1\n"
		              "CTest::CTest() for Ent2\n"
		              "CTest::test() for Ent1\n"
		              "CTest::test() for Ent2\n"
		              "CTest::~CTest() for Ent1\n"
		              "CTest::~CTest() for Ent1\n" )
		{
			PRINTF("%s", buffer.c_str());
			TEST_FAILED;
		}
	}

	engine->Release();

	// Success
	return fail;
}

} // namespace

