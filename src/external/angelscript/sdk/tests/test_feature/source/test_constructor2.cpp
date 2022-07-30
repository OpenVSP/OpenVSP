// Written by Alain "abrken" Bridel on October 12th, 2004

#include "utils.h"
using namespace std;

namespace TestConstructor2
{

static const char * const TESTNAME = "TestConstructor2";

class CMyObj {
public :
	CMyObj() {};
	virtual ~CMyObj() {};
};

void ConstrMyObj(CMyObj &obj)
{
	new(&obj) CMyObj();
}

void DestrMyObj(CMyObj &obj)
{
	obj.~CMyObj();
}

class CMySecondObj {
	CMyObj *m_myObj;
public :
	CMySecondObj(CMyObj *in_myObj = NULL) {m_myObj = in_myObj;};
	virtual ~CMySecondObj()  {};
};

void ConstrMySecondObj(CMySecondObj &obj)
{
	new(&obj) CMySecondObj();
}

void ConstrMySecondObj(CMyObj &o, CMySecondObj &obj)
{
	new(&obj) CMySecondObj(&o);
}

void DestrMySecondObj(CMySecondObj &obj)
{
	obj.~CMySecondObj();
}

const char *script1 =
"class InternalClass            \n"
"{                              \n"
"	InternalClass()             \n"
"	{                           \n"
"		m_x = 3;                \n"
"       m_y = 773456;           \n"
"	}                           \n"
"                               \n"
"	int8 m_x;                   \n"
"   int m_y;                    \n"
"}                              \n"
"class MyClass                  \n"
"{                              \n"
"	MyClass()                   \n"
"	{                           \n"
"      m_c = InternalClass();   \n"
"	}                           \n"
"	void Test()                 \n"
"	{                           \n"
"		Assert( m_c.m_x == 3 ); \n"
"       Assert( m_c.m_y == 773456 ); \n"
"	}                           \n"
"	InternalClass m_c;          \n"
"}                              \n";

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;

	asIScriptEngine *engine;
	int r;
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->RegisterObjectType("MyObj", sizeof(CMyObj), asOBJ_VALUE | asOBJ_APP_CLASS); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("MyObj", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstrMyObj), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("MyObj", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestrMyObj), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectType("MySecondObj", sizeof(CMySecondObj), asOBJ_VALUE | asOBJ_APP_CLASS); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("MySecondObj", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ConstrMySecondObj, (CMySecondObj &), void), asCALL_CDECL_OBJLAST);	assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("MySecondObj", asBEHAVE_CONSTRUCT, "void f(MyObj &in)", asFUNCTIONPR(ConstrMySecondObj, (CMyObj &, CMySecondObj &), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("MySecondObj", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestrMySecondObj), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	ExecuteString(engine, "MyObj obj; {MySecondObj secObj(obj);}");
	engine->Release();


	// test to make sure default constructors of script classes are called
	COutStream out;
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC ); assert( r >= 0 );
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", script1, strlen(script1));
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile\n", TESTNAME);
	}

	r = ExecuteString(engine, "{ MyClass test; test.Test(); }", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->Release();


	// Success
	return fail;
}


}
