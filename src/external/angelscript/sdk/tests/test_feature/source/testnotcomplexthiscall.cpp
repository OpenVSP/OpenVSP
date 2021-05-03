//
// This test was designed to test the asOBJ_IS_NOT_COMPLEX flag
// and returns from methods.
//
// Author: Andreas Jönsson
//

#include "utils.h"

static const char * const TESTNAME = "TestNotComplexThisCall";

class Class1
{
public:
	asDWORD a;
};

class Class2
{
public:
	asDWORD a;
	asDWORD b;
};

class Class3
{
public:
	asDWORD a;
	asDWORD b;
	asDWORD c;
};

class ClassNotComplex
{
public:
	Class1 notComplex1();
	Class2 notComplex2();
	Class3 notComplex3();
};

Class1 ClassNotComplex::notComplex1()
{
	Class1 c = {0xDEADC0DE};
	return c;
}

Class2 ClassNotComplex::notComplex2()
{
	Class2 c = {0xDEADC0DE, 0x01234567};
	return c;
}

Class3 ClassNotComplex::notComplex3()
{
	Class3 c = {0xDEADC0DE, 0x01234567, 0x89ABCDEF};
	return c;
}

static Class1 c1;
static Class2 c2;
static Class3 c3;

static ClassNotComplex factory;


bool TestNotComplexThisCall()
{
	RET_ON_MAX_PORT

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterObjectType("class1", sizeof(Class1), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class2", sizeof(Class2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class3", sizeof(Class3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("factory", 0, asOBJ_REF | asOBJ_NOHANDLE);

	engine->RegisterGlobalProperty("class1 c1", &c1);
	engine->RegisterGlobalProperty("class2 c2", &c2);
	engine->RegisterGlobalProperty("class3 c3", &c3);
	engine->RegisterGlobalProperty("factory f", &factory);

	engine->RegisterObjectMethod("factory", "class1 notComplex1()", asMETHOD(ClassNotComplex, notComplex1), asCALL_THISCALL);
	engine->RegisterObjectMethod("factory", "class2 notComplex2()", asMETHOD(ClassNotComplex, notComplex2), asCALL_THISCALL);
	engine->RegisterObjectMethod("factory", "class3 notComplex3()", asMETHOD(ClassNotComplex, notComplex3), asCALL_THISCALL);

	COutStream out;

	c1.a = 0;

	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	int r = ExecuteString(engine, "c1 = f.notComplex1();");
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

	r = ExecuteString(engine, "c2 = f.notComplex2();");
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

	r = ExecuteString(engine, "c3 = f.notComplex3();");
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

	engine->Release();

	return fail;
}
