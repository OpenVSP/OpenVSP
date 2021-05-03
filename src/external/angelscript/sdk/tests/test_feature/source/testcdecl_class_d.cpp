//
// This test was designed to test the asOBJ_CLASS_D flag with cdecl
//
// Author: Andreas Jonsson
//

#include "utils.h"

static const char * const TESTNAME = "TestCDecl_ClassD";

class ClassD1
{
public:
	~ClassD1() {a = 0;}
	asDWORD a;
};

class ClassD2
{
public:
	~ClassD2() {a = 0; b = 0;}
	asDWORD a;
	asDWORD b;
};

class ClassD3
{
public:
	~ClassD3() {a = 0; b = 0; c = 0;}
	asDWORD a;
	asDWORD b;
	asDWORD c;
};

static ClassD1 classD1()
{
	ClassD1 c = {0xDEADC0DE};
	return c;
}

static ClassD2 classD2()
{
	ClassD2 c = {0xDEADC0DE, 0x01234567};
	return c;
}

static ClassD3 classD3()
{
	ClassD3 c = {0xDEADC0DE, 0x01234567, 0x89ABCDEF};
	return c;
}

static ClassD1 c1;
static ClassD2 c2;
static ClassD3 c3;

static void class1ByVal(ClassD1 c)
{
	assert( c.a == 0xDEADC0DE );
}

static void class2ByVal(ClassD2 c)
{
	assert( c.a == 0xDEADC0DE && c.b == 0x01234567 );
}

static void class3ByVal(ClassD3 c)
{
	assert( c.a == 0xDEADC0DE && c.b == 0x01234567 && c.c == 0x89ABCDEF );
}

static ClassD2 test(ClassD2 c)
{
	return c;
}

bool TestCDecl_ClassD()
{
	RET_ON_MAX_PORT

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterObjectType("class1", sizeof(ClassD1), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_D);
	engine->RegisterObjectType("class2", sizeof(ClassD2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_D);
	engine->RegisterObjectType("class3", sizeof(ClassD3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_D);

	engine->RegisterGlobalProperty("class1 c1", &c1);
	engine->RegisterGlobalProperty("class2 c2", &c2);
	engine->RegisterGlobalProperty("class3 c3", &c3);

	engine->RegisterGlobalFunction("class1 _class1()", asFUNCTION(classD1), asCALL_CDECL);
	engine->RegisterGlobalFunction("class2 _class2()", asFUNCTION(classD2), asCALL_CDECL);
	engine->RegisterGlobalFunction("class3 _class3()", asFUNCTION(classD3), asCALL_CDECL);


	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	c1.a = 0;

	int r = ExecuteString(engine, "c1 = _class1();");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c1.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c1.a = %X\n", TESTNAME, (unsigned int)c1.a);
		TEST_FAILED;
	}


	c2.a = 0;
	c2.b = 0;

	r = ExecuteString(engine, "c2 = _class2();");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c2.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c2.a = %X\n", TESTNAME, (unsigned int)c2.a);
		TEST_FAILED;
	}

	if( c2.b != 0x01234567 )
	{
		PRINTF("%s: Failed to assign object returned from function. c2.b = %X\n", TESTNAME, (unsigned int)c2.b);
		TEST_FAILED;
	}

	c3.a = 0;
	c3.b = 0;
	c3.c = 0;

	r = ExecuteString(engine, "c3 = _class3();");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c3.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.a = %X\n", TESTNAME, (unsigned int)c3.a);
		TEST_FAILED;
	}

	if( c3.b != 0x01234567 )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.b = %X\n", TESTNAME, (unsigned int)c3.b);
		TEST_FAILED;
	}

	if( c3.c != 0x89ABCDEF )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.c = %X\n", TESTNAME, (unsigned int)c3.c);
		TEST_FAILED;
	}

	// Test passing the object types by value to a system function
	r = engine->RegisterGlobalFunction("void class1ByVal(class1)", asFUNCTION(class1ByVal), asCALL_CDECL); assert( r >= 0 );
	r = ExecuteString(engine, "class1 c = _class1(); class1ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = engine->RegisterGlobalFunction("void class2ByVal(class2)", asFUNCTION(class2ByVal), asCALL_CDECL); assert( r >= 0 );
	r = ExecuteString(engine, "class2 c = _class2(); class2ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = engine->RegisterGlobalFunction("void class3ByVal(class3)", asFUNCTION(class3ByVal), asCALL_CDECL); assert( r >= 0 );
	r = ExecuteString(engine, "class3 c = _class3(); class3ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	// Test returning the same object that was passed in
	// http://www.gamedev.net/topic/667966-calling-convention-bug/
	r = engine->RegisterGlobalFunction("class2 test(class2)", asFUNCTION(test), asCALL_CDECL); assert( r >= 0 );
	r = ExecuteString(engine, "class2 c = _class2(); c = test(c); class2ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	return fail;
}
