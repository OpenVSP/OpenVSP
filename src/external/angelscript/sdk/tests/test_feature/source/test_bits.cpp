#include "utils.h"

namespace TestBits
{

// uint8 ReturnByte(uint8)
void ReturnByte(asIScriptGeneric *gen)
{
	asBYTE b = *(asBYTE*)gen->GetAddressOfArg(0);
	// Return a full dword, even though AngelScript should only use a byte
#ifdef __BIG_ENDIAN__
	if( b )
		*(asDWORD*)gen->GetAddressOfReturnLocation() = 0x00000000 | (int(b)<<24);
	else
		*(asDWORD*)gen->GetAddressOfReturnLocation() = 0x00FFFFFF | (int(b)<<24);
#else
	if( b )
		*(asDWORD*)gen->GetAddressOfReturnLocation() = 0x00000000 | b;
	else
		*(asDWORD*)gen->GetAddressOfReturnLocation() = 0xFFFFFF00 | b;
#endif
}

// uint16 ReturnWord(uint16)
void ReturnWord(asIScriptGeneric *gen)
{
	asWORD w = *(asWORD*)gen->GetAddressOfArg(0);
	// Return a full dword, even though AngelScript should only use a word
#ifdef __BIG_ENDIAN__
	if( w )
		*(asDWORD*)gen->GetAddressOfReturnLocation() = 0x00000000 | (int(w)<<16);
	else
		*(asDWORD*)gen->GetAddressOfReturnLocation() = 0X0000FFFF | (int(w)<<16);
#else
	if( w )
		*(asDWORD*)gen->GetAddressOfReturnLocation() = 0x00000000 | w;
	else
		*(asDWORD*)gen->GetAddressOfReturnLocation() = 0XFFFF0000 | w;
#endif
}

bool Test2();

bool Test()
{
	bool fail = Test2();
	int r;
	COutStream out;
	CBufferedOutStream bout;

	// Disallow bitwise operators on non-integer types
	// https://www.gamedev.net/forums/topic/693281-bitwise-operator/
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = ExecuteString(engine, "float a = 1; a = a << 1;");
		if (r >= 0)
			TEST_FAILED;
		r = ExecuteString(engine, "double b = 1; b = b ^ 1;");
		if (r >= 0)
			TEST_FAILED;
		r = ExecuteString(engine, "bool c = true; c = c | 1;");
		if (r >= 0)
			TEST_FAILED;
		
		if (bout.buffer != "ExecuteString (1, 20) : Error   : Illegal operation on 'float'\n"
						   "ExecuteString (1, 21) : Error   : Illegal operation on 'double'\n"
						   "ExecuteString (1, 22) : Error   : No conversion from 'bool' to 'int' available.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Ordinary tests
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptString(engine);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script =
			"const uint8	mask0=1;      \n"
			"const uint8	mask1=1<<1;   \n"
			"const uint8	mask2=1<<2;   \n"
			"const uint8	mask3=1<<3;   \n"
			"const uint8	mask4=1<<4;   \n"
			"const uint8	mask5=1<<5;   \n"
			"void BitsTest(uint8 b)       \n"
			"{                            \n"
			"  Assert((b&mask4) == 0);    \n"
			"}                            \n";
		mod->AddScriptSection("script", script, strlen(script));
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		r = ExecuteString(engine, "uint8 newmask = 0xFF, mask = 0x15; Assert( (newmask & ~mask) == 0xEA );");
		if (r != asEXECUTION_FINISHED) TEST_FAILED;

		r = ExecuteString(engine, "uint8 newmask = 0xFF; newmask = newmask & (~mask2) & (~mask3) & (~mask5); Assert( newmask == 0xD3 );", mod);
		if (r != asEXECUTION_FINISHED) TEST_FAILED;

		r = ExecuteString(engine, "uint8 newmask = 0XFE; Assert( (newmask & mask0) == 0 );", mod);
		if (r != asEXECUTION_FINISHED) TEST_FAILED;

		r = ExecuteString(engine, "uint8 b = 0xFF; b &= ~mask4; BitsTest(b);", mod);
		if (r != asEXECUTION_FINISHED) TEST_FAILED;

		// Test octals
		r = ExecuteString(engine, "uint b = 0o777; Assert( b == 0x1FF );");
		if (r != asEXECUTION_FINISHED) TEST_FAILED;

		// Test binaries
		r = ExecuteString(engine, "uint b = 0b10101010; Assert( b == 0xAA );");
		if (r != asEXECUTION_FINISHED) TEST_FAILED;

		// Test decimals
		r = ExecuteString(engine, "uint b = 0d255; Assert( b == 0xFF );");
		if (r != asEXECUTION_FINISHED) TEST_FAILED;

		engine->RegisterGlobalFunction("uint8 ReturnByte(uint8)", asFUNCTION(ReturnByte), asCALL_GENERIC);
		engine->RegisterGlobalFunction("uint16 ReturnWord(uint16)", asFUNCTION(ReturnWord), asCALL_GENERIC);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script2 =
			"uint8 gb;              \n"
			"uint16 gw;             \n"
			"void Test()            \n"
			"{                      \n"
			"  gb = ReturnByte(1);  \n"
			"  Assert(gb == 1);     \n"
			"  gb = ReturnByte(0);  \n"
			"  Assert(gb == 0);     \n"
			"  gw = ReturnWord(1);  \n"
			"  Assert(gw == 1);     \n"
			"  gw = ReturnWord(0);  \n"
			"  Assert(gw == 0);     \n"
			"}                      \n";
		mod->AddScriptSection("script", script2, strlen(script2));
		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		r = ExecuteString(engine, "Test()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// bitwise operators should maintain signed/unsigned type of left hand operand
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		r = ExecuteString(engine, "int a = 0, b = 0; bool c = (a < (b>>1));");
		if (r < 0)
			TEST_FAILED;
		r = ExecuteString(engine, "uint a = 0, b = 0; bool c = (a < (b>>1));");
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Success
	return fail;
}

// Reported by jkhax0r
bool Test2()
{
	const char *script = 
	"uint16 Z1; \n"
	"uint8 b1 = 2; \n"
	"uint8 b2 = 2; \n"

	//Using '+'
	"Z1 = (b1 & 0x00FF) + ((b2 << 8) & 0xFF00); \n"
	"b1 = Z1 & 0x00FF; \n"
	"b2 = (Z1 >> 8) & 0x00FF; \n"
	"assert( b1 == 2 && b2 == 2 ); \n"

	//Using '|'
	"Z1 = (b1 & 0x00FF) | ((b2 << 8) & 0xFF00); \n"
	"b1 = Z1 & 0x00FF; \n"
	"b2 = (Z1 >> 8) & 0x00FF;  \n"
	"assert( b1 == 2 && b2 == 2 ); \n";

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	int r = ExecuteString(engine, script);
	if( r != asEXECUTION_FINISHED ) 
	{
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

} // namespace

