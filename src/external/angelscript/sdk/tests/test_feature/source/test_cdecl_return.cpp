#include "utils.h"

namespace TestCDeclReturn
{

static const char * const TESTNAME = "TestReturn";

bool TestReturnF();
bool TestReturnD();
bool TestReturnStruct();

//---------------------------------------------------

static bool returned = false;

static bool cfunction_b()
{
	return true;
}

static void cfunction_b_gen(asIScriptGeneric *gen) 
{
	*(bool*)gen->GetAddressOfReturnLocation() = true;
}

static bool retfalse()
{
	return false;
}

static int retfalse_fake()
{
	if( sizeof(bool) == 1 )
		// This function is designed to test AS ability to handle bools that may not be returned in full 32 bit values
		return 0x00FFFF00;
	else
		return 0;
}

static void retfalse_fake_gen(asIScriptGeneric *gen)
{
	*(asDWORD*)gen->GetAddressOfReturnLocation() = retfalse_fake();
}

static void retfalse_gen(asIScriptGeneric *gen) 
{
	gen->SetReturnDWord(false);
}

static asINT64 reti64()
{
	return I64(0x102030405);
}

bool Test()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
	engine->RegisterGlobalProperty("bool returned", &returned);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		engine->RegisterGlobalFunction("bool cfunction_b()", asFUNCTION(cfunction_b_gen), asCALL_GENERIC);
		engine->RegisterGlobalFunction("bool retfalse()", asFUNCTION(retfalse_gen), asCALL_GENERIC);
		engine->RegisterGlobalFunction("bool retfalse2()", asFUNCTION(retfalse_fake_gen), asCALL_GENERIC);
	}
	else
	{
		engine->RegisterGlobalFunction("bool cfunction_b()", asFUNCTION(cfunction_b), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool retfalse()", asFUNCTION(retfalse), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool retfalse2()", asFUNCTION(retfalse_fake), asCALL_CDECL);
		engine->RegisterGlobalFunction("int64 reti64()", asFUNCTION(reti64), asCALL_CDECL);
		
		int r = ExecuteString(engine, "assert(reti64() == 0x102030405)");
		if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	}

	int r = ExecuteString(engine, "returned = cfunction_b()");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	if (!returned) 
	{
		PRINTF("\nTestReturn: cfunction didn't return properly\n\n");
		TEST_FAILED;
	}

	r = ExecuteString(engine, "assert(!retfalse() == cfunction_b())");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "assert(retfalse() == false)");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "returned = retfalse()");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	if( returned )
	{
		PRINTF("\nTestReturn: retfalse didn't return properly\n\n");
		TEST_FAILED;
	}

	r = ExecuteString(engine, "assert(!retfalse2() == cfunction_b())");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "assert(retfalse2() == false)");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "returned = retfalse2()");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	if( returned )
	{
		PRINTF("\nTestReturn: retfalse2 didn't return properly\n\n");
		TEST_FAILED;
	}

	engine->Release();
	engine = NULL;

	fail = TestReturnF() || fail;
	fail = TestReturnD() || fail;
	fail = TestReturnStruct() || fail;

	return fail;
}

//----------------------------------------------

static float returnValue_f = 0.0f;

static float cfunction_f() 
{
	return 18.87f;
}

static void cfunction_f_gen(asIScriptGeneric *gen) 
{
	gen->SetReturnFloat(18.87f);
}
bool TestReturnF()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->RegisterGlobalProperty("float returnValue_f", &returnValue_f);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		engine->RegisterGlobalFunction("float cfunction_f()", asFUNCTION(cfunction_f_gen), asCALL_GENERIC);
	else
		engine->RegisterGlobalFunction("float cfunction_f()", asFUNCTION(cfunction_f), asCALL_CDECL);

	int r = ExecuteString(engine, "returnValue_f = cfunction_f()");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;

	if( returnValue_f != 18.87f ) 
	{
		PRINTF("\n%s: cfunction didn't return properly. Expected %f, got %f\n\n", TESTNAME, 18.87f, returnValue_f);
		TEST_FAILED;
	}

	engine->Release();
	
	// Success
	return fail;
}

//-------------------------------------------

static double returnValue_d = 0.0f;

static double cfunction_d() 
{
	return 88.32;
}

static void cfunction_d_gen(asIScriptGeneric *gen) 
{
	gen->SetReturnDouble(88.32);
}

bool TestReturnD()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->RegisterGlobalProperty("double returnValue_d", &returnValue_d);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		engine->RegisterGlobalFunction("double cfunction_d()", asFUNCTION(cfunction_d_gen), asCALL_GENERIC);
	else
		engine->RegisterGlobalFunction("double cfunction_d()", asFUNCTION(cfunction_d), asCALL_CDECL);

	int r = ExecuteString(engine, "returnValue_d = cfunction_d()");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;

	if( returnValue_d != 88.32 ) 
	{
		PRINTF("\n%s: cfunction didn't return properly. Expected %f, got %f\n\n", TESTNAME, 88.32, returnValue_d);
		TEST_FAILED;
	}

	engine->Release();
	
	// Success
	return fail;
}

//-----------------------------------------------

struct asPoint
{
    float x,y;
};

struct asRect
{
    asPoint tl,br;    
};

asPoint TestPoint()
{
    asPoint p={1,2};
    return p;
}

asRect TestRect()
{
    asRect r={{3,4},{5,6}};
    return r;
}

bool TestReturnStruct()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;

    asIScriptEngine *engine=asCreateScriptEngine(ANGELSCRIPT_VERSION);
    r = engine->RegisterObjectType("point", sizeof(asPoint), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS|asOBJ_APP_CLASS_ALLFLOATS); assert( r >= 0 );
    r = engine->RegisterObjectType("rect", sizeof(asRect), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS|asOBJ_APP_CLASS_ALLFLOATS); assert( r >= 0 );
    r = engine->RegisterGlobalFunction("point Point()",asFUNCTION(TestPoint),asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterGlobalFunction("rect Rect()",asFUNCTION(TestRect),asCALL_CDECL); assert( r >= 0 );
    asPoint p={0,0};
    asRect rc={{0,0},{0,0}};
    r = engine->RegisterGlobalProperty("point p",&p); assert( r >= 0 );
    r = engine->RegisterGlobalProperty("rect r",&rc); assert( r >= 0 );
    r = ExecuteString(engine,"p=Point();");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
	if( p.x != 1 || p.y != 2 )
		TEST_FAILED;
    r = ExecuteString(engine,"r=Rect();");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
	if( rc.tl.x != 3 || rc.tl.y != 4 || rc.br.x != 5 || rc.br.y != 6 )
	{
		TEST_FAILED;
		PRINTF("Got {%f, %f, %f, %f}\n", rc.tl.x, rc.tl.y, rc.br.x, rc.br.y);
	}
    
    // at that point, 'p' should contain 1,2 while 'rc' should contain 3,4,5,6
    
    engine->Release();

	return fail;
}


} // end namespace
