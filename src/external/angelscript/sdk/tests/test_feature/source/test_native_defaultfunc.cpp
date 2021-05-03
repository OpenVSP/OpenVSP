#include "utils.h"

#define TEST_GLM 0

#if TEST_GLM == 0
#include "glm_vec2.h"
#else
#include "D:\Projects\AngelScript\glm-master\glm\glm.hpp"
#include "D:\Projects\AngelScript\glm-master\glm\gtc\quaternion.hpp"
#endif

namespace Test_Native_DefaultFunc
{

// class without constructors and operators
class Class1
{
public:
	union {
		float x, s;
	};
	union {
		float y, t;
	};
};

// Class with defined constructors and operators
class Class2
{
public:
	Class2() { x = 0; y = 0; }
	Class2(const Class2& o) { x = o.x; y = o.y; }
	Class2(float x, float y) { this->x = x; this->y = y; }
	Class2& operator=(const Class2& o) { x = o.x; y = o.y; return *this; }
	union {
		float x, s;
	};
	union {
		float y, t;
	};
};

// Class with defaulted constructors and operators
class Class3
{
public:
	Class3() = default;
	Class3(const Class3& o) = default;
	Class3& operator=(const Class3& o) = default;
	union {
		float x, s;
	};
	union {
		float y, t;
	};
};

// Class with defaulted constructors and operators plus an additional constructor
class Class4
{
public:
	Class4() = default;
	Class4(const Class4& o) = default;
	// The presence of the constructor with parameters, causes MSVC to return this class in memory
	Class4(float x, float y) { this->x = x; this->y = y; }
	Class4& operator=(const Class4& o) = default;
	union {
		float x, s;
	};
	union {
		float y, t;
	};
};

// On MSVC 2019 the class is returned in registers
Class1 retClass1(float a)
{
	Class1 c;
	c.x = a;
	c.y = a+1;
	return c;
}

// On MSVC 2019 the class is returned in memory
Class2 retClass2(float a)
{
	Class2 c;
	c.x = a;
	c.y = a + 1;
	return c;
}

// On MSVC 2019 this is identical to retClass1
Class3 retClass3(float a)
{
	Class3 c;
	c.x = a;
	c.y = a + 1;
	return c;
}

// On MSVC 2019 this is identical to retClass2 (i.e. returned in memory)
Class4 retClass4(float a)
{
	Class4 c;
	c.x = a;
	c.y = a + 1;
	return c;
}

// On MSVC 2019 this is identical to retClass2 (i.e. returned in memory)
glm::vec2 retGlmVec2(float a)
{
	glm::vec2 c;
	c.x = a;
	c.y = a + 1;
	return c;
}

#if TEST_GLM == 1
glm::quat retGlmQuat(float a)
{
	glm::quat c;
	c.x = a;
	c.y = a + 1;
	c.z = a + 2;
	c.w = a + 3;
	return c;
}
#endif

// On MSVC 2019 this is passed on stack
float checkClass1(Class1 v)
{
	return v.x + v.y;
}

// On MSVC 2019 this is passed on stack
float checkClass2(Class2 v)
{
	return v.x + v.y;
}

// On MSVC 2019 this is passed on stack
float checkClass3(Class3 v)
{
	return v.x + v.y;
}

// On MSVC 2019 this is passed on stack
float checkClass4(Class4 v)
{
	return v.x + v.y;
}

// On MSVC 2019 this is passed on stack
float checkGlmVec2(glm::vec2 v)
{
	return v.x + v.y;
}

#if TEST_GLM == 1
float checkGlmQuat(glm::quat v)
{
	return v.x + v.y + v.z + v.w;
}
#endif

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	CBufferedOutStream bout;
	int r;
	asIScriptEngine* engine;


	asDWORD typeTraits1 = asGetTypeTraits<Class1>();
	asDWORD typeTraits2 = asGetTypeTraits<Class2>();
	asDWORD typeTraits3 = asGetTypeTraits<Class3>();
	asDWORD typeTraits4 = asGetTypeTraits<Class4>();
	asDWORD typeTraitsGlmVec2 = asGetTypeTraits<glm::vec2>();

	if (typeTraits1 != asOBJ_APP_CLASS)
		TEST_FAILED;
	if (typeTraits2 != asOBJ_APP_CLASS_CAK)
		TEST_FAILED;
	if (typeTraits3 != asOBJ_APP_CLASS)
		TEST_FAILED;
	if (typeTraits4 != asOBJ_APP_CLASS)
		TEST_FAILED;
	if (typeTraitsGlmVec2 != asOBJ_APP_CLASS)
		TEST_FAILED;

	// Test registering the Class1 type
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterObjectType("vec2", sizeof(Class1), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Class1>() | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float x", asOFFSET(Class1 , x)); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float y", asOFFSET(Class1, y)); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("vec2 test(float)", asFUNCTION(retClass1), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterGlobalFunction("float test2(vec2)", asFUNCTION(checkClass1), asCALL_CDECL); assert(r >= 0);

		r = ExecuteString(engine, "vec2 v; v = test(1); assert( v.x == 1 && v.y == 2 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "vec2 v; v.x = 1; v.y = 2; assert( test2(v) == 3 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test registering the Class2 type
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterObjectType("vec2", sizeof(Class2), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Class2>() | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float x", asOFFSET(Class2, x)); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float y", asOFFSET(Class2, y)); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("vec2 test(float)", asFUNCTION(retClass2), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterGlobalFunction("float test2(vec2)", asFUNCTION(checkClass2), asCALL_CDECL); assert(r >= 0);

		r = ExecuteString(engine, "vec2 v; v = test(1); assert( v.x == 1 && v.y == 2 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "vec2 v; v.x = 1; v.y = 2; assert( test2(v) == 3 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test registering the Class3 type
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterObjectType("vec2", sizeof(Class3), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Class3>() | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float x", asOFFSET(Class3, x)); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float y", asOFFSET(Class3, y)); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("vec2 test(float)", asFUNCTION(retClass3), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterGlobalFunction("float test2(vec2)", asFUNCTION(checkClass3), asCALL_CDECL); assert(r >= 0);

		r = ExecuteString(engine, "vec2 v; v = test(1); assert( v.x == 1 && v.y == 2 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "vec2 v; v.x = 1; v.y = 2; assert( test2(v) == 3 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test registering the Class4 type
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterObjectType("vec2", sizeof(Class4), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Class4>() | asOBJ_APP_CLASS_MORE_CONSTRUCTORS | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float x", asOFFSET(Class4, x)); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float y", asOFFSET(Class4, y)); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("vec2 test(float)", asFUNCTION(retClass4), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterGlobalFunction("float test2(vec2)", asFUNCTION(checkClass4), asCALL_CDECL); assert(r >= 0);

		r = ExecuteString(engine, "vec2 v; v = test(1); assert( v.x == 1 && v.y == 2 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "vec2 v; v.x = 1; v.y = 2; assert( test2(v) == 3 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test registering the glm::vec2 type
	// https://www.gamedev.net/forums/topic/705135-x64-calling-convention-doesnt-make-room-for-return-value-in-rcx/5418741/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterObjectType("vec2", sizeof(glm::vec2), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<glm::vec2>() | asOBJ_APP_CLASS_MORE_CONSTRUCTORS | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float x", asOFFSET(glm::vec2, x)); assert(r >= 0);
		r = engine->RegisterObjectProperty("vec2", "float y", asOFFSET(glm::vec2, y)); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("vec2 test(float)", asFUNCTION(retGlmVec2), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterGlobalFunction("float test2(vec2)", asFUNCTION(checkGlmVec2), asCALL_CDECL); assert(r >= 0);

		r = ExecuteString(engine, "vec2 v; v = test(1); assert( v.x == 1 && v.y == 2 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "vec2 v; v.x = 1; v.y = 2; assert( test2(v) == 3 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

#if TEST_GLM == 1
	// Test registering the glm::quat type
	// Reported by Patrick Jeeves
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterObjectType("quat", sizeof(glm::quat), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<glm::quat>() | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);
		r = engine->RegisterObjectProperty("quat", "float x", asOFFSET(glm::quat, x)); assert(r >= 0);
		r = engine->RegisterObjectProperty("quat", "float y", asOFFSET(glm::quat, y)); assert(r >= 0);
		r = engine->RegisterObjectProperty("quat", "float z", asOFFSET(glm::quat, z)); assert(r >= 0);
		r = engine->RegisterObjectProperty("quat", "float w", asOFFSET(glm::quat, w)); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("quat test(float)", asFUNCTION(retGlmQuat), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterGlobalFunction("float test2(quat)", asFUNCTION(checkGlmQuat), asCALL_CDECL); assert(r >= 0);

		// On MSVC 2019 this fails when GLM_CONFIG_DEFAULTED_FUNCTIONS == GLM_ENABLE
//#if (_MSC_VER && GLM_CONFIG_DEFAULTED_FUNCTIONS == GLM_ENABLE)
//		PRINTF("Skipping test on returning glm::quat by value on MSVC\n");
//#else
		r = ExecuteString(engine, "quat v; v = test(1); assert( v.x == 1 && v.y == 2 && v.z == 3 && v.w == 4 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
//#endif

		r = ExecuteString(engine, "quat v; v.x = 1; v.y = 2; v.z = 3; v.w = 4; assert( test2(v) == 10 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}
#else
	PRINTF("Skipped testing glm::quat\n");
#endif

	return fail;
}

}
