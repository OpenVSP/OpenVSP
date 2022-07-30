//
// This test was designed to test the asOBJ_CLASS_C flag with cdecl
//
// Author: Andreas Jonsson
//

#include "utils.h"

static const char * const TESTNAME = "TestCDecl_ClassC";

// A complex class is a class that has a defined constructor or
// destructor, or an overridden assignment operator. Compilers
// normally treat these classes differently in that they are
// returned by reference even though they are small enough to
// fit in registers. This is because of the need of exception
// handling in case something goes wrong.

class ClassC1
{
public:
	ClassC1() {a = 0xDEADC0DE;}
	asDWORD a;
};

class ClassC2
{
public:
	ClassC2() {a = 0xDEADC0DE; b = 0x01234567;}
	asDWORD a;
	asDWORD b;
};

class ClassC3
{
public:
	ClassC3() {a = 0xDEADC0DE; b = 0x01234567; c = 0x89ABCDEF;}
	asDWORD a;
	asDWORD b;
	asDWORD c;
};

static ClassC1 classC1()
{
	ClassC1 c;
	return c;
}

static ClassC2 classC2()
{
	ClassC2 c;
	return c;
}

static ClassC3 classC3()
{
	ClassC3 c;
	return c;
}

static ClassC1 c1;
static ClassC2 c2;
static ClassC3 c3;

static void class1ByVal(ClassC1 c)
{
	assert( c.a == 0xDEADC0DE );
}

static void class2ByVal(ClassC2 c)
{
	assert( c.a == 0xDEADC0DE && c.b == 0x01234567 );
}

static void class3ByVal(ClassC3 c)
{
	assert( c.a == 0xDEADC0DE && c.b == 0x01234567 && c.c == 0x89ABCDEF );
}

bool TestCDecl_ClassC()
{
	RET_ON_MAX_PORT

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterObjectType("class1", sizeof(ClassC1), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class2", sizeof(ClassC2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class3", sizeof(ClassC3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C | asOBJ_APP_CLASS_ALLINTS);

	engine->RegisterGlobalProperty("class1 c1", &c1);
	engine->RegisterGlobalProperty("class2 c2", &c2);
	engine->RegisterGlobalProperty("class3 c3", &c3);

	engine->RegisterGlobalFunction("class1 _class1()", asFUNCTION(classC1), asCALL_CDECL);
	engine->RegisterGlobalFunction("class2 _class2()", asFUNCTION(classC2), asCALL_CDECL);
	engine->RegisterGlobalFunction("class3 _class3()", asFUNCTION(classC3), asCALL_CDECL);

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
		PRINTF("%s: Failed to assign complex object returned from function. c1.a = %X\n", TESTNAME, (unsigned int)c1.a);
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
		PRINTF("%s: Failed to assign complex object returned from function. c2.a = %X\n", TESTNAME, (unsigned int)c2.a);
		TEST_FAILED;
	}

	if( c2.b != 0x01234567 )
	{
		PRINTF("%s: Failed to assign complex object returned from function. c2.b = %X\n", TESTNAME, (unsigned int)c2.b);
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
		PRINTF("%s: Failed to assign complex object returned from function. c3.a = %X\n", TESTNAME, (unsigned int)c3.a);
		TEST_FAILED;
	}

	if( c3.b != 0x01234567 )
	{
		PRINTF("%s: Failed to assign complex object returned from function. c3.b = %X\n", TESTNAME, (unsigned int)c3.b);
		TEST_FAILED;
	}

	if( c3.c != 0x89ABCDEF )
	{
		PRINTF("%s: Failed to assign complex object returned from function. c3.c = %X\n", TESTNAME, (unsigned int)c3.c);
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

	engine->Release();

	return fail;
}
