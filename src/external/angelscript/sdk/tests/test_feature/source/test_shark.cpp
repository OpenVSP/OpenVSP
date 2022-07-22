#include "utils.h"

namespace TestShark
{

static const char * const TESTNAME = "TestShark";

class Point
{
public:
	int iRefCount;
	int x;
	int y;
	Point(void) : iRefCount(1), x(0), y(0) {};
	~Point(void) {};
	inline void Add(Point p) { this->x += p.x; this->y += p.y; };
	inline Point &operator=(const Point &pointOther)
	{
		// leave ref count untouched
		this->x = pointOther.x;
		this->y = pointOther.y;
		return *this;
	}
	inline void AddRef(void) { ++iRefCount; } ;
	inline void Release(void)
	{
		if( --iRefCount == 0 )
		delete this;
	};
};

Point *Point_Factory()
{
	return new Point();
}

void Point_Destruct(Point &point)
{
	point.~Point();
}

Point &Point_Assign(Point &point, Point &pointOther)
{
	point = pointOther;
	return point;
}

void Point_Add(Point &point, Point &p)
{
	point.Add(p);
}

int *Point_Index(Point &point, int iIndex)
{
	if ((iIndex < 0) || (iIndex > 1))
		asGetActiveContext()->SetException("Point index out of range.");
	if (iIndex == 0)
		return &point.x;
	else
		return &point.y;
}

void Point_AddRef(Point &point)
{
	point.AddRef();
}

void Point_Release(Point &point)
{
	point.Release();
}


static const char *script =
"Point AddPoints(Point p1, Point p2) \n"
"{                                   \n"
"Point p3 = p1;                      \n"
"p3.x += p2.x;                       \n"
"p3.y += p2.y;                       \n"
"return p3;                          \n"
"}                                   \n";

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	r = engine->RegisterObjectType("Point", sizeof(Point), asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectProperty("Point", "int x", asOFFSET(Point, x)); assert( r >= 0 );
	r = engine->RegisterObjectProperty("Point", "int y", asOFFSET(Point, y)); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Point", "void Add(Point&in)", asFUNCTION(Point_Add), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Point", asBEHAVE_FACTORY, "Point@ f()", asFUNCTION(Point_Factory), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Point", "Point &opAssign(Point &in)", asFUNCTION(Point_Assign), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Point", "int &opIndex(int)", asFUNCTION(Point_Index), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Point", asBEHAVE_ADDREF, "void f()", asFUNCTION(Point_AddRef), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Point", asBEHAVE_RELEASE, "void f()", asFUNCTION(Point_Release), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

	COutStream out;
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script));
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		PRINTF("%s: Failed to build\n", TESTNAME);
		TEST_FAILED;
	}
	else
	{
		// Internal return
		asIScriptFunction *func = engine->GetModule(0)->GetFunctionByName("AddPoints");
		asIScriptContext *ctx = engine->CreateContext();
		ctx->Prepare(func);
		Point a, b, c;
		a.x = 1; a.y = 1;
		b.x = 2; b.y = 2;
		ctx->SetArgObject(0, &a);
		ctx->SetArgObject(1, &b);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		Point *ret = (Point*)ctx->GetReturnObject();
		c = *ret;
		ctx->Release();
	}

	engine->Release();

	return fail;
}

} // End namespace
