//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Leak Detection http://www.codeproject.com/KB/applications/visualleakdetector.aspx
//#include "vld.h"  

#include <stdio.h>
#ifdef WIN32
#include <windows.h>		
#endif

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Gl_Window.H>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif
#include "mainScreen.h"

#include "main.h"
#include "vspScreen.h"
#include "geomScreen.h"
#include "vspGlWindow.h"
#include "geom.h"
#include "aircraft.h"
#include "screenMgr.h"
#include "scriptMgr.h"
#include "FeaMeshMgr.h"
#include "CfdMeshMgr.h"
#include "VspPreferences.h"

// Include OpenNurbs for Rhino Dump
// ON Needs to be undefined for it to compile
//
#undef ON
#include "opennurbs.h"
#include "opennurbs_extensions.h"

int timerFlag = 0;
float timerTime = 5.0;
Stringc timerScriptFile;
Aircraft* airPtr = NULL;
ScreenMgr* screenMgrPtr = NULL;

//=====================================================//
//===== Batch Mode Check - Parse the Command Line =====//
//=====================================================//
int batchMode(int argc, char *argv[], Aircraft* airPtr)
{

//	printf("number of args = %d : \n", argc);
//	for (int p = 0; p < argc; p++) {
//		printf("%s, ", argv[p]);
//	}
//	printf("\n");


    int i;
    int batchModeFlag = 0;
    int compgeomFlag = 0;
	int meshFlag = 0;
    int sliceFlag = 0;
	int numSlices = 0;
    int writeXsecFlag = 0;
    int writeFelisaFlag = 0;
    int writeStereoFlag = 0;
    int writeTRIFlag = 0;
    int writeRhinoFlag = 0;
    int writeNascartFlag = 0;
	int doxmlFlag = 0;
	int scriptFlag = 0;
	int feaMeshFlag = 0;
	int cfdMeshFlag = 0;
	int setTempDirFlag = 0;
	Stringc tempDirName;
	double cfdMeshScale = 1.0;
	int cfdDefaultSourcesFlag = 1;
	double Mach,sliceAngle;
	int coneSections;
	Stringc scriptFile,xmlFile;
    char airName[255];
    Stringc exec;
	int userParmFlag = 0;
	int userParmID = 0;
	double userParmVal = 0.0;

	//==== Output Names =====//
	enum { COMPGEOM_TXT, COMPGEOM_CSV, CFDMESH_STL, CFDMESH_POLY, CFDMESH_TRI, CFDMESH_OBJ,
		   CFDMESH_DAT, CFDMESH_KEY, FEAMESH_STL, FEAMESH_MASS, FEAMESH_NASTRAN, 
		   FEAMESH_CALC_GEOM, FEAMESH_CALC_THICK, COMPGEOM_DRAG, NUM_OUT_NAMES }; 

	char* outFileTypes[] = {"compgeom_txt", "compgeom_csv", "cfdmesh_stl", "cfdmesh_poly",
	                        "cfdmesh_tri",  "cfdmesh_obj",  "cfdmesh_dat", "cfdmesh_key",
	                        "feamesh_stl",  "feamesh_mass", "feamesh_nastran",
	                        "feamesh_calc_geom",  "feamesh_calc_thick", "compgeom_drag" };
	bool outNameFlags[NUM_OUT_NAMES];
	Stringc outNames[NUM_OUT_NAMES];

	for ( int i = 0 ; i < NUM_OUT_NAMES ; i++ )
		outNameFlags[i] = false;

    memset( airName, '\0', 255 );

    /* Parse the input */
    i = 1;

    while ( i <= argc - 1 ) {

       /* Do batch */
       if ( strcmp(argv[i],"-batch") == 0 ) {
		   if (i+1 < argc)
		   {
	          strcpy( airName, argv[++i] ); 
		      batchModeFlag = 1;
		   }
       }
       if ( strcmp(argv[i],"-script") == 0 ) {
		   if (i+1 < argc)
		   {
				scriptFile = argv[++i]; 
		        scriptFlag = 1;
		   }
       }
	   if ( strcmp(argv[i],"-loop") == 0 ) {
		   if (i+2 < argc)
		   {
				sscanf(argv[++i], "%f", &timerTime);
				timerScriptFile = argv[++i]; 
		        timerFlag = 1;
		   }
       }
       if ( strcmp(argv[i],"-settempdir") == 0 ) {
		   if (i+1 < argc)
		   {
				tempDirName = argv[++i]; 
		        setTempDirFlag = 1;
		   }
       }
       if ( strcmp(argv[i],"-compgeom") == 0 ) {
          compgeomFlag = 1;
       }
       if ( strcmp(argv[i],"-mesh") == 0 ) {
          meshFlag = 1;
       }
       if ( strcmp(argv[i],"-slice") == 0 ) {
		   if (i+1 < argc)
		   {
				numSlices = atoi( argv[++i] );
				Mach = atof(argv[++i] );
				sliceAngle = asin((double)1.0/(double)Mach);
				sliceAngle = RAD2DEG(sliceAngle);
				coneSections = atoi( argv[++i] );
				sliceFlag = 1;
		   }
       }
       if ( strcmp(argv[i],"-xsec") == 0 ) {
          writeXsecFlag = 1;
       }
       if ( strcmp(argv[i],"-felisa") == 0 ) {
          writeFelisaFlag = 1;
       }
       if ( strcmp(argv[i],"-stereo") == 0 ) {
          writeStereoFlag = 1;
       }
       if ( strcmp(argv[i],"-tri") == 0 ) {
          writeTRIFlag = 1;
       }
       if ( strcmp(argv[i],"-rhino") == 0 ) {
          writeRhinoFlag = 1;
       }
	   if ( strcmp(argv[i],"-nascart") == 0 ) {
          writeNascartFlag = 1;
	   }
	   if ( strcmp(argv[i],"-doxml") == 0 ) {
		   if (i+1 < argc)
		   {
			   xmlFile = argv[++i]; 	
			   doxmlFlag = 1;
		   }
	   }
	   if ( strcmp(argv[i],"-feamesh") == 0 ) {
          feaMeshFlag = 1;
	   }

       if ( strcmp(argv[i],"-cfdmesh") == 0 ) {
		   cfdMeshFlag = 1;
		   if ( (i+1 < argc) && ( argv[i+1][0] != '-' ) )
		   {
			   cfdMeshScale = atof( argv[++i] );
			   if ( cfdMeshScale <= 0 )
			   {
				printf( "invalid cfdMeshScale '%s'\n", argv[i] );
				exit( 1 );
			   }
		   }
	   }
       if ( strcmp(argv[i],"-nocfddefsources") == 0 ) {
		   cfdDefaultSourcesFlag = 0;
	   }
       if ( strcmp(argv[i],"-userparm") == 0 ) {
		   if (i+1 < argc && i+2 < argc )
		   {
		       userParmFlag = 1;
			   userParmID  = atoi( argv[++i] );
			   userParmVal = atof( argv[++i] );
		   }
	   }
      if ( strcmp(argv[i],"-outname") == 0 ) 
	   {
		   if (i+1 < argc && i+2 < argc )
		   {
			   for ( int n = 0 ; n < NUM_OUT_NAMES ; n++ )
			   {
					if ( strcmp( argv[i+1], outFileTypes[n] ) == 0 )
					{
						outNameFlags[n] = true;
						outNames[n] = argv[i+2];
						i += 2;
//printf("OutName: %s = %s \n", outFileTypes[n], outNames[n].get_char_star() );

						break;
					}
			   }
		   }
	   }

       if ( strcmp(argv[i],"-help") == 0 ) 
       {
		 printf("\n");
         printf("          %s\n",VSPVERSION1);     
         printf("--------------------------------------------\n");     
         printf("Usage: vsp [inputfile.vsp] (run interactive version)\n");
         printf("     : vsp -batch  <filename>  (batch mode)\n");
         printf("     : vsp -script <filename>  (batch mode)\n");
		 printf("     : vsp -loop <seconds> <filename> (GUI is visible)\n");
         printf("--------------------------------------------\n");     
         printf("\n");     
         printf("VSP batch options listed below:\n");     
         printf("  -compgeom          Batch run compgeom\n" );
         printf("  -slice #           Batch run slice\n" );
         printf("  -mesh              Batch run mesh\n" );
		 printf("  -cfdmesh val       Batch run CFD mesh ( val = scale tri size )\n" );
		 printf("  -nocfddefsources   Do not add default sources.\n" );
         printf("  -feamesh           Batch run fea mesh\n" );
         printf("  -xsec              Write Herm file (Vorview format) \n");
         printf("  -felisa            Write Felisa files \n");
         printf("  -stereo            Write Stereolith file \n");
         printf("  -tri               Write Cart3D file \n");
         printf("  -rhino             Write Rhino3D file \n");
         printf("  -nascart           Write Nascart file \n");
		 printf("  -doxml             Process an external xml file \n");
		 printf("  -userparm # val    Set the value of the user parm \n");
		 printf("  -tempdir pathname  Set the path name of the dir to write temp files\n");
		 printf("  -outname type name Set the filenames for output where type = \n");
		 printf("                      %s, %s, %s, %s, \n", outFileTypes[0], outFileTypes[1], outFileTypes[2], outFileTypes[3] );
		 printf("                      %s, %s, %s, %s, \n", outFileTypes[4], outFileTypes[5], outFileTypes[6], outFileTypes[7] );
		 printf("                      %s, %s, %s \n", outFileTypes[8], outFileTypes[9], outFileTypes[10] );
		 printf("                      %s, %s \n", outFileTypes[11], outFileTypes[12] );
		 //printf("                      cfdmesh_tri, cfdmesh_obj, cfdmesh_dat, cfdmesh_key, \n");
		 //printf("                      feamesh_stl, feamesh_mass, feamesh_nastran, \n");
		 //printf("                      feamesh_calc_geom, feamesh_calc_thick  \n");
		 printf("  -help              This message\n");
         printf("\n");     
         printf("--------------------------------------------\n");     
         return 1;
       }

       i++;      
    }

    if ( batchModeFlag == 0 && scriptFlag == 0 && timerFlag == 0 )
      return 0;


	if ( scriptFlag )
	{
		scriptMgr->setScriptMode(ScriptMgr::SCRIPT);
  	  ScreenMgr* screenMgrPtr = new ScreenMgr(airPtr);
	  airPtr->setScreenMgr( screenMgrPtr );
	  
	  long fileSize = 0;
	  char * buffer = NULL;

	  FILE * filePtr = fopen ( scriptFile , "rb" );
	  if (filePtr!=NULL) 
	  {
		  printf( "processing script...\n");
		  // obtain file size.
		  fseek (filePtr , 0 , SEEK_END);
		  fileSize = ftell (filePtr);
		  rewind (filePtr);

		  // allocate memory to contain the whole file.
		  buffer = (char*) malloc (fileSize+1);
		  if (buffer == NULL) exit (2);

		  // copy the file into the buffer.
		  fread (buffer,1,fileSize,filePtr);
		  buffer[fileSize] = '\0'; // add end

		  scriptMgr->parseScript(Stringc(buffer));

		  // terminate
		  fclose (filePtr);
		  free (buffer);
		  printf( "finished\n");
	  }
	  else
	  {
		  printf( "ERROR: Unable to find file.");
	  }

	}
	else if (batchModeFlag)
	{


		//==== Set Base File Name ====//
		Stringc base_name, slice_name;
 
		Stringc ram_file_name = airName;
		int vsp_loc = ram_file_name.search_for_substring(".vsp");

		if ( vsp_loc < 0 ) 
		  base_name = ram_file_name;
		else
		  base_name = ram_file_name.get_range(0, vsp_loc-1);

		//==== Read VSP File ====//
	//    ram_file_name = base_name;
	//    ram_file_name.concatenate(".vsp");

		//==== Check for File ====//
		airPtr->openFile(ram_file_name);
		airPtr->setActiveGeom( 0 );
		airPtr->update_bbox();



		if ( compgeomFlag )
		{
			//==== CompGeom File Names ====//
			if ( outNameFlags[COMPGEOM_TXT] )
			{
				airPtr->setExortFileName( outNames[0], Aircraft::COMP_GEOM_TXT_TYPE );
			}
			if ( outNameFlags[COMPGEOM_CSV] )
			{
				airPtr->setExportCompGeomCsvFile( true );
				airPtr->setExortFileName( outNames[1], Aircraft::COMP_GEOM_CSV_TYPE );
			}
			if ( outNameFlags[COMPGEOM_DRAG] )
			{
				airPtr->setExportDragBuildTsvFile( true );
				airPtr->setExortFileName( outNames[COMPGEOM_DRAG], Aircraft::DRAG_BUILD_TSV_TYPE );
			}

			Geom* geom = airPtr->comp_geom(0);
		}
		if ( meshFlag )
		{
			Geom* geom = airPtr->comp_geom(0, 1);
		}
		if ( feaMeshFlag )
		{
			//==== FeaMesh Names ====//
			if ( outNameFlags[FEAMESH_STL] )
				feaMeshMgrPtr->SetFeaExportFileName( outNames[FEAMESH_STL], FeaMeshMgr::STL_FEA_NAME );
			if ( outNameFlags[FEAMESH_MASS] )
				feaMeshMgrPtr->SetFeaExportFileName( outNames[FEAMESH_MASS], FeaMeshMgr::MASS_FILE_NAME );
			if ( outNameFlags[FEAMESH_NASTRAN] )
				feaMeshMgrPtr->SetFeaExportFileName( outNames[FEAMESH_NASTRAN], FeaMeshMgr::NASTRAN_FILE_NAME );
			if ( outNameFlags[FEAMESH_CALC_GEOM] )
				feaMeshMgrPtr->SetFeaExportFileName( outNames[FEAMESH_CALC_GEOM], FeaMeshMgr::GEOM_FILE_NAME );
			if ( outNameFlags[FEAMESH_CALC_THICK] )
				feaMeshMgrPtr->SetFeaExportFileName( outNames[FEAMESH_CALC_THICK], FeaMeshMgr::THICK_FILE_NAME );

			feaMeshMgrPtr->SetBatchFlag( true );
			feaMeshMgrPtr->LoadSurfaces();
			feaMeshMgrPtr->Build();
			feaMeshMgrPtr->Export();
		}
		if ( userParmFlag )
		{
			airPtr->getUserGeom()->SetUserParmValue( userParmID, userParmVal );
			airPtr->update_bbox();
		}

		if (sliceFlag)
		{
			slice_name = Stringc("slice.txt");
			Geom* geom = airPtr->slice(2,numSlices,(double)sliceAngle,(double)coneSections,slice_name);
		}
		if ( cfdMeshFlag )
		{
			//==== CFD Mesh File Names ====//
			if ( outNameFlags[CFDMESH_STL] )
				cfdMeshMgrPtr->SetExportFileName( outNames[CFDMESH_STL], CfdMeshMgr::STL_FILE_NAME );
			if ( outNameFlags[CFDMESH_POLY] )
				cfdMeshMgrPtr->SetExportFileName( outNames[CFDMESH_POLY], CfdMeshMgr::POLY_FILE_NAME );
			if ( outNameFlags[CFDMESH_TRI] )
				cfdMeshMgrPtr->SetExportFileName( outNames[CFDMESH_TRI], CfdMeshMgr::TRI_FILE_NAME );
			if ( outNameFlags[CFDMESH_OBJ] )
				cfdMeshMgrPtr->SetExportFileName( outNames[CFDMESH_OBJ], CfdMeshMgr::OBJ_FILE_NAME );
			if ( outNameFlags[CFDMESH_DAT] )
				cfdMeshMgrPtr->SetExportFileName( outNames[CFDMESH_DAT], CfdMeshMgr::DAT_FILE_NAME );
			if ( outNameFlags[CFDMESH_KEY] )
				cfdMeshMgrPtr->SetExportFileName( outNames[CFDMESH_KEY], CfdMeshMgr::KEY_FILE_NAME );

			cfdMeshMgrPtr->SetBatchFlag( true );
			Stringc bezTempFile = airPtr->getTempDir();
			bezTempFile.concatenate( Stringc( "cfdmesh.bez" ) );

			airPtr->write_bezier_file( bezTempFile );
			cfdMeshMgrPtr->CleanUp();
			cfdMeshMgrPtr->ReadSurfs( bezTempFile );
			cfdMeshMgrPtr->UpdateSources();
			if ( cfdDefaultSourcesFlag )
			{
				cfdMeshMgrPtr->AddDefaultSources();
				cfdMeshMgrPtr->UpdateSources();
			}
			cfdMeshMgrPtr->ScaleTriSize( cfdMeshScale );
			cfdMeshMgrPtr->BuildGrid();
			cfdMeshMgrPtr->Intersect();
			cfdMeshMgrPtr->UpdateSources();
			cfdMeshMgrPtr->InitMesh();
			cfdMeshMgrPtr->Remesh( CfdMeshMgr::NO_OUTPUT );

			//Stringc stereo_file_name = base_name;
			//stereo_file_name.concatenate(".stl");
			//cfdMeshMgrPtr->WriteSTL( stereo_file_name );
			//Stringc bodyin_dat_file_name = base_name;
			//bodyin_dat_file_name.concatenate("bodyin.dat");
			//Stringc bodyin_key_file_name = base_name;
			//bodyin_key_file_name.concatenate("bodyin.key");

//			cfdMeshMgrPtr->WriteNASCART(bodyin_dat_file_name, bodyin_key_file_name );
			//cfdMeshMgrPtr->WriteTetGen("tetgen.poly");
			Stringc resultTxt = cfdMeshMgrPtr->CheckWaterTight();
			printf( resultTxt.get_char_star() );

			fflush( stdout );

			cfdMeshMgrPtr->SetExportFileFlag( true, CfdMeshMgr::STL_FILE_NAME );
			cfdMeshMgrPtr->SetExportFileFlag( true, CfdMeshMgr::POLY_FILE_NAME );
			cfdMeshMgrPtr->SetExportFileFlag( true, CfdMeshMgr::TRI_FILE_NAME );
			cfdMeshMgrPtr->SetExportFileFlag( true, CfdMeshMgr::OBJ_FILE_NAME );
			cfdMeshMgrPtr->SetExportFileFlag( true, CfdMeshMgr::DAT_FILE_NAME );
			cfdMeshMgrPtr->SetExportFileFlag( true, CfdMeshMgr::KEY_FILE_NAME );

			cfdMeshMgrPtr->ExportFiles();
		}


		if ( writeXsecFlag )
		{
		  Stringc xsec_vorview_file_name = base_name;
		  xsec_vorview_file_name.concatenate(".hrm");

		  airPtr->write_xsec_file( xsec_vorview_file_name );
		  printf( "xsec file name = %s\n",  xsec_vorview_file_name() );
		}

		if ( writeFelisaFlag )
		{
		  Stringc felisa_file_name = base_name;
		  felisa_file_name.concatenate(".fel");

		  Stringc felisa_background_file_name = base_name;
		  felisa_background_file_name.concatenate(".bac");

		  Stringc fps3d_bco_file_name = base_name;
		  fps3d_bco_file_name.concatenate(".bsf");

  		  airPtr->write_felisa_file( felisa_file_name );
		  airPtr->write_felisa_background_file( felisa_background_file_name );
		  airPtr->write_fps3d_bco_file( fps3d_bco_file_name );
		  printf( "felissa file name = %s, %s, %s\n",  felisa_file_name(), felisa_background_file_name(), fps3d_bco_file_name() );
		}
		if ( writeStereoFlag )
		{
		  Stringc stereo_file_name = base_name;
		  stereo_file_name.concatenate(".stl");

  		  airPtr->write_stl_file( stereo_file_name );
		  printf( "stereolith file name = %s\n",  stereo_file_name());
		}
		if ( writeTRIFlag )
		{
		  Stringc tri_file_name = base_name;
		  tri_file_name.concatenate(".tri");

  		  airPtr->write_cart3d_files( tri_file_name );
		  printf( "cart3d file name = %s\n",  tri_file_name());
		}
		if ( writeRhinoFlag )
		{
		  Stringc rhino_file_name = base_name;
		  rhino_file_name.concatenate(".3dm");

  		  airPtr->write_rhino_file( rhino_file_name );
		  printf( "rhino3d file name = %s\n",  rhino_file_name());
		}
		if ( writeNascartFlag )
		{
			//==== Extract Path ====//
			Stringc nascart_file_name = base_name;

			int last = -1;
			for ( i = 0 ; i < nascart_file_name.get_length() ; i++ )
			{
				if ( nascart_file_name[i] == '/' )
					last = i;
			}

			if ( last == -1 )
				nascart_file_name = Stringc("bodyin.dat");

			else
			{
				nascart_file_name.delete_range(last+1,nascart_file_name.get_length()-1);
				nascart_file_name.concatenate( "bodyin.dat" );
			}

			airPtr->write_nascart_files( nascart_file_name );
			printf( "nascart file name = %s\n",  nascart_file_name());
		}
		if ( setTempDirFlag )
		{
			airPtr->setTempDir( tempDirName, true );
		}
		



	}
    return (batchModeFlag || scriptFlag);

}

void vsp_exit()
{
	if ( screenMgrPtr )
		delete screenMgrPtr;

	if ( airPtr )
		delete airPtr;



	exit(0);
}


void autoSaveTimeoutHandler(void *data)
{
	if ( airPtr )
	{
		Stringc backupfn = airPtr->getTempDir();
		backupfn.concatenate("vsp_autobackup.vsp"); 
		airPtr->writeFile(backupfn.get_char_star(), true);
	}

	Fl::repeat_timeout(VspPreferences::Instance()->autoSaveInterval, autoSaveTimeoutHandler, data );
}


//========================= Main =========================//
int main( int argc, char** argv)
{

//FILE* filePtr = fopen("debug.txt", "w" );
//freopen("debug.txt", "w", stdout); 

#ifdef CHECK_FOR_KEY
	if ( !validKey() )
	{
		printf( "Invalid Reg Key\n" );
		printf( "Press Enter to Exit\n" );
		char str[256];
		gets( str );
		exit(0);
	}
#endif
	
	airPtr = new Aircraft();

	if ( batchMode( argc, argv, airPtr ) )
	    exit(0);

	screenMgrPtr = new ScreenMgr(airPtr);

	//==== Link Objects ====//
	airPtr->setScreenMgr( screenMgrPtr );

	// if only 1 argument it is the input filename
	if (argc == 2)
	{
		screenMgrPtr->s_open(ScriptMgr::GUI, argv[argc-1]);
	}

	// this works better after the 
	if (timerFlag) 
	{
		screenMgrPtr->showGui();
		Fl::add_timeout(timerTime, ScriptMgr::staticTimeoutHandler, scriptMgr);
		scriptMgr->setTimeout(timerTime, timerScriptFile);
	} 
	else 
	{
		if (VspPreferences::Instance()->autoSaveInterval > 0)
			Fl::add_timeout( VspPreferences::Instance()->autoSaveInterval, autoSaveTimeoutHandler, airPtr );
		screenMgrPtr->showGui( argc-1, argv );
	}

	return Fl::run();
		
//fclose(filePtr);

}
