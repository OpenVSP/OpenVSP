//
// This test was designed to test returns with asCALL_CDECL_OBJFIRST
//
// Author: Andreas Jönsson
//

#include "utils.h"

static const char * const TESTNAME = "TestReturnWithCDeclObjFirst";

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

class ClassNotComplex {};
static ClassNotComplex factory;

static Class1 notComplex1(ClassNotComplex *obj, int a)
{
	assert(obj == &factory);
	Class1 C = {asDWORD(a)};
	return C;
}

static Class2 notComplex2(ClassNotComplex *obj, int a, int b)
{
	assert(obj == &factory);
	Class2 C = {asDWORD(a), asDWORD(b)};
	return C;
}

static Class3 notComplex3(ClassNotComplex *obj, int a, int b, int c)
{
	assert(obj == &factory);
	Class3 C = {asDWORD(a), asDWORD(b), asDWORD(c)};
	return C;
}

static Class1 c1;
static Class2 c2;
static Class3 c3;



bool TestReturnWithCDeclObjFirst()
{
	RET_ON_MAX_PORT

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	// On Linux 64bit it is necessary to inform angelscript that the class
	// contains only integers, otherwise it will not be possible to know how
	// the type is returned (either RAX:RDX, or XMM0:XMM1)
	engine->RegisterObjectType("class1", sizeof(Class1), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class2", sizeof(Class2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class3", sizeof(Class3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("factory", 0, asOBJ_REF | asOBJ_NOHANDLE);

	engine->RegisterGlobalProperty("class1 c1", &c1);
	engine->RegisterGlobalProperty("class2 c2", &c2);
	engine->RegisterGlobalProperty("class3 c3", &c3);
	engine->RegisterGlobalProperty("factory f", &factory);

	engine->RegisterObjectMethod("factory", "class1 notComplex1(int)", asFUNCTION(notComplex1), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectMethod("factory", "class2 notComplex2(int, int)", asFUNCTION(notComplex2), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectMethod("factory", "class3 notComplex3(int, int, int)", asFUNCTION(notComplex3), asCALL_CDECL_OBJFIRST);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	c1.a = 0;
	int r = 0;
	r = ExecuteString(engine, "c1 = f.notComplex1(int(0xDEADC0DE));");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c1.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c1.a = %X\n", TESTNAME, c1.a);
		TEST_FAILED;
	}

	c2.a = 0;
	c2.b = 0;

	r = ExecuteString(engine, "c2 = f.notComplex2(int(0xDEADC0DE), 0x01234567);");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c2.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c2.a = %X\n", TESTNAME, c2.a);
		TEST_FAILED;
	}

	if( c2.b != 0x01234567 )
	{
		PRINTF("%s: Failed to assign object returned from function. c2.b = %X\n", TESTNAME, c2.b);
		TEST_FAILED;
	}

	c3.a = 0;
	c3.b = 0;
	c3.c = 0;

	r = ExecuteString(engine, "c3 = f.notComplex3(int(0xDEADC0DE), 0x01234567, int(0x89ABCDEF));");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c3.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.a = %X\n", TESTNAME, c3.a);
		TEST_FAILED;
	}

	if( c3.b != 0x01234567 )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.b = %X\n", TESTNAME, c3.b);
		TEST_FAILED;
	}

	if( c3.c != 0x89ABCDEF )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.c = %X\n", TESTNAME, c3.c);
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}
