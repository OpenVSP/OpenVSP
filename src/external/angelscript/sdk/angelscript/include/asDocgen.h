//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// asDocgen.h
//
// AngelScript documentation generator
// Released into the public domain by the author on this website:
// http://www.gamedev.net/topic/629517-document-generation-for-registered-interface/
//
// Modified by Rob McDonald & Justin Gravett (ESAero)
//

#ifndef asdocgen_h
#define asdocgen_h

#include "main.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

namespace asDocgen
{
    using std::string;
    using std::cout;
    using std::endl;
    using std::ios_base;

    class ScriptTypeComment
    {
    public:

        string comment;

        std::map<string, string> members;
    };

    std::map<string, ScriptTypeComment> typeComments;
    std::map<string, string> globalComments;
    std::map<string, std::map<string, string>> enumeratorComments;
    std::map<string, string> enumerationComment;
    std::map<string, string> globalPropertyComments;

    std::map<string, string> skippedComments;
    std::map<string, string> groupTitles;
    std::map<string, string> groupDescriptions;

    std::map<string, ScriptTypeComment> typeGroups;
    std::map<string, string> globalGroups;
    std::map<string, std::map<string, string>> enumeratorGroups;
    std::map<string, string> enumerationGroups;
    std::map<string, string> globalPropertyGroups;

    void AddTypeComment( const string& name, const string& comment )
    {
        if ( typeComments.count( name ) )
        {
            typeComments[name].comment = comment;
        }
        else
        {
            typeComments[name] = ScriptTypeComment();
            typeComments[name].comment = comment;
        }
    }

    void AddTypeMemberComment( const string& name, const string& member, const string& comment )
    {
        if ( typeComments.count( name ) )
        {
            typeComments[name].members[member] = comment;
        }
        else
        {
            typeComments[name].comment = "// TODO, add comment";
            typeComments[name] = ScriptTypeComment();
            typeComments[name].members[member] = comment;
        }
    }

    void AddGlobalFunctionComment( const string& decl, const string& comment )
    {
        globalComments[decl] = comment;
    }

    void AddGlobalPropertyComment( const string& decl, const string& comment )
    {
        globalPropertyComments[decl] = comment;
    }

    void AddEnumerationComment( const string& decl, const string& comment )
    {
        enumerationComment[decl] = comment;
    }

    void AddEnumeratorComment( const string& enumname, const string& enummember, const string& comment )
    {
        if ( enumeratorComments.count( enumname ) )
        {
            enumeratorComments[enumname][enummember] = comment;
        }
        else
        {
            enumeratorComments[enumname] = std::map<string, string>();
            enumeratorComments[enumname][enummember] = comment;
        }
    }

    void AddSkippedComment( const string& decl, const string& comment )
    {
        skippedComments[decl] = comment;
    }

    void AddGroup( const string& group, const string& title, const string& description )
    {
        groupTitles[group] = title;
        groupDescriptions[group] = description;
    }

    void AddTypeGroup( const string& name, const string& group )
    {
        if ( typeGroups.count( name ) )
        {
            typeGroups[name].comment = group;
        }
        else
        {
            typeGroups[name] = ScriptTypeComment();
            typeGroups[name].comment = group;
        }
    }

    void AddTypeMemberGroup( const string& name, const string& member, const string& group )
    {
        if ( typeGroups.count( name ) )
        {
            typeGroups[name].members[member] = group;
        }
        else
        {
            typeGroups[name].comment = "// TODO, add comment";
            typeGroups[name] = ScriptTypeComment();
            typeGroups[name].members[member] = group;
        }
    }

    void AddGlobalFunctionGroup( const string& decl, const string& group )
    {
        globalGroups[decl] = group;
    }

    void AddGlobalPropertyGroup( const string& decl, const string& group )
    {
        globalPropertyGroups[decl] = group;
    }

    void AddEnumerationGroup( const string& decl, const string& group )
    {
        enumerationGroups[decl] = group;
    }

    void AddEnumeratorGroup( const string& enumname, const string& enummember, const string& group )
    {
        if ( enumeratorGroups.count( enumname ) )
        {
            enumeratorGroups[enumname][enummember] = group;
        }
        else
        {
            enumeratorGroups[enumname] = std::map<string, string>();
            enumeratorGroups[enumname][enummember] = group;
        }
    }

    void ClearComments()
    {
        typeComments.clear();
        globalComments.clear();
        enumeratorComments.clear();
        enumerationComment.clear();
        globalPropertyComments.clear();

        skippedComments.clear();
    }

    void ClearGroups()
    {
        typeGroups.clear();
        globalGroups.clear();
        enumeratorGroups.clear();
        enumerationGroups.clear();
        globalPropertyGroups.clear();
    }

    void replaceSubStr( std::string& str, const std::string& from, const std::string& to )
    {
        size_t start_pos = 0;
        while ( ( start_pos = str.find( from, start_pos ) ) != std::string::npos )
        {
            str.replace( start_pos, from.length(), to );
            start_pos += to.length();
        }
    }

    void ProcessFunctionDecleration( std::string& str )
    {
        replaceSubStr( str, "opIndex", "operator[]" );
        replaceSubStr( str, "opAddAssign", "operator+=" );
        replaceSubStr( str, "opSubAssign", "operator-=" );
        replaceSubStr( str, "opMulAssign", "operator*=" );
        replaceSubStr( str, "opDivAssign", "operator/=" );
        replaceSubStr( str, "opModAssign", "operator%=" );
        replaceSubStr( str, "opAndAssign", "operator&=" );
        replaceSubStr( str, "opOrAssign", "operator|=" );
        replaceSubStr( str, "opXorAssign", "operator^=" );
        replaceSubStr( str, "opShlAssign", "operator<<=" );
        replaceSubStr( str, "opShrAssign", "operator>>=" );
        replaceSubStr( str, "opUShrAssign", "operator>>>=" );
        replaceSubStr( str, "opAssign", "operator=" );
        replaceSubStr( str, "opAdd_r", "operator+" );
        replaceSubStr( str, "opAdd", "operator+" );
        replaceSubStr( str, "opSub_r", "operator-" );
        replaceSubStr( str, "opSub", "operator-" );
        replaceSubStr( str, "opMul_r", "operator*" );
        replaceSubStr( str, "opMul", "operator*" );
        replaceSubStr( str, "opDiv_r", "operator/" );
        replaceSubStr( str, "opDiv", "operator/" );
        replaceSubStr( str, "opMod_r", "operator%" );
        replaceSubStr( str, "opMod", "operator%" );
        replaceSubStr( str, "opAnd_r", "operator&" );
        replaceSubStr( str, "opAnd", "operator&" );
        replaceSubStr( str, "opOr_r", "operator|" );
        replaceSubStr( str, "opOr", "operator|" );
        replaceSubStr( str, "opXor_r", "operator^" );
        replaceSubStr( str, "opXor", "operator^" );
        replaceSubStr( str, "opShl_r", "operator<<" );
        replaceSubStr( str, "opShl", "operator<<" );
        replaceSubStr( str, "opShr_r", "operator>>" );
        replaceSubStr( str, "opShr", "operator>>" );
        replaceSubStr( str, "opUShr_r", "operator>>>" );
        replaceSubStr( str, "opUShr", "operator>>>" );
        replaceSubStr( str, "opCmp", "operator>" ); // TODO, maybe add all 4 operators
        replaceSubStr( str, "opEquals", "operator==" );
    }

    void GenDoc( asIScriptEngine* engine, const string& filename )
    {
        class SObject
        {
        public:
            SObject()
            {
                istemplate = false;
                nspace = "";
            }
            string name;
            bool istemplate;
            std::vector<string> methods;
            std::vector<string> properties;
            std::vector<string> behaviours;
            string nspace;
        };

        std::vector<string> globalFunctions;

        std::vector<SObject> objects;

        std::vector<string> globalProperties;

        std::map<string, std::vector<std::pair<std::string, int>> > enums;

        unsigned int funcCount = engine->GetGlobalFunctionCount();
        for ( unsigned int i = 0; i < funcCount; ++i )
        {
            globalFunctions.push_back( engine->GetGlobalFunctionByIndex( i )->GetDeclaration( false, true, true ) );  // Object name, namespace, parm names
        }

        unsigned int globalPropertyCount = engine->GetGlobalPropertyCount();
        for ( unsigned int i = 0; i < globalPropertyCount; ++i )
        {
            const char* name;
            const char* nameSpace;
            int typeId;
            bool isConst;
            engine->GetGlobalPropertyByIndex( i, &name, &nameSpace, &typeId, &isConst );
            string decl = "";
            if ( isConst )
                decl += "const ";
            decl += engine->GetTypeDeclaration( typeId );
            decl += " ";
            if ( nameSpace != ( string ) "" )
            {
                cout << nameSpace << endl;
                decl += nameSpace;
                decl += "::";
            }
            decl += name;
            globalProperties.push_back( decl );
        }

        unsigned int typeCount = engine->GetObjectTypeCount();
        for ( unsigned int i = 0; i < typeCount; ++i )
        {
            SObject obj;
            asITypeInfo* objtype = engine->GetObjectTypeByIndex( i );
            obj.name = objtype->GetName();

            obj.nspace = objtype->GetNamespace();

            if ( objtype->GetTypeId() & asTYPEID_TEMPLATE )
                obj.istemplate = true;

            unsigned int propertyCount = objtype->GetPropertyCount();
            for ( unsigned int k = 0; k < propertyCount; ++k )
            {
                obj.properties.push_back( objtype->GetPropertyDeclaration( k ) );
            }

            unsigned int methodCount = objtype->GetMethodCount();
            for ( unsigned int k = 0; k < methodCount; ++k )
            {
                string method = objtype->GetMethodByIndex( k )->GetDeclaration( false, true, true );  // Object name, namespace, parm names
                ProcessFunctionDecleration( method );
                obj.methods.push_back( method );
            }

            unsigned int behCount = objtype->GetBehaviourCount();
            for ( unsigned int k = 0; k < behCount; ++k )
            {
                string method = objtype->GetBehaviourByIndex( k, 0 )->GetDeclaration( false, true, true );  // Object name, namespace, parm names
                obj.behaviours.push_back( method );
            }

            objects.push_back( obj );
        }

        unsigned int enumCount = engine->GetEnumCount();
        for ( unsigned int i = 0; i < enumCount; ++i )
        {
            int enumid = 0;
            asITypeInfo* e = engine->GetEnumByIndex( i );
            string enumdef = e->GetName();
            unsigned int count = e->GetEnumValueCount();

            enums[enumdef] = std::vector<std::pair<std::string, int>>();

            for ( unsigned int i = 0; i < count; ++i )
            {
                int value = 0;
                string enumname = e->GetEnumValueByIndex( i, &value );

                enums[enumdef].push_back( make_pair( enumname, value ) );
            }

        }


        // Generate Mainpage Markdown File
        std::ofstream mainpage_md;
        mainpage_md.open( "APIReadme.md", ios_base::out );

        // TODO: Improve Readme
        // The documentation has been generated from running **Doxygen** on a header 
        // file output from running OpenVSP through the command line with the **-doc** input argument.
        //## Documentation Navigation
        //All OpenVSP API classes, enumerations, and functions are defined in** [openvsp_as.h]( openvsp__as_8h.html )**, 
        //so this is the best place to start.All API members are indexed for documentation, so can also be found through 
        //the** [File Members]( globals.html )** drop - down or through the search bar.

    //### Developers
    //Below is a list of source files related to the OpenVSP API for developers to improve:
    //    -src\geom_api\APIReadme.me
    //        - Markup file for the API documentation introduction
    //    - src\geom_api\APIDoxyfile
    //        - Doxyfile used by Doxygen to generate the API documentation
    //    - src\geom_api\VSP_Geom_API.cppand src\geom_api\VSP_Geom_API.h
    //        - C++and Python API class, enumeration, and function definitions
    //    - src\geom_core\ScriptMgr.cppand src\geom_core\ScriptMgr.h
    //        - Exposes API to Angelscript
    //        - Includes API documentation comments that are written out to openvsp_as.h when the OpenVSP is run with the "-doc" input argument

        std::string readme_str = R"(

    # Documentation for the OpenVSP API

This is the documentation for the classes, enumerations, and functions available in through the OpenVSP API. 

The API functions are organized into the following groups:

)";

        std::map<std::string, std::string>::iterator git;

        for ( git = groupTitles.begin(); git != groupTitles.end(); git++ )
        {
            readme_str += "- [";
            readme_str += git->second;
            readme_str += "]";

            // Identify group HTML name
            readme_str += "(group__";

            for ( size_t i = 0; i < git->first.size(); i++ )
            {
                if ( isupper( git->first[i] ) )
                {
                    readme_str += "_";
                    readme_str += tolower( git->first[i] );
                }
                else
                {
                    readme_str += git->first[i];
                }
            }

            readme_str += ".html)";

            readme_str += "\n";
        }

        readme_str += R"(

The OpenVSP API classes with member functions are identified here:

)";

        for ( size_t i = 0; i < objects.size(); ++i )
        {
            readme_str += "- [";
            readme_str += objects[i].name;
            readme_str += "]";

            // Identify group HTML name
            readme_str += "(class";

            for ( size_t j = 0; j < objects[i].name.size(); j++ )
            {
                if ( isupper( objects[i].name[j] ) )
                {
                    readme_str += "_";
                    readme_str += tolower( objects[i].name[j] );
                }
                else
                {
                    readme_str += objects[i].name[j];
                }
            }

            readme_str += ".html)";

            readme_str += "\n";
        }

        readme_str += R"(

A master page containing all groups, classes, enums, and un-documented functions is available here: 

- [Master API Documentation Link](openvsp__as_8h.html)

## Help
### Examples
OpenVSP API examples are available in the **scripts** directory of the distribution. These example scripts are written in 
AngelScript, but map very closely for the Python API. An example for using the Python API can be found in **python/openvsp/openvsp/tests**. 
For specific requests, the [OpenVSP Google Group](https://groups.google.com/forum/#!forum/openvsp) is available.

### Python API Instructions
View the **README** file in the **python** directory of the distribution for instructions on Python API instructions. Note, the Python 
and OpenVSP versions (32-bit or 64-bit) must match.

### MATLAB API
The MATLAB API is not included with the OpenVSP distribution and must be built by the user. A presentation with instructions is 
available from the [2019 OpenVSP Workshop](http://openvsp.org/wiki/doku.php?id=workshop2019)

## Improvements
### Users
Users are encouraged to make use of the [GitHub Issue Tracker](https://github.com/OpenVSP/OpenVSP) if they have a suggestions, 
feature request, or bug report for the OpenVSP developers.

## Links

 - [Wiki](http://openvsp.org/wiki/doku.php)
 - [OpenVSP Main Page](http://openvsp.org/)
 - [Google Group](https://groups.google.com/forum/#!forum/openvsp)
 - [Source Code on GitHub](https://github.com/OpenVSP/OpenVSP)

    )";

        mainpage_md << readme_str;

        mainpage_md.close();

        //
        // WRITE TO FILE
        //

        std::ofstream outfile;
        outfile.open( filename.c_str(), ios_base::out );

        std::string comment_str;

        if ( groupTitles.size() > 0 )
        {
            comment_str = "/*!";

            for ( git = groupTitles.begin(); git != groupTitles.end(); git++ )
            {
                comment_str += R"(
    \defgroup )";

                comment_str += git->first;
                comment_str += " ";
                comment_str += git->second;

                comment_str += groupDescriptions[git->first];
            }
            comment_str += "\n";

            comment_str += "*/ \n";
            outfile << comment_str;
        }

        // Write file header
        comment_str = R"(
/*! 
    \file )";

        comment_str += filename;

        comment_str += R"(
    \brief This is the Master API Documentation Reference. This page serves to identify all grouped enums, classes, and global functions. Any that are not grouped will 
    be documented here. 
    Global functions are orgainized into the following groups:\n\n )";

        for ( git = groupTitles.begin(); git != groupTitles.end(); git++ )
        {
            comment_str += R"(
    \ref )";
            comment_str += git->first;
            comment_str += R"( \n )";
        }

        comment_str += R"(
    \n \ref index "Click here to return to the main page"
    \author    Justin Gravett
    \author    Hunter Harris
    \version   )";

        comment_str += VSPVERSION4;

        comment_str += R"(
    \date      2019
    \copyright NASA Open Source Agreement (NOSA) version 1.3 as detailed in the LICENSE file which accompanies this software
*/ )";

    // TODO Update details and date
    // Note: other special comment codes include \version, \pre, \bug, \warning

        outfile << comment_str << "\n\n";

        for ( unsigned int i = 0; i < objects.size(); ++i )
        {
            bool skip_flag = false;
            if ( skippedComments.count( objects[i].name ) )
            {
                outfile << skippedComments[objects[i].name] << endl;
                skip_flag = true;
            }

            if ( typeComments.count( objects[i].name ) )
                outfile << typeComments[objects[i].name].comment << endl;
            if ( objects[i].istemplate )
                outfile << "template<typename T>" << endl;
            if ( objects[i].nspace != string( "" ) )
                outfile << "namespace " << objects[i].nspace << endl << "{" << endl;
            outfile << "class " << objects[i].name << "\n";
            outfile << "{\n";
            outfile << "public:\n";

            if ( skip_flag )
            {
                outfile << "#ifndef DOXYGEN_SHOULD_SKIP_THIS\n"; // Note "DOXYGEN_SHOULD_SKIP_THIS" must be set in the Doxyfile for PREDEFINED
            }

            if ( !objects[i].properties.empty() )
                outfile << "  // Properties\n";
            for ( unsigned int k = 0; k < objects[i].properties.size(); ++k )
            {
                outfile << "  " << objects[i].properties[k] << ";\n";
            }
            outfile << "  // Methods\n";

            for ( unsigned int k = 0; k < objects[i].methods.size(); ++k )
            {
                if ( typeComments.count( objects[i].name ) )
                {
                    if ( typeComments[objects[i].name].members.count( objects[i].methods[k] ) )
                    {
                        outfile << "  " << typeComments[objects[i].name].members[objects[i].methods[k]] << endl;
                    }
                }

                outfile << "  " << objects[i].methods[k] << ";\n";
            }

            if ( skip_flag )
            {
                outfile << "#endif /* DOXYGEN_SHOULD_SKIP_THIS */\n";
            }

            /*
             outfile << "  // Behaviours\n";
             for (unsigned int k = 0; k < objects[i].behaviours.size(); ++k)
             {
             outfile << "  " << objects[i].behaviours[k] << ";\n";
             }
             */
            outfile << "};\n";

            if ( objects[i].nspace != string( "" ) )
                outfile << "};" << endl;
            outfile << endl;

        }

        for ( std::map<string, std::vector<std::pair<string, int>>>::iterator it = enums.begin(); it != enums.end(); ++it )
        {
            if ( enumerationGroups.count( it->first ) )
            {
                comment_str = R"(
/*! 
    \ingroup )";
                comment_str += enumerationGroups[it->first];
                comment_str += R"(
*/ )";
                comment_str += "\n";

                outfile << comment_str;
            }

            if ( enumerationComment.count( it->first ) )
                outfile << enumerationComment[it->first] << endl;

            outfile << "enum " << it->first << endl;
            outfile << "{" << endl;

            for ( unsigned int i = 0; i < it->second.size(); ++i )
            {

                outfile << "  " << it->second[i].first << " = " << it->second[i].second;

                if ( i != it->second.size() - 1 )
                    outfile << ",";

                if ( enumeratorComments.count( it->first ) )
                {
                    if ( enumeratorComments[it->first].count( it->second[i].first ) )
                    {
                        outfile << " " << enumeratorComments[it->first][it->second[i].first] << "\n";
                    }
                    else
                        outfile << "\n";
                }
                else
                    outfile << "\n";

            }

            outfile << "};" << endl;
        }

        outfile << endl;

        for ( unsigned int i = 0; i < globalFunctions.size(); ++i )
        {
            if ( globalGroups.count( globalFunctions[i] ) )
            {
                comment_str = R"(
/*! 
    \ingroup )";
                comment_str += globalGroups[globalFunctions[i]];
                comment_str += R"(
*/ )";
                comment_str += "\n";

                outfile << comment_str;
            }

            if ( globalComments.count( globalFunctions[i] ) )
            {
                outfile << globalComments[globalFunctions[i]] << endl;
            }
            outfile << globalFunctions[i] << ";" << endl;
        }

        outfile << "\n";

        for ( unsigned int i = 0; i < globalProperties.size(); ++i )
        {
            if ( globalPropertyComments.count( globalProperties[i] ) )
            {
                outfile << globalPropertyComments[globalProperties[i]] << endl;
            }
            outfile << globalProperties[i] << ";" << endl;
        }

        outfile << "\n";

        outfile.close();

        ClearComments();
        ClearGroups();

    }

};

/** Generates document for registered interface */
void GenerateDocument( asIScriptEngine* engine, const char* filename )
{
    asDocgen::GenDoc( engine, filename );
}


#endif
