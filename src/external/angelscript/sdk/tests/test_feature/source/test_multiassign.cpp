//
// Tests assigning a value to multiple objects in one statement
//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestMultiAssign
{

static const char * const TESTNAME = "TestMultiAssign";



static const char *script = 
"void Init()            \n"
"{                      \n"
"  a = b = c = d = clr; \n"
"}                      \n";


static asDWORD a, b, c, d, clr;

static asDWORD &Assign(asDWORD &src, asDWORD &dst)
{
	dst = src;
	return dst;
}

static void Assign_gen(asIScriptGeneric *gen)
{
	asDWORD *dst = (asDWORD*)gen->GetObject();
	asDWORD *src = (asDWORD*)gen->GetArgAddress(0);
	*dst = *src;
	gen->SetReturnAddress(dst);
}

bool Test()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterObjectType("CLR", sizeof(asDWORD), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		engine->RegisterObjectMethod("CLR", "CLR &opAssign(CLR &in)", asFUNCTION(Assign_gen), asCALL_GENERIC);
	else
		engine->RegisterObjectMethod("CLR", "CLR &opAssign(CLR &in)", asFUNCTION(Assign), asCALL_CDECL_OBJLAST);


	engine->RegisterGlobalProperty("CLR a", &a);
	engine->RegisterGlobalProperty("CLR b", &b);
	engine->RegisterGlobalProperty("CLR c", &c);
	engine->RegisterGlobalProperty("CLR d", &d);
	engine->RegisterGlobalProperty("CLR clr", &clr);

	a = b = c = d = 0;
	clr = 0x12345678;

	COutStream out;
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	mod->Build();

	ExecuteString(engine, "Init();", mod);

	if( a != 0x12345678 || b != 0x12345678 || c != 0x12345678 || d != 0x12345678 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to assign all objects equally\n", TESTNAME);
	}

	if( clr != 0x12345678 )
	{
		TEST_FAILED;
		PRINTF("%s: Src object changed during operation\n", TESTNAME);
	}

	engine->Release();


	// Success
	return fail;
}

} // namespace

