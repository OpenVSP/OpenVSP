#include "utils.h"
#if AS_CAN_USE_CPP11
#include "../../../add_on/datetime/datetime.h"
#endif
#include <sstream>

namespace Test_Addon_DateTime
{

bool Test()
{
#if !defined(AS_CAN_USE_CPP11)
	PRINTF("Skipped due to lack of C++11 support\n");
	return false;
#else

	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;
	asIScriptEngine *engine;

	// Test odd behaviour with 'datetime dt(2019,01,01);'
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		RegisterScriptDateTime(engine);
		
		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"datetime dt(2019,01,01); \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		CDateTime* dt = reinterpret_cast<CDateTime*>(mod->GetAddressOfGlobalVar(0));
		if (dt->getYear() != 2019 || dt->getMonth() != 1 || dt->getDay() != 1)
		{
			PRINTF("%d/%d/%d\n", dt->getYear(), dt->getMonth(), dt->getDay());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "assert(dt.year == 2019 && dt.month == 1 && dt.day == 1);", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->Release();
	}

	// Test returning the datetime object from script function
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		RegisterScriptDateTime(engine);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"datetime getDate() { \n"
			"  datetime dt; \n"
			"  dt.setDate(2018, 10, 14); \n"
			"  return dt; \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "datetime dt = getDate(); assert(dt.year == 2018 && dt.month == 10 && dt.day == 14);", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->Release();
	}

	// Test the datetime object
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		RegisterScriptDateTime(engine);

		CDateTime dt;
		std::stringstream s;
		s << "datetime dt; assert( dt.year == " << dt.getYear() << " && dt.month == " << dt.getMonth() << " );";

		r = ExecuteString(engine, s.str().c_str());
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// setDate with valid values
		r = ExecuteString(engine, "datetime dt; \n"
			                      "assert( dt.setDate(2003,3,28) ); \n"
			                      "assert( dt.year == 2003 && dt.month == 3 && dt.day == 28 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// setDate with invalid values
		r = ExecuteString(engine, "datetime dt; assert( dt.setDate(2001,2,29) == false );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// setTime with value values
		r = ExecuteString(engine, "datetime dt; assert( dt.setTime(0,0,0) ); assert( dt.hour == 0 && dt.minute == 0 && dt.second == 0 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// setTime with invalid values
		r = ExecuteString(engine, "datetime dt; assert( dt.setTime(25,61,61) == false );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// diff
		r = ExecuteString(engine, "datetime dt1, dt2; dt1.setDate(2000,1,1); dt1.setTime(0,0,0); dt2.setDate(2000,1,2); dt2.setTime(0,0,0); assert( dt2 - dt1 == 60*60*24 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// add seconds
		r = ExecuteString(engine, "datetime dt1; dt1.setDate(2000,1,1); dt1.setTime(0,0,0); dt1 += 60*60; \n"
			                      "assert( dt1.hour == 1 ); \n"
			                      "dt1 += 24*60*60; \n"
			                      "assert( dt1.day == 2 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// add seconds with reverse operator
		r = ExecuteString(engine, "datetime dt1; datetime dt2(dt1); dt1 = 3600 + dt2; assert( dt1 - dt2 == 3600 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// comparison
		r = ExecuteString(engine, "datetime dt1; datetime dt2(dt1); assert( dt1 == dt2 ); \n"
			                      "dt1 -= 1; assert( dt1 < dt2 ); \n");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;



		engine->Release();
	}

	// Success
	return fail;
#endif
}


} // namespace

