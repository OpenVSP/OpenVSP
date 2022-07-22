#include "utils.h"

namespace TestAutoHandle
{
using std::string;

void TestConstructor(string &arg1, CScriptString *arg2, double /*d*/, string &arg3, void * /*obj*/)
{
	assert(arg1 == "1");
	assert(arg2->buffer == "2");
	assert(arg3 == "3");

	arg2->Release();
}

void TestFunc(string &arg1, CScriptString *arg2, double /*d*/, string &arg3)
{
	assert(arg1 == "1");
	assert(arg2->buffer == "2");
	assert(arg3 == "3");

	arg2->Release();
}

CScriptString *str = 0;
CScriptString *TestFunc2()
{
	if( str == 0 )
		str = new CScriptString();

	str->buffer = "Test";

	return str;
}

class A
{
public:
	A() 
	{ 
		str = new CScriptString(); 
	}
	~A() 
	{ 
		str->Release(); 
	}

	CScriptString *getString()
	{
		// The autohandle will increment the refcount
		return str;
	}

	CScriptString *str;
};

bool Test()
{
	RET_ON_MAX_PORT


	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	RegisterScriptString(engine);

	engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
	engine->RegisterGlobalFunction("void TestFunc(string@+, string@, double, string@+)", asFUNCTION(TestFunc), asCALL_CDECL);
	engine->RegisterGlobalFunction("string@+ TestFunc2()", asFUNCTION(TestFunc2), asCALL_CDECL);

	engine->RegisterObjectType("object", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectBehaviour("object", asBEHAVE_CONSTRUCT, "void f(string@+, string@, double, string@+)", asFUNCTION(TestConstructor), asCALL_CDECL_OBJLAST);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = ExecuteString(engine, "TestFunc('1', '2', 1.0f, '3')");
	if( r != 0 ) TEST_FAILED;

	r = ExecuteString(engine, "Assert(TestFunc2() == 'Test')");
	if( r != 0 ) TEST_FAILED;

	r = ExecuteString(engine, "object obj('1', '2', 1.0f, '3')");
	if( r != 0 ) TEST_FAILED;

	// Test autohandle for the return value of a class method
	r = engine->RegisterObjectType("A", 0, asOBJ_REF | asOBJ_NOHANDLE); assert( r >= 0 );
	r = engine->RegisterObjectMethod("A", "string @+ getString() const", asMETHOD(A, getString), asCALL_THISCALL); assert( r >= 0 );
	
	A a;
	r = engine->RegisterGlobalProperty("A a", &a);

	r = ExecuteString(engine, "string @s = a.getString()");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->Release();

	if( str ) 
		str->Release();

	// Success
	return fail;
}

} // namespace

