#include "utils.h"

namespace TestCDeclObjLast
{

class Foo
{
public:
	Foo() {}
	~Foo() {}

protected:

} foo;

static int NoArg(Foo *f)
{
	assert( f == &foo );

	return 42;
}

static double FloatArg(float a, Foo *f)
{
	assert( a < 3.01f && a > 2.99f );
	assert( f == &foo );

	return 24;
}

static float MixArgs(int a, double b, int c, float d, Foo *f)
{
	assert( a == 1 );
	assert( b < 2.01 && b > 1.99 );
	assert( c == 3 );
	assert( d < 4.01 && d > 3.99 );
	assert( f == &foo );

	return 12;
}

static std::string RetStringByVal(int a, Foo *f)
{
	assert( a == 1 );
	assert( f == &foo );

	return "test";
}


static const char * const TESTNAME = "TestCDeclObjLast";

bool Test()
{
	if( strstr(asGetLibraryOptions(), "MAX_PORTABILITY") )
	{
		PRINTF("%s skipped due to max portability\n", TESTNAME);
		return false;
	}

	bool fail = false;
	int r;

	COutStream out;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	RegisterStdString(engine);

	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	engine->RegisterObjectType("Foo", sizeof(Foo), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CD);
	engine->RegisterGlobalProperty("Foo foo", &foo);

	engine->RegisterObjectMethod("Foo", "int noarg()", asFUNCTION(NoArg), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Foo", "double floatarg(float)", asFUNCTION(FloatArg), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Foo", "float mixargs(int, double, int, float)", asFUNCTION(MixArgs), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Foo", "string retstring(int a)", asFUNCTION(RetStringByVal), asCALL_CDECL_OBJLAST);

	r = ExecuteString(engine, "int a = foo.noarg(); assert( a == 42 );");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ExecuteString(engine, "double a = foo.floatarg(3); assert( a < 24.1 && a > 23.9 );");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ExecuteString(engine, "float a = foo.mixargs(1, 2, 3, 4); assert( a < 12.1 && a > 11.9 );");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ExecuteString(engine, "string a = foo.retstring(1); assert( a == 'test' );");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	return fail;
}

} // end namespace
