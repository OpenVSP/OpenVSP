//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// materialMgr.cpp: implementation of the materialMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "materialMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
Material::Material()
{


	name = "Default";
	amb[0] = amb[1] = amb[2] = 0.25f;
	amb[3] = 1.0f;

	diff[0] = diff[1] = diff[2] = 0.5f;
	diff[3] = 1.0f;

	spec[0] = spec[1] = spec[2] = 0.5f;
	spec[3] = 1.0f;

	emiss[0] = emiss[1] = emiss[2] = 0.0f;
	emiss[3] = 1.0f;

	shine = 10.0;
}



void Material::bind()
{
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT,   amb );	
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE,   diff );	
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR,  spec );	
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION,  emiss );	
	glMaterialf(  GL_FRONT_AND_BACK, GL_SHININESS, shine );	

}

void Material::read( xmlNodePtr root )
{
	int i;
	double tmp[4];
	xmlNodePtr node;

	char* str = xmlFindString( root, "Name", "Default_Name" );
	name = str;				// name is a stringc

    node = xmlGetNode( root, "Ambient", 0 );
	xmlExtractDoubleArray( node, ',', tmp, 4 );
	for ( i = 0 ; i < 4 ; i++ )
		amb[i] = (float)tmp[i];
	
    node = xmlGetNode( root, "Diffuse", 0 );
	xmlExtractDoubleArray( node, ',', tmp, 4 );
	for ( i = 0 ; i < 4 ; i++ )
		diff[i] = (float)tmp[i];

    node = xmlGetNode( root, "Specular", 0 );
	xmlExtractDoubleArray( node, ',', tmp, 4 );
	for ( i = 0 ; i < 4 ; i++ )
		spec[i] = (float)tmp[i];

    node = xmlGetNode( root, "Emission", 0 );
	xmlExtractDoubleArray( node, ',', tmp, 4 );
	for ( i = 0 ; i < 4 ; i++ )
		emiss[i] = (float)tmp[i];

	node = xmlGetNode( root, "Shininess", 0 );
	shine = (float)xmlExtractDouble( node );

}



//=============================================================//
//=============================================================//

MaterialMgr::MaterialMgr()
{
	Material* def = new Material;
	matVec.push_back( def );


	//readFile();

	//==== Load In Some Def Material Incase List Not Found ====//
	Material* redMat = new Material;
	redMat->name = Stringc("Red_Default");
	redMat->amb[0] = 0.60f;	redMat->amb[1] = 0.00f;	redMat->amb[2] = 0.00f;
	matVec.push_back( redMat );

	Material* greenMat = new Material;
	greenMat->name = Stringc("Green_Default");
	greenMat->amb[0] = 0.00f;	greenMat->amb[1] = 0.60f;	greenMat->amb[2] = 0.00f;
	matVec.push_back( greenMat );

	Material* blueMat = new Material;
	blueMat->name = Stringc("Blue_Default");
	blueMat->amb[0] = 0.00f;	blueMat->amb[1] = 0.00f;	blueMat->amb[2] = 0.60f;
	matVec.push_back( blueMat );
}

MaterialMgr::~MaterialMgr()
{
	for ( int i = 0 ; i < (int)matVec.size() ; i++ )
	{
		delete matVec[i];
	}
}

Material MaterialMgr::getMaterialCopy(int index)
{
	if ( index < 0 || index > (int)matVec.size() )
		index = 0;

	Material mat;
	for ( int i = 0 ; i < 4 ; i++ )
	{
		mat.amb[i]   = matVec[index]->amb[i];
		mat.diff[i]  = matVec[index]->diff[i];
		mat.spec[i]  = matVec[index]->spec[i];
		mat.emiss[i] = matVec[index]->emiss[i];
	}
	mat.shine = matVec[index]->shine;

	return mat;
}
	
Material MaterialMgr::getWhiteMaterial( float brightness, float shine )
{
	Material mat;
	for ( int i = 0 ; i < 3 ; i++ )
	{
		mat.amb[i]   = brightness;
		mat.diff[i]  = brightness;
		mat.spec[i]  = brightness;
	}
	mat.shine = shine;
	return mat;
}


void MaterialMgr::readFile(xmlNodePtr materialDefines)
{
	//==== Read Materials ====//
	int num_mats = xmlGetNumNames( materialDefines, "Material" );

	for ( int i = 0 ; i < num_mats ; i++ )
	{
		xmlNodePtr mat_node = xmlGetNode( materialDefines, "Material", i );

		Material* mat = new Material;
		mat->read( mat_node );
		
		matVec.push_back( mat );

	}
}

Material*  MaterialMgr::getMaterial( int i )
{
	if ( i < 0 || i >= (int)matVec.size() )
		return 0;

	return matVec[i];
}


Single::Single()
{
	static MaterialMgr* ptr = 0; 

    if (!ptr) 
		ptr = new MaterialMgr();

	matMgr = ptr;     
}
