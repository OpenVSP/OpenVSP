//
// This test was designed to test the asOBJ_CLASS flag with THISCALL
//
// Author: Andreas Jonsson
//

#include "utils.h"

static const char * const TESTNAME = "TestThiscallClass";

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

class Class2_2
{
public:
	asQWORD a;
};

class Class3
{
public:
	asDWORD a;
	asDWORD b;
	asDWORD c;
};

class Class4
{
public:
	asDWORD a;
	std::string text;

	Class1 class1()
	{
		assert(a == 0x1337C0DE);
		Class1 c = {0xDEADC0DE};
		return c;
	}

	Class2 class2()
	{
		assert(a == 0x1337C0DE);
		Class2 c = {0xDEADC0DE, 0x01234567};
		return c;
	}
	Class2_2 class2_2()
	{
		assert(a == 0x1337C0DE);
		Class2_2 c = {0xDEADC0DE01234567L};
		return c;
	}

	Class3 class3()
	{
		assert(a == 0x1337C0DE);
		Class3 c = {0xDEADC0DE, 0x01234567, 0x89ABCDEF};
		return c;
	}

	void class1ByVal(Class1 c)
	{
		assert(a == 0x1337C0DE);
		assert( c.a == 0xDEADC0DE );
	}

	void class2ByVal(Class2 c)
	{
		assert(a == 0x1337C0DE);
		assert( c.a == 0xDEADC0DE && c.b == 0x01234567 );
	}

	void class2_2ByVal(Class2_2 c)
	{
		assert(a == 0x1337C0DE);
		assert( c.a == 0xDEADC0DE01234567L );
	}

	void class3ByVal(Class3 c)
	{
		assert(a == 0x1337C0DE);
		assert( c.a == 0xDEADC0DE && c.b == 0x01234567 && c.c == 0x89ABCDEF );
	}

	std::string GetText()
	{
		return text;
	}
};

static Class1 c1;
static Class2 c2;
static Class2_2 c2_2;
static Class3 c3;
static Class4 c4;

bool TestThiscallClass()
{
	RET_ON_MAX_PORT

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	RegisterStdString(engine);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	// On 64bit Linux these types would be returned in RAX:RDX, and must be informed with asOBJ_APP_CLASS_ALLINTS
	engine->RegisterObjectType("class1", sizeof(Class1), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class2", sizeof(Class2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class2_2", sizeof(Class2_2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_ALIGN8);
	engine->RegisterObjectType("class3", sizeof(Class3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);

	engine->RegisterObjectType("class4", sizeof(Class4), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectMethod("class4", "class1 class1()", asMETHOD(Class4, class1), asCALL_THISCALL);
	engine->RegisterObjectMethod("class4", "class2 class2()", asMETHOD(Class4, class2), asCALL_THISCALL);
	engine->RegisterObjectMethod("class4", "class2_2 class2_2()", asMETHOD(Class4, class2_2), asCALL_THISCALL);
	engine->RegisterObjectMethod("class4", "class3 class3()", asMETHOD(Class4, class3), asCALL_THISCALL);
	engine->RegisterObjectMethod("class4", "void class1ByVal(class1)", asMETHOD(Class4, class1ByVal), asCALL_THISCALL);
	engine->RegisterObjectMethod("class4", "void class2ByVal(class2)", asMETHOD(Class4, class2ByVal), asCALL_THISCALL);
	engine->RegisterObjectMethod("class4", "void class2_2ByVal(class2_2)", asMETHOD(Class4, class2_2ByVal), asCALL_THISCALL);
	engine->RegisterObjectMethod("class4", "void class3ByVal(class3)", asMETHOD(Class4, class3ByVal), asCALL_THISCALL);
	engine->RegisterObjectMethod("class4", "string GetText()", asMETHOD(Class4, GetText), asCALL_THISCALL);

	engine->RegisterGlobalProperty("class1 c1", &c1);
	engine->RegisterGlobalProperty("class2 c2", &c2);
	engine->RegisterGlobalProperty("class2_2 c2_2", &c2_2);
	engine->RegisterGlobalProperty("class3 c3", &c3);
	engine->RegisterGlobalProperty("class4 c4", &c4);

	c4.a = 0x1337C0DE;
	c4.text = "hello";


	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	c1.a = 0;

	int r = ExecuteString(engine, "c1 = c4.class1();");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c1.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c1.a = %X\n", TESTNAME, (unsigned int)(c1.a));
		TEST_FAILED;
	}


	c2.a = 0;
	c2.b = 0;

	r = ExecuteString(engine, "c2 = c4.class2();");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c2.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c2.a = %X\n", TESTNAME, (unsigned int)(c2.a));
		TEST_FAILED;
	}

	if( c2.b != 0x01234567 )
	{
		PRINTF("%s: Failed to assign object returned from function. c2.b = %X\n", TESTNAME, (unsigned int)(c2.b));
		TEST_FAILED;
	}

	c2_2.a = 0;
	r = ExecuteString(engine, "c2_2 = c4.class2_2();");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c2_2.a != 0xDEADC0DE01234567L )
	{
		PRINTF("%s: Failed to assign object returned from function. c2.a = %llx\n", TESTNAME, c2_2.a);
		TEST_FAILED;
	}

	c3.a = 0;
	c3.b = 0;
	c3.c = 0;

	r = ExecuteString(engine, "c3 = c4.class3();");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c3.a != 0xDEADC0DE )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.a = %X\n", TESTNAME, (unsigned int)(c3.a));
		TEST_FAILED;
	}

	if( c3.b != 0x01234567 )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.b = %X\n", TESTNAME, (unsigned int)(c3.b));
		TEST_FAILED;
	}

	if( c3.c != 0x89ABCDEF )
	{
		PRINTF("%s: Failed to assign object returned from function. c3.c = %X\n", TESTNAME, (unsigned int)(c3.c));
		TEST_FAILED;
	}


	// Test passing the object types by value to a system function
	r = ExecuteString(engine, "class1 c = c4.class1(); c4.class1ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ExecuteString(engine, "class2 c = c4.class2(); c4.class2ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ExecuteString(engine, "class2_2 c = c4.class2_2(); c4.class2_2ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ExecuteString(engine, "class3 c = c4.class3(); c4.class3ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	// Test thiscall that returns a complex class
	r = ExecuteString(engine, "assert( c4.GetText() == 'hello' );");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	return fail;
}
