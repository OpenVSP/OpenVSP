#include "utils.h"

using namespace std;

namespace TestDict
{

static const char * const TESTNAME = "TestDict";



static const char *script1 =
"void TestDict()                   \n"
"{                                 \n"
"   Dict d;                        \n"
"   d[\"test\\n\"];                \n"
"}                                 \n";

class CDict
{
public:
	CDict() {}
	~CDict() {}

	CDict &operator=(const CDict &) { return *this; }

	CDict &operator[](string s) 
	{ 
//		PRINTF(s.c_str()); 
		return *this;
	}
};

void Construct(CDict *o)
{
	new(o) CDict();
}

void Destruct(CDict *o)
{
	o->~CDict();
}

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterStdString(engine);

	engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	engine->RegisterObjectType("Dict", sizeof(CDict), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);	
	engine->RegisterObjectBehaviour("Dict", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Construct), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("Dict", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Destruct), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Dict", "Dict &opAssign(const Dict &in)", asMETHOD(CDict,operator=), asCALL_THISCALL);

	engine->RegisterObjectMethod("Dict", "Dict &opIndex(string)", asMETHOD(CDict, operator[]), asCALL_THISCALL);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	asIScriptContext *ctx = engine->CreateContext();
	r = ExecuteString(engine, "TestDict()", mod, ctx);
	if( r != asEXECUTION_FINISHED )
	{
		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());

		PRINTF("%s: Failed to execute script\n", TESTNAME);
		TEST_FAILED;
	}
	if( ctx ) ctx->Release();

	engine->Release();

	// Success
	return fail;
}

} // namespace

