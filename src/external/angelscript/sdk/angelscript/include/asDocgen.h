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
    std::map<string, std::map<string, string> > enumeratorComments;
    std::map<string, string> enumerationComment;
    std::map<string, string> globalPropertyComments;
    std::map<string, bool> globalTestFlags;

    std::map<string, string> skippedComments;
    std::map<string, string> groupTitles;
    std::map<string, string> groupDescriptions;

    std::map<string, ScriptTypeComment> typeGroups;
    std::map<string, string> globalGroups;
    std::map<string, std::map<string, string> > enumeratorGroups;
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

    void AddGlobalFunctionTestFlag( const string& decl, bool flag )
    {
        globalTestFlags[decl] = flag;
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

    bool sortByVal( const std::pair<string, string> &a,
        const std::pair<string, string> &b )
    {
        return ( a.second < b.second );
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

    //Checks that characters are there first before erasing them, to avoid errors
    bool CheckForTarget(std::string tempCaptureString,std::string targetString)
    {
         int commentCodeStartLocation = tempCaptureString.find( targetString, 0 );
         return commentCodeStartLocation != string::npos;
    }

    //We use this to get function names from strings
    std::string extractTargetString(std::string target1, std::string target2, std::string tempCaptureString)
    {
        int firstLocation = tempCaptureString.find( target1, 0 );
        int secondLocation = tempCaptureString.find( target2, 0 );
        tempCaptureString = tempCaptureString.substr( firstLocation, secondLocation-firstLocation );
        return tempCaptureString;
    }

    //Pulls names of functions and comments from mapped data and uses them to help format and write the API unit test file
    void CreateAPITestDoc(std::vector<string> globalFunctions)
    {
        int targetLocation = 0;
        std::string target = "";
        std::string target2 = "";
        std::string tempCaptureString = "";
        std::string extractCommentsTempString = "";
        std::ofstream codeFile;
        std::vector<string> capturedFunctionStrings;

        //Make our stream obj
        codeFile.open( "APITestCode.vspscript", ios_base::out );

        //First we set the main header
        codeFile << "void main()\n{\n";

        //Start going thru the mapped function commnets first to make sure each function has actual code
        //if not we skip that function (for now?)
        for ( unsigned int i = 0; i < globalFunctions.size(); ++i )
        {
            if ( globalComments.count( globalFunctions[i] ) && globalTestFlags[globalFunctions[i]] )
            {
                 tempCaptureString = globalComments[globalFunctions[i]];

                 target = R"(\code{.cpp})";

                 //If target is in string then we want to get the function name
                 if ( CheckForTarget(tempCaptureString, R"(\code{.cpp})" ))
                 {  
                     //We use this to get function names from strings using 2 target characters
                     tempCaptureString = extractTargetString( " ", "(", globalFunctions[i] );

                     //We check to make sure extracted function name is not a repeat, if so we dont add anthing to codeFile
                     if ( ! ( std::find(capturedFunctionStrings.begin(), capturedFunctionStrings.end(), tempCaptureString) != capturedFunctionStrings.end() ))
                     {
                         //We format the function name and add it to the stram object to write to file
                         codeFile <<"\t" << tempCaptureString << "_UT();" << "\n";

                         //Then we push the captured string in the vector so we can check it for those dam repeats
                         capturedFunctionStrings.push_back( tempCaptureString );
                     }
                 }
            }
        }

        //end of main function and start of the function defintions
        codeFile << "}\n\n//-----------------------------Functions---------------------------------------\n\n";

        //Clear out those function names, cause we have to do it again
        capturedFunctionStrings.clear();

         
        for ( unsigned int i = 0; i < globalFunctions.size(); ++i )
        {
            if ( globalComments.count( globalFunctions[i] ) && globalTestFlags[globalFunctions[i]] )
            {
                tempCaptureString = globalComments[globalFunctions[i]];

                target = R"(\code{.cpp})";

                //if target is in string then we want to get the function name and the code comments/docs
                if ( CheckForTarget(tempCaptureString, target ) && globalComments.count( globalFunctions[i] ) )
                {
                    //We use this to get function names from strings
                    tempCaptureString = extractTargetString( " ", "(", globalFunctions[i] );

                    //We check to make sure extracted function name is not a repeat, if so we dont add anthing to codeFile
                    if ( ! ( std::find(capturedFunctionStrings.begin(), capturedFunctionStrings.end(), tempCaptureString) != capturedFunctionStrings.end() ))
                    {
                        //We format the function name and add it to the stram object to write to file
                        codeFile << "void" << tempCaptureString << "_UT()" <<  "\n{";

                        std::string temp_function_name = tempCaptureString;

                        //Then we push the captured string in the vector so we can check it for those dam repeats
                        capturedFunctionStrings.push_back( tempCaptureString );

                        //This is where we have to strip out parts of the comments that are not actual code
                        if ( globalComments.count( globalFunctions[i] ) )
                        {
                            tempCaptureString = globalComments[globalFunctions[i]];

                            target = R"(/*!)";
                            target2 = R"(\code{.cpp})";

                            //Here we erase the target up to a specific length (all comments after target)
                            if ( CheckForTarget( tempCaptureString, target ) &&  CheckForTarget( tempCaptureString, target2 ))
                            {
                                targetLocation = tempCaptureString.find( target, 0 );
                                //Get the stuff we want to exract after the target (comments) using target2
                                extractCommentsTempString = extractTargetString( target, target2, tempCaptureString );
                                //Cut out what we dont want 
                                tempCaptureString.erase( targetLocation, extractCommentsTempString.length() + target2.length());

                                tempCaptureString.insert( targetLocation, ( "    Print( \"" + temp_function_name + "\" );\n    VSPRenew();\n" ) ); // Print function name and call renew for each function
                            }

                            target = R"(\endcode)";
                        
                            //Little different with this one, we erase everything after the target
                            if ( CheckForTarget( tempCaptureString, target ) )
                            {
                                targetLocation = tempCaptureString.find( target, 0 );
                                tempCaptureString.erase( targetLocation );
                            }
                     
                            //close up function block
                            codeFile << tempCaptureString <<"\n}\n\n";
                        }
                    }
                }
            }
        }

        codeFile.close();
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

        std::map<string, std::vector<std::pair<std::string, int> > > enums;

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

            enums[enumdef] = std::vector<std::pair<std::string, int> >();

            for ( unsigned int j = 0; j < count; ++j )
            {
                int value = 0;
                string enumname = e->GetEnumValueByIndex( j, &value );

                enums[enumdef].push_back( make_pair( enumname, value ) );
            }

        }


        // Generate Mainpage Markdown File
        std::ofstream mainpage_md;
        mainpage_md.open( "APIReadme.md", ios_base::out );

        std::string readme_str = R"(

    # Documentation for the OpenVSP API

This is the documentation for the classes, enumerations, and functions available in the OpenVSP API. 

The API functions are organized into the following groups:

)";

        // Alphabetize by title
        // create a empty vector of pairs
        std::vector< std::pair < string, string > > sorted_group_title_vec;

        // copy key-value pairs from the map to the vector
        std::map<std::string, std::string>::iterator git;
        for( git = groupTitles.begin(); git != groupTitles.end(); git++ )
        {
            sorted_group_title_vec.push_back( std::make_pair( git->first, git->second ) );
        }

        // // sort the vector by increasing order of its pair's second value
        sort( sorted_group_title_vec.begin(), sorted_group_title_vec.end(), sortByVal );

        for ( size_t i_g = 0; i_g < sorted_group_title_vec.size(); i_g++ )
        {
            readme_str += "- [";
            readme_str += sorted_group_title_vec[i_g].second;
            readme_str += "]";

            // Identify group HTML name
            readme_str += "(group__";

            // FIXME: asDocInfo group can't contain any underscores!

            for ( size_t i = 0; i < sorted_group_title_vec[i_g].first.size(); i++ )
            {
                if ( isupper( sorted_group_title_vec[i_g].first[i] ) )
                {
                    readme_str += "_";
                    readme_str += tolower( sorted_group_title_vec[i_g].first[i] );
                }
                else
                {
                    readme_str += sorted_group_title_vec[i_g].first[i];
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
### Overview
OpenVSP includes an API written in C++ that exposes all of the functionality of the GUI to a programming interface. This allows
OpenVSP modeling and analysis tools to be run on headless systems, directly integrated with external software programs, and 
automated for trade studies and optimization purposes. The OpenVSP API & MATLAB/Python Integration presentation from 
the [2020 OpenVSP Workshop](http://openvsp.org/wiki/doku.php?id=workshop2020) is a good resource to learn more about the API. 
For specific API questions, the [OpenVSP Google Group](https://groups.google.com/forum/#!forum/openvsp) is available.

### Examples
OpenVSP API examples are available in the **scripts** directory of the distribution. These example scripts are written in 
AngelScript, but map very closely for the Python API. CustomGeom examples, also written in Angelscipt, are available in the 
**CustomScripts** directory. An example for using the Python API can be found in **python/openvsp/openvsp/tests**. The 
matlab_api directory includes examples for the MATLAB API in the form of test suites. 

### Angelscript 
All OpenVSP C++ API functions are registered with Angelscript, a language that is virtually identical in syntax to C++. 
API functions can be called from *.vspscript files and then run from either the GUI (File -> Run Script...) or through the 
vspscript executable's command line interface. Custom Geom scripts, identified by the *.vsppart file extension, also call 
API functions. These scripts are loaded automatically when OpenVSP is launched. Note, all models saved with a Custom Geom 
will include the *.vsppart code in the *.vsp3 file. 

### Python API Instructions
View the **README** file in the **python** directory of the distribution for instructions on Python API installation. Note, the Python
version must be the same as what OpenVSP was compiled with. For instance OpenVSP 3.21.2 Win64 requires Python 3.6-x64. If a different 
verison of Python is desired, the user must compile OpenVSP themselves. 

### MATLAB API
The MATLAB API template (matlab_api) is included with the OpenVSP source code, but not the distributed binaries. This is because it requires
an unreleased branch of SWIG and MATLAB with a valid license. Users are able to build the MATLAB API if they are willing
to compile OpenVSP themselves. Presentations on compiling OpenVSP and building the MATLAB API are available from the 
[2020 OpenVSP Workshop](http://openvsp.org/wiki/doku.php?id=workshop2020)

## Improvements
### Users
Users are encouraged to make use of the [GitHub Issue Tracker](https://github.com/OpenVSP/OpenVSP) if they have a suggestions, 
feature request, or bug report for the OpenVSP developers. Please add an issue if an API function or capability is missing, 
not working correctly, or poorly documented.

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

        //function that creates the API Test Code file
        CreateAPITestDoc( globalFunctions );

        std::string comment_str;

        if ( groupTitles.size() > 0 )
        {
            comment_str = "/*!";

            for ( git = groupTitles.begin(); git != groupTitles.end(); ++git )
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

        for ( git = groupTitles.begin(); git != groupTitles.end(); ++git )
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
