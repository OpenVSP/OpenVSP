#include "VspPreferences.h"
#include "XmlUtil.h"

VspPreferences::VspPreferences()
{
    xmlDocPtr doc;
    xmlNodePtr mat_root;

    LIBXML_TEST_VERSION
    xmlKeepBlanksDefault( 0 );

    doc = xmlParseFile( "VspPreferences.xml" );

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == NULL )
    {
        fprintf( stderr, "empty document\n" );
        xmlFreeDoc( doc );
    }
    else
    {
        mat_root = XmlUtil::GetNode( root, "Vsp_Material_Defines", 0 );

        int numOfMats = XmlUtil::GetNumNames( mat_root, "Material" );
        for( int i = 0; i < numOfMats; i++ )
        {
            MaterialPref mat;

            xmlNodePtr mat_node = XmlUtil::GetNode( mat_root, "Material", i );

            xmlNodePtr nam_node = XmlUtil::GetNode( mat_node, "Name", 0 );
            xmlNodePtr amb_node = XmlUtil::GetNode( mat_node, "Ambient", 0 );
            xmlNodePtr dif_node = XmlUtil::GetNode( mat_node, "Diffuse", 0 );
            xmlNodePtr spe_node = XmlUtil::GetNode( mat_node, "Specular", 0 );
            xmlNodePtr emi_node = XmlUtil::GetNode( mat_node, "Emission", 0 );
            xmlNodePtr shi_node = XmlUtil::GetNode( mat_node, "Shininess", 0 );
        
            mat.name = XmlUtil::ExtractString( nam_node );
            mat.shininess = (float)XmlUtil::ExtractDouble( shi_node );

            XmlUtil::ExtractDoubleArray( amb_node, ',', &mat.ambi[0], 4 );
            XmlUtil::ExtractDoubleArray( dif_node, ',', &mat.diff[0], 4 );
            XmlUtil::ExtractDoubleArray( spe_node, ',', &mat.spec[0], 4 );
            XmlUtil::ExtractDoubleArray( emi_node, ',', &mat.emis[0], 4 );

            m_MaterialPrefs.push_back( mat );
        }
    }
}
VspPreferences::~VspPreferences()
{
}

bool VspPreferences::findMaterialPref( std::string name, VspPreferences::MaterialPref& out )
{
    for(int i = 0; i < (int) m_MaterialPrefs.size(); i++)
    {
        if( m_MaterialPrefs[i].name == name )
        {
            out = m_MaterialPrefs[i];
            return true;
        }
    }
    return false;
}

bool VspPreferences::findMaterialPref( int index, VspPreferences::MaterialPref& out )
{
    if( index >= 0 && index < (int)m_MaterialPrefs.size() )
    {
        out = m_MaterialPrefs[index];
        return true;
    }
    return false;
}
