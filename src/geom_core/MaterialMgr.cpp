#include "MaterialMgr.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "XmlUtil.h"

#include <assert.h>


Material::Material()
{
    SetMaterialToDefault();
}

Material::~Material()
{
}

xmlNodePtr Material::EncodeNameXml( xmlNodePtr & node )
{
    xmlNodePtr material_node = xmlNewChild( node, NULL, BAD_CAST "Material", NULL );
    if ( material_node )
    {
        XmlUtil::AddStringNode( material_node, "Name", m_Name );
    }
    return material_node;
}

xmlNodePtr Material::DecodeNameXml( xmlNodePtr & node )
{
    xmlNodePtr material_node = XmlUtil::GetNode( node, "Material", 0 );
    if ( material_node )
    {
        string name = XmlUtil::FindString(  material_node, "Name", m_Name );
        SetMaterial( name );
    }
    return material_node;
}

xmlNodePtr Material::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr material_node = xmlNewChild( node, NULL, BAD_CAST "Material", NULL );
    if ( material_node )
    {
        XmlUtil::AddStringNode( material_node, "Name", m_Name );

        vector < double > a( m_Ambi, m_Ambi + 4 );
        XmlUtil::AddVectorDoubleNode( material_node, "Ambient", a );
        vector < double > d( m_Diff, m_Diff + 4 );
        XmlUtil::AddVectorDoubleNode( material_node, "Diffuse", d );
        vector < double > s( m_Spec, m_Spec + 4 );
        XmlUtil::AddVectorDoubleNode( material_node, "Specular", s );
        vector < double > e( m_Emis, m_Emis + 4 );
        XmlUtil::AddVectorDoubleNode( material_node, "Emissive", e );

        XmlUtil::AddDoubleNode( material_node, "Shininess", m_Shininess);
    }
    return material_node;
}

xmlNodePtr Material::DecodeXml( xmlNodePtr & material_node )
{
    if ( material_node )
    {
        m_Name = XmlUtil::FindString(  material_node, "Name", m_Name );

        vector < double > a = XmlUtil::ExtractVectorDoubleNode( material_node, "Ambient" );
        vector < double > d = XmlUtil::ExtractVectorDoubleNode( material_node, "Diffuse" );
        vector < double > s = XmlUtil::ExtractVectorDoubleNode( material_node, "Specular" );
        vector < double > e = XmlUtil::ExtractVectorDoubleNode( material_node, "Emissive" );

        m_Shininess = (float)XmlUtil::FindDouble( material_node, "Shininess", m_Shininess );

        std::copy( a.begin(), a.end(), m_Ambi );
        std::copy( d.begin(), d.end(), m_Diff );
        std::copy( s.begin(), s.end(), m_Spec );
        std::copy( e.begin(), e.end(), m_Emis );

    }
    return material_node;
}

void Material::SetMaterialToDefault( )
{
    m_Name = "Default";
    m_Ambi[0] = 0.2; m_Ambi[1] = 0.2; m_Ambi[2] = 0.2, m_Ambi[3] = 1.0;
    m_Diff[0] = 0.8; m_Diff[1] = 0.8; m_Diff[2] = 0.8; m_Diff[3] = 1.0;
    m_Spec[0] = 0.0; m_Spec[1] = 0.0; m_Spec[2] = 0.0; m_Spec[3] = 1.0;
    m_Emis[0] = 0.0; m_Emis[1] = 0.0; m_Emis[2] = 0.0; m_Emis[3] = 1.0;
    m_Shininess = 128.0;
    m_UserMaterial = false;
}

void Material::SetMaterial( std::string name )
{
    MaterialMgr.FindMaterial( name, *this );
}

void Material::SetMaterial( Material * material )
{
    for ( int j = 0; j < 4; j++ )
        m_Ambi[j] = material->m_Ambi[j];

    for ( int j = 0; j < 4; j++ )
        m_Diff[j] = material->m_Diff[j];

    for ( int j = 0; j < 4; j++ )
        m_Spec[j] = material->m_Spec[j];

    for ( int j = 0; j < 4; j++ )
        m_Emis[j] = material->m_Emis[j];

    m_Name = material->m_Name;
    m_Shininess = material->m_Shininess;
}

void Material::SetMaterial( std::string name, double ambi[], double diff[], double spec[], double emis[], double shin )
{
    for ( int j = 0; j < 4; j++ )
        m_Ambi[j] = ambi[j];

    for ( int j = 0; j < 4; j++ )
        m_Diff[j] = diff[j];

    for ( int j = 0; j < 4; j++ )
        m_Spec[j] = spec[j];

    for ( int j = 0; j < 4; j++ )
        m_Emis[j] = emis[j];

    m_Name = name;
    m_Shininess = (float)shin;
}

void Material::SetAmbient( vec3d color )
{
    for ( int j = 0; j < 3; j++ )
        m_Ambi[j] = color[j] / 255.0;
}

void Material::SetDiffuse( vec3d color )
{
    for ( int j = 0; j < 3; j++ )
        m_Diff[j] = color[j] / 255.0;
}

void Material::SetSpecular( vec3d color )
{
    for ( int j = 0; j < 3; j++ )
        m_Spec[j] = color[j] / 255.0;
}

void Material::SetEmissive( vec3d color )
{
    for ( int j = 0; j < 3; j++ )
        m_Emis[j] = color[j] / 255.0;
}

void Material::SetAlpha( double alpha )
{
    m_Diff[3] = alpha;
}

void Material::SetShininess( double shiny )
{
    m_Shininess = (float)shiny;
}

void Material::GetAmbient( vec3d &color )
{
    for ( int j = 0; j < 3; j++ )
        color[j] = m_Ambi[j] * 255.0;
}

void Material::GetDiffuse( vec3d &color )
{
    for ( int j = 0; j < 3; j++ )
        color[j] = m_Diff[j] * 255.0;
}

void Material::GetSpecular( vec3d &color )
{
    for ( int j = 0; j < 3; j++ )
        color[j] = m_Spec[j] * 255.0;
}

void Material::GetEmissive( vec3d &color )
{
    for ( int j = 0; j < 3; j++ )
        color[j] = m_Emis[j] * 255.0;
}

void Material::GetAlpha( double &alpha )
{
    alpha = m_Diff[3];
}

void Material::GetShininess( double &shiny )
{
    shiny = m_Shininess;
}

MaterialMgrSingleton::MaterialMgrSingleton()
{
    m_Alpha.Init( "Alpha", "Material", this, 1.0, 0.0, 1.0 );
    m_Shininess.Init( "Shininess", "Material", this, 0.0, 0.0, 128 );

    Material mat;

    mat.m_Name = "Emerald";
    mat.m_Ambi[0] = 0.0215; mat.m_Ambi[1] = 0.1745; mat.m_Ambi[2] = 0.0215, mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.07568; mat.m_Diff[1] = 0.61424; mat.m_Diff[2] = 0.07568; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.633; mat.m_Spec[1] = 0.727811; mat.m_Spec[2] = 0.633; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 76.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Jade";
    mat.m_Ambi[0] = 0.135; mat.m_Ambi[1] = 0.2225; mat.m_Ambi[2] = 0.1575; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.54; mat.m_Diff[1] = 0.89; mat.m_Diff[2] = 0.63; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.316; mat.m_Spec[1] = 0.316; mat.m_Spec[2] = 0.316; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 12.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Obsidian";
    mat.m_Ambi[0] = 0.054; mat.m_Ambi[1] = 0.05; mat.m_Ambi[2] = 0.066; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.183; mat.m_Diff[1] = 0.17; mat.m_Diff[2] = 0.225; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.333; mat.m_Spec[1] = 0.329; mat.m_Spec[2] = 0.346; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 38.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Pearl";
    mat.m_Ambi[0] = 0.250; mat.m_Ambi[1] = 0.207; mat.m_Ambi[2] = 0.207; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 1.000; mat.m_Diff[1] = 0.829; mat.m_Diff[2] = 0.829; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.297; mat.m_Spec[1] = 0.297; mat.m_Spec[2] = 0.297; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 11.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Ruby";
    mat.m_Ambi[0] = 0.175; mat.m_Ambi[1] = 0.01175; mat.m_Ambi[2] = 0.01175; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.61424; mat.m_Diff[1] = 0.04136; mat.m_Diff[2] = 0.04136; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.727811; mat.m_Spec[1] = 0.626959; mat.m_Spec[2] = 0.626959; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 76.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Turquoise";
    mat.m_Ambi[0] = 0.1; mat.m_Ambi[1] = 0.18725; mat.m_Ambi[2] = 0.1745; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.396; mat.m_Diff[1] = 0.74151; mat.m_Diff[2] = 0.69102; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.297254; mat.m_Spec[1] = 0.30829; mat.m_Spec[2] = 0.306678; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 12.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Brass";
    mat.m_Ambi[0] = 0.330; mat.m_Ambi[1] = 0.223; mat.m_Ambi[2] = 0.0275; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.78; mat.m_Diff[1] = 0.57; mat.m_Diff[2] = 0.11; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.99; mat.m_Spec[1] = 0.94; mat.m_Spec[2] = 0.81; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 27.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Bronze";
    mat.m_Ambi[0] = 0.2125; mat.m_Ambi[1] = 0.1275; mat.m_Ambi[2] = 0.054; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.714; mat.m_Diff[1] = 0.4284; mat.m_Diff[2] = 0.18144; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.393548; mat.m_Spec[1] = 0.271906; mat.m_Spec[2] = 0.166721; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 27.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Chrome";
    mat.m_Ambi[0] = 0.25; mat.m_Ambi[1] = 0.25; mat.m_Ambi[2] = 0.25; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.4; mat.m_Diff[1] = 0.4; mat.m_Diff[2] = 0.4; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.77; mat.m_Spec[1] = 0.77; mat.m_Spec[2] = 0.77; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 77.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Copper";
    mat.m_Ambi[0] = 0.19125; mat.m_Ambi[1] = 0.0735; mat.m_Ambi[2] = 0.0225; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.7038; mat.m_Diff[1] = 0.27048; mat.m_Diff[2] = 0.0828; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.256777; mat.m_Spec[1] = 0.137622; mat.m_Spec[2] = 0.086014; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 13.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Gold";
    mat.m_Ambi[0] = 0.25; mat.m_Ambi[1] = 0.20; mat.m_Ambi[2] = 0.07; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.75; mat.m_Diff[1] = 0.61; mat.m_Diff[2] = 0.22; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.62; mat.m_Spec[1] = 0.55; mat.m_Spec[2] = 0.37; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 51.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Silver";
    mat.m_Ambi[0] = 0.20; mat.m_Ambi[1] = 0.20; mat.m_Ambi[2] = 0.20; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.50; mat.m_Diff[1] = 0.50; mat.m_Diff[2] = 0.50; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.51; mat.m_Spec[1] = 0.51; mat.m_Spec[2] = 0.51; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 51.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Aluminum";
    mat.m_Ambi[0] = 0.25, mat.m_Ambi[1] = 0.25, mat.m_Ambi[2] = 0.25, mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.5, mat.m_Diff[1] = 0.5, mat.m_Diff[2] = 0.5, mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.6, mat.m_Spec[1] = 0.6, mat.m_Spec[2] = 0.6, mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0, mat.m_Emis[1] = 0.0, mat.m_Emis[2] = 0.0, mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 5.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Shiny Gold";
    mat.m_Ambi[0] = 0.4; mat.m_Ambi[1] = 0.2; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.9; mat.m_Diff[1] = 0.5; mat.m_Diff[2] = 0.0; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.9; mat.m_Spec[1] = 0.9; mat.m_Spec[2] = 0.0; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 2.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Black Plastic";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.0; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.01; mat.m_Diff[1] = 0.01; mat.m_Diff[2] = 0.01; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.50; mat.m_Spec[1] = 0.50; mat.m_Spec[2] = 0.50; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 32.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Cyan Plastic";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.1; mat.m_Ambi[2] = 0.06; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.0; mat.m_Diff[1] = 0.51; mat.m_Diff[2] = 0.51; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.50; mat.m_Spec[1] = 0.50; mat.m_Spec[2] = 0.50; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 32.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name =  "Green Plastic";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.0; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.1; mat.m_Diff[1] = 0.35; mat.m_Diff[2] = 0.1; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.45; mat.m_Spec[1] = 0.55; mat.m_Spec[2] = 0.45; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 32.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Red Plastic";
    mat.m_Ambi[0] = 0.0, mat.m_Ambi[1] = 0.0, mat.m_Ambi[2] = 0.0, mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.5, mat.m_Diff[1] = 0.0, mat.m_Diff[2] = 0.0, mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.7, mat.m_Spec[1] = 0.6, mat.m_Spec[2] = 0.6, mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0, mat.m_Emis[1] = 0.0, mat.m_Emis[2] = 0.0, mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 32.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Blue Plastic";
    mat.m_Ambi[0] = 0.5; mat.m_Ambi[1] = 0.5; mat.m_Ambi[2] = 0.87; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.4; mat.m_Diff[1] = 0.4; mat.m_Diff[2] = 0.7; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 1.0; mat.m_Spec[1] = 1.0; mat.m_Spec[2] = 1.0; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 35.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Yellow Plastic";
    mat.m_Ambi[0] = 0.3; mat.m_Ambi[1] = 0.2; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.6; mat.m_Diff[1] = 0.4; mat.m_Diff[2] = 0.1; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.55; mat.m_Spec[1] = 0.55; mat.m_Spec[2] = 0.35; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 30.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

// Materials from reference, but they don't look good.
//    mat.name = "White Plastic";
//    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
//    mat.diff[0] = 0.55; mat.diff[1] = 0.55; mat.diff[2] = 0.55; mat.diff[3] = 1.0;
//    mat.spec[0] = 0.7; mat.spec[1] = 0.7; mat.spec[2] = 0.7; mat.spec[3] = 1.0;
//    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
//    mat.shininess = 32.0;
//    mat.m_UserMaterial = false;
//    m_Materials.push_back(mat);
//
//    mat.name = "Yellow Plastic 2";
//    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
//    mat.diff[0] = 0.5; mat.diff[1] = 0.5; mat.diff[2] = 0.0; mat.diff[3] = 1.0;
//    mat.spec[0] = 0.6; mat.spec[1] = 0.6; mat.spec[2] = 0.5; mat.spec[3] = 1.0;
//    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
//    mat.shininess = 32.0;
//    mat.m_UserMaterial = false;
//    m_Materials.push_back(mat);

    mat.m_Name = "White";
    mat.m_Ambi[0] = 0.9; mat.m_Ambi[1] = 0.9; mat.m_Ambi[2] = 0.9; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.8; mat.m_Diff[1] = 0.8; mat.m_Diff[2] = 0.8; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.8; mat.m_Spec[1] = 0.8; mat.m_Spec[2] = 0.8; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 40.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Black Rubber";
    mat.m_Ambi[0] = 0.02; mat.m_Ambi[1] = 0.02; mat.m_Ambi[2] = 0.02; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.01; mat.m_Diff[1] = 0.01; mat.m_Diff[2] = 0.01; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.4; mat.m_Spec[1] = 0.4; mat.m_Spec[2] = 0.4; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Cyan Rubber";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.05; mat.m_Ambi[2] = 0.05; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.4; mat.m_Diff[1] = 0.5; mat.m_Diff[2] = 0.5; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.04; mat.m_Spec[1] = 0.7; mat.m_Spec[2] = 0.7; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Green Rubber";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.05; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.4; mat.m_Diff[1] = 0.5; mat.m_Diff[2] = 0.4; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.04; mat.m_Spec[1] = 0.7; mat.m_Spec[2] = 0.04; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Red Rubber";
    mat.m_Ambi[0] = 0.05; mat.m_Ambi[1] = 0.0; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.5; mat.m_Diff[1] = 0.4; mat.m_Diff[2] = 0.4; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.7; mat.m_Spec[1] = 0.04; mat.m_Spec[2] = 0.04; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Blue Rubber";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.0; mat.m_Ambi[2] = 0.05; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.4; mat.m_Diff[1] = 0.4; mat.m_Diff[2] = 0.5; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.04; mat.m_Spec[1] = 0.04; mat.m_Spec[2] = 0.7; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "White Rubber";
    mat.m_Ambi[0] = 0.05; mat.m_Ambi[1] = 0.05; mat.m_Ambi[2] = 0.05; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.5; mat.m_Diff[1] = 0.5; mat.m_Diff[2] = 0.5; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.7; mat.m_Spec[1] = 0.7; mat.m_Spec[2] = 0.7; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Yellow Rubber";
    mat.m_Ambi[0] = 0.05; mat.m_Ambi[1] = 0.05; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.5; mat.m_Diff[1] = 0.5; mat.m_Diff[2] = 0.4; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.7; mat.m_Spec[1] = 0.7; mat.m_Spec[2] = 0.04; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Glass Light";
    mat.m_Ambi[0] = 0.2; mat.m_Ambi[1] = 0.2; mat.m_Ambi[2] = 0.2; mat.m_Ambi[3] = 0.2;
    mat.m_Diff[0] = 0.1; mat.m_Diff[1] = 0.1; mat.m_Diff[2] = 0.1; mat.m_Diff[3] = 0.33;
    mat.m_Spec[0] = 0.7; mat.m_Spec[1] = 0.7; mat.m_Spec[2] = 0.7; mat.m_Spec[3] = 0.7;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 5.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Glass Med";
    mat.m_Ambi[0] = 0.2; mat.m_Ambi[1] = 0.2; mat.m_Ambi[2] = 0.2; mat.m_Ambi[3] = 0.2;
    mat.m_Diff[0] = 0.1; mat.m_Diff[1] = 0.1; mat.m_Diff[2] = 0.1; mat.m_Diff[3] = 0.5;
    mat.m_Spec[0] = 0.7; mat.m_Spec[1] = 0.7; mat.m_Spec[2] = 0.7; mat.m_Spec[3] = 0.7;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 5.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Glass Dark";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.0; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 0.0;
    mat.m_Diff[0] = 0.0; mat.m_Diff[1] = 0.0; mat.m_Diff[2] = 0.0; mat.m_Diff[3] = 0.67;
    mat.m_Spec[0] = 0.7; mat.m_Spec[1] = 0.7; mat.m_Spec[2] = 0.7; mat.m_Spec[3] = 0.7;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 5.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Glass Golden";
    mat.m_Ambi[0] = 0.27; mat.m_Ambi[1] = 0.28; mat.m_Ambi[2] = 0.23; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.61; mat.m_Diff[1] = 0.41; mat.m_Diff[2] = 0.27; mat.m_Diff[3] = 0.5;
    mat.m_Spec[0] = 0.97; mat.m_Spec[1] = 0.64; mat.m_Spec[2] = 0.28; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Blank";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.0; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.0; mat.m_Diff[1] = 0.0; mat.m_Diff[2] = 0.0; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.0; mat.m_Spec[1] = 0.0; mat.m_Spec[2] = 0.0; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 0.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Red Default";
    mat.m_Ambi[0] = 0.6; mat.m_Ambi[1] = 0.0; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.5; mat.m_Diff[1] = 0.5; mat.m_Diff[2] = 0.5; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.5; mat.m_Spec[1] = 0.5; mat.m_Spec[2] = 0.5; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Green Default";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.6; mat.m_Ambi[2] = 0.0; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.5; mat.m_Diff[1] = 0.5; mat.m_Diff[2] = 0.5; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.5; mat.m_Spec[1] = 0.5; mat.m_Spec[2] = 0.5; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

    mat.m_Name = "Blue Default";
    mat.m_Ambi[0] = 0.0; mat.m_Ambi[1] = 0.0; mat.m_Ambi[2] = 0.6; mat.m_Ambi[3] = 1.0;
    mat.m_Diff[0] = 0.5; mat.m_Diff[1] = 0.5; mat.m_Diff[2] = 0.5; mat.m_Diff[3] = 1.0;
    mat.m_Spec[0] = 0.5; mat.m_Spec[1] = 0.5; mat.m_Spec[2] = 0.5; mat.m_Spec[3] = 1.0;
    mat.m_Emis[0] = 0.0; mat.m_Emis[1] = 0.0; mat.m_Emis[2] = 0.0; mat.m_Emis[3] = 1.0;
    mat.m_Shininess = 10.0;
    mat.m_UserMaterial = false;
    m_Materials.push_back(mat);

}

MaterialMgrSingleton::~MaterialMgrSingleton()
{
}

void MaterialMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    Geom* g = VehicleMgr.GetVehicle()->FindGeom( m_ActiveGeom );
    if ( g )
    {
        if ( parm_ptr->GetID() == m_Alpha.GetID() )
        {
            g->GetMaterial()->SetAlpha( m_Alpha.Get() );
        }
        else if ( parm_ptr->GetID() == m_Shininess.GetID() )
        {
            g->GetMaterial()->SetShininess( m_Shininess.Get() );
        }
    }

    if ( VehicleMgr.GetVehicle() )
    {
        VehicleMgr.GetVehicle()->ParmChanged( parm_ptr, type );
    }
}

bool MaterialMgrSingleton::FindMaterial( std::string name, Material& mat_out )
{
    for( int i = 0; i < (int)m_Materials.size(); i++ )
    {
        if( name == m_Materials[i].m_Name )
        {
            mat_out = m_Materials[i];
            return true;
        }
    }
    return false;
}

bool MaterialMgrSingleton::FindMaterial( int index, Material& mat_out )
{
    if( index >= 0 && index < (int)m_Materials.size() )
    {
        mat_out = m_Materials[index];
        return true;
    }
    return false;
}

std::vector<std::string> MaterialMgrSingleton::GetNames()
{
    std::vector<std::string> names;

    for( int i = 0; i < (int)m_Materials.size(); i++ )
    {
        names.push_back(m_Materials[i].m_Name);
    }

    return names;
}

void MaterialMgrSingleton::AddMaterial( const Material &mat )
{
    m_Materials.push_back( mat );
}

xmlNodePtr MaterialMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr materials_node = xmlNewChild( node, NULL, BAD_CAST "Materials", NULL );
    if ( materials_node )
    {
        for( int i = 0; i < (int)m_Materials.size(); i++ )
        {
            if( m_Materials[i].m_UserMaterial )
            {
                m_Materials[i].EncodeXml( materials_node );
            }
        }
    }
    return materials_node;
}

xmlNodePtr MaterialMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr materials_node = XmlUtil::GetNode( node, "Materials", 0 );
    if ( materials_node )
    {
        int num = XmlUtil::GetNumNames( materials_node, "Material" );
        for ( int i = 0 ; i < num ; i++ )
        {
            xmlNodePtr mat_node = XmlUtil::GetNode( materials_node, "Material", i );
            if ( mat_node )
            {

                Material mat;
                mat.DecodeXml( mat_node );
                mat.m_UserMaterial = true;

                AddMaterial( mat );
            }
        }
    }
    return materials_node;
}
