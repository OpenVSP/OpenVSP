//
// This test was designed to test the asOBJ_CLASS flag with cdecl
//
// Author: Andreas Jonsson
//

#include "utils.h"

static const char * const TESTNAME = "TestCDecl_Class";

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

static Class1 class1()
{
	Class1 c = {0xDEADC0DE};
	return c;
}

static Class2 class2()
{
	Class2 c = {0xDEADC0DE, 0x01234567};
	return c;
}

static Class2_2 class2_2()
{
	Class2_2 c = {0xDEADC0DE01234567L};
	return c;
}


static Class3 class3()
{
	Class3 c = {0xDEADC0DE, 0x01234567, 0x89ABCDEF};
	return c;
}


static Class1 c1;
static Class2 c2;
static Class2_2 c2_2;
static Class3 c3;


static void class1ByVal(Class1 c)
{
	assert( c.a == 0xDEADC0DE );
}

static void class2ByVal(Class2 c)
{
	assert( c.a == 0xDEADC0DE && c.b == 0x01234567 );
}

static void class3ByVal(Class3 c)
{
	assert( c.a == 0xDEADC0DE && c.b == 0x01234567 && c.c == 0x89ABCDEF );
}


// This C style structure seems to be handled differently on Linux 64bit
// http://www.gamedev.net/topic/610537-re-vector3-pod-types-in-c-this-time/
typedef float vec_t;
typedef vec_t vec3_t[3];
typedef struct asvec3_s
{
	vec3_t v;
} asvec3_t;

static asvec3_t vec3_123()
{
	asvec3_t v = {1,2,3};
	return v;
}


static asvec3_t v3;

bool TestCDecl_Class()
{
	RET_ON_MAX_PORT

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	if( sizeof(Class3) != 12 || sizeof(asvec3_t) != 12 )
	{
		PRINTF("sizeof(Class1) == %d\n", (int)sizeof(Class1));
		PRINTF("sizeof(Class2) == %d\n", (int)sizeof(Class2));
		PRINTF("sizeof(Class3) == %d\n", (int)sizeof(Class3));
		PRINTF("sizeof(asvec3_t) == %d\n", (int)sizeof(asvec3_t));
	}

	// On 64bit Linux these types would be returned in RAX:RDX, and must be informed with asOBJ_APP_CLASS_ALLINTS
	engine->RegisterObjectType("class1", sizeof(Class1), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class2", sizeof(Class2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);
	engine->RegisterObjectType("class2_2", sizeof(Class2_2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_ALIGN8);
	engine->RegisterObjectType("class3", sizeof(Class3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);

	engine->RegisterGlobalProperty("class1 c1", &c1);
	engine->RegisterGlobalProperty("class2 c2", &c2);
	engine->RegisterGlobalProperty("class2_2 c2_2", &c2_2);
	engine->RegisterGlobalProperty("class3 c3", &c3);

	engine->RegisterGlobalFunction("class1 _class1()", asFUNCTION(class1), asCALL_CDECL);
	engine->RegisterGlobalFunction("class2 _class2()", asFUNCTION(class2), asCALL_CDECL);
	engine->RegisterGlobalFunction("class2_2 _class2_2()", asFUNCTION(class2_2), asCALL_CDECL);
	engine->RegisterGlobalFunction("class3 _class3()", asFUNCTION(class3), asCALL_CDECL);

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

	c2_2.a = 0;

	r = ExecuteString(engine, "c2_2 = _class2_2();");
	if( r < 0 )
	{
		PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	if( c2_2.a != 0xDEADC0DE01234567L )
	{
		PRINTF("%s: Failed to assign object returned from function. c2_2.a = %X\n", TESTNAME, (unsigned int)c2_2.a);
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

	// Test the vec3 C structure
	// On 64bit Linux this type would be returned in XMM0:XMM1, which is why we need to inform asOBJ_APP_CLASS_ALLFLOATS
	engine->RegisterObjectType("vec3", sizeof(asvec3_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLFLOATS);
	engine->RegisterGlobalProperty("vec3 v3", &v3);
	engine->RegisterGlobalFunction("vec3 vec3_123()", asFUNCTION(vec3_123), asCALL_CDECL);

	v3.v[0] = 0;
	v3.v[1] = 0;
	v3.v[2] = 0;
	r = ExecuteString(engine, "v3 = vec3_123();");
	if( r < 0 )
		TEST_FAILED;
	if( v3.v[0] != 1 || v3.v[1] != 2 || v3.v[2] != 3 )
	{
		PRINTF("%s: Got (%f, %f, %f)\n", TESTNAME, v3.v[0], v3.v[1], v3.v[2]);
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

	Class3 c = class3(); class3ByVal(c);
	r = engine->RegisterGlobalFunction("void class3ByVal(class3)", asFUNCTION(class3ByVal), asCALL_CDECL); assert( r >= 0 );
	r = ExecuteString(engine, "class3 c = _class3(); class3ByVal(c)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	// Test registering a float[3] typedef
	// http://www.gamedev.net/topic/653085-dont-think-im-handling-objects-properly/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		// This type is really an array, and not a structure
		// On MSVC and most likely other compilers too it is always passed by reference
		typedef float vec3_t[3];

		struct helper
		{
			static bool TestArrayByVal( vec3_t a1, vec3_t a2 ) 
			{
				bool ok = true;
				ok &= a1[0] == 1;
				ok &= a1[1] == 2;
				ok &= a1[2] == 3;
				ok &= a2[0] == 4;
				ok &= a2[1] == 5;
				ok &= a2[2] == 6;
				return ok;
			}
		};

#ifdef AS_CAN_USE_CPP11
		asUINT appFlags = asGetTypeTraits<vec3_t>();
		if( appFlags != asOBJ_APP_ARRAY )
			TEST_FAILED;
#endif

		engine->RegisterObjectType("vec3_t", sizeof(vec3_t), asOBJ_VALUE | asOBJ_APP_ARRAY | asOBJ_POD);
		engine->RegisterObjectProperty("vec3_t", "float x", 0);
		engine->RegisterObjectProperty("vec3_t", "float y", 4);
		engine->RegisterObjectProperty("vec3_t", "float z", 8);

		engine->RegisterGlobalFunction("bool TestArrayByVal(vec3_t, vec3_t)", asFUNCTION(helper::TestArrayByVal), asCALL_CDECL);

		vec3_t a1 = {1,2,3}, a2 = {4,5,6};
		engine->RegisterGlobalProperty("vec3_t a1", &a1);
		engine->RegisterGlobalProperty("vec3_t a2", &a2);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void test() { assert( TestArrayByVal(a1, a2) ); } \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		ctx->Prepare(mod->GetFunctionByName("test"));
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();

		engine->Release();
	}

	return fail;
}
