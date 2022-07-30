#include "utils.h"


namespace TestComposition
{

	class Thing
	{
	public:
		Thing() : someProperty(10) {}
		void DoSomething() 
		{ 
			someProperty += 100; 
		}
		int someProperty;
	};

	class OwnerDirect
	{
	public:
		OwnerDirect() : thing(), filler(0) {}
		asINT64 filler;
		Thing thing;
	};

	class OwnerIndirect
	{
	public:
		OwnerIndirect() : thing(new Thing()), filler(0) {}
		~OwnerIndirect() { delete thing; }
		asINT64 filler;
		Thing *thing;
	};

	bool Test()
	{
		bool fail = false;
		COutStream out;
		int r;

		// Test composition without indirection
		SKIP_ON_MAX_PORT
		{
			asIScriptEngine *engine = asCreateScriptEngine();
			engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

			r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);

			r = engine->RegisterObjectType("owner", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(r >= 0);
			r = engine->RegisterObjectMethod("owner", "void DoSomething()", asMETHOD(Thing, DoSomething), asCALL_THISCALL, 0, asOFFSET(OwnerDirect, thing), false); assert(r >= 0);

			r = engine->RegisterObjectProperty("owner", "int someProperty", asOFFSET(Thing, someProperty), asOFFSET(OwnerDirect, thing), false); assert(r >= 0);

			OwnerDirect owner;
			r = engine->RegisterGlobalProperty("owner o", &owner); assert(r >= 0);

			asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
			mod->AddScriptSection("test", "void main() { o.DoSomething(); assert( o.someProperty == 110 ); }");
			r = mod->Build();
			if (r < 0)
				TEST_FAILED;
			r = ExecuteString(engine, "main()", mod);
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;

			if (owner.filler != 0)
				TEST_FAILED;
			if (owner.thing.someProperty != 110)
				TEST_FAILED;

			CBytecodeStream strm("test");
			r = mod->SaveByteCode(&strm);
			if (r < 0)
				TEST_FAILED;

			mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
			r = mod->LoadByteCode(&strm);
			if (r < 0)
				TEST_FAILED;

			owner.thing.someProperty = 10;
			r = ExecuteString(engine, "main()", mod);
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;

			if (owner.filler != 0)
				TEST_FAILED;
			if (owner.thing.someProperty != 110)
				TEST_FAILED;

			engine->ShutDownAndRelease();
		}

		// Test composition with indirection
		SKIP_ON_MAX_PORT
		{
			asIScriptEngine *engine = asCreateScriptEngine();
			engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

			r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);

			r = engine->RegisterObjectType("owner", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(r >= 0);
			r = engine->RegisterObjectMethod("owner", "void DoSomething()", asMETHOD(Thing, DoSomething), asCALL_THISCALL, 0, asOFFSET(OwnerDirect, thing), true); assert(r >= 0);

			r = engine->RegisterObjectProperty("owner", "int someProperty", asOFFSET(Thing, someProperty), asOFFSET(OwnerDirect, thing), true); assert(r >= 0);

			OwnerIndirect owner;
			r = engine->RegisterGlobalProperty("owner o", &owner); assert(r >= 0);

			asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
			mod->AddScriptSection("test", "void main() { o.DoSomething(); assert( o.someProperty == 110 ); }");
			r = mod->Build();
			if (r < 0)
				TEST_FAILED;
			r = ExecuteString(engine, "main()", mod);
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;

			if (owner.filler != 0)
				TEST_FAILED;
			if (owner.thing->someProperty != 110)
				TEST_FAILED;

			CBytecodeStream strm("test");
			r = mod->SaveByteCode(&strm);
			if (r < 0)
				TEST_FAILED;

			mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
			r = mod->LoadByteCode(&strm);
			if (r < 0)
				TEST_FAILED;

			owner.thing->someProperty = 10;
			r = ExecuteString(engine, "main()", mod);
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;

			if (owner.filler != 0)
				TEST_FAILED;
			if (owner.thing->someProperty != 110)
				TEST_FAILED;

			engine->ShutDownAndRelease();
		}

		return fail;
	}
}