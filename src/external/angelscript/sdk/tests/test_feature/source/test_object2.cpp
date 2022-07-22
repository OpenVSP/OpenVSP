#include "utils.h"

namespace TestObject2
{

static const char * const TESTNAME = "TestObject2";




class CGuiButton
{
public:
	CGuiButton() {refCount = 1;}
	~CGuiButton() {}
	void AddRef() {refCount++;}
	void Release() {refCount--; if( refCount == 0 ) delete this;}

	void SetName(const std::string &) {}

	int refCount;
};

CGuiButton *CGuiButton_Factory()
{
	return new CGuiButton();
}

class CVector2
{
public:
	CVector2() {}
	CVector2(int a, int b) {_a = a; _b = b;}

	int _a;
	int _b;
};

void CVector2_Construct(CVector2 *o)
{
	new(o) CVector2();
}

void CVector2_Construct(int a, int b, CVector2 *o)
{
	new(o) CVector2(a, b);
}

int GUI;
CGuiButton button;
CGuiButton* Gui_AddButton(const std::string& , const CVector2& , const CVector2& , int *)
{
	return &button;
}

CGuiButton* Gui_GetButton(const std::string& )
{
	return &button;
}


bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	COutStream out;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptString(engine);

	engine->RegisterObjectType("Vector2", sizeof(CVector2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C);
	engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(CVector2_Construct, (CVector2*), void), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(int,int)", asFUNCTIONPR(CVector2_Construct, (int, int, CVector2*), void), asCALL_CDECL_OBJLAST);

	engine->RegisterObjectType("GuiButton", sizeof(CGuiButton), asOBJ_REF);	
	engine->RegisterObjectBehaviour("GuiButton", asBEHAVE_FACTORY, "GuiButton@ f()", asFUNCTION(CGuiButton_Factory), asCALL_CDECL);
	engine->RegisterObjectBehaviour("GuiButton", asBEHAVE_ADDREF, "void f()", asMETHOD(CGuiButton, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour("GuiButton", asBEHAVE_RELEASE, "void f()", asMETHOD(CGuiButton, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod("GuiButton", "void SetName(string@+)", asMETHOD(CGuiButton, SetName), asCALL_THISCALL);

	engine->RegisterObjectType("Gui", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectMethod("Gui", "GuiButton& AddButton(const string& in, const Vector2& in, const Vector2& in)", asFUNCTION(Gui_AddButton), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Gui", "GuiButton& GetButton(string@+)", asFUNCTION(Gui_GetButton), asCALL_CDECL_OBJLAST);
	engine->RegisterGlobalProperty("Gui GUI", &GUI);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	const char *script1 =
		"void TestObject2()                                                                          \n"
		"{                                                                                           \n"
		"  GuiButton@ btn = GUI.AddButton('Hello world 3!', Vector2(200, 50), Vector2(100, 50));     \n"
		"  GUI.GetButton('Test').SetName('Test2');                                                   \n"
		"}                                                                                           \n";
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	r = ExecuteString(engine, "TestObject2()", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	// Success
	return fail;
}

} // namespace

