#include "utils.h"
/*
using std::string;
#include "cstring.h"

namespace TestCString
{

static const char * const TESTNAME = "TestCString";

static string printOutput;

// This function receives the string by reference
// (in fact it is a reference to copy of the string)
static void PrintString(asIScriptGeneric *gen)
{
	CString *str = (CString*)gen->GetArgAddress(0);
	printOutput = str->GetBuffer(0);
}

// This script tests that variables are created and destroyed in the correct order
static const char *script2 =
"void testString()                         \n"
"{                                         \n"
"  print(getString(\"I\" \"d\" \"a\"));    \n"
"}                                         \n"
"string getString(string &in str)          \n"
"{                                         \n"
"  return \"hello \" + str;                \n"
"}                                         \n";

static const char *script3 =
"string str = 1;                \n"
"const string str2 = \"test\";  \n"
"void test()                    \n"
"{                              \n"
"   string s = str2;            \n"
"}                              \n";

static const char *script4 =
"void test()                    \n"
"{                              \n"
"   string s = \"\"\"           \n"
"Heredoc\\x20test\n"
"            \"\"\" \"\\x21\";  \n"
"   print(s);                   \n"
"}                              \n";

static const char *script7 =
"void test()                    \n"
"{                              \n"
"   Func(\"test\");             \n"
"}                              \n"
"void Func(const string &in str)\n"
"{                              \n"
"}                              \n";

bool Get(int *obj, const CString &szURL, CString &szHTML)
{
	assert(&szHTML != 0);
	assert(szURL == "stringtest");
	szHTML.SetString("output");
	return false;
}

bool Test()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	asString<CString>::Register(engine);

	engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(PrintString), asCALL_GENERIC);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	mod->AddScriptSection(0, TESTNAME, script2, strlen(script2), 0);
	mod->Build(0);

	engine->ExecuteString(0, "testString()");

	if( printOutput != "hello Ida" )
	{
		fail = true;
		printf("%s: Failed to print the correct string\n", TESTNAME);
	}

	engine->ExecuteString(0, "string s = \"test\\\\test\\\\\"");

	// Verify that it is possible to use the string in constructor parameters
	printOutput = "";
	engine->ExecuteString(0, "string a; a = 1; print(a);");
	if( printOutput != "1" ) fail = true;

	printOutput = "";
	engine->ExecuteString(0, "string a; a += 1; print(a);");
	if( printOutput != "1" ) fail = true;

	printOutput = "";
	engine->ExecuteString(0, "string a = 1; print(a);");
	if( printOutput != "1" ) fail = true;

    printOutput = "";
    engine->ExecuteString(0, "string a = \" \"; a[0] = 65; print(a);");
    if( printOutput != "A" ) fail = true;

	mod->AddScriptSection(0, TESTNAME, script3, strlen(script3), 0);
	if( mod->Build(0) < 0 )
		fail = true;

	printOutput = "";
	mod->AddScriptSection(0, TESTNAME, script4, strlen(script4), 0);
	if( mod->Build(0) < 0 )
		fail = true;
	engine->ExecuteString(0, "test()");
	if( printOutput != "Heredoc\\x20test!" ) fail = true;

	CString *a = new CString("a");
	engine->RegisterGlobalProperty("string a", a);
	int r = engine->ExecuteString(0, "print(a == \"a\" ? \"t\" : \"f\")");
	if( r != asEXECUTION_FINISHED )
	{
		fail = true;
		printf("%s: ExecuteString() failed\n", TESTNAME);
	}
	delete a;

	//-------------------------------------
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	mod->AddScriptSection(0, "test", script7, strlen(script7), 0, false);
	mod->Build(0);
	r = engine->ExecuteString(0, "test()");
	if( r != asEXECUTION_FINISHED ) fail = true;

	engine->RegisterObjectType("Http", sizeof(int), asOBJ_PRIMITIVE);
	engine->RegisterObjectMethod("Http","bool get(const string &in,string &out)", asFUNCTION(Get),asCALL_CDECL_OBJFIRST);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	r = engine->ExecuteString(0, "Http h; string str; h.get(\"stringtest\", str); assert(str == \"output\");");
	if( r != asEXECUTION_FINISHED ) fail = true;

	r = engine->ExecuteString(0, "Http h; string a = \"test\", b; h.get(\"string\"+a, b); assert(b == \"output\");");
	if( r != asEXECUTION_FINISHED ) fail = true;

	engine->Release();

	return fail;
}

} // namespace

*/
