//
// Tests constant properties to see if they can be overwritten
//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestConstProperty
{

static const char * const TESTNAME = "TestConstProperty";

class CVec3
{
public:
	CVec3() {}
	CVec3(const CVec3 &o) : x(o.x), y(o.y), z(o.z) {}
	CVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	float x,y,z;
};

class CObj
{
public:
	CVec3 simplevec;
	CVec3 constvec;

	CObj() {}
	~CObj() {}
};

CVec3 vec3add(const CVec3& v1, const CVec3& v2)
{
	return CVec3(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z);
}

static const char *script =
"void Init()          \n"
"{                    \n"
"  CObj someObj;      \n"
"  CVec3 someVec;     \n"
"  someVec = someObj.simplevec + someObj.constvec; \n"
"  someVec = vec3add(someObj.simplevec,someObj.constvec); \n"
"}                    \n";

static const char *script2 =
//"Obj1 myObj1;         \n"
//"Obj2 myObj2;         \n"
"float myFloat;       \n"
"                     \n"
"void Init()          \n"
"{                    \n"
//"  g_Obj1 = myObj1;   \n"
//"  g_Obj2 = myObj2;   \n"
"  g_Float = myFloat; \n"
"}                    \n";


bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;

	// TEST 1
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->RegisterObjectType("CVec3", sizeof(CVec3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CK); assert( r >= 0 );
	r = engine->RegisterObjectProperty("CVec3", "float x", asOFFSET(CVec3,x)); assert( r >= 0 );
	r = engine->RegisterObjectProperty("CVec3", "float y", asOFFSET(CVec3,y)); assert( r >= 0 );
	r = engine->RegisterObjectProperty("CVec3", "float z", asOFFSET(CVec3,z)); assert( r >= 0 );

	r = engine->RegisterObjectMethod("CVec3", "CVec3 opAdd(const CVec3 &in) const", asFUNCTION(vec3add), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

	r = engine->RegisterGlobalFunction("CVec3 vec3add(const CVec3 &in, const CVec3 &in)", asFUNCTION(vec3add), asCALL_CDECL); assert( r >= 0 );

	r = engine->RegisterObjectType("CObj", sizeof(CObj), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CD); assert( r >= 0 );
	r = engine->RegisterObjectProperty("CObj", "CVec3 simplevec", asOFFSET(CObj,simplevec)); assert( r >= 0 );
	r = engine->RegisterObjectProperty("CObj", "const CVec3 constvec", asOFFSET(CObj,constvec)); assert( r >= 0 );

	CBufferedOutStream out;
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &out, asCALL_THISCALL);
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	mod->Build();

	if( !out.buffer.empty() )
	{
		PRINTF("%s: Failed to pass argument as 'const type &in'\n%s", TESTNAME, out.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	// TEST 2
 	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterObjectType("Obj1", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectProperty("Obj1", "int val", 0);
	engine->RegisterObjectMethod("Obj1", "Obj1 &opAssign(Obj1 &in)", asFUNCTION(0), asCALL_GENERIC);

	engine->RegisterObjectType("Obj2", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectProperty("Obj2", "int val", 0);

//	int constantProperty1 = 0;
//	engine->RegisterGlobalProperty("const Obj1 g_Obj1", &constantProperty1);

//	int constantProperty2 = 0;
//	engine->RegisterGlobalProperty("const Obj2 g_Obj2", &constantProperty2);

	float constantFloat = 0;
	engine->RegisterGlobalProperty("const float g_Float", &constantFloat);

	out.buffer = "";
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &out, asCALL_THISCALL);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	mod->Build();

	if( out.buffer != "TestConstProperty (3, 1) : Info    : Compiling void Init()\n"
		              "TestConstProperty (5, 11) : Error   : Reference is read-only\n" )
	{
		PRINTF("%s: Failed to detect all properties as constant\n%s", TESTNAME, out.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	// Test const property with identity comparison
	// http://www.gamedev.net/topic/662239-issue-with-implicit-const-qualifier-on-member-handle-vs-object/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &out, asCALL_THISCALL);
		out.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"class Interface {}; \n"
			"class Object : Interface {}; \n"
			"class Test \n"
			"{ \n"
			"    bool PerformTest() const \n"
			"    { \n"
			"        bool a = m_interface is @m_object; \n"
			"        bool b = m_interface is m_object; \n"
			"        bool c = @m_interface is @m_object; \n"
			"        bool d = @m_interface is m_object; \n"
			"        return a; \n"
			"    } \n"
			"    private Interface@ m_interface; \n"
			"    private Object m_object; \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( out.buffer != "" )
		{
			PRINTF("%s", out.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

