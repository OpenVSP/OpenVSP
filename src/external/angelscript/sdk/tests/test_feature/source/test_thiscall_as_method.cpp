//
// Tests of asCALL_THISCALL_OBJLAST and asCALL_THISCALL_OBJFIRST.
//
// Test author: Jordi Oliveras Rovira
//

#include "utils.h"

#include "../../../add_on/scriptstdstring/scriptstdstring.h"

#include <map>
#include <vector>
#include <string>

namespace TestThisCallMethod
{
	static const char * const TESTNAME = "TestThisCallMethod";

	class TestType
	{
		public:
			std::map<std::string, std::string> mapStrings;
			std::map<std::string, int> mapInt;

			void AddRef() { }

			void Release() { }

			void SetString(const std::string &key, const std::string &value)
			{
				mapStrings[key] = value;
			}

			std::string GetString(const std::string &key)
			{
				std::map<std::string, std::string>::iterator it = mapStrings.find(key);
				if (it != mapStrings.end())
				{
					return it->second;
				}
				return "";
			}

			void SetInt(const std::string &key, int value)
			{
				mapInt[key] = value;
			}

			int GetInt(const std::string &key)
			{
				std::map<std::string, int>::iterator it = mapInt.find(key);
				if (it != mapInt.end())
				{
					return it->second;
				}
				return 0;
			}
	} foo;

	struct AVirtualProperty
	{
        AVirtualProperty(std::string propertyName) : mPropertyName(propertyName) { }

        virtual ~AVirtualProperty() { }

        std::string mPropertyName;
	};

	struct VirtualPropertyString : public AVirtualProperty
	{
        VirtualPropertyString(std::string propertyName) : AVirtualProperty(propertyName)
        {
        }

        virtual ~VirtualPropertyString() { }

        void Set(TestType *obj, const std::string &value)
        {
            obj->SetString(mPropertyName, value);
        }

        std::string Get(TestType *obj)
        {
            return obj->GetString(mPropertyName);
        }
	};

	struct VirtualPropertyInt : public AVirtualProperty
	{
        VirtualPropertyInt(std::string propertyName) : AVirtualProperty(propertyName)
        {
        }

        virtual ~VirtualPropertyInt() { }

        void Set(int value, TestType *obj)
        {
            obj->SetInt(mPropertyName, value);
        }

        int Get(TestType *obj)
        {
            return obj->GetInt(mPropertyName);
        }
	};

    // For tests withs many parameters
	struct VirtualMethodWithManyParameters : public AVirtualProperty
	{
	    // Reusing the same base classe to save in separate variable
        VirtualMethodWithManyParameters() : AVirtualProperty("_virtual_method_key_") { }

        virtual ~VirtualMethodWithManyParameters() { }

        // return v4 string
        std::string ObjLast(int v1, const std::string &v2, TestType &o1, const std::string &v3, const std::string &v4, int v5, TestType *obj)
        {
            obj->SetInt(mPropertyName, v1);
            obj->SetString(mPropertyName, v2);
            obj->SetString(mPropertyName, v3);
            obj->SetString(mPropertyName, v4);
            obj->SetInt(mPropertyName, v5);
            o1.SetInt(mPropertyName, v1);
            o1.SetString(mPropertyName, v2);
            o1.SetString(mPropertyName, v3);
            o1.SetString(mPropertyName, v4);
            o1.SetInt(mPropertyName, v5);
            return obj->GetString(mPropertyName) == o1.GetString(mPropertyName) &&
                obj->GetInt(mPropertyName) == o1.GetInt(mPropertyName) ? v4 : v2;
        }

        // return v3 string
        std::string ObjFirst(TestType *obj, int v1, const std::string &v2, TestType &o1, const std::string &v3, const std::string &v4, int v5)
        {
            obj->SetInt(mPropertyName, v1);
            obj->SetString(mPropertyName, v2);
            obj->SetString(mPropertyName, v3);
            obj->SetString(mPropertyName, v4);
            obj->SetInt(mPropertyName, v5);
            o1.SetInt(mPropertyName, v1);
            o1.SetString(mPropertyName, v2);
            o1.SetString(mPropertyName, v3);
            o1.SetString(mPropertyName, v4);
            o1.SetInt(mPropertyName, v5);
            return obj->GetString(mPropertyName) == o1.GetString(mPropertyName) &&
                obj->GetInt(mPropertyName) == o1.GetInt(mPropertyName) ? v3 : v2;
        }
	};

	// Tests withs 64 bits params.
	// Android and Linux arm aligment for 64 bits.
	struct VirtualMethodWith64bitsParameters : public AVirtualProperty
	{
		// Reusing the same base classe to save in separate variable
		VirtualMethodWith64bitsParameters() : AVirtualProperty("_64bits_parameters_") {}

		virtual ~VirtualMethodWith64bitsParameters() {}

		// return v2 string
		std::string ObjLast64bits(asINT64 v1, const std::string &v2, TestType &o1, double /*f*/, int v3, TestType *obj)
		{
			obj->SetInt(mPropertyName, (int)v1);
			obj->SetString(mPropertyName, v2);
			obj->SetInt(mPropertyName, v3);
			o1.SetInt(mPropertyName, (int)v1);
			o1.SetString(mPropertyName, v2);
			o1.SetInt(mPropertyName, v3);
			return obj->GetString(mPropertyName) == o1.GetString(mPropertyName) &&
				obj->GetInt(mPropertyName) == o1.GetInt(mPropertyName) ? v2 : "---------";
		}

		// return v2 string
		std::string ObjFirst64bits(TestType *obj, asINT64 v1, const std::string &v2, TestType &o1, double /*f*/, int v3)
		{
			obj->SetInt(mPropertyName, (int)v1);
			obj->SetString(mPropertyName, v2);
			obj->SetInt(mPropertyName, v3);
			o1.SetInt(mPropertyName, (int)v1);
			o1.SetString(mPropertyName, v2);
			o1.SetInt(mPropertyName, v3);
			return obj->GetString(mPropertyName) == o1.GetString(mPropertyName) &&
				obj->GetInt(mPropertyName) == o1.GetInt(mPropertyName) ? v2 : "---------";
		}
	};

	bool Test()
	{
		RET_ON_MAX_PORT

		if (strstr(asGetLibraryOptions(), "THISCALL_METHOD_NO_IMPLEMENTED"))
		{
			printf("%s skipped due to this call method not implemented/not supported\n", TESTNAME);
			return false;
		}

		bool fail = false;
		COutStream out;
		int r;

		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterStdString(engine); // String addon
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("Foo", 0, asOBJ_REF);
        engine->RegisterObjectBehaviour("Foo", asBEHAVE_ADDREF, "void f()",  asMETHOD(TestType, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("Foo", asBEHAVE_RELEASE, "void f()", asMETHOD(TestType, Release), asCALL_THISCALL);
		engine->RegisterGlobalProperty("Foo foo", &foo);

		const char *StringPropertyNames[] = {"STR1", "STR2", "STR3"};
		const char *IntPropertyNames[] = {"INT1", "INT2", "INT3"};
		const int NumProperties = 3;

		std::vector<AVirtualProperty*> registeredVirtualProperties;

		for (int i = 0; i < NumProperties; ++i)
		{
			// String
			{
				std::string name(StringPropertyNames[i]);

				VirtualPropertyString *funcStr = new VirtualPropertyString(name);
				registeredVirtualProperties.push_back(funcStr);

				engine->RegisterObjectMethod("Foo", ("void set_" + name + "(const string &in) property").c_str(),
                    asMETHOD(VirtualPropertyString, Set), asCALL_THISCALL_OBJFIRST, funcStr);

				engine->RegisterObjectMethod("Foo", ("string get_" + name + "() property").c_str(),
                    asMETHOD(VirtualPropertyString, Get), asCALL_THISCALL_OBJFIRST, funcStr);
			}

			// Int
			{
				std::string name(IntPropertyNames[i]);

				VirtualPropertyInt *funcInt = new VirtualPropertyInt(name);
				registeredVirtualProperties.push_back(funcInt);

				engine->RegisterObjectMethod("Foo", ("void set_" + name + "(int) property").c_str(),
                    asMETHOD(VirtualPropertyInt, Set), asCALL_THISCALL_OBJLAST, funcInt);

				engine->RegisterObjectMethod("Foo", ("int get_" + name + "() property").c_str(),
                    asMETHOD(VirtualPropertyInt, Get), asCALL_THISCALL_OBJLAST, funcInt);
			}
		}

		// Tests with simple methods
		{
			// Tests with strings
			r = ExecuteString(engine, "string a = 'abc'; foo.STR1 = a; assert(foo.STR1 == 'abc' && foo.STR1 == a);");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			r = ExecuteString(engine, "string b = 'xyz'; foo.STR2 = b; assert(foo.STR2 == 'xyz' && foo.STR2 != foo.STR1);");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			r = ExecuteString(engine, "string c = 'qwe'; foo.STR3 = c; assert(foo.STR3 == 'qwe');");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			// Set from app, check from script
			foo.SetString("STR1", "12345");
			foo.SetString("STR2", "mnbvc");
			foo.SetString("STR3", "tgbhu");
			r = ExecuteString(engine, "assert(foo.STR1 == '12345' && foo.STR2 == 'mnbvc' && foo.STR3 == 'tgbhu');");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			// Set from script, check from app
			r = ExecuteString(engine, "foo.STR1 = ''; foo.STR2 = '314'; foo.STR3 = 'p';");
			if (r != asEXECUTION_FINISHED || foo.GetString("STR1") != "" || foo.GetString("STR2") != "314" || foo.GetString("STR3") != "p")
				TEST_FAILED;

			// Tests with int
			r = ExecuteString(engine, "int a = 1; foo.INT1 = a; assert(foo.INT1 == 1 && foo.INT1 == a);");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			r = ExecuteString(engine, "int b = -2; foo.INT2 = b; assert(foo.INT2 == -2 && foo.INT2 != foo.INT1);");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			r = ExecuteString(engine, "int c = 12345; foo.INT3 = 12345; assert(foo.INT3 == 12345);");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			// Set from app, check from script
			foo.SetInt("INT1", 987);
			foo.SetInt("INT2", 1234);
			foo.SetInt("INT3", 657);
			r = ExecuteString(engine, "assert(foo.INT1 == 987 && foo.INT2 == 1234 && foo.INT3 == 657);");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			// Set from script, check from app
			r = ExecuteString(engine, "foo.INT1 = 0; foo.INT2 = 314; foo.INT3 = 281;");
			if (r != asEXECUTION_FINISHED || foo.GetInt("INT1") != 0 || foo.GetInt("INT2") != 314 || foo.GetInt("INT3") != 281)
				TEST_FAILED;
		}

		// Tests with multiple parameters
		{
			VirtualMethodWithManyParameters *funcMeth = new VirtualMethodWithManyParameters();
			registeredVirtualProperties.push_back(funcMeth);

			engine->RegisterObjectMethod("Foo", "string ObjLast(int, const string &in, Foo @, const string &in, const string &in, int)",
				asMETHOD(VirtualMethodWithManyParameters, ObjLast), asCALL_THISCALL_OBJLAST, funcMeth);

			engine->RegisterObjectMethod("Foo", "string ObjFirst(int, const string &in, Foo @, const string &in, const string &in, int)",
				asMETHOD(VirtualMethodWithManyParameters, ObjFirst), asCALL_THISCALL_OBJFIRST, funcMeth);

			// Test with methods
			r = ExecuteString(engine, "assert(foo.ObjLast(1, 'abc', @foo, 'xyz', '123', 7) == '123');");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			r = ExecuteString(engine, "assert(foo.ObjFirst(1, 'abc', @foo, 'xyz', '123', 7) == 'xyz');");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
		}

		// Tests with 64 bits parameters (android and linux arm)
		{
			VirtualMethodWith64bitsParameters *func64bits = new VirtualMethodWith64bitsParameters();
			registeredVirtualProperties.push_back(func64bits);

			engine->RegisterObjectMethod("Foo", "string ObjLast64bits(int64, const string &in, Foo @, double, int)",
				asMETHOD(VirtualMethodWith64bitsParameters, ObjLast64bits), asCALL_THISCALL_OBJLAST, func64bits);

			engine->RegisterObjectMethod("Foo", "string ObjFirst64bits(int64, const string &in, Foo @, double, int)",
				asMETHOD(VirtualMethodWith64bitsParameters, ObjFirst64bits), asCALL_THISCALL_OBJFIRST, func64bits);

			// Test with methods
			r = ExecuteString(engine, "assert(foo.ObjLast64bits(1, 'abc', @foo, 3.14159, 7) == 'abc');");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
			r = ExecuteString(engine, "assert(foo.ObjFirst64bits(10, 'xyz', @foo, 2.71828, 78) == 'xyz');");
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;
		}

		engine->Release();

        std::vector<AVirtualProperty*>::iterator it = registeredVirtualProperties.begin();
		for (; it != registeredVirtualProperties.end(); ++it)
        {
            delete *it;
        }

		return fail;
	}
}
