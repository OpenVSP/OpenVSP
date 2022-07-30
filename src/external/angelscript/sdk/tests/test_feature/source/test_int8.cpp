#include "utils.h"

namespace TestInt8
{

static const char * const TESTNAME = "TestInt8";

char RetInt8(char in)
{
	if( in != 1 )
	{
		PRINTF("failed to pass parameter correctly\n");
	}
	return 1;
}

class TestInt8Class
{
public:
	TestInt8Class()
	{
		m_fail = false;
	}

	void Test1(char value)
	{
		if( value != 1 )
		{
			m_fail = true;
		}
	}

	void Test0(char value)
	{
		if( value != 0 )
		{
			m_fail = true;
		}
	}

	bool m_fail;
};

static const char *script3 =
"void TestInt8ToMember()           \n"
"{                                 \n"
"   int8 flag = 1;                 \n"
"   TestInt8Class.Test1(flag);     \n"
"   flag = 0;                      \n"
"   TestInt8Class.Test0(flag);     \n"
"}                                 \n";


bool Test()
{
	RET_ON_MAX_PORT

	int r;
	bool fail = false;
	COutStream out;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, true);
	RegisterScriptString(engine);
	engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	// We'll test two things with this function
	// 1. The native interface is able to pass byte parameters correctly
	// 2. The native interface is able to return byte values correctly
	engine->RegisterGlobalFunction("int8 RetInt8(int8)", asFUNCTION(RetInt8), asCALL_CDECL);

	char var = 0;
	engine->RegisterGlobalProperty("int8 gvar", &var);

	ExecuteString(engine, "gvar = RetInt8(1)");
	if( var != 1 )
	{
		PRINTF("failed to return value correctly\n");
		TEST_FAILED;
	}
	
	ExecuteString(engine, "Assert(RetInt8(1) == 1)");

	
	// Test to make sure bools can be passed to member functions properly
	engine->RegisterObjectType("Int8Tester", 0, asOBJ_REF | asOBJ_NOHANDLE);
	engine->RegisterObjectMethod("Int8Tester", "void Test1(int8)", asMETHOD(TestInt8Class, Test1), asCALL_THISCALL);
	engine->RegisterObjectMethod("Int8Tester", "void Test0(int8)", asMETHOD(TestInt8Class, Test0), asCALL_THISCALL);	
	TestInt8Class testInt8;
	r = engine->RegisterGlobalProperty("Int8Tester TestInt8Class", &testInt8 );
	if( r < 0 ) TEST_FAILED;
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script3, strlen(script3));
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}
	else
	{
		r = ExecuteString(engine, "TestInt8ToMember();", mod);
		if( r != asEXECUTION_FINISHED ) TEST_FAILED;

		if( testInt8.m_fail ) TEST_FAILED;
	}

	// Shift operations with int8 should result in int32
	r = ExecuteString(engine, "uint8[] buf={1,2,3,4,5,6}; "
                                 "uint32 ver; "
                                 "ver = buf[0]; "
                                 "ver |= buf[1]<<8; "
                                 "ver |= buf[2]<<16; "
                                 "ver |= buf[3]<<24; "
								 "Assert(ver == 0x04030201);"); // If this is changed to 0x01020304 Avira accuses the compiled obj file as virus
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}


	engine->Release();

	return fail;
}

} // namespace

